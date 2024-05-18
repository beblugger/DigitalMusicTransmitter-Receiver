#include "scoreRec.h"

void initScoreRecorder(scoreRecorder *rec, noteCmd *noteCmdList, uint32_t maxSize)
{
    rec->noteCmdList = noteCmdList;
    rec->maxSize = maxSize;
    rec->cursor = 0;
}

void recordNoteCmd(scoreRecorder *rec, noteCmd *noteCmd)
{
    if (!isRecFull(rec))
    {
        rec->noteCmdList[rec->cursor++] = *noteCmd;
    }
}

bool isRecFull(scoreRecorder *rec)
{
    return rec->cursor >= rec->maxSize;
}

void resetRec(scoreRecorder *rec)
{
    rec->cursor = 0;
}

bool isCmdLeft(scoreRecorder *rec)
{
    return rec->cursor < rec->maxSize;
}

void getNoteCmd(scoreRecorder *rec, noteCmd *noteCmd)
{
    *noteCmd = rec->noteCmdList[rec->cursor++];
}
