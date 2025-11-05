#include "busy.h"

void busy_busy(void *params)
{
    for (int i = 0; ; i++);
}

void busy_yield(void *params)
{
    for (int i = 0; ; i++) {
        taskYIELD();
    }
}