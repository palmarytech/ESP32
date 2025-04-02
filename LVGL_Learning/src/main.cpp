#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include "ui.h"
#include "task.h"
#include <tasks.h>
#include <wifi_manager.h>

TFT_eSPI tft = TFT_eSPI(); // TFT 实例

// LVGL 显示缓冲区
#define BUF_SIZE (LV_HOR_RES_MAX * 10)
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[BUF_SIZE];

// 显示刷新回调
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();
  lv_disp_flush_ready(disp);
}


void setup()
{
  // 初始化 TFT
  tft.begin();
  tft.setRotation(1); // 根据需要调整屏幕方向

  // 初始化 LVGL
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, BUF_SIZE);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 170;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // 初始化 SquareLine UI
  ui_init();

  // 打开背光（GPIO15）
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
  // 初始化 Label 文本
  // lv_label_set_text(ui_Label6, "Stop");

  // 初始化 WiFi
  initWiFi();

  // 创建任务
  createTasks();
}

void loop()
{
  // FreeRTOS 环境下 loop() 为空
  vTaskDelay(portMAX_DELAY); // 让出 CPU
}