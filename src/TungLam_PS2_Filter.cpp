#include "TungLam_PS2_Filter.h"

TungLamPS2StickFilter::TungLamPS2StickFilter()
    : centerX_(128),
      centerY_(128),
      inputX_(128),
      inputY_(128),
      historyCount_(0),
      historyIndex_(0),
      filteredX_(128),
      filteredY_(128),
      direction_(PS2StickDirection::Center),
      candidate_(PS2StickDirection::Center),
      candidateCount_(0) {
  historyX_[0] = historyX_[1] = historyX_[2] = 128;
  historyY_[0] = historyY_[1] = historyY_[2] = 128;
}

void TungLamPS2StickFilter::reset() {
  inputX_ = centerX_;
  inputY_ = centerY_;
  historyCount_ = 0;
  historyIndex_ = 0;
  filteredX_ = centerX_;
  filteredY_ = centerY_;
  direction_ = PS2StickDirection::Center;
  candidate_ = PS2StickDirection::Center;
  candidateCount_ = 0;

  historyX_[0] = historyX_[1] = historyX_[2] = centerX_;
  historyY_[0] = historyY_[1] = historyY_[2] = centerY_;
}

void TungLamPS2StickFilter::setConfig(const PS2StickFilterConfig& config) {
  config_ = config;

  if (config_.stableSamples == 0) {
    config_.stableSamples = 1;
  }

  if (config_.exitThreshold > config_.enterThreshold) {
    config_.exitThreshold = config_.enterThreshold;
  }
}

const PS2StickFilterConfig& TungLamPS2StickFilter::config() const {
  return config_;
}

void TungLamPS2StickFilter::setCenter(uint8_t centerX, uint8_t centerY) {
  centerX_ = centerX;
  centerY_ = centerY;
  reset();
}

uint8_t TungLamPS2StickFilter::centerX() const { return centerX_; }
uint8_t TungLamPS2StickFilter::centerY() const { return centerY_; }

void TungLamPS2StickFilter::push(uint8_t rawX, uint8_t rawY) {
  inputX_ = rawX;
  inputY_ = rawY;

  historyX_[historyIndex_] = rawX;
  historyY_[historyIndex_] = rawY;
  historyIndex_ = static_cast<uint8_t>((historyIndex_ + 1U) % 3U);

  if (historyCount_ < 3) {
    ++historyCount_;
  }

  if (historyCount_ < 3) {
    filteredX_ = rawX;
    filteredY_ = rawY;
  } else {
    filteredX_ = median3(historyX_[0], historyX_[1], historyX_[2]);
    filteredY_ = median3(historyY_[0], historyY_[1], historyY_[2]);
  }

  const int16_t filteredCenteredX = x();
  const int16_t filteredCenteredY = y();

  if (keepCurrentDirection(filteredCenteredX, filteredCenteredY)) {
    candidate_ = direction_;
    candidateCount_ = 0;
    return;
  }

  const PS2StickDirection next =
      classify(filteredCenteredX, filteredCenteredY);

  // Fail-safe: vùng mơ hồ không được giữ hướng chuyển động cũ.
  if (next == PS2StickDirection::Unknown) {
    direction_ = PS2StickDirection::Unknown;
    candidate_ = next;
    candidateCount_ = 0;
    return;
  }

  // Thả joystick về tâm phải dừng ngay, không đợi stableSamples.
  if (next == PS2StickDirection::Center) {
    direction_ = PS2StickDirection::Center;
    candidate_ = next;
    candidateCount_ = 0;
    return;
  }

  if (next == direction_) {
    candidate_ = next;
    candidateCount_ = 0;
    return;
  }

  if (next != candidate_) {
    candidate_ = next;
    candidateCount_ = 1;
  } else if (candidateCount_ < 255) {
    ++candidateCount_;
  }

  // Trong lúc xác nhận hướng mới, không giữ hướng cũ.
  direction_ = PS2StickDirection::Unknown;

  if (candidateCount_ >= config_.stableSamples) {
    direction_ = candidate_;
    candidateCount_ = 0;
  }
}

uint8_t TungLamPS2StickFilter::rawX() const { return inputX_; }
uint8_t TungLamPS2StickFilter::rawY() const { return inputY_; }

int16_t TungLamPS2StickFilter::x() const {
  return static_cast<int16_t>(filteredX_) -
         static_cast<int16_t>(centerX_);
}

int16_t TungLamPS2StickFilter::y() const {
  return static_cast<int16_t>(filteredY_) -
         static_cast<int16_t>(centerY_);
}

PS2StickDirection TungLamPS2StickFilter::direction() const {
  return direction_;
}

uint8_t TungLamPS2StickFilter::median3(uint8_t a,
                                      uint8_t b,
                                      uint8_t c) {
  if (a > b) {
    const uint8_t tmp = a;
    a = b;
    b = tmp;
  }

  if (b > c) {
    const uint8_t tmp = b;
    b = c;
    c = tmp;
  }

  if (a > b) {
    const uint8_t tmp = a;
    a = b;
    b = tmp;
  }

  return b;
}

int16_t TungLamPS2StickFilter::abs16(int16_t value) {
  return value < 0 ? static_cast<int16_t>(-value) : value;
}

bool TungLamPS2StickFilter::keepCurrentDirection(int16_t x,
                                                 int16_t y) const {
  const int16_t absX = abs16(x);
  const int16_t absY = abs16(y);
  const int16_t perpendicular =
      static_cast<int16_t>(config_.perpendicularLimit) + 10;

  switch (direction_) {
    case PS2StickDirection::Up:
      return y <= -static_cast<int16_t>(config_.exitThreshold) &&
             absX <= perpendicular;

    case PS2StickDirection::Down:
      return y >= static_cast<int16_t>(config_.exitThreshold) &&
             absX <= perpendicular;

    case PS2StickDirection::Left:
      return x <= -static_cast<int16_t>(config_.exitThreshold) &&
             absY <= perpendicular;

    case PS2StickDirection::Right:
      return x >= static_cast<int16_t>(config_.exitThreshold) &&
             absY <= perpendicular;

    default:
      return false;
  }
}

PS2StickDirection TungLamPS2StickFilter::classify(int16_t x,
                                                  int16_t y) const {
  const int16_t absX = abs16(x);
  const int16_t absY = abs16(y);

  if (absX <= config_.deadzone && absY <= config_.deadzone) {
    return PS2StickDirection::Center;
  }

  if (x >= config_.enterThreshold &&
      absY <= config_.perpendicularLimit) {
    return PS2StickDirection::Right;
  }

  if (x <= -static_cast<int16_t>(config_.enterThreshold) &&
      absY <= config_.perpendicularLimit) {
    return PS2StickDirection::Left;
  }

  if (y <= -static_cast<int16_t>(config_.enterThreshold) &&
      absX <= config_.perpendicularLimit) {
    return PS2StickDirection::Up;
  }

  if (y >= config_.enterThreshold &&
      absX <= config_.perpendicularLimit) {
    return PS2StickDirection::Down;
  }

  return PS2StickDirection::Unknown;
}
