# Hướng dẫn sử dụng TungLam_PS2

Tài liệu này đi theo đúng luồng từ **đấu dây → đọc cơ bản → button → joystick → debug → raw test → recovery**.

## 1. Code production tối thiểu

```cpp
#include <TungLam_PS2.h>

TungLamPS2 ps2;

void setup() {
  ps2.begin(10);
}

void loop() {
  ps2.update();

  if (!ps2.connected()) {
    return;
  }

  if (ps2.leftDirection() == PS2StickDirection::Up) {
    // robot.forward(...);
  }

  if (ps2.pressed(PS2Button::Cross)) {
    // action one-shot
  }
}
```

Không cần `delay()`. Poll mặc định là 50 Hz.

## 2. Ba kiểu đọc nút

```cpp
ps2.button(PS2Button::R1);     // đang giữ
ps2.pressed(PS2Button::Cross); // vừa nhấn, one-shot
ps2.released(PS2Button::Cross);// vừa nhả, one-shot
```

## 3. Joystick

Hướng đã lọc:

```cpp
ps2.leftDirection();
ps2.rightDirection();
```

Giá trị đã median-filter và trừ tâm:

```cpp
ps2.leftX();
ps2.leftY();
ps2.rightX();
ps2.rightY();
```

Raw 0..255:

```cpp
ps2.leftRawX();
ps2.leftRawY();
ps2.rightRawX();
ps2.rightRawY();
```

Quy ước trục:
- X dương: phải.
- X âm: trái.
- Y âm: lên.
- Y dương: xuống.

## 4. Debug không spam Serial

Production:

```cpp
ps2.update();
```

Không có log tự động.

Khi cần debug:

```cpp
ps2.update();
ps2.debug(Serial);
```

`debug()` chỉ in khi state thay đổi và gom vào một dòng:

```text
[PS2] LINK=CONNECTED_ANALOG | BTN=CROSS:PRESSED | LEFT=UP(1,-117)
```

## 5. Raw/service snapshot

```cpp
ps2.printState(Serial);
```

Hàm này luôn in khi gọi, nên cần rate-limit nếu đặt trong loop.

## 6. Poll rate

```cpp
ps2.setPollRate(PS2PollRate::Hz20);
ps2.setPollRate(PS2PollRate::Hz50);   // mặc định
ps2.setPollRate(PS2PollRate::Hz100);
```

Poll rate khác với SPI clock.

## 7. Hiệu chỉnh tâm

Để yên hai cần analog ở tâm rồi gọi một lần:

```cpp
ps2.calibrateCenter();
```

Không gọi liên tục trong loop.

## 8. Fail-safe

Luôn xử lý trường hợp:

```cpp
if (!ps2.connected()) {
  robot.stop();
  return;
}
```

Khi frame lỗi/mất link, thư viện neutralize input và tự recovery.
