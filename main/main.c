#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "gps_task.h"
#include "spp_task.h"
#include "hw.h"
#include <string.h>
#include "driver/uart.h"
#include "driver/gpio.h"

void monitor_task(void *pvParameters);


void app_main(void)
{
    hw_init();

    // === RTOS sync objects ===
    static rtos_sync_t sync = {0};

    // coada: 10 elemente x 256 bytes 
    sync.queue   = xQueueCreate(10, sizeof(char[256]));
    if (!sync.queue) { printf("Failed to create GPS queue\n"); return; }

    // counting semaphore: max 10 (egal cu capacitatea cozii), initial 0
    sync.msg_sem = xSemaphoreCreateCounting(10, 0);
    if (!sync.msg_sem) { printf("Failed to create msg semaphore\n"); return; }

    // mutex pentru UART1 (BT)
    sync.bt_mutex = xSemaphoreCreateMutex();
    if (!sync.bt_mutex) { printf("Failed to create BT mutex\n"); return; }

    // === Task-uri ===
    xTaskCreate(gps_task, "GPS Task", 4096, &sync, 5, NULL);
    xTaskCreate(spp_task, "SPP Task", 4096, &sync, 4, NULL);
    xTaskCreate(monitor_task, "Monitor Task", 4096, NULL, 2, NULL);

}

void monitor_task(void *pvParameters)
{
    char stats[512];

    while (1) {
        vTaskGetRunTimeStats(stats);
        printf("=== Runtime Stats ===\n%s\n", stats);

        vTaskDelay(pdMS_TO_TICKS(5000)); // o dată la 5 secunde
    }
}