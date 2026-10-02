# Changelog

## 0.2.0 - 2026-10-02

### Changed
- Default controller poll rate changed from 100 Hz to 50 Hz.
- Poll scheduler now uses `micros()` internally instead of millisecond scheduling.
- `update()` remains non-blocking at application level: it can be called continuously without user `delay()`.
- Added simple presets:
  - `PS2PollRate::Hz20`
  - `PS2PollRate::Hz50`
  - `PS2PollRate::Hz100`
- Added `setPollRateHz()` for 1..200 Hz custom rates.
- Added `setPollIntervalUs()` for advanced timing control.
- Kept `setPollIntervalMs()` for compatibility.

### Rationale
- 50 Hz gives a 20 ms input period, fast enough for manual robot control while giving wireless/clone PS2 receivers more processing margin.
- Poll rate and SPI clock/byte timing are now explicitly separate concepts.

## 0.1.0 - 2026-10-02

### Added
- Independent PS2 controller protocol implementation for Arduino.
- Three initialization paths:
  - `begin(csPin)` using the board's default hardware SPI.
  - `begin(spiBus, csPin)` for boards with multiple SPI buses.
  - `beginBitBang(clk, cmd, cs, dat)` for arbitrary pins and legacy wiring.
- Three button semantics: held, pressed edge and released edge.
- Raw joystick access plus median-filtered centered axes.
- Discrete joystick states: Center, Up, Down, Left, Right and Unknown.
- Median-of-3, deadzone, hysteresis and stable-sample confirmation.
- Fail-safe Unknown state for ambiguous joystick regions.
- Automatic analog-mode configuration and reconnect state machine.
- Immediate input neutralization on communication failure.
- Button edge resynchronization after begin/reconnect to avoid false events.
- Configurable SPI timing profiles and manual clock/delay tuning.
- Multi-architecture CI for AVR, SAMD, Renesas and Mbed Arduino cores.
- Strict Arduino Lint and host-side joystick filter unit tests.

### Validation
- Software/CI validated.
- Real PS2 receiver hardware validation is still pending for this release.
