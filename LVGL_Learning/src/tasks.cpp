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
bool socketOn = false;      // 新增：插座开关状态
int selectedDevice = 0;     // 新增：当前选中的设备（0: 灯, 1: 插座）
bool mqttConnected = false; // 新增：MQTT 连接状态
SemaphoreHandle_t lightMutex;

OneButton button(ENCODER_KEY, true, true); // 新增：OneButton 实例，true 表示低电平触发，true 表示启用内部上拉

// 样式定义
static lv_style_t style_selected;
static lv_style_t style_normal;
// 点按回调函数
void onClick()
{
    xSemaphoreTake(lightMutex, portMAX_DELAY);
    if (selectedDevice == 0)
    { // 灯
        lightOn = !lightOn;
        setLightPower(lightOn); // 通过 MQTT 控制灯的开关
        Serial.printf("Light: %s\n", lightOn ? "ON" : "OFF");
    }
    else
    { // 插座
        socketOn = !socketOn;
        setPlugPower(socketOn); // 通过 MQTT 控制插座的开关
        Serial.printf("Socket: %s\n", socketOn ? "ON" : "OFF");
    }
    xSemaphoreGive(lightMutex);
}

// 长按回调函数
void onLongPress()
{
    selectedDevice = (selectedDevice + 1) % 2; // 在 0 和 1 之间切换
    Serial.printf("Selected Device: %s\n", selectedDevice == 0 ? "Light" : "Socket");
}

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
            xSemaphoreTake(lightMutex, portMAX_DELAY);
            if (selectedDevice == 0)
            { // 只有选中灯时才调整亮度
                lightBrightness = counter;
                if (lightBrightness < 0)
                    lightBrightness = 0;
                if (lightBrightness > 100)
                    lightBrightness = 100;
                encoder.setCounter(lightBrightness);
                if (lightOn)
                { // 只有灯开启时才发布亮度命令
                    setLightBrightness(lightBrightness);
                }
            }
            xSemaphoreGive(lightMutex);

            lastCounter = counter;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// 按键任务
void buttonTask(void *parameter)
{
    Serial.begin(115200);
    button.attachClick(onClick);
    button.attachLongPressStart(onLongPress);
    button.setPressMs(1000);

    while (1)
    {
        button.tick();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
// LVGL 任务
void lvglTask(void *parameter)
{
    Serial.begin(115200);
    static int lastSelectedDevice = -1;
    static int lastLightBrightness = -1;
    static bool lastLightOn = false;
    static bool lastSocketOn = false;
    static bool lastMqttConnected = false;

    while (1)
    {
        lv_timer_handler();

        // 更新选中设备的高亮
        xSemaphoreTake(lightMutex, portMAX_DELAY);
        if (selectedDevice != lastSelectedDevice)
        {
            if (selectedDevice == 0)
            {
                // 高亮灯，普通显示插座
                lv_obj_remove_style(ui_Image3, &style_normal, 0);
                lv_obj_add_style(ui_Image3, &style_selected, 0);
                lv_obj_remove_style(ui_Image1, &style_selected, 0);
                lv_obj_add_style(ui_Image1, &style_normal, 0);
                Serial.println("UI: Light selected with red border");
            }
            else
            {
                // 选中插座
                lv_obj_remove_style(ui_Image3, &style_selected, 0);
                lv_obj_add_style(ui_Image3, &style_normal, 0);
                lv_obj_remove_style(ui_Image1, &style_normal, 0);
                lv_obj_add_style(ui_Image1, &style_selected, 0);
                Serial.println("UI: Socket selected with red border");
            }
            lastSelectedDevice = selectedDevice;
        }
        // 更新灯的图标
        if (lightOn != lastLightOn)
        {
            if (lightOn)
            {
                lv_img_set_src(ui_Image3, &ui_img_569232113); // 灯 ON
                Serial.println("UI: Light image set to ON");
            }
            else
            {
                lv_img_set_src(ui_Image3, &ui_img_1634008757); // 灯 OFF
                Serial.println("UI: Light image set to OFF");
            }
            lastLightOn = lightOn;
        }

        // 更新插座的图标和标签（合并逻辑）
        if (socketOn != lastSocketOn)
        {
            if (socketOn)
            {
                lv_img_set_src(ui_Image1, &ui_img_570863325); // 插座 ON
                lv_label_set_text(ui_Label1, "ON");           // 更新标签
                Serial.println("UI: Socket image set to ON");
                Serial.println("UI: Socket state updated to ON");
            }
            else
            {
                lv_img_set_src(ui_Image1, &ui_img_433637665); // 插座 OFF
                lv_label_set_text(ui_Label1, "OFF");          // 更新标签
                Serial.println("UI: Socket image set to OFF");
                Serial.println("UI: Socket state updated to OFF");
            }
            lastSocketOn = socketOn;
        }
        // 更新灯的亮度标签
        if (lightBrightness != lastLightBrightness)
        {
            String brightnessText = String(lightBrightness) + "%";
            lv_label_set_text(ui_Label4, brightnessText.c_str());
            Serial.printf("UI: Light Brightness updated to %d%%\n", lightBrightness);
            lastLightBrightness = lightBrightness;
        }
        // 更新 MQTT 连接状态图标
        if (mqttConnected != lastMqttConnected)
        {
            if (mqttConnected)
            {
                lv_img_set_src(ui_Image2, &ui_img_mqtt_on_png);
                Serial.println("UI: MQTT connected, image set to ON");
            }
            else
            {
                lv_img_set_src(ui_Image2, &ui_img_mqtt_off_png);
                Serial.println("UI: MQTT disconnected, image set to OFF");
            }
            lastMqttConnected = mqttConnected;
        }
        xSemaphoreGive(lightMutex);

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
// 创建MQTT任务的函数
void mqttTask(void *parameter)
{
    Serial.begin(115200);
    while (1)
    {
        // 检查 MQTT 连接状态
        xSemaphoreTake(lightMutex, portMAX_DELAY);
        bool currentMqttState = isMQTTConnected(); // 使用封装的函数
        if (currentMqttState != mqttConnected)
        {
            mqttConnected = currentMqttState;
            Serial.printf("MQTT State: %s\n", mqttConnected ? "Connected" : "Disconnected");
        }
        xSemaphoreGive(lightMutex);

        loopMQTT();
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

    // 初始化样式
    lv_style_init(&style_selected);
    lv_style_set_border_color(&style_selected, lv_color_hex(0xFF0000)); // 红色轮廓
    lv_style_set_border_width(&style_selected, 2);                      // 边框宽度 2 像素
    lv_style_set_border_opa(&style_selected, LV_OPA_100);               // 边框完全不透明
    lv_style_set_radius(&style_selected, LV_RADIUS_CIRCLE);             // 圆形边框
    lv_style_set_border_side(&style_selected, LV_BORDER_SIDE_FULL);     // 完整边框

    lv_style_init(&style_normal);
    lv_style_set_border_width(&style_normal, 0);          // 无边框
    lv_style_set_radius(&style_normal, LV_RADIUS_CIRCLE); // 确保普通样式也是圆形
    // 初始时选中灯
    lv_obj_add_style(ui_Image3, &style_selected, 0);
    lv_obj_add_style(ui_Image1, &style_normal, 0);
    lv_img_set_src(ui_Image3, &ui_img_1634008757);
    lv_img_set_src(ui_Image1, &ui_img_433637665); // 灯 OFF，插座 OFF

    // 初始化标签
    lv_label_set_text(ui_Label4, "100%"); // 灯亮度初始化为 100%
    lv_label_set_text(ui_Label1, "OFF");  // 插座开关初始化为 OFF
    lightMutex = xSemaphoreCreateMutex();
    if (lightMutex == NULL)
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