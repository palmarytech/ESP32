#ifndef MQTT_CONTROL_H
#define MQTT_CONTROL_H

#include <Arduino.h>

// 初始化MQTT
void initMQTT();

// 控制台灯开关
void setLightPower(bool power);

// 控制插座开关
void setPlugPower(bool power);

// 设置台灯亮度
void setLightBrightness(int brightness);

void loopMQTT();

// 检查 MQTT 连接状态
bool isMQTTConnected();

#endif