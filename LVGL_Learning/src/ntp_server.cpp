#include "ntp_server.h"
#include "time.h"

// NTP 服务器设置（中国区）
const char* ntpServer1 = "cn.pool.ntp.org";  // 主要服务器：中国区 NTP 池
const char* ntpServer2 = "ntp.aliyun.com";   // 备选服务器：阿里云 NTP
const long gmtOffset_sec = 8 * 3600;         // 设置时区为 GMT+8（中国标准时间）
const int daylightOffset_sec = 0;            // 中国无夏令时，设为 0

void initNTP() {
    // 初始化 NTP 时间同步，使用中国区服务器
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
    Serial.println("NTP time synchronization initialized with China servers");
}

String getCurrentTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return "Failed to obtain time";
    }

    // 格式化时间为 "HH:MM:SS"
    char timeString[20]; // HH:MM:SS + 结尾空字符
    strftime(timeString, sizeof(timeString), "%Y/%m/%d %H:%M:%S", &timeinfo);
    return String(timeString);
}