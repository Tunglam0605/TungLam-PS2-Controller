#pragma once

#include <stdint.h>

/**
 * Trạng thái rời rạc của một joystick sau khi đã lọc.
 */
enum class PS2StickDirection : uint8_t {
  Unknown = 0,
  Center,
  Up,
  Down,
  Left,
  Right
};

/**
 * Cấu hình bộ lọc/phân vùng joystick.
 *
 * - deadzone: vùng tâm.
 * - enterThreshold: ngưỡng để xác nhận một hướng mới.
 * - exitThreshold: ngưỡng hysteresis để giữ hướng hiện tại.
 * - perpendicularLimit: độ lệch tối đa của trục vuông góc.
 * - stableSamples: số mẫu liên tiếp để xác nhận hướng mới.
 */
struct PS2StickFilterConfig {
  uint8_t deadzone = 18;
  uint8_t enterThreshold = 100;
  uint8_t exitThreshold = 75;
  uint8_t perpendicularLimit = 35;
  uint8_t stableSamples = 2;
};

class TungLamPS2StickFilter {
 public:
  TungLamPS2StickFilter();

  void reset();
  void setConfig(const PS2StickFilterConfig& config);
  const PS2StickFilterConfig& config() const;

  void setCenter(uint8_t centerX, uint8_t centerY);
  uint8_t centerX() const;
  uint8_t centerY() const;

  void push(uint8_t rawX, uint8_t rawY);

  // Giá trị frame gốc gần nhất từ controller.
  uint8_t rawX() const;
  uint8_t rawY() const;

  // Giá trị đã median-filter và trừ tâm.
  int16_t x() const;
  int16_t y() const;

  PS2StickDirection direction() const;

 private:
  static uint8_t median3(uint8_t a, uint8_t b, uint8_t c);
  static int16_t abs16(int16_t value);

  PS2StickDirection classify(int16_t x, int16_t y) const;
  bool keepCurrentDirection(int16_t x, int16_t y) const;

  PS2StickFilterConfig config_;

  uint8_t centerX_;
  uint8_t centerY_;

  uint8_t inputX_;
  uint8_t inputY_;

  uint8_t historyX_[3];
  uint8_t historyY_[3];
  uint8_t historyCount_;
  uint8_t historyIndex_;

  uint8_t filteredX_;
  uint8_t filteredY_;

  PS2StickDirection direction_;
  PS2StickDirection candidate_;
  uint8_t candidateCount_;
};
