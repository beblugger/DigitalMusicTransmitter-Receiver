#ifndef NOTE_H
#define NOTE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t pitch, delay;
    uint16_t intensity;
} noteCommand;

void clearNoteCommand(noteCommand *note);
void setNoteCommand(noteCommand *note, uint8_t pitch, uint8_t delay, uint16_t intensity);
bool noteCommandAlignedFill(noteCommand *note, uint8_t byte);

#endif // NOTE_H
