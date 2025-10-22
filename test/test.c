#include <stdio.h>
#include <pico/stdlib.h>
#include "pico/cyw43_arch.h"
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"
#include "func.h"
#include <FreeRTOS.h>
#include "task.h"
#include <pico/time.h>

void setUp(void) 
{        
    printf("Start test\n");
}

void tearDown(void) 
{
    printf("Finished test\n");
}

void test_variable_assignment()
{
    int x = 1;
    TEST_ASSERT_TRUE_MESSAGE(x == 1,"Variable assignment failed.");
}

void test_multiplication(void)
{
    int x = 30;
    int y = 6;
    int z = x / y;
    TEST_ASSERT_TRUE_MESSAGE(z == 5, "Multiplication of two integers returned incorrect value.");
}

void test_priority_inversion(void)
{
    TaskHandle_t main, side_low, side_medium, side_high;
    SemaphoreHandle_t semaphore = xSemaphoreCreateBinary();
    threadArgs arg = {semaphore};

    TickType_t start_ticks = xTaskGetTickCount();
    configRUN_TIME_COUNTER_TYPE start_count = portGET_RUN_TIME_COUNTER_VALUE();

    xTaskCreate(side_thread_low, "Thread1",
                SIDE_TASK_STACK_SIZE, (void *) &arg, SIDE_TASK_PRIORITY, &side_low);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    xTaskCreate(side_thread_medium, "Thread2",
                SIDE_TASK_STACK_SIZE, (void *) &arg, SIDE_TASK_PRIORITY+1, &side_medium);
    xTaskCreate(side_thread_high, "Thread3",
                SIDE_TASK_STACK_SIZE, (void *) &arg, SIDE_TASK_PRIORITY+2, &side_high);
    
    vTaskDelay(4000/portTICK_PERIOD_MS);
    
    TickType_t end_ticks = xTaskGetTickCount();
    configRUN_TIME_COUNTER_TYPE end_count = portGET_RUN_TIME_COUNTER_VALUE();
    configRUN_TIME_COUNTER_TYPE low = ulTaskGetRunTimeCounter(side_low);
    configRUN_TIME_COUNTER_TYPE medium = ulTaskGetRunTimeCounter(side_medium);
    configRUN_TIME_COUNTER_TYPE high = ulTaskGetRunTimeCounter(side_high);
    configRUN_TIME_COUNTER_TYPE elapsed = end_count - start_count;
    TickType_t elapsed_ticks = end_ticks - start_ticks;

    printf("low %lld medium %lld high %lld start %lld end %lld elapsed %lld (us) in %d to %d ticks\n",
            low, medium, high, start_count, end_count, elapsed, start_ticks, end_ticks);

    vTaskDelete(side_low);
    vTaskDelete(side_medium);
    vTaskDelete(side_high);
}

//Main test running thread
void runTestThread(__unused void *args)
{
    while(1) 
    {
        sleep_ms(10000);
        UNITY_BEGIN();
        RUN_TEST(test_variable_assignment);
        RUN_TEST(test_multiplication);
        RUN_TEST(test_priority_inversion);

        UNITY_END();
        sleep_ms(10000);
    }
}

int main (void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    xTaskCreate(runTestThread,"RunningTests",
                SIDE_TASK_STACK_SIZE,NULL,SIDE_TASK_PRIORITY+4,NULL);
    vTaskStartScheduler();
    return 0;
}
