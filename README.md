# Esp32-BLE
# Mô Tả Hoạt Động

~ESP32 khởi tạo BLE với tên `cpp "ESP_BLE"` và tạo một service chứa 2 đặc tính: LED (đọc/ghi) và Sensor (chỉ đọc).

~Client BLE có thể ghi giá trị `cpp"1"` hoặc `cpp"0"` vào characteristic LED để bật/tắt chân GPIO2 (LED).

~Sensor characteristic luôn trả về giá trị `cpp"30.0"` khi client đọc.

~Trong `cpploop()`, ESP32 liên tục kiểm tra giá trị mới của LED characteristic và điều khiển chân GPIO2 tương ứng.

~Nếu giá trị thay đổi, ESP32 in giá trị mới lên Serial để theo dõi.

# CODE Đầy Đủ
```
cpp.
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <RF24.h>

#define SERVICE_UUID "e5d25641-6d62-467e-8194-c2a1b6f703f6"
#define LED_CHAR_UUID "127a319c-89be-41cb-bc70-5072119cbd75"
#define SENSOR_CHAR_UUID "fd776028-0926-41bb-aaa1-5b877daab630"

BLECharacteristic *pLedCharacteristic;
String x ="";

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

BLEDevice::init("ESP_BLE");
BLEServer *pServer= BLEDevice::createServer();
BLEService *pService= pServer -> createService(SERVICE_UUID);
pLedCharacteristic= pService -> createCharacteristic(LED_CHAR_UUID,
BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

BLECharacteristic *pSensorCharacteristic= pService -> createCharacteristic(SENSOR_CHAR_UUID,
BLECharacteristic::PROPERTY_READ);
pSensorCharacteristic -> setValue("30.0");

pService -> start();
BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
BLEDevice::startAdvertising();

Serial.print("BLE bắt đầu");


}

void loop() {
String n= pLedCharacteristic -> getValue();
if (n!=x){
  Serial.println(n);
}

 if(n=="1"){
 digitalWrite(2, HIGH);
 }else if(n=="0"){
  digitalWrite(2,LOW);
 }
x=n;
}
```
# Giải Thích Chi Tiết Từng Phần 
1) Thư viện và macros đầu file

```
   cpp
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <RF24.h>

```

`cppBLEDevice.h`
`cppBLEUtils.h`
`cppBLEServer.h`
`cppBLE2902.h`
các header của thư viện BLE (ESP32 BLE Arduino) — cung cấp API để khởi tạo thiết bị BLE, tạo service/characteristic, quản lý advertising, descriptor (BLE2902 thường dùng để bật Notify/Indicate).

`cppRF24.h` thư viện cho mô-đun nRF24L01 (radio 2.4GHz)
```
cpp
#define SERVICE_UUID "e5d25641-6d62-467e-8194-c2a1b6f703f6"
#define LED_CHAR_UUID "127a319c-89be-41cb-bc70-5072119cbd75"
#define SENSOR_CHAR_UUID "fd776028-0926-41bb-aaa1-5b877daab630"
```

Đây là UUID cho service và hai characteristic. BLE dùng UUID để phân biệt service/characteristic giữa client/server.
```
cpp
2) Biến toàn cục
BLECharacteristic *pLedCharacteristic;
String x ="";
```

`cpppLedCharacteristic`: con trỏ tới characteristic dùng để nhận/đọc/ghi trạng thái LED.

`cppString x = ""`: biến lưu giá trị trước đó của characteristic 

3) `cppsetup()` — khởi tạo ở startup
```
cpp
void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
```

`cppSerial.begin(115200)`: mở cổng Serial để debug (115200 bps).

`cpppinMode(2, OUTPUT)`: cấu hình chân GPIO2 là OUTPUT. Trên nhiều board ESP32, GPIO2 nối với LED tích hợp (on-board LED) — do đó dùng chân 2 để bật/tắt LED.

`cppdigitalWrite(2, LOW)`: đảm bảo LED tắt khi khởi động.
```
cpp
BLEDevice::init("ESP_BLE");
BLEServer *pServer= BLEDevice::createServer();
BLEService *pService= pServer -> createService(SERVICE_UUID);
```

`cppBLEDevice::init("ESP_BLE")`: khởi tạo stack BLE và đặt tên thiết bị (device name) hiển thị khi client scan.

`cppcreateServer()`: tạo một BLE server (ESP32 đóng vai server).

`cppcreateService(SERVICE_UUID)`: tạo service với UUID đã định nghĩa.
```
cpp

pLedCharacteristic= pService -> createCharacteristic(LED_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
```

Tạo characteristic cho LED với quyền READ và WRITE — tức client có thể đọc giá trị hiện tại và ghi giá trị mới (để điều khiển LED).

`cpppLedCharacteristic` giữ con trỏ tới đối tượng này để bạn kiểm tra/get value trong loop().
```
cpp
BLECharacteristic *pSensorCharacteristic= pService -> createCharacteristic(SENSOR_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ);
pSensorCharacteristic -> setValue("30.0");
```

Tạo characteristic cảm biến (sensor) chỉ READ. Bạn đặt giá trị ban đầu `cpp"30.0"` — client khi đọc sẽ nhận `cpp"30.0"`.

```
cpp
pService -> start();
BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
BLEDevice::startAdvertising();

Serial.print("BLE bắt đầu");
}
```

`cpppService->start()`: kích hoạt service để BLE stack có thể phục vụ client.

`cppBLEDevice::getAdvertising()` trả về con trỏ advertising (ở đây biến pAdvertising không dùng thêm — nhưng có thể dùng để cấu hình quảng cáo).

`cppBLEDevice::startAdvertising()`: bắt đầu quảng cáo (device sẽ xuất hiện trong danh sách scan của client).

`cppSerial.print("BLE bắt đầu")`: in thông báo bắt đầu BLE.


4) `cpploop()` — vòng lặp chính
```
cpp
void loop() {
  String n= pLedCharacteristic -> getValue();
  if (n!=x){
    Serial.println(n);
  }
 
  if(n=="1"){
    digitalWrite(2, HIGH);
  } else if(n=="0"){
    digitalWrite(2,LOW);
  }
  x=n;
}
```

`cpppLedCharacteristic->getValue()` trả về giá trị hiện tại của characteristic Giá trị này là giá trị client ghi lên.

`cppif (n != x) { Serial.println(n); }`: nếu giá trị mới khác với giá trị cũ x, in ra Serial — tránh in lặp nhiều lần khi giá trị không đổi.

`cppif(n=="1"){ digitalWrite(2, HIGH); } else if(n=="0"){ digitalWrite(2,LOW); }`: nếu giá trị bằng `cpp"1"` thì bật LED, bằng `cpp"0"` thì tắt. Đây là cách đơn giản để client điều khiển LED bằng cách ghi `cpp"1"` hoặc `cpp"0"` vào characteristic.

`cppx = n;`: cập nhật `cppx `để so sánh lần sau.

# Kết quả thử nghiệm sau khi chạy 

Khi app BLE gửi `cpp"1"` → LED trên chân GPIO2 sáng và Serial in `cpp1`.

Khi gửi `cpp"0"` → LED tắt và Serial in `cpp0`.

Khi đọc sensor từ app BLE → nhận giá trị `cpp"30.0"` đúng như setValue ban đầu.

Thiết bị xuất hiện khi scan BLE với tên `cpp"ESP_BLE"` và kết nối ổn định.
