#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include <stdlib.h>
#include <time.h>

#define LED1_GPIO       GPIO_NUM_2
#define LED2_GPIO       GPIO_NUM_4
#define LED3_GPIO       GPIO_NUM_5

#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES   LEDC_TIMER_13_BIT   // 8192 levels
#define LEDC_FREQUENCY  5000                // 5 kHz
#define FADE_TIME       1000                // 1 วินาทีต่อการหายใจ (in/out)

static const char *TAG = "PWM_BREATHING";

// =============================
// 🔹 กำหนด channel สำหรับแต่ละ LED
// =============================
ledc_channel_config_t ledc_channels[] = {
    { .channel = LEDC_CHANNEL_0, .duty = 0, .gpio_num = LED1_GPIO, .speed_mode = LEDC_MODE, .hpoint = 0, .timer_sel = LEDC_TIMER },
    { .channel = LEDC_CHANNEL_1, .duty = 0, .gpio_num = LED2_GPIO, .speed_mode = LEDC_MODE, .hpoint = 0, .timer_sel = LEDC_TIMER },
    { .channel = LEDC_CHANNEL_2, .duty = 0, .gpio_num = LED3_GPIO, .speed_mode = LEDC_MODE, .hpoint = 0, .timer_sel = LEDC_TIMER }
};

// =============================
// 🔹 Init LEDC
// =============================
void ledc_init(void) {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[i]));
    }

    ESP_ERROR_CHECK(ledc_fade_func_install(0));
}

// =============================
// 🔹 Helper: fade LED ON หรือ OFF
// =============================
void led_fade(int led_index, int on) {
    int duty = on ? 8191 : 0;
    ESP_ERROR_CHECK(ledc_set_fade_with_time(LEDC_MODE,
                                            ledc_channels[led_index].channel,
                                            duty, FADE_TIME));
    ESP_ERROR_CHECK(ledc_fade_start(LEDC_MODE,
                                    ledc_channels[led_index].channel,
                                    LEDC_FADE_WAIT_DONE));
}

// =============================
// 🔹 Pattern 1: Knight Rider
// =============================
void knight_rider_pattern(void) {
    static int pos = 0, dir = 1;
    for (int i = 0; i < 3; i++) led_fade(i, 0);
    led_fade(pos, 1);
    pos += dir;
    if (pos == 2 || pos == 0) dir = -dir;
}

// =============================
// 🔹 Pattern 2: Binary Counter
// =============================
void binary_counter_pattern(void) {
    static int counter = 0;
    for (int i = 0; i < 3; i++) {
        int state = (counter >> i) & 1;
        led_fade(i, state);
    }
    counter = (counter + 1) % 8;
}

// =============================
// 🔹 Pattern 3: Random Blinking
// =============================
void random_blink_pattern(void) {
    for (int i = 0; i < 3; i++) {
        int r = rand() % 2;
        led_fade(i, r);
    }
}

// =============================
// 🔹 แสดงชื่อโหมด
// =============================
void print_mode(int mode) {
    switch (mode) {
        case 0: ESP_LOGI(TAG, "Mode 0: Knight Rider (Breathing)"); break;
        case 1: ESP_LOGI(TAG, "Mode 1: Binary Counter (Breathing)"); break;
        case 2: ESP_LOGI(TAG, "Mode 2: Random Blinking (Breathing)"); break;
    }
}

// =============================
// 🔹 Task รวมทุกโหมด
// =============================
void led_pattern_task(void *pvParameters) {
    srand((unsigned)time(NULL));
    int mode = 0;
    int counter = 0;

    print_mode(mode);

    while (1) {
        switch (mode) {
            case 0: knight_rider_pattern(); break;
            case 1: binary_counter_pattern(); break;
            case 2: random_blink_pattern(); break;
        }
        vTaskDelay(pdMS_TO_TICKS(FADE_TIME));
        counter++;

        // เปลี่ยนโหมดทุก 10 วินาที
        if (counter >= (10000 / FADE_TIME)) {
            counter = 0;
            mode = (mode + 1) % 3;
            print_mode(mode);
        }
    }
}

// =============================
// 🔹 Main
// =============================
void app_main(void) {
    ESP_LOGI(TAG, "ESP32 LED Breathing Pattern Started");
    ledc_init();

    xTaskCreate(led_pattern_task, "led_pattern_task", 4096, NULL, 5, NULL);
}