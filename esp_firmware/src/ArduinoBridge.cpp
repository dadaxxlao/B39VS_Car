#include "ArduinoBridge.h"

ArduinoBridge::ArduinoBridge(Stream* serial) : _serial(serial) {
    memset(_buffer, 0, sizeof(_buffer));
}

bool ArduinoBridge::begin() {
    if (!_serial) {
        return false;
    }
    
    _initialized = true;
    return true;
}

void ArduinoBridge::setStateCallback(StateCallback callback) {
    _stateCallback = callback;
}

void ArduinoBridge::setLogCallback(LogCallback callback) {
    _logCallback = callback;
}

void ArduinoBridge::setDataCallback(DataCallback callback) {
    _dataCallback = callback;
}

bool ArduinoBridge::sendCommand(CommandType cmdType, const char* params) {
    if (!_initialized || !_serial) {
        return false;
    }
    
    _serial->print(PREFIX_CMD);
    _serial->print(commandTypeToString(cmdType));
    
    if (params != nullptr) {
        _serial->print(",");
        _serial->print(params);
    }
    
    _serial->print(MSG_TERMINATOR);
    return true;
}

bool ArduinoBridge::sendCommand(const char* cmdName, const char* params) {
    if (!_initialized || !_serial) {
        return false;
    }
    
    _serial->print(PREFIX_CMD);
    _serial->print(cmdName);
    
    if (params != nullptr) {
        _serial->print(",");
        _serial->print(params);
    }
    
    _serial->print(MSG_TERMINATOR);
    return true;
}

bool ArduinoBridge::requestStatus() {
    return sendCommand(CMD_GET_STATUS);
}

void ArduinoBridge::update() {
    if (!_initialized || !_serial) {
        return;
    }
    
    // 读取串口数据
    while (_serial->available() > 0) {
        char c = _serial->read();
        
        // 检测消息结束标记
        if (c == MSG_TERMINATOR) {
            if (_bufferIndex > 0) {
                _buffer[_bufferIndex] = '\0';
                _parseMessage(_buffer);
                _bufferIndex = 0;
            }
        } 
        // 存储字符到缓冲区
        else if (_bufferIndex < sizeof(_buffer) - 1) {
            _buffer[_bufferIndex++] = c;
        }
    }
}

void ArduinoBridge::_parseMessage(const char* message) {
    // 解析状态消息
    if (strncmp(message, PREFIX_STATE, strlen(PREFIX_STATE)) == 0) {
        const char* stateData = message + strlen(PREFIX_STATE);
        char* params = nullptr;
        
        // 查找参数分隔符
        params = strchr(const_cast<char*>(stateData), ',');
        if (params) {
            *params = '\0'; // 分隔状态码和参数
            params++; // 指向参数部分
        }
        
        // 解析状态码
        SystemStateCode stateCode = STATE_UNKNOWN;
        int stateInt = atoi(stateData);
        if (stateInt >= 0 && stateInt < STATE_UNKNOWN) {
            stateCode = static_cast<SystemStateCode>(stateInt);
        }
        
        // 调用状态回调函数
        if (_stateCallback) {
            _stateCallback(stateCode, params);
        }
    }
    // 解析日志消息
    else if (strncmp(message, PREFIX_LOG, strlen(PREFIX_LOG)) == 0) {
        const char* logData = message + strlen(PREFIX_LOG);
        char* logMessage = nullptr;
        
        // 查找消息分隔符
        logMessage = strchr(const_cast<char*>(logData), ',');
        if (logMessage) {
            *logMessage = '\0'; // 分隔级别和消息
            logMessage++; // 指向消息部分
            
            // 调用日志回调函数
            if (_logCallback) {
                _logCallback(logData, logMessage);
            }
        }
    }
    // 解析数据消息
    else if (strncmp(message, PREFIX_DATA, strlen(PREFIX_DATA)) == 0) {
        const char* dataInfo = message + strlen(PREFIX_DATA);
        char* dataValues = nullptr;
        
        // 查找数据分隔符
        dataValues = strchr(const_cast<char*>(dataInfo), ',');
        if (dataValues) {
            *dataValues = '\0'; // 分隔数据类型和值
            dataValues++; // 指向值部分
            
            // 调用数据回调函数
            if (_dataCallback) {
                _dataCallback(dataInfo, dataValues);
            }
        }
    }
    // 无法识别的消息类型
    else {
        Serial.print("Unknown message type: ");
        Serial.println(message);
    }
} 