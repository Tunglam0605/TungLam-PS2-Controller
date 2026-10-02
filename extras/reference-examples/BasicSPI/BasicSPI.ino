/**
 * @file BasicSPI.ino
 * @brief Chỉ tập trung vào cách khởi tạo PS2 bằng SPI mặc định của board.
 *
 * Khi gọi ps2.begin(CS_PIN):
 * - DAT/MISO -> chân MISO/CIPO mặc định của board.
 * - CMD/MOSI -> chân MOSI/COPI mặc định của board.
 * - CLK/SCK  -> chân SCK mặc định của board.
 * - CS/ATT   -> GPIO do bạn chọn.
 *
 * Ví dụ Mega 2560 nếu CS=D10:
 *   DAT -> D50, CMD -> D51, CLK -> D52, CS -> D10.
 *
 * Serial trong ví dụ này chỉ in kết quả begin() MỘT LẦN.
 */

#include <TungLam_PS2.h>

TungLamPS2 ps2;
constexpr uint8_t PS2_CS_PIN = 10;

void setup() {
  Serial.begin(115200);

  const bool ready = ps2.begin(PS2_CS_PIN);

  Serial.print(F("[INIT] PS2="));
  Serial.println(ready ? F("READY") : F("NOT_READY - update() se tu recovery"));

  // 50 Hz là mặc định, không bắt buộc gọi dòng này.
  // ps2.setPollRate(PS2PollRate::Hz50);
}

void loop() {
  // Không cần delay().
  ps2.update();

  // Không Serial.print() trong loop ở ví dụ khởi tạo này.
}
