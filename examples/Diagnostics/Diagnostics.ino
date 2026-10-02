/**
 * @file Diagnostics.ino
 * @brief Một sketch duy nhất để test kết nối, nút, joystick, raw data và reconnect.
 *
 * Dùng sketch này khi lắp tay PS2 lần đầu hoặc khi cần debug phần cứng.
 */

#include <TungLam_PS2.h>

TungLamPS2 ps2;

constexpr uint8_t PS2_CS_PIN = 53;
constexpr bool PRINT_SNAPSHOT_10HZ = false;

bool lastConnected = false;
bool firstLinkState = true;
unsigned long lastSnapshotMs = 0;

void setup() {
  Serial.begin(115200);
  ps2.begin(PS2_CS_PIN);
  ps2.setPollRate(PS2PollRate::Hz50);
}

void loop() {
  ps2.update();

  // Chỉ in khi button/joystick/link/error thay đổi.
  ps2.debug(Serial);

  const bool connected = ps2.connected();
  if (firstLinkState || connected != lastConnected) {
    Serial.print(F("[LINK] "));
    Serial.println(connected ? F("CONNECTED") : F("DISCONNECTED/RECOVERING"));
    firstLinkState = false;
    lastConnected = connected;
  }

  // Bật khi cần xem snapshot raw + filtered ở 10 Hz.
  if (PRINT_SNAPSHOT_10HZ && millis() - lastSnapshotMs >= 100) {
    lastSnapshotMs = millis();
    ps2.printState(Serial);
  }
}
