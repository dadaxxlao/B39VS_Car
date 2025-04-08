#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HardwareSerial.h>
#include <algorithm> // 添加 algorithm 头文件

// --- 配置 ---
const char* ssid = "S23";
const char* password = "12345678";

// 连接 Arduino 的 UART 配置
#define ARDUINO_SERIAL_PORT 1 // 使用 ESP32 的 UART1
#define ARDUINO_RX_PIN 1      // ESP32 RX GPIO1 连接 Arduino TX2 PIN16 
#define ARDUINO_TX_PIN 0      // ESP32 TX GPIO0 连接 Arduino RX2 PIN17 
#define ARDUINO_BAUD_RATE 115200 // 必须与 Arduino Serial.begin() 的波特率一致

// Web 服务器 & WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Arduino 串口实例
HardwareSerial ArduinoSerial(ARDUINO_SERIAL_PORT);

// UART -> WebSocket 转发缓冲区
char uartBuffer[256];
size_t uartBufferPos = 0;
unsigned long lastUartSendTime = 0;
const unsigned long UART_SEND_TIMEOUT_MS = 20; // 缓冲发送超时

// --- 前端 HTML/CSS/JS ---
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 Remote Serial</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; }
    #output { width: 95%; height: 60vh; border: 1px solid #ccc; overflow-y: scroll; white-space: pre-wrap; font-family: monospace; margin-bottom: 10px; background-color: #f4f4f4; }
    #input { width: 80%; padding: 8px; margin-right: 5px; border: 1px solid #ccc; }
    #sendBtn { padding: 8px 12px; cursor: pointer; background-color: #4CAF50; color: white; border: none; }
    #sendBtn:hover { background-color: #45a049; }
    #status { margin-top: 10px; font-size: 0.8em; color: grey; }
  </style>
</head>
<body>
  <h1>ESP32 Remote Serial</h1>
  <div id="status">Status: Connecting...</div>
  <textarea id="output" readonly></textarea><br>
  <input type="text" id="input" placeholder="Send to Arduino...">
  <button id="sendBtn" onclick="sendData()">Send</button>

  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;
    var outputArea = document.getElementById('output');
    var inputField = document.getElementById('input');
    var statusDiv = document.getElementById('status');

    function initWebSocket() {
      console.log('Trying to open a WebSocket connection...');
      statusDiv.textContent = 'Status: Connecting...';
      websocket = new WebSocket(gateway);
      websocket.onopen    = onOpen;
      websocket.onclose   = onClose;
      websocket.onmessage = onMessage;
      websocket.onerror   = onError;
    }

    function onOpen(event) {
      console.log('Connection opened');
      statusDiv.textContent = 'Status: Connected';
    }

    function onClose(event) {
      console.log('Connection closed');
      statusDiv.textContent = 'Status: Disconnected. Retrying in 2s...';
      setTimeout(initWebSocket, 2000); // Try to reconnect after 2 seconds
    }

    function onMessage(event) {
      //console.log('Received: ', event.data);
      outputArea.value += event.data; // Append received data
      outputArea.scrollTop = outputArea.scrollHeight; // Auto-scroll
    }

    function onError(event) {
        console.error('WebSocket Error: ', event);
        statusDiv.textContent = 'Status: Connection Error';
    }

    function sendData() {
      var data = inputField.value;
      if (data.length > 0 && websocket && websocket.readyState === WebSocket.OPEN) {
        console.log('Sending: ', data);
        websocket.send(data); // Directly send the string
        inputField.value = ''; // Clear input field
      } else if (!websocket || websocket.readyState !== WebSocket.OPEN) {
          console.error('WebSocket is not connected.');
          statusDiv.textContent = 'Status: Not Connected. Cannot send data.';
      }
    }

    // Send on Enter key press
    inputField.addEventListener('keypress', function(event) {
        if (event.key === 'Enter') {
            event.preventDefault(); // Prevent default form submission
            sendData();
        }
    });

    window.addEventListener('load', initWebSocket);
  </script>
</body>
</html>
)rawliteral";


// --- WebSocket 事件处理 ---
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AsyncWebSocketClient *client = (AsyncWebSocketClient*)arg;
  if (client->status() == WS_CONNECTED) {
    // 将从 Web 客户端收到的数据直接写入 Arduino 串口
    ArduinoSerial.write(data, len);
    // 为了减少自身日志串口的干扰，这里可以注释掉或用 DEBUG 宏控制
    // Serial.printf("[WS-%u] Sent %u bytes to Arduino\n", client->id(), len);
  }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("[WS-%u] Client connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      // 可以选择在连接时发送一些状态信息
      client->text("Connected to ESP32 Serial Bridge\n");
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("[WS-%u] Client disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      // 处理收到的数据 (文本或二进制)
      // Serial.printf("[WS-%u] Received %u bytes\n", client->id(), len);
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
       // Serial.printf("[WS-%u] Pong received\n", client->id());
       break;
    case WS_EVT_ERROR:
      Serial.printf("[WS-%u] WebSocket Error(%u): %s\n", client->id(), *((uint16_t*)arg), (char*)data);
      break;
  }
}

// --- UART 数据转发函数 ---
void sendBufferedUartData() {
    if (uartBufferPos > 0) {
        // 使用 binaryAll 发送以确保原始字节流，Web 端需要能处理 ArrayBuffer
        // 或者继续使用 textAll，假设 Arduino 主要发送文本
        ws.textAll(uartBuffer, uartBufferPos); // 将缓冲区内容发送给所有 WS 客户端
        // Serial.printf("[UART->WS] Sent %d bytes\n", uartBufferPos); // 减少日志干扰
        uartBufferPos = 0; // 清空缓冲区
    }
    lastUartSendTime = millis(); // 重置计时器
}


// --- Arduino Setup ---
void setup() {
  Serial.begin(115200); // ESP32 自身的日志串口
  // 等待串口连接建立 (可选，但有时有用)
  while (!Serial) {
    delay(10); 
  }
  Serial.println("\n--- ESP32 Setup Started ---"); 
  Serial.println("ESP32 Remote Serial Bridge");

  // 初始化连接 Arduino 的串口
  Serial.println("Initializing Arduino Serial...");
  Serial.printf("Using UART%d: RX=%d, TX=%d, Baud=%d\n",
                ARDUINO_SERIAL_PORT, ARDUINO_RX_PIN, ARDUINO_TX_PIN, ARDUINO_BAUD_RATE);
  ArduinoSerial.begin(ARDUINO_BAUD_RATE, SERIAL_8N1, ARDUINO_RX_PIN, ARDUINO_TX_PIN);
  Serial.println("Arduino Serial Initialized.");

  // 连接 WiFi
  Serial.printf("Attempting to connect to WiFi SSID: %s ...", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) { // 15秒超时
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nWiFi Connection Failed! Restarting...");
      delay(1000);
      ESP.restart();
  } else {
      Serial.println("\nWiFi Connected!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
  }

  // 设置 WebSocket 事件回调
  Serial.println("Setting up WebSocket...");
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  Serial.println("WebSocket Handler Added.");

  // 设置 HTTP 服务器路由，提供网页
  Serial.println("Setting up HTTP Server Routes...");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", HTML_PAGE);
  });

  // 处理未找到的路由
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Not found");
  });
  Serial.println("HTTP Routes Configured.");


  // 启动服务器
  Serial.println("Starting Web Server...");
  server.begin();
  Serial.println("--- ESP32 Setup Complete. Web Server and WebSocket started. Ready. ---");
}

// --- Arduino Loop ---
unsigned long lastLoopMsgTime = 0;
void loop() {
  // 添加一个标记表明 loop 正在运行，但不要过于频繁输出
  if (millis() - lastLoopMsgTime > 5000) { // 每 5 秒输出一次
    Serial.println("[Loop] Running...");
    lastLoopMsgTime = millis();
  }

  // 1. 检查 Arduino 串口是否有数据传入
  bool dataRead = false;
  // 优化读取：一次读取所有可用字节，减少循环次数
  size_t availableBytes = ArduinoSerial.available();
  if (availableBytes > 0) {
      // 使用 std::min 并确保类型一致
      size_t spaceInBuffer = sizeof(uartBuffer) - uartBufferPos - 1; // 计算缓冲区剩余空间
      size_t readLen = std::min(availableBytes, spaceInBuffer); // 避免缓冲区溢出
      if (readLen > 0) {
          size_t actuallyRead = ArduinoSerial.readBytes(&uartBuffer[uartBufferPos], readLen);
          uartBufferPos += actuallyRead;
          uartBuffer[uartBufferPos] = '\0'; // 确保缓冲区以 null 结尾（如果用 textAll） - 注意这里改为 \0
          dataRead = true;
      }
      // 如果缓冲区满了，立即尝试发送
      if (uartBufferPos >= sizeof(uartBuffer) - 1) {
          sendBufferedUartData();
      }
  }


  // 2. 如果读取到了数据，重置发送计时器
  if (dataRead) {
      lastUartSendTime = millis();
  }

  // 3. 检查是否超时未发送（处理没有换行符的数据段）
  if (uartBufferPos > 0 && (millis() - lastUartSendTime > UART_SEND_TIMEOUT_MS)) {
      sendBufferedUartData();
  }

  // 清理断开的 WebSocket 客户端 (根据库文档，可能不需要手动调用，但保留无害)
  ws.cleanupClients();

  // 短暂延时避免过于繁忙 (可选)
  // delay(1);
}