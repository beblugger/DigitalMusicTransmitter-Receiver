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

#include "initialization.h"
#include "note.h"
#include "ASynthesis.h"
#include "scoreBuf.h"
#include "scoreRec.h"
#include "UARTui.h"

void SysTick_Handler(void);
void updateK0State(void);
void updateP0State(void);
void updateF3State(void);
void replay(scoreBuffer *LocalBuffer, playerState *Localplayer, scoreRecorder *LocalRec);