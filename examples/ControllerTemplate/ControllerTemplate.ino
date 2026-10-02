/**
 * @file ControllerTemplate.ino
 * @brief Template chính để copy vào project rồi điền chức năng vào các hàm TODO.
 *
 * Luồng chuẩn:
 *   ps2.update()
 *      -> kiểm tra kết nối
 *      -> handleLeftStick()
 *      -> handleRightStick()
 *      -> handleButtons()
 *
 * Không delay(), không Serial trong đường chạy production.
 */

#include <TungLam_PS2.h>

TungLamPS2 ps2;

constexpr uint8_t PS2_CS_PIN = 53;  // Mega 2560: DAT=50, CMD=51, CLK=52, CS=53

void handleLeftStick();
void handleRightStick();
void handleButtons();
void onControllerLost();

void onLeftUp();
void onLeftDown();
void onLeftLeft();
void onLeftRight();
void onLeftIdle();

void onRightLeft();
void onRightRight();
void onRightIdle();

void onCrossPressed();
void onCirclePressed();
void onR1Held();
void onR1Released();

void setup() {
  ps2.begin(PS2_CS_PIN);
  ps2.setPollRate(PS2PollRate::Hz50);
}

void loop() {
  ps2.update();

  if (!ps2.connected()) {
    onControllerLost();
    return;
  }

  handleLeftStick();
  handleRightStick();
  handleButtons();
}

void handleLeftStick() {
  switch (ps2.leftDirection()) {
    case PS2StickDirection::Up:    onLeftUp();    break;
    case PS2StickDirection::Down:  onLeftDown();  break;
    case PS2StickDirection::Left:  onLeftLeft();  break;
    case PS2StickDirection::Right: onLeftRight(); break;
    case PS2StickDirection::Center:
    case PS2StickDirection::Unknown:
    default:
      onLeftIdle();
      break;
  }
}

void handleRightStick() {
  switch (ps2.rightDirection()) {
    case PS2StickDirection::Left:  onRightLeft();  break;
    case PS2StickDirection::Right: onRightRight(); break;
    case PS2StickDirection::Center:
    case PS2StickDirection::Up:
    case PS2StickDirection::Down:
    case PS2StickDirection::Unknown:
    default:
      onRightIdle();
      break;
  }
}

void handleButtons() {
  if (ps2.pressed(PS2Button::Cross)) {
    onCrossPressed();
  }

  if (ps2.pressed(PS2Button::Circle)) {
    onCirclePressed();
  }

  if (ps2.button(PS2Button::R1)) {
    onR1Held();
  }

  if (ps2.released(PS2Button::R1)) {
    onR1Released();
  }

  // Thêm các nút khác theo đúng mẫu trên khi project cần.
}

// ============================================================================
// USER FUNCTIONS - chỉ cần điền chức năng thật của project vào đây.
// ============================================================================

void onControllerLost() {
  // TODO: đưa robot/cơ cấu về trạng thái an toàn.
}

void onLeftUp() {
  // TODO: ví dụ robot.forward(...);
}

void onLeftDown() {
  // TODO: ví dụ robot.backward(...);
}

void onLeftLeft() {
  // TODO: ví dụ robot.strafeLeft(...);
}

void onLeftRight() {
  // TODO: ví dụ robot.strafeRight(...);
}

void onLeftIdle() {
  // TODO: ví dụ robot.stop();
}

void onRightLeft() {
  // TODO: ví dụ robot.rotateLeft(...);
}

void onRightRight() {
  // TODO: ví dụ robot.rotateRight(...);
}

void onRightIdle() {
  // TODO: nếu joystick phải không dùng thì có thể để trống.
}

void onCrossPressed() {
  // TODO: chức năng one-shot khi vừa nhấn CROSS.
}

void onCirclePressed() {
  // TODO: chức năng one-shot khi vừa nhấn CIRCLE.
}

void onR1Held() {
  // TODO: chức năng chạy liên tục khi đang giữ R1.
}

void onR1Released() {
  // TODO: chức năng one-shot khi vừa nhả R1.
}
