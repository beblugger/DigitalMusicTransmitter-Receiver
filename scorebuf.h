#ifndef SCOREBUF_H
#define SCOREBUF_H

#include <stdint.h>
#include <stdbool.h>

#include "note.h"

typedef struct
{
    noteCommand buffer[32];
#define BUFFER_MASK 31
    uint8_t head, tail;
    uint16_t timeSinceLastCmd;
} scoreBuffer;

void updateBufCounter(scoreBuffer *buf);
uint8_t getBufRemainingSize(scoreBuffer *buf);
bool isBufFull(scoreBuffer *buf);
void addNoteToBuf(scoreBuffer *buf, noteCommand *note);
bool isCmdAvail(scoreBuffer *buf);
void getCmdFromBuf(scoreBuffer *buf, noteCommand *note);

#endif // SCOREBUF_H
