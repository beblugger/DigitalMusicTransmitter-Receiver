#include "note.h"

bool reSyncFlag;
void clearNoteCmd(noteCmd *note)
{
    note->pitch = 0xff;
    note->delay = 0xffff;
    note->intensity = 0xff;
}

void setNoteCmd(noteCmd *note, uint8_t pitch, uint8_t intensity, uint16_t delay)
{
    note->pitch = pitch;
    note->intensity = intensity;
    note->delay = delay;
}

bool noteCmdAlignedFill(noteCmd *note, uint8_t byte)
{
    if (byte == 0xff)
    {
        clearNoteCmd(note);
        reSyncFlag = true;
        return false;
    }
    else if (note->pitch == 0xff)
    {
        note->pitch = byte;
        return false;
    }
    else if (note->intensity == 0xff)
    {
        note->intensity = byte;
        return false;
    }
    else if (note->delay == 0xffff)
    {
        note->delay = byte;
        return !(byte & 0x80);
    }
    else if (note->delay & 0x0080)
    {
        note->delay = (note->delay & 0xff7f) | ((uint16_t)byte << 7);
        return true;
    }
    else
    {
        return true;
    }
}

bool noteCmdSplit(noteCmd *note, uint8_t *byte)
{
    if (note->pitch != 0xff)
    {
        *byte = note->pitch;
        note->pitch = 0xff;
        return false;
    }
    else if (note->intensity != 0xff)
    {
        *byte = note->intensity;
        note->intensity = 0xff;
        return false;
    }
    else if ((uint8_t)note->delay != 0xff)
    {
        *byte = ((uint8_t)note->delay & 0x7f);
        note->delay = note->delay << 1 | 0x00ff;
        if (note->delay == 0x00ff)
        {
            note->delay = 0xffff;
            return true;
        }
        else
        {
            *byte |= 0x80;
            return false;
        }
    }
    else
    {
        if (note->delay == 0xffff)
        {
            *byte = 0xff;
            return true;
        }
        *byte = note->delay >> 8;
        note->delay = 0xffff;
        return true;
    }
}