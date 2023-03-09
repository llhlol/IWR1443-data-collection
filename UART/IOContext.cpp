#include "IOContext.h"
#include "Log.h"

#include <limits>

static constexpr const ULONG_PTR QUIT_HANDLE = std::numeric_limits<ULONG_PTR>::max();

IOContext::IOContext() noexcept : ioCompletePort(nullptr) {}

IOContext::~IOContext() noexcept {
    if (ioCompletePort != nullptr) {
        CloseHandle(ioCompletePort);
        ioCompletePort = nullptr;
    }
}

auto IOContext::Initialize() noexcept -> std::error_code {
    if (ioCompletePort != nullptr) {
        LogWarning("IO complete port already initialized. Duplicate initialization is ignored.");
        return std::error_code();
    }

    ioCompletePort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (ioCompletePort == nullptr) {
        DWORD errorCode = GetLastError();
        LogError("Failed to create IO complete port for IOContext: {}.", errorCode);
        return std::error_code(errorCode, std::system_category());
    }

    return std::error_code();
}

auto IOContext::Register(IAsync *connection) noexcept -> std::error_code {
    HANDLE handle = connection->GetHandle();

    HANDLE result =
        CreateIoCompletionPort(handle, ioCompletePort, reinterpret_cast<ULONG_PTR>(connection), 0);
    if (result != ioCompletePort) {
        DWORD errorCode = GetLastError();
        LogError("Failed to register connection to IO complete port: {}.", errorCode);
        return std::error_code(errorCode, std::system_category());
    }

    connection->OnRegister();
    return std::error_code();
}

auto IOContext::Run() noexcept -> std::error_code {
    DWORD       bytesTransferred;
    ULONG_PTR   completeKey;
    OVERLAPPED *overlapped;

    for (;;) {
        if (!GetQueuedCompletionStatus(
                ioCompletePort, &bytesTransferred, &completeKey, &overlapped, INFINITE)) {
            DWORD errorCode = GetLastError();
            LogError("IOContext failed to get queued completion status: {}.", errorCode);
            return std::error_code(errorCode, std::system_category());
        }

        if (completeKey == QUIT_HANDLE)
            break;

        IAsync *connection = reinterpret_cast<IAsync *>(completeKey);
        connection->OnIOComplete(bytesTransferred, overlapped);
    }

    return std::error_code();
}

auto IOContext::Quit() noexcept -> void {
    PostQueuedCompletionStatus(ioCompletePort, 0, QUIT_HANDLE, nullptr);
}
