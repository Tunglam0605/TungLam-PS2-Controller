#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  Serial.begin(115200);

  // Cách tổng quát: truyền trực tiếp SPI bus.
  //
  // Với ps2.begin(SPI, 10):
  //   DAT/MISO -> MISO của SPI
  //   CMD/MOSI -> MOSI của SPI
  //   CLK/SCK  -> SCK của SPI
  //   CS/ATT   -> D10
  ps2.begin(SPI, 10);

  // Trên board/core có SPI1 hoặc SPI2:
  // ps2.begin(SPI1, 7);
  //
  // Khi đó DAT/CMD/CLK phải nối vào MISO/MOSI/SCK của SPI1,
  // còn CS/ATT nối vào D7.
}

void loop() {
  ps2.update();
}
