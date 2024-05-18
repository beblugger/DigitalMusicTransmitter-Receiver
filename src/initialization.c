#ifndef PART_TM4C1294NCPDT
#define PART_TM4C1294NCPDT
#endif

#include "initialization.h"

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h" // 基址宏定义
#include "inc/hw_ints.h"

#include "driverlib/pin_map.h" // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"  // 系统控制宏定义
#include "driverlib/gpio.h"    // 通用IO口宏定义
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h" //UART相关宏定义

void InitGPIO(void)
{
    // Enable the GPIO port that is used for the on-board buzzer.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Check if the peripheral access is enabled.
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
        ;

    // Enable the GPIO pin for the buzzer (PF3).  Set the direction as output, and
    // enable the GPIO pin for digital function.
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
        ;

    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK))
        ;
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_0); // Physically linked to UART Rx, for converted UARTM output
    GPIOPinTypeGPIOInput(GPIO_PORTK_BASE, GPIO_PIN_1);  // Physically linked to UART Tx, for converted UARTM input

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP))
        ;

    GPIOPinTypeGPIOOutput(GPIO_PORTP_BASE, GPIO_PIN_0); // Physically linked to P1, for UARTM output
    GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_1);  // Physically linked to P0, for UARTM input
}

void InitSysTick(uint32_t SystemClkFrequency, uint32_t frequency)
{
    // Set up the period for the SysTick timer.  The SysTick timer period will
    // be equal to the system clock / frequency.

    // SysCtlClockGet is for TM4C123X only, it's not available for TM4C129X
    SysTickPeriodSet(SystemClkFrequency / frequency);

    // Enable SysTick.
    SysTickEnable();
    // Enable the SysTick Interrupt.
    SysTickIntEnable();
}

void InitUART(uint32_t SystemClkFrequency, uint32_t UARTMBaseFreq)
{
    // Enable the GPIO port that is used for the on-board UART.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Check if the peripheral access is enabled.
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
        ;

    GPIOPinConfigure(GPIO_PA0_U0RX); // 设置PA0为UART0 RX引脚
    GPIOPinConfigure(GPIO_PA1_U0TX); // 设置PA1为UART0 TX引脚

    // Enable the GPIO pin for the UART0 (PA0 & PA1).  Set the direction as input
    // and enable the GPIO pin for digital function.
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Enable the UART0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Check if the peripheral access is enabled.
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
        ;

    // Configure the UART for 115,200, 8-N-1 operation.
    UARTConfigSetExpClk(UART0_BASE, SystemClkFrequency, 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    // Check if the peripheral access is enabled
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
        ;

    GPIOPinConfigure(GPIO_PD4_U2RX);
    GPIOPinConfigure(GPIO_PD5_U2TX);

    GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART2))
        ;

    UARTConfigSetExpClk(UART2_BASE, SystemClkFrequency, UARTMBaseFreq,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_EVEN));
}
