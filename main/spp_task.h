#ifndef SPP_TASK_H
#define SPP_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

// Context/handlere comune pentru ambele task-uri.
// - queue     : coada de mesaje GPS - SPP
// - msg_sem   : counting semaphore; semnalizeaza "mesaj nou in coada"
// - bt_mutex  : mutex pentru scriere pe UART1 (Bluetooth)
typedef struct {
    QueueHandle_t     queue;
    SemaphoreHandle_t msg_sem;
    SemaphoreHandle_t bt_mutex;
} rtos_sync_t;

void spp_task(void *pvParameters);

#endif
