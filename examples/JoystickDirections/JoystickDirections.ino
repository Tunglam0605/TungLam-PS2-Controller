#include <TungLam_PS2.h>

TungLamPS2 ps2;

const char* directionName(PS2StickDirection direction) {
  switch (direction) {
    case PS2StickDirection::Center: return "CENTER";
    case PS2StickDirection::Up: return "UP";
    case PS2StickDirection::Down: return "DOWN";
    case PS2StickDirection::Left: return "LEFT";
    case PS2StickDirection::Right: return "RIGHT";
    default: return "UNKNOWN";
  }
}

void setup() {
  Serial.begin(115200);
  ps2.begin(10);
}

void loop() {
  ps2.update();

  if (!ps2.connected()) {
    return;
  }

  Serial.print("Left: ");
  Serial.print(directionName(ps2.leftDirection()));

  Serial.print("  Right: ");
  Serial.println(directionName(ps2.rightDirection()));

  // Thư viện KHÔNG map joystick thành tốc độ.
  // Người lập trình tự quyết định hành vi.
  if (ps2.leftDirection() == PS2StickDirection::Up) {
    // robot.forward(180);
  }

  delay(50);
}
