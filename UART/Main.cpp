#include "IOContext.h"
#include "Log.h"
#include "Serial.h"

#include <iostream>
#include <string>

class ControlSerial final : public Serial {
public:
    using Serial::Serial;

    auto OnRead(const void *data, size_t size) noexcept -> void override {
        WriteFile(
            GetStdHandle(STD_OUTPUT_HANDLE), data, static_cast<DWORD>(size), nullptr, nullptr);
    }
};

class DataSerial final : public Serial {
public:
    using Serial::Serial;

    auto OnRead(const void *data, size_t size) noexcept -> void override {
        (void)data;
        std::string msg = std::format("Received {} bytes of data from data serial.\n", size);
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),
                  msg.data(),
                  static_cast<DWORD>(msg.size()),
                  nullptr,
                  nullptr);
    }
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
    errorCode = controlSerial.Initialize("COM4", 115200);
    if (errorCode.value() != 0) {
        LogError("Failed to initialize control serial: {}.", errorCode.message());
        return EXIT_FAILURE;
    }

    DataSerial dataSerial;
    errorCode = dataSerial.Initialize("COM3", 921600);
    if (errorCode.value() != 0) {
        LogError("Failed to initialize data serial: {}.", errorCode.message());
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
