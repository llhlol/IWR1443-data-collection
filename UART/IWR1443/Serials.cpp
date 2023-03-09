#include "Serials.h"
#include "Data.h"

#include <Windows.h>

using namespace iwr1443;

iwr1443::ControlSerial::ControlSerial() noexcept : Serial() {}

iwr1443::ControlSerial::~ControlSerial() noexcept {}

auto iwr1443::ControlSerial::Initialize() noexcept -> std::error_code {
    return Serial::Initialize("COM4", 115200);
}

auto iwr1443::ControlSerial::OnRead(const void *data, size_t size) noexcept -> void {
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), data, static_cast<DWORD>(size), nullptr, nullptr);
}

iwr1443::DataSerial::DataSerial() noexcept : Serial(), buffer() {}

iwr1443::DataSerial::~DataSerial() noexcept {}

auto iwr1443::DataSerial::Initialize() noexcept -> std::error_code {
    return Serial::Initialize("COM3", 921600);
}

static auto LocateFrameHeader(const void *start, size_t size) noexcept -> const void * {
    const std::byte *searchStart = static_cast<const std::byte *>(start);
    const std::byte *searchEnd   = searchStart + size - sizeof(FrameHeader) + 1;

    constexpr const uint16_t magic[4] = {0x0102, 0x0304, 0x0506, 0x0708};

    while (searchStart != searchEnd) {
        if (memcmp(searchStart, magic, sizeof(magic)) == 0)
            return searchStart;

        ++searchStart;
    }

    return nullptr;
}

auto iwr1443::DataSerial::OnRead(const void *data, size_t size) noexcept -> void {
    buffer.insert(buffer.end(),
                  static_cast<const std::byte *>(data),
                  static_cast<const std::byte *>(data) + size);

    if (buffer.size() < sizeof(FrameHeader))
        return;

    const void *frameStart = LocateFrameHeader(buffer.data(), buffer.size());
    if (frameStart == nullptr) {
        buffer.clear();
        return;
    }

    // Move frame to start of the buffer.
    if (frameStart != buffer.data()) {
        size_t eraseSize = static_cast<const std::byte *>(frameStart) - buffer.data();
        // frameStart may be invalidated here.
        buffer.erase(buffer.begin(), buffer.begin() + eraseSize);
    }

    // Wait for the whole frame.
    FrameHeader *frameHeader = reinterpret_cast<FrameHeader *>(buffer.data());
    if (buffer.size() < size_t(frameHeader->packetLength))
        return;

    HandleFrame(frameHeader);
    buffer.clear();
}

auto iwr1443::DataSerial::SetPersistantWriter(
    std::function<void(const void *, size_t)> writer) noexcept -> void {
    persistantWriter = std::move(writer);
}

auto iwr1443::DataSerial::Persistant(const void *data, size_t size) noexcept -> void {
    if (persistantWriter)
        persistantWriter(data, size);
    else
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), data, DWORD(size), nullptr, nullptr);
}

static auto HandleTLV(std::string &ctx, const void *tlv) noexcept -> size_t {
    const TLVHeader *tlvHeader = static_cast<const TLVHeader *>(tlv);
    const void      *data      = static_cast<const std::byte *>(tlv) + sizeof(TLVHeader);

    std::format_to(std::back_inserter(ctx), "{{\"Type\": \"{}\", ", tlvHeader->type);
    switch (tlvHeader->type) {
    case TLVType::DetectedPoints: {
        const size_t         pointCount = tlvHeader->length / sizeof(DetectedPoint);
        const DetectedPoint *points     = static_cast<const DetectedPoint *>(data);

        std::format_to(std::back_inserter(ctx), "\"Data\": [");
        for (size_t i = 0; i < pointCount; ++i) {
            if (i != 0)
                std::format_to(std::back_inserter(ctx), ", ");
            std::format_to(std::back_inserter(ctx), "{}", points[i]);
        }
        std::format_to(std::back_inserter(ctx), "]");
        break;
    }

    case TLVType::RangeProfile: {
        const size_t  valueCount = tlvHeader->length / sizeof(Q9Real);
        const Q9Real *values     = static_cast<const Q9Real *>(data);

        std::format_to(std::back_inserter(ctx), "\"Data\": [");
        for (size_t i = 0; i < valueCount; ++i) {
            if (i != 0)
                std::format_to(std::back_inserter(ctx), ", ");
            std::format_to(std::back_inserter(ctx), "{}", values[i]);
        }
        std::format_to(std::back_inserter(ctx), "]");
        break;
    }

    case TLVType::Statistics: {
        const Statistics *value = static_cast<const Statistics *>(data);
        std::format_to(std::back_inserter(ctx), "\"Data\": {}", *value);
        break;
    }

    case TLVType::DetectedPointsSideInfo: {
        const DetectedPointSideInfo *values     = static_cast<const DetectedPointSideInfo *>(data);
        const size_t                 valueCount = tlvHeader->length / sizeof(DetectedPointSideInfo);

        std::format_to(std::back_inserter(ctx), "\"Data\": [");
        for (size_t i = 0; i < valueCount; ++i) {
            if (i != 0)
                std::format_to(std::back_inserter(ctx), ", ");
            std::format_to(std::back_inserter(ctx), "{}", values[i]);
        }

        std::format_to(std::back_inserter(ctx), "]");
        break;
    }

    case TLVType::TemperatureStatistics: {
        const auto *statistics = static_cast<const TemperatureStatistics *>(data);
        std::format_to(std::back_inserter(ctx), "\"Data\": {}", *statistics);
        break;
    }

    case TLVType::SphericalCoordinates: {
        const auto  *values     = static_cast<const SphericalCoordinate *>(data);
        const size_t valueCount = tlvHeader->length / sizeof(SphericalCoordinate);

        std::format_to(std::back_inserter(ctx), "\"Data\": [");
        for (size_t i = 0; i < valueCount; ++i) {
            if (i != 0)
                std::format_to(std::back_inserter(ctx), ", ");
            std::format_to(std::back_inserter(ctx), "{}", values[i]);
        }

        std::format_to(std::back_inserter(ctx), "]");
        break;
    }

    case TLVType::TargetList: {
        const auto  *values     = static_cast<const Tracked3DTarget *>(data);
        const size_t valueCount = tlvHeader->length / sizeof(Tracked3DTarget);

        std::format_to(std::back_inserter(ctx), "\"Data\": [");
        for (size_t i = 0; i < valueCount; ++i) {
            if (i != 0)
                std::format_to(std::back_inserter(ctx), ", ");
            std::format_to(std::back_inserter(ctx), "{}", values[i]);
        }

        std::format_to(std::back_inserter(ctx), "]");
        break;
    }

    case TLVType::TargetIndex: {
        const auto  *values     = static_cast<const uint8_t *>(data);
        const size_t valueCount = tlvHeader->length;

        std::format_to(std::back_inserter(ctx), "\"Data\": [");
        for (size_t i = 0; i < valueCount; ++i) {
            if (i != 0)
                std::format_to(std::back_inserter(ctx), ", ");
            std::format_to(std::back_inserter(ctx), "{}", values[i]);
        }

        std::format_to(std::back_inserter(ctx), "]");
        break;
    }

    case TLVType::SphericalCompressedPointCloud: {
        std::format_to(std::back_inserter(ctx), "\"Data\": {{");
        const auto *header = static_cast<const SphericalCompressedPointCloudHeader *>(data);
        std::format_to(std::back_inserter(ctx), "\"Header\": {}, ", *header);

        const auto *points = reinterpret_cast<const SphericalCompressedPoint *>(
            static_cast<const std::byte *>(data) + sizeof(SphericalCompressedPointCloudHeader));

        const size_t pointCount =
            (tlvHeader->length - sizeof(SphericalCompressedPointCloudHeader)) /
            sizeof(SphericalCompressedPoint);

        std::format_to(std::back_inserter(ctx), "\"Points\": [");
        for (size_t i = 0; i < pointCount; ++i) {
            if (i != 0)
                std::format_to(std::back_inserter(ctx), ", ");
            std::format_to(std::back_inserter(ctx), "{}", points[i]);
        }

        std::format_to(std::back_inserter(ctx), "]}}");
        break;
    }

    default:
        break;
    }

    std::format_to(std::back_inserter(ctx), "}}");
    return tlvHeader->length + sizeof(TLVHeader);
}

auto iwr1443::DataSerial::HandleFrame(const void *frame) noexcept -> void {
    const FrameHeader *frameHeader = static_cast<const FrameHeader *>(frame);

    std::string ctx;
    std::format_to(std::back_inserter(ctx), "{{\"Header\": {}, \"TLVs\": [", *frameHeader);

    const std::byte *iter = static_cast<const std::byte *>(frame);
    iter += sizeof(FrameHeader);

    for (uint32_t i = 0; i < frameHeader->tlvCount; ++i) {
        if (i != 0)
            std::format_to(std::back_inserter(ctx), ", ");
        iter += HandleTLV(ctx, iter);
    }

    std::format_to(std::back_inserter(ctx), "]}}, ");
    Persistant(ctx.data(), ctx.size());
}
