SET(SOURCES ui_Screen1.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_wifi_off_png.c
    ui_img_mqtt_off_png.c
    ui_img_1634008757.c
    ui_img_569232113.c
    ui_img_wifi_on_png.c
    ui_img_433637665.c
    ui_img_570863325.c
    ui_img_mqtt_on_png.c)

add_library(ui ${SOURCES})
