#pragma once

#include <cinttypes>
#include <cstdio>
#include <string>
#include <vector>

namespace zehnder_comfoair_q
{
    inline std::string airflow_constraints_to_string(const std::vector<uint8_t> &data)
    {
        if (data.size() != 8)
            return {};
        uint64_t value = 0;
        for (size_t i = 0; i < 8; i++)
            value |= uint64_t(data[i]) << (8 * i);
        char raw[19];
        std::snprintf(raw, sizeof(raw), "0x%016" PRIx64, value);
        std::string output = std::string(raw) + ": ";
        if (!(value & (uint64_t(1) << 45)))
            return output + "unavailable";

        const struct Reason { uint64_t mask; const char *name; } reasons[] = {
            {(1ULL << 2) | (1ULL << 3), "Resistance"},
            {1ULL << 4, "PreheaterNegative"},
            {(1ULL << 5) | (1ULL << 7), "NoiseGuard"},
            {(1ULL << 6) | (1ULL << 8), "ResistanceGuard"},
            {1ULL << 9, "FrostProtection"}, {1ULL << 10, "Bypass"},
            {1ULL << 12, "AnalogInput1"}, {1ULL << 13, "AnalogInput2"},
            {1ULL << 14, "AnalogInput3"}, {1ULL << 15, "AnalogInput4"},
            {1ULL << 16, "Hood"}, {1ULL << 18, "AnalogPreset"},
            {1ULL << 19, "ComfoCool"}, {1ULL << 22, "PreheaterPositive"},
            {1ULL << 23, "RFSensorFlowPreset"}, {1ULL << 24, "RFSensorFlowProportional"},
            {1ULL << 25, "TemperatureComfort"}, {1ULL << 26, "HumidityComfort"},
            {1ULL << 27, "HumidityProtection"},
        };
        std::string labels;
        auto append = [&labels](const std::string &name) {
            if (!labels.empty()) labels += ", ";
            labels += name;
        };
        uint64_t known = 1ULL << 45;
        for (const auto &reason : reasons)
        {
            known |= reason.mask;
            if (value & reason.mask) append(reason.name);
        }
        for (unsigned bit = 47; bit <= 54; bit++)
        {
            known |= 1ULL << bit;
            if (value & (1ULL << bit)) append("CO2ZoneX" + std::to_string(bit - 46));
        }
        for (unsigned bit = 0; bit < 64; bit++)
            if (value & ~known & (1ULL << bit)) append("Unknown(bit " + std::to_string(bit) + ")");
        return output + (labels.empty() ? "None" : labels);
    }
}
