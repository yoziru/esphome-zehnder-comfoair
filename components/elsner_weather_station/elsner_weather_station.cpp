#include "elsner_weather_station.h"

namespace elsner_weather_station
{
    void ElsnerWeatherStation::setup()
    {
        reset_local_wdt();
    }

    void ElsnerWeatherStation::loop()
    {
        while (available())
        {
            char sensor_data = read();
            ESP_LOGVV(TAG, "Received char '%c'", sensor_data);

            // ignore everything before start of message
            if (sensor_data == 'W' || !sensor_message_.empty())
            {
                auto message_has_error = false;
                size_t message_length = 0;
                uint checksum_sent = 0, checksum_calculated = 0;

                // start of message from sensor?
                if (sensor_data == 'W')
                    sensor_message_.clear();

                sensor_message_ += sensor_data;
                message_length = sensor_message_.length();

                // end of message from sensor?
                if (sensor_data == 0x03)
                {
                    if (message_length >= 26)
                    {
                        for (int i = 0; i < message_length - 5; i++)
                            checksum_calculated += sensor_message_[i];

                        checksum_sent = atoi(sensor_message_.substr(message_length - 5, 4).c_str());

                        if (checksum_calculated == checksum_sent)
                        {
                            ESP_LOGV(TAG, "Received valid sensor message: '%s'", sensor_message_.c_str());
                            reset_local_wdt(true); // also clear local error just in case

                            float temp = atof(sensor_message_.substr(1, 5).c_str());
                            uint sun_s = atoi(sensor_message_.substr(6, 2).c_str());
                            uint sun_w = atoi(sensor_message_.substr(8, 2).c_str());
                            uint sun_e = atoi(sensor_message_.substr(10, 2).c_str());
                            bool is_dark = sensor_message_[12] == 'J';
                            uint daylight = atoi(sensor_message_.substr(13, 3).c_str());
                            float wind_ms = atof(sensor_message_.substr(16, 4).c_str());
                            bool is_raining = sensor_message_[20] == 'J';

                            ESP_LOGD(TAG, "Received measurements: temp: %f, sun_s: %u, sun_w: %u, sun_e: %u, is_dark: %u, daylight: %u, wind_ms: %f, is_raining: %u",
                                     temp, sun_s, sun_w, sun_e, is_dark, daylight, wind_ms, is_raining);

                            if (temperature_sensor_)
                                temperature_sensor_->publish_state(temp);
                            if (sun_south_sensor_)
                                sun_south_sensor_->publish_state(sun_s);
                            if (sun_west_sensor_)
                                sun_west_sensor_->publish_state(sun_w);
                            if (sun_east_sensor_)
                                sun_east_sensor_->publish_state(sun_e);
                            if (is_dark_sensor_)
                                is_dark_sensor_->publish_state(is_dark);
                            if (illuminance_sensor_)
                                illuminance_sensor_->publish_state(daylight);
                            if (wind_speed_sensor_)
                                wind_speed_sensor_->publish_state(wind_ms);
                            if (is_raining_sensor_)
                                is_raining_sensor_->publish_state(is_raining);

                            sensor_message_.clear();
                        }
                        else
                            message_has_error = true;
                    }
                    else
                        message_has_error = true;
                }

                if (message_length > 100)
                    message_has_error = true;

                if (message_has_error)
                {
                    ESP_LOGE(TAG, "Received invalid message: '%s', length: %u, checksum_calculated: %u, checksum_sent: %u",
                             sensor_message_.c_str(), message_length, checksum_calculated, checksum_sent);
                    set_local_error();

                    sensor_message_.clear();
                }
            }
        }
    }

    void ElsnerWeatherStation::reset_local_wdt(bool also_clear_local_error)
    {
        if (also_clear_local_error)
            clear_local_error();

        set_timeout("ERROR", wdt_timeout_,
                    [this]()
                    {
                        ESP_LOGE(TAG, "Did not receive any message from sensor within %u ms!", this->wdt_timeout_);
                        this->set_local_error();
                    });
    }

    void ElsnerWeatherStation::set_local_error()
    {
        status_set_error();

        if (alarm_sensor_)
            alarm_sensor_->publish_state(true);
    }

    void ElsnerWeatherStation::clear_local_error()
    {
        status_clear_error();

        if (alarm_sensor_)
            alarm_sensor_->publish_state(false);
    }
}
