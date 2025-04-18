#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HardwareSerial.h>
#include <AsyncWebSocket.h>

// --- WiFi 配置 ---
const char* WIFI_SSID = "S23";
const char* WIFI_PASS = "12345678";

// --- Arduino UART 配置 ---
#define ARDUINO_SERIAL_PORT 1 // 使用 ESP32 的 UART1
#define ARDUINO_RX_PIN 1      // ESP32 RX GPIO1 连接 Arduino TX2 (Pin 16 on Mega) - 确认接线!
#define ARDUINO_TX_PIN 0      // ESP32 TX GPIO0 连接 Arduino RX2 (Pin 17 on Mega) - 确认接线!
#define ARDUINO_BAUD_RATE 115200

// --- 全局对象 ---
HardwareSerial ArduinoSerial(ARDUINO_SERIAL_PORT);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // WebSocket服务器实例，监听 /ws 路径

// --- HTML 页面内容 ---

// 主页面 HTML (添加 WebSocket 串口监视器 UI 和 JS)
const char MAIN_PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>上位机控制面板</title>
<style>
  body { font-family: Arial, sans-serif; padding: 20px; background-color: #f4f4f4; }
  .container { max-width: 800px; margin: auto; background: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
  h1 { color: #333; text-align: center; }
  h2 { color: #444; margin-top: 30px; border-bottom: 1px solid #eee; padding-bottom: 5px; }
  p { color: #555; line-height: 1.6; }
  a { color: #007bff; text-decoration: none; }
  a:hover { text-decoration: underline; }
  .status, .serial-monitor { margin-top: 15px; padding: 15px; background-color: #e9ecef; border-radius: 4px; }
  #serialOutput { width: 98%; height: 200px; margin-bottom: 10px; border: 1px solid #ccc; background-color: #fff; font-family: monospace; font-size: 13px; overflow-y: scroll; padding: 5px; }
  #serialInput { width: calc(100% - 80px); padding: 8px; border: 1px solid #ccc; border-radius: 4px; margin-right: 5px; }
  button { padding: 8px 15px; background-color: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer; }
  button:hover { background-color: #0056b3; }
</style>
</head>
<body>
<div class="container">
  <h1>上位机 控制面板</h1>
  
  <div class="status">
    <h2>系统状态</h2>
    <p><strong>WiFi 状态:</strong> <span id="wifiStatus">%WIFI_STATUS%</span></p>
    <p><strong>IP 地址:</strong> <span id="ipAddress">%IP_ADDRESS%</span></p>
    <p><strong>WebSocket:</strong> <span id="wsStatus">未连接</span></p>
  </div>

  <div class="serial-monitor">
    <h2>远程串口 (Arduino - Serial2)</h2>
    <textarea id="serialOutput" readonly></textarea>
    <input type="text" id="serialInput" placeholder="输入要发送到 Arduino 的数据...">
    <button onclick="sendSerialData()">发送</button>
  </div>

  <p style="text-align: center; margin-top: 20px;">
    <a href="/color">0</a>
  </p>
</div>

<script>
  let gateway = `ws://${window.location.hostname}/ws`;
  let websocket;
  const serialOutput = document.getElementById('serialOutput');
  const serialInput = document.getElementById('serialInput');
  const wsStatusElement = document.getElementById('wsStatus');

  function initWebSocket() {
    console.log('尝试连接 WebSocket...');
    wsStatusElement.textContent = '正在连接...';
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
    websocket.onerror   = onError;
  }

  function onOpen(event) {
    console.log('WebSocket 连接已打开');
    wsStatusElement.textContent = '已连接';
    wsStatusElement.style.color = 'green';
  }

  function onClose(event) {
    console.log('WebSocket 连接已关闭');
    wsStatusElement.textContent = '已断开';
    wsStatusElement.style.color = 'red';
    setTimeout(initWebSocket, 2000); // 尝试 2 秒后重连
  }

  function onMessage(event) {
    console.log('收到消息:', event.data);
    // 尝试将收到的数据（可能是 Blob）转为文本
    if (event.data instanceof Blob) {
        let reader = new FileReader();
        reader.onload = function() {
            appendSerialOutput(reader.result);
        };
        reader.onerror = function(e) {
            console.error("FileReader Error: ", e);
             appendSerialOutput(`[Error reading Blob: ${e}]\n`);
        }
        reader.readAsText(event.data); // 假设是文本数据
    } else {
        // 如果不是 Blob，直接作为文本处理
        appendSerialOutput(event.data);
    }
  }

 function appendSerialOutput(text) {
    serialOutput.value += text; // 直接追加文本
    // 自动滚动到底部
    serialOutput.scrollTop = serialOutput.scrollHeight;
 }

  function onError(event) {
    console.error('WebSocket 错误:', event);
    wsStatusElement.textContent = '错误';
     wsStatusElement.style.color = 'orange';
  }

  function sendSerialData() {
    const dataToSend = serialInput.value;
    if (dataToSend && websocket && websocket.readyState === WebSocket.OPEN) {
      console.log('发送数据:', dataToSend);
      websocket.send(dataToSend);
      serialInput.value = ''; // 清空输入框
    } else {
        console.log('WebSocket 未连接或输入为空');
        if (!websocket || websocket.readyState !== WebSocket.OPEN) {
             wsStatusElement.textContent = '未连接，无法发送';
             wsStatusElement.style.color = 'red';
        }
    }
  }

  // 处理 Enter 键发送
  serialInput.addEventListener('keyup', function(event) {
      if (event.key === 'Enter') {
          event.preventDefault(); // 防止可能的表单提交
          sendSerialData();
      }
  });

  // 页面加载时初始化 WebSocket
  window.addEventListener('load', initWebSocket);

  // 更新主页面的 WiFi 状态和 IP (如果它们是通过模板动态设置的)
  // 如果 processor 函数正常工作, 这些可能不需要JS更新，但保留以备后用
  document.addEventListener('DOMContentLoaded', () => {
      const wifiStatusSpan = document.getElementById('wifiStatus');
      const ipAddressSpan = document.getElementById('ipAddress');
      // 你可以通过 fetch 或其他方式定期更新这些状态，如果需要的话
      // 例如: fetch('/api/status').then(...)
      // 但对于静态值，模板替换就足够了
  });

</script>
</body>
</html>
)rawliteral";

// 颜色选择页面 HTML (原 HTML_PAGE_CONTENT)
const char COLOR_PAGE_HTML[] PROGMEM = R"rawliteral(
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
  button[data-color="1"] { background-color:rgb(251, 251, 251); } /* Red */
  button[data-color="2"] { background-color:rgb(0, 0, 0); } /* Green */
  button[data-color="3"] { background-color:rgb(255, 0, 0); } /* Blue */
  button[data-color="4"] { background-color:rgb(0, 8, 255); color: #333; } /* Yellow */
  button[data-color="5"] { background-color:rgb(246, 255, 0); } /* Black */
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

// 辅助函数：处理 HTML 模板替换
String processor(const String& var){
  Serial.println("Processing variable: " + var);
  if(var == "WIFI_STATUS"){
    return (WiFi.status() == WL_CONNECTED) ? "已连接" : "未连接";
  }
  if(var == "IP_ADDRESS"){
    return (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "N/A";
  }
  return String(); // 返回空字符串表示未找到变量
}

// WebSocket 事件处理函数
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:{
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      // 发送欢迎消息或初始状态 (可选)
      // client->text("Welcome!"); 
      }
      break;
    case WS_EVT_DISCONNECT:{
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      }
      break;
    case WS_EVT_DATA:{
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len) {
        // 仅处理完整的文本消息 (示例简化)
        if (info->opcode == WS_TEXT) {
          data[len] = 0; // 确保空终止
          String msg = (char*)data;
          Serial.printf("WS msg from client #%u: %s\n", client->id(), msg.c_str());
          // 将收到的文本数据发送给 Arduino
          ArduinoSerial.print(msg); // 使用 print 发送文本
          Serial.printf("Forwarded to Arduino (Serial2): %s\n", msg.c_str());
        }
        // 可以添加对 WS_BINARY 的处理，如果需要接收二进制
      }
      } 
      break;
    case WS_EVT_PONG:{
      Serial.printf("WS pong from client #%u\n", client->id());
      } 
      break;
    case WS_EVT_ERROR:{
      Serial.printf("WS error from client #%u code[%u] msg[%s]\n", client->id(), *((uint16_t*)arg), (char*)data);
      } 
      break;
  }
}

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

  // 根路径: 提供主页面 HTML，使用模板处理器替换状态
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("收到根路径请求 /");
    // 使用 processor 函数替换模板中的占位符
    request->send_P(200, "text/html", MAIN_PAGE_HTML, processor);
  });

  // 颜色子页面路径
  server.on("/color", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("收到颜色页面请求 /color");
    request->send_P(200, "text/html", COLOR_PAGE_HTML);
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
      if (*endptr == '\0' && codeStr.length() > 0 && colorCode_long >= 1 && colorCode_long <= 5) {
        int colorCode = (int)colorCode_long; // 安全转换
        ArduinoSerial.println(colorCode); // 发送给 Arduino (println 会添加换行符)
        Serial.printf("收到网页请求: 发送颜色代码 %d 到 Arduino (UART1)\n", colorCode);
        responseMessage = "颜色代码 " + codeStr + " 已发送到 Arduino。";
        httpCode = 200; // OK
      } else {
        Serial.printf("收到无效颜色代码请求: '%s'\n", codeStr.c_str());
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

  // --- 配置并启动 WebSocket 服务器 ---
  ws.onEvent(onWebSocketEvent); // 设置事件回调
  server.addHandler(&ws);       // 将 WebSocket 处理器添加到 Web 服务器
  Serial.println("WebSocket 服务器已配置在 /ws");

  // 启动服务器 (现在包括 WebSocket)
  server.begin();
  Serial.println("HTTP 和 WebSocket 服务器已启动");
  Serial.println("-----------------------------------------");
  // 发送启动消息给 Arduino (可选)
  // ArduinoSerial.println("ESP32_Web_Ready");

} // end setup

// --- Loop ---
void loop() {
  // 清理旧的 WS 客户端 (如果需要，ESPAsyncWebServer 会处理)
  // ws.cleanupClients(); 

  // 检查来自 Arduino 的数据并转发给 WebSocket 客户端
  if (ArduinoSerial.available() > 0) {
    // 读取所有可用字节
    size_t len = ArduinoSerial.available();
    uint8_t buf[128]; // 使用固定大小的缓冲区
    len = ArduinoSerial.readBytes(buf, min((size_t)128, len)); // 读取最多128字节
    
    if (len > 0) {
        // 转发给所有 WebSocket 客户端 (使用 binary 发送)
        ws.binaryAll(buf, len);
        
        // 同时在 ESP32 的 Serial 上打印调试信息 (可选)
        Serial.print("收到来自 Arduino (UART1) 并转发到 WS: ");
        // 打印为字符（如果可打印）或十六进制
        for(size_t i=0; i<len; i++) {
           if (isprint(buf[i])) {
               Serial.print((char)buf[i]);
           } else {
               Serial.printf("[%02X]", buf[i]);
           }
        }
        Serial.println();
    }
  }

  // ESPAsyncWebServer 和 WebSocket 都是异步的，不需要 delay()
}