#include "func.h"
#include <stdio.h>

void side_thread_low(void *params)
{
    threadArgs *args = (threadArgs *) params;
    if(xSemaphoreTake(args->semaphore,portMAX_DELAY))
    {
        printf("Low priority thread obtained semaphore");
        vTaskDelay(1000/portTICK_PERIOD_MS);
        xSemaphoreGive(args->semaphore);
        printf("Low priority thread released semaphore");
    } else
    {
        printf("Low prioirty thread never obtained semaphore");
    }
    while(1)
    {}
} 

void side_thread_high(void *params)
{
    threadArgs *args = (threadArgs *) params;
    if(xSemaphoreTake(args->semaphore,portMAX_DELAY))
    {
        printf("High priority thread obtained semaphore");
        xSemaphoreGive(args->semaphore);
        printf("High priority thread released semaphore");
    } else
    {
        printf("High prioirty thread never obtained semaphore");
    }
    while(1)
    {}
} 

void side_thread_medium(void *params)
{
    printf("Medium thread active");
    while(1)
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
} 