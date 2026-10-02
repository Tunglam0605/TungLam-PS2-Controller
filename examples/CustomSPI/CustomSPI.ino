#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  Serial.begin(115200);

  // Cách tổng quát: truyền trực tiếp SPI bus.
  ps2.begin(SPI, 10);

  // Trên board/core có SPI1 hoặc SPI2:
  // ps2.begin(SPI1, 7);
  // ps2.begin(SPI2, 7);
}

void loop() {
  ps2.update();
}
