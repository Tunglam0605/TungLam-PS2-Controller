/**
 * @file ConnectionRecovery.ino
 * @brief Test fail-safe và khả năng tự reconnect của TungLam_PS2.
 *
 * QUY TRÌNH TEST
 * --------------------------------------------------------------------------
 * 1. Cắm receiver, bật tay cầm.
 * 2. Mở Serial Monitor 115200.
 * 3. Rút receiver hoặc tắt tay cầm.
 * 4. Xác nhận connected() về false và application đi vào fail-safe.
 * 5. Cắm/bật lại tay cầm.
 * 6. Thư viện sẽ tự thử cấu hình lại; khi thành công connected() trở lại true.
 *
 * Không cần reset Arduino và không cần delay() trong loop().
 */

#include <TungLam_PS2.h>

TungLamPS2 ps2;
constexpr uint8_t PS2_CS_PIN = 10;

bool lastConnected = false;
bool firstState = true;

void setup() {
  Serial.begin(115200);
  ps2.begin(PS2_CS_PIN);
}

void loop() {
  ps2.update();

  const bool nowConnected = ps2.connected();

  // Chỉ in khi trạng thái kết nối đổi.
  if (firstState || nowConnected != lastConnected) {
    Serial.print(F("[LINK] "));
    Serial.println(nowConnected ? F("CONNECTED") : F("DISCONNECTED/RECOVERING"));

    firstState = false;
    lastConnected = nowConnected;
  }

  if (!nowConnected) {
    // Đây là nơi project thật phải fail-safe:
    // robot.stop();
    // actuatorOff();
    return;
  }

  // Khi reconnect thành công, code điều khiển tiếp tục bình thường.
}
