#include <stdio.h>
#include <pico/stdlib.h>
#include "pico/cyw43_arch.h"
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"

#include "func.h"
#include "busy.h"

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

//ACTIVITY 0
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

    configRUN_TIME_COUNTER_TYPE sum = low + medium + high;

    configRUN_TIME_COUNTER_TYPE low_percent = (low*100)/sum;
    configRUN_TIME_COUNTER_TYPE medium_percent = (medium*100)/sum;
    configRUN_TIME_COUNTER_TYPE high_percent = (high*100)/sum;

    configRUN_TIME_COUNTER_TYPE elapsed = end_count - start_count;
    TickType_t elapsed_ticks = end_ticks - start_ticks;

    printf("low %lld medium %lld high %lld start %lld end %lld elapsed %lld (us) in %d to %d ticks\n",
            low, medium, high, start_count, end_count, elapsed, start_ticks, end_ticks);

    printf("low %lld medium %lld high %lld sum %lld\n",low_percent, medium_percent, high_percent,sum);

    vTaskDelete(side_low);
    vTaskDelete(side_medium);
    vTaskDelete(side_high);

    //TEST_ASSERT_UINT64_WITHIN(uint64_t(15),uint64_t(85),medium_percent);
    TEST_ASSERT_TRUE_MESSAGE(medium_percent > 80, "Medium thread does not starve the processor enough\n");
    TEST_ASSERT_TRUE_MESSAGE(low_percent < 15, "Low thread runs more than 15 percent of the time\n");
    TEST_ASSERT_TRUE_MESSAGE(high_percent < 15, "High thread runs more than 15 percent of the time\n");
}

//ACTIVITY 1
void test_priority_inversion_mutex(void)
{
    TaskHandle_t main, side_low, side_medium, side_high;
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    threadArgs arg = {mutex};

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

    configRUN_TIME_COUNTER_TYPE sum = low + medium + high;

    configRUN_TIME_COUNTER_TYPE low_percent = (low*100)/sum;
    configRUN_TIME_COUNTER_TYPE medium_percent = (medium*100)/sum;
    configRUN_TIME_COUNTER_TYPE high_percent = (high*100)/sum;

    configRUN_TIME_COUNTER_TYPE elapsed = end_count - start_count;
    TickType_t elapsed_ticks = end_ticks - start_ticks;

    printf("low %lld medium %lld high %lld start %lld end %lld elapsed %lld (us) in %d to %d ticks\n",
            low, medium, high, start_count, end_count, elapsed, start_ticks, end_ticks);

    printf("low %lld medium %lld high %lld sum %lld\n",low_percent, medium_percent, high_percent,sum);

    vTaskDelete(side_low);
    vTaskDelete(side_medium);
    vTaskDelete(side_high);

    //Priority Inheritance should fix this
    TEST_ASSERT_TRUE_MESSAGE(medium_percent < 10, "Medium thread runs more than 10 percent of the time\n");
    TEST_ASSERT_TRUE_MESSAGE(low_percent < 10, "Low thread runs more than 10 percent of the time\n");
    TEST_ASSERT_TRUE_MESSAGE(high_percent > 80, "High thread runs less than 80 percent of the time\n");
}

//ACTIVITY 2.1
void runtime_analysis(TaskFunction_t task1Name,
                    int task1Priority,
                    configRUN_TIME_COUNTER_TYPE *task1runtime,

                    TaskFunction_t task2Name,
                    int task2Priority,
                    configRUN_TIME_COUNTER_TYPE *task2runtime,

                    configRUN_TIME_COUNTER_TYPE *total_runtime,
                    TickType_t *total_ticks,
                    configRUN_TIME_COUNTER_TYPE t1_t2_delay_ms
)
{
    TaskHandle_t thread1, thread2;
    TickType_t start_ticks = xTaskGetTickCount();
    configRUN_TIME_COUNTER_TYPE start_count = portGET_RUN_TIME_COUNTER_VALUE();

    xTaskCreate(task1Name, "Thread1",
                SIDE_TASK_STACK_SIZE, NULL, task1Priority, &thread1);
    vTaskDelay(t1_t2_delay_ms/portTICK_PERIOD_MS);
    xTaskCreate(task2Name, "Thread2",
                SIDE_TASK_STACK_SIZE, NULL, task2Priority, &thread2);
    vTaskDelay(4000/portTICK_PERIOD_MS);

    TickType_t end_ticks = xTaskGetTickCount();
    configRUN_TIME_COUNTER_TYPE end_count = portGET_RUN_TIME_COUNTER_VALUE();
    
    *task1runtime = ulTaskGetRunTimeCounter(thread1);
    *task2runtime= ulTaskGetRunTimeCounter(thread2);

    *total_runtime = end_count - start_count;
    *total_ticks = end_ticks - start_ticks;

    printf("task1 %lld task2 %lld start %lld end %lld elapsed %lld (us) in %d to %d ticks\n",
            *task1runtime, *task2runtime, start_count, end_count, *total_runtime, start_ticks, end_ticks);
    
    vTaskDelete(thread1);
    vTaskDelete(thread2);
}

void busy_busy_same(void) 
{    
    uint64_t thread1_time = 0;
    uint64_t thread2_time = 0;
    uint64_t total_time = 0;
    TickType_t total_ticks = 0;
    runtime_analysis(busy_busy,SIDE_TASK_PRIORITY,&thread1_time, //thread 1
                     busy_busy,SIDE_TASK_PRIORITY,&thread2_time, //thread 2
        &total_time, &total_ticks, 0
    );

    configRUN_TIME_COUNTER_TYPE sum = thread1_time + thread2_time;
    configRUN_TIME_COUNTER_TYPE thread1_percent = (thread1_time*100)/sum;
    configRUN_TIME_COUNTER_TYPE thread2_percent = (thread2_time*100)/sum;

    printf("task1 %lld task2 %lld sum %lld\n", thread1_percent, thread2_percent, sum);


    TEST_ASSERT_TRUE_MESSAGE(thread1_percent > 40, "Task1 less than 40 percent\n");
    TEST_ASSERT_TRUE_MESSAGE(thread1_percent < 60, "Task1 greater than 60 percent\n");

    TEST_ASSERT_TRUE_MESSAGE(thread2_percent > 40, "Task2 less than 40 percent\n");
    TEST_ASSERT_TRUE_MESSAGE(thread2_percent < 60, "Task2 greater than 60 percent\n");
}

void yield_yield_same(void) 
{    
    uint64_t thread1_time = 0;
    uint64_t thread2_time = 0;
    uint64_t total_time = 0;
    TickType_t total_ticks = 0;
    runtime_analysis(busy_yield,SIDE_TASK_PRIORITY,&thread1_time, //thread 1
                     busy_yield,SIDE_TASK_PRIORITY,&thread2_time, //thread 2
        &total_time, &total_ticks, 0
    );

    configRUN_TIME_COUNTER_TYPE sum = thread1_time + thread2_time;
    configRUN_TIME_COUNTER_TYPE thread1_percent = (thread1_time*100)/sum;
    configRUN_TIME_COUNTER_TYPE thread2_percent = (thread2_time*100)/sum;

    printf("task1 %lld task2 %lld sum %lld\n", thread1_percent, thread2_percent, sum);


    TEST_ASSERT_TRUE_MESSAGE(thread1_percent > 40, "Task1 less than 40 percent\n");
    TEST_ASSERT_TRUE_MESSAGE(thread1_percent < 60, "Task1 greater than 60 percent\n");

    TEST_ASSERT_TRUE_MESSAGE(thread2_percent > 40, "Task2 less than 40 percent\n");
    TEST_ASSERT_TRUE_MESSAGE(thread2_percent < 60, "Task2 greater than 60 percent\n");
}

void busy_yield_same(void) 
{    
    uint64_t thread1_time = 0;
    uint64_t thread2_time = 0;
    uint64_t total_time = 0;
    TickType_t total_ticks = 0;
    runtime_analysis(busy_busy,SIDE_TASK_PRIORITY,&thread1_time,  //thread 1
                     busy_yield,SIDE_TASK_PRIORITY,&thread2_time, //thread 2
        &total_time, &total_ticks, 0
    );

    configRUN_TIME_COUNTER_TYPE sum = thread1_time + thread2_time;
    configRUN_TIME_COUNTER_TYPE thread1_percent = (thread1_time*100)/sum;
    configRUN_TIME_COUNTER_TYPE thread2_percent = (thread2_time*100)/sum;

    printf("task1 %lld task2 %lld sum %lld\n", thread1_percent, thread2_percent, sum);


    TEST_ASSERT_TRUE_MESSAGE(thread1_percent > 90, "Task1 less than 90 percent\n");

    TEST_ASSERT_TRUE_MESSAGE(thread2_percent < 10, "Task2 greater than 10 percent\n");
}

//ACTIVITY 2.2
void busy_busy_highFirst(void)
{    
    uint64_t thread1_time = 0;
    uint64_t thread2_time = 0;
    uint64_t total_time = 0;
    TickType_t total_ticks = 0;
    runtime_analysis(busy_busy,SIDE_TASK_PRIORITY+1,&thread1_time, //thread 1
        busy_busy,SIDE_TASK_PRIORITY,&thread2_time,              //thread 2
        &total_time, &total_ticks, 100
    );

    configRUN_TIME_COUNTER_TYPE sum = thread1_time + thread2_time;
    configRUN_TIME_COUNTER_TYPE thread1_percent = (thread1_time*100)/sum;
    configRUN_TIME_COUNTER_TYPE thread2_percent = (thread2_time*100)/sum;

    printf("task1 %lld task2 %lld sum %lld\n", thread1_percent, thread2_percent, sum);


    TEST_ASSERT_TRUE_MESSAGE(thread1_percent > 90, "High Priority less than than 90 percent\n");
    TEST_ASSERT_TRUE_MESSAGE(thread2_percent < 10, "Low Priority greater than 10 percent\n");
}

void busy_busy_lowFirst(void) 
{    
    uint64_t thread1_time = 0;
    uint64_t thread2_time = 0;
    uint64_t total_time = 0;
    TickType_t total_ticks = 0;
    runtime_analysis(busy_busy,SIDE_TASK_PRIORITY,&thread1_time, //thread 1
        busy_busy,SIDE_TASK_PRIORITY+1,&thread2_time,              //thread 2
        &total_time, &total_ticks, 100
    );

    configRUN_TIME_COUNTER_TYPE sum = thread1_time + thread2_time;
    configRUN_TIME_COUNTER_TYPE thread1_percent = (thread1_time*100)/sum;
    configRUN_TIME_COUNTER_TYPE thread2_percent = (thread2_time*100)/sum;

    printf("task1 %lld task2 %lld sum %lld\n", thread1_percent, thread2_percent, sum);

    TEST_ASSERT_TRUE_MESSAGE(thread1_percent < 10, "Low Priority greater than 10 percent\n");
    TEST_ASSERT_TRUE_MESSAGE(thread2_percent > 90, "High Priority less than 90 percent\n");
}

void busy_yield_highFirst(void) 
{    
    uint64_t thread1_time = 0;
    uint64_t thread2_time = 0;
    uint64_t total_time = 0;
    TickType_t total_ticks = 0;
    runtime_analysis(busy_yield,SIDE_TASK_PRIORITY+1,&thread1_time, //thread 1
        busy_yield,SIDE_TASK_PRIORITY,&thread2_time,              //thread 2
        &total_time, &total_ticks, 100
    );

    configRUN_TIME_COUNTER_TYPE sum = thread1_time + thread2_time;
    configRUN_TIME_COUNTER_TYPE thread1_percent = (thread1_time*100)/sum;
    configRUN_TIME_COUNTER_TYPE thread2_percent = (thread2_time*100)/sum;

    printf("task1 %lld task2 %lld sum %lld\n", thread1_percent, thread2_percent, sum);

    TEST_ASSERT_TRUE_MESSAGE(thread1_percent > 90, "High Priority less than than 90 percent\n");
    TEST_ASSERT_TRUE_MESSAGE(thread2_percent < 10, "Low Priority greater than 10 percent\n");
}

void busy_yield_lowFirst(void) 
{    
    uint64_t thread1_time = 0;
    uint64_t thread2_time = 0;
    uint64_t total_time = 0;
    TickType_t total_ticks = 0;
    runtime_analysis(busy_yield,SIDE_TASK_PRIORITY,&thread1_time, //thread 1
        busy_yield,SIDE_TASK_PRIORITY+1,&thread2_time,              //thread 2
        &total_time, &total_ticks,100
    );

    configRUN_TIME_COUNTER_TYPE sum = thread1_time + thread2_time;
    configRUN_TIME_COUNTER_TYPE thread1_percent = (thread1_time*100)/sum;
    configRUN_TIME_COUNTER_TYPE thread2_percent = (thread2_time*100)/sum;

    printf("task1 %lld task2 %lld sum %lld\n", thread1_percent, thread2_percent, sum);

    TEST_ASSERT_TRUE_MESSAGE(thread1_percent < 10, "Low Priority greater than 10 percent\n");
    TEST_ASSERT_TRUE_MESSAGE(thread2_percent > 90, "High Priority less than 90 percent\n");
}

//Main test running thread
void runTestThread(__unused void *args)
{
    sleep_ms(5000);
    while(1) 
    {
        UNITY_BEGIN();
        RUN_TEST(test_priority_inversion);
        RUN_TEST(test_priority_inversion_mutex);
        RUN_TEST(busy_busy_same);
        RUN_TEST(yield_yield_same);
        RUN_TEST(busy_yield_same);
        RUN_TEST(busy_busy_highFirst);
        RUN_TEST(busy_busy_lowFirst);
        RUN_TEST(busy_yield_highFirst);
        RUN_TEST(busy_yield_lowFirst);

        UNITY_END();
        sleep_ms(1000);
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
