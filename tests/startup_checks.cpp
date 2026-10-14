// Compiled as an on_boot lambda inside the real generated ESPHome firmware.
using esphome::test_canbus::check;
auto &bus = *id(test_bus);
check(bus.sent.empty(), "setup must not write ventilation settings");
bus.sent.clear();

const auto high_request_count = bus.requests.size();
id(comfoair).request_pdo(1024);
check(bus.requests.size() == high_request_count + 1 && bus.requests.back().can_id == ((1024u << 14) | 0x6a) &&
      bus.requests.back().remote_transmission_request && bus.requests.back().use_extended_id,
      "high PDO emits the correct extended RTR address");
id(comfoair).request_pdo(2048);
check(bus.requests.size() == high_request_count + 1, "out-of-range direct request is rejected");
bus.requests.clear();

// Literal payloads reconstruct through the actual component's public CAN boundary.
uint32_t previous_sequence = 0;
for (size_t length : {8u, 9u, 14u, 15u, 4u}) {
  std::vector<uint8_t> payload(length);
  for (size_t i = 0; i < length; i++)
    payload[i] = uint8_t(0x20 + i);
  bus.sent.clear();
  id(comfoair).send_command(payload);
  const bool fragmented = length > 8;
  check(bus.sent.size() == (fragmented ? (length + 6) / 7 : 1), "command frame count respects 8/7 byte boundaries");
  if (bus.sent.empty()) continue; // Report the missing frames rather than dereferencing front().
  std::vector<uint8_t> reconstructed;
  for (size_t i = 0; i < bus.sent.size(); i++) {
    const auto &frame = bus.sent[i];
    check(frame.use_extended_id && !frame.remote_transmission_request,
          "commands use extended data frames, not RTR");
    check(frame.can_id == bus.sent.front().can_id && (frame.can_id & 0x3f) == 0x2a &&
          ((frame.can_id >> 6) & 0x3f) == 1 && ((frame.can_id >> 14) & 1) == fragmented,
          "command fragments share destination, local node, sequence and fragmentation flag");
    if (fragmented)
      check(frame.data[0] == (i | (i + 1 == bus.sent.size() ? 0x80 : 0)),
            "only the final fragment has its final marker");
    reconstructed.insert(reconstructed.end(), frame.data + (fragmented ? 1 : 0),
                         frame.data + frame.can_data_length_code);
  }
  check(reconstructed == payload, "all payload bytes survive CAN fragmentation in order");
  const auto sequence = (bus.sent.front().can_id >> 17) & 3;
  if (length != 8)
    check(sequence == ((previous_sequence + 1) & 3), "command sequence advances and wraps modulo four");
  previous_sequence = sequence;
}
bus.sent.clear();

for (const auto &entry : std::vector<std::pair<uint16_t, sensor::Sensor *>>{
         {119, id(exhaust_fan_flow)}, {120, id(supply_fan_flow)},
         {121, id(exhaust_fan_speed)}, {122, id(supply_fan_speed)},
         {128, id(power)}, {129, id(energy_ytd)}, {130, id(energy_since_start)},
         {192, id(filter_replacement_remaining_days)},
         {213, id(avoided_heating_actual)}, {214, id(avoided_heating_ytd)},
         {215, id(avoided_heating_total)}, {216, id(avoided_cooling_actual)},
         {217, id(avoided_cooling_ytd)}, {218, id(avoided_cooling_total)}}) {
  bus.receive(entry.first, std::vector<uint8_t>{0x40, 0x9c}); // 40000 unsigned
  check(entry.second->raw_state == 40000, "documented UINT16 PDO retains values above 32767");
}
bus.receive(276, std::vector<uint8_t>{0x85, 0xff}); // -123 tenths C
check(std::abs(id(outdoor_air_temp).state + 12.3f) < 0.001f, "INT16 PDO preserves negative temperature");
for (const auto &payload : std::vector<std::vector<uint8_t>>{{}, {1, 2, 3}, {1, 2, 3, 4, 5}}) {
  bus.receive(129, payload);
  check(id(energy_ytd).raw_state == 40000, "unsupported PDO widths do not publish invented state");
}
check(bus.sent.empty(), "numeric PDO decoding never sends control commands");

bus.receive(81, std::vector<uint8_t>{0x00, 0x00, 0x00, 0x80});
check(id(next_fan_change_in).state == "24855d3h14m8s", "UINT32 duration above INT32_MAX is not a no-change sentinel");
bus.receive(82, std::vector<uint8_t>{0xfe, 0xff, 0xff, 0xff});
check(id(next_bypass_change_in).state == "49710d6h28m14s", "UINT32 duration adjacent to sentinel remains exact");
bus.receive(81, std::vector<uint8_t>{0xff, 0xff, 0xff, 0xff});
check(id(next_fan_change_in).state == "n/a", "all-ones duration sentinel means no scheduled change");
bus.receive(82, std::vector<uint8_t>{1, 2, 3});
check(id(next_bypass_change_in).state == "49710d6h28m14s", "malformed duration does not replace real state");
check(bus.sent.empty(), "duration PDO feedback is read-only");

for (const auto &entry : std::vector<std::pair<std::vector<uint8_t>, float>>{
         {{0, 0}, 0}, {{0x2c, 0x01}, 300}, {{0xff, 0xff}, 65535}}) {
  bus.receive(226, entry.first);
  check(id(modulated_fan_speed).state == entry.second, "modulated speed is independent UINT16 diagnostic state");
}
bus.receive(226, std::vector<uint8_t>{1});
check(id(modulated_fan_speed).state == 65535 && bus.sent.empty(),
      "malformed modulated speed cannot change state or send commands");

for (const auto &entry : std::vector<std::pair<std::vector<uint8_t>, std::string>>{
         {{0, 0, 0, 0, 0, 0, 0, 0}, "0x0000000000000000: unavailable"},
         {{0, 0, 0, 0, 0, 0x20, 0, 0}, "0x0000200000000000: None"},
         {{0, 2, 0, 0, 0, 0x20, 0, 0}, "0x0000200000000200: FrostProtection"},
         {{0xa0, 0, 0, 0, 0, 0x20, 0x40, 0}, "0x00402000000000a0: NoiseGuard, CO2ZoneX8"},
         {{0, 0, 0, 0, 0, 0x20, 0, 0x80}, "0x8000200000000000: Unknown(bit 63)"}}) {
  bus.receive(230, entry.first);
  check(id(airflow_constraints).state == entry.second,
        "constraints retain all bits and interpret validity, grouped reasons and high/unknown bits");
}
bus.receive(230, std::vector<uint8_t>{1});
check(id(airflow_constraints).state == "0x8000200000000000: Unknown(bit 63)" && bus.sent.empty(),
      "malformed constraints preserve state and feedback sends no commands");

for (const auto &entry : std::vector<std::pair<float, std::vector<uint8_t>>>{
         {300, {0x03, 0x1c, 0x01, 0x03, 0x2c, 0x01}},
         {0, {0x03, 0x1c, 0x01, 0x03, 0, 0}},
         {65535, {0x03, 0x1c, 0x01, 0x03, 0xff, 0xff}}}) {
  bus.sent.clear();
  auto call = id(filter_lifetime_requested).make_call();
  call.set_value(entry.first);
  call.perform();
  check(bus.sent.size() == 1 &&
        std::vector<uint8_t>(bus.sent[0].data, bus.sent[0].data + bus.sent[0].can_data_length_code) == entry.second &&
        id(filter_lifetime_requested).state == entry.first,
        "explicit filter lifetime uses exact little-endian UINT16 property bytes");
}
for (float invalid : {-1.0f, 300.5f, 65536.0f, NAN, INFINITY}) {
  bus.sent.clear();
  auto call = id(filter_lifetime_requested).make_call();
  call.set_value(invalid);
  call.perform();
  check(bus.sent.empty() && id(filter_lifetime_requested).state == 65535,
        "invalid lifetime does not send a command or replace requested state");
  check(!id(comfoair).set_filter_lifetime(invalid) && bus.sent.empty(),
        "direct lifetime API also validates values at its boundary");
}
id(comfoair).set_temperature_passive(zehnder_comfoair_q::OAO_ON);
check(bus.sent.size() == 1 &&
      std::vector<uint8_t>(bus.sent[0].data, bus.sent[0].data + bus.sent[0].can_data_length_code) ==
          std::vector<uint8_t>({0x03, 0x1d, 0x01, 0x04, 0x02}),
      "existing UINT8 property writes remain unchanged");
bus.sent.clear();

for (const auto &entry : std::vector<std::pair<std::string, uint8_t>>{{"Adaptive", 0}, {"Fixed", 1}}) {
  bus.sent.clear();
  auto call = id(temperature_profile_mode_requested).make_call();
  call.set_option(entry.first);
  call.perform();
  check(bus.sent.size() == 1 &&
        std::vector<uint8_t>(bus.sent[0].data, bus.sent[0].data + bus.sent[0].can_data_length_code) ==
            std::vector<uint8_t>({0x03, 0x1d, 0x01, 0x08, entry.second}) &&
        id(temperature_profile_mode_requested).current_option() == entry.first,
        "profile mode writes its UINT8 property, not a Normal/Cool/Warm timer");
}
bus.sent.clear();
auto invalid_mode = id(temperature_profile_mode_requested).make_call();
invalid_mode.set_option("External setpoint");
invalid_mode.perform();
check(bus.sent.empty(), "unverified external-setpoint mode is not exposed");

// The reporter's polling workaround: no lambda means update is a no-op.
id(temperature_profile).update();
check(!id(temperature_profile).has_state(), "polling must not invent a temperature profile");
check(bus.sent.empty(), "polling must not send profile commands");

bus.receive(67, 2);
check(id(temperature_profile).state == "Warm", "profile PDO updates the sensor");
check(id(temperature_profile_select).current_option() == "Warm", "profile PDO updates the select");
check(id(temperature_profile_mode_requested).current_option() == "Fixed",
      "Normal/Cool/Warm feedback is independent of requested profile mode");
check(bus.sent.empty(), "profile feedback must not write to CAN");
id(temperature_profile).update();
check(id(temperature_profile).state == "Warm" && bus.sent.empty(),
      "polling after a PDO must preserve the device's profile");

bus.receive(65, 1);
check(id(comfoair_fan).state && id(comfoair_fan).speed == 1 && bus.sent.empty(),
      "first nonzero fan PDO before any mode PDO must update without sending commands");

// Exercise both PDO orders and all fan levels, including ON/OFF transitions.
for (bool mode_first : {false, true}) {
  for (uint8_t level : {uint8_t(0), uint8_t(1), uint8_t(3), uint8_t(2), uint8_t(0)}) {
    bus.sent.clear();
    if (mode_first)
      bus.receive(49, 1);  // Manual (limited)
    bus.receive(65, level);
    if (!mode_first)
      bus.receive(49, 1);
    check(id(comfoair_fan).state == (level != 0), "fan PDO updates ON/OFF state");
    if (level != 0)
      check(id(comfoair_fan).speed == level, "fan PDO updates speed");
    check(bus.sent.empty(), "fan/mode feedback must not change the unit's mode or level");
  }
}

// Feedback suppression must not suppress genuine user controls.
bus.sent.clear();
auto profile_call = id(temperature_profile_select).make_call();
profile_call.set_option("Cool");
profile_call.perform();
check(bus.sent.size() == 2 && bus.sent[0].data[1] == 0x84 &&
      bus.sent[0].data[3] == 0x03 && bus.sent[1].data[6] == 1,
      "user profile selection sends the Cool timer command");

bus.sent.clear();
auto level_call = id(fan_level_select).make_call();
level_call.set_option("High");
level_call.perform();
check(bus.sent.size() == 2 && bus.sent[0].data[3] == 0x01 &&
      bus.sent[1].data[6] == 3, "user fan selection sends the High timer command");

bus.sent.clear();
id(auto_ventilation).turn_off();
check(bus.sent.size() == 2 && bus.sent[0].data[3] == 0x08,
      "user manual-mode switch still sends a command");
bus.sent.clear();
id(away_mode).turn_on();
check(bus.sent.size() == 2 && bus.sent[0].data[4] == 0x0b,
      "user Away switch still sends a command");

bus.receive(49, 255);  // Auto: no delayed manual-limited workaround in this case.
bus.sent.clear();
auto fan_call = id(comfoair_fan).turn_on();
fan_call.set_speed(1);
fan_call.perform();
check(bus.sent.size() == 4 && bus.sent[0].data[0] == 0x85 &&
      bus.sent[0].data[2] == 0x08 && bus.sent[1].data[2] == 0x01 && bus.sent[3].data[6] == 1,
      "user fan ON/Low still sends Auto and Low commands");
bus.receive(65, 1);
bus.sent.clear();
id(auto_ventilation).turn_on();
check(bus.sent.size() == 2 &&
      std::vector<uint8_t>(bus.sent[0].data, bus.sent[0].data + bus.sent[0].can_data_length_code) == std::vector<uint8_t>({0x85, 0x15, 0x08, 0x01}) &&
      std::vector<uint8_t>(bus.sent[1].data, bus.sent[1].data + bus.sent[1].can_data_length_code) == std::vector<uint8_t>({0x85, 0x15, 0x01, 0x01}),
      "explicit Auto cancels manual mode then the temporary speed override");
bus.sent.clear();
id(comfoair_fan).turn_off().perform();
check(bus.sent.size() == 2 && bus.sent[1].data[6] == 0,
      "user fan OFF still sends Away level");

bus.sent.clear();
