#ifdef TEST_ESP_COMMUNICATION
/**
 * ESP32 通信测试文件
 * 
 * 目的：验证 Arduino Mega 与 ESP32 通过 Serial2 的双向通信。
 * - Arduino -> ESP32 数据发送 (定时发送 + USB转发)
 * - ESP32 -> Arduino 数据接收
 * - Logger 通过 Serial2 输出
 * 
 * 设置：
 * - 将此文件放置在 'src/Test/' 目录。
 * - 修改 platformio.ini, 设置 build_flags = -D TEST_ESP_COMMUNICATION
 * - 确保已按照计划修改 Logger.h (添加 COMM_ESP32)。
 * - 确保 Config.h 中定义了 ESP_BAUD_RATE。
 * 
 * 硬件要求：
 * - Arduino Mega
 * - ESP32 (已烧录远程串口桥接固件)
 * - 正确连接：Mega TX2(16) <-> ESP32 RX, Mega RX2(17) <-> ESP32 TX, GND <-> GND
 * 
 * 操作步骤：
 * 1. 编译并上传此代码到 Arduino Mega。
 * 2. 连接 ESP32 并为其供电。
 * 3. 打开 Arduino Mega 的串口监视器 (波特率 115200)。
 * 4. 连接到 ESP32 的 WiFi，并在浏览器中打开 ESP32 的 IP 地址访问 Web 串口界面。
 * 5. 观察 Arduino 串口监视器的输出 (本地调试信息，从 ESP 接收的数据)。
 * 6. 观察 ESP32 Web 界面的输出 (Arduino 定时发送的数据，Logger 日志，从 USB 转发的数据)。
 * 7. 在 Arduino 串口监视器中输入文本并按回车，观察该文本是否出现在 ESP32 Web 界面。
 * 8. 在 ESP32 Web 界面中输入文本并发送，观察该文本是否出现在 Arduino 串口监视器。
 */

#include <Arduino.h>
#include "../Utils/Config.h"           // 需要 ESP_BAUD_RATE
#include "../Utils/Logger.h"           // 需要 Logger 类和 COMM_ESP32

// --- 全局变量 --- 
unsigned long lastSendMillis = 0;   // 用于定时发送
const long sendInterval = 5000;     // 定时发送间隔（毫秒）
unsigned long messageCounter = 0;   // 发送消息计数器

// --- 初始化函数 --- 
void setup() {
  // 1. 初始化本地 USB 串口 (Serial)
  Serial.begin(115200);
  while (!Serial);
  Serial.println("--- ESP32 Communication Test --- ");
  Serial.println("Initializing USB Serial (Serial)... OK");

  // 2. 初始化与 ESP32 通信的硬件串口 (Serial2)
  Serial2.begin(ESP_BAUD_RATE);
  Serial.print("Initializing Hardware Serial2 (Pins 16, 17) with Baud: ");
  Serial.println(ESP_BAUD_RATE);
  Serial.println("Initializing Serial2... OK");

  // 3. 初始化 Logger 系统
  Logger::init();
  Serial.println("Initializing Logger... OK");

  // 4. 配置 Logger 通过 Serial2 (COMM_ESP32) 输出
  Logger::setStream(COMM_ESP32, &Serial2);
  Logger::enableComm(COMM_ESP32, true);
  Logger::setLogLevel(COMM_ESP32, LOG_LEVEL_DEBUG); // 发送所有级别的日志到 ESP32
  Serial.println("Configuring Logger for COMM_ESP32 (Serial2)... OK");

  // 5. 配置 Logger 的 COMM_SERIAL (USB) 输出
  // 为了方便测试，我们同时在 USB 打印日志
  Logger::enableComm(COMM_SERIAL, true); // 确保启用 (虽然默认是启用)
  Logger::setLogLevel(COMM_SERIAL, LOG_LEVEL_DEBUG); // USB 也显示所有日志
  Serial.println("Logger COMM_SERIAL output enabled.");

  // 6. 打印操作说明到 USB 串口
  Serial.println("\n操作说明:");
  Serial.println(" - 输入文本并按回车，将通过 Serial2 发送给 ESP32。 ");
  Serial.println(" - 从 ESP32 通过 Serial2 收到的数据会显示在这里。 ");
  Serial.println(" - Arduino 会每隔 5 秒自动发送一条测试消息给 ESP32。 ");
  Serial.println(" - Logger 日志会同时输出到 USB(这里) 和 Serial2(ESP32)。 ");
  Serial.println("-----------------------------------------");

  // 7. 通过 Logger 发送启动消息 (会到 Serial 和 Serial2)
  Logger::info("EspComTest", "ESP32 Communication Test Started.");
}

// --- 主循环函数 --- 
void loop() {

  // 1. 检查并处理来自 ESP32 (Serial2) 的数据
  if (Serial2.available() > 0) {
    String receivedData = "";
    // 尝试读取整行或所有可用字节
    // 使用 readStringUntil 或逐字节读取取决于预期的数据格式
    receivedData = Serial2.readString(); // 读取所有可用字节
    
    Serial.print("[Serial2 RX]: "); // 在 USB 标注来源
    Serial.println(receivedData);
    
    // (可选) 通过 Logger 记录接收事件
    // 注意：这也会将日志消息发送回 Serial2
    // Logger::debug("EspComTest", "Received %d bytes via Serial2", receivedData.length());
  }

  // 2. 检查并处理来自 USB 串口监视器 (Serial) 的数据
  if (Serial.available() > 0) {
    String dataToSend = Serial.readStringUntil('\n');
    dataToSend.trim(); // 去除可能的回车换行符
    
    if (dataToSend.length() > 0) {
      Serial.print("[Serial TX -> Serial2]: "); // 在 USB 显示即将发送的内容
      Serial.println(dataToSend);
      
      Serial2.println(dataToSend); // 通过 Serial2 发送给 ESP32
      
      // 通过 Logger 记录转发事件
      Logger::debug("EspComTest", "Forwarded %d bytes from Serial to Serial2", dataToSend.length());
    }
  }

  // 3. 定时通过 Serial2 发送测试消息给 ESP32
  unsigned long currentMillis = millis();
  if (currentMillis - lastSendMillis >= sendInterval) {
    lastSendMillis = currentMillis; // 重置计时器
    
    messageCounter++;
    String testMessage = "Arduino Ping #" + String(messageCounter);
    
    Serial.print("[Serial2 TX]: "); // 在 USB 显示即将发送的内容
    Serial.println(testMessage);
    
    Serial2.println(testMessage); // 通过 Serial2 发送给 ESP32
    
    // 通过 Logger 记录发送事件
    Logger::info("EspComTest", "Sent automatic test message #%lu via Serial2", messageCounter);
  }
  
  // 4. 短暂延迟，避免循环过快，给串口处理时间
  delay(20); 

}

#endif // TEST_ESP_COMMUNICATION 