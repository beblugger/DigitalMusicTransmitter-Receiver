#include "asynthesis.h"
#include "pitchperiod.h"

void clearPlayerState(playerState *player)
{
    // To optimize the performance, we don't need to clear the whole struct.
    player->playerTime = 0;
    player->activatedNoteNum = 0;
}

void setCommandNote(playerState *player, noteCmd *noteCmd)
{
    bool presentNoteFlag = false;
    uint8_t i = 0, j = 0;
    // Scan if the note is already in the list.
    for (i = 0; i < player->activatedNoteNum; i++)
    {
        if (player->noteList[i].pitch == noteCmd->pitch)
        {
            presentNoteFlag = true;
            break;
        }
    }

    if (!presentNoteFlag)
    {
        if (player->activatedNoteNum >= noteListSize || noteCmd->intensity == 0)
        {
            // The note may be lost, but it's stop command can be received.
            return;
        }
        player->noteList[player->activatedNoteNum] = *noteCmd;
        player->clearCounter[player->activatedNoteNum] = TIMEOUT;
        player->activatedNoteNum++;
    }
    else if (noteCmd->intensity == 0)
    {
        for (j = i; j < player->activatedNoteNum - 1; i++)
        {
            player->noteList[j] = player->noteList[j + 1];
        }
        player->activatedNoteNum--;
    }
    else
    {
        player->noteList[i].intensity = noteCmd->intensity;
    }
}

uint16_t getOutputIntensityBasic(playerState *player, uint32_t time)
{
    uint16_t outputIntensity = 0;
    uint16_t period = 0;
    uint16_t phase = 0;
    if (player->activatedNoteNum == 0)
    {
        return 0;
    }
    period = pitchPeriod[player->noteList[0].pitch];
    phase = time % period;
    outputIntensity += (phase < period / 2) ? 1 : 0;
    if (!player->noteList[0].intensity)
    {
        outputIntensity = 0;
    }
    return outputIntensity;
}

uint16_t getOutputIntensity(playerState *player, uint32_t time)
{
    // 有坑，不要用它, without guarantee of the correctness of the output.
    static uint16_t outputIntensity = 0;
    static uint16_t period = 0;
    static uint16_t phase = 0;
    uint16_t i = 0;
    if (player->activatedNoteNum == 0)
    {
        return 0;
    }
    for (i = 0; i < player->activatedNoteNum; i++)
    {
        period = pitchPeriod[player->noteList[i].pitch];
        phase = time % period;
        outputIntensity += (phase < period / 2) ? player->noteList[i].intensity : 0;
    }
    return outputIntensity;
}

void maintainPlayerState(playerState *player)
{
    uint8_t i = 0, j = 0;
    for (i = 0; i < player->activatedNoteNum; i++)
    {
        while (i < player->activatedNoteNum && player->clearCounter[i] <= 0)
        {
            for (j = i; j < player->activatedNoteNum - 1; j++)
            {
                player->noteList[j] = player->noteList[j + 1];
            }
            player->activatedNoteNum--;
        }
        player->clearCounter[i]--;
    }
}