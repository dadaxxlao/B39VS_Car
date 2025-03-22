#include "Logger.h"

// 定义静态成员变量
Stream* Logger::commStreams[COMM_COUNT] = {nullptr};
bool Logger::commEnabled[COMM_COUNT] = {false}; 