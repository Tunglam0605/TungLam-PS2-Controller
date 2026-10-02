#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  Serial.begin(115200);
  ps2.begin(53);

  Serial.println(F("Press / hold / release Cross and R1"));
}

void loop() {
  ps2.update();

  if (!ps2.connected()) {
    return;
  }

  // Giữ nút: true trong thời gian nút đang được giữ.
  if (ps2.button(PS2Button::R1)) {
    // Chỉ in ví dụ này để quan sát; production thường thay bằng hành động.
  }

  // Vừa nhấn: event một-shot.
  if (ps2.pressed(PS2Button::Cross)) {
    Serial.println(F("Cross PRESSED"));
  }

  // Vừa nhả: event một-shot.
  if (ps2.released(PS2Button::Cross)) {
    Serial.println(F("Cross RELEASED"));
  }

  if (ps2.pressed(PS2Button::R1)) {
    Serial.println(F("R1 PRESSED"));
  }

  if (ps2.released(PS2Button::R1)) {
    Serial.println(F("R1 RELEASED"));
  }
}
