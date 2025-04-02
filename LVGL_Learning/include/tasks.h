#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "EncoderRead.h"
#include "ui.h"
#include "mqtt_control.h"
#include <OneButton.h>

// 旋转编码器引脚定义
#define ENCODER_S1 1  // A 相 (CLK)
#define ENCODER_S2  2  // B 相 (DT)
#define ENCODER_KEY  16 // 按键 (Key)

// 共享变量和信号量
extern int sliderValue;
extern SemaphoreHandle_t sliderMutex;
// 编码器实例
extern EncoderRead encoder;

extern bool lightOn;
extern int lightBrightness;
extern bool socketOn; // 新增：插座开关状态
extern int selectedDevice; // 新增：当前选中的设备（0: 灯, 1: 插座）
extern bool mqttConnected; // 新增：MQTT 连接状态
//lightMutex 是一个互斥信号量，用于保护 lightOn 和 lightBrightness，确保同一时间只有一个任务可以访问或修改它们
extern SemaphoreHandle_t lightMutex;

extern OneButton button; // 新增：OneButton 实例
// 任务创建函数
void createTasks();

// 任务函数声明（供调试使用，可选）
void encoderTask(void *parameter);
void buttonTask(void *parameter);
void lvglTask(void *parameter);
void wifiTask(void *parameter); // 新增 WiFi 任务
void timeTask(void *parameter); // 新增时间任务
void lightTask(void *parameter);
void mqttTask(void *parameter);

#endif