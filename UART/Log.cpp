#include "Log.h"

#include <Windows.h>

#include <chrono>

static constexpr const size_t BUFFER_SIZE = 4096;
static constexpr const size_t FLUSH_SIZE  = BUFFER_SIZE - 256;

static auto LogLevelName(LogLevel level) noexcept -> std::string_view {
    switch (level) {
    case LogLevel::Trace:
        return "Trace";
    case LogLevel::Debug:
        return "Debug";
    case LogLevel::Info:
        return "Info";
    case LogLevel::Warning:
        return "Warning";
    case LogLevel::Error:
        return "Error";
    default:
        return "WTF";
    }
}

LogSystem::LogSystem(LogLevel level) noexcept
    : filterLevel(level), buffer(), bufferMutex(), writer() {
    buffer.reserve(BUFFER_SIZE);
}

LogSystem::~LogSystem() {
    if (writer != nullptr)
        writer->Write(buffer.data(), buffer.size());
    else
        WriteFile(
            GetStdHandle(STD_ERROR_HANDLE), buffer.data(), DWORD(buffer.size()), nullptr, nullptr);

    buffer.clear();
}

auto LogSystem::LogMessage(LogLevel severity, std::string_view message) -> void {
    if (severity < filterLevel)
        return;

    static thread_local const DWORD threadID = GetCurrentThreadId();

    const auto        now = std::chrono::system_clock::now();
    std::vector<char> flushBuffer;

    { // Write message to buffer.
        std::lock_guard<std::mutex> lock(bufferMutex);
        std::format_to(std::back_inserter(buffer),
                       "{} {} [{}] {}\n",
                       threadID,
                       now,
                       LogLevelName(severity),
                       message);

        if (buffer.size() >= FLUSH_SIZE || severity >= LogLevel::Error) {
            flushBuffer.reserve(BUFFER_SIZE);
            flushBuffer.swap(buffer);
        }
    }

    if (!flushBuffer.empty()) {
        if (writer != nullptr)
            writer->Write(flushBuffer.data(), flushBuffer.size());
        else
            WriteFile(GetStdHandle(STD_ERROR_HANDLE),
                      flushBuffer.data(),
                      DWORD(flushBuffer.size()),
                      nullptr,
                      nullptr);
    }
}

auto LogSystem::Flush() -> void {
    std::vector<char> flushBuffer;
    flushBuffer.reserve(BUFFER_SIZE);

    { // Swap buffers to avoid locking for too long.
        std::lock_guard<std::mutex> lock(bufferMutex);
        buffer.swap(flushBuffer);
    }

    if (writer != nullptr)
        writer->Write(flushBuffer.data(), flushBuffer.size());
    else
        WriteFile(GetStdHandle(STD_ERROR_HANDLE),
                  flushBuffer.data(),
                  DWORD(flushBuffer.size()),
                  nullptr,
                  nullptr);
}

auto LogSystem::GetSingleton() noexcept -> LogSystem * {
    static LogSystem instance;
    return &instance;
}
