#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  Serial.begin(115200);

  // Wiring RoboBall cũ:
  // CLK = D22
  // CMD = D26
  // CS/SEL = D24
  // DAT = D28
  ps2.beginBitBang(22, 26, 24, 28);
}

void loop() {
  ps2.update();

  if (ps2.leftDirection() == PS2StickDirection::Up) {
    // Tự gán hành vi robot ở đây.
  }
}
