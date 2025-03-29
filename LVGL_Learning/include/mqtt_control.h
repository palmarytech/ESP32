#ifndef MQTT_CONTROL_H
#define MQTT_CONTROL_H

#include <Arduino.h>

// 初始化MQTT
void initMQTT();

// 控制台灯开关
void setLightPower(bool power);

// 设置台灯亮度
void setLightBrightness(int brightness);

void loopMQTT();

#endif