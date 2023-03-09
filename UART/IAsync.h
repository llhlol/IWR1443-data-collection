#pragma once

#include <Windows.h>

class IAsync {
public:
    IAsync() noexcept = default;

    /// @brief
    ///   Copy constructor is disabled.
    IAsync(const IAsync &) = delete;

    /// @brief
    ///   Copy assignment is disabled.
    auto operator=(const IAsync &) = delete;

    /// @brief
    ///   Destroy this object.
    virtual ~IAsync() noexcept = default;

    /// @brief
    ///   This method is called once registered to IOContext.
    virtual auto OnRegister() noexcept -> void = 0;

    /// @brief
    ///   IO complete callback. This is called when an IO operation is completed.
    virtual auto OnIOComplete(DWORD bytesTransferred, OVERLAPPED *overlapped) noexcept -> void = 0;

    /// @brief
    ///   Get win32 native handle so that IO complete port could register this object.
    virtual auto GetHandle() const noexcept -> HANDLE = 0;
};
