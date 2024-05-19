#ifndef ASYNTHESIS_H
#define ASYNTHESIS_H

#include "note.h"

#define noteListSize 8
// Define the timeout for clearing the note. It should be in range of int16_t.
#define TIMEOUT 4096
/**
 * @brief Represents the state of a player.
 *
 * The `playerState` struct contains information about the notes being played, the player time, and the number of activated notes.
 */
typedef struct
{
    noteCmd noteList[noteListSize];     /**< The list of notes being played. */
    int16_t clearCounter[noteListSize]; /**< The counter for clearing each note. */
    uint32_t playerTime;                /**< The current time of the player. Reserved */
    uint8_t activatedNoteNum;           /**< The number of activated notes. */
} playerState;

/**
 * @brief Clears the state of the player.
 *
 * This function resets the player state.
 *
 * @param player A pointer to the playerState structure to be cleared.
 */
void clearPlayerState(playerState *player);
/**
 * Sets the command note for the player.
 *
 * @param player A pointer to the player state.
 * @param noteCmd A pointer to the note command.
 */
void setCommandNote(playerState *player, noteCmd *noteCmd);
/**
 * @brief Calculates the output intensity based on the player state and time.
 *
 * @param player A pointer to the playerState struct representing the player.
 * @param time The time at which to retrieve the output intensity.
 * @return The output intensity range in [0,4095].
 */
uint16_t getOutputIntensity(playerState *player, uint32_t time);
/**
 * @brief 有坑，不要用它. Calculates the output intensity based on the player state and time. With simpler timbre and harmonics.
 *
 * @param player A pointer to the playerState struct representing the player.
 * @param time The time at which to retrieve the output intensity.
 * @return The output intensity range in [0,4095].
 */
uint16_t getOutputIntensityBasic(playerState *player, uint32_t time);

/**
 * @brief Clear notes without stop command.
 *
 * Clear the note if the note is played too long (after the fuction is executed for TIMEOUT times). If the stop command is lost, the note will be cleared here.
 *
 * @param player A pointer to a `playerState` structure.
 */
void maintainPlayerState(playerState *player);

#endif // ASYNTHESIS_H
