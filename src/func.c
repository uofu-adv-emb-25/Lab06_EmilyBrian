#include "func.h"
#include <stdio.h>

void side_thread_low(void *params)
{
    threadArgs *args = (threadArgs *) params;
    if(xSemaphoreTake(args->semaphore,portMAX_DELAY))
    {
        printf("Low priority thread obtained semaphore\n");
        vTaskDelay(1000/portTICK_PERIOD_MS);
        xSemaphoreGive(args->semaphore);
        printf("Low priority thread released semaphore\n");
    } else
    {
        printf("Low prioirty thread never obtained semaphore\n");
    }
    while(1)
    {}
} 

void side_thread_high(void *params)
{
    threadArgs *args = (threadArgs *) params;
    if(xSemaphoreTake(args->semaphore,portMAX_DELAY))
    {
        printf("High priority thread obtained semaphore\n");
        xSemaphoreGive(args->semaphore);
        printf("High priority thread released semaphore\n");
    } else
    {
        printf("High prioirty thread never obtained semaphore\n");
    }
    while(1)
    {}
} 

void side_thread_medium(void *params)
{
    printf("Medium thread active\n");
    while(1)
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
} 