#pragma once

#include "esphome/components/canbus/canbus.h"
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <deque>

namespace esphome::test_canbus {

// Fake only the physical bus, not callbacks, entities, restore or automations.
class TestCanbus : public canbus::Canbus {
 public:
  std::vector<canbus::CanFrame> sent;
  std::vector<canbus::CanFrame> requests;

  void receive(uint16_t pdo, uint8_t value) { receive(pdo, std::vector<uint8_t>{value}); }

  void receive(uint16_t pdo, const std::vector<uint8_t> &data) {
    canbus::CanFrame frame{};
    frame.use_extended_id = true;
    frame.can_id = (uint32_t(pdo) << 14) | 0x41;
    frame.can_data_length_code = data.size();
    std::copy(data.begin(), data.end(), frame.data);
    incoming_.push_back(frame);
    this->loop();
  }

 protected:
  bool setup_internal() override { return true; }
  canbus::Error send_message(canbus::CanFrame *frame) override {
    if (!frame->remote_transmission_request)
      sent.push_back(*frame);
    else
      requests.push_back(*frame);
    return canbus::ERROR_OK;
  }
  canbus::Error read_message(canbus::CanFrame *frame) override {
    if (incoming_.empty())
      return canbus::ERROR_NOMSG;
    *frame = incoming_.front();
    incoming_.pop_front();
    return canbus::ERROR_OK;
  }
  std::deque<canbus::CanFrame> incoming_;
};

inline int failures = 0;
inline void check(bool ok, const char *contract) {
  std::printf("%s: %s\n", ok ? "PASS" : "FAIL", contract);
  if (!ok)
    failures++;
}

}  // namespace esphome::test_canbus
