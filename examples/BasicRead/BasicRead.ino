#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  // Ví dụ Arduino Mega 2560 dùng hardware SPI mặc định:
  // DAT/MISO -> D50
  // CMD/MOSI -> D51
  // CLK/SCK  -> D52
  // CS/ATT   -> D53
  ps2.begin(53);

  // 50 Hz là mặc định.
  // ps2.setPollRate(PS2PollRate::Hz20);
  // ps2.setPollRate(PS2PollRate::Hz100);
}

void loop() {
  ps2.update();

  if (!ps2.connected()) {
    // Fail-safe của project nếu cần.
    return;
  }

  // Đọc joystick đã lọc.
  const PS2StickDirection left = ps2.leftDirection();
  const PS2StickDirection right = ps2.rightDirection();

  // Đọc nút đang giữ.
  if (ps2.button(PS2Button::R1)) {
    // liftUp();
  }

  // Event chỉ xuất hiện trong đúng vòng update() nhận frame mới.
  if (ps2.pressed(PS2Button::Cross)) {
    // toggleGripper();
  }

  if (ps2.released(PS2Button::Circle)) {
    // stopMechanism();
  }

  // Ví dụ dùng hướng joystick.
  if (left == PS2StickDirection::Up) {
    // robot.forward(180);
  }

  if (right == PS2StickDirection::Right) {
    // robot.rotateRight(140);
  }
}
