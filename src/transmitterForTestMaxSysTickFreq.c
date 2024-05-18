#ifndef PART_TM4C1294NCPDT
#define PART_TM4C1294NCPDT
#endif

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"  // 基址宏定义
#include "inc/hw_types.h"   // 数据类型宏定义，寄存器访问函数
#include "driverlib/gpio.h" // 通用IO口宏定义
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h" // 系统控制宏定义
#include "driverlib/uart.h"

#include "customizedUARTIO.h"
#include "initialization.h"

#define UARTMBaseFreq 2500
#define SysTickResloution 120

static uint32_t SystemClkFrequency = 0;

volatile static VUARTStreamBuffer txBuffer;

void SysTick_Handler(void);
void updateK0State(void);
void updateP0State(void);

static uint8_t WhatRxReceiverd[64];
static uint16_t WhatRxReceiverdIndex = 0;
static uint8_t counter = 0;

static uint32_t debugpui32Baud, debugpui32Config;

int main(void)
{
    // initalize
    InitGPIO();
    SystemClkFrequency = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                            120000000);
    VUARTInitTransmitter(&txBuffer, 1, 1);
    InitSysTick(SystemClkFrequency, UARTMBaseFreq * SysTickResloution);
    InitUART(SystemClkFrequency, UARTMBaseFreq);
    UARTConfigGetExpClk(UART2_BASE, SystemClkFrequency, &debugpui32Baud, &debugpui32Config);

    while (true)
    {
        while (UARTCharsAvail(UART2_BASE) && WhatRxReceiverdIndex < 64)
        {
            WhatRxReceiverd[WhatRxReceiverdIndex] = UARTCharGet(UART2_BASE);
            WhatRxReceiverdIndex++;
        }
        // if (!UARTBusy(UART2_BASE))
        // {
        //     UARTCharPut(UART2_BASE, 0x37);
        //     counter++;
        // }
        if (VUARTGetBufferRemainingSize(txBuffer.head, txBuffer.tail) > 35)
        {
            VUARTWriteByteToTransmitter(&txBuffer, counter);
            // VUARTAddStopBit(&txBuffer, 16);
            counter++;
        }
    }
    return 0;
}

// static volatile uint16_t debugCounter = 0;

void SysTick_Handler(void)
{
    updateP0State();
    updateK0State();
}

void updateP0State(void)
{
    static volatile uint16_t SysTickPhaseCounter = 0;
    static volatile bool bitWritingtoP0 = 1;
    // convert VUART -> UARTM, txBuffer -> P0

    if (SysTickPhaseCounter >= SysTickResloution)
    {
        SysTickPhaseCounter = 0;
        // debugCounter++;
        bitWritingtoP0 = VUARTReadBitFromTransmitter(&txBuffer);
        GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_0, GPIO_PIN_0);
    }
    else if (SysTickPhaseCounter == SysTickResloution * 2 / 3)
    {
        if (bitWritingtoP0)
        {
            GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_0, 0);
        }
    }
    else if (SysTickPhaseCounter == SysTickResloution / 3)
    {
        if (!bitWritingtoP0)
        {
            GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_0, 0);
        }
    }
    SysTickPhaseCounter++;
}

void updateK0State(void)
{
    static volatile int16_t activatedLength = 0;
    static volatile uint16_t counterSinceLastUpdateK0 = 0;
    // convert UARTM -> UART, P1 -> K0

    // After waiting too long for a bit, put 0 to K0
    counterSinceLastUpdateK0++;
    if (counterSinceLastUpdateK0 > SysTickResloution * 2)
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
            counterSinceLastUpdateK0 = 0;
            activatedLength = 0;
            GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, 0);
        }
        else if (activatedLength > SysTickResloution / 6)
        {
            counterSinceLastUpdateK0 = 0;
            activatedLength = 0;
            GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, GPIO_PIN_0);
        }
        activatedLength--;
        if (activatedLength < 0)
        {
            activatedLength = 0;
        }
    }
}