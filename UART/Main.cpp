#include "IOContext.h"
#include "IWR1443/Serials.h"
#include "Log.h"

#include <iostream>
#include <string>

using namespace iwr1443;

class FileWriter {
public:
    /// @brief
    ///   Create an null json writer.
    FileWriter() noexcept;

    /// @brief
    ///   Destroy this json writer.
    ~FileWriter() noexcept;

    /// @brief
    ///   Try to open the specified file as output file.
    auto Open(std::string_view path) noexcept -> std::error_code;

    /// @brief
    ///   Try to append data to the end of file.
    auto Write(const void *data, size_t size) noexcept -> std::error_code;

private:
    /// @brief
    ///   Handle of the json file.
    HANDLE fileHandle;
};

auto main() -> int {
    std::error_code errorCode;

    IOContext ioContext;
    errorCode = ioContext.Initialize();
    if (errorCode.value() != 0) {
        LogError("Failed to initialize IO context: {}.", errorCode.message());
        return EXIT_FAILURE;
    }

    ControlSerial controlSerial;
    errorCode = controlSerial.Initialize();
    if (errorCode.value() != 0) {
        LogError("Failed to initialize control serial: {}.", errorCode.message());
        return EXIT_FAILURE;
    }

    DataSerial dataSerial;
    errorCode = dataSerial.Initialize();
    if (errorCode.value() != 0) {
        LogError("Failed to initialize data serial: {}.", errorCode.message());
        return EXIT_FAILURE;
    }

    FileWriter radarDataWriter;
    errorCode = radarDataWriter.Open("data.json");
    if (errorCode.value() != 0) {
        LogError("Failed to open data file {}: {}.", "data.json", errorCode.message());
        return EXIT_FAILURE;
    }

    errorCode = ioContext.Register(&controlSerial);
    if (errorCode.value() != 0) {
        LogError("Failed to register control serial to IO context: {}.", errorCode.message());
        return EXIT_FAILURE;
    }

    errorCode = ioContext.Register(&dataSerial);
    if (errorCode.value() != 0) {
        LogError("Failed to register data serial to IO context: {}.", errorCode.message());
        return EXIT_FAILURE;
    }

    dataSerial.SetPersistantWriter([&radarDataWriter](const void *data, size_t size) -> void {
        radarDataWriter.Write(data, size);
    });

    std::jthread task([&ioContext]() -> void { ioContext.Run(); });

    std::string command;
    for (;;) {
        std::getline(std::cin, command);
        if (command == "exit")
            break;

        command.push_back('\n');
        controlSerial.AsyncWrite(command.data(), command.size());
        command.clear();
    }

    ioContext.Quit();
    task.join();

    return 0;
}

FileWriter::FileWriter() noexcept : fileHandle(INVALID_HANDLE_VALUE) {}

FileWriter::~FileWriter() noexcept {
    if (fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(fileHandle);
        fileHandle = INVALID_HANDLE_VALUE;
    }
}

auto FileWriter::Open(std::string_view path) noexcept -> std::error_code {
    const int count =
        MultiByteToWideChar(CP_UTF8, 0, path.data(), static_cast<int>(path.size()), nullptr, 0);

    if (count <= 0)
        return std::error_code(GetLastError(), std::system_category());

    std::wstring widePath;
    widePath.resize(static_cast<size_t>(count));

    MultiByteToWideChar(
        CP_UTF8, 0, path.data(), static_cast<int>(path.size()), widePath.data(), count);

    HANDLE newFile = CreateFile(widePath.c_str(),
                                GENERIC_WRITE,
                                FILE_SHARE_READ,
                                nullptr,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                nullptr);

    if (newFile == INVALID_HANDLE_VALUE)
        return std::error_code(GetLastError(), std::system_category());

    if (fileHandle != INVALID_HANDLE_VALUE)
        CloseHandle(fileHandle);

    fileHandle = newFile;
    return std::error_code();
}

auto FileWriter::Write(const void *data, size_t size) noexcept -> std::error_code {
    if (!WriteFile(fileHandle, data, static_cast<DWORD>(size), nullptr, nullptr))
        return std::error_code(GetLastError(), std::system_category());
    return std::error_code();
}
