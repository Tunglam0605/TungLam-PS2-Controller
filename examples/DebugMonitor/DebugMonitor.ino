#include <TungLam_PS2.h>

// Đổi thành 0 khi build production.
// Khi = 0, sketch không gọi Serial debug path.
#define PS2_DEBUG_ENABLED 1

TungLamPS2 ps2;

void setup() {
#if PS2_DEBUG_ENABLED
  Serial.begin(115200);
#endif

  ps2.begin(53);
}

void loop() {
  ps2.update();

#if PS2_DEBUG_ENABLED
  // Không spam Serial:
  // chỉ in khi status, button, joystick direction,
  // error hoặc reconnect thay đổi.
  ps2.debug(Serial);
#endif

  // Phần điều khiển robot vẫn chạy bình thường ở đây.
}
