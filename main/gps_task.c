#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "nmea.h"
#include "gps_task.h"
#include "driver/gpio.h"
#include "hw.h"
#include "spp_task.h"  

#define GPS_UART_NUM UART_NUM_2
#define BUF_SIZE 1024

void gps_task(void *pvParameters)
{
    // preluare context care contine toate primitivele RTOS (queue, semafor, mutex)
    rtos_sync_t *ctx = (rtos_sync_t *)pvParameters;
    configASSERT(ctx && ctx->queue && ctx->msg_sem);

    // === Configurare UART2 pentru modulul GPS ===
    uart_config_t uart_config = {
        .baud_rate = 9600,              
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(GPS_UART_NUM, &uart_config);

    // Conectăm TX GPS -> RX ESP32 si RX GPS <- TX ESP32
    uart_set_pin(GPS_UART_NUM, 16, 17, -1, -1);

    uart_driver_install(GPS_UART_NUM, BUF_SIZE, 0, 0, NULL, 0);

    // Buffere pentru citirea și procesarea propozițiilor NMEA
    char rx[256];    // buffer pentru linia brută citită
    int rx_len = 0;  // lungimea curentă a liniei
    char out[256];   // buffer pentru linia pregatita de trimis prin coada

    while (1) {
        uint8_t c;
        // Citim un caracter de pe UART2 cu timeout 100ms
        int n = uart_read_bytes(GPS_UART_NUM, &c, 1, pdMS_TO_TICKS(100));
        if (n == 1) {
            if (c == '\n' || c == '\r') {
                // S-a terminat o linie NMEA
                if (rx_len > 0) {
                    rx[rx_len] = 0; 

                    // Parsare propozitie NMEA 
                    nmea_fix_t fix = {0};
                    if (nmea_validate(rx) && nmea_parse_sentence(rx, &fix)) {
                        // Daca propozitia este valid - construiesc o linie cu date utile
                        snprintf(out, sizeof(out),
                            "{\"time\":\"%s\",\"date\":\"%s\",\"valid\":%d,"
                            "\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.1f,"
                            "\"sats\":%d,\"speed\":%.2f}\r\n",
                            fix.time_utc[0] ? fix.time_utc : "",
                            fix.date_utc[0] ? fix.date_utc : "",
                            fix.valid ? 1 : 0,
                            fix.lat_deg, fix.lon_deg,
                            fix.alt_m, fix.sats, fix.speed_kn * 1.852
                        );

                        // Trimit in coada mesajul
                        if (xQueueSend(ctx->queue, out, pdMS_TO_TICKS(10)) == pdTRUE) {
                            // Semnalizez prin semafor ca exista mesaj nou
                            xSemaphoreGive(ctx->msg_sem);
                        }

                        // Feedback vizual
                        gpio_set_level(LED_GPIO, 1);
                        vTaskDelay(pdMS_TO_TICKS(10));
                        gpio_set_level(LED_GPIO, 0);
                    } 
                    else {
                        // Propozitia nu este valida 
                        vTaskDelay(pdMS_TO_TICKS(200));  // ~200ms pauză
                        /*snprintf(out, sizeof(out),
                            "{\"time\":\"%s\",\"date\":\"%s\",\"valid\":%d,"
                            "\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.1f,"
                            "\"sats\":%d,\"speed\":%.2f}\r\n",
                            fix.time_utc[0] ? fix.time_utc : "",
                            fix.date_utc[0] ? fix.date_utc : "",
                            fix.valid ? 1 : 0,
                            fix.lat_deg, fix.lon_deg,
                            fix.alt_m, fix.sats, fix.speed_kn * 1.852
                        );
                        if (xQueueSend(ctx->queue, out, 0) == pdTRUE) {
                            xSemaphoreGive(ctx->msg_sem);
                        }
                            */
                    }

                    // Resetare buffer pentru urmatoarea linie
                    rx_len = 0;
                }
            } else if (rx_len < (int)sizeof(rx) - 1) {
                // Adaugare caracter la linia curenta
                rx[rx_len++] = (char)c;
            } else {
                // Overflow - resetare buffer
                rx_len = 0;
            }
        }
    }
}
