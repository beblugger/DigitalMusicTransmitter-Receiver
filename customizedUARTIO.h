// The bit-band alias is only supported in ARM Cortex-M3/M4, and when buffer is in bit-band alias
// The buffer size should be 2^n

#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 16
#define BUFFER_ALIAS_SIZE (BUFFER_SIZE * 8)
#define BUFFER_ALIAS_SIZE_MASK (BUFFER_ALIAS_SIZE - 1)

// VUART 状态标志
#define STATE_WAITING_START 0b10000000
#define STATE_READING_DATA 0b01000000
#define STATE_READING_PARITY 0b00100000
#define STATE_READING_STOP 0b00010000
#define STATE_ERROR_PARITY 0b00001000
#define STATE_CONNECTION_LOST 0b00000000

// 干扰很大，必须要连续收到32个1才能认定发射设备存在
// 另外，连续4次奇偶校验失败或者连续32位检测不到起始位或者停止位，就认为通信中断

// Only 1 start bit is supported
typedef struct
{
    uint8_t buffer[BUFFER_SIZE];
    uint32_t *bufferAlias;
    uint32_t head;
    uint32_t tail;
    //     uint8_t state;
    //     // state:
    //     // 0b10000xxx: 通信正常，等待起始位
    //     // 0b01000xxx: 通信正常，已读入起始位x个，等待数据位 Unused
    //     // 0b00100xxx: 通信正常，已读入数据位x个，等待校验位
    //     // 0b00010xxx: 通信正常，已读入校验位x个，等待停止位
    //     // 0b00001xxx: 通信正常，已读入停止位x个，等待下一个起始位
    //     // 0b00000100: 非致命错误，奇偶校验失败 Unused
    //     // 0b00000000: 通信中断
    //     int8_t connectionHealth; // 通信健康状态,[0,31], Only at 31, the connection will be enabled
    // #define CONNECTION_HEALTH_THRESHOLD 31
    //     uint8_t errorLevel;
    uint8_t stopBits;
    uint8_t parityBits;
    uint8_t parity;
} VUARTStreamBuffer;

#define BIT_BAND_BASE 0x22000000
#define SRAM_MASK 0x00ffffff

#define BIT_BAND_ALIAS_SRAM(addr, bit) (uint32_t *)((BIT_BAND_BASE) | ((((uint32_t)(addr)) & SRAM_MASK) << 5) | ((uint32_t)(bit) << 2))
#define GET_BIT_BAND_ADDR_FROM_ALIAS(alias) (uint32_t *)(((uint32_t)(alias) & SRAM_MASK) >> 5)

// Function declarations
void VUARTInitTransmitter(VUARTStreamBuffer *tx, uint8_t stopBits, uint8_t parityBits);
// void VUARTInitReceiver(VUARTStreamBuffer *rx, uint8_t stopBits, uint8_t parityBits);
void VUARTWriteByteToTransmitter(VUARTStreamBuffer *tx, uint8_t data);
uint8_t VUARTReadBitFromTransmitter(VUARTStreamBuffer *tx);
// uint8_t VUARTReadByteFromReceiver(VUARTStreamBuffer *rx);
// void VUARTWriteBitToReceiver(VUARTStreamBuffer *rx, int bit);
static uint32_t *GetBitBandAlias(uint32_t address, uint8_t bit);
static uint32_t *GetAddressFromBitBandAlias(uint32_t *alias);
bool VUARTIsBufferFull(uint32_t head, uint32_t tail);
bool VUARTIsBufferEmpty(uint32_t head, uint32_t tail);
uint32_t VUARTGETBufferRemainingSize(uint32_t head, uint32_t tail);
static uint8_t calculateParity(uint8_t byte);
static void aliasPointerIncrease(uint32_t *pointer);
static void aliasPointerDecrease(uint32_t *pointer);
bool VUARTIsAbleToWriteToBuffer(VUARTStreamBuffer *tx);
//  bool VUARTIsAbleToReadByteFromBuffer(VUARTStreamBuffer *rx);

// 初始化发射器
void VUARTInitTransmitter(VUARTStreamBuffer *tx, uint8_t stopBits, uint8_t parityBits)
{
    if (parityBits > 1 || stopBits < 1 || stopBits >= 8)
    {
        return;
    }
    tx->head = 0;
    tx->tail = 8;
    tx->stopBits = stopBits;
    tx->parityBits = parityBits;
    tx->buffer[0] = 0xff;
    tx->bufferAlias = (uint32_t *)GetBitBandAlias((uint32_t)tx->buffer, 0);
}

// // 初始化接收器
// void VUARTInitReceiver(VUARTStreamBuffer *rx, uint8_t stopBits, uint8_t parityBits)
// {
//     if (parityBits > 1 || stopBits < 1 || stopBits >= 8)
//     {
//         return;
//     }
//     rx->head = 0;
//     rx->tail = 0;
//     rx->stopBits = stopBits;
//     rx->parityBits = parityBits;
//     rx->parity = 0;
//     rx->state = 0b00000000;
//     rx->connectionHealth = 0;
//     rx->bufferAlias = (uint32_t *)GetBitBandAlias((uint32_t)rx->buffer, 0);
// }

// 发射器：向缓冲区写入数据，以字节为单位
void VUARTWriteByteToTransmitter(VUARTStreamBuffer *tx, uint8_t data)
{
    if (VUARTGETBufferRemainingSize(tx->head, tx->tail) < 9 + tx->stopBits + tx->parityBits)
    {
        // 处理缓冲区满的情况
        return;
    }

    // 添加起始位
    tx->bufferAlias[tx->tail] = 0; // 起始位为0
    aliasPointerIncrease(&tx->tail);

    // 添加数据位
    for (int i = 0; i < 8; i++)
    {
        tx->bufferAlias[tx->tail] = (data >> i) & 0x01;
        aliasPointerIncrease(&tx->tail);
    }

    // 添加校验位
    if (tx->parityBits > 0)
    {
        uint8_t parity = calculateParity(data);
        tx->bufferAlias[tx->tail] = parity;
        aliasPointerIncrease(&tx->tail);
    }

    // 添加停止位
    for (int i = 0; i < tx->stopBits; i++)
    {
        tx->bufferAlias[tx->tail] = 1; // 停止位为1
        aliasPointerIncrease(&tx->tail);
    }
}

// 发射器：从缓冲区读出数据，以比特为单位
uint8_t VUARTReadBitFromTransmitter(VUARTStreamBuffer *tx)
{
    if (VUARTIsBufferEmpty(tx->head, tx->tail))
    {
        // When no signal is transmitting, UART requires a high level signal
        return 1;
    }
    uint8_t bit = tx->bufferAlias[tx->tail];
    aliasPointerIncrease(&tx->tail);
    return bit;
}

// 接收器：从缓冲区读出数据，以字节为单位
// uint8_t VUARTReadByteFromReceiver(VUARTStreamBuffer *rx)
// {
//     static uint32_t cursor = 0;
//     cursor = rx->tail;
//     if (!VUARTIsAbleToReadByteFromBuffer(rx))
//     {
//         // 处理缓冲区空的情况
//         return 0;
//     }
//     /*// errorLevel：Add all the errors in a frame, and it is processed and cleared after a frame

//     if (rx->state & 0b10000000)
//     {
//         // 通信正常，等待起始位

//         // Handle the error in the previous frame
//         if (rx->errorLevel > 0)
//         {
//             // Discard the frame if there are errors
//             rx->tail = (rx->tail - 8) & BUFFER_ALIAS_SIZE_MASK;
//             if (rx->errorLevel >= rx->connectionHealth)
//             {
//                 // If the accumulated error is too large, the connection is considered to be interrupted
//                 rx->connectionHealth = 0;
//                 rx->state = 0b00000000;
//             }
//             else
//             {
//                 rx->connectionHealth -= rx->errorLevel;
//             }
//             rx->errorLevel = 0;
//         }
//         else if (rx->connectionHealth != CONNECTION_HEALTH_THRESHOLD)
//         {
//             rx->connectionHealth++;
//         }

//         if (bit == 0)
//         {
//             // Start bit detected, move to the next state
//             rx->state = 0b00100000;
//         }
//     }
//     // else if (rx->state & 0b01000000)
//     // {
//     //     // 通信正常，已读入起始位x个，等待数据位 Unused
//     //     rx->state = 0b00100000;
//     // }
//     else if (rx->state & 0b00100000)
//     {
//         // 通信正常，已读入数据位
//         rx->bufferAlias[rx->head] = bit;
//         aliasPointerIncrease(&rx->head);

//         // 检查是否已读入8位数据
//         if ((rx->state & 0b00000111) == 7)
//         {
//             rx->parity = calculateParity(rx->buffer[rx->head >> 3]);
//             rx->state = 0b00010000; // 进入校验位状态
//         }
//         else
//         {
//             rx->state++;
//         }
//     }
//     else if (rx->state & 0b00010000)
//     {
//         // 通信正常，已读入校验位
//         if (bit != rx->parity)
//         {
//             rx->errorLevel += 8;
//         }
//         rx->state = 0b00001000; // 进入停止位状态
//     }
//     else if (rx->state & 0b00001000)
//     {
//         // 通信正常，已读入停止位
//         if (bit != 1)
//         {
//             rx->errorLevel++;
//         }

//         // 检查是否已读入所有停止位
//         if ((rx->state & 0b00000111) == (rx->stopBits - 1))
//         {
//             rx->state = 0b10000000; // 重新进入等待起始位状态
//         }
//         else
//         {
//             rx->state++;
//         }
//     }
//     else if (rx->state == 0b00000000)
//     {
//         // 通信中断
//         if (bit == 1)
//         {
//             rx->connectionHealth++;
//             if (rx->connectionHealth >= CONNECTION_HEALTH_THRESHOLD)
//             {
//                 rx->state = 0b10000000;
//             }
//         }
//         else
//         {
//             rx->connectionHealth = 0;
//         }
//     }
//     else
//     {
//         // Invaild state
//         // todo: Error handling
//     }*/
//     if (VUARTGETBufferRemainingSize(rx->head, rx->tail) < 8)
//     {
//         // 处理缓冲区空的情况
//         return 0;
//     }
//     uint8_t data = rx->buffer[(rx->head) >> 3];
//     rx->tail = (rx->tail + 8) & BUFFER_ALIAS_SIZE_MASK;
//     return data;
// }

// 接收器：向缓冲区写入数据，以比特为单位，自行检查是否有合法的信号和合法的数据
// void VUARTWriteBitToReceiver(VUARTStreamBuffer *rx, int bit)
// {
//     if (VUARTIsBufferFull(rx->head, rx->tail))
//     {
//         // Handle the buffer full condition
//         return;
//     }
//     rx->bufferAlias[rx->head] = bit;
//     aliasPointerIncrease(&rx->head);
// }

// 位带别名映射
static uint32_t *GetBitBandAlias(uint32_t address, uint8_t bit)
{
    return BIT_BAND_ALIAS_SRAM(address, bit);
}

static uint32_t *GetAddressFromBitBandAlias(uint32_t *alias)
{
    return (uint32_t *)GET_BIT_BAND_ADDR_FROM_ALIAS(alias);
}

// 检查缓冲区是否为满
bool VUARTIsBufferFull(uint32_t head, uint32_t tail)
{
    return ((head + 1) & BUFFER_ALIAS_SIZE_MASK) == tail;
}

// 检查缓冲区是否为空
bool VUARTIsBufferEmpty(uint32_t head, uint32_t tail)
{
    return head == tail;
}

// 获取缓冲区剩余大小, 以bit为单位
uint32_t VUARTGETBufferRemainingSize(uint32_t head, uint32_t tail)
{
    if (head >= tail)
    {
        return (tail - head) - 1;
    }
    return (BUFFER_ALIAS_SIZE - (tail - head)) - 1;
}

bool VUARTIsAbleToWriteToBuffer(VUARTStreamBuffer *tx)
{
    return VUARTGETBufferRemainingSize(tx->head, tx->tail) >= 9 + tx->stopBits + tx->parityBits;
}

//  bool VUARTIsAbleToReadByteFromBuffer(VUARTStreamBuffer *rx)
// {
//     int8_t remainingSize = VUARTGETBufferRemainingSize(rx->head, rx->tail);
//     // Condition: There's another Byte be wtritten/writting to the buffer, or current Byte is complete and no error
//     return (remainingSize > 8) || (remainingSize == 8 && (rx->state & 0b10000000) && rx->errorLevel == 0);
// }

// 计算奇偶校验位
static uint8_t calculateParity(uint8_t byte)
{
    uint8_t parity = 0;
    while (byte)
    {
        parity ^= (byte & 1);
        byte >>= 1;
    }
    return parity;
}

static void aliasPointerIncrease(uint32_t *pointerToAliasVirtualPointer)
{
    (*pointerToAliasVirtualPointer)++;
    (*pointerToAliasVirtualPointer) &= BUFFER_ALIAS_SIZE_MASK;
}

static void aliasPointerDecrease(uint32_t *pointerToAliasVirtualPointer)
{
    (*pointerToAliasVirtualPointer)--;
    (*pointerToAliasVirtualPointer) &= BUFFER_ALIAS_SIZE_MASK;
}
