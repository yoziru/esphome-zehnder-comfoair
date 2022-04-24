#pragma once

#include <esphome.h>
#include <esphome/components/uart/uart.h>

using namespace esphome; // needed for ESP_LOGx to work
using namespace esphome::uart;

namespace elsner_weather_station
{
    static const char *const TAG = "weather";

    class ElsnerWeatherStation : public Component, public uart::UARTDevice
    {
    public:
        float get_setup_priority() const { return setup_priority::DATA; }

        void setup() override;
        void loop() override;

        void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
        void set_sun_south_sensor(sensor::Sensor *sun_south_sensor) { sun_south_sensor_ = sun_south_sensor; }
        void set_sun_west_sensor(sensor::Sensor *sun_west_sensor) { sun_west_sensor_ = sun_west_sensor; }
        void set_sun_east_sensor(sensor::Sensor *sun_east_sensor) { sun_east_sensor_ = sun_east_sensor; }
        void set_is_dark_sensor(binary_sensor::BinarySensor *is_dark_sensor) { is_dark_sensor_ = is_dark_sensor; }
        void set_illuminance_sensor(sensor::Sensor *illuminance_sensor) { illuminance_sensor_ = illuminance_sensor; }
        void set_wind_speed_sensor(sensor::Sensor *wind_speed_sensor) { wind_speed_sensor_ = wind_speed_sensor; }
        void set_is_raining_sensor(binary_sensor::BinarySensor *is_raining_sensor) { is_raining_sensor_ = is_raining_sensor; }
        void set_alarm_sensor(binary_sensor::BinarySensor *alarm_sensor) { alarm_sensor_ = alarm_sensor; }

        void set_watchdog_timeout(uint32_t wdt_timeout) { wdt_timeout_ = wdt_timeout; }

    protected:
        std::string sensor_message_{};

        sensor::Sensor *temperature_sensor_{nullptr};
        sensor::Sensor *sun_south_sensor_{nullptr};
        sensor::Sensor *sun_west_sensor_{nullptr};
        sensor::Sensor *sun_east_sensor_{nullptr};
        binary_sensor::BinarySensor *is_dark_sensor_{nullptr};
        sensor::Sensor *illuminance_sensor_{nullptr};
        sensor::Sensor *wind_speed_sensor_{nullptr};
        binary_sensor::BinarySensor *is_raining_sensor_{nullptr};
        binary_sensor::BinarySensor *alarm_sensor_{nullptr};

        uint32_t wdt_timeout_;

        void reset_local_wdt(bool also_clear_local_error = false);
        void clear_local_error();
        void set_local_error();
    };
}
