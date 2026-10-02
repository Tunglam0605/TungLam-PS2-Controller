#include <TungLam_PS2.h>

TungLamPS2 ps2;

static unsigned long lastPrintMs = 0;

void setup() {
  Serial.begin(115200);
  ps2.begin(53);

  Serial.println(F("PS2 raw analog test"));
  Serial.println(F("Move both sticks slowly through full range."));
}

void loop() {
  ps2.update();

  // printState() là snapshot chủ động.
  // Rate-limit 100 ms để chỉ in 10 dòng/giây.
  const unsigned long now = millis();

  if (static_cast<unsigned long>(now - lastPrintMs) >= 100UL) {
    lastPrintMs = now;
    ps2.printState(Serial);
  }
}
