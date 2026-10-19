// Maintain in ascending PDO order. Reference: aiocomfoconnect
// 8fe395b3f8657e25ef8c5a6ccea5aab06808a514 (docs/PROTOCOL-PDO.md and sensors.py).
#pragma once

#include "pdo_decode.h"
#include <algorithm>
#include <iterator>

namespace zehnder_comfoair_q {

struct PdoDefinition {
    uint16_t id;
    PdoType type;
};

inline const PdoDefinition *find_pdo_definition(uint16_t id) {
    static constexpr PdoDefinition definitions[] = {
        {16, PdoType::UINT8},
        {17, PdoType::UINT8},
        {18, PdoType::UINT8},
        {33, PdoType::UINT8},
        {35, PdoType::UINT8},
        {36, PdoType::UINT8},
        {37, PdoType::UINT8},
        {40, PdoType::UINT8},
        {42, PdoType::UINT8},
        {49, PdoType::UINT8},
        {50, PdoType::UINT8},
        {51, PdoType::UINT8},
        {52, PdoType::UINT8},
        {53, PdoType::UINT8},
        {54, PdoType::UINT8},
        {55, PdoType::UINT8},
        {56, PdoType::UINT8},
        {57, PdoType::UINT8},
        {58, PdoType::UINT8},
        {65, PdoType::UINT8},
        {66, PdoType::UINT8},
        {67, PdoType::UINT8},
        {68, PdoType::UINT8},
        // PDO 69 has no declared type in either reference; deliberately omit it.
        {70, PdoType::UINT8},
        {71, PdoType::UINT8},
        {72, PdoType::UINT8},
        {73, PdoType::UINT8},
        {74, PdoType::UINT8},
        {81, PdoType::UINT32},
        {82, PdoType::UINT32},
        {85, PdoType::UINT32},
        {86, PdoType::UINT32},
        {87, PdoType::UINT32},
        {88, PdoType::UINT32},
        {89, PdoType::UINT32},
        {90, PdoType::UINT32},
        {96, PdoType::BOOL},
        {115, PdoType::BOOL},
        {116, PdoType::BOOL},
        {117, PdoType::UINT8},
        {118, PdoType::UINT8},
        {119, PdoType::UINT16},
        {120, PdoType::UINT16},
        {121, PdoType::UINT16},
        {122, PdoType::UINT16},
        {128, PdoType::UINT16},
        {129, PdoType::UINT16},
        {130, PdoType::UINT16},
        {144, PdoType::UINT16},
        {145, PdoType::UINT16},
        {146, PdoType::UINT16},
        {176, PdoType::UINT8},
        {192, PdoType::UINT16},
        {208, PdoType::UINT8},
        {209, PdoType::INT16},
        {210, PdoType::BOOL},
        {211, PdoType::BOOL},
        // 212: registry INT16; documentation UINT8 conflicts with observed two-byte width.
        // Positive samples establish width, not signedness independently of the registry.
        {212, PdoType::INT16},
        // 213/216: retain raw-watts scaling; documentation examples imply an unverified /100.
        {213, PdoType::UINT16},
        {214, PdoType::UINT16},
        {215, PdoType::UINT16},
        {216, PdoType::UINT16},
        {217, PdoType::UINT16},
        {218, PdoType::UINT16},
        {219, PdoType::UINT16},
        {220, PdoType::INT16},
        {221, PdoType::INT16},
        {224, PdoType::UINT8},
        {225, PdoType::UINT8},
        {226, PdoType::UINT16},
        {227, PdoType::UINT8},
        {228, PdoType::UINT8},
        {229, PdoType::BOOL},
        {230, PdoType::INT64},
        {256, PdoType::UINT8},
        {257, PdoType::UINT8},
        {274, PdoType::INT16},
        {275, PdoType::INT16},
        {276, PdoType::INT16},
        {277, PdoType::INT16},
        {278, PdoType::INT16},
        {290, PdoType::UINT8},
        {291, PdoType::UINT8},
        {292, PdoType::UINT8},
        {293, PdoType::UINT8},
        {294, PdoType::UINT8},
        {321, PdoType::UINT16},
        {325, PdoType::UINT16},
        {328, PdoType::UINT16},
        {330, PdoType::UINT16},
        {337, PdoType::UINT32},
        {338, PdoType::UINT32},
        {341, PdoType::UINT32},
        {342, PdoType::UINT32},
        {343, PdoType::UINT32},
        {344, PdoType::UINT32},
        {345, PdoType::UINT32},
        {346, PdoType::UINT32},
        {369, PdoType::UINT8},
        {370, PdoType::UINT8},
        {371, PdoType::UINT8},
        {372, PdoType::UINT8},
        {384, PdoType::INT16},
        {386, PdoType::BOOL},
        {400, PdoType::INT16},
        {401, PdoType::UINT8},
        {402, PdoType::BOOL},
        {416, PdoType::INT16},
        {417, PdoType::INT16},
        {418, PdoType::UINT8},
        {419, PdoType::BOOL},
        {513, PdoType::UINT16},
        {514, PdoType::UINT16},
        {515, PdoType::UINT16},
        {516, PdoType::UINT16},
        {517, PdoType::UINT8},
        {518, PdoType::UINT8},
        {519, PdoType::UINT8},
        {520, PdoType::UINT8},
        {521, PdoType::INT16},
        {522, PdoType::INT16},
        {523, PdoType::INT16},
        {524, PdoType::UINT8},
        {784, PdoType::UINT8},
        {785, PdoType::BOOL},
        {801, PdoType::INT16},
        {802, PdoType::INT16},
        {803, PdoType::INT16},
        {1024, PdoType::UINT16},
        {1025, PdoType::UINT16},
        {1026, PdoType::UINT16},
        {1027, PdoType::UINT16},
        {1028, PdoType::UINT16},
        {1029, PdoType::UINT16},
        {1030, PdoType::UINT16},
        {1031, PdoType::UINT16},
        {1056, PdoType::INT16},
        {1124, PdoType::INT16},
        {1125, PdoType::INT16},
        {1126, PdoType::INT16},
        {1127, PdoType::INT16},
        {1281, PdoType::UINT16},
        {1282, PdoType::UINT16},
        {1283, PdoType::UINT16},
        {1284, PdoType::UINT16},
        {1285, PdoType::UINT16},
        {1286, PdoType::UINT16},
        {1287, PdoType::UINT16},
        {1288, PdoType::UINT16},
        {1297, PdoType::BOOL},
        {1298, PdoType::BOOL},
        {1299, PdoType::BOOL},
        {1300, PdoType::BOOL},
        {1301, PdoType::BOOL},
        {1302, PdoType::BOOL},
        {1303, PdoType::BOOL},
        {1304, PdoType::BOOL},
    };
    static_assert([] {
        for (size_t i = 1; i < std::size(definitions); i++)
            if (definitions[i - 1].id >= definitions[i].id) return false;
        return true;
    }(), "PDO definitions must be sorted and unique");
    const auto *last = definitions + std::size(definitions);
    const auto *found = std::lower_bound(definitions, last, id,
        [](const PdoDefinition &definition, uint16_t value) { return definition.id < value; });
    return found != last && found->id == id ? found : nullptr;
}

inline bool decode_pdo(uint16_t id, const std::vector<uint8_t> &data, PdoValue &value) {
    const auto *definition = find_pdo_definition(id);
    if (definition == nullptr) return false;
    return decode_pdo_value(definition->type, data, value);
}

} // namespace zehnder_comfoair_q
