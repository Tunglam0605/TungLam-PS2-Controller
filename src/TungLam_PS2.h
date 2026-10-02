#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "TungLam_PS2_Filter.h"

/**
 * Các nút chuẩn trên tay cầm PS2.
 */
enum class PS2Button : uint16_t {
  Select   = 0x0001,
  L3       = 0x0002,
  R3       = 0x0004,
  Start    = 0x0008,
  Up       = 0x0010,
  Right    = 0x0020,
  Down     = 0x0040,
  Left     = 0x0080,
  L2       = 0x0100,
  R2       = 0x0200,
  L1       = 0x0400,
  R1       = 0x0800,
  Triangle = 0x1000,
  Circle   = 0x2000,
  Cross    = 0x4000,
  Square   = 0x8000
};

enum class PS2ConnectionStatus : uint8_t {
  Disconnected = 0,
  Probing,
  Configuring,
  ConnectedDigital,
  ConnectedAnalog,
  ConnectedPressure,
  Recovering,
  NoResponse,
  ConfigRejected
};

enum class PS2TimingProfile : uint8_t {
  Compatible = 0,
  Balanced,
  Fast
};

/**
 * Driver tay cầm PS2 cho Arduino.
 *
 * Có 3 cách khởi tạo:
 *   ps2.begin(csPin);                    // SPI mặc định
 *   ps2.begin(SPI1, csPin);              // SPI bus cụ thể
 *   ps2.beginBitBang(clk, cmd, cs, dat); // pin tùy ý
 */
class TungLamPS2 {
 public:
  TungLamPS2();

  /**
   * Khởi tạo bằng hardware SPI mặc định của board.
   *
   * Người dùng chỉ chọn CS. MISO/MOSI/SCK do Arduino Core quản lý.
   */
  bool begin(uint8_t csPin) {
    return begin(SPI, csPin);
  }

  /**
   * Khởi tạo bằng một SPI bus cụ thể, ví dụ SPI, SPI1 hoặc SPI2.
   *
   * Template giúp API dùng được với nhiều Arduino Core có kiểu class SPI
   * khác nhau mà không hard-code theo từng kiến trúc.
   */
  template <typename TBus>
  bool begin(TBus& bus, uint8_t csPin) {
    transport_ = Transport::HardwareSPI;
    spiContext_ = static_cast<void*>(&bus);
    spiBegin_ = &spiBeginThunk<TBus>;
    spiBeginTransaction_ = &spiBeginTransactionThunk<TBus>;
    spiTransfer_ = &spiTransferThunk<TBus>;
    spiEndTransaction_ = &spiEndTransactionThunk<TBus>;
    csPin_ = csPin;
    return beginCommon();
  }

  /**
   * Khởi tạo bằng 4 GPIO tùy ý.
   *
   * Thứ tự: CLK, CMD/MOSI, CS/ATT, DAT/MISO.
   */
  bool beginBitBang(uint8_t clockPin,
                    uint8_t commandPin,
                    uint8_t csPin,
                    uint8_t dataPin);

  /**
   * Đọc frame mới và cập nhật toàn bộ state.
   */
  bool update();

  bool connected() const;
  PS2ConnectionStatus status() const;

  uint32_t packetCount() const;
  uint32_t errorCount() const;
  uint16_t reconnectCount() const;

  // Ba kiểu gọi nút: đang giữ / vừa nhấn / vừa nhả.
  bool button(PS2Button key) const;
  bool pressed(PS2Button key) const;
  bool released(PS2Button key) const;
  uint16_t buttons() const;

  // Joystick đã lọc và phân vùng.
  PS2StickDirection leftDirection() const;
  PS2StickDirection rightDirection() const;

  // Giá trị đã lọc, trừ tâm. Up thường có Y âm.
  int16_t leftX() const;
  int16_t leftY() const;
  int16_t rightX() const;
  int16_t rightY() const;

  // Giá trị frame gốc gần nhất 0..255.
  uint8_t leftRawX() const;
  uint8_t leftRawY() const;
  uint8_t rightRawX() const;
  uint8_t rightRawY() const;

  void setStickFilter(const PS2StickFilterConfig& config);

  void setStickCenters(uint8_t leftX,
                       uint8_t leftY,
                       uint8_t rightX,
                       uint8_t rightY);

  /**
   * Hiệu chỉnh tâm hai joystick.
   *
   * Khi gọi hàm này cần thả hai cần analog ở vị trí tâm.
   */
  bool calibrateCenter(uint8_t samples = 16,
                       uint16_t sampleIntervalMs = 8);

  void setTimingProfile(PS2TimingProfile profile);
  void setClockHz(uint32_t clockHz);
  void setByteDelayUs(uint16_t byteDelayUs);
  void setPollIntervalMs(uint16_t pollIntervalMs);

 private:
  enum class Transport : uint8_t {
    None = 0,
    HardwareSPI,
    BitBang
  };

  typedef void (*SpiBeginFn)(void*);
  typedef void (*SpiBeginTransactionFn)(void*, uint32_t);
  typedef uint8_t (*SpiTransferFn)(void*, uint8_t);
  typedef void (*SpiEndTransactionFn)(void*);

  template <typename TBus>
  static void spiBeginThunk(void* context) {
    static_cast<TBus*>(context)->begin();
  }

  template <typename TBus>
  static void spiBeginTransactionThunk(void* context,
                                       uint32_t clockHz) {
    static_cast<TBus*>(context)->beginTransaction(
        SPISettings(clockHz, LSBFIRST, SPI_MODE3));
  }

  template <typename TBus>
  static uint8_t spiTransferThunk(void* context, uint8_t value) {
    return static_cast<TBus*>(context)->transfer(value);
  }

  template <typename TBus>
  static void spiEndTransactionThunk(void* context) {
    static_cast<TBus*>(context)->endTransaction();
  }

  bool beginCommon();
  bool configureController();
  bool pollFrame(bool updatePublicState);

  bool isValidMode(uint8_t mode) const;
  bool isAnalogMode(uint8_t mode) const;
  void updateConnectionStatus(uint8_t mode);

  void sendCommand(const uint8_t* tx,
                   uint8_t* rx,
                   uint8_t length);

  void beginFrame();
  void endFrame();

  uint8_t transferByte(uint8_t value);
  uint8_t transferBitBang(uint8_t value);

  Transport transport_;

  void* spiContext_;
  SpiBeginFn spiBegin_;
  SpiBeginTransactionFn spiBeginTransaction_;
  SpiTransferFn spiTransfer_;
  SpiEndTransactionFn spiEndTransaction_;

  uint8_t clockPin_;
  uint8_t commandPin_;
  uint8_t csPin_;
  uint8_t dataPin_;

  uint32_t clockHz_;
  uint16_t byteDelayUs_;
  uint16_t pollIntervalMs_;
  uint16_t recoveryIntervalMs_;

  uint8_t packet_[21];
  uint8_t mode_;

  uint16_t buttons_;
  uint16_t previousButtons_;
  bool buttonHistoryValid_;

  TungLamPS2StickFilter leftStick_;
  TungLamPS2StickFilter rightStick_;

  bool connected_;
  PS2ConnectionStatus status_;

  uint32_t packetCount_;
  uint32_t errorCount_;
  uint16_t reconnectCount_;

  unsigned long lastPollMs_;
  unsigned long lastRecoveryMs_;
};
