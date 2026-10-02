/**
 * @file ButtonEvents.ino
 * @brief Test 3 cách đọc nút: giữ, vừa nhấn, vừa nhả.
 *
 * Serial chỉ in khi có PRESSED/RELEASED event.
 * Không in liên tục trạng thái HOLD để tránh spam.
 */

#include <TungLam_PS2.h>

TungLamPS2 ps2;
constexpr uint8_t PS2_CS_PIN = 10;

void setup() {
  Serial.begin(115200);
  ps2.begin(PS2_CS_PIN);

  Serial.println(F("[TEST] PS2 button events"));
  Serial.println(F("[TEST] Thu nhan/giu/tha CROSS va R1"));
}

void loop() {
  ps2.update();

  if (!ps2.connected()) {
    return;
  }

  // button(): dùng trong logic production khi cần hành động liên tục.
  // Ví dụ giữ R1 để nâng cơ cấu.
  if (ps2.button(PS2Button::R1)) {
    // liftUp();
  }

  // pressed(): chỉ true đúng một vòng loop sau frame phát hiện cạnh nhấn.
  if (ps2.pressed(PS2Button::Cross)) {
    Serial.println(F("[BUTTON] CROSS:PRESSED"));
  }

  // released(): chỉ true đúng một vòng loop sau frame phát hiện cạnh nhả.
  if (ps2.released(PS2Button::Cross)) {
    Serial.println(F("[BUTTON] CROSS:RELEASED"));
  }

  if (ps2.pressed(PS2Button::R1)) {
    Serial.println(F("[BUTTON] R1:PRESSED"));
  }

  if (ps2.released(PS2Button::R1)) {
    Serial.println(F("[BUTTON] R1:RELEASED"));
  }
}
