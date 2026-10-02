/**
 * @file BasicRead.ino
 * @brief Ví dụ tối thiểu để dùng TungLam_PS2 trong project thật.
 *
 * MỤC ĐÍCH
 * --------------------------------------------------------------------------
 * - Khởi tạo đầu thu PS2 bằng hardware SPI mặc định của board.
 * - Gọi ps2.update() liên tục trong loop().
 * - Đọc joystick và button mà KHÔNG in Serial, KHÔNG delay().
 *
 * ĐẤU DÂY
 * --------------------------------------------------------------------------
 * Với Arduino Mega 2560:
 *   PS2 DAT/MISO -> D50
 *   PS2 CMD/MOSI -> D51
 *   PS2 CLK/SCK  -> D52
 *   PS2 CS/ATT   -> D10 trong ví dụ này
 *   PS2 GND      -> GND
 *   PS2 VCC      -> nguồn đúng theo module receiver
 *
 * Lưu ý:
 * - CS có thể đổi sang GPIO khác, ví dụ D53 trên Mega.
 * - MISO/MOSI/SCK phải dùng đúng SPI mặc định của board khi gọi begin(CS).
 * - Poll mặc định = 50 Hz, nhưng loop() vẫn chạy tự do.
 */

#include <TungLam_PS2.h>

TungLamPS2 ps2;

constexpr uint8_t PS2_CS_PIN = 10;

void setup() {
  // Không cần Serial.begin() nếu project không debug.
  ps2.begin(PS2_CS_PIN);

  // Mặc định đã là 50 Hz.
  // Khi cần có thể đổi:
  // ps2.setPollRate(PS2PollRate::Hz20);
  // ps2.setPollRate(PS2PollRate::Hz100);
}

void loop() {
  // Bắt buộc gọi liên tục.
  // Scheduler bên trong tự giới hạn transaction theo poll rate.
  ps2.update();

  // Mất tay cầm -> project nên đi vào trạng thái an toàn.
  if (!ps2.connected()) {
    // Ví dụ:
    // robot.stop();
    return;
  }

  // ------------------------------------------------------------------------
  // JOYSTICK
  // ------------------------------------------------------------------------
  // leftDirection()/rightDirection() đã qua:
  // median -> center compensation -> deadzone -> hysteresis -> stable samples.
  const PS2StickDirection left = ps2.leftDirection();
  const PS2StickDirection right = ps2.rightDirection();

  if (left == PS2StickDirection::Up) {
    // robot.forward(...);
  } else if (left == PS2StickDirection::Down) {
    // robot.backward(...);
  } else if (left == PS2StickDirection::Left) {
    // robot.strafeLeft(...);
  } else if (left == PS2StickDirection::Right) {
    // robot.strafeRight(...);
  }

  if (right == PS2StickDirection::Left) {
    // robot.rotateLeft(...);
  } else if (right == PS2StickDirection::Right) {
    // robot.rotateRight(...);
  }

  // ------------------------------------------------------------------------
  // BUTTON
  // ------------------------------------------------------------------------

  // 1) button(): true trong suốt thời gian đang giữ.
  if (ps2.button(PS2Button::R1)) {
    // liftUp();
  }

  // 2) pressed(): one-shot đúng lúc vừa nhấn.
  if (ps2.pressed(PS2Button::Cross)) {
    // toggleGripper();
  }

  // 3) released(): one-shot đúng lúc vừa nhả.
  if (ps2.released(PS2Button::Circle)) {
    // stopMechanism();
  }

  // Không cần delay().
}
