#pragma once

#include "IAsync.h"

#include <system_error>

class IOContext {
public:
    /// @brief
    ///   Create an empty IOContext. Initialize this IOContext before using.
    IOContext() noexcept;

    /// @brief
    ///   Destroy this IOContext.
    ~IOContext() noexcept;

    /// @brief
    ///   Initialize this IOContext.
    ///
    /// @return std::error_code
    ///   Return an error code that represents the initialize result.
    auto Initialize() noexcept -> std::error_code;

    /// @brief
    ///   Register a connection to this IOContext.
    ///
    /// @param[in] connection   The connection to be registered.
    auto Register(IAsync *connection) noexcept -> std::error_code;

    /// @brief
    ///   Start looping over all connections.
    auto Run() noexcept -> std::error_code;

    /// @brief
    ///   Stop looping of this IOContext.
    auto Quit() noexcept -> void;

private:
    /// @brief
    ///   IO complete port handle.
    HANDLE ioCompletePort;
};
