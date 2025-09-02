#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "spp_task.h"
#include "driver/gpio.h"
#include "hw.h"

#define SPP_UART_NUM UART_NUM_1

void spp_task(void *pvParameters)
{
    rtos_sync_t *ctx = (rtos_sync_t *)pvParameters;
    configASSERT(ctx && ctx->queue && ctx->msg_sem && ctx->bt_mutex);

    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(SPP_UART_NUM, &uart_config);
    uart_set_pin(SPP_UART_NUM, 4, 21, -1, -1); 
    uart_driver_install(SPP_UART_NUM, 1024, 0, 0, NULL, 0);

    char buf[256];
    TickType_t last_hb = xTaskGetTickCount();

    // Boot message (protejat de mutex)
    const char *boot_msg = "DBG: SPP boot OK\r\n";
    xSemaphoreTake(ctx->bt_mutex, portMAX_DELAY);
    uart_write_bytes(SPP_UART_NUM, boot_msg, strlen(boot_msg));
    xSemaphoreGive(ctx->bt_mutex);

    while (1)
    {
        // Asteapta semaforul "mesaj nou" pana la 100 ms
        if (xSemaphoreTake(ctx->msg_sem, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Citim 1 element din coada (fara blocare - 0)
            if (xQueueReceive(ctx->queue, buf, 0) == pdTRUE) {
                gpio_set_level(LED_GPIO, 1);

                // scriere pe BT protejata de mutex
                xSemaphoreTake(ctx->bt_mutex, portMAX_DELAY);
                uart_write_bytes(SPP_UART_NUM, buf, strlen(buf));
                xSemaphoreGive(ctx->bt_mutex);

                gpio_set_level(LED_GPIO, 0);
            }
        }

        // Heartbeat la 1Hz, protejat de mutex
        if (xTaskGetTickCount() - last_hb >= pdMS_TO_TICKS(1000)) {
            //const char *hb = "DBG: SPP alive 1Hz\r\n";
            xSemaphoreTake(ctx->bt_mutex, portMAX_DELAY);
            //uart_write_bytes(SPP_UART_NUM, hb, strlen(hb));
            xSemaphoreGive(ctx->bt_mutex);
            last_hb = xTaskGetTickCount();
        }
    }
}
