#ifndef SCOREBUF_H
#define SCOREBUF_H

#include <stdint.h>
#include <stdbool.h>

#include "note.h"

typedef struct
{
    noteCmd buffer[32];
#define SCORE_BUFFER_MASK 31
    uint8_t head, tail;
    int16_t timeSinceLastCmd;
} scoreBuffer;

void initBuf(scoreBuffer *buf);
void updateBufCounter(scoreBuffer *buf);
uint8_t getBufRemainingSize(scoreBuffer *buf);
bool isBufFull(scoreBuffer *buf);
void addNoteToBuf(scoreBuffer *buf, noteCmd *note);
bool isCmdAvail(scoreBuffer *buf);
void getCmdFromBuf(scoreBuffer *buf, noteCmd *note);

#endif // SCOREBUF_H
