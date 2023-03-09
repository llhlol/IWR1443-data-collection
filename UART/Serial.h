#pragma once

#include "IAsync.h"

#include <mutex>
#include <queue>
#include <system_error>
#include <vector>

class Serial : public IAsync {
public:
    /// @brief
    ///   Create an empty serial.
    Serial() noexcept;

    /// @brief
    ///   Destroy this serial.
    ~Serial() noexcept override;

    /// @brief
    ///   Initialize this serial with the specified config.
    ///
    /// @param portName     Port to be connected.
    /// @param baudRate     Baud rate of this connection.
    ///
    /// @return std::error_code
    ///   Return an error_code that represents the initialization result.
    auto Initialize(std::string_view portName, uint32_t baudRate) noexcept -> std::error_code;

    /// @brief
    ///   This method is called once registered to IOContext.
    auto OnRegister() noexcept -> void final;

    /// @brief
    ///   IO complete callback. This is called when an IO operation is completed.
    auto OnIOComplete(DWORD bytesTransferred, OVERLAPPED *overlapped) noexcept -> void final;

    /// @brief
    ///   Get win32 native handle so that IO complete port could register this object.
    auto GetHandle() const noexcept -> HANDLE final;

    /// @brief
    ///   Pend data to write to this serial.
    auto AsyncWrite(const void *data, size_t size) noexcept -> void;

    /// @brief
    ///   Data receive callback.
    ///
    /// @param[in] data     Pointer to start of data received from serial.
    /// @param     size     Size in byte of data transferred.
    virtual auto OnRead(const void *data, size_t size) noexcept -> void;

    /// @brief
    ///   Write complete callback.
    virtual auto OnWriteComplete() noexcept -> void;

private:
    /// @brief
    ///   Start async read task.
    ///
    /// @return std::error_code
    ///   Return std::error_code that represents the read result.
    auto AsyncRead() noexcept -> std::error_code;

    /// @brief
    ///   Write data in next buffer to serial port.
    ///
    /// @return std::error_code
    ///   Return std::error_code that represents the write result.
    auto WriteNextBuffer() noexcept -> std::error_code;

private:
    /// @brief
    ///   Win32 file handle of this serial.
    HANDLE fileHandle;

    /// @brief
    ///   Port of this serial that is connected to.
    std::string port;

    /// @brief
    ///   Overlapped struct that is used to synchronize with read operations.
    OVERLAPPED overlappedRead;

    /// @brief
    ///   Overlapped struct that is used to synchronize with write operations.
    OVERLAPPED overlappedWrite;

    /// @brief
    ///   Overlapped read buffer.
    std::byte readBuffer[4096];

    /// @brief
    ///   Bytes read from read buffer in the specified read operation.
    DWORD bytesRead;

    /// @brief
    ///   Data to be written to serial.
    std::queue<std::vector<std::byte>> dataToWrite;

    /// @brief
    ///   Mutex that is used to control async write.
    mutable std::mutex writeMutex;
};
