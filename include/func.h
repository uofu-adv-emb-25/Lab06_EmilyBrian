#pragma once
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>


#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

#define SIDE_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define SIDE_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

typedef struct threadArgs {
    SemaphoreHandle_t semaphore;
} threadArgs;

void side_thread_low(void *params);
void side_thread_high(void *params);
void side_thread_medium(void *params);