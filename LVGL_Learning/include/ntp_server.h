#ifndef NTP_SERVER_H
#define NTP_SERVER_H

#include <Arduino.h>

// 初始化 NTP 客户端
void initNTP();

// 获取当前时间的函数
String getCurrentTime();

#endif