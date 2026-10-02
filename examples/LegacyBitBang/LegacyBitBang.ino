#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  Serial.begin(115200);

  // Wiring RoboBall cũ:
  // PS2 CLK/SCK  -> D22
  // PS2 CMD/MOSI -> D26
  // PS2 CS/SEL   -> D24
  // PS2 DAT/MISO -> D28
  // PS2 GND      -> GND
  // PS2 VCC      -> nguồn đúng theo module receiver
  ps2.beginBitBang(22, 26, 24, 28);
}

void loop() {
  ps2.update();

  if (ps2.leftDirection() == PS2StickDirection::Up) {
    // Tự gán hành vi robot ở đây.
  }
}
