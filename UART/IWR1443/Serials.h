#pragma once

#include "../Serial.h"

#include <functional>

namespace iwr1443 {

class ControlSerial final : public Serial {
public:
    /// @brief
    ///   Create an empty control serial. Initialize this serial before using.
    ControlSerial() noexcept;

    /// @brief
    ///   Destroy this control serial.
    ~ControlSerial() noexcept override;

    /// @brief
    ///   Initialize this control serial and connect to IWR1443 CLI port.
    ///
    /// @return std::error_code
    ///   Return an error code that represents the connect result.
    auto Initialize() noexcept -> std::error_code;

    /// @brief
    ///   Data receive callback.
    ///
    /// @param[in] data     Pointer to start of data received from serial.
    /// @param     size     Size in byte of data transferred.
    auto OnRead(const void *data, size_t size) noexcept -> void override;
};

class DataSerial final : public Serial {
public:
    /// @brief
    ///   Create an empty data serial. Initialize this serial before using.
    DataSerial() noexcept;

    /// @brief
    ///   Destroy this data serial.
    ~DataSerial() noexcept override;

    /// @brief
    ///   Initialize this control serial and connect to IWR1443 data port.
    ///
    /// @return std::error_code
    ///   Return an error code that represents the connect result.
    auto Initialize() noexcept -> std::error_code;

    /// @brief
    ///   Data receive callback.
    ///
    /// @param[in] data     Pointer to start of data received from serial.
    /// @param     size     Size in byte of data transferred.
    auto OnRead(const void *data, size_t size) noexcept -> void override;

    /// @brief
    ///   Set data persistant writer for this data serial.
    ///
    /// @param writer   The data persistant writer function. The writer should not throw exception.
    auto SetPersistantWriter(std::function<void(const void *, size_t)> writer) noexcept -> void;

private:
    /// @brief
    ///   Persistant data using writer.
    auto Persistant(const void *data, size_t size) noexcept -> void;

    /// @brief
    ///   Serialize the specified frame.
    auto HandleFrame(const void *frame) noexcept -> void;

private:
    /// @brief
    ///   Data buffer that is used to temporary store received binary data.
    std::vector<std::byte> buffer;

    /// @brief
    ///   Data persistant writer.
    std::function<void(const void *, size_t)> persistantWriter;
};

} // namespace iwr1443
