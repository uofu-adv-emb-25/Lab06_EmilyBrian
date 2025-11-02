#pragma once
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

#include <FreeRTOS.h>
#include "task.h"

void busy_busy(void *params);
void busy_yield(void *params);