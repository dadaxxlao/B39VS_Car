#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "CommProtocol.h"

// 蓝牙服务UUID
#define SERVICE_UUID        "4FAFC201-1FB5-459E-8FCC-C5C9C331914B"
// 命令特征UUID (写入)
#define COMMAND_CHAR_UUID   "BEB5483E-36E1-4688-B7F5-EA07361B26A8"
// 状态特征UUID (读取/通知)
#define STATUS_CHAR_UUID    "5FB5483E-36E1-4688-B7F5-EA07361B26A9"
// 日志特征UUID (读取/通知)
#define LOG_CHAR_UUID       "6FB5483E-36E1-4688-B7F5-EA07361B26AA"
// 数据特征UUID (读取/通知)
#define DATA_CHAR_UUID      "7FB5483E-36E1-4688-B7F5-EA07361B26AB"

/**
 * 蓝牙服务模块
 * 提供蓝牙通信功能
 */
class BLEServiceManager {
public:
    // 命令回调函数类型
    using CommandCallback = std::function<void(const char*)>;
    // 连接状态改变回调函数类型
    using ConnectionCallback = std::function<void(bool)>;

private:
    // 蓝牙服务器
    BLEServer* _pServer = nullptr;
    // 蓝牙服务
    BLEService* _pService = nullptr;
    // 特征
    BLECharacteristic* _pCommandCharacteristic = nullptr;
    BLECharacteristic* _pStatusCharacteristic = nullptr;
    BLECharacteristic* _pLogCharacteristic = nullptr;
    BLECharacteristic* _pDataCharacteristic = nullptr;
    
    // 设备名称
    String _deviceName;
    // 设备是否已连接
    bool _deviceConnected = false;
    // 上次通知时间
    uint32_t _lastNotifyTime = 0;
    
    // 命令回调函数
    CommandCallback _commandCallback = nullptr;
    // 连接状态回调函数
    ConnectionCallback _connectionCallback = nullptr;
    
    // 服务器回调类
    class ServerCallbacks : public BLEServerCallbacks {
    private:
        BLEServiceManager* _manager;
    public:
        ServerCallbacks(BLEServiceManager* manager) : _manager(manager) {}
        
        void onConnect(BLEServer* pServer) override {
            _manager->_deviceConnected = true;
            if (_manager->_connectionCallback) {
                _manager->_connectionCallback(true);
            }
        }
        
        void onDisconnect(BLEServer* pServer) override {
            _manager->_deviceConnected = false;
            if (_manager->_connectionCallback) {
                _manager->_connectionCallback(false);
            }
            // 重新开始广播
            pServer->startAdvertising();
        }
    };
    
    // 命令特征回调类
    class CommandCallbacks : public BLECharacteristicCallbacks {
    private:
        BLEServiceManager* _manager;
    public:
        CommandCallbacks(BLEServiceManager* manager) : _manager(manager) {}
        
        void onWrite(BLECharacteristic* pCharacteristic) override {
            std::string value = pCharacteristic->getValue();
            if (value.length() > 0 && _manager->_commandCallback) {
                _manager->_commandCallback(value.c_str());
            }
        }
    };

public:
    /**
     * 构造函数
     * @param deviceName 蓝牙设备名称
     */
    BLEServiceManager(const String& deviceName = "B39VS_Car");
    
    /**
     * 初始化蓝牙服务
     * @return 初始化是否成功
     */
    bool begin();
    
    /**
     * 设置命令回调函数
     * @param callback 命令回调函数
     */
    void setCommandCallback(CommandCallback callback);
    
    /**
     * 设置连接状态回调函数
     * @param callback 连接状态回调函数
     */
    void setConnectionCallback(ConnectionCallback callback);
    
    /**
     * 发送状态更新
     * @param state 系统状态
     * @param extraParams 额外参数，可为nullptr
     * @return 发送是否成功
     */
    bool sendStatus(SystemStateCode state, const char* extraParams = nullptr);
    
    /**
     * 发送日志信息
     * @param level 日志级别
     * @param message 日志消息
     * @return 发送是否成功
     */
    bool sendLog(const char* level, const char* message);
    
    /**
     * 发送数据信息
     * @param dataType 数据类型
     * @param values 数据值
     * @return 发送是否成功
     */
    bool sendData(const char* dataType, const char* values);
    
    /**
     * 检查设备是否已连接
     * @return 连接状态
     */
    bool isConnected() const { return _deviceConnected; }
    
    /**
     * 更新函数，在主循环中调用
     */
    void update();
};

extern BLEServiceManager BLEService;

#endif // BLE_SERVICE_H 