#include "TungLam_PS2.h"

namespace {

static const uint8_t kEnterConfig[] = {
    0x01, 0x43, 0x00, 0x01, 0x00
};

static const uint8_t kSetAnalogMode[] = {
    0x01, 0x44, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t kExitConfig[] = {
    0x01, 0x43, 0x00, 0x00, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A
};

static const uint8_t kPollCommand[] = {
    0x01, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void printConnectionStatus(Stream& output,
                           PS2ConnectionStatus status) {
  switch (status) {
    case PS2ConnectionStatus::Probing:
      output.print(F("Probing"));
      break;
    case PS2ConnectionStatus::Configuring:
      output.print(F("Configuring"));
      break;
    case PS2ConnectionStatus::ConnectedDigital:
      output.print(F("ConnectedDigital"));
      break;
    case PS2ConnectionStatus::ConnectedAnalog:
      output.print(F("ConnectedAnalog"));
      break;
    case PS2ConnectionStatus::ConnectedPressure:
      output.print(F("ConnectedPressure"));
      break;
    case PS2ConnectionStatus::Recovering:
      output.print(F("Recovering"));
      break;
    case PS2ConnectionStatus::NoResponse:
      output.print(F("NoResponse"));
      break;
    case PS2ConnectionStatus::ConfigRejected:
      output.print(F("ConfigRejected"));
      break;
    case PS2ConnectionStatus::Disconnected:
    default:
      output.print(F("Disconnected"));
      break;
  }
}

void printStickDirection(Stream& output,
                         PS2StickDirection direction) {
  switch (direction) {
    case PS2StickDirection::Center:
      output.print(F("Center"));
      break;
    case PS2StickDirection::Up:
      output.print(F("Up"));
      break;
    case PS2StickDirection::Down:
      output.print(F("Down"));
      break;
    case PS2StickDirection::Left:
      output.print(F("Left"));
      break;
    case PS2StickDirection::Right:
      output.print(F("Right"));
      break;
    case PS2StickDirection::Unknown:
    default:
      output.print(F("Unknown"));
      break;
  }
}

void printButtonName(Stream& output, PS2Button button) {
  switch (button) {
    case PS2Button::Select: output.print(F("Select")); break;
    case PS2Button::L3: output.print(F("L3")); break;
    case PS2Button::R3: output.print(F("R3")); break;
    case PS2Button::Start: output.print(F("Start")); break;
    case PS2Button::Up: output.print(F("Up")); break;
    case PS2Button::Right: output.print(F("Right")); break;
    case PS2Button::Down: output.print(F("Down")); break;
    case PS2Button::Left: output.print(F("Left")); break;
    case PS2Button::L2: output.print(F("L2")); break;
    case PS2Button::R2: output.print(F("R2")); break;
    case PS2Button::L1: output.print(F("L1")); break;
    case PS2Button::R1: output.print(F("R1")); break;
    case PS2Button::Triangle: output.print(F("Triangle")); break;
    case PS2Button::Circle: output.print(F("Circle")); break;
    case PS2Button::Cross: output.print(F("Cross")); break;
    case PS2Button::Square: output.print(F("Square")); break;
  }
}

}  // namespace

TungLamPS2::TungLamPS2()
    : transport_(Transport::None),
      spiContext_(nullptr),
      spiBegin_(nullptr),
      spiBeginTransaction_(nullptr),
      spiTransfer_(nullptr),
      spiEndTransaction_(nullptr),
      clockPin_(0),
      commandPin_(0),
      csPin_(0),
      dataPin_(0),
      clockHz_(125000UL),
      byteDelayUs_(20),
      pollIntervalUs_(20000UL),
      recoveryIntervalMs_(250),
      mode_(0),
      buttons_(0),
      previousButtons_(0),
      pressedButtons_(0),
      releasedButtons_(0),
      buttonHistoryValid_(false),
      connected_(false),
      status_(PS2ConnectionStatus::Disconnected),
      packetCount_(0),
      errorCount_(0),
      reconnectCount_(0),
      debugInitialized_(false),
      debugLastStatus_(PS2ConnectionStatus::Disconnected),
      debugLastLeftDirection_(PS2StickDirection::Center),
      debugLastRightDirection_(PS2StickDirection::Center),
      debugLastErrorCount_(0),
      debugLastReconnectCount_(0),
      lastPollUs_(0),
      lastRecoveryMs_(0) {
  for (uint8_t i = 0; i < sizeof(packet_); ++i) {
    packet_[i] = 0xFF;
  }
}

bool TungLamPS2::beginBitBang(uint8_t clockPin,
                              uint8_t commandPin,
                              uint8_t csPin,
                              uint8_t dataPin) {
  transport_ = Transport::BitBang;
  clockPin_ = clockPin;
  commandPin_ = commandPin;
  csPin_ = csPin;
  dataPin_ = dataPin;

  spiContext_ = nullptr;
  spiBegin_ = nullptr;
  spiBeginTransaction_ = nullptr;
  spiTransfer_ = nullptr;
  spiEndTransaction_ = nullptr;

  return beginCommon();
}

bool TungLamPS2::beginCommon() {
  connected_ = false;
  status_ = PS2ConnectionStatus::Probing;
  buttons_ = 0;
  previousButtons_ = 0;
  pressedButtons_ = 0;
  releasedButtons_ = 0;
  buttonHistoryValid_ = false;
  resetDebug();

  leftStick_.reset();
  rightStick_.reset();

  pinMode(csPin_, OUTPUT);
  digitalWrite(csPin_, HIGH);

  if (transport_ == Transport::HardwareSPI) {
    if (!spiContext_ ||
        !spiBegin_ ||
        !spiBeginTransaction_ ||
        !spiTransfer_ ||
        !spiEndTransaction_) {
      status_ = PS2ConnectionStatus::NoResponse;
      return false;
    }

    spiBegin_(spiContext_);
  } else if (transport_ == Transport::BitBang) {
    pinMode(clockPin_, OUTPUT);
    pinMode(commandPin_, OUTPUT);
    pinMode(dataPin_, INPUT_PULLUP);

    digitalWrite(clockPin_, HIGH);
    digitalWrite(commandPin_, HIGH);
  } else {
    status_ = PS2ConnectionStatus::NoResponse;
    return false;
  }

  delay(100);
  return configureController();
}

bool TungLamPS2::configureController() {
  status_ = PS2ConnectionStatus::Configuring;

  uint8_t rx[21] = {0};

  bool responded = false;

  for (uint8_t i = 0; i < 3; ++i) {
    if (pollFrame(false)) {
      responded = true;
      break;
    }

    delay(20);
  }

  for (uint8_t attempt = 0; attempt < 8; ++attempt) {
    sendCommand(kEnterConfig, rx, sizeof(kEnterConfig));
    delay(static_cast<unsigned long>(2U + attempt));

    sendCommand(kSetAnalogMode, rx, sizeof(kSetAnalogMode));
    delay(static_cast<unsigned long>(2U + attempt));

    sendCommand(kExitConfig, rx, sizeof(kExitConfig));
    delay(static_cast<unsigned long>(5U + attempt));

    if (pollFrame(true) && isAnalogMode(mode_)) {
      connected_ = true;
      updateConnectionStatus(mode_);
      return true;
    }
  }

  connected_ = false;
  ++errorCount_;

  status_ = responded
                ? PS2ConnectionStatus::ConfigRejected
                : PS2ConnectionStatus::NoResponse;

  return false;
}

bool TungLamPS2::update() {
  // Edge events chỉ có hiệu lực trong đúng một vòng application loop
  // có gọi update(). Điều này tránh pressed()/released() lặp lại nhiều lần
  // khi loop nhanh hơn poll rate của tay cầm.
  pressedButtons_ = 0;
  releasedButtons_ = 0;

  const unsigned long nowUs = micros();
  const unsigned long nowMs = millis();

  if (connected_) {
    // Non-blocking scheduler:
    // update() có thể được gọi liên tục, nhưng PS2 chỉ được poll đúng rate.
    if (static_cast<unsigned long>(nowUs - lastPollUs_) <
        pollIntervalUs_) {
      return true;
    }

    if (pollFrame(true) && isAnalogMode(mode_)) {
      return true;
    }

    // Fail-safe ngay từ frame lỗi đầu tiên:
    // không giữ button/joystick command cũ trong lúc link có vấn đề.
    ++errorCount_;
    connected_ = false;
    buttons_ = 0;
    previousButtons_ = 0;
    pressedButtons_ = 0;
    releasedButtons_ = 0;
    buttonHistoryValid_ = false;
    status_ = PS2ConnectionStatus::Recovering;

    leftStick_.reset();
    rightStick_.reset();

    lastRecoveryMs_ = nowMs;
    return false;
  }

  if (static_cast<unsigned long>(nowMs - lastRecoveryMs_) <
      recoveryIntervalMs_) {
    return false;
  }

  lastRecoveryMs_ = nowMs;
  status_ = PS2ConnectionStatus::Recovering;
  ++reconnectCount_;

  return configureController();
}

bool TungLamPS2::pollFrame(bool updatePublicState) {
  uint8_t rx[9] = {0};

  sendCommand(kPollCommand, rx, sizeof(kPollCommand));
  lastPollUs_ = micros();

  mode_ = rx[1];

  if (!isValidMode(mode_) || rx[2] != 0x5A) {
    return false;
  }

  if (!updatePublicState) {
    return true;
  }

  for (uint8_t i = 0; i < 9; ++i) {
    packet_[i] = rx[i];
  }

  const uint16_t activeLow =
      static_cast<uint16_t>(rx[3]) |
      (static_cast<uint16_t>(rx[4]) << 8U);

  const uint16_t newButtons =
      static_cast<uint16_t>(~activeLow);

  if (!buttonHistoryValid_) {
    // Frame đầu sau begin/reconnect chỉ đồng bộ trạng thái.
    // Không tạo pressed()/released() giả.
    buttons_ = newButtons;
    previousButtons_ = newButtons;
    pressedButtons_ = 0;
    releasedButtons_ = 0;
    buttonHistoryValid_ = true;
  } else {
    previousButtons_ = buttons_;
    buttons_ = newButtons;

    pressedButtons_ = static_cast<uint16_t>(
        buttons_ & static_cast<uint16_t>(~previousButtons_));

    releasedButtons_ = static_cast<uint16_t>(
        previousButtons_ & static_cast<uint16_t>(~buttons_));
  }

  rightStick_.push(rx[5], rx[6]);
  leftStick_.push(rx[7], rx[8]);

  connected_ = true;
  ++packetCount_;

  updateConnectionStatus(mode_);
  return true;
}

bool TungLamPS2::isValidMode(uint8_t mode) const {
  return mode == 0x41 ||
         mode == 0x73 ||
         mode == 0x79;
}

bool TungLamPS2::isAnalogMode(uint8_t mode) const {
  return mode == 0x73 ||
         mode == 0x79;
}

void TungLamPS2::updateConnectionStatus(uint8_t mode) {
  if (mode == 0x79) {
    status_ = PS2ConnectionStatus::ConnectedPressure;
  } else if (mode == 0x73) {
    status_ = PS2ConnectionStatus::ConnectedAnalog;
  } else if (mode == 0x41) {
    status_ = PS2ConnectionStatus::ConnectedDigital;
  } else {
    status_ = PS2ConnectionStatus::Disconnected;
  }
}

void TungLamPS2::sendCommand(const uint8_t* tx,
                             uint8_t* rx,
                             uint8_t length) {
  beginFrame();

  for (uint8_t i = 0; i < length; ++i) {
    rx[i] = transferByte(tx[i]);

    if (byteDelayUs_ > 0) {
      delayMicroseconds(byteDelayUs_);
    }
  }

  endFrame();
}

void TungLamPS2::beginFrame() {
  if (transport_ == Transport::HardwareSPI) {
    spiBeginTransaction_(spiContext_, clockHz_);
  }

  digitalWrite(csPin_, LOW);
  delayMicroseconds(10);
}

void TungLamPS2::endFrame() {
  delayMicroseconds(10);
  digitalWrite(csPin_, HIGH);

  if (transport_ == Transport::HardwareSPI) {
    spiEndTransaction_(spiContext_);
  }
}

uint8_t TungLamPS2::transferByte(uint8_t value) {
  if (transport_ == Transport::HardwareSPI) {
    return spiTransfer_(spiContext_, value);
  }

  return transferBitBang(value);
}

uint8_t TungLamPS2::transferBitBang(uint8_t value) {
  uint8_t result = 0;

  uint16_t halfPeriodUs =
      clockHz_ > 0
          ? static_cast<uint16_t>(500000UL / clockHz_)
          : 4;

  if (halfPeriodUs == 0) {
    halfPeriodUs = 1;
  }

  for (uint8_t bit = 0; bit < 8; ++bit) {
    digitalWrite(
        commandPin_,
        (value & static_cast<uint8_t>(1U << bit))
            ? HIGH
            : LOW);

    digitalWrite(clockPin_, LOW);
    delayMicroseconds(halfPeriodUs);

    if (digitalRead(dataPin_)) {
      result |= static_cast<uint8_t>(1U << bit);
    }

    digitalWrite(clockPin_, HIGH);
    delayMicroseconds(halfPeriodUs);
  }

  digitalWrite(commandPin_, HIGH);
  return result;
}

bool TungLamPS2::connected() const {
  return connected_;
}

PS2ConnectionStatus TungLamPS2::status() const {
  return status_;
}

uint32_t TungLamPS2::packetCount() const {
  return packetCount_;
}

uint32_t TungLamPS2::errorCount() const {
  return errorCount_;
}

uint16_t TungLamPS2::reconnectCount() const {
  return reconnectCount_;
}

bool TungLamPS2::button(PS2Button key) const {
  return (buttons_ & static_cast<uint16_t>(key)) != 0;
}

bool TungLamPS2::pressed(PS2Button key) const {
  return (pressedButtons_ & static_cast<uint16_t>(key)) != 0;
}

bool TungLamPS2::released(PS2Button key) const {
  return (releasedButtons_ & static_cast<uint16_t>(key)) != 0;
}

uint16_t TungLamPS2::buttons() const {
  return buttons_;
}

void TungLamPS2::debug(Stream& output) {
  if (!debugInitialized_) {
    output.println(F("[PS2] Debug monitor started"));
    printState(output);

    debugInitialized_ = true;
    debugLastStatus_ = status_;
    debugLastLeftDirection_ = leftStick_.direction();
    debugLastRightDirection_ = rightStick_.direction();
    debugLastErrorCount_ = errorCount_;
    debugLastReconnectCount_ = reconnectCount_;
    return;
  }

  if (status_ != debugLastStatus_) {
    output.print(F("[PS2] STATUS "));
    printConnectionStatus(output, status_);
    output.println();
  }

  for (uint8_t bit = 0; bit < 16; ++bit) {
    const uint16_t mask = static_cast<uint16_t>(1U << bit);
    const PS2Button key = static_cast<PS2Button>(mask);

    if ((pressedButtons_ & mask) != 0) {
      output.print(F("[PS2] BUTTON "));
      printButtonName(output, key);
      output.println(F(" PRESSED"));
    }

    if ((releasedButtons_ & mask) != 0) {
      output.print(F("[PS2] BUTTON "));
      printButtonName(output, key);
      output.println(F(" RELEASED"));
    }
  }

  const PS2StickDirection left = leftStick_.direction();
  if (left != debugLastLeftDirection_) {
    output.print(F("[PS2] LEFT "));
    printStickDirection(output, left);
    output.print(F(" x="));
    output.print(leftStick_.x());
    output.print(F(" y="));
    output.println(leftStick_.y());
  }

  const PS2StickDirection right = rightStick_.direction();
  if (right != debugLastRightDirection_) {
    output.print(F("[PS2] RIGHT "));
    printStickDirection(output, right);
    output.print(F(" x="));
    output.print(rightStick_.x());
    output.print(F(" y="));
    output.println(rightStick_.y());
  }

  if (errorCount_ != debugLastErrorCount_) {
    output.print(F("[PS2] ERRORS "));
    output.println(errorCount_);
  }

  if (reconnectCount_ != debugLastReconnectCount_) {
    output.print(F("[PS2] RECONNECTS "));
    output.println(reconnectCount_);
  }

  debugLastStatus_ = status_;
  debugLastLeftDirection_ = left;
  debugLastRightDirection_ = right;
  debugLastErrorCount_ = errorCount_;
  debugLastReconnectCount_ = reconnectCount_;
}

void TungLamPS2::printState(Stream& output) const {
  output.print(F("[PS2] status="));
  printConnectionStatus(output, status_);

  output.print(F(" buttons=0x"));
  output.print(buttons_, HEX);

  output.print(F(" left="));
  printStickDirection(output, leftStick_.direction());
  output.print(F("("));
  output.print(leftStick_.x());
  output.print(F(","));
  output.print(leftStick_.y());
  output.print(F(") raw("));
  output.print(leftStick_.rawX());
  output.print(F(","));
  output.print(leftStick_.rawY());
  output.print(F(")"));

  output.print(F(" right="));
  printStickDirection(output, rightStick_.direction());
  output.print(F("("));
  output.print(rightStick_.x());
  output.print(F(","));
  output.print(rightStick_.y());
  output.print(F(") raw("));
  output.print(rightStick_.rawX());
  output.print(F(","));
  output.print(rightStick_.rawY());
  output.print(F(")"));

  output.print(F(" packets="));
  output.print(packetCount_);
  output.print(F(" errors="));
  output.print(errorCount_);
  output.print(F(" reconnects="));
  output.println(reconnectCount_);
}

void TungLamPS2::resetDebug() {
  debugInitialized_ = false;
  debugLastStatus_ = status_;
  debugLastLeftDirection_ = leftStick_.direction();
  debugLastRightDirection_ = rightStick_.direction();
  debugLastErrorCount_ = errorCount_;
  debugLastReconnectCount_ = reconnectCount_;
}

PS2StickDirection TungLamPS2::leftDirection() const {
  return leftStick_.direction();
}

PS2StickDirection TungLamPS2::rightDirection() const {
  return rightStick_.direction();
}

int16_t TungLamPS2::leftX() const {
  return leftStick_.x();
}

int16_t TungLamPS2::leftY() const {
  return leftStick_.y();
}

int16_t TungLamPS2::rightX() const {
  return rightStick_.x();
}

int16_t TungLamPS2::rightY() const {
  return rightStick_.y();
}

uint8_t TungLamPS2::leftRawX() const {
  return leftStick_.rawX();
}

uint8_t TungLamPS2::leftRawY() const {
  return leftStick_.rawY();
}

uint8_t TungLamPS2::rightRawX() const {
  return rightStick_.rawX();
}

uint8_t TungLamPS2::rightRawY() const {
  return rightStick_.rawY();
}

void TungLamPS2::setStickFilter(
    const PS2StickFilterConfig& config) {
  leftStick_.setConfig(config);
  rightStick_.setConfig(config);
}

void TungLamPS2::setStickCenters(uint8_t leftX,
                                 uint8_t leftY,
                                 uint8_t rightX,
                                 uint8_t rightY) {
  leftStick_.setCenter(leftX, leftY);
  rightStick_.setCenter(rightX, rightY);
}

bool TungLamPS2::calibrateCenter(uint8_t samples,
                                 uint16_t sampleIntervalMs) {
  if (!connected_ || samples == 0) {
    return false;
  }

  uint32_t leftXSum = 0;
  uint32_t leftYSum = 0;
  uint32_t rightXSum = 0;
  uint32_t rightYSum = 0;

  uint8_t accepted = 0;

  for (uint8_t i = 0; i < samples; ++i) {
    delay(sampleIntervalMs);

    uint8_t frame[9] = {0};
    sendCommand(kPollCommand, frame, sizeof(frame));

    if (!isValidMode(frame[1]) || frame[2] != 0x5A) {
      continue;
    }

    rightXSum += frame[5];
    rightYSum += frame[6];
    leftXSum += frame[7];
    leftYSum += frame[8];

    ++accepted;
  }

  const uint8_t minimumAccepted =
      samples < 2 ? 1 : static_cast<uint8_t>(samples / 2U);

  if (accepted < minimumAccepted) {
    return false;
  }

  setStickCenters(
      static_cast<uint8_t>(leftXSum / accepted),
      static_cast<uint8_t>(leftYSum / accepted),
      static_cast<uint8_t>(rightXSum / accepted),
      static_cast<uint8_t>(rightYSum / accepted));

  lastPollUs_ = micros();
  return true;
}

void TungLamPS2::setTimingProfile(PS2TimingProfile profile) {
  switch (profile) {
    case PS2TimingProfile::Fast:
      clockHz_ = 500000UL;
      byteDelayUs_ = 3;
      break;

    case PS2TimingProfile::Balanced:
      clockHz_ = 250000UL;
      byteDelayUs_ = 10;
      break;

    case PS2TimingProfile::Compatible:
    default:
      clockHz_ = 125000UL;
      byteDelayUs_ = 20;
      break;
  }
}

void TungLamPS2::setClockHz(uint32_t clockHz) {
  if (clockHz > 0) {
    clockHz_ = clockHz;
  }
}

void TungLamPS2::setByteDelayUs(uint16_t byteDelayUs) {
  byteDelayUs_ = byteDelayUs;
}

void TungLamPS2::setPollRate(PS2PollRate rate) {
  setPollRateHz(static_cast<uint16_t>(rate));
}

bool TungLamPS2::setPollRateHz(uint16_t rateHz) {
  if (rateHz < 1 || rateHz > 200) {
    return false;
  }

  pollIntervalUs_ = 1000000UL / rateHz;
  return true;
}

void TungLamPS2::setPollIntervalUs(uint32_t pollIntervalUs) {
  if (pollIntervalUs > 0) {
    pollIntervalUs_ = pollIntervalUs;
  }
}

void TungLamPS2::setPollIntervalMs(uint16_t pollIntervalMs) {
  if (pollIntervalMs > 0) {
    pollIntervalUs_ =
        static_cast<uint32_t>(pollIntervalMs) * 1000UL;
  }
}
