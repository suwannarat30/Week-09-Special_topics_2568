#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// กำหนดค่าคงที่
#define LED1_GPIO   GPIO_NUM_2
#define LED2_GPIO   GPIO_NUM_4
#define LED3_GPIO   GPIO_NUM_5
#define BLINK_PERIOD 1000   // 1 วินาที (1000ms)

static const char *TAG = "LED_CONTROL";

/**
 * @brief กำหนดค่าเริ่มต้นสำหรับ GPIO
 */
void led_init(void) {
    ESP_LOGI(TAG, "Initializing LEDs on GPIO %d, %d, %d", LED1_GPIO, LED2_GPIO, LED3_GPIO);

    gpio_config_t io_conf = {0};
    io_conf.pin_bit_mask = (1ULL << LED1_GPIO) | (1ULL << LED2_GPIO) | (1ULL << LED3_GPIO);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // ปิด LED ทั้งหมดตอนเริ่ม
    gpio_set_level(LED1_GPIO, 0);
    gpio_set_level(LED2_GPIO, 0);
    gpio_set_level(LED3_GPIO, 0);

    ESP_LOGI(TAG, "LED initialization completed");
}

/**
 * @brief ควบคุม LED ทั้งหมด (เปิด/ปิดพร้อมกัน)
 * @param state: 1 = ON, 0 = OFF
 */
void led_control(int state) {
    gpio_set_level(LED1_GPIO, state);
    gpio_set_level(LED2_GPIO, state);
    gpio_set_level(LED3_GPIO, state);

    ESP_LOGI(TAG, "ALL LEDs %s", state ? "ON" : "OFF");
}

/**
 * @brief Task สำหรับกระพริบ LED ทั้งหมดพร้อมกัน
 */
void led_blink_task(void *pvParameters) {
    int led_state = 0;

    while (1) {
        led_state = !led_state;
        led_control(led_state);

        vTaskDelay(pdMS_TO_TICKS(BLINK_PERIOD));
    }
}

/**
 * @brief Main application entry point
 */
void app_main(void) {
    ESP_LOGI(TAG, "ESP32 LED Control Demo Started");

    // Initialize LEDs
    led_init();

    // Create LED blink task
    xTaskCreate(
        led_blink_task,
        "led_blink_task",
        2048,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "LED blink task created");
}