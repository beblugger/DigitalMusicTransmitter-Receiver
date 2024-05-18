#include "scoreBuf.h"

// 初始化缓冲区
void initBuf(scoreBuffer *buf)
{
    buf->head = 0;
    buf->tail = 0;
    buf->timeSinceLastCmd = 0;
}

// 更新缓冲区计数器
void updateBufCounter(scoreBuffer *buf)
{
    buf->timeSinceLastCmd++;
}

// 获取缓冲区剩余空间大小
uint8_t getBufRemainingSize(scoreBuffer *buf)
{
    return SCORE_BUFFER_MASK - ((buf->tail - buf->head) & SCORE_BUFFER_MASK);
}

// 检查缓冲区是否已满
bool isBufFull(scoreBuffer *buf)
{
    return buf->head == ((buf->tail + 1) & SCORE_BUFFER_MASK);
}

// 将音符指令添加到缓冲区
void addNoteToBuf(scoreBuffer *buf, noteCmd *note)
{
    if (!isBufFull(buf))
    {
        buf->buffer[buf->tail] = *note;
        buf->tail = (buf->tail + 1) & SCORE_BUFFER_MASK;
    }
}

// 检查是否有可用的指令
bool isCmdAvail(scoreBuffer *buf)
{
    if (buf->head == buf->tail)
    {
        return false;
    }
    return buf->buffer[buf->head].delay <= buf->timeSinceLastCmd;
}

// 从缓冲区中获取指令
void getCmdFromBuf(scoreBuffer *buf, noteCmd *note)
{
    if (isCmdAvail(buf))
    {
        *note = buf->buffer[buf->head];
        buf->head = (buf->head + 1) & SCORE_BUFFER_MASK;
        buf->timeSinceLastCmd -= note->delay;
    }
}
