#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  Serial.begin(115200);

  // Hardware SPI mặc định của board.
  // MISO/MOSI/SCK do Arduino Core chọn, người dùng chỉ chọn CS.
  if (!ps2.begin(10)) {
    Serial.println("PS2 not found");
  }
}

void loop() {
  ps2.update();

  if (!ps2.connected()) {
    return;
  }

  // 1) Đang giữ.
  if (ps2.button(PS2Button::R1)) {
    Serial.println("R1 held");
  }

  // 2) Vừa nhấn.
  if (ps2.pressed(PS2Button::Cross)) {
    Serial.println("Cross pressed");
  }

  // 3) Vừa nhả.
  if (ps2.released(PS2Button::Square)) {
    Serial.println("Square released");
  }
}
