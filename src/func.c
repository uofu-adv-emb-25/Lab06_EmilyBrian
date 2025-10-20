#include "func.h"
#include <stdio.h>

void side_thread_semaphore(void *params)
{
    threadArgs *args = (threadArgs *) params;
    while(1)
    {
        xSemaphoreTake(args->semaphore,PORTMAX_DELAY);
        busy_wait_us(100000);
        xSemaphoreGive(args->semaphore);
    }
} 

void side_thread_medium(void *params)
{
    while(1)
    {
        busy_wait_us(100000);
    }
} 