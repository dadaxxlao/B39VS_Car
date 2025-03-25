#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "RemoteSerialDebug.h"

// 这是一个简单的测试程序，用于测试ESP32C3与Arduino之间的串口通信
// 将此文件重命名为main.cpp并编译上传即可测试

// 定义消息类型前缀
#define PREFIX_CMD    "$CMD:"    // 命令消息前缀
#define PREFIX_STATE  "$STATE:"  // 状态消息前缀
#define PREFIX_LOG    "$LOG:"    // 日志消息前缀
#define PREFIX_DATA   "$DATA:"   // 数据消息前缀
#define MSG_TERMINATOR '#'       // 消息结束标记

// WiFi配置
const char* WIFI_SSID = "S23";     // 修改为您的WiFi名称
const char* WIFI_PASSWORD = "12345678"; // 修改为您的WiFi密码
const char* HOSTNAME = "b39vs-car";         // 主机名
const char* AP_SSID = "B39VS_Car";          // AP模式SSID
const char* AP_PASSWORD = "12345678";       // AP模式密码

// 蓝牙服务UUID
#define SERVICE_UUID        "4FAFC201-1FB5-459E-8FCC-C5C9C331914B"
// 命令特征UUID (写入)
#define COMMAND_CHAR_UUID   "BEB5483E-36E1-4688-B7F5-EA07361B26A8"
// 状态特征UUID (读取/通知)
#define STATUS_CHAR_UUID    "5FB5483E-36E1-4688-B7F5-EA07361B26A9"

// Web服务器
AsyncWebServer webServer(80);
AsyncWebSocket ws("/ws");

// 测试变量
int counter = 0;
uint32_t lastSendTime = 0;
uint32_t lastStatusUpdate = 0;
uint32_t lastWiFiCheck = 0;
bool receivedCommand = false;
String lastCommand;
bool wifiConnected = false;
bool apMode = false;

// BLE相关变量
BLEServer* pServer = NULL;
BLECharacteristic* pCommandCharacteristic = NULL;
BLECharacteristic* pStatusCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// 状态和日志缓冲
String lastStatus = "Idle";
String lastSensorData = "No data";
DynamicJsonDocument logBuffer(4096);
int logIndex = 0;

// 板载LED引脚
const int LED_BLE = 12;   // 蓝牙连接指示灯 (GPIO12)
const int LED_WIFI = 13;  // WiFi连接指示灯 (GPIO13)

// 接收缓冲区
char buffer[256];
int bufferIndex = 0;

// 远程串口调试对象
RemoteSerialDebug* remoteDebugger = nullptr;
// 用于Arduino通信的串口
HardwareSerial ArduinoSerial(1); // UART1，用于与Arduino通信

// 函数前向声明
void parseMessage(const char* message);
void setupAPMode();
void addLogToBuffer(const char* level, const char* message);
void sendStatusViaBLE(const char* status);

// HTML页面
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>B39VS Car Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; margin: 0; padding: 20px; }
    .card { background-color: white; box-shadow: 0 2px 5px 0 rgba(0,0,0,0.16); border-radius: 5px; padding: 15px; margin-bottom: 20px; }
    .button { background-color: #4CAF50; border: none; color: white; padding: 10px 20px; text-align: center; 
        font-size: 16px; border-radius: 4px; cursor: pointer; margin: 5px; }
    .button.red { background-color: #f44336; }
    .button.blue { background-color: #2196F3; }
    .log { height: 200px; overflow-y: auto; background-color: #f5f5f5; padding: 10px; border-radius: 4px; font-family: monospace; }
    .status { font-weight: bold; margin-bottom: 10px; }
    #sensorData { margin-bottom: 10px; }
  </style>
</head>
<body>
  <div class="card">
    <h2>B39VS Car Control</h2>
    <div class="status">Status: <span id="status">Waiting...</span></div>
    <div id="sensorData">Sensor Data: Waiting...</div>
    <div>
      <button class="button" onclick="sendCommand('START')">Start</button>
      <button class="button red" onclick="sendCommand('STOP')">Stop</button>
      <button class="button blue" onclick="sendCommand('GET_STATUS')">Get Status</button>
    </div>
  </div>
  
  <div class="card">
    <h2>远程调试</h2>
    <div class="status">状态: <span id="debugStatus">未激活</span></div>
    <div>
      <button class="button blue" onclick="toggleDebug()">开启/关闭远程调试</button>
      <div id="debugInfo" style="display:none;">
        <p>调试服务器已开启，端口: <span id="debugPort">8880</span></p>
        <p>连接方式: <code>telnet ESP32的IP地址 8880</code> 或使用TCP客户端</p>
        <p>调试命令: <code>$transparent</code> 进入透明传输模式，<code>$normal</code> 返回普通模式</p>
      </div>
    </div>
  </div>
  
  <div class="card">
    <h2>System Log</h2>
    <div class="log" id="log"></div>
  </div>
  
  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;
    
    window.onload = function() {
      initWebSocket();
    }
    
    function initWebSocket() {
      console.log('Trying to connect to: ' + gateway);
      websocket = new WebSocket(gateway);
      websocket.onopen = onOpen;
      websocket.onclose = onClose;
      websocket.onmessage = onMessage;
    }
    
    function onOpen(event) {
      console.log('WebSocket connection opened');
      sendCommand('GET_STATUS');
    }
    
    function onClose(event) {
      console.log('WebSocket connection closed');
      setTimeout(initWebSocket, 2000);
    }
    
    function onMessage(event) {
      var data = JSON.parse(event.data);
      console.log('WebSocket message:', data);
      
      if (data.type === 'status') {
        document.getElementById('status').innerText = data.value;
      } 
      else if (data.type === 'sensor') {
        document.getElementById('sensorData').innerText = 'Sensor Data: ' + data.value;
      } 
      else if (data.type === 'log') {
        var logDiv = document.getElementById('log');
        logDiv.innerHTML += '<div>' + data.time + ' [' + data.level + '] ' + data.message + '</div>';
        logDiv.scrollTop = logDiv.scrollHeight;
      }
    }
    
    function sendCommand(command, params) {
      if (websocket.readyState === WebSocket.OPEN) {
        var msg = {
          command: command
        };
        if (params) {
          msg.params = params;
        }
        websocket.send(JSON.stringify(msg));
      }
    }
    
    // 添加调试控制函数
    var debugActive = false;
    var debugTransparent = false;
    
    function updateDebugStatus() {
      fetch('/api/debug', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: 'action=status'
      })
      .then(response => response.json())
      .then(data => {
        debugActive = data.active;
        if (data.active) {
          document.getElementById('debugStatus').innerText = data.connected ? '已连接' : '已激活';
          document.getElementById('debugPort').innerText = data.port;
          document.getElementById('debugInfo').style.display = 'block';
          debugTransparent = data.transparent;
        } else {
          document.getElementById('debugStatus').innerText = '未激活';
          document.getElementById('debugInfo').style.display = 'none';
        }
      });
    }
    
    function toggleDebug() {
      const action = debugTransparent ? 'disable' : 'enable';
      fetch('/api/debug', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: 'action=' + action
      })
      .then(response => response.json())
      .then(data => {
        if (data.success) {
          debugTransparent = data.transparent;
          document.getElementById('debugStatus').innerText = debugTransparent ? '透明模式已启用' : '正常模式';
        } else {
          alert('操作失败: ' + data.message);
        }
      });
    }
    
    // 在网页加载完成后检查调试状态
    window.addEventListener('load', function() {
      // 初始化WebSocket
      initWebSocket();
      // 检查调试状态
      setTimeout(updateDebugStatus, 1000);
      // 每10秒检查一次调试状态
      setInterval(updateDebugStatus, 10000);
    });
  </script>
</body>
</html>
)rawliteral";

// WebSocket事件回调
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        
        // 发送状态历史记录
        DynamicJsonDocument statusDoc(256);
        statusDoc["type"] = "status";
        statusDoc["value"] = lastStatus;
        String statusMessage;
        serializeJson(statusDoc, statusMessage);
        client->text(statusMessage);
        
        // 发送传感器数据
        DynamicJsonDocument sensorDoc(256);
        sensorDoc["type"] = "sensor";
        sensorDoc["value"] = lastSensorData;
        String sensorMessage;
        serializeJson(sensorDoc, sensorMessage);
        client->text(sensorMessage);
        
        // 发送日志历史
        for (int i = 0; i < logBuffer.size(); i++) {
            String logMessage;
            serializeJson(logBuffer[i], logMessage);
            client->text(logMessage);
        }
    } 
    else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    } 
    else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len) {
            // 确保数据以null结尾
            data[len] = 0;
            String message = String((char*)data);
            Serial.print("WebSocket received: ");
            Serial.println(message);
            
            // 解析JSON命令
            DynamicJsonDocument doc(256);
            DeserializationError error = deserializeJson(doc, message);
            
            if (!error) {
                const char* command = doc["command"];
                const char* params = doc["params"] | "";
                
                if (command) {
                    // 生成命令消息并交给解析器处理
                    String cmdMsg = String(PREFIX_CMD) + command;
                    if (strlen(params) > 0) {
                        cmdMsg += "," + String(params);
                    }
                    cmdMsg += MSG_TERMINATOR;
                    parseMessage(cmdMsg.c_str());
                }
            }
        }
    }
}

// 添加日志到缓冲区
void addLogToBuffer(const char* level, const char* message) {
    DynamicJsonDocument logEntry(256);
    logEntry["type"] = "log";
    logEntry["level"] = level;
    logEntry["message"] = message;
    
    // 获取当前时间（毫秒）
    unsigned long now = millis();
    // 格式化为 mm:ss.mmm
    char timeStr[12];
    sprintf(timeStr, "%02d:%02d.%03d", 
            (int)((now / 60000) % 60),     // 分钟 
            (int)((now / 1000) % 60),      // 秒
            (int)(now % 1000));            // 毫秒
    logEntry["time"] = timeStr;
    
    // 限制日志缓冲区大小
    if (logBuffer.size() >= 50) {
        // 移除最老的日志
        for (int i = 0; i < logBuffer.size() - 1; i++) {
            logBuffer[i] = logBuffer[i + 1];
        }
        logBuffer.remove(logBuffer.size() - 1);
    }
    
    // 添加新日志
    logBuffer.add(logEntry);
    
    // 通过WebSocket广播日志
    String logMessage;
    serializeJson(logEntry, logMessage);
    ws.textAll(logMessage);
}

// 设置Web服务器
void setupWebServer() {
    // 设置WebSocket处理器
    ws.onEvent(onWebSocketEvent);
    webServer.addHandler(&ws);
    
    // 设置Web服务器路由
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });
    
    // API路由，获取状态
    webServer.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["status"] = lastStatus;
        
        String response;
        serializeJson(jsonDoc, response);
        request->send(200, "application/json", response);
    });
    
    // API路由，发送命令
    webServer.on("/api/command", HTTP_POST, [](AsyncWebServerRequest *request){
        int params = request->params();
        String command;
        String parameters;
        
        for(int i=0; i<params; i++) {
            AsyncWebParameter* p = request->getParam(i);
            if (p->name() == "cmd") {
                command = p->value();
            } else if (p->name() == "params") {
                parameters = p->value();
            }
        }
        
        if (command.length() > 0) {
            // 生成命令消息并交给解析器处理
            String cmdMsg = String(PREFIX_CMD) + command;
            if (parameters.length() > 0) {
                cmdMsg += "," + parameters;
            }
            cmdMsg += MSG_TERMINATOR;
            parseMessage(cmdMsg.c_str());
            
            DynamicJsonDocument jsonDoc(256);
            jsonDoc["success"] = true;
            jsonDoc["message"] = "Command sent: " + command;
            
            String response;
            serializeJson(jsonDoc, response);
            request->send(200, "application/json", response);
        } else {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing command parameter\"}");
        }
    });
    
    // 添加远程调试控制API
    webServer.on("/api/debug", HTTP_POST, [](AsyncWebServerRequest *request){
        int params = request->params();
        String action;
        
        for(int i=0; i<params; i++) {
            AsyncWebParameter* p = request->getParam(i);
            if (p->name() == "action") {
                action = p->value();
            }
        }
        
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["success"] = true;
        
        if (action == "enable") {
            if (remoteDebugger) {
                remoteDebugger->setTransparentMode(true);
                jsonDoc["message"] = "透明模式已启用";
                jsonDoc["transparent"] = true;
            } else {
                jsonDoc["success"] = false;
                jsonDoc["message"] = "调试器未初始化";
            }
        } 
        else if (action == "disable") {
            if (remoteDebugger) {
                remoteDebugger->setTransparentMode(false);
                jsonDoc["message"] = "透明模式已禁用";
                jsonDoc["transparent"] = false;
            } else {
                jsonDoc["success"] = false;
                jsonDoc["message"] = "调试器未初始化";
            }
        }
        else if (action == "status") {
            if (remoteDebugger) {
                jsonDoc["active"] = true;
                jsonDoc["port"] = remoteDebugger->getDebugPort();
                jsonDoc["connected"] = remoteDebugger->hasClient();
                jsonDoc["transparent"] = remoteDebugger->isTransparentMode();
            } else {
                jsonDoc["active"] = false;
            }
        }
        else {
            jsonDoc["success"] = false;
            jsonDoc["message"] = "未知操作: " + action;
        }
        
        String response;
        serializeJson(jsonDoc, response);
        request->send(200, "application/json", response);
    });
    
    // 如果没有处理的路由显示404页面
    webServer.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not found");
    });
    
    // 启动Web服务器
    webServer.begin();
    Serial.println("HTTP server started");
}

// BLE服务器回调
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("BLE device connected!");
        digitalWrite(LED_BLE, HIGH);
        addLogToBuffer("INFO", "BLE device connected");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("BLE device disconnected!");
        digitalWrite(LED_BLE, LOW);
        addLogToBuffer("INFO", "BLE device disconnected");
    }
};

// BLE命令特征回调
class CommandCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.print("BLE command received: ");
            Serial.println(value.c_str());
            addLogToBuffer("INFO", ("BLE command: " + String(value.c_str())).c_str());
            
            // 生成命令消息并交给解析器处理
            String cmdMsg = PREFIX_CMD + String(value.c_str()) + MSG_TERMINATOR;
            parseMessage(cmdMsg.c_str());
        }
    }
};

// 设置BLE服务
void setupBLE() {
    Serial.println("Setting up BLE service...");
    
    // 初始化BLE设备
    BLEDevice::init("B39VS_Car");
    
    // 创建BLE服务器
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    
    // 创建BLE服务
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    // 创建命令特征（用于接收命令）
    pCommandCharacteristic = pService->createCharacteristic(
                               COMMAND_CHAR_UUID,
                               BLECharacteristic::PROPERTY_WRITE
                             );
    pCommandCharacteristic->setCallbacks(new CommandCallbacks());
    
    // 创建状态特征（用于发送状态）
    pStatusCharacteristic = pService->createCharacteristic(
                              STATUS_CHAR_UUID,
                              BLECharacteristic::PROPERTY_READ |
                              BLECharacteristic::PROPERTY_NOTIFY
                            );
    pStatusCharacteristic->addDescriptor(new BLE2902());
    
    // 启动服务
    pService->start();
    
    // 开始广播
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // iPhone连接参数
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE service started. Waiting for connections...");
    addLogToBuffer("INFO", "BLE service started");
}

// 通过BLE发送状态更新
void sendStatusViaBLE(const char* status) {
    if (deviceConnected) {
        pStatusCharacteristic->setValue(status);
        pStatusCharacteristic->notify();
        Serial.print("BLE status sent: ");
        Serial.println(status);
    }
}

// 检查WiFi连接状态
void checkWiFiStatus() {
    bool newStatus = WiFi.status() == WL_CONNECTED;
    if (newStatus != wifiConnected) {
        wifiConnected = newStatus;
        if (wifiConnected) {
            Serial.println("WiFi connected!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            Serial.print("Hostname: ");
            Serial.println(WiFi.getHostname());
            
            // 大字体显示IP地址，便于查看
            Serial.println("*******************************");
            Serial.println("*                             *");
            Serial.print("*     IP: ");
            Serial.print(WiFi.localIP());
            Serial.println("     *");
            Serial.println("*                             *");
            Serial.println("*******************************");
            
            // 更新LED状态
            digitalWrite(LED_WIFI, HIGH);
            addLogToBuffer("INFO", ("WiFi connected. IP: " + WiFi.localIP().toString()).c_str());
        } else {
            Serial.println("WiFi connection lost! Starting AP mode...");
            digitalWrite(LED_WIFI, LOW);
            addLogToBuffer("WARN", "WiFi connection lost");
            
            // 启动AP模式
            setupAPMode();
        }
    }
}

// 设置AP模式
void setupAPMode() {
    Serial.println("Setting up AP mode...");
    
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    apMode = true;
    
    // 大字体显示AP IP地址，便于查看
    Serial.println("*******************************");
    Serial.println("*                             *");
    Serial.print("*   AP IP: ");
    Serial.print(IP);
    Serial.println("   *");
    Serial.println("*   SSID: " + String(AP_SSID) + "        *");
    Serial.println("*   Pass: " + String(AP_PASSWORD) + "         *");
    Serial.println("*                             *");
    Serial.println("*******************************");
    
    addLogToBuffer("INFO", ("AP Mode enabled. SSID: " + String(AP_SSID) + ", IP: " + IP.toString()).c_str());
}

// 解析接收到的消息
void parseMessage(const char* message) {
    Serial.print("Message received: ");
    Serial.println(message);
    
    if (strncmp(message, PREFIX_CMD, strlen(PREFIX_CMD)) == 0) {
        const char* cmdData = message + strlen(PREFIX_CMD);
        lastCommand = String(cmdData);
        receivedCommand = true;
        
        // 响应状态
        Serial.print(PREFIX_STATE);
        Serial.print("1,Received command: ");
        Serial.print(cmdData);
        Serial.println(MSG_TERMINATOR);
        
        // 更新Web界面状态
        lastStatus = "Processing: " + lastCommand;
        DynamicJsonDocument statusDoc(256);
        statusDoc["type"] = "status";
        statusDoc["value"] = lastStatus;
        String statusMessage;
        serializeJson(statusDoc, statusMessage);
        ws.textAll(statusMessage);
        
        // 通过BLE发送状态
        String bleStatus = "Received: " + lastCommand;
        sendStatusViaBLE(bleStatus.c_str());
        
        // 响应日志
        Serial.print(PREFIX_LOG);
        Serial.print("INFO,Processing command: ");
        Serial.print(cmdData);
        Serial.println(MSG_TERMINATOR);
        
        addLogToBuffer("INFO", ("Processing command: " + String(cmdData)).c_str());
    }
    else if (strncmp(message, PREFIX_DATA, strlen(PREFIX_DATA)) == 0) {
        const char* dataStr = message + strlen(PREFIX_DATA);
        lastSensorData = String(dataStr);
        
        // 更新Web界面传感器数据
        DynamicJsonDocument sensorDoc(256);
        sensorDoc["type"] = "sensor";
        sensorDoc["value"] = lastSensorData;
        String sensorMessage;
        serializeJson(sensorDoc, sensorMessage);
        ws.textAll(sensorMessage);
    }
    else if (strncmp(message, PREFIX_STATE, strlen(PREFIX_STATE)) == 0) {
        const char* stateStr = message + strlen(PREFIX_STATE);
        lastStatus = String(stateStr);
        
        // 更新Web界面状态
        DynamicJsonDocument statusDoc(256);
        statusDoc["type"] = "status";
        statusDoc["value"] = lastStatus;
        String statusMessage;
        serializeJson(statusDoc, statusMessage);
        ws.textAll(statusMessage);
    }
    else if (strncmp(message, PREFIX_LOG, strlen(PREFIX_LOG)) == 0) {
        const char* logData = message + strlen(PREFIX_LOG);
        // 查找第一个逗号分隔符
        const char* comma = strchr(logData, ',');
        if (comma) {
            String level = String(logData).substring(0, comma - logData);
            String logMessage = String(comma + 1);
            addLogToBuffer(level.c_str(), logMessage.c_str());
        }
    }
}

void setupWiFi() {
    Serial.println("Setting up WiFi...");
    
    // 设置主机名
    WiFi.setHostname(HOSTNAME);
    
    // 开始连接WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // 等待连接，最多等待10秒
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Hostname: ");
        Serial.println(WiFi.getHostname());
        wifiConnected = true;
        
        // 大字体显示IP地址，便于查看
        Serial.println("*******************************");
        Serial.println("*                             *");
        Serial.print("*     IP: ");
        Serial.print(WiFi.localIP());
        Serial.println("     *");
        Serial.println("*                             *");
        Serial.println("*******************************");
        
        // 更新LED状态
        digitalWrite(LED_WIFI, HIGH);
        addLogToBuffer("INFO", ("WiFi connected. IP: " + WiFi.localIP().toString()).c_str());
    } else {
        Serial.println("WiFi connection failed! Starting AP mode...");
        wifiConnected = false;
        digitalWrite(LED_WIFI, LOW);
        
        // 启动AP模式
        setupAPMode();
    }
}

void setup() {
    // 初始化串口通信
    Serial.begin(115200);
    delay(500);
    
    // 初始化LED引脚
    pinMode(LED_BLE, OUTPUT);
    pinMode(LED_WIFI, OUTPUT);
    digitalWrite(LED_BLE, LOW);
    digitalWrite(LED_WIFI, LOW);
    
    Serial.println("\n\nESP32-C3 Full Test Starting...");
    Serial.println("This program tests WiFi, Web Server, BLE and simulates Arduino data");
    
    // 添加初始日志
    addLogToBuffer("INFO", "System starting");
    
    // 设置WiFi
    setupWiFi();
    
    // 设置BLE
    setupBLE();
    
    // 设置Web服务器
    setupWebServer();
    
    // 初始化Arduino串口 (UART1)
    ArduinoSerial.begin(115200, SERIAL_8N1, 18, 19); // RX=18, TX=19
    
    // 初始化远程调试
    remoteDebugger = new RemoteSerialDebug(&ArduinoSerial);
    if (remoteDebugger) {
        remoteDebugger->begin();
        Serial.println("RemoteSerialDebug initialized on port 8880");
        addLogToBuffer("INFO", "远程调试功能已初始化，端口：8880");
    }
    
    Serial.println("All services started. Waiting for commands...");
    addLogToBuffer("INFO", "All services started");
}

void loop() {
    // 处理接收到的数据
    while (Serial.available()) {
        char c = Serial.read();
        
        if (c == MSG_TERMINATOR) {
            if (bufferIndex > 0) {
                buffer[bufferIndex] = '\0';
                parseMessage(buffer);
                bufferIndex = 0;
            }
        } 
        else if (bufferIndex < sizeof(buffer) - 1) {
            buffer[bufferIndex++] = c;
        }
    }
    
    // 获取当前时间
    uint32_t currentTime = millis();
    
    // 检查WiFi状态
    if (currentTime - lastWiFiCheck > 5000) {  // 每5秒检查一次
        lastWiFiCheck = currentTime;
        if (!apMode) {
            checkWiFiStatus();
        }
    }
    
    // 每秒发送传感器数据
    if (currentTime - lastSendTime > 1000) {
        lastSendTime = currentTime;
        
        // 生成随机传感器数据
        int distance = random(10, 100);
        int color = random(1, 5);
        
        // 发送传感器数据
        String dataMsg = PREFIX_DATA;
        dataMsg += "SENSORS,distance:";
        dataMsg += distance;
        dataMsg += ",color:";
        dataMsg += color;
        dataMsg += MSG_TERMINATOR;
        
        // 使用parseMessage处理，这样会同时更新web界面
        parseMessage(dataMsg.c_str());
    }
    
    // 每5秒发送一次状态更新
    if (currentTime - lastStatusUpdate > 5000) {
        lastStatusUpdate = currentTime;
        counter++;
        
        // 定期显示IP地址，方便用户查看
        if (counter % 6 == 0) { // 每30秒显示一次(每5秒更新一次，所以6次是30秒)
            if (wifiConnected) {
                Serial.println("*******************************");
                Serial.print("*     IP: ");
                Serial.print(WiFi.localIP());
                Serial.println("     *");
                Serial.println("*******************************");
            } else if (apMode) {
                Serial.println("*******************************");
                Serial.print("*   AP IP: ");
                Serial.print(WiFi.softAPIP());
                Serial.println("   *");
                Serial.println("*******************************");
            }
        }
        
        // 发送状态更新
        String stateMsg = PREFIX_STATE;
        stateMsg += "0,Idle,counter:";
        stateMsg += counter;
        stateMsg += MSG_TERMINATOR;
        
        // 使用parseMessage处理，这样会同时更新web界面
        parseMessage(stateMsg.c_str());
        
        // 发送日志
        String logMsg = PREFIX_LOG;
        logMsg += "DEBUG,Heartbeat count: ";
        logMsg += counter;
        logMsg += ", WiFi: ";
        logMsg += wifiConnected ? "Connected" : (apMode ? "AP Mode" : "Disconnected");
        logMsg += ", BLE: ";
        logMsg += deviceConnected ? "Connected" : "Disconnected";
        logMsg += MSG_TERMINATOR;
        
        // 使用parseMessage处理，这样会同时更新web界面
        parseMessage(logMsg.c_str());
        
        // 通过BLE发送状态
        if (deviceConnected) {
            String bleStatus = "Status: " + String(counter);
            sendStatusViaBLE(bleStatus.c_str());
        }
    }
    
    // 如果收到命令，模拟一些操作
    if (receivedCommand) {
        receivedCommand = false;
        
        // 创建随机延迟模拟处理时间
        int processTime = random(500, 2000);
        delay(processTime);
        
        // 发送处理完成的状态
        String stateMsg = PREFIX_STATE;
        stateMsg += "1,Processed command: ";
        stateMsg += lastCommand;
        stateMsg += " in ";
        stateMsg += processTime;
        stateMsg += "ms";
        stateMsg += MSG_TERMINATOR;
        
        // 使用parseMessage处理，这样会同时更新web界面
        parseMessage(stateMsg.c_str());
        
        // 发送日志
        String logMsg = PREFIX_LOG;
        logMsg += "INFO,Command ";
        logMsg += lastCommand;
        logMsg += " executed successfully";
        logMsg += MSG_TERMINATOR;
        
        // 使用parseMessage处理，这样会同时更新web界面
        parseMessage(logMsg.c_str());
        
        // 通过BLE发送完成状态
        if (deviceConnected) {
            String bleStatus = "Completed: " + lastCommand;
            sendStatusViaBLE(bleStatus.c_str());
        }
    }
    
    // 断开连接后再重新连接
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // 等待蓝牙堆栈准备好
        pServer->startAdvertising(); // 重新开始广播
        Serial.println("BLE started advertising");
        oldDeviceConnected = deviceConnected;
    }
    
    // 连接状态更新
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
    
    // WebSocket清理
    ws.cleanupClients();
    
    // 处理远程调试数据
    if (remoteDebugger) {
        remoteDebugger->update();
    }
    
    delay(10);
} 