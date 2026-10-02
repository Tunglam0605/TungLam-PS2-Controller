/**
 * @file JoystickDirections.ino
 * @brief Test hướng cuối cùng của hai joystick mà không spam Serial.
 *
 * Chỉ in khi hướng LEFT hoặc RIGHT joystick thực sự thay đổi.
 * Không có delay() và không map analog trực tiếp thành tốc độ robot.
 *
 * Lưu ý: thư viện chỉ trả về trạng thái joystick. Việc UP nghĩa là tiến,
 * RIGHT nghĩa là xoay hay điều khiển một cơ cấu nào khác hoàn toàn do project
 * quyết định. Ví dụ lái đế robot nằm ở repo TungLam_OmniMecanum_4WD.
 */

#include <TungLam_PS2.h>

TungLamPS2 ps2;
constexpr uint8_t PS2_CS_PIN = 10;

PS2StickDirection lastLeft = PS2StickDirection::Unknown;
PS2StickDirection lastRight = PS2StickDirection::Unknown;
bool firstPrint = true;

const __FlashStringHelper* directionName(PS2StickDirection direction) {
  switch (direction) {
    case PS2StickDirection::Center: return F("CENTER");
    case PS2StickDirection::Up: return F("UP");
    case PS2StickDirection::Down: return F("DOWN");
    case PS2StickDirection::Left: return F("LEFT");
    case PS2StickDirection::Right: return F("RIGHT");
    default: return F("UNKNOWN");
  }
}

void setup() {
  Serial.begin(115200);
  ps2.begin(PS2_CS_PIN);

  Serial.println(F("[TEST] PS2 joystick directions"));
}

void loop() {
  ps2.update();

  if (!ps2.connected()) {
    return;
  }

  const PS2StickDirection left = ps2.leftDirection();
  const PS2StickDirection right = ps2.rightDirection();

  // Chỉ in khi state thay đổi -> Serial Monitor rất dễ đọc.
  if (firstPrint || left != lastLeft || right != lastRight) {
    Serial.print(F("[JOY] LEFT="));
    Serial.print(directionName(left));
    Serial.print(F("("));
    Serial.print(ps2.leftX());
    Serial.print(F(","));
    Serial.print(ps2.leftY());
    Serial.print(F(") | RIGHT="));
    Serial.print(directionName(right));
    Serial.print(F("("));
    Serial.print(ps2.rightX());
    Serial.print(F(","));
    Serial.print(ps2.rightY());
    Serial.println(F(")"));

    firstPrint = false;
    lastLeft = left;
    lastRight = right;
  }

  // Ví dụ ứng dụng:
  // if (left == PS2StickDirection::Up) {
  //   robot.forward(180);
  // }
}
