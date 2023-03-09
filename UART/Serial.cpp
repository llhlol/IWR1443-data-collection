#include "Serial.h"
#include "Log.h"

#include <cassert>

Serial::Serial() noexcept
    : IAsync(),
      fileHandle(INVALID_HANDLE_VALUE),
      port(),
      overlappedRead(),
      overlappedWrite(),
      readBuffer(),
      bytesRead(),
      dataToWrite() {}

Serial::~Serial() noexcept {
    if (fileHandle != INVALID_HANDLE_VALUE) {
        assert(overlappedRead.hEvent != nullptr);
        assert(overlappedWrite.hEvent != nullptr);

        CloseHandle(overlappedRead.hEvent);
        CloseHandle(overlappedWrite.hEvent);
        CloseHandle(fileHandle);
    }
}

auto Serial::Initialize(std::string_view portName, uint32_t baudRate) noexcept -> std::error_code {
    if (fileHandle != INVALID_HANDLE_VALUE) {
        LogWarning("Serial {} is already initialized. Duplicate initialization is ignored.", port);
        return std::error_code();
    }

    // Config port name.
    if (portName.starts_with("COM") && portName.size() >= 4) {
        if (portName[3] >= '8' || portName.size() > 4)
            port = std::format("\\\\.\\{}", portName);
        else
            port = portName;
    }

    // Create connection
    fileHandle = CreateFileA(port.c_str(),
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             nullptr,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                             nullptr);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();
        LogError("Failed to connect to serial port {}: {}.", port, errorCode);
        return std::error_code(errorCode, std::system_category());
    }

    // Setup a 4k buffer.
    SetupComm(fileHandle, 4096, 4096);

    // Configure port.
    DCB dcb;
    GetCommState(fileHandle, &dcb);
    dcb.BaudRate      = baudRate;
    dcb.ByteSize      = 8;
    dcb.Parity        = NOPARITY;
    dcb.fParity       = 0;
    dcb.StopBits      = ONESTOPBIT;
    dcb.fBinary       = 1;
    dcb.fRtsControl   = RTS_CONTROL_ENABLE;
    dcb.fOutxCtsFlow  = 0;
    dcb.fDtrControl   = DTR_CONTROL_ENABLE;
    dcb.fOutxDsrFlow  = 0;
    dcb.fOutX         = 0;
    dcb.fInX          = 0;
    dcb.fNull         = 0;
    dcb.fErrorChar    = 0;
    dcb.fAbortOnError = 0;
    dcb.XonChar       = '\x11';
    dcb.XoffChar      = '\x13';

    if (!SetCommState(fileHandle, &dcb)) {
        DWORD errorCode = GetLastError();
        LogError("Failed to set comm state for serial {}: {}.", port, errorCode);

        // Close original handle.
        CloseHandle(fileHandle);
        fileHandle = INVALID_HANDLE_VALUE;

        return std::error_code(errorCode, std::system_category());
    }

    // Clear buffer.
    PurgeComm(fileHandle, PURGE_TXCLEAR | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_RXABORT);

    // Create events for overlapped IO.
    overlappedRead.hEvent  = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    overlappedWrite.hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    return std::error_code();
}

auto Serial::OnRegister() noexcept -> void {
    AsyncRead();
}

auto Serial::OnIOComplete(DWORD bytesTransferred, OVERLAPPED *overlapped) noexcept -> void {
    if (overlapped == &overlappedRead) {
        if (bytesTransferred != bytesRead)
            LogInfo(
                "IO complete port transferred bytes {} is different from overlapped IO bytes {}.",
                bytesTransferred,
                bytesRead);

        if (bytesRead != 0)
            this->OnRead(readBuffer, bytesRead);

        std::error_code errorCode = AsyncRead();
        if (errorCode.value() != 0)
            LogError(
                "Failed to start async read task for serial {}: {}.", port, errorCode.message());
    } else if (overlapped == &overlappedWrite) {
        OnWriteComplete();

        std::lock_guard<std::mutex> lock(writeMutex);
        dataToWrite.pop();

        if (!dataToWrite.empty()) {
            std::error_code errorCode = WriteNextBuffer();
            if (errorCode.value() != 0) {
                LogError(
                    "Failed to write next buffer to serial {}: {}.", port, errorCode.message());
            }
        }
    } else {
        LogWarning("{}: Unknown overlapped object received. Ignored.", port);
    }
}

auto Serial::GetHandle() const noexcept -> HANDLE {
    return fileHandle;
}

auto Serial::AsyncWrite(const void *data, size_t size) noexcept -> void {
    std::lock_guard<std::mutex> lock(writeMutex);
    dataToWrite.emplace(static_cast<const std::byte *>(data),
                        static_cast<const std::byte *>(data) + size);

    if (dataToWrite.size() == 1) {
        std::error_code errorCode = WriteNextBuffer();
        if (errorCode.value() != 0)
            LogError("Failed to write next buffer to serial {}: {}.", port, errorCode.message());
    }
}

auto Serial::OnRead(const void *data, size_t size) noexcept -> void {
    (void)data;
    (void)size;
}

auto Serial::OnWriteComplete() noexcept -> void {}

auto Serial::AsyncRead() noexcept -> std::error_code {
    DWORD   errorCode;
    COMSTAT comStat;

    ResetEvent(overlappedRead.hEvent);
    if (!ClearCommError(fileHandle, &errorCode, &comStat)) {
        errorCode = GetLastError();
        LogError("Failed to clear comm error for serial {}: {}.", port, errorCode);
        return std::error_code(errorCode, std::system_category());
    }

    const DWORD readSize = comStat.cbInQue < static_cast<DWORD>(sizeof(readBuffer))
                               ? comStat.cbInQue
                               : static_cast<DWORD>(sizeof(readBuffer));

    if (!ReadFile(fileHandle, readBuffer, readSize, &bytesRead, &overlappedRead)) {
        errorCode = GetLastError();
        LogError("Failed to start overlapped read task for serial {}: {}.", port, errorCode);
        return std::error_code(errorCode, std::system_category());
    }

    return std::error_code();
}

auto Serial::WriteNextBuffer() noexcept -> std::error_code {
    DWORD       errorCode;
    const void *data     = dataToWrite.front().data();
    DWORD       dataSize = static_cast<DWORD>(dataToWrite.front().size());

    if (!WriteFile(fileHandle, data, dataSize, nullptr, &overlappedWrite)) {
        errorCode = GetLastError();
        if (errorCode != ERROR_IO_PENDING) {
            LogError("Failed to write data to serial {}: {}.", port, errorCode);
            return std::error_code(errorCode, std::system_category());
        }
    }

    return std::error_code();
}
