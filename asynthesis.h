#ifndef ASYNTHESIS_H
#define ASYNTHESIS_H

#include "note.h"
typedef struct
{
    noteCmd noteList[8];
    uint32_t playerTime;
    uint8_t activatedNoteNum;
} playerState;

void clearPlayerState(playerState *player);
void setCommandNote(playerState *player, noteCmd *noteCmd);
uint16_t getOutputIntensity(playerState *player, uint32_t time);

#endif // ASYNTHESIS_H
