#pragma once

#include <format>
#include <memory>
#include <mutex>
#include <vector>

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Off,
};

class LogWriter {
public:
    /// @brief
    ///   Virtual dtor.
    virtual ~LogWriter() = default;

    /// @brief
    ///   Persistent log system data. Multiple threads may persistent data concurrently.
    ///
    /// @param[in] data     Pointer to start of data to be written.
    /// @param     size     Size in byte of data to be written.
    virtual auto Write(const void *data, size_t size) -> void = 0;
};

class LogSystem {
public:
    /// @brief
    ///   Create a log system and write message to stderr.
    ///
    /// @param level    Log filter level.
    LogSystem(LogLevel level = LogLevel::Info) noexcept;

    /// @brief
    ///   Destroy this log system.
    ~LogSystem();

    /// @brief
    ///   Write a new log message.
    ///
    /// @param severity     Severity of this log message. Messages with lower severity than log
    ///                     filters will be ignored.
    /// @param message      The log message to be written.
    auto LogMessage(LogLevel severity, std::string_view message) -> void;

    /// @brief
    ///   Write a trace log message.
    ///
    /// @param message      The log message to be written.
    auto Trace(std::string_view message) -> void {
        LogMessage(LogLevel::Trace, message);
    }

    /// @brief
    ///   Write a debug log message.
    ///
    /// @param message      The log message to be written.
    auto Debug(std::string_view message) -> void {
        LogMessage(LogLevel::Debug, message);
    }

    /// @brief
    ///   Write an info log message.
    ///
    /// @param message      The log message to be written.
    auto Info(std::string_view message) -> void {
        LogMessage(LogLevel::Info, message);
    }

    /// @brief
    ///   Write a warning log message.
    ///
    /// @param message      The log message to be written.
    auto Warning(std::string_view message) -> void {
        LogMessage(LogLevel::Warning, message);
    }

    /// @brief
    ///   Write an error log message.
    ///
    /// @param message      The log message to be written.
    auto Error(std::string_view message) -> void {
        LogMessage(LogLevel::Error, message);
    }

    /// @brief
    ///   Format and write a trace log message.
    ///
    /// @tparam Args    Types of arguments to be formatted.
    /// @param  format  The format string specifies how to format the message.
    /// @param  args    Arguments to be formatted.
    template <typename... Args>
    auto Trace(std::format_string<Args...> format, Args &&...args) -> void {
        if (filterLevel > LogLevel::Trace)
            return;

        std::string message(std::format(format, std::forward<Args>(args)...));
        LogMessage(LogLevel::Trace, message);
    }

    /// @brief
    ///   Format and write a debug log message.
    ///
    /// @tparam Args    Types of arguments to be formatted.
    /// @param  format  The format string specifies how to format the message.
    /// @param  args    Arguments to be formatted.
    template <typename... Args>
    auto Debug(std::format_string<Args...> format, Args &&...args) -> void {
        if (filterLevel > LogLevel::Debug)
            return;

        std::string message(std::format(format, std::forward<Args>(args)...));
        LogMessage(LogLevel::Debug, message);
    }

    /// @brief
    ///   Format and write an info log message.
    ///
    /// @tparam Args    Types of arguments to be formatted.
    /// @param  format  The format string specifies how to format the message.
    /// @param  args    Arguments to be formatted.
    template <typename... Args>
    auto Info(std::format_string<Args...> format, Args &&...args) -> void {
        if (filterLevel > LogLevel::Info)
            return;

        std::string message(std::format(format, std::forward<Args>(args)...));
        LogMessage(LogLevel::Info, message);
    }

    /// @brief
    ///   Format and write a warning log message.
    ///
    /// @tparam Args    Types of arguments to be formatted.
    /// @param  format  The format string specifies how to format the message.
    /// @param  args    Arguments to be formatted.
    template <typename... Args>
    auto Warning(std::format_string<Args...> format, Args &&...args) -> void {
        if (filterLevel > LogLevel::Warning)
            return;

        std::string message(std::format(format, std::forward<Args>(args)...));
        LogMessage(LogLevel::Warning, message);
    }

    /// @brief
    ///   Format and write an error log message.
    ///
    /// @tparam Args    Types of arguments to be formatted.
    /// @param  format  The format string specifies how to format the message.
    /// @param  args    Arguments to be formatted.
    template <typename... Args>
    auto Error(std::format_string<Args...> format, Args &&...args) -> void {
        if (filterLevel > LogLevel::Error)
            return;

        std::string message(std::format(format, std::forward<Args>(args)...));
        LogMessage(LogLevel::Error, message);
    }

    /// @brief
    ///   Consistant all data in buffer.
    auto Flush() -> void;

    /// @brief
    ///   Get log level of message filter.
    ///
    /// @return LogLevel
    ///   Return current message filter level.
    auto GetLevel() const noexcept -> LogLevel {
        return filterLevel;
    }

    /// @brief
    ///   Set log level of message filter.
    ///
    /// @param level    New log filter level.
    auto SetLevel(LogLevel level) noexcept -> void {
        filterLevel = level;
    }

    /// @brief
    ///   Create and set a new writer for data consistance.
    ///
    /// @tparam T       Type of persistant writer.
    /// @tparam Args    Type of arguments to create the writer.
    /// @param  args    Arguments to create the writer.
    ///
    /// @return T *
    ///   Return pointer to the new writer.
    template <typename T,
              typename... Args,
              typename = std::enable_if_t<std::is_constructible<T, Args...>::value &&
                                          std::is_base_of<LogWriter, T>::value>>
    auto
    SetPersistantWriter(Args &&...args) noexcept(std::is_nothrow_constructible<T, Args...>::value)
        -> T * {
        writer = std::make_unique<T>(std::forward<Args>(args)...);
        return static_cast<T *>(writer.get());
    }

    /// @brief
    ///   Get log system singleton instance. This instance is created when this function is called
    ///   for the first time.
    ///
    /// @return LogSystem *
    ///   Return pointer to the log system instance.
    static auto GetSingleton() noexcept -> LogSystem *;

private:
    /// @brief
    ///   Log filter level. Messages with lower level are ignored.
    LogLevel filterLevel;

    /// @brief
    ///   Current buffer that is used to cache log messages.
    std::vector<char> buffer;

    /// @brief
    ///   Mutex that is used to protect @p buffer from data race.
    std::mutex bufferMutex;

    /// @brief
    ///   Log writer that is used to persistant data.
    std::unique_ptr<LogWriter> writer;
};

inline auto LogTrace(std::string_view message) -> void {
    LogSystem::GetSingleton()->Trace(message);
}

template <typename... Args>
inline auto LogTrace(std::format_string<Args...> format, Args &&...args) -> void {
    LogSystem::GetSingleton()->Trace(format, std::forward<Args>(args)...);
}

inline auto LogDebug(std::string_view message) -> void {
    LogSystem::GetSingleton()->Debug(message);
}

template <typename... Args>
inline auto LogDebug(std::format_string<Args...> format, Args &&...args) -> void {
    LogSystem::GetSingleton()->Debug(format, std::forward<Args>(args)...);
}

inline auto LogInfo(std::string_view message) -> void {
    LogSystem::GetSingleton()->Info(message);
}

template <typename... Args>
inline auto LogInfo(std::format_string<Args...> format, Args &&...args) -> void {
    LogSystem::GetSingleton()->Info(format, std::forward<Args>(args)...);
}

inline auto LogWarning(std::string_view message) -> void {
    LogSystem::GetSingleton()->Warning(message);
}

template <typename... Args>
inline auto LogWarning(std::format_string<Args...> format, Args &&...args) -> void {
    LogSystem::GetSingleton()->Warning(format, std::forward<Args>(args)...);
}

inline auto LogError(std::string_view message) -> void {
    LogSystem::GetSingleton()->Error(message);
}

template <typename... Args>
inline auto LogError(std::format_string<Args...> format, Args &&...args) -> void {
    LogSystem::GetSingleton()->Error(format, std::forward<Args>(args)...);
}
