# TungLam PS2 Controller

Thư viện đọc tay cầm PS2 dành cho Arduino/robotics, thiết kế theo hướng **đa board, linh hoạt, an toàn và dễ dùng**.

## Triết lý sử dụng

Thư viện tách rõ **đường chạy production** và **đường debug**:

```text
Production:
  ps2.update()
  -> không Serial
  -> không delay theo poll rate
  -> đọc state đã lọc

Debug:
  ps2.update()
  ps2.debug(Serial)
  -> chỉ in khi state thay đổi
  -> một newline cho mỗi snapshot

Service/raw:
  ps2.printState(Serial)
  -> luôn in khi được gọi
  -> người dùng tự rate-limit
```

Nút và joystick không gắn cứng chức năng robot. Thư viện chỉ chuẩn hóa input; project quyết định hành vi.

## Mục tiêu

- Không bắt người dùng tự đọc `LX/LY/RX/RY`, trừ tâm rồi viết lại hàng loạt `if`.
- Không map joystick trực tiếp thành tốc độ robot.
- Nút bấm để mở hoàn toàn cho từng project tự mapping chức năng.
- Hardware SPI là đường dùng chính; BitBang giữ cho wiring cũ và pin tùy ý.
- API/identifier dùng tiếng Anh chuẩn kỹ thuật; tài liệu mặc định tiếng Việt.

## Sơ đồ đấu nối

### Hardware SPI mặc định: `ps2.begin(CS_PIN)`

### Bảng tra nhanh SPI mặc định

| Board | PS2 DAT / MISO(CIPO) | PS2 CMD / MOSI(COPI) | PS2 CLK / SCK | CS gợi ý |
|---|---:|---:|---:|---:|
| Arduino Mega 2560 Rev3 | D50 | D51 | D52 | D53 |
| Arduino UNO R3 | D12 | D11 | D13 | D10 |
| Arduino Nano classic | D12 | D11 | D13 | D10 |
| Arduino UNO R4 Minima / WiFi | D12 | D11 | D13 | D10 |
| Arduino Nano 33 IoT | D12 | D11 | D13 | D10 |
| Arduino Nano 33 BLE | D12 | D11 | D13 | D10 |
| Arduino MKR WiFi 1010 | D10 | D8 | D9 | D7 |
| Arduino Micro | D14 | D16 | D15 | D17 |
| Arduino Leonardo | ICSP CIPO | ICSP COPI | ICSP SCK | D10 |

> **CS gợi ý không phải bắt buộc.** Với `ps2.begin(CS_PIN)`, ba dây DAT/CMD/CLK phải đi vào SPI mặc định của board; riêng CS/ATT có thể chọn GPIO digital khác nếu không xung đột.

Ví dụ Arduino Mega 2560:

```text
Đầu thu PS2            Arduino Mega 2560
-----------------------------------------
DAT / MISO      ->     D50 / MISO
CMD / MOSI      ->     D51 / MOSI
CLK / SCK       ->     D52 / SCK
CS / ATT / SEL  ->     chân CS bạn chọn, ví dụ D53
GND             ->     GND
VCC             ->     nguồn đúng theo module receiver
```

Code:

```cpp
ps2.begin(53);
```

Với board khác, không dùng bảng chân của Mega. Hãy nhìn pinout của board và nối DAT/CMD/CLK vào MISO/MOSI/SCK của SPI mặc định.

### Chọn SPI bus cụ thể: `ps2.begin(SPIx, CS_PIN)`

```cpp
ps2.begin(SPI1, 7);
```

Khi đó:

```text
DAT / MISO      -> MISO của SPI1
CMD / MOSI      -> MOSI của SPI1
CLK / SCK       -> SCK của SPI1
CS / ATT / SEL  -> D7
GND             -> GND
VCC             -> nguồn đúng theo module receiver
```

### BitBang: tự chọn từng chân

```cpp
ps2.beginBitBang(22, 26, 24, 28);
```

Tương ứng:

```text
CLK -> D22
CMD -> D26
CS  -> D24
DAT -> D28
```

> VCC không được mặc định hiểu là 5 V hay 3.3 V cho mọi receiver. Hãy cấp theo đúng module/adapter đang sử dụng và luôn nối chung GND.

## 3 cách khởi tạo

### 1. Đơn giản nhất: SPI mặc định của board

```cpp
TungLamPS2 ps2;

void setup() {
  ps2.begin(10);  // chỉ cần CS
}
```

MISO/MOSI/SCK do Arduino Core của board quản lý. Thư viện không hard-code bảng chân SPI cho từng board.

### 2. Chọn SPI bus cụ thể

```cpp
ps2.begin(SPI1, 7);
```

Dùng khi board/core có nhiều bus như `SPI1`, `SPI2`. API dùng template nên không phụ thuộc tên concrete class của SPI trên AVR, SAMD, Mbed, Renesas...

### 3. Chọn từng chân như cách cũ

```cpp
ps2.beginBitBang(
    22,  // CLK
    26,  // CMD / MOSI
    24,  // CS / ATT
    28   // DAT / MISO
);
```

## Nút bấm: đúng 3 kiểu cơ bản

```cpp
ps2.button(PS2Button::Cross);    // đang giữ
ps2.pressed(PS2Button::Cross);   // vừa nhấn
ps2.released(PS2Button::Cross);  // vừa nhả
```

Thư viện không cố định Cross, R1, L2... phải làm chức năng gì.

## Joystick: gọi kết quả cuối

```cpp
PS2StickDirection left = ps2.leftDirection();
PS2StickDirection right = ps2.rightDirection();
```

Các state:

```text
Center
Up
Down
Left
Right
Unknown
```

Ví dụ:

```cpp
if (ps2.leftDirection() == PS2StickDirection::Up) {
    robot.forward(180);  // tốc độ do người lập trình quyết định
}
```

Nếu cần giá trị analog:

```cpp
ps2.leftRawX();   // raw frame 0..255
ps2.leftRawY();

ps2.leftX();      // đã median-filter + trừ tâm
ps2.leftY();
```

## Pipeline lọc joystick

```text
raw 0..255
   ↓
median-of-3
   ↓
center compensation
   ↓
deadzone
   ↓
hysteresis
   ↓
stable-sample confirmation
   ↓
Up / Down / Left / Right / Center / Unknown
```

Hai nguyên tắc fail-safe:

1. Vùng chéo/mơ hồ → `Unknown`, không giữ hướng cũ.
2. Khi xác nhận một hướng mới → tạm `Unknown`, không tiếp tục hướng trước.

## Hiệu chỉnh tâm

Mặc định tâm là 128/128. Với tay clone bị lệch tâm:

```cpp
// Thả cả hai joystick rồi gọi:
ps2.calibrateCenter();
```

Hoặc set thủ công:

```cpp
ps2.setStickCenters(128, 127, 129, 128);
```

## Reconnect

`update()` kiểm tra từng frame. Ngay ở frame lỗi đầu tiên, thư viện neutralize button/joystick state để không giữ lệnh cũ, sau đó chuyển sang Recovering và tự thử cấu hình lại controller.

```cpp
if (!ps2.connected()) {
    // dừng cơ cấu điều khiển nếu cần
}
```

Frame đầu tiên sau `begin()` hoặc reconnect chỉ dùng để đồng bộ button state, vì vậy không phát sinh `pressed()`/`released()` giả nếu người dùng đang giữ nút trong lúc kết nối trở lại.

Diagnostics:

```cpp
ps2.status();
ps2.packetCount();
ps2.errorCount();
ps2.reconnectCount();
```

## Debug và test không spam Serial

Core driver không tự in bất kỳ thứ gì ra `Serial`. Chỉ `ps2.update()` là đủ cho production.

### Production

```cpp
void loop() {
  ps2.update();

  if (!ps2.connected()) {
    return;
  }

  // robot control...
}
```

Không cần `Serial.begin()`, không cần `delay()`.

### Debug theo sự kiện

Khi cần debug:

```cpp
void setup() {
  Serial.begin(115200);
  ps2.begin(53);
}

void loop() {
  ps2.update();
  ps2.debug(Serial);
}
```

`debug()` chỉ in khi có thay đổi đáng chú ý:

Mỗi lần có thay đổi, `debug()` chỉ phát **một dòng tổng hợp**:

```text
[PS2] LINK=CONNECTED_ANALOG | BTN=CROSS:PRESSED | LEFT=UP(1,-117) | RIGHT=CENTER(0,2) | ERR=0 | REC=0
```

Ví dụ các lần thay đổi tiếp theo:

```text
[PS2] BTN=CROSS:RELEASED,R1:PRESSED
[PS2] LEFT=CENTER(0,1)
[PS2] LINK=RECOVERING | ERR=1
```

Nếu không có event hoặc thay đổi trạng thái, `debug()` không in thêm dữ liệu. Serial Monitor có thể tự wrap nếu cửa sổ hẹp, nhưng thư viện chỉ tạo **một newline cho mỗi snapshot thay đổi**.

Nếu tay cầm đứng yên, Serial cũng đứng yên.

Có thể bật/tắt ở compile time:

```cpp
#define PS2_DEBUG_ENABLED 1

void setup() {
#if PS2_DEBUG_ENABLED
  Serial.begin(115200);
#endif

  ps2.begin(53);
}

void loop() {
  ps2.update();

#if PS2_DEBUG_ENABLED
  ps2.debug(Serial);
#endif
}
```

Đổi `PS2_DEBUG_ENABLED` thành `0` khi build production.

### Snapshot / raw analog test

```cpp
ps2.printState(Serial);
```

Mỗi lần gọi sẽ in một snapshot đầy đủ, vì vậy khi test raw analog nên rate-limit:

```cpp
static unsigned long lastPrint = 0;

if (millis() - lastPrint >= 100) {
  lastPrint = millis();
  ps2.printState(Serial);  // 10 dòng/giây
}
```

### Examples

Nếu mới dùng thư viện, chỉ cần bắt đầu với **2 example**:

| Example | Khi nào dùng |
|---|---|
| `BasicRead` | Học API production tối thiểu: `update()`, `connected()`, button, joystick; không Serial, không delay |
| `DebugMonitor` | Khi cần kiểm tra tay cầm thật trên Serial; chỉ in khi state thay đổi |

Sau đó mới dùng các example chuyên biệt:

| Example | Mục đích | Serial |
|---|---|---|
| `BasicSPI` | Học đấu dây và `begin(CS)` | Chỉ in kết quả init một lần |
| `ButtonEvents` | Phân biệt `button()`, `pressed()`, `released()` | Chỉ in edge event |
| `JoystickDirections` | Xem hướng LEFT/RIGHT joystick sau lọc | Chỉ in khi hướng thay đổi |
| `RawAnalogTest` | Xem raw + filtered + counters để tune | Rate-limit 10 Hz |
| `ConnectionRecovery` | Test rút/cắm receiver, fail-safe và reconnect | Chỉ in khi link đổi |
| `CustomSPI` | Chọn trực tiếp `SPI`, `SPI1`, `SPI2` | Không dùng |
| `LegacyBitBang` | Giữ wiring GPIO cũ CLK/CMD/CS/DAT | Không dùng |

### Ví dụ điều khiển robot nằm ở thư viện đế

`TungLam_PS2` chỉ chuẩn hóa input, không gắn cứng cách lái robot. Hai example điều khiển xe hoàn chỉnh được giữ ở repo **TungLam_OmniMecanum_4WD**:

- `PS2RobotControl`: khuyến nghị, joystick trái tiến/lùi/ngang; joystick phải LEFT/RIGHT có priority cao hơn để xoay.
- `PS2RobotVectorMix`: nâng cao, joystick trái tạo vx/vy và joystick phải tạo wz để vừa tịnh tiến vừa quay.

Cách tách này giúp thư viện PS2 vẫn dùng được cho robot khác, cơ cấu máy, gamepad test, ESP32 hay project không dùng đế Mecanum.

Hướng dẫn tiếng Việt đầy đủ: `docs/HUONG_DAN_SU_DUNG_VI.md`.

### Semantics của button edge

`pressed()` và `released()` là event theo frame mới. Chúng chỉ có hiệu lực trong vòng application loop ngay sau `update()` nhận được frame PS2 mới, nên loop chạy nhanh hơn 50 Hz cũng không làm một lần nhấn bị xử lý lặp lại.

## Poll rate và timing

Mặc định thư viện poll tay cầm ở **50 Hz**:

```text
50 Hz = 1 frame mỗi 20 ms
```

Người dùng vẫn gọi `ps2.update()` liên tục; thư viện tự quyết định khi nào mới phát transaction PS2 nên không cần `delay()` trong `loop()`.

Ba mức khuyến nghị:

```cpp
ps2.setPollRate(PS2PollRate::Hz20);   // 20 Hz = 50 ms
ps2.setPollRate(PS2PollRate::Hz50);   // 50 Hz = 20 ms, mặc định
ps2.setPollRate(PS2PollRate::Hz100);  // 100 Hz = 10 ms
```

Advanced:

```cpp
ps2.setPollRateHz(75);        // 1..200 Hz
ps2.setPollIntervalUs(20000); // đặt trực tiếp interval
```

SPI timing là khái niệm riêng với poll rate:

```cpp
ps2.setTimingProfile(PS2TimingProfile::Compatible);
ps2.setClockHz(250000);
ps2.setByteDelayUs(10);
```

Các profile SPI:

```cpp
PS2TimingProfile::Compatible
PS2TimingProfile::Balanced
PS2TimingProfile::Fast
```

## Phạm vi hiện tại

Bản đầu tiên tập trung vào:

- đọc controller ở digital/analog mode;
- đưa controller về analog mode;
- button hold/pressed/released;
- joystick filtering + discrete directions;
- reconnect;
- default SPI / custom SPI bus / BitBang.

Pressure buttons và rumble sẽ được thêm sau khi baseline này được test trên phần cứng thật.

## Tham khảo kỹ thuật

Thiết kế được nghiên cứu từ protocol PlayStation/PS2 và hành vi của nhiều implementation công khai như PS2X, PsxNewLib và các AVR PS2 controller drivers. Source của thư viện này được viết độc lập, không copy implementation từ các thư viện tham khảo.

Xem thêm `extras/REFERENCES.md`.

## License

MIT.
