#include <stdio.h>
#include <pico/stdlib.h>
#include "pico/cyw43_arch.h"
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"
#include "func.h"

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
    semaphore = xSemaphoreCreateBinary();
    threadArgs arg = {semaphore};

    xTaskCreate(side_thread_semaphore, "SideThread",
                SIDE_TASK_STACK_SIZE, (void *) &arg, SIDE_TASK_PRIORITY, &side_low);
    xTaskCreate(side_thread_medium, "SideThread",
                SIDE_TASK_STACK_SIZE, (void *) &arg, SIDE_TASK_PRIORITY+1, &side_medium);
    xTaskCreate(side_thread_semaphore, "SideThread",
                SIDE_TASK_STACK_SIZE, (void *) &arg, SIDE_TASK_PRIORITY+2, &side_high);
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
