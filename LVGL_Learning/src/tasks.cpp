#include "tasks.h"
#include <wifi_manager.h>
#include "ntp_server.h" // 引入 NTP 模块
#include "mqtt_control.h"

// 共享变量和信号量定义
int sliderValue = 0;
SemaphoreHandle_t sliderMutex;

// 编码器实例
EncoderRead encoder(ENCODER_S1, ENCODER_S2, ENCODER_KEY);

bool lightOn = false;
int lightBrightness = 100;
SemaphoreHandle_t lightMutex;

// 按键去抖状态
static bool lastButtonState = false;
static unsigned long lastButtonChange = 0;
#define BUTTON_DEBOUNCE_TIME 50

// 编码器任务
void encoderTask(void *parameter)
{
    Serial.begin(115200);
    static int32_t lastCounter = 0;
    while (1)
    {
        int32_t counter = encoder.getCounter();
        if (counter != lastCounter)
        {
            Serial.printf("Encoder Counter: %d\n", counter);
            xSemaphoreTake(sliderMutex, portMAX_DELAY);
            sliderValue = counter;
            if (sliderValue < 0)
                sliderValue = 0;
            if (sliderValue > 100)
                sliderValue = 100;
            encoder.setCounter(sliderValue);
            xSemaphoreTake(lightMutex, portMAX_DELAY);
            lightBrightness = sliderValue;
            if (lightOn)
            {
                setLightBrightness(lightBrightness); // 通过 MQTT 设置亮度
            }
            xSemaphoreGive(lightMutex);
            xSemaphoreGive(sliderMutex);
            lastCounter = counter;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// 按键任务
void buttonTask(void *parameter)
{
    Serial.begin(115200);
    while (1)
    {
        bool currentButtonState = encoder.encBtn();
        unsigned long currentTime = millis();

        if (currentButtonState != lastButtonState && (currentTime - lastButtonChange >= BUTTON_DEBOUNCE_TIME))
        {
            Serial.printf("Button State: %d\n", currentButtonState);
            if (currentButtonState == true)
            {
                while (encoder.encBtn() == true)
                {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                Serial.println("Button Pressed and Released");

                xSemaphoreTake(lightMutex, portMAX_DELAY);
                lightOn = !lightOn;
                setLightPower(lightOn); // 通过 MQTT 设置开关
                xSemaphoreGive(lightMutex);

                const char *currentText = lv_label_get_text(ui_Label6);
                if (strcmp(currentText, "Stop") == 0)
                {
                    lv_label_set_text(ui_Label6, "Open");
                    lv_img_set_src(ui_Image3, &ui_img_569232113);
                    Serial.println("Set to Open, Image: lighton.png");
                }
                else if (strcmp(currentText, "Open") == 0)
                {
                    lv_label_set_text(ui_Label6, "Stop");
                    lv_img_set_src(ui_Image3, &ui_img_1634008757);
                    Serial.println("Set to Stop, Image: lightoff.png");
                }
            }
            lastButtonChange = currentTime;
        }
        lastButtonState = currentButtonState;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
// LVGL 任务
void lvglTask(void *parameter)
{
    Serial.begin(115200);
    static int lastSliderValue = -1;
    while (1)
    {
        lv_timer_handler();
        int currentSliderValue;
        xSemaphoreTake(sliderMutex, portMAX_DELAY);
        currentSliderValue = sliderValue;
        xSemaphoreGive(sliderMutex);
        if (currentSliderValue != lastSliderValue)
        {
            Serial.printf("Updating Slider to: %d\n", currentSliderValue);
            lv_slider_set_value(ui_Slider1, currentSliderValue, LV_ANIM_OFF);
            lv_event_send(ui_Slider1, LV_EVENT_VALUE_CHANGED, NULL);
            lastSliderValue = currentSliderValue;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// WiFi 状态检查任务
void wifiTask(void *parameter)
{
    Serial.begin(115200);
    bool lastWifiState = false;
    while (1)
    {
        bool wifiConnected = isWiFiConnected();
        if (wifiConnected != lastWifiState)
        {
            if (wifiConnected)
            {
                lv_img_set_src(ui_Image4, &ui_img_wifi_on_png);
                Serial.println("WiFi Connected, Image: wifi_on.png");
            }
            else
            {
                lv_img_set_src(ui_Image4, &ui_img_wifi_off_png);
                Serial.println("WiFi Disconnected, Image: wifi_off.png");
            }
            lastWifiState = wifiConnected;
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每 1 秒检查一次
    }
}

// 时间显示任务
void timeTask(void *parameter)
{
    Serial.begin(115200);
    while (1)
    {
        if (isWiFiConnected())
        {
            String currentTime = getCurrentTime(); // 调用 ntp_server 模块的函数
            lv_label_set_text(ui_Label2, currentTime.c_str());
            Serial.println("Time updated: " + currentTime);
        }
        else
        {
            lv_label_set_text(ui_Label2, "No WiFi");
            Serial.println("No WiFi connection, cannot update time");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
// 创建任务的函数
void mqttTask(void *parameter)
    {
        Serial.begin(115200);
        while (1)
        {
            loopMQTT(); // 保持 MQTT 连接
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

void lightTask(void *parameter)
{
    Serial.begin(115200);
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void createTasks()
{
    // 初始化编码器
    encoder.begin();

    // 初始化 Slider 值
    sliderValue = lv_slider_get_value(ui_Slider1);

    // 创建信号量
    sliderMutex = xSemaphoreCreateMutex();
    lightMutex = xSemaphoreCreateMutex();
    if (sliderMutex == NULL || lightMutex == NULL)
    {
        Serial.println("Failed to create mutex!");
        while (1)
            ;
    }
    

    initMQTT(); // 初始化 MQTT
    // 创建任务
    BaseType_t result1 = xTaskCreate(encoderTask, "EncoderTask", 4096, NULL, 2, NULL);
    BaseType_t result2 = xTaskCreate(lvglTask, "LvglTask", 8192, NULL, 1, NULL);
    BaseType_t result3 = xTaskCreate(buttonTask, "ButtonTask", 4096, NULL, 1, NULL);
    BaseType_t result4 = xTaskCreate(wifiTask, "WiFiTask", 4096, NULL, 1, NULL);
    BaseType_t result5 = xTaskCreate(timeTask, "TimeTask", 4096, NULL, 1, NULL);
    BaseType_t result6 = xTaskCreate(lightTask, "LightTask", 4096, NULL, 1, NULL);
    BaseType_t result7 = xTaskCreate(mqttTask, "MqttTask", 4096, NULL, 1, NULL);
}