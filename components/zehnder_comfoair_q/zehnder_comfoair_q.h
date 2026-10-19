#pragma once

#include <esphome.h>
#include "airflow_constraints.h"
#include "pdo_types.h"
#include <cmath>

using namespace esphome; // needed for ESP_LOGx to work
using namespace esphome::canbus;

namespace zehnder_comfoair_q
{
    static const char *const TAG = "comfoairq";

    enum VentilationLevel : uint8_t
    {
        VENTILATION_LEVEL_AWAY = 0,
        VENTILATION_LEVEL_LOW = 1,
        VENTILATION_LEVEL_MEDIUM = 2,
        VENTILATION_LEVEL_HIGH = 3,
    };
    enum TemperatureProfile : uint8_t
    {
        TEMP_PROFILE_NORMAL = 0,
        TEMP_PROFILE_COOL = 1,
        TEMP_PROFILE_WARM = 2,
    };

    enum BypassMode : uint8_t
    {
        BYPASS_AUTO = 0,
        BYPASS_ACTIVATE = 1,
        BYPASS_DEACTIVATE = 2,
    };

    enum OffAutoOn : uint8_t
    {
        OAO_OFF = 0,
        OAO_AUTO = 1,
        OAO_ON = 2,
    };

    enum BalanceMode : uint8_t
    {
        BALANCE_MODE_BALANCED = 0,
        BALANCE_MODE_SUPPLY_ONLY = 1,
        BALANCE_MODE_EXHAUST_ONLY = 2,
    };

    class ZehnderComfoAirQ : public PollingComponent
    {
    public:
        void setup() override;
        void update() override;

        void set_parent(Canbus *parent) { parent_ = parent; }
        void set_request_ids(const std::vector<uint16_t> &request_ids) { request_ids_ = request_ids; }
        void set_request_delay(uint32_t request_delay) { request_delay_ = request_delay; }
        void set_local_node_id(uint8_t local_node_id) { local_node_id_ = local_node_id; }

        void request_all_pdos();
        void request_pdo(uint16_t pdo_id);

        void set_level(uint8_t level);
        void set_level_float(float state);

        void set_boost(uint32_t duration_secs) { send_command_set_timer(duration_secs > 0, 0x01, 0x06, 3, duration_secs); }
        void set_manual_mode(bool enable) { send_command_set_timer(enable, 0x08, 0x01, enable ? 1 : 0); }
        void set_ventilation_level(VentilationLevel ventilation_level) { set_level(ventilation_level); }
        void set_temp_profile(TemperatureProfile temp_profile) { send_command_set_timer(true, 0x03, 0x01, temp_profile, 0xffffffff); }
        void set_bypass_mode(BypassMode bypass_mode, uint32_t duration_secs) { send_command_set_timer(bypass_mode != BYPASS_AUTO, 0x02, 0x01, bypass_mode, duration_secs); }
        void set_temperature_passive(OffAutoOn oao) { send_command_set_property(0x1d /* TEMPHUMCONTROL */, 0x01, 0x04, oao); }
        void set_humidity_comfort(OffAutoOn oao) { send_command_set_property(0x1d /* TEMPHUMCONTROL */, 0x01, 0x06, oao); }
        void set_humidity_protection(OffAutoOn oao) { send_command_set_property(0x1d /* TEMPHUMCONTROL */, 0x01, 0x07, oao); }
        void set_away(bool enable) { send_command_set_timer(enable, 0x01, 0x0B, 0x00, 0xffffffff); }
        void set_supply_only(bool enable) { send_command_set_timer(enable, 0x06, 0x01, 0x00, 0xffffffff); }
        void set_extract_only(bool enable) { send_command_set_timer(enable, 0x07, 0x01, 0x00, 0xffffffff); }
        void set_balance_mode(BalanceMode balance_mode)
        {
            if (balance_mode == BALANCE_MODE_BALANCED)
            {
                set_supply_only(false);
                set_extract_only(false);
            }
            else if (balance_mode == BALANCE_MODE_SUPPLY_ONLY)
            {
                set_supply_only(true);
                set_extract_only(false);
            }
            else if (balance_mode == BALANCE_MODE_EXHAUST_ONLY)
            {
                set_supply_only(false);
                set_extract_only(true);
            }
            else
            {
                ESP_LOGW(TAG, "Unknown balance mode: %d", balance_mode);
            }
        }

        void send_command_set_timer(bool enable, uint8_t subunit_id, uint8_t property_id, uint8_t property_value = 0x00,
                                    uint32_t duration_secs = 1 /* seems to be the constant for timers with pre-defined durations */);
        void send_command_set_property(uint8_t unit_id, uint8_t subunit_id, uint8_t property_id, uint8_t property_value);
        void send_command_set_property_uint16(uint8_t unit_id, uint8_t subunit_id, uint8_t property_id, uint16_t value)
        {
            send_command({0x03, unit_id, subunit_id, property_id, uint8_t(value), uint8_t(value >> 8)});
        }
        bool set_filter_lifetime(float days)
        {
            if (!std::isfinite(days) || days < 0 || days > 65535 || std::floor(days) != days)
                return false;
            send_command_set_property_uint16(0x1c, 0x01, 0x03, uint16_t(days));
            return true; // Valid request sent, not a device acknowledgement.
        }
        void send_command(const std::vector<uint8_t> &command);

        // keep template functions in here to make sure they get implemented for all required types by being used from lambdas inside yaml

        template <class T, typename U,
                  typename std::enable_if<std::is_base_of<sensor::Sensor, T>::value || std::is_base_of<binary_sensor::BinarySensor, T>::value, bool>::type = true>
        static void publish_state(T *state_id, U state_value)
        {
            id(state_id).publish_state(state_value);
        }
        template <class T, typename U,
                  typename std::enable_if<std::is_base_of<text_sensor::TextSensor, T>::value, bool>::type = true>
        static void publish_state(T *state_id, U state_value)
        {
            id(state_id).publish_state(to_string(state_value));
        }

        template <class T>
        static void publish_pdo_state(T state_id, const std::vector<uint8_t> &can_message,
                                      uint16_t pdo_id)
        {
            PdoValue value;
            if (!decode_pdo(pdo_id, can_message, value))
            {
                ESP_LOGW(TAG, "Invalid payload for PDO %u (length %zu)", pdo_id, can_message.size());
                return;
            }
            if (value.is_signed)
                publish_state(state_id, value.signed_value());
            else
                publish_state(state_id, value.raw);
        }

        static __attribute((always_inline)) inline uint16_t get_pdo_id_from_can_id(uint32_t can_id)
        {
            return can_id >> 14;
        }

        static void publish_duration_state(sensor::Sensor *raw, text_sensor::TextSensor *formatted,
                                           const std::vector<uint8_t> &message)
        {
            if (message.size() != 4)
                return;
            uint32_t seconds = 0;
            for (size_t i = 0; i < 4; i++)
                seconds |= uint32_t(message[i]) << (8 * i);
            raw->publish_state(seconds);
            formatted->publish_state(seconds_to_human_readable(seconds));
        }

        static std::string seconds_to_human_readable(uint32_t seconds)
        {
            if (seconds == UINT32_MAX)
                return "n/a";

            std::string output;
            if (output.length() > 0 || seconds >= 86400)
            {
                auto days = seconds / 86400;
                output += to_string(days) + "d";
                seconds -= days * 86400;
            }
            if (output.length() > 0 || seconds >= 3600)
            {
                auto hours = seconds / 3600;
                output += to_string(hours) + "h";
                seconds -= hours * 3600;
            }
            if (output.length() > 0 || seconds >= 60)
            {
                auto minutes = seconds / 60;
                output += to_string(minutes) + "m";
                seconds -= minutes * 60;
            }
            output += to_string(seconds) + "s";

            return output;
        }

    protected:
        Canbus *parent_ = nullptr;

        std::vector<uint16_t> request_ids_{};
        uint32_t request_delay_;
        int request_next_pdo_pos_ = 0;
        void request_next_pdo_();

        uint8_t local_node_id_ = 0x2a;

        // Builds the CAN id of a command frame sent from this node to the ComfoAir Q unit (node 0x01).
        uint32_t get_command_can_id_(bool is_multi_message_command, uint8_t sequence_number);
        uint8_t get_command_next_sequence_number_();
        uint8_t command_sequence_number_ = 0;

        void send_can_message_(uint32_t can_id, bool remote_transmission_request, const std::vector<uint8_t> &data = {});
    };
}

#define comfoair_on_frame_case(pdo_id, comfoair_component, sensor_id) \
    case pdo_id:                                                                \
        comfoair_component->publish_pdo_state(sensor_id, x, pdo_id);             \
        break;
