#include "mqtt_control.h"
#include <WiFi.h>
#include <PubSubClient.h>

// MQTT 配置
const char *mqttBroker = "192.168.3.160"; // 树莓派的 IP 地址
const int mqttPort = 1883;
const char *mqttUser = "mqtt_xiaomi";
const char *mqttPassword = "password";
const char *mqttClientId = "esp32_light_controller";
const char *powerTopic = "homeassistant/light/control/power";
const char *plugTopic = "homeassistant/plug/control/power";
const char *brightnessTopic = "homeassistant/light/control/brightness";

WiFiClient espClient;
PubSubClient client(espClient);

void initMQTT()
{
    client.setServer(mqttBroker, mqttPort);
    while (!client.connected())
    {
        Serial.println("Connecting to MQTT...");
        if (client.connect(mqttClientId, mqttUser, mqttPassword))
        {
            Serial.println("MQTT Connected!");
        }
        else
        {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void setLightPower(bool power)
{
    if (client.connected())
    {
        String payload = power ? "ON" : "OFF";
        client.publish(powerTopic, payload.c_str());
        Serial.printf("Published power: %s\n", payload.c_str());
    }
    else
    {
        Serial.println("MQTT not connected, cannot set power");
    }
}

void setPlugPower(bool power)
{
    if (client.connected())
    {
        String payload = power ? "ON" : "OFF";
        client.publish(plugTopic, payload.c_str());
        Serial.printf("Published power: %s\n", payload.c_str());
    }
    else
    {
        Serial.println("MQTT not connected, cannot set power");
    }
}

void setLightBrightness(int brightness)
{
    if (client.connected())
    {
        String payload = String(brightness);
        client.publish(brightnessTopic, payload.c_str());
        Serial.printf("Published brightness: %d\n", brightness);
    }
    else
    {
        Serial.println("MQTT not connected, cannot set brightness");
    }
}

bool isMQTTConnected()
{
    return client.connected();
}

void loopMQTT()
{
    if (!client.connected())
    {
        initMQTT();
    }
    client.loop();
}