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

void test_increment_count(void)
{
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1,1);
    int count = 0;
    int result = increment_counter(semaphore, &count);
    TEST_ASSERT_EQUAL_MESSAGE(result, pdTRUE, "Lock was not acquired");
    TEST_ASSERT_EQUAL_MESSAGE(count, 1, "Count was not incremented");
}

void test_increment_count_fail(void)
{
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1,1);
    int count = 0;
    xSemaphoreTake(semaphore, portMAX_DELAY);
    int result = increment_counter(semaphore, &count);
    TEST_ASSERT_EQUAL_MESSAGE(result, pdFALSE, "Lock was acquired");
    TEST_ASSERT_EQUAL_MESSAGE(count, 0, "Count was incremented");
}


void test_deadlock_case1(void)
{
    SemaphoreHandle_t semaphore_A = xSemaphoreCreateCounting(1,1);
    SemaphoreHandle_t semaphore_B = xSemaphoreCreateCounting(1,1);
    deadlockParams argsA = {semaphore_A,semaphore_B, 3};
    deadlockParams argsB = {semaphore_A,semaphore_B, 0};

    TaskHandle_t task_A, task_B;
    // Create Task A that obtains lock A, pauses, then waits for lock B
    xTaskCreate(taskA, "taskA",
                SIDE_TASK_STACK_SIZE, (void *)&argsA, SIDE_TASK_PRIORITY, &task_A);
                
    // Create Task B that obtains lock B and waits for lock A
    xTaskCreate(taskB, "taskB",
                SIDE_TASK_STACK_SIZE, (void *)&argsB, SIDE_TASK_PRIORITY, &task_B);

    // Allow tasks A and B to lock
    vTaskDelay(1000);

    TEST_ASSERT_EQUAL(argsA.counter, 4);
    TEST_ASSERT_EQUAL(argsB.counter, 1);

    vTaskDelete(task_A);
    vTaskDelete(task_B);
}

void test_orphaned_lock(void)
{
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1,1);

    orphanParams args = {semaphore, 0};

    TaskHandle_t task_orphan;
    xTaskCreate(orphaned_lock, "task_orphan",
            SIDE_TASK_STACK_SIZE, (void *)&args, SIDE_TASK_PRIORITY, &task_orphan);

    // Let thread take, give, then orphan lock
    vTaskDelay(1000);

    TEST_ASSERT_EQUAL(args.counter, 1);
    TEST_ASSERT_EQUAL(0, uxSemaphoreGetCount(args.semaphore));

    vTaskDelete(task_orphan);

}

void test_orphaned_lock_fix(void)
{
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1,1);

    orphanParams args = {semaphore, 0};

    TaskHandle_t task_orphan_fix;
    xTaskCreate(orphaned_lock_fix, "task_orphan_fix",
            SIDE_TASK_STACK_SIZE, (void *)&args, SIDE_TASK_PRIORITY, &task_orphan_fix);

    vTaskDelay(1000);

    TEST_ASSERT_TRUE(args.counter > 1000);
    TEST_ASSERT_EQUAL(1, uxSemaphoreGetCount(args.semaphore));

    vTaskDelete(task_orphan_fix);

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
        RUN_TEST(test_increment_count);
        RUN_TEST(test_increment_count_fail);
        RUN_TEST(test_deadlock_case1);
        RUN_TEST(test_orphaned_lock);
        RUN_TEST(test_orphaned_lock_fix);
        UNITY_END();
        sleep_ms(10000);
    }
}

int main (void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    xTaskCreate(runTestThread,"RunningTests",
                SIDE_TASK_STACK_SIZE,NULL,SIDE_TASK_PRIORITY+1,NULL);
    vTaskStartScheduler();
    return 0;
}
