#include <Arduino.h>
#include <WiFi.h>
#include "wifi_manager.h"
#include "ntp_server.h" // 引入 NTP 模块


void initWiFi()
{
    Serial.begin(115200);
    Serial.println("Connecting to WiFi...");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // 等待连接（最多 10 秒）
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        initNTP();

    }
    else
    {
        Serial.println("\nFailed to connect to WiFi");
    }
}

bool isWiFiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}
