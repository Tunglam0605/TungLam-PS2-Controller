/**
 * @file LegacyBitBang.ino
 * @brief Dùng PS2 với 4 chân GPIO tùy ý, không phụ thuộc hardware SPI pin.
 *
 * Phù hợp khi:
 * - PCB/robot cũ đã đấu PS2 vào các chân khác SPI;
 * - muốn giữ nguyên wiring RoboBall cũ;
 * - hardware SPI đang dành cho thiết bị khác.
 *
 * Wiring RoboBall cũ:
 *   PS2 CLK/SCK  -> D22
 *   PS2 CMD/MOSI -> D26
 *   PS2 CS/SEL   -> D24
 *   PS2 DAT/MISO -> D28
 *   PS2 GND      -> GND
 *   PS2 VCC      -> nguồn đúng theo receiver
 *
 * Public API phía trên không đổi: update(), button(), pressed(),
 * leftDirection()... dùng y hệt hardware SPI.
 */

#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  ps2.beginBitBang(
      22,  // CLK
      26,  // CMD / MOSI
      24,  // CS / ATT
      28   // DAT / MISO
  );
}

void loop() {
  ps2.update();

  if (!ps2.connected()) {
    return;
  }

  if (ps2.leftDirection() == PS2StickDirection::Up) {
    // robot.forward(...);
  }
}
