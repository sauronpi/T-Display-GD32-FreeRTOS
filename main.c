/*!
    \file    main.c
    \brief   running led

    \version 2019-06-05, V1.0.0, firmware for GD32VF103
    \version 2020-08-04, V1.1.0, firmware for GD32VF103
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

// #include <stdio.h>
// #include <unistd.h>
#include "gd32vf103.h"
// #include "gd32vf103v_eval.h"
#include "systick.h"
// #include "n200_timer.h"
/* FreeRTOS kernel includes. */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtprint.h"

#define debugprint rtprintf
#define DEBUG 0
#define HOOK_DEBUG 0
#define DEBUG_TASK 1

// 系统初始化函数使用标准工具链时没有在main函数前调用,声明后手动调用
extern void _init();
extern void uprintf(char *string);

void TaskCreate(void);
void TaskCreater(void *parameters);
void TaskA(void *parameters);
void TaskB(void *parameters);
void TaskC(void *parameters);
void TaskD(void *parameters);

TaskHandle_t taskCreater = NULL;
TaskHandle_t taskA = NULL;
TaskHandle_t taskB = NULL;
TaskHandle_t taskC = NULL;
TaskHandle_t taskD = NULL;

void IRQConfigure(void)
{
    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL4_PRIO0); // 四位优先级组全配置为lvl
    eclic_global_interrupt_enable(); // 使能全局中断
    // eclic_irq_enable(USART0_IRQn, 1, 0);
}

void UARTConfigure(void)
{
    /* USART0 */
    rcu_periph_clock_enable(RCU_USART0);
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);        // TX
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10); // RX

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
#if DEBUG
    debugprint("UARTConfigure Done\r\n");
#endif
}

void LEDConfigure(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);
    /* LED on pin 1 of GPIOA */
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    gpio_bit_reset(GPIOA, GPIO_PIN_1);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    gpio_bit_reset(GPIOA, GPIO_PIN_2);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_bit_reset(GPIOC, GPIO_PIN_13);
#if DEBUG
    debugprint("LEDConfigure Done\r\n");
#endif
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    size_t size = 0;
    /* systesm init */
    _init();
    IRQConfigure();
    UARTConfigure();
    LEDConfigure();

#if DEBUG
    debugprint("1\r\n");
#endif
    TaskCreate();
    // xTaskCreate(TaskCreater, "TaskCreater", 256, NULL, 2, &taskCreater);
#if DEBUG
    debugprint("2\r\n");
#endif
    vTaskStartScheduler();
#if DEBUG
    while (1)
    {
        debugprint("RTOS Exit\r\n");
    }
#endif
}

/* retarget the C library printf function to the USART */
// int _put_char(int ch)
// {
//     usart_data_transmit(USART0, (uint8_t)ch);
//     while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET);
//     return ch;
// }

void uprintf(char *string)
{
    uint32_t i = 0;
    taskENTER_CRITICAL();
    while (string[i] != '\0')
    {
        usart_data_transmit(USART0, (uint8_t)string[i]);
        while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET)
            ;
        i++;
    }
    taskEXIT_CRITICAL();
}

void TaskCreate(void)
{
    size_t size = 0;
    size = xPortGetFreeHeapSize();
#if DEBUG
    debugprint("tc1 size %d\r\n", size);
#endif
    xTaskCreate(TaskA, "TaskA", 256, NULL, 3, &taskA);
    size = xPortGetFreeHeapSize();
#if DEBUG
    debugprint("tc2 size %d\r\n", size);
#endif
    xTaskCreate(TaskB, "TaskB", 256, NULL, 3, &taskB);
    size = xPortGetFreeHeapSize();
#if DEBUG
    debugprint("tc3 size %d\r\n", size);
#endif
    xTaskCreate(TaskC, "TaskC", 256, NULL, 3, &taskC);
    size = xPortGetFreeHeapSize();
#if DEBUG
    debugprint("tc4 size %d\r\n", size);
#endif
    xTaskCreate(TaskD, "TaskD", 256, NULL, 2, &taskD);
    size = xPortGetFreeHeapSize();
#if DEBUG
    debugprint("tc5 size %d\r\n", size);
#endif
#if DEBUG
    debugprint("A=%x\r\n", taskA);
    debugprint("B=%x\r\n", taskB);
    debugprint("C=%x\r\n", taskC);
    debugprint("D=%x\r\n", taskD);
#endif
}

void TaskCreater(void *parameters)
{
    taskENTER_CRITICAL();
    xTaskCreate(TaskA, "TaskA", 256, NULL, 2, &taskA);
    xTaskCreate(TaskB, "TaskB", 256, NULL, 2, &taskB);
    xTaskCreate(TaskC, "TaskC", 256, NULL, 2, &taskC);
    taskEXIT_CRITICAL();
    vTaskDelete(taskCreater);
}

void TaskA(void *parameters)
{
    size_t size = 0;
    UBaseType_t mark = 0;
    while (1)
    {
#if DEBUG
        taskENTER_CRITICAL();
        debugprint("TaskA\r\n");
        // size = xPortGetFreeHeapSize();
        // debugprint("ta size %d\r\n", size);
        // mark = uxTaskGetStackHighWaterMark(taskA);
        // debugprint("mark %d\r\n", mark);
        taskEXIT_CRITICAL();
#endif
        gpio_bit_write(GPIOA, GPIO_PIN_1, (bit_status)(1 - gpio_input_bit_get(GPIOA, GPIO_PIN_1)));
        vTaskDelay(500);
    }
}

void TaskB(void *parameters)
{
    while (1)
    {
#if DEBUG
        taskENTER_CRITICAL();
        rtprintf("TaskB\r\n");
        taskEXIT_CRITICAL();
#endif
        gpio_bit_write(GPIOA, GPIO_PIN_2, (bit_status)(1 - gpio_input_bit_get(GPIOA, GPIO_PIN_2)));
        vTaskDelay(1000);
    }
}

void TaskC(void *parameters)
{
    while (1)
    {
#if DEBUG
        taskENTER_CRITICAL();
        debugprint("TaskC\r\n");
        taskEXIT_CRITICAL();
#endif
        gpio_bit_write(GPIOC, GPIO_PIN_13, (bit_status)(1 - gpio_input_bit_get(GPIOC, GPIO_PIN_13)));
        vTaskDelay(1500);
    }
}

void TaskD(void *parameters)
{
    size_t size = 0;
    configSTACK_DEPTH_TYPE mark = 0;
    while (1)
    {
#if DEBUG
        taskENTER_CRITICAL();
        debugprint("TaskD\r\n");
        size = xPortGetFreeHeapSize();
        debugprint("FreeHeapSize %d\r\n", size);
        mark = uxTaskGetStackHighWaterMark2(taskD);
        debugprint("StackHighWaterMark %d\r\n", mark);
        taskEXIT_CRITICAL();
#endif
        vTaskDelay(1000);
    }
}

void freertos_risc_v_application_exception_handler(UBaseType_t mcause)
{
    UBaseType_t status = 0;

    status = taskENTER_CRITICAL_FROM_ISR();
    debugprint("exception: 0x%04x\r\n", mcause);
    taskEXIT_CRITICAL_FROM_ISR(status);
    // write(1, "trap\n", 5);
    // printf("In trap handler, the mcause is %d\n", mcause);
    // printf("In trap handler, the mepc is 0x%x\n", read_csr(mepc));
    // printf("In trap handler, the mtval is 0x%x\n", read_csr(mbadaddr));
    // _exit(mcause);
    // return 0;
}

void freertos_risc_v_application_interrupt_handler(UBaseType_t mcause)
{
    UBaseType_t status = 0;

    status = taskENTER_CRITICAL_FROM_ISR();
    debugprint("interrupt: 0x%04x\r\n", mcause);
    taskEXIT_CRITICAL_FROM_ISR(status);
}

void vApplicationTickHook(void)
{
    // rtprintf("Tick\r\n");
}

void vApplicationIdleHook(void)
{
    // rtprintf("Idle\r\n");
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
#if HOOK_DEBUG
    debugprint("task：%s Overflow\r\n", pcTaskName);
#endif
}

void vApplicationMallocFailedHook(void)
{
#if HOOK_DEBUG
    debugprint("MallocFailed\r\n");
#endif
}

void vApplicationDaemonTaskStartupHook(void)
{
#if HOOK_DEBUG
    debugprint("DaemonTask\r\n");
#endif
}