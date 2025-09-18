#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// =============================
// 🔹 กำหนดค่าคงที่
// =============================
#define LED1_GPIO   GPIO_NUM_2
#define LED2_GPIO   GPIO_NUM_4
#define LED3_GPIO   GPIO_NUM_5
#define BLINK_PERIOD 300    // หน่วงเวลา (ms)

static const char *TAG = "LED_PATTERN";

// =============================
// 🔹 ฟังก์ชันช่วยเหลือ
// =============================
void led_init(void) {
    gpio_config_t io_conf = {0};
    io_conf.pin_bit_mask = (1ULL << LED1_GPIO) | (1ULL << LED2_GPIO) | (1ULL << LED3_GPIO);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(LED1_GPIO, 0);
    gpio_set_level(LED2_GPIO, 0);
    gpio_set_level(LED3_GPIO, 0);
}

void led_set(int led1, int led2, int led3) {
    gpio_set_level(LED1_GPIO, led1);
    gpio_set_level(LED2_GPIO, led2);
    gpio_set_level(LED3_GPIO, led3);
}

// =============================
// 🔹 Pattern 1: Knight Rider
// =============================
void knight_rider_pattern(void) {
    static int pos = 0;
    static int dir = 1;
    led_set(0,0,0);

    if (pos == 0) led_set(1,0,0);
    else if (pos == 1) led_set(0,1,0);
    else if (pos == 2) led_set(0,0,1);

    pos += dir;
    if (pos == 2 || pos == 0) dir = -dir;
}

// =============================
// 🔹 Pattern 2: Binary Counter
// =============================
void binary_counter_pattern(void) {
    static int counter = 0;
    int b1 = (counter >> 0) & 1;
    int b2 = (counter >> 1) & 1;
    int b3 = (counter >> 2) & 1;
    led_set(b1, b2, b3);
    counter = (counter + 1) % 8;
}

// =============================
// 🔹 Pattern 3: Random Blinking
// =============================
void random_blink_pattern(void) {
    int r1 = rand() % 2;
    int r2 = rand() % 2;
    int r3 = rand() % 2;
    led_set(r1, r2, r3);
}

// =============================
// 🔹 Task รวมทุกโหมด
// =============================
void led_pattern_task(void *pvParameters) {
    srand((unsigned)time(NULL));
    int mode = 0;
    int counter = 0;

    while (1) {
        switch (mode) {
            case 0: knight_rider_pattern(); break;
            case 1: binary_counter_pattern(); break;
            case 2: random_blink_pattern(); break;
        }

        vTaskDelay(pdMS_TO_TICKS(BLINK_PERIOD));
        counter++;

        // เปลี่ยนโหมดทุก 10 วินาที
        if (counter >= (10000 / BLINK_PERIOD)) {
            counter = 0;
            mode = (mode + 1) % 3;
            ESP_LOGI(TAG, "Switching to Mode %d", mode);
        }
    }
}

// =============================
// 🔹 Main
// =============================
void app_main(void) {
    ESP_LOGI(TAG, "ESP32 LED Pattern Demo Started");
    led_init();

    // รัน Task ที่รวม Pattern ทั้งหมด
    xTaskCreate(led_pattern_task, "led_pattern_task", 2048, NULL, 5, NULL);
}