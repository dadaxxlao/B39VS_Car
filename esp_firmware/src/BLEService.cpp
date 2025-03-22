#include "BLEService.h"

// 全局实例
BLEServiceManager BLEService;

BLEServiceManager::BLEServiceManager(const String& deviceName) : _deviceName(deviceName) {
}

bool BLEServiceManager::begin() {
    // 创建BLE设备
    BLEDevice::init(_deviceName.c_str());
    
    // 创建BLE服务器
    _pServer = BLEDevice::createServer();
    if (!_pServer) {
        Serial.println("Failed to create BLE server");
        return false;
    }
    
    // 设置服务器回调
    _pServer->setCallbacks(new ServerCallbacks(this));
    
    // 创建BLE服务
    _pService = _pServer->createService(SERVICE_UUID);
    if (!_pService) {
        Serial.println("Failed to create BLE service");
        return false;
    }
    
    // 创建特征
    // 命令特征（写入）
    _pCommandCharacteristic = _pService->createCharacteristic(
        COMMAND_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    _pCommandCharacteristic->setCallbacks(new CommandCallbacks(this));
    
    // 状态特征（读取/通知）
    _pStatusCharacteristic = _pService->createCharacteristic(
        STATUS_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _pStatusCharacteristic->addDescriptor(new BLE2902());
    
    // 日志特征（读取/通知）
    _pLogCharacteristic = _pService->createCharacteristic(
        LOG_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _pLogCharacteristic->addDescriptor(new BLE2902());
    
    // 数据特征（读取/通知）
    _pDataCharacteristic = _pService->createCharacteristic(
        DATA_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _pDataCharacteristic->addDescriptor(new BLE2902());
    
    // 启动服务
    _pService->start();
    
    // 开始广播
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // iPhone连接参数请求的最小连接间隔
    pAdvertising->setMinPreferred(0x12);  // iPhone连接参数请求的最大连接间隔
    BLEDevice::startAdvertising();
    
    Serial.println("BLE service started, waiting for connections...");
    return true;
}

void BLEServiceManager::setCommandCallback(CommandCallback callback) {
    _commandCallback = callback;
}

void BLEServiceManager::setConnectionCallback(ConnectionCallback callback) {
    _connectionCallback = callback;
}

bool BLEServiceManager::sendStatus(SystemStateCode state, const char* extraParams) {
    if (!_deviceConnected) {
        return false;
    }
    
    String statusStr = String(PREFIX_STATE) + stateCodeToString(state);
    if (extraParams != nullptr) {
        statusStr += "," + String(extraParams);
    }
    
    _pStatusCharacteristic->setValue(statusStr.c_str());
    _pStatusCharacteristic->notify();
    _lastNotifyTime = millis();
    return true;
}

bool BLEServiceManager::sendLog(const char* level, const char* message) {
    if (!_deviceConnected) {
        return false;
    }
    
    String logStr = String(PREFIX_LOG) + level + "," + message;
    _pLogCharacteristic->setValue(logStr.c_str());
    _pLogCharacteristic->notify();
    _lastNotifyTime = millis();
    return true;
}

bool BLEServiceManager::sendData(const char* dataType, const char* values) {
    if (!_deviceConnected) {
        return false;
    }
    
    String dataStr = String(PREFIX_DATA) + dataType + "," + values;
    _pDataCharacteristic->setValue(dataStr.c_str());
    _pDataCharacteristic->notify();
    _lastNotifyTime = millis();
    return true;
}

void BLEServiceManager::update() {
    // 目前没有需要定期更新的内容
    // 如果需要定期发送状态，可以在这里添加
} 