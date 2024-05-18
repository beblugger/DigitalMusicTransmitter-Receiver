#ifndef NOTE_H
#define NOTE_H

#include <stdint.h>
#include <stdbool.h>

extern bool reSyncFlag;
// no 0xff is allow to appear in noteCmd, it's used for resync
typedef struct
{
    uint8_t pitch, intensity;
    uint16_t delay;
} noteCmd;

void clearNoteCmd(noteCmd *note);
void setNoteCmd(noteCmd *note, uint8_t pitch, uint8_t intensity, uint16_t delay);
bool noteCmdAlignedFill(noteCmd *note, uint8_t byte);
bool noteCmdSplit(noteCmd *note, uint8_t *byte);

#endif // NOTE_H
