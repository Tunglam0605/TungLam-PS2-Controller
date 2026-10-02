#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "TungLam_PS2_Filter.h"

/**
 * @brief Danh sách đầy đủ 16 nút số của tay cầm PS2.
 *
 * Giá trị enum là bit-mask nội bộ đã được chuyển sang logic dương:
 * button(...) == true nghĩa là nút đang được nhấn.
 */
enum class PS2Button : uint16_t {
  Select   = 0x0001,  ///< Nút SELECT.
  L3       = 0x0002,  ///< Nhấn cần analog trái.
  R3       = 0x0004,  ///< Nhấn cần analog phải.
  Start    = 0x0008,  ///< Nút START.
  Up       = 0x0010,  ///< D-pad lên.
  Right    = 0x0020,  ///< D-pad phải.
  Down     = 0x0040,  ///< D-pad xuống.
  Left     = 0x0080,  ///< D-pad trái.
  L2       = 0x0100,  ///< Nút L2.
  R2       = 0x0200,  ///< Nút R2.
  L1       = 0x0400,  ///< Nút L1.
  R1       = 0x0800,  ///< Nút R1.
  Triangle = 0x1000,  ///< Nút tam giác.
  Circle   = 0x2000,  ///< Nút tròn.
  Cross    = 0x4000,  ///< Nút X / Cross.
  Square   = 0x8000   ///< Nút vuông.
};

/**
 * @brief Trạng thái kết nối/cấu hình hiện tại của controller.
 *
 * Dùng status() khi cần chẩn đoán sâu hơn connected().
 */
enum class PS2ConnectionStatus : uint8_t {
  Disconnected = 0,   ///< Chưa có liên kết hợp lệ.
  Probing,             ///< Đang thăm dò controller.
  Configuring,         ///< Đang gửi chuỗi cấu hình analog mode.
  ConnectedDigital,    ///< Nhận được controller ở digital mode.
  ConnectedAnalog,     ///< Kết nối analog mode bình thường.
  ConnectedPressure,   ///< Kết nối pressure/extended analog mode.
  Recovering,          ///< Đang chờ/thử phục hồi sau lỗi link.
  NoResponse,          ///< Không nhận được frame phản hồi hợp lệ.
  ConfigRejected       ///< Controller phản hồi nhưng không vào analog mode.
};

/**
 * @brief Profile timing của transaction PS2.
 *
 * Profile này điều chỉnh SPI clock và khoảng nghỉ giữa byte,
 * KHÔNG phải tần số poll controller.
 */
enum class PS2TimingProfile : uint8_t {
  Compatible = 0,  ///< Chậm nhất, ưu tiên receiver clone/khó tính.
  Balanced,        ///< Cân bằng tốc độ và độ tương thích.
  Fast             ///< Nhanh hơn; chỉ dùng khi phần cứng đã test ổn định.
};

/**
 * Tần số poll tay cầm ở tầng application.
 *
 * Đây là khoảng cách giữa hai frame đọc tay cầm, không phải SPI clock.
 */
enum class PS2PollRate : uint8_t {
  Hz20 = 20,   ///< 20 lần/giây, chu kỳ 50 ms.
  Hz50 = 50,   ///< 50 lần/giây, chu kỳ 20 ms. Đây là mặc định.
  Hz100 = 100  ///< 100 lần/giây, chu kỳ 10 ms.
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
  /**
   * @brief Tạo đối tượng PS2 ở trạng thái an toàn/chưa kết nối.
   *
   * Constructor không truy cập GPIO/SPI. Phần cứng chỉ bắt đầu hoạt động
   * khi gọi begin(...) hoặc beginBitBang(...).
   */
  TungLamPS2();

  /**
   * @brief Khởi tạo PS2 bằng hardware SPI mặc định của board.
   *
   * Chỉ cần truyền chân CS/ATT. Ba chân còn lại dùng SPI mặc định
   * do Arduino Core của board cung cấp.
   *
   * Sơ đồ tín hiệu:
   *   PS2 DAT / MISO  -> MISO của board
   *   PS2 CMD / MOSI  -> MOSI của board
   *   PS2 CLK / SCK   -> SCK của board
   *   PS2 CS / ATT    -> csPin
   *   PS2 GND         -> GND chung
   *   PS2 VCC         -> nguồn đúng theo module receiver
   *
   * Ví dụ Arduino Mega 2560:
   *   DAT/MISO -> D50
   *   CMD/MOSI -> D51
   *   CLK/SCK  -> D52
   *   CS/ATT   -> D53 nếu gọi begin(53)
   *
   * Bảng đấu nối nhanh với SPI mặc định:
   *
   * @verbatim
   * Board / dòng Arduino     DAT / MISO(CIPO)   CMD / MOSI(COPI)   CLK / SCK   CS gợi ý
   * --------------------------------------------------------------------------------------
   * Mega 2560 Rev3          D50                D51                D52         D53
   * UNO R3                  D12                D11                D13         D10
   * Nano classic            D12                D11                D13         D10
   * UNO R4 Minima / WiFi    D12                D11                D13         D10
   * Nano 33 IoT             D12                D11                D13         D10
   * Nano 33 BLE             D12                D11                D13         D10
   * MKR WiFi 1010           D10                D8                 D9          D7
   * Micro                    D14                D16                D15         D17
   * Leonardo                 ICSP CIPO          ICSP COPI          ICSP SCK    D10
   * @endverbatim
   *
   * CS gợi ý chỉ là lựa chọn thuận tiện. Có thể dùng GPIO digital khác
   * nếu không xung đột với phần cứng/project.
   *
   * Lưu ý: thư viện không hard-code chân MISO/MOSI/SCK. Với board khác,
   * xem pinout của board để xác định các chân SPI mặc định.
   *
   * @param csPin Chân GPIO nối với CS/ATT/SEL của đầu thu PS2.
   * @return true nếu cấu hình và giao tiếp với controller thành công.
   */
  bool begin(uint8_t csPin) {
    return begin(SPI, csPin);
  }

  /**
   * @brief Khởi tạo PS2 bằng một SPI bus cụ thể.
   *
   * Dùng khi board có nhiều SPI bus, ví dụ SPI, SPI1 hoặc SPI2.
   * MISO/MOSI/SCK phải đấu theo đúng pin mapping của bus được truyền vào.
   *
   * Sơ đồ tín hiệu:
   *   PS2 DAT / MISO  -> MISO của bus
   *   PS2 CMD / MOSI  -> MOSI của bus
   *   PS2 CLK / SCK   -> SCK của bus
   *   PS2 CS / ATT    -> csPin
   *   PS2 GND         -> GND chung
   *   PS2 VCC         -> nguồn đúng theo module receiver
   *
   * Ví dụ:
   *   ps2.begin(SPI1, 7);
   *
   * Khi đó DAT/CMD/CLK phải nối vào MISO/MOSI/SCK của SPI1,
   * còn CS/ATT nối vào D7.
   *
   * Template giúp API dùng được với nhiều Arduino Core có kiểu class SPI
   * khác nhau mà không hard-code theo từng kiến trúc.
   *
   * @param bus Đối tượng SPI bus muốn sử dụng.
   * @param csPin Chân GPIO nối với CS/ATT/SEL của đầu thu PS2.
   * @return true nếu cấu hình và giao tiếp với controller thành công.
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
   * @brief Khởi tạo PS2 bằng 4 GPIO tùy ý theo kiểu BitBang.
   *
   * Dùng khi muốn tự chọn chân hoặc cần tương thích wiring cũ.
   * Thứ tự tham số: CLK, CMD/MOSI, CS/ATT, DAT/MISO.
   *
   * Sơ đồ tín hiệu:
   *   PS2 CLK / SCK   -> clockPin
   *   PS2 CMD / MOSI  -> commandPin
   *   PS2 CS / ATT    -> csPin
   *   PS2 DAT / MISO  -> dataPin
   *   PS2 GND         -> GND chung
   *   PS2 VCC         -> nguồn đúng theo module receiver
   *
   * Ví dụ wiring RoboBall cũ:
   *   ps2.beginBitBang(22, 26, 24, 28);
   *
   *   CLK -> D22
   *   CMD -> D26
   *   CS  -> D24
   *   DAT -> D28
   *
   * @param clockPin Chân CLK/SCK.
   * @param commandPin Chân CMD/MOSI.
   * @param csPin Chân CS/ATT/SEL.
   * @param dataPin Chân DAT/MISO.
   * @return true nếu cấu hình và giao tiếp với controller thành công.
   */
  bool beginBitBang(uint8_t clockPin,
                    uint8_t commandPin,
                    uint8_t csPin,
                    uint8_t dataPin);

  /**
   * @brief Cập nhật trạng thái tay cầm; cần gọi liên tục trong loop().
   *
   * Ở trạng thái kết nối bình thường, hàm dùng scheduler nội bộ bằng micros():
   * loop() có thể chạy rất nhanh nhưng transaction PS2 chỉ phát theo poll rate
   * (mặc định 50 Hz). Vì vậy code người dùng không cần delay().
   *
   * Khi mất kết nối, input được neutralize ngay và thư viện tự recovery.
   * Quá trình cấu hình/recovery có thể chứa các khoảng chờ ngắn cần thiết
   * cho protocol, nhưng đường chạy bình thường không chặn theo poll interval.
   *
   * @return true khi controller hiện đang có liên kết analog hợp lệ;
   *         false khi mất kết nối/đang recovery.
   */
  bool update();

  /**
   * @brief Kiểm tra controller có đang sẵn sàng cho điều khiển hay không.
   * @return true khi đang kết nối analog/pressure hợp lệ.
   */
  bool connected() const;

  /**
   * @brief Đọc trạng thái chi tiết của state machine kết nối.
   * @return Một giá trị PS2ConnectionStatus.
   */
  PS2ConnectionStatus status() const;

  /** @brief Tổng số frame hợp lệ đã cập nhật vào public state. */
  uint32_t packetCount() const;

  /** @brief Tổng số lỗi frame/link đã phát hiện. */
  uint32_t errorCount() const;

  /** @brief Tổng số lần thư viện bắt đầu thử reconnect. */
  uint16_t reconnectCount() const;

  /**
   * @brief Kiểm tra một nút đang được giữ hay không.
   * @param key Nút cần kiểm tra.
   * @return true trong toàn bộ thời gian nút đang được giữ.
   *
   * Ví dụ: if (ps2.button(PS2Button::R1)) { ... }
   */
  bool button(PS2Button key) const;

  /**
   * @brief Bắt cạnh nhấn của một nút (one-shot PRESSED event).
   * @param key Nút cần kiểm tra.
   * @return true đúng một vòng application loop sau frame mới phát hiện nhấn.
   *
   * Phù hợp cho toggle mode, đóng/mở cơ cấu, Start/Stop...
   */
  bool pressed(PS2Button key) const;

  /**
   * @brief Bắt cạnh nhả của một nút (one-shot RELEASED event).
   * @param key Nút cần kiểm tra.
   * @return true đúng một vòng application loop sau frame mới phát hiện nhả.
   */
  bool released(PS2Button key) const;

  /**
   * @brief Lấy toàn bộ 16 nút dưới dạng bit-mask logic dương.
   * @return Bit = 1 nghĩa là nút tương ứng đang được giữ.
   *
   * Đây là API nâng cao; code thông thường nên dùng button/pressed/released.
   */
  uint16_t buttons() const;

  /**
   * @brief In debug theo sự kiện, không spam Serial.
   *
   * Hàm này KHÔNG được gọi tự động bởi update(). Chỉ gọi trong loop()
   * khi đang debug:
   *
   *   ps2.update();
   *   ps2.debug(Serial);
   *
   * Chỉ in khi có thay đổi đáng chú ý:
   * Mỗi lần có thay đổi chỉ in MỘT dòng tổng hợp, ví dụ:
   *
   *   [PS2] LINK=CONNECTED_ANALOG | BTN=CROSS:PRESSED
   *         | LEFT=UP(1,-117) | RIGHT=CENTER(0,2) | ERR=0 | REC=0
   *
   * Trên Serial Monitor dòng có thể tự wrap theo chiều rộng cửa sổ,
   * nhưng thư viện chỉ phát một newline cho mỗi event snapshot.
   *
   * Nếu không gọi debug(), core driver không tạo Serial output.
   *
   * @param output Stream nhận log, ví dụ Serial, Serial1...
   */
  void debug(Stream& output);

  /**
   * @brief In một snapshot đầy đủ của trạng thái controller.
   *
   * Đây là hàm one-shot: mỗi lần gọi sẽ in một dòng. Dùng cho test
   * hoặc raw analog inspection; không nên gọi không giới hạn trong loop().
   *
   * @param output Stream nhận log, ví dụ Serial.
   */
  void printState(Stream& output) const;

  /**
   * @brief Reset baseline của debug event monitor.
   *
   * Lần debug() tiếp theo sẽ in lại snapshot ban đầu.
   */
  void resetDebug();

  /**
   * @brief Lấy hướng rời rạc của joystick trái sau lọc.
   * @return Center/Up/Down/Left/Right/Unknown.
   *
   * Unknown dùng cho vùng chéo/mơ hồ hoặc chuyển tiếp, giúp tránh giữ lệnh cũ.
   */
  PS2StickDirection leftDirection() const;

  /** @brief Lấy hướng rời rạc của joystick phải sau lọc. */
  PS2StickDirection rightDirection() const;

  /**
   * @brief Trục X joystick trái sau median-filter và trừ tâm.
   * @return Giá trị có dấu xấp xỉ -128..127; dương = sang phải.
   */
  int16_t leftX() const;

  /**
   * @brief Trục Y joystick trái sau median-filter và trừ tâm.
   * @return Giá trị có dấu xấp xỉ -128..127; âm = đẩy lên, dương = kéo xuống.
   */
  int16_t leftY() const;

  /** @brief Trục X joystick phải sau lọc/trừ tâm; dương = sang phải. */
  int16_t rightX() const;

  /** @brief Trục Y joystick phải sau lọc/trừ tâm; âm = lên, dương = xuống. */
  int16_t rightY() const;

  /** @brief Giá trị raw LX gần nhất từ frame PS2, 0..255. */
  uint8_t leftRawX() const;

  /** @brief Giá trị raw LY gần nhất từ frame PS2, 0..255. */
  uint8_t leftRawY() const;

  /** @brief Giá trị raw RX gần nhất từ frame PS2, 0..255. */
  uint8_t rightRawX() const;

  /** @brief Giá trị raw RY gần nhất từ frame PS2, 0..255. */
  uint8_t rightRawY() const;

  /**
   * @brief Áp dụng cùng một cấu hình lọc cho cả hai joystick.
   * @param config Deadzone, ngưỡng vào/ra, giới hạn trục phụ và stable samples.
   *
   * Người dùng cơ bản không cần gọi hàm này; giá trị mặc định đã ưu tiên
   * tay cầm PS2 clone/wireless dùng điều khiển robot.
   */
  void setStickFilter(const PS2StickFilterConfig& config);

  /**
   * @brief Đặt tâm thủ công cho bốn trục analog.
   * @param leftX Tâm raw LX, thường gần 128.
   * @param leftY Tâm raw LY, thường gần 128.
   * @param rightX Tâm raw RX, thường gần 128.
   * @param rightY Tâm raw RY, thường gần 128.
   */
  void setStickCenters(uint8_t leftX,
                       uint8_t leftY,
                       uint8_t rightX,
                       uint8_t rightY);

  /**
   * @brief Tự đo tâm cho cả hai joystick.
   *
   * Khi gọi phải THẢ cả hai cần analog ở vị trí tự nhiên, không chạm tay.
   * Hàm lấy nhiều frame rồi tính trung bình và cập nhật center.
   * Đây là thao tác setup/service, có chờ giữa mẫu; không gọi liên tục trong loop().
   *
   * @param samples Số mẫu yêu cầu, mặc định 16.
   * @param sampleIntervalMs Khoảng cách giữa các mẫu, mặc định 8 ms.
   * @return true nếu thu đủ số frame hợp lệ để hiệu chỉnh.
   */
  bool calibrateCenter(uint8_t samples = 16,
                       uint16_t sampleIntervalMs = 8);

  /**
   * @brief Chọn profile timing của transaction PS2.
   * @param profile Compatible/Balanced/Fast.
   *
   * Compatible là lựa chọn an toàn khi chưa biết chất lượng receiver.
   */
  void setTimingProfile(PS2TimingProfile profile);

  /**
   * @brief Đặt SPI/bit-bang clock thủ công.
   * @param clockHz Tần số clock tính bằng Hz.
   * @warning Clock quá cao có thể làm receiver clone mất frame.
   */
  void setClockHz(uint32_t clockHz);

  /**
   * @brief Đặt khoảng nghỉ giữa hai byte trong một transaction.
   * @param byteDelayUs Thời gian microsecond.
   */
  void setByteDelayUs(uint16_t byteDelayUs);

  /**
   * @brief Chọn nhanh tần số đọc controller 20/50/100 Hz.
   * @param rate Preset mong muốn.
   *
   * Mặc định là 50 Hz = 20 ms/frame. update() vẫn gọi liên tục;
   * scheduler nội bộ mới quyết định lúc nào phát transaction.
   */
  void setPollRate(PS2PollRate rate);

  /**
   * @brief Đặt poll rate tùy ý.
   * @param rateHz Tần số từ 1..200 Hz.
   * @return false nếu ngoài khoảng 1..200 Hz; true nếu đã áp dụng.
   */
  bool setPollRateHz(uint16_t rateHz);

  /**
   * @brief Đặt trực tiếp chu kỳ poll theo microsecond.
   * @param pollIntervalUs Khoảng thời gian >0 us giữa hai lần poll.
   *
   * API nâng cao; thông thường nên dùng setPollRate().
   */
  void setPollIntervalUs(uint32_t pollIntervalUs);

  /**
   * @brief Đặt chu kỳ poll theo millisecond để tương thích code cũ.
   * @param pollIntervalMs Khoảng thời gian >0 ms.
   */
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
  uint32_t pollIntervalUs_;
  uint16_t recoveryIntervalMs_;

  uint8_t packet_[21];
  uint8_t mode_;

  uint16_t buttons_;
  uint16_t previousButtons_;
  uint16_t pressedButtons_;
  uint16_t releasedButtons_;
  bool buttonHistoryValid_;

  TungLamPS2StickFilter leftStick_;
  TungLamPS2StickFilter rightStick_;

  bool connected_;
  PS2ConnectionStatus status_;

  uint32_t packetCount_;
  uint32_t errorCount_;
  uint16_t reconnectCount_;

  bool debugInitialized_;
  PS2ConnectionStatus debugLastStatus_;
  PS2StickDirection debugLastLeftDirection_;
  PS2StickDirection debugLastRightDirection_;
  uint32_t debugLastErrorCount_;
  uint16_t debugLastReconnectCount_;

  unsigned long lastPollUs_;
  unsigned long lastRecoveryMs_;
};
