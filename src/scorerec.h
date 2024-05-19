#ifndef SCOREREC_H
#define SCOREREC_H

#include <stdint.h>
#include <stdbool.h>

#include "note.h"

/**
 * @brief Structure representing a score recorder.
 */
typedef struct
{
    noteCmd *noteCmdList; /**< Pointer to the list of note commands. */
    uint32_t maxSize;     /**< Maximum size of the note command list. */
    uint32_t cursor;      /**< Current position in the note command list. */
} scoreRecorder;

/**
 * @brief Initializes a score recorder.
 *
 * @param rec Pointer to the score recorder.
 * @param noteCmdList Pointer to the list of note commands.
 * @param maxSize Maximum size of the note command list.
 */
void initScoreRecorder(scoreRecorder *rec, noteCmd *noteCmdList, uint32_t maxSize);

/**
 * @brief Records a note command in the score recorder.
 *
 * @param rec Pointer to the score recorder.
 * @param noteCmd Pointer to the note command to be recorded.
 */
void recordNoteCmd(scoreRecorder *rec, noteCmd *noteCmd);

/**
 * @brief Checks if the score recorder has reached the end.
 *
 * @param rec Pointer to the score recorder.
 * @return true if the end of the score recorder has been reached, false otherwise.
 */
bool isRecEnd(scoreRecorder *rec);

/**
 * @brief Resets the score recorder to its initial state.
 *
 * @param rec Pointer to the score recorder.
 */
void resetRec(scoreRecorder *rec);

/**
 * @brief Checks if there are any note commands left in the score recorder.
 *
 * @param rec Pointer to the score recorder.
 * @return true if there are note commands left, false otherwise.
 */
bool isCmdLeft(scoreRecorder *rec);

/**
 * @brief Retrieves the next note command from the score recorder.
 *
 * @param rec Pointer to the score recorder.
 * @param noteCmd Pointer to store the retrieved note command.
 */
void getNoteCmd(scoreRecorder *rec, noteCmd *noteCmd);

#endif // SCOREREC_H
