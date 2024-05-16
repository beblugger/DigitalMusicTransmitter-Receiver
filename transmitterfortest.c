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

// #include "customizedUARTIO.h"

static uint16_t i = 0;
static uint32_t SystemClkFrequency = 0;

void InitGPIO();
void InitSysTick(int frequency);
void InitUART();
void SysTick_Handler(void);

// VUARTTransmitter mainTransmitter;
// volatile

int main(void)
{
    InitGPIO();
    SystemClkFrequency = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                            120000000);
    InitSysTick(6000);
    VUARTInitTransmitter(&mainTransmitter, 1, 1);

    while (true)
    {
        if (VUARTIsAbleToWriteToBuffer(&mainTransmitter))
        {
            VUARTWriteToBuffer(&mainTransmitter, i);
            i++;
            i &= 0xff;
        }
    }
    return 0;
}

void SysTick_Handler(void)
{
    if (VUARTIsBufferEmpty(mainTransmitter.head, mainTransmitter.tail))
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
        VUARTReadFromBuffer(&mainTransmitter);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    }
}

void InitGPIO()
{
    // Enable the GPIO port that is used for the on-board buzzer.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Check if the peripheral access is enabled.
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
        ;

    // Enable the GPIO pin for the buzzer (PF3).  Set the direction as output, and
    // enable the GPIO pin for digital function.
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
}

void InitSysTick(int frequency)
{
    // Set up the period for the SysTick timer.  The SysTick timer period will
    // be equal to the system clock / frequency.
    SysTickPeriodSet(SysCtlClockGet() / frequency);

    // Enable SysTick.
    SysTickEnable();
    // Enable the SysTick Interrupt.
    SysTickIntEnable();
}

void InitUART()
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

    // Enable the UART interrupt.
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}