#include <cassert>

#include "../src/TungLam_PS2_Filter.h"

static void pushMany(TungLamPS2StickFilter& filter,
                     uint8_t x,
                     uint8_t y,
                     int count = 5) {
  for (int i = 0; i < count; ++i) {
    filter.push(x, y);
  }
}

int main() {
  TungLamPS2StickFilter filter;

  pushMany(filter, 128, 128);
  assert(filter.direction() == PS2StickDirection::Center);

  filter.push(132, 125);
  filter.push(126, 131);
  filter.push(129, 127);
  assert(filter.direction() == PS2StickDirection::Center);

  filter.push(128, 10);
  assert(filter.direction() == PS2StickDirection::Unknown);
  filter.push(128, 10);
  filter.push(128, 10);
  assert(filter.direction() == PS2StickDirection::Up);

  filter.push(128, 40);
  filter.push(128, 45);
  assert(filter.direction() == PS2StickDirection::Up);

  pushMany(filter, 200, 55);
  assert(filter.direction() == PS2StickDirection::Unknown);

  filter.push(245, 128);
  assert(filter.direction() == PS2StickDirection::Unknown);
  filter.push(245, 128);
  filter.push(245, 128);
  assert(filter.direction() == PS2StickDirection::Right);

  pushMany(filter, 128, 128);
  assert(filter.direction() == PS2StickDirection::Center);

  return 0;
}
