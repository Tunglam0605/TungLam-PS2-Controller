# Changelog

## 0.5.0 - 2026-10-02

### Example architecture
- Reduced the Arduino IDE menu from nine focused demos to three project-oriented templates: `ControllerTemplate`, `Diagnostics`, and `ConnectionModes`.
- `ControllerTemplate` exposes ready-made user hooks for stick directions, button events, and controller-loss fail-safe so a user can copy the sketch and fill only project behavior.
- Consolidated link, event, raw-state and reconnect testing into `Diagnostics`.
- Consolidated default SPI, explicit SPI bus and BitBang setup into `ConnectionModes`.
- Moved the previous detailed examples to `extras/reference-examples/` instead of deleting them.

### Compatibility
- No PS2 protocol, filtering, reconnect, timing or public API behavior changed.
- Real PS2 receiver hardware validation remains pending.

## 0.4.1 - 2026-10-02

### Documentation and examples
- Reorganized examples into a clearer learning path: start with `BasicRead` and `DebugMonitor`, then move to focused button/joystick/raw/recovery/transport examples.
- Clarified that TungLam_PS2 is an input library and does not hard-code robot driving behavior.
- Documented the two robot-driving examples that live in TungLam_OmniMecanum_4WD:
  - `PS2RobotControl`: RoboBall/V5-style right-stick rotation priority.
  - `PS2RobotVectorMix`: simultaneous translation + rotation through vx/vy/wz.
- Added historical RoboBall joystick-control notes to the Vietnamese usage guide.
- Clarified in `JoystickDirections` that application behavior is intentionally project-defined.

## 0.4.0 - 2026-10-02

### Documentation
- Expanded Vietnamese Doxygen comments for public enums and APIs.
- Documented update/connected/status/counters semantics.
- Documented held/pressed/released button behavior and one-shot edge semantics.
- Documented filtered joystick coordinates, raw values and axis conventions.
- Documented calibration, timing profiles, poll rate and advanced timing APIs.
- Expanded joystick filter documentation in Vietnamese.
- Added `docs/HUONG_DAN_SU_DUNG_VI.md`.

### Examples
- Reworked examples to avoid teaching Serial spam or delay-based polling.
- Added detailed purpose, wiring and expected behavior comments.
- Added `ConnectionRecovery` for fail-safe/reconnect testing.
- `JoystickDirections` now prints only on direction changes.
- `BasicSPI` only prints initialization result once.
- `BasicRead` demonstrates production usage with no Serial and no delay.

## 0.3.1 - 2026-10-02

### Changed
- Debug monitor output is now human-readable and consolidated.
- Each change snapshot produces exactly one newline.
- Multiple simultaneous changes are combined with ` | `.
- Button events use `BTN=NAME:PRESSED/RELEASED`.
- Joystick events use `LEFT/RIGHT=DIRECTION(X,Y)`.
- Link, error and reconnect state can share the same line.
- Debug remains event-driven: no output is produced while state is unchanged.

## 0.3.0 - 2026-10-02

### Added
- Opt-in event-based `debug(Stream&)` API.
- One-shot `printState(Stream&)` snapshot API.
- `resetDebug()` for debug baseline reset.
- New examples: `BasicRead`, `ButtonEvents`, `DebugMonitor`, `RawAnalogTest`.

### Changed
- Core `update()` path never prints to Serial.
- Debug output is emitted only when the user explicitly calls `debug()` or `printState()`.
- `debug()` is event-driven and only prints state/button/direction/error/reconnect changes.

### Fixed
- `pressed()` / `released()` are now one-application-iteration edge events, so a fast loop cannot process the same button edge repeatedly between 50 Hz controller frames.

## 0.2.2 - 2026-10-02

### Documentation
- Added board-by-board SPI wiring lookup table directly in the `begin(CS_PIN)` Doxygen comment.
- Added wiring rows for Mega 2560, UNO R3, Nano classic, UNO R4, Nano 33 IoT, Nano 33 BLE, MKR WiFi 1010, Micro and Leonardo.
- Added the same quick-reference table to README.
- Clarified that CS is selectable GPIO; the table shows recommended/convenient CS pins only.

## 0.2.1 - 2026-10-02

### Documentation
- Added Doxygen wiring diagrams directly to all `begin()` overloads.
- Added explicit PS2-to-SPI signal mapping for DAT/MISO, CMD/MOSI, CLK/SCK and CS/ATT.
- Added Arduino Mega 2560 wiring example for `begin(53)`.
- Added custom SPI bus wiring guidance for `begin(SPI1, csPin)`.
- Added BitBang wiring example for legacy RoboBall pins.
- Added README wiring section and power/GND caution.

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
