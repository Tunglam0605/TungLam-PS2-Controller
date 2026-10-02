/**
 * @file ConnectionModes.ino
 * @brief Chọn một trong ba cách kết nối: SPI mặc định, SPI bus chỉ định hoặc BitBang.
 *
 * Chỉ đổi PS2_CONNECTION_MODE và phần chân/bus tương ứng.
 */

#include <SPI.h>
#include <TungLam_PS2.h>

TungLamPS2 ps2;

#define PS2_CONNECTION_MODE 0
// 0 = Hardware SPI mặc định
// 1 = Chỉ định SPI bus
// 2 = BitBang tùy chân

constexpr uint8_t PS2_CS_PIN = 53;

constexpr uint8_t BB_CLK_PIN = 22;
constexpr uint8_t BB_CMD_PIN = 26;
constexpr uint8_t BB_CS_PIN  = 24;
constexpr uint8_t BB_DAT_PIN = 28;

void setup() {
#if PS2_CONNECTION_MODE == 0
  ps2.begin(PS2_CS_PIN);

#elif PS2_CONNECTION_MODE == 1
  // Đổi SPI thành SPI1/SPI2 nếu board của bạn có bus đó.
  ps2.begin(SPI, PS2_CS_PIN);

#elif PS2_CONNECTION_MODE == 2
  ps2.beginBitBang(BB_CLK_PIN, BB_CMD_PIN, BB_CS_PIN, BB_DAT_PIN);

#else
  #error "PS2_CONNECTION_MODE must be 0, 1 or 2"
#endif

  ps2.setPollRate(PS2PollRate::Hz50);
}

void loop() {
  ps2.update();

  if (!ps2.connected()) {
    // TODO: fail-safe của project.
    return;
  }

  // TODO: đọc joystick/button như trong ControllerTemplate.
}
