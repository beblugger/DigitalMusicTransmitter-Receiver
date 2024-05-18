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
    volatile uint8_t buffer[BUFFER_SIZE];
    volatile uint32_t *bufferAlias;
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint8_t stopBits;
    volatile uint8_t parityBits;
    volatile uint8_t parity;
} VUARTStreamBuffer;

#define BIT_BAND_BASE 0x22000000
#define SRAM_MASK 0x00ffffff

#define BIT_BAND_ALIAS_SRAM(addr, bit) (volatile uint32_t *)((BIT_BAND_BASE) | ((((volatile uint32_t)(addr)) & SRAM_MASK) << 5) | ((volatile uint32_t)(bit) << 2))

// Function declarations
void VUARTInitTransmitter(volatile VUARTStreamBuffer *tx, volatile uint8_t stopBits, volatile uint8_t parityBits);
void VUARTWriteByteToTransmitter(volatile VUARTStreamBuffer *tx, volatile uint8_t data);
uint8_t VUARTReadBitFromTransmitter(volatile VUARTStreamBuffer *tx);
static volatile uint32_t *GetBitBandAlias(volatile uint32_t address, volatile uint8_t bit);
bool VUARTIsBufferEmpty(volatile uint32_t head, volatile uint32_t tail);
uint32_t VUARTGETBufferRemainingSize(volatile uint32_t head, volatile uint32_t tail);
static uint8_t calculateParity(volatile uint8_t byte);
static void aliasPointerIncrease(volatile uint32_t *pointer);
bool VUARTIsAbleToWriteToBuffer(volatile VUARTStreamBuffer *tx);
void VUARTAddStopBit(volatile VUARTStreamBuffer *tx, volatile uint8_t stopBits);

// 初始化发射器
void VUARTInitTransmitter(volatile VUARTStreamBuffer *tx, volatile uint8_t stopBits, volatile uint8_t parityBits)
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
    tx->bufferAlias = (volatile uint32_t *)GetBitBandAlias((volatile uint32_t)tx->buffer, 0);
}

void VUARTAddStopBit(volatile VUARTStreamBuffer *tx, volatile uint8_t stopBits)
{
    if (VUARTGETBufferRemainingSize(tx->head, tx->tail) < stopBits)
    {
        return;
    }
    for (int i = 0; i < stopBits; i++)
    {
        tx->bufferAlias[tx->tail] = 1;
        aliasPointerIncrease(&tx->tail);
    }
}

// 发射器：向缓冲区写入数据，以字节为单位
void VUARTWriteByteToTransmitter(volatile VUARTStreamBuffer *tx, volatile uint8_t data)
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
        volatile uint8_t parity = calculateParity(data);
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
uint8_t VUARTReadBitFromTransmitter(volatile VUARTStreamBuffer *tx)
{
    volatile uint8_t bit;
    if (VUARTIsBufferEmpty(tx->head, tx->tail))
    {
        // When no signal is transmitting, UART requires a high level signal
        return 1;
    }
    bit = tx->bufferAlias[tx->head];
    aliasPointerIncrease(&tx->head);
    return bit;
}

// 位带别名映射
static volatile uint32_t *GetBitBandAlias(volatile uint32_t address, volatile uint8_t bit)
{
    return BIT_BAND_ALIAS_SRAM(address, bit);
}

// 检查缓冲区是否为空
bool VUARTIsBufferEmpty(volatile uint32_t head, volatile uint32_t tail)
{
    return head == tail;
}

// 获取缓冲区剩余大小, 以bit为单位
uint32_t VUARTGETBufferRemainingSize(volatile uint32_t head, volatile uint32_t tail)
{
    if (head <= tail)
    {
        return (BUFFER_ALIAS_SIZE - (tail - head)) - 1;
    }
    return (head - tail) - 1;
}

bool VUARTIsAbleToWriteToBuffer(volatile VUARTStreamBuffer *tx)
{
    return VUARTGETBufferRemainingSize(tx->head, tx->tail) >= 9 + tx->stopBits + tx->parityBits;
}

// 计算奇偶校验位
static uint8_t calculateParity(volatile uint8_t byte)
{
    volatile uint8_t parity = 0;
    while (byte)
    {
        parity ^= (byte & 1);
        byte >>= 1;
    }
    return parity;
}

static void aliasPointerIncrease(volatile uint32_t *pointerToAliasVirtualPointer)
{
    (*pointerToAliasVirtualPointer)++;
    (*pointerToAliasVirtualPointer) &= BUFFER_ALIAS_SIZE_MASK;
}
