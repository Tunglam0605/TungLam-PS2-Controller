/**
 * @file DebugMonitor.ino
 * @brief Debug toàn bộ tay cầm theo event, mỗi thay đổi chỉ một dòng.
 *
 * CÁCH DÙNG
 * --------------------------------------------------------------------------
 * 1. Đặt PS2_DEBUG_ENABLED = 1.
 * 2. Mở Serial Monitor 115200 baud.
 * 3. Nhấn nút / đẩy joystick / rút receiver để xem event.
 *
 * Ví dụ:
 *   [PS2] BTN=CROSS:PRESSED | LEFT=UP(1,-117)
 *   [PS2] BTN=CROSS:RELEASED | LEFT=CENTER(0,2)
 *   [PS2] LINK=RECOVERING | ERR=1
 *
 * Khi tay cầm đứng yên -> không in thêm dòng.
 *
 * BUILD PRODUCTION
 * --------------------------------------------------------------------------
 * Đổi PS2_DEBUG_ENABLED thành 0. Khi đó sketch không gọi debug path.
 * Core ps2.update() bản thân không tự Serial.print().
 */

#include <TungLam_PS2.h>

#define PS2_DEBUG_ENABLED 1

TungLamPS2 ps2;
constexpr uint8_t PS2_CS_PIN = 10;

void setup() {
#if PS2_DEBUG_ENABLED
  Serial.begin(115200);
#endif

  ps2.begin(PS2_CS_PIN);
}

void loop() {
  ps2.update();

#if PS2_DEBUG_ENABLED
  ps2.debug(Serial);
#endif

  // Robot/control logic vẫn đặt ở đây và không cần delay().
}
