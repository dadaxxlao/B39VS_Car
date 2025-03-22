#include "WebServer.h"

// HTML页面
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>B39VS Car Controller</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f0f0f0; }
        h1 { color: #333; }
        .container { max-width: 800px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .panel { margin-bottom: 20px; padding: 15px; border-radius: 8px; background-color: #f9f9f9; }
        .panel h2 { margin-top: 0; }
        .log-container { height: 150px; overflow-y: auto; background-color: #333; color: #f0f0f0; padding: 10px; border-radius: 4px; font-family: monospace; }
        .log-item { margin: 2px 0; }
        .log-error { color: #ff6b6b; }
        .log-warning { color: #feca57; }
        .log-info { color: #54a0ff; }
        .log-debug { color: #1dd1a1; }
        .btn-container { display: flex; justify-content: center; gap: 10px; flex-wrap: wrap; margin: 20px 0; }
        button { padding: 12px 24px; border: none; border-radius: 4px; background-color: #4CAF50; color: white; cursor: pointer; font-size: 16px; }
        button:hover { opacity: 0.9; }
        button:disabled { background-color: #cccccc; cursor: not-allowed; }
        .btn-stop { background-color: #e74c3c; }
        .btn-start { background-color: #2ecc71; }
        .btn-reset { background-color: #3498db; }
        .status-badge { display: inline-block; padding: 6px 12px; border-radius: 16px; color: white; background-color: #95a5a6; font-weight: bold; }
        .status-idle { background-color: #95a5a6; }
        .status-running { background-color: #2ecc71; }
        .status-error { background-color: #e74c3c; }
        .status-finding { background-color: #3498db; }
        .status-grabbing { background-color: #9b59b6; }
        .status-placing { background-color: #f1c40f; }
        .status-returning { background-color: #1abc9c; }
    </style>
</head>
<body>
    <div class="container">
        <h1>B39VS Car Controller</h1>
        
        <div class="panel">
            <h2>Status: <span id="status-badge" class="status-badge status-idle">IDLE</span></h2>
            <p id="status-details">No additional status information available.</p>
        </div>
        
        <div class="btn-container">
            <button id="btn-start" class="btn-start">Start Task</button>
            <button id="btn-stop" class="btn-stop">Stop Task</button>
            <button id="btn-reset" class="btn-reset">Reset</button>
            <button id="btn-status" class="btn-status">Get Status</button>
        </div>
        
        <div class="panel">
            <h2>Live Log</h2>
            <div id="log-container" class="log-container"></div>
        </div>
        
        <div class="panel">
            <h2>Sensor Data</h2>
            <pre id="sensor-data">No sensor data available.</pre>
        </div>
    </div>

    <script>
        // WebSocket connection
        let socket;
        const connectWebSocket = () => {
            const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            socket = new WebSocket(`${protocol}//${window.location.host}/ws`);
            
            socket.onopen = () => {
                console.log('WebSocket connected');
                addLogMessage('INFO', 'WebSocket connected');
            };
            
            socket.onclose = () => {
                console.log('WebSocket disconnected');
                addLogMessage('ERROR', 'WebSocket disconnected');
                // Try to reconnect after 2 seconds
                setTimeout(connectWebSocket, 2000);
            };
            
            socket.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    handleMessage(data);
                } catch (e) {
                    console.error('Error parsing WebSocket message:', e);
                }
            };
            
            socket.onerror = (error) => {
                console.error('WebSocket error:', error);
                addLogMessage('ERROR', 'WebSocket error: ' + error);
            };
        };
        
        // Handle incoming messages
        const handleMessage = (data) => {
            if (data.type === 'status') {
                updateStatus(data.state, data.details);
            } else if (data.type === 'log') {
                addLogMessage(data.level, data.message);
            } else if (data.type === 'data') {
                updateSensorData(data.dataType, data.values);
            }
        };
        
        // Update status display
        const updateStatus = (state, details) => {
            const statusBadge = document.getElementById('status-badge');
            statusBadge.textContent = state.toUpperCase();
            statusBadge.className = 'status-badge';
            
            switch (state.toLowerCase()) {
                case 'idle': statusBadge.classList.add('status-idle'); break;
                case 'running': statusBadge.classList.add('status-running'); break;
                case 'error': statusBadge.classList.add('status-error'); break;
                case 'finding': statusBadge.classList.add('status-finding'); break;
                case 'grabbing': statusBadge.classList.add('status-grabbing'); break;
                case 'placing': statusBadge.classList.add('status-placing'); break;
                case 'returning': statusBadge.classList.add('status-returning'); break;
                default: statusBadge.classList.add('status-idle');
            }
            
            document.getElementById('status-details').textContent = details || 'No additional status information available.';
        };
        
        // Add a log message
        const addLogMessage = (level, message) => {
            const logContainer = document.getElementById('log-container');
            const logItem = document.createElement('div');
            logItem.className = `log-item log-${level.toLowerCase()}`;
            logItem.textContent = `[${level}] ${message}`;
            logContainer.appendChild(logItem);
            logContainer.scrollTop = logContainer.scrollHeight;
            
            // Keep only the most recent 100 messages
            while (logContainer.childElementCount > 100) {
                logContainer.removeChild(logContainer.firstChild);
            }
        };
        
        // Update sensor data
        const updateSensorData = (dataType, values) => {
            const sensorDataElem = document.getElementById('sensor-data');
            if (dataType && values) {
                sensorDataElem.textContent = `${dataType}: ${values}`;
            }
        };
        
        // Send command to server
        const sendCommand = (command, params = null) => {
            if (socket && socket.readyState === WebSocket.OPEN) {
                const message = {
                    command: command,
                    params: params
                };
                socket.send(JSON.stringify(message));
                addLogMessage('INFO', `Command sent: ${command}`);
            } else {
                addLogMessage('ERROR', 'WebSocket not connected. Cannot send command.');
            }
        };
        
        // Button click handlers
        document.getElementById('btn-start').addEventListener('click', () => {
            sendCommand('START');
        });
        
        document.getElementById('btn-stop').addEventListener('click', () => {
            sendCommand('STOP');
        });
        
        document.getElementById('btn-reset').addEventListener('click', () => {
            sendCommand('RESET');
        });
        
        document.getElementById('btn-status').addEventListener('click', () => {
            sendCommand('GET_STATUS');
        });
        
        // Connect WebSocket when page loads
        window.addEventListener('load', () => {
            connectWebSocket();
            addLogMessage('INFO', 'Page loaded');
        });
    </script>
</body>
</html>
)rawliteral";

// 全局实例
WebServerManager WebServer;

WebServerManager::WebServerManager(const String& ssid, const String& password, const String& hostname)
    : _ssid(ssid), _password(password), _hostname(hostname) {
}

void WebServerManager::setWiFiCredentials(const String& ssid, const String& password) {
    _ssid = ssid;
    _password = password;
}

void WebServerManager::setHostname(const String& hostname) {
    _hostname = hostname;
}

bool WebServerManager::begin(bool startAP) {
    // 初始化WiFi
    bool wifiConnected = _setupWiFi();
    
    if (!wifiConnected && startAP) {
        _setupAPMode();
    }
    
    // 初始化Web服务器
    _server = new AsyncWebServer(80);
    _ws = new AsyncWebSocket("/ws");
    
    if (!_server || !_ws) {
        Serial.println("Failed to create web server or WebSocket");
        return false;
    }
    
    // 设置WebSocket事件处理
    _ws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
        } else if (type == WS_EVT_DATA) {
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            if (info->final && info->index == 0 && info->len == len) {
                // 完整消息
                data[len] = 0;
                _handleWebSocketMessage(client, (const char*)data);
            }
        }
    });
    
    _server->addHandler(_ws);
    
    // 设置Web页面和API路由
    _setupWebServer();
    
    // 启动服务器
    _server->begin();
    Serial.println("Web server started");
    
    return true;
}

bool WebServerManager::_setupWiFi() {
    if (_ssid.isEmpty()) {
        Serial.println("WiFi SSID not set");
        return false;
    }
    
    Serial.printf("Connecting to WiFi %s...\n", _ssid.c_str());
    
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(_hostname.c_str());
    WiFi.begin(_ssid.c_str(), _password.c_str());
    
    // 等待连接，但有超时
    uint32_t startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        
        if (millis() - startTime > WIFI_CONNECT_TIMEOUT) {
            Serial.println("\nWiFi connection timeout");
            return false;
        }
    }
    
    _wifiConnected = true;
    _isAPMode = false;
    
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    return true;
}

void WebServerManager::_setupAPMode() {
    Serial.println("Setting up AP mode");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(_hostname.c_str(), "12345678");
    
    _wifiConnected = true;
    _isAPMode = true;
    
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

void WebServerManager::_setupWebServer() {
    // 主页
    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });
    
    // API: 获取状态
    _server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(256);
        doc["state"] = stateCodeToString(_currentState);
        
        // 分析额外参数
        if (!_sensorData.isEmpty()) {
            JsonObject data = doc.createNestedObject("data");
            // 分割数据字符串
            int colonPos = _sensorData.indexOf(":");
            if (colonPos > 0) {
                String dataType = _sensorData.substring(0, colonPos);
                String dataValues = _sensorData.substring(colonPos + 1);
                data["type"] = dataType;
                data["values"] = dataValues;
            }
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // API: 发送命令
    _server->on("/api/command", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Command received via POST");
    }, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, data, len);
        
        if (error) {
            Serial.print("JSON parsing error: ");
            Serial.println(error.c_str());
            request->send(400, "text/plain", "Invalid JSON");
            return;
        }
        
        const char* command = doc["command"];
        const char* params = doc["params"] | "";
        
        if (!command) {
            request->send(400, "text/plain", "Missing command");
            return;
        }
        
        if (_commandCallback) {
            _commandCallback(command, params);
        }
        
        request->send(200, "application/json", "{\"status\":\"success\"}");
    });
}

void WebServerManager::_handleWebSocketMessage(AsyncWebSocketClient* client, const char* data) {
    Serial.print("WebSocket data received: ");
    Serial.println(data);
    
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, data);
    
    if (error) {
        Serial.print("JSON parsing error: ");
        Serial.println(error.c_str());
        return;
    }
    
    const char* command = doc["command"];
    const char* params = doc["params"] | "";
    
    if (command && _commandCallback) {
        _commandCallback(command, params);
    }
}

void WebServerManager::_sendStatusUpdate() {
    DynamicJsonDocument doc(256);
    doc["type"] = "status";
    doc["state"] = stateCodeToString(_currentState);
    
    // 分析额外参数
    if (!_sensorData.isEmpty()) {
        int colonPos = _sensorData.indexOf(":");
        if (colonPos > 0) {
            String dataType = _sensorData.substring(0, colonPos);
            String dataValues = _sensorData.substring(colonPos + 1);
            doc["details"] = dataValues.c_str();
        }
    }
    
    String status;
    serializeJson(doc, status);
    _ws->textAll(status);
}

void WebServerManager::updateState(SystemStateCode state, const char* extraParams) {
    _currentState = state;
    
    // 发送WebSocket通知
    DynamicJsonDocument doc(256);
    doc["type"] = "status";
    doc["state"] = stateCodeToString(state);
    doc["details"] = extraParams ? extraParams : "";
    
    String status;
    serializeJson(doc, status);
    _ws->textAll(status);
}

void WebServerManager::updateLog(const char* level, const char* message) {
    _lastLog = String(level) + ": " + String(message);
    
    // 发送WebSocket通知
    DynamicJsonDocument doc(256);
    doc["type"] = "log";
    doc["level"] = level;
    doc["message"] = message;
    
    String log;
    serializeJson(doc, log);
    _ws->textAll(log);
}

void WebServerManager::updateData(const char* dataType, const char* values) {
    _sensorData = String(dataType) + ":" + String(values);
    
    // 发送WebSocket通知
    DynamicJsonDocument doc(256);
    doc["type"] = "data";
    doc["dataType"] = dataType;
    doc["values"] = values;
    
    String data;
    serializeJson(doc, data);
    _ws->textAll(data);
}

String WebServerManager::getLocalIP() const {
    if (_isAPMode) {
        return WiFi.softAPIP().toString();
    } else if (_wifiConnected) {
        return WiFi.localIP().toString();
    }
    return "Not connected";
}

void WebServerManager::setCommandCallback(CommandCallback callback) {
    _commandCallback = callback;
}

void WebServerManager::setConnectionCallback(ConnectionCallback callback) {
    _connectionCallback = callback;
}

void WebServerManager::_checkWiFiStatus() {
    if (_isAPMode) {
        return; // AP模式不需要检查连接状态
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        if (_wifiConnected) {
            _wifiConnected = false;
            Serial.println("WiFi disconnected");
            
            if (_connectionCallback) {
                _connectionCallback(false);
            }
        }
    } else if (!_wifiConnected) {
        _wifiConnected = true;
        Serial.println("WiFi reconnected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
        if (_connectionCallback) {
            _connectionCallback(true);
        }
    }
}

void WebServerManager::update() {
    // 定期检查WiFi状态
    uint32_t currentTime = millis();
    if (currentTime - _lastCheckTime > 5000) { // 每5秒检查一次
        _lastCheckTime = currentTime;
        _checkWiFiStatus();
    }
    
    // 清理WebSocket连接
    _ws->cleanupClients();
} 