/**
 * @file CustomSPI.ino
 * @brief Chọn trực tiếp SPI bus khi board có nhiều SPI peripheral.
 *
 * Cách gọi:
 *   ps2.begin(SPI, 10);
 *   ps2.begin(SPI1, 7);   // nếu Arduino Core của board có SPI1
 *   ps2.begin(SPI2, 7);   // nếu có SPI2
 *
 * Quy tắc đấu dây:
 *   DAT/MISO -> MISO của chính bus đã truyền vào
 *   CMD/MOSI -> MOSI của chính bus đã truyền vào
 *   CLK/SCK  -> SCK của chính bus đã truyền vào
 *   CS/ATT   -> csPin
 *
 * Library không hard-code concrete SPI class nên API có thể dùng trên
 * nhiều Arduino Core khác nhau.
 */

#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  // Ví dụ portable: dùng object SPI mặc định nhưng gọi overload có bus.
  ps2.begin(SPI, 10);

  // Ví dụ khi board hỗ trợ:
  // ps2.begin(SPI1, 7);
}

void loop() {
  ps2.update();
}
