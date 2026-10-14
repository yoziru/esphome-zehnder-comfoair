#include "pdo_address.h"

#include <iostream>
#include <vector>

int main() {
    using namespace zehnder_comfoair_q;
    int failures = 0;
    auto check = [&](bool condition, const char *message) {
        if (!condition) { std::cerr << message << '\n'; failures++; }
    };
    uint32_t id = 0;
    for (auto pdo : std::vector<uint16_t>{0, 16, 65, 1023, 1024, 1304, 2047}) {
        for (auto node : std::vector<uint8_t>{1, 42, 63}) {
            check(make_pdo_can_id(pdo, node, id) && (id >> 14) == pdo &&
                  (id & 0x3f) == node && is_pdo_can_id(id), "PDO and node round-trip");
        }
    }
    check(!make_pdo_can_id(2048, 42, id) && !make_pdo_can_id(65535, 42, id) &&
          !make_pdo_can_id(1, 0, id) && !make_pdo_can_id(1, 64, id), "reject unsupported PDO/node");
    check(is_pdo_can_id(0x01000041), "accept PDO 1024 namespace");
    for (auto unrelated : std::vector<uint32_t>{0x10000001, 0x1f000a81, 0x1f800000, 0x80000041, 0x02000041})
        check(!is_pdo_can_id(unrelated), "exclude heartbeat/RMI/firmware/invalid or unsupported namespace");
    std::cout << "PDO addressing checks: " << failures << " failures\n";
    return failures ? 1 : 0;
}
