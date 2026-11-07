#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace zehnder_comfoair_q {

// Values match the protocol type codes, not the number of payload bytes.
enum class PdoType : uint8_t {
    BOOL = 0, UINT8 = 1, UINT16 = 2, UINT32 = 3,
    INT8 = 5, INT16 = 6, INT64 = 8, NONE = 255,
};

struct PdoValue {
    uint64_t raw = 0;
    bool is_signed = false;

    int64_t signed_value() const {
        // Avoid an out-of-range unsigned-to-signed cast at the INT64 boundary.
        return raw <= uint64_t(INT64_MAX) ? int64_t(raw) : -1 - int64_t(~raw);
    }
};

inline size_t pdo_type_size(PdoType type) {
    switch (type) {
        case PdoType::BOOL: case PdoType::UINT8: case PdoType::INT8: return 1;
        case PdoType::UINT16: case PdoType::INT16: return 2;
        case PdoType::UINT32: return 4;
        case PdoType::INT64: return 8;
        default: return 0;
    }
}

inline bool decode_pdo_value(PdoType type, const std::vector<uint8_t> &data, PdoValue &output) {
    const size_t width = pdo_type_size(type);
    if (width == 0 || data.size() != width || (type == PdoType::BOOL && data[0] > 1))
        return false;

    PdoValue value;
    for (size_t i = 0; i < width; i++)
        value.raw |= uint64_t(data[i]) << (8 * i);
    value.is_signed = type == PdoType::INT8 || type == PdoType::INT16 || type == PdoType::INT64;
    if (value.is_signed && width < 8 && (value.raw & (uint64_t(1) << (width * 8 - 1))))
        value.raw |= UINT64_MAX << (width * 8);
    output = value;
    return true;
}

} // namespace zehnder_comfoair_q
