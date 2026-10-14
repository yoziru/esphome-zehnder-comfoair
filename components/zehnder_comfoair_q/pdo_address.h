#pragma once

#include <cstdint>

namespace zehnder_comfoair_q {

// The reference includes IDs above 1023. Support the required eleven PDO bits,
// without admitting the unrelated heartbeat, RMI, or firmware namespaces.
constexpr uint16_t MAX_SUPPORTED_PDO = 0x7ff;
constexpr uint32_t PDO_RECEIVE_MASK = 0x1e003fc0;

inline bool make_pdo_can_id(uint16_t pdo, uint8_t node, uint32_t &id) {
    if (pdo > MAX_SUPPORTED_PDO || node == 0 || node > 0x3f)
        return false;
    id = (uint32_t(pdo) << 14) | 0x40u | node;
    return true;
}

inline bool is_pdo_can_id(uint32_t id) {
    return (id & ~0x1fffffffu) == 0 && (id & PDO_RECEIVE_MASK) == 0x40u;
}

} // namespace zehnder_comfoair_q
