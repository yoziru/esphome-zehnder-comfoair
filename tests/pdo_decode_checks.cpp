#include "pdo_types.h"
#include "airflow_constraints.h"
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace zehnder_comfoair_q;

static unsigned failures = 0;
static void check(bool result, const char *contract) {
    if (!result) {
        std::cerr << "FAIL: " << contract << '\n';
        failures++;
    }
}

int main() {
    PdoValue value;
    check(decode_pdo_value(PdoType::UINT8, {255}, value) && value.raw == 255 && !value.is_signed, "UINT8 sentinel");
    check(decode_pdo_value(PdoType::UINT16, {0xff, 0xff}, value) && value.raw == 65535, "UINT16 maximum");
    check(decode_pdo_value(PdoType::UINT32, {0xff, 0xff, 0xff, 0xff}, value) && value.raw == 4294967295ULL, "UINT32 sentinel");
    check(decode_pdo_value(PdoType::INT8, {0x80}, value) && value.signed_value() == -128, "INT8 minimum");
    check(decode_pdo_value(PdoType::INT8, {0xff}, value) && value.signed_value() == -1, "INT8 negative one");
    check(decode_pdo_value(PdoType::INT8, {0x7f}, value) && value.signed_value() == 127, "INT8 maximum");
    check(decode_pdo_value(PdoType::INT16, {0x85, 0xff}, value) && value.signed_value() == -123, "INT16 negative temperature");
    check(decode_pdo_value(PdoType::INT16, {0, 0x80}, value) && value.signed_value() == -32768, "INT16 minimum");
    check(decode_pdo_value(PdoType::INT16, {0xff, 0x7f}, value) && value.signed_value() == 32767, "INT16 maximum");
    check(decode_pdo_value(PdoType::INT64, {0, 0, 0, 0, 0, 0, 0, 0x80}, value) &&
          value.signed_value() == std::numeric_limits<int64_t>::min(), "INT64 minimum without signed overflow");
    check(decode_pdo_value(PdoType::INT64, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f}, value) &&
          value.signed_value() == std::numeric_limits<int64_t>::max(), "INT64 maximum");
    check(decode_pdo_value(PdoType::INT64, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, value) &&
          value.signed_value() == -1 && value.raw == UINT64_MAX, "INT64 negative one retains bitset");
    check(decode_pdo_value(PdoType::BOOL, {0}, value) && value.raw == 0, "BOOL false");
    check(decode_pdo_value(PdoType::BOOL, {1}, value) && value.raw == 1, "BOOL true");
    check(!decode_pdo_value(PdoType::BOOL, {2}, value) && value.raw == 1, "invalid BOOL preserves output");
    for (auto type : {PdoType::BOOL, PdoType::UINT8, PdoType::UINT16, PdoType::UINT32,
                      PdoType::INT8, PdoType::INT16, PdoType::INT64}) {
        for (size_t width = 0; width <= 9; width++) {
            if (width == pdo_type_size(type)) continue;
            check(!decode_pdo_value(type, std::vector<uint8_t>(width, 0), value), "reject every incorrect width");
        }
    }
    check(!decode_pdo_value(PdoType::NONE, {}, value), "unsupported type is rejected");
    check(decode_pdo(129, {0x40, 0x9c}, value) && value.raw == 40000, "declared UINT16 preserves high values");
    check(!decode_pdo(129, {1}, value) && value.raw == 40000, "known PDO rejects wrong supported width");
    check(!decode_pdo(129, {1, 0, 0, 0}, value) && value.raw == 40000, "known PDO rejects four-byte UINT16");
    check(!decode_pdo(212, {233}, value) && value.raw == 40000, "short target payload rejected without alternate decoding");
    check(decode_pdo(212, {0x2c, 1}, value) && value.signed_value() == 300, "registry target width");
    check(!decode_pdo(212, {1, 0, 0, 0}, value), "unverified target width rejected");
    check(decode_pdo(81, {0, 0, 0, 0x80}, value) && value.raw == 2147483648ULL && !value.is_signed, "unsigned timer high bit");
    check(decode_pdo(230, {0, 0, 0, 0, 0, 0x20, 0, 0x80}, value) && value.raw == 0x8000200000000000ULL, "full 64-bit constraint payload");
    check(airflow_constraints_to_string({0, 0, 0, 0, 0, 0x20, 0, 0x80}) ==
          "0x8000200000000000: Unknown(bit 63)", "constraint output survives shared decoder");
    check(airflow_constraints_to_string({0, 0, 0, 0, 0, 0x20, 0x40, 0}) ==
          "0x0040200000000000: CO2ZoneX8", "high CO2 bit stays exact");
    check(find_pdo_definition(16) && find_pdo_definition(1304), "first and last mapped IDs resolve");
    check(!find_pdo_definition(0) && !find_pdo_definition(69) && !find_pdo_definition(305) &&
          !find_pdo_definition(1305), "unknown/untyped entries are not invented");
    check(!decode_pdo(305, {0x67, 0}, value), "unknown PDO never falls back to length inference");
    check(decode_pdo_value(PdoType::UINT16, {0x67, 0}, value) && value.raw == 103,
          "raw diagnostics require an explicit representation");
    std::cout << "PDO decoder checks: " << failures << " failures\n";
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
