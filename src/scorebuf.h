#ifndef SCOREBUF_H
#define SCOREBUF_H

#include <stdint.h>
#include <stdbool.h>

#include "note.h"

/**
 * @brief Structure representing a score buffer.
 */
typedef struct
{
    noteCmd buffer[32];       /**< Array to store note commands. */
#define SCORE_BUFFER_MASK 31  /**< Mask to limit buffer size to 32. */
    uint8_t head, tail;       /**< Indices for head and tail of the buffer. */
    int16_t timeSinceLastCmd; /**< Time since the last command was added to the buffer. */
} scoreBuffer;

/**
 * @brief Initializes the score buffer.
 *
 * @param buf Pointer to the score buffer.
 */
void initBuf(scoreBuffer *buf);

/**
 * @brief Updates the buffer counter.
 *
 * @param buf Pointer to the score buffer.
 */
void updateBufCounter(scoreBuffer *buf);

/**
 * @brief Gets the remaining size of the buffer.
 *
 * @param buf Pointer to the score buffer.
 * @return The remaining size of the buffer.
 */
uint8_t getBufRemainingSize(scoreBuffer *buf);

/**
 * @brief Checks if the buffer is full.
 *
 * @param buf Pointer to the score buffer.
 * @return True if the buffer is full, false otherwise.
 */
bool isBufFull(scoreBuffer *buf);

/**
 * @brief Adds a note command to the buffer.
 *
 * @param buf Pointer to the score buffer.
 * @param note Pointer to the note command to be added.
 */
void addNoteToBuf(scoreBuffer *buf, noteCmd *note);

/**
 * @brief Checks if a command is available in the buffer.
 *
 * @param buf Pointer to the score buffer.
 * @return True if a command is available, false otherwise.
 */
bool isCmdAvail(scoreBuffer *buf);

/**
 * @brief Gets a command from the buffer.
 *
 * @param buf Pointer to the score buffer.
 * @param note Pointer to store the retrieved command.
 */
void getCmdFromBuf(scoreBuffer *buf, noteCmd *note);

#endif // SCOREBUF_H
