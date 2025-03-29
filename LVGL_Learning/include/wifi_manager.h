#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

// WiFi 凭据（根据你的网络修改）
#define WIFI_SSID "Family-24"
#define WIFI_PASSWORD "8202240907"

// 初始化 WiFi
void initWiFi();

// 获取 WiFi 连接状态
bool isWiFiConnected();

#endif