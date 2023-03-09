#pragma once

#include <format>

namespace iwr1443 {

struct FrameHeader {
    uint16_t magic[4];
    uint32_t version;
    uint32_t packetLength;
    uint32_t platform;
    uint32_t frameNumber;
    uint32_t time;
    uint32_t detectedObjectCount;
    uint32_t tlvCount;
    //uint32_t subframeNumber;
};

enum class TLVType : uint32_t {
    DetectedPoints                = 1,
    RangeProfile                  = 2,
    NoiseFloorProfile             = 3,
    AzimuthStaticHeatmap          = 4,
    RangeDopplerHeatmap           = 5,
    Statistics                    = 6,
    DetectedPointsSideInfo        = 7,
    AzimuthElevationStaticHeatmap = 8,
    TemperatureStatistics         = 9,
    SphericalCoordinates          = 1000,
    TargetList                    = 1010,
    TargetIndex                   = 1011,
    SphericalCompressedPointCloud = 1020,
    PresenceDetection             = 1021,
    OccupancyStateMachineOutput   = 1030,
};

struct TLVHeader {
    TLVType  type;
    uint32_t length;
};

struct DetectedPoint {
    float x;
    float y;
    float z;
    float doppler;
};

struct Q9Real {
    uint16_t sign     : 1;
    uint16_t integer  : 9;
    uint16_t fraction : 5;
};

struct Statistics {
    uint32_t interFrameProcessingTime;
    uint32_t transmitOutputTime;
    uint32_t interFrameProcessingMargin;
    uint32_t interChirpProcessingMargin;
    uint32_t activeFrameCPULoad;
    uint32_t interFrameCPULoad;
};

struct DetectedPointSideInfo {
    uint16_t snr;
    uint16_t noise;
};

struct TemperatureStatistics {
    uint32_t tempReportValid;
    uint32_t time;
    uint16_t tmpRx0Sens;
    uint16_t tmpRx1Sens;
    uint16_t tmpRx2Sens;
    uint16_t tmpRx3Sens;
    uint16_t tmpTx0Sens;
    uint16_t tmpTx1Sens;
    uint16_t tmpTx2Sens;
    uint16_t tmpPmSens;
    uint16_t tmpDig0Sens;
    uint16_t tmpDig1Sens;
};

struct SphericalCoordinate {
    float range;     /* meter */
    float azimuth;   /* rad */
    float elevation; /* rad */
    float doppler;   /* meter/second */
};

struct Tracked3DTarget {
    float trackID;

    struct {
        float x;
        float y;
        float z;
    } position;

    struct {
        float x;
        float y;
        float z;
    } velocity;

    struct {
        float x;
        float y;
        float z;
    } acceleration;

    float errorCovariance[3][3];
    float gatingFunctionGain;
    float confidenceLevel;
};

struct SphericalCompressedPointCloudHeader {
    float elevationUnit;
    float azimuthUnit;
    float dopplerUnit;
    float rangeUnit;
    float snrUnit;
};

struct SphericalCompressedPoint {
    int8_t   elevation;
    int8_t   azimuth;
    int16_t  doppler;
    uint16_t range;
    uint16_t snr;
};

} // namespace iwr1443

template <>
struct std::formatter<iwr1443::FrameHeader> : std::formatter<uint32_t> {
    template <typename FormatContext>
    auto format(const iwr1443::FrameHeader &value, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        return std::format_to(
            ctx.out(),
            R"({{"version": {}, "packetLength": {}, "platform": {}, "frameNumber": {}, "time": {}, "detectedObjectCount": {}, "tlvCount": {}}})",
            value.version,
            value.packetLength,
            value.platform,
            value.frameNumber,
            value.time,
            value.detectedObjectCount,
            value.tlvCount);
    }
};

template <>
struct std::formatter<iwr1443::TLVType> : std::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const iwr1443::TLVType value, FormatContext &ctx) const -> decltype(ctx.out()) {
        switch (value) {
        case iwr1443::TLVType::DetectedPoints:
            return std::format_to(ctx.out(), "DetectedPoints");
        case iwr1443::TLVType::RangeProfile:
            return std::format_to(ctx.out(), "RangeProfile");
        case iwr1443::TLVType::NoiseFloorProfile:
            return std::format_to(ctx.out(), "NoiseFloorProfile");
        case iwr1443::TLVType::AzimuthStaticHeatmap:
            return std::format_to(ctx.out(), "AzimuthStaticHeatmap");
        case iwr1443::TLVType::RangeDopplerHeatmap:
            return std::format_to(ctx.out(), "RangeDopplerHeatmap");
        case iwr1443::TLVType::Statistics:
            return std::format_to(ctx.out(), "Statistics");
        case iwr1443::TLVType::DetectedPointsSideInfo:
            return std::format_to(ctx.out(), "DetectedPointsSideInfo");
        case iwr1443::TLVType::AzimuthElevationStaticHeatmap:
            return std::format_to(ctx.out(), "AzimuthElevationStaticHeatmap");
        case iwr1443::TLVType::TemperatureStatistics:
            return std::format_to(ctx.out(), "TemperatureStatistics");
        case iwr1443::TLVType::SphericalCoordinates:
            return std::format_to(ctx.out(), "SphericalCoordinates");
        case iwr1443::TLVType::TargetList:
            return std::format_to(ctx.out(), "TargetList");
        case iwr1443::TLVType::TargetIndex:
            return std::format_to(ctx.out(), "TargetIndex");
        case iwr1443::TLVType::SphericalCompressedPointCloud:
            return std::format_to(ctx.out(), "SphericalCompressedPointCloud");
        case iwr1443::TLVType::PresenceDetection:
            return std::format_to(ctx.out(), "PresenceDetection");
        case iwr1443::TLVType::OccupancyStateMachineOutput:
            return std::format_to(ctx.out(), "OccupancyStateMachineOutput");
        default:
            return std::format_to(ctx.out(), "Unknown");
        }
    }
};

template <>
struct std::formatter<iwr1443::DetectedPoint> : std::formatter<float> {
    template <typename FormatContext>
    auto format(const iwr1443::DetectedPoint &value, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        return std::format_to(ctx.out(),
                              R"({{"x": {}, "y": {}, "z": {}, "doppler": {}}})",
                              value.x,
                              value.y,
                              value.z,
                              value.doppler);
    }
};

template <>
struct std::formatter<iwr1443::Q9Real> : std::formatter<uint16_t> {
    template <typename FormatContext>
    auto format(const iwr1443::Q9Real &value, FormatContext &ctx) const -> decltype(ctx.out()) {
        if (value.sign)
            return std::format_to(ctx.out(), "-{}.{}", value.integer, value.fraction);
        else
            return std::format_to(ctx.out(), "{}.{}", value.integer, value.fraction);
    }
};

template <>
struct std::formatter<iwr1443::Statistics> : std::formatter<uint32_t> {
    template <typename FormatContext>
    auto format(const iwr1443::Statistics &value, FormatContext &ctx) const -> decltype(ctx.out()) {
        return std::format_to(
            ctx.out(),
            R"({{"interFrameProcessingTime": {}, "transmitOutputTime": {}, "interFrameProcessingMargin": {}, "interChirpProcessingMargin": {}, "activeFrameCPULoad": {}, "interFrameCPULoad": {}}})",
            value.interFrameProcessingTime,
            value.transmitOutputTime,
            value.interFrameProcessingMargin,
            value.interChirpProcessingMargin,
            value.activeFrameCPULoad,
            value.interFrameCPULoad);
    }
};

template <>
struct std::formatter<iwr1443::DetectedPointSideInfo> : std::formatter<uint16_t> {
    template <typename FormatContext>
    auto format(const iwr1443::DetectedPointSideInfo &value, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), R"({{"snr": {}, "noise": {}}})", value.snr, value.noise);
    }
};

template <>
struct std::formatter<iwr1443::TemperatureStatistics> : std::formatter<uint32_t> {
    template <typename FormatContext>
    auto format(const iwr1443::TemperatureStatistics &value, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        return std::format_to(ctx.out(),
                              "{{\"tempReportValid\": {}, "
                              "\"time\": {}, "
                              "\"tmpRx0Sens\": {}, "
                              "\"tmpRx1Sens\": {}, "
                              "\"tmpRx2Sens\": {}, "
                              "\"tmpRx3Sens\": {}, "
                              "\"tmpTx0Sens\": {}, "
                              "\"tmpTx1Sens\": {}, "
                              "\"tmpTx2Sens\": {}, "
                              "\"tmpPmSens\": {}, "
                              "\"tmpDig0Sens\": {}, "
                              "\"tmpDig1Sens\": {}}}",
                              value.tempReportValid,
                              value.time,
                              value.tmpRx0Sens,
                              value.tmpRx1Sens,
                              value.tmpRx2Sens,
                              value.tmpRx3Sens,
                              value.tmpTx0Sens,
                              value.tmpTx1Sens,
                              value.tmpTx2Sens,
                              value.tmpPmSens,
                              value.tmpDig0Sens,
                              value.tmpDig1Sens);
    }
};

template <>
struct std::formatter<iwr1443::SphericalCoordinate> : std::formatter<float> {
    template <typename FormatContext>
    auto format(const iwr1443::SphericalCoordinate &value, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        return std::format_to(ctx.out(),
                              R"({{"range": {}, "azimuth": {}, "elevation": {}, "doppler": {}}})",
                              value.range,
                              value.azimuth,
                              value.elevation,
                              value.doppler);
    }
};

template <>
struct std::formatter<iwr1443::Tracked3DTarget> : std::formatter<float> {
    template <typename FormatContext>
    auto format(const iwr1443::Tracked3DTarget &value, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        return std::format_to(ctx.out(),
                              "{{\"trackID\": {}, "
                              "\"position\": {{"
                              "\"x\": {}, "
                              "\"y\": {}, "
                              "\"z\": {}}}, "
                              "\"velocity\": {{"
                              "\"x\": {}, "
                              "\"y\": {}, "
                              "\"z\": {}}}, "
                              "\"acceleration\": {{"
                              "\"x\": {}, "
                              "\"y\": {}, "
                              "\"z\": {}}}, "
                              "\"errorCovariance\": [[{}, {}, {}], [{}, {}, {}], [{}, {}, {}]], "
                              "\"gatingFunctionGain\": {}, "
                              "\"confidenceLevel\": {}}}",
                              value.trackID,
                              value.position.x,
                              value.position.y,
                              value.position.z,
                              value.velocity.x,
                              value.velocity.y,
                              value.velocity.z,
                              value.acceleration.x,
                              value.acceleration.y,
                              value.acceleration.z,
                              value.errorCovariance[0][0],
                              value.errorCovariance[0][1],
                              value.errorCovariance[0][2],
                              value.errorCovariance[1][0],
                              value.errorCovariance[1][1],
                              value.errorCovariance[1][2],
                              value.errorCovariance[2][0],
                              value.errorCovariance[2][1],
                              value.errorCovariance[2][2],
                              value.gatingFunctionGain,
                              value.confidenceLevel);
    }
};

template <>
struct std::formatter<iwr1443::SphericalCompressedPointCloudHeader> : std::formatter<float> {
    template <typename FormatContext>
    auto format(const iwr1443::SphericalCompressedPointCloudHeader &value, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        return std::format_to(
            ctx.out(),
            R"({{"elevationUnit": {}, "azimuthUnit": {}, "dopplerUnit": {}, "rangeUnit": {}, "snrUnit": {}}})",
            value.elevationUnit,
            value.azimuthUnit,
            value.dopplerUnit,
            value.rangeUnit,
            value.snrUnit);
    }
};

template <>
struct std::formatter<iwr1443::SphericalCompressedPoint> : std::formatter<uint16_t> {
    template <typename FormatContext>
    auto format(const iwr1443::SphericalCompressedPoint &value, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        return std::format_to(
            ctx.out(),
            R"({{"elevation": {}, "azimuth": {}, "doppler": {}, "range": {}, "snr": {}}})",
            value.elevation,
            value.azimuth,
            value.doppler,
            value.range,
            value.snr);
    }
};
