/**
 * @file RawAnalogTest.ino
 * @brief Xem raw analog + filtered analog + counters để tune joystick.
 *
 * printState() luôn in một snapshot đầy đủ khi được gọi, vì vậy ví dụ này
 * chủ động giới hạn 10 dòng/giây. Đây là chế độ SERVICE/DEBUG, không phải
 * cách dùng production.
 *
 * Output điển hình:
 * [PS2][STATE] LINK=CONNECTED_ANALOG BUTTONS=0x0
 * LEFT=CENTER(1,-2) RAW(129,126) RIGHT=CENTER(0,1) RAW(128,129)
 * PACKETS=123 ERRORS=0 RECONNECTS=0
 */

#include <TungLam_PS2.h>

TungLamPS2 ps2;
constexpr uint8_t PS2_CS_PIN = 10;

unsigned long lastPrintMs = 0;

void setup() {
  Serial.begin(115200);
  ps2.begin(PS2_CS_PIN);

  Serial.println(F("[TEST] PS2 raw/filtered analog"));
  Serial.println(F("[TEST] Di chuyen 2 joystick cham qua cac huong"));
}

void loop() {
  ps2.update();

  const unsigned long now = millis();

  // Chỉ in 10 Hz để Serial không chiếm quá nhiều CPU/thời gian truyền.
  if (static_cast<unsigned long>(now - lastPrintMs) >= 100UL) {
    lastPrintMs = now;
    ps2.printState(Serial);
  }
}
