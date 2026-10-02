#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  Serial.begin(115200);

  // Hardware SPI mặc định của board.
  //
  // Arduino Mega 2560:
  //   PS2 DAT/MISO -> D50
  //   PS2 CMD/MOSI -> D51
  //   PS2 CLK/SCK  -> D52
  //   PS2 CS/ATT   -> D10 trong ví dụ này
  //   PS2 GND      -> GND
  //   PS2 VCC      -> nguồn đúng theo module receiver
  //
  // Với board khác, nối DAT/CMD/CLK vào MISO/MOSI/SCK
  // của SPI mặc định của board.
  if (!ps2.begin(10)) {
    Serial.println("PS2 not found");
  }

  // Mặc định đã là 50 Hz.
  // Có thể chọn 20/50/100 Hz nếu muốn:
  ps2.setPollRate(PS2PollRate::Hz50);
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
