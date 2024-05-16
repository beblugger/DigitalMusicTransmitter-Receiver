#define PART_TM4C1294NCPDT

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"     // 基址宏定义
#include "inc/hw_types.h"      // 数据类型宏定义，寄存器访问函数
#include "driverlib/gpio.h"    // 通用IO口宏定义
#include "driverlib/pin_map.h" // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"  // 系统控制宏定义
#include "driverlib/uart.h"    //UART相关宏定义
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#define UARTMBaseFreq 5000
#define SysTickResloution 120

static uint32_t SystemClkFrequency = 0;

static volatile int16_t activatedLength = 0;
static volatile uint16_t counterSinceLastUpdateK0 = 0;

static volatile uint16_t SysTickP0PulseWidth = 0;
static volatile bool GPIOK1State = 0, GPIOK1Backup = 0, GPIOP0State = 0;

void InitGPIO(void);
void InitSysTick(uint32_t frequency);
void InitUART(void);
void SysTick_Handler(void);

static uint8_t WhatRxReceiverd[1024];
static uint16_t WhatRxReceiverdIndex = 0;

int main(void)
{
    InitGPIO();
    SystemClkFrequency = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                            120000000);
    InitUART();
    InitSysTick(UARTMBaseFreq * SysTickResloution);

    uint8_t counter = 0;
    while (true)
    {
        while (UARTCharsAvail(UART2_BASE))
        {
            WhatRxReceiverd[WhatRxReceiverdIndex] = UARTCharGet(UART2_BASE);
            WhatRxReceiverdIndex++;
        }
        if (!UARTBusy(UART2_BASE))
        {
            UARTCharPut(UART2_BASE, counter);
            counter++;
            UARTCharPut(UART2_BASE, counter);
            counter++;
        }
    }
    return 0;
}

void SysTick_Handler(void)
{
    // convert UART -> UARTM, K1 -> P0
    uint16_t SysTickP0PulseWidthMax;
    GPIOK1Backup = GPIOK1State;
    GPIOK1State = GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_1);
    if (GPIOK1Backup != GPIOK1State)
    {
        // Sync with edges of K1
        if (GPIOP0State && SysTickP0PulseWidth > 4)
        {
            SysTickP0PulseWidthMax = ((GPIOK1State ^ GPIOP0State) ? SysTickResloution / 3 : SysTickResloution * 2 / 3);
            SysTickP0PulseWidth = 0;
        }
        else
        {
            SysTickP0PulseWidthMax = SysTickResloution;
        }
    }
    SysTickP0PulseWidth++;
    if (SysTickP0PulseWidth >= SysTickP0PulseWidthMax)
    {
        SysTickP0PulseWidth = 0;
        GPIOP0State = !GPIOP0State;
        GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_0, GPIOP0State ? GPIO_PIN_0 : 0);
    }

    // convert UARTM -> UART, P1 -> K0

    // After waiting too long for a bit, put 0 to K0
    counterSinceLastUpdateK0++;
    if (counterSinceLastUpdateK0 > SysTickResloution)
    {
        counterSinceLastUpdateK0 = 0;
        activatedLength = 0;
        GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, 0);
    }

    // update K0 if a bit is received
    if (GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_1) == 0)
    {
        activatedLength++;
    }
    else
    {
        if (activatedLength > SysTickResloution / 2)
        {
            GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, 0);
        }
        else if (activatedLength > SysTickResloution / 6)
        {
            GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, GPIO_PIN_0);
        }
        activatedLength -= 2;
        if (activatedLength < 0)
        {
            activatedLength = 0;
        }
    }
}

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

void InitSysTick(uint32_t frequency)
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

void InitUART(void)
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
    // Enable the UART interrupt.
    // IntEnable(INT_UART0);
    // UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}
