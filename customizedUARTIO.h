// The bit-band alias is only supported in ARM Cortex-M3/M4, and when buffer is in bit-band alias
// The buffer size should be 2^n

#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 16
#define BUFFER_ALIAS_SIZE (BUFFER_SIZE * 8)
#define BUFFER_ALIAS_SIZE_MASK (BUFFER_ALIAS_SIZE - 1)

// 干扰很大，必须要连续收到32个1才能认定发射设备存在
// 另外，连续4次奇偶校验失败或者连续32位检测不到起始位或者停止位，就认为通信中断

// Only 1 start bit is supported
typedef struct
{
    uint8_t buffer[BUFFER_SIZE];
    uint32_t *bufferAlias;
    uint32_t head; // bufferAlias的头指针
    uint32_t tail; // bufferAlias的尾指针
    uint8_t stopBits;
    uint8_t parityBits;
} virtualUART_Transmitter;

typedef struct
{
    uint8_t buffer[BUFFER_SIZE];
    uint8_t state;
    // state:
    // 0b10000xxx: 通信正常，等待起始位
    // 0b01000xxx: 通信正常，已读入起始位x个，等待数据位 Unused
    // 0b00100xxx: 通信正常，已读入数据位x个，等待校验位
    // 0b00010xxx: 通信正常，已读入校验位x个，等待停止位
    // 0b00001xxx: 通信正常，已读入停止位x个，等待下一个起始位
    // 0b00000100: 非致命错误，奇偶校验失败 Unused
    // 0b00000000: 通信中断
    int8_t connectionHealth; // 通信健康状态,[0,31], Only at 31, the connection will be enabled
#define CONNECTION_HEALTH_THRESHOLD 31
    uint8_t errorLevel;
    uint32_t *bufferAlias;
    uint32_t head;
    uint32_t tail;
    uint8_t stopBits;
    uint8_t parityBits;
    uint8_t parity = 0;
} virtualUART_Receiver;

#define BIT_BAND_BASE 0x22000000
#define SRAM_MASK 0x00ffffff

#define BIT_BAND_ALIAS_SRAM(addr, bit) (uint32_t *)((BIT_BAND_BASE) | ((((uint32_t)(addr)) & SRAM_MASK) << 5) | ((uint32_t)(bit) << 2))
#define GET_BIT_BAND_ADDR_FROM_ALIAS(alias) (uint32_t *)(((uint32_t)(alias) & SRAM_MASK) >> 5)

// Function declarations
void virtualUART_InitTransmitter(virtualUART_Transmitter *tx, uint8_t stopBits, uint8_t parityBits);
void virtualUART_InitReceiver(virtualUART_Receiver *rx, uint8_t stopBits, uint8_t parityBits);
void virtualUART_WriteByteToTransmitter(virtualUART_Transmitter *tx, uint8_t data);
uint8_t virtualUART_ReadBitFromTransmitter(virtualUART_Transmitter *tx);
uint8_t virtualUART_ReadByteFromReceiver(virtualUART_Receiver *rx);
void virtualUART_WriteBitToReceiver(virtualUART_Receiver *rx, int bit);
static uint32_t *GetBitBandAlias(uint32_t address, uint8_t bit);
static uint32_t *GetAddressFromBitBandAlias(uint32_t *alias);
inline bool virtualUART_IsBufferFull(int head, int tail);
inline bool virtualUART_IsBufferEmpty(int head, int tail);
uint32_t virtualUART_GETBufferRemainingSize(int head, int tail);
static uint8_t calculateParity(uint8_t byte);
static inline void aliasPointerIncrease(uint32_t *pointer);
static inline void aliasPointerDecrease(uint32_t *pointer);
inline bool virtualUART_IsAbleToWriteToBuffer(virtualUART_Transmitter *tx);
inline bool virtualUART_IsAbleToReadFromBuffer(virtualUART_Receiver *rx);

// 初始化发射器
void virtualUART_InitTransmitter(virtualUART_Transmitter *tx, uint8_t stopBits, uint8_t parityBits)
{
    if (parityBits > 1 || stopBits < 1 || stopBits >= 8)
    {
        return;
    }
    tx->head = 0;
    tx->tail = 0;
    tx->stopBits = stopBits;
    tx->parityBits = parityBits;
    tx->bufferAlias = (uint32_t *)GetBitBandAlias((uint32_t)tx->buffer, 0);
}

// 初始化接收器
void virtualUART_InitReceiver(virtualUART_Receiver *rx, uint8_t stopBits, uint8_t parityBits)
{
    if (parityBits > 1 || stopBits < 1 || stopBits >= 8)
    {
        return;
    }
    rx->head = 0;
    rx->tail = 0;
    rx->stopBits = stopBits;
    rx->parityBits = parityBits;
    rx->parity = 0;
    rx->state = 0b00000000;
    rx->connectionHealth = 0;
    rx->bufferAlias = (uint32_t *)GetBitBandAlias((uint32_t)rx->buffer, 0);
}

// 发射器：向缓冲区写入数据，以字节为单位
void virtualUART_WriteByteToTransmitter(virtualUART_Transmitter *tx, uint8_t data)
{
    if (virtualUART_GETBufferRemainingSize(tx->head, tx->tail) < 9 + tx->stopBits + tx->parityBits)
    {
        // 处理缓冲区满的情况
        return;
    }

    // 添加起始位
    tx->bufferAlias[tx->head] = 0; // 起始位为0
    aliasPointerIncrease(&tx->head);

    // 添加数据位
    for (int i = 0; i < 8; i++)
    {
        tx->bufferAlias[tx->head] = (data >> i) & 0x01;
        aliasPointerIncrease(&tx->head);
    }

    // 添加校验位
    if (tx->parityBits > 0)
    {
        uint8_t parity = calculateParity(data);
        tx->bufferAlias[tx->head] = parity;
        aliasPointerIncrease(&tx->head);
    }

    // 添加停止位
    for (int i = 0; i < tx->stopBits; i++)
    {
        tx->bufferAlias[tx->head] = 1; // 停止位为1
        aliasPointerIncrease(&tx->head);
    }
}

// 发射器：从缓冲区读出数据，以比特为单位
uint8_t virtualUART_ReadBitFromTransmitter(virtualUART_Transmitter *tx)
{
    if (virtualUART_IsBufferEmpty(tx->head, tx->tail))
    {
        // When no signal is transmitting, UART requires a high level signal
        return 1;
    }
    uint8_t bit = tx->bufferAlias[tx->tail];
    aliasPointerIncrease(&tx->tail);
    return bit;
}

// 接收器：从缓冲区读出数据，以字节为单位
uint8_t virtualUART_ReadByteFromReceiver(virtualUART_Receiver *rx)
{
    if (virtualUART_GETBufferRemainingSize(rx->head, rx->tail) < 8)
    {
        // 处理缓冲区空的情况
        return 0;
    }
    uint8_t data = rx->buffer[(rx->head) >> 3];
    rx->tail = (rx->tail + 8) & BUFFER_ALIAS_SIZE_MASK;
    return data;
}

// 接收器：向缓冲区写入数据，以比特为单位，自行检查是否有合法的信号和合法的数据
/**
 * @brief Writes a bit to the UART receiver.
 *
 * This function is responsible for handling the reception of bits in the UART receiver.
 * It updates the receiver's state based on the received bit and performs error checking.
 *
 * @param rx A pointer to the virtualUART_Receiver structure representing the receiver.
 * @param bit The bit to be written to the receiver.
 */
void virtualUART_WriteBitToReceiver(virtualUART_Receiver *rx, int bit)
{
    // errorLevel：Add all the errors in a frame, and it is processed and cleared after a frame

    if (rx->state & 0b10000000)
    {
        // 通信正常，等待起始位

        // Handle the error in the previous frame
        if (rx->errorLevel > 0)
        {
            // Discard the frame if there are errors
            rx->tail = (rx->tail - 8) & BUFFER_ALIAS_SIZE_MASK;
            if (rx->errorLevel >= rx->connectionHealth)
            {
                // If the accumulated error is too large, the connection is considered to be interrupted
                rx->connectionHealth = 0;
                rx->state = 0b00000000;
            }
            else
            {
                rx->connectionHealth -= rx->errorLevel;
            }
            rx->errorLevel = 0;
        }
        else if (rx->connectionHealth != CONNECTION_HEALTH_THRESHOLD)
        {
            rx->connectionHealth++;
        }

        if (bit == 0)
        {
            // Start bit detected, move to the next state
            rx->state = 0b00100000;
        }
    }
    // else if (rx->state & 0b01000000)
    // {
    //     // 通信正常，已读入起始位x个，等待数据位 Unused
    //     rx->state = 0b00100000;
    // }
    else if (rx->state & 0b00100000)
    {
        // 通信正常，已读入数据位
        rx->bufferAlias[rx->head] = bit;
        aliasPointerIncrease(&rx->head);

        // 检查是否已读入8位数据
        if ((rx->state & 0b00000111) == 7)
        {
            rx->parity = calculateParity(rx->buffer[rx->head >> 3]);
            rx->state = 0b00010000; // 进入校验位状态
        }
        else
        {
            rx->state++;
        }
    }
    else if (rx->state & 0b00010000)
    {
        // 通信正常，已读入校验位
        if (bit != rx->parity)
        {
            rx->errorLevel += 8;
        }
        rx->state = 0b00001000; // 进入停止位状态
    }
    else if (rx->state & 0b00001000)
    {
        // 通信正常，已读入停止位
        if (bit != 1)
        {
            rx->errorLevel++;
        }

        // 检查是否已读入所有停止位
        if ((rx->state & 0b00000111) == (rx->stopBits - 1))
        {
            rx->state = 0b10000000; // 重新进入等待起始位状态
        }
        else
        {
            rx->state++;
        }
    }
    else if (rx->state == 0b00000000)
    {
        // 通信中断
        if (bit == 1)
        {
            rx->connectionHealth++;
            if (rx->connectionHealth >= CONNECTION_HEALTH_THRESHOLD)
            {
                rx->state = 0b10000000;
            }
        }
        else
        {
            rx->connectionHealth = 0;
        }
    }
    else
    {
        // Invaild state
        // todo: Error handling
    }
}

// 位带别名映射
static inline uint32_t *GetBitBandAlias(uint32_t address, uint8_t bit)
{
    return BIT_BAND_ALIAS_SRAM(address, bit);
}

static inline uint32_t *GetAddressFromBitBandAlias(uint32_t *alias)
{
    return (uint32_t *)GET_BIT_BAND_ADDR_FROM_ALIAS(alias);
}

// 检查缓冲区是否为满
inline bool virtualUART_IsBufferFull(int head, int tail)
{
    return ((head + 1) & BUFFER_ALIAS_SIZE_MASK) == tail;
}

// 检查缓冲区是否为空
inline bool virtualUART_IsBufferEmpty(int head, int tail)
{
    return head == tail;
}

// 获取缓冲区剩余大小, 以bit为单位
inline uint32_t virtualUART_GETBufferRemainingSize(int head, int tail)
{
    if (head >= tail)
    {
        return (BUFFER_ALIAS_SIZE - (head - tail));
    }
    return (tail - head);
}

inline bool virtualUART_IsAbleToWriteToBuffer(virtualUART_Transmitter *tx)
{
    return virtualUART_GETBufferRemainingSize(tx->head, tx->tail) >= 9 + tx->stopBits + tx->parityBits;
}

inline bool virtualUART_IsAbleToReadFromBuffer(virtualUART_Receiver *rx)
{
    int8_t remainingSize = virtualUART_GETBufferRemainingSize(rx->head, rx->tail);
    // Condition: There's another Byte be wtritten/writting to the buffer, or current Byte is complete and no error
    return (remainingSize > 8) || (remainingSize == 8 && (rx->state & 0b10000000) && rx->errorLevel == 0);
}

// 计算奇偶校验位
static inline uint8_t calculateParity(uint8_t byte)
{
    uint8_t parity = 0;
    while (byte)
    {
        parity ^= (byte & 1);
        byte >>= 1;
    }
    return parity;
}

static inline void aliasPointerIncrease(uint32_t *pointerToAliasVirtualPointer)
{
    (*pointerToAliasVirtualPointer)++;
    (*pointerToAliasVirtualPointer) &= BUFFER_ALIAS_SIZE_MASK;
}

static inline void aliasPointerDecrease(uint32_t *pointerToAliasVirtualPointer)
{
    (*pointerToAliasVirtualPointer)--;
    (*pointerToAliasVirtualPointer) &= BUFFER_ALIAS_SIZE_MASK;
}

/*
The bright day is done, and we are for the dark.

*/