#ifndef SCOREREC_H
#define SCOREREC_H

#include <stdint.h>
#include <stdbool.h>

#include "note.h"
typedef struct
{
    noteCommand *noteCmdList;
    uint32_t maxSize;
    uint32_t cursor;
} scoreRecorder;

void initScoreRecorder(scoreRecorder *rec, noteCommand *noteCmdList, uint32_t maxSize);
void recordNoteCmd(scoreRecorder *rec, noteCommand *noteCmd);
bool isRecFull(scoreRecorder *rec);
void resetRec(scoreRecorder *rec);
bool isCmdLeft(scoreRecorder *rec);
void getNoteCmd(scoreRecorder *rec, noteCommand *noteCmd);

#endif // SCOREREC_H
