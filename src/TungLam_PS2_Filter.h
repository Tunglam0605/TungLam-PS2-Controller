#pragma once

#include <stdint.h>

/**
 * @brief Trạng thái rời rạc của một joystick sau toàn bộ pipeline lọc.
 *
 * Library cố tình có Unknown để fail-safe ở vùng chéo/mơ hồ/chuyển tiếp,
 * thay vì giữ lại hướng cũ có thể làm robot tiếp tục chạy ngoài ý muốn.
 */
enum class PS2StickDirection : uint8_t {
  Unknown = 0,  ///< Không đủ chắc chắn để kết luận một hướng.
  Center,       ///< Joystick nằm trong vùng tâm/deadzone.
  Up,           ///< Đẩy lên.
  Down,         ///< Kéo xuống.
  Left,         ///< Đẩy sang trái.
  Right         ///< Đẩy sang phải.
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
  uint8_t deadzone = 18;            ///< |X| và |Y| <= deadzone => Center.
  uint8_t enterThreshold = 100;     ///< Biên độ tối thiểu để vào một hướng mới.
  uint8_t exitThreshold = 75;       ///< Ngưỡng thấp hơn để hysteresis giữ hướng.
  uint8_t perpendicularLimit = 35;  ///< Sai lệch trục vuông góc tối đa.
  uint8_t stableSamples = 2;        ///< Số mẫu ổn định liên tiếp trước khi nhận hướng.
};

class TungLamPS2StickFilter {
 public:
  /** @brief Tạo filter với tâm 128/128 và cấu hình mặc định an toàn. */
  TungLamPS2StickFilter();

  /** @brief Xóa lịch sử mẫu và đưa state về Center tại tâm hiện tại. */
  void reset();

  /** @brief Ghi cấu hình lọc mới; stableSamples=0 tự được sửa thành 1. */
  void setConfig(const PS2StickFilterConfig& config);

  /** @brief Đọc cấu hình lọc đang dùng. */
  const PS2StickFilterConfig& config() const;

  /** @brief Đặt tâm raw cho hai trục rồi reset lịch sử lọc. */
  void setCenter(uint8_t centerX, uint8_t centerY);

  /** @brief Đọc tâm X raw hiện tại. */
  uint8_t centerX() const;

  /** @brief Đọc tâm Y raw hiện tại. */
  uint8_t centerY() const;

  /** @brief Đưa một mẫu raw mới 0..255 vào pipeline lọc. */
  void push(uint8_t rawX, uint8_t rawY);

  /** @brief Giá trị raw X của frame gần nhất, trước median-filter. */
  uint8_t rawX() const;

  /** @brief Giá trị raw Y của frame gần nhất, trước median-filter. */
  uint8_t rawY() const;

  /** @brief X đã median-filter và trừ tâm; dương = phải. */
  int16_t x() const;

  /** @brief Y đã median-filter và trừ tâm; âm = lên, dương = xuống. */
  int16_t y() const;

  /** @brief Hướng rời rạc cuối cùng sau deadzone/hysteresis/stability. */
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
