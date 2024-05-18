#ifndef INITIALIZATION_H
#define INITIALIZATION_H

#include <stdint.h>

void InitGPIO(void);
void InitSysTick(uint32_t SystemClkFrequency, uint32_t frequency);
void InitUART(uint32_t SystemClkFrequency, uint32_t UARTMBaseFreq);

#endif /* INITIALIZATION_H */
