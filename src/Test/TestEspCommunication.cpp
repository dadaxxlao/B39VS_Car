#ifdef TEST_ESP_COMMUNICATION
#include <Arduino.h>
#include "../Utils/Config.h"
// 定义与 ESP32 通信的波特率
#define ESP_BAUD_RATE 115200 



// --- 全局变量 ---
ColorCode m_detectedColorCode = COLOR_UNKNOWN; // 用于存储检测到的颜色

// --- 测试函数 (从 SimpleStateMachine::readColorCodeFromSerial2 复制并修改) ---
ColorCode readColorCodeFromSerial2_test(unsigned long timeoutMillis = 20000) {
    Serial.println("readColorCodeFromSerial2_test: 开始尝试读取颜色代码...");
    String colorStr = "";
    unsigned long startTime = millis();
    bool receivedData = false;
    ColorCode detectedColor = COLOR_UNKNOWN; // 默认值

    while (millis() - startTime < timeoutMillis) {
        if (Serial2.available() > 0) {
            receivedData = true;
            char c = Serial2.read();
             Serial.print(c); // 实时回显

            if (c == '\n' || c == '\r') {
                if (colorStr.length() > 0) {
                    // Serial.print("\nreadColorCodeFromSerial2_test: 收到结束符，字符串为: '"); Serial.print(colorStr); Serial.println("'");
                    break; // 收到结束符且字符串非空，结束读取
                } else {
                    // 忽略开头的或连续的换行/回车符
                    continue;
                }
            }
            // 只添加数字字符到字符串
            if (isdigit(c)) {
                 colorStr += c;
            } else {
                 // Serial.print("\nreadColorCodeFromSerial2_test: 收到非数字字符 '"); Serial.print(isprint(c) ? c : '.'); Serial.print("' (ASCII: "); Serial.print(c); Serial.println(")，已忽略。");
            }
        } 
        // 不需要delay
    }

    if (colorStr.length() > 0) {
        Serial.println("\nreadColorCodeFromSerial2_test: 读取完成，最终字符串: '" + colorStr + "'");
        int colorValue = colorStr.toInt();
        Serial.println("readColorCodeFromSerial2_test: 转换为整数: " + String(colorValue));

        if (colorValue >= 1 && colorValue <= 5) {
            detectedColor = static_cast<ColorCode>(colorValue);
            Serial.println("readColorCodeFromSerial2_test: 解析到有效颜色代码: " + String(detectedColor));
        } else {
            Serial.println("readColorCodeFromSerial2_test: 收到无效或超出范围的颜色代码值: " + String(colorValue) + " (来自字符串 '" + colorStr + "')");
            detectedColor = COLOR_UNKNOWN;
        }
    } else if (receivedData) {
        Serial.println("\nreadColorCodeFromSerial2_test: 只收到了换行/回车或非数字字符，未读取到有效颜色代码。");
        detectedColor = COLOR_UNKNOWN;
    } else {
        // 超时
        Serial.println("\nreadColorCodeFromSerial2_test: 读取颜色代码超时 (" + String(timeoutMillis) + " ms)，未收到有效数据。");
        detectedColor = COLOR_UNKNOWN;
    }

    Serial.println("readColorCodeFromSerial2_test: 函数返回: " + String(detectedColor));
    return detectedColor;
}

void setup() {
  // 初始化 USB 串口 (Serial) 用于调试输出
  Serial.begin(115200);
  while (!Serial); // 等待串口连接
  Serial.println("--- Serial2 颜色代码读取测试 ---");

  // 初始化 Serial2 用于与 ESP32 通信
  Serial2.begin(ESP_BAUD_RATE);
  Serial.print("初始化 Serial2 (引脚 16, 17)，波特率: ");
  Serial.println(ESP_BAUD_RATE);
  Serial.println("等待来自 Serial2 的颜色代码 (数字 1-5 后跟换行符)...");
  Serial.println("从 Serial 输入的数据将转发到 Serial2");
  Serial.println("-----------------------------------------");
}

void loop() {
  // 检查 Serial2 是否有数据传入 (来自 ESP32)
  if (Serial2.available() > 0) {
      Serial.println("检测到 Serial2 输入，调用 readColorCodeFromSerial2_test()...");
      
      // 调用测试函数读取颜色代码
      ColorCode receivedColor = readColorCodeFromSerial2_test();
      
      // 处理和显示结果
      Serial.print("Loop: readColorCodeFromSerial2_test 返回: ");
      switch(receivedColor) {
          case COLOR_WHITE: Serial.println("白色 (1)"); break;
          case COLOR_BLACK: Serial.println("黑色 (2)"); break;
          case COLOR_RED: Serial.println("红色 (3)"); break;
          case COLOR_BLUE: Serial.println("蓝色 (4)"); break;
          case COLOR_YELLOW: Serial.println("黄色 (5)"); break;
          default: Serial.println("未知/无效 (0)"); break;
      }
      m_detectedColorCode = receivedColor; // 更新全局变量 (如果需要)
      
      Serial.println("-----------------------------------------");
      Serial.println("等待下一次来自 Serial2 的颜色代码...");

  } // end if (Serial2.available() > 0)

  // 检查 Serial (USB) 是否有数据传入 (可选，用于从电脑发送数据到ESP32)
  if (Serial.available() > 0) {
    byte outgoingByte = Serial.read();
    // 将从 Serial 读取到的字节直接发送到 Serial2 (ESP32)
    Serial.print("转发到 Serial2: ");
    Serial.write(outgoingByte); // 在 Serial 上也显示一下发送的内容
    Serial2.write(outgoingByte);
  }

}
#endif // TEST_ESP_COMMUNICATION