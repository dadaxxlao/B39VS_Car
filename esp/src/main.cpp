#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HardwareSerial.h>

// --- WiFi 配置 ---
const char* WIFI_SSID = "S23";
const char* WIFI_PASS = "12345678";

// --- Arduino UART 配置 ---
#define ARDUINO_SERIAL_PORT 1 // 使用 ESP32 的 UART1
#define ARDUINO_RX_PIN 1      // ESP32 RX GPIO1 连接 Arduino TX2 (Pin 17 on Mega) - 确认接线!
#define ARDUINO_TX_PIN 0      // ESP32 TX GPIO0 连接 Arduino RX2 (Pin 16 on Mega) - 确认接线!
#define ARDUINO_BAUD_RATE 115200

// --- 全局对象 ---
HardwareSerial ArduinoSerial(ARDUINO_SERIAL_PORT);
AsyncWebServer server(80);

// --- HTML 页面内容 (PROGMEM 以节省内存) ---
const char HTML_PAGE_CONTENT[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 颜色控制器</title>
<style>
  body { font-family: Arial, sans-serif; padding: 20px; background-color: #f4f4f4; }
  .container { max-width: 400px; margin: auto; background: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); text-align: center; }
  h1 { color: #333; }
  button {
    background-color: #555;
    color: white;
    padding: 15px 20px;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    font-size: 16px;
    margin: 10px 5px;
    width: calc(50% - 15px); /* Two buttons per row approx */
    box-sizing: border-box;
    transition: background-color 0.3s;
  }
  button:hover { background-color: #777; }
  #status { margin-top: 20px; font-weight: bold; color: #333; }
  /* Specific button colors */
  button[data-color="1"] { background-color: #f44336; } /* Red */
  button[data-color="2"] { background-color: #4CAF50; } /* Green */
  button[data-color="3"] { background-color: #2196F3; } /* Blue */
  button[data-color="4"] { background-color: #ffeb3b; color: #333; } /* Yellow */
  button[data-color="5"] { background-color: #000000; } /* Black */
  button[data-color="1"]:hover { background-color: #d32f2f; }
  button[data-color="2"]:hover { background-color: #388E3C; }
  button[data-color="3"]:hover { background-color: #1976D2; }
  button[data-color="4"]:hover { background-color: #fbc02d; }
  button[data-color="5"]:hover { background-color: #424242; }
</style>
</head>
<body>
<div class="container">
  <h1>选择颜色发送给 Arduino</h1>
  <button data-color="1" onclick="sendColor(1)">白色 (1)</button>
  <button data-color="2" onclick="sendColor(2)">黑色 (2)</button>
  <button data-color="3" onclick="sendColor(3)">红色 (3)</button>
  <button data-color="4" onclick="sendColor(4)">蓝色 (4)</button>
  <button data-color="5" onclick="sendColor(5)">黄色 (5)</button>
  <p id="status">请选择一个颜色。</p>
</div>

<script>
function sendColor(code) {
  const statusElement = document.getElementById('status');
  statusElement.textContent = '正在发送颜色代码 ' + code + '...';
  console.log('Sending color code: ' + code); // Debug log in browser console

  fetch('/setcolor?code=' + code)
    .then(response => {
      if (!response.ok) {
        // Try to get error message from response body
        return response.text().then(text => { throw new Error('请求失败: ' + response.status + ' - ' + (text || '未知错误')); });
      }
      return response.text(); // Get success message
    })
    .then(data => {
      console.log('Server response:', data);
      statusElement.textContent = '颜色代码 ' + code + ' 已发送！';
    })
    .catch(error => {
      console.error('发送错误:', error);
      statusElement.textContent = '错误: ' + error.message;
    });
}
</script>
</body>
</html>
)rawliteral";


// --- Setup ---
void setup() {
  // 初始化日志串口
  Serial.begin(115200);
  while (!Serial);
  Serial.println("--- ESP32 WiFi 颜色控制器启动 ---");
  Serial.printf("连接 Arduino: UART%d, RX=%d, TX=%d, Baud=%d",
                ARDUINO_SERIAL_PORT, ARDUINO_RX_PIN, ARDUINO_TX_PIN, ARDUINO_BAUD_RATE);
  Serial.println("请务必确认 ESP32 的 TX(GPIO0) 连接 Arduino 的 RX2(16)，ESP32 的 RX(GPIO1) 连接 Arduino 的 TX2(17)！");

  // 初始化连接 Arduino 的串口
  ArduinoSerial.begin(ARDUINO_BAUD_RATE, SERIAL_8N1, ARDUINO_RX_PIN, ARDUINO_TX_PIN);
  delay(100); // 短暂等待串口稳定

  // 连接 WiFi
  Serial.printf("正在连接 WiFi: %s ...", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int connect_timeout = 20; // 等待 WiFi 连接的秒数
  while (WiFi.status() != WL_CONNECTED && connect_timeout > 0) {
    delay(1000);
    Serial.print(".");
    connect_timeout--;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi 连接成功!");
    Serial.print("IP 地址: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi 连接失败! 请检查 SSID 和密码。将无法使用网页控制。");
    // 这里可以选择停止执行或进入其他模式
  }

  // --- 配置 Web 服务器路由 ---

  // 根路径: 提供 HTML 页面
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("收到根路径请求 /");
    request->send_P(200, "text/html", HTML_PAGE_CONTENT);
  });

  // 设置颜色路径
  server.on("/setcolor", HTTP_GET, [](AsyncWebServerRequest *request){
    String responseMessage = "";
    int httpCode = 400; // Default to Bad Request

    if (request->hasParam("code")) {
      String codeStr = request->getParam("code")->value();
      // 尝试将参数转换为整数
      char *endptr;
      long colorCode_long = strtol(codeStr.c_str(), &endptr, 10);

      // 检查转换是否成功且没有多余字符，并且值在范围内
      if (*endptr == ' ' && codeStr.length() > 0 && colorCode_long >= 1 && colorCode_long <= 5) {
        int colorCode = (int)colorCode_long; // 安全转换
        ArduinoSerial.println(colorCode); // 发送给 Arduino (println 会添加换行符)
        Serial.printf("收到网页请求: 发送颜色代码 %d 到 Arduino (UART1)", colorCode);
        responseMessage = "颜色代码 " + codeStr + " 已发送到 Arduino。";
        httpCode = 200; // OK
      } else {
        Serial.printf("收到无效颜色代码请求: '%s'", codeStr.c_str());
        responseMessage = "无效的颜色代码: '" + codeStr + "'. 请输入 1 到 5 之间的数字。";
        httpCode = 400; // Bad Request
      }
    } else {
      Serial.println("收到 /setcolor 请求，但缺少 'code' 参数");
      responseMessage = "请求中缺少 'code' 参数。";
      httpCode = 400; // Bad Request
    }
    request->send(httpCode, "text/plain", responseMessage);
  });

  // 处理未找到的路由 (可选)
  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("收到未找到的请求: %s", request->url().c_str());
    request->send(404, "text/plain", "页面未找到");
  });

  // 启动服务器
  server.begin();
  Serial.println("HTTP 服务器已启动");
  Serial.println("-----------------------------------------");
  // 发送启动消息给 Arduino (可选)
  // ArduinoSerial.println("ESP32_Web_Ready");

} // end setup

// --- Loop ---
void loop() {
  // ESPAsyncWebServer 是异步的，大部分工作在后台处理
  // 这里可以保留读取 Arduino 消息的功能，用于调试

  if (ArduinoSerial.available() > 0) {
    String msgFromArduino = ArduinoSerial.readStringUntil('\n');
    msgFromArduino.trim(); // 去除可能的首尾空白字符
    if (msgFromArduino.length() > 0) {
        Serial.print("收到来自 Arduino (UART1): ");
        Serial.println(msgFromArduino);
    }
    // 注意：如果 Arduino 发送大量数据，readStringUntil 可能阻塞或内存不足
    // 对于简单确认消息，通常没问题
  }

  // 不需要 delay()
}