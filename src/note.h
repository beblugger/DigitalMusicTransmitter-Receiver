#ifndef NOTE_H
#define NOTE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Structure representing a musical note command.
 *
 * This structure contains information about the pitch, intensity, and delay since the last command
 * 0xff is not allowed to appear in the struct, and is used as a resync tag in the communication protocol.
 */
typedef struct
{
    uint8_t pitch;     /**< The pitch of the note. */
    uint8_t intensity; /**< The intensity of the note. */
    uint16_t delay;    /**< The delay before playing the note. Unit is microsecond */
} noteCmd;

/**
 * @brief Clears the note command.
 *
 * This function sets all the fields of the note command to 0xff.
 *
 * @param note Pointer to the note command to be cleared.
 */
void clearNoteCmd(noteCmd *note);

/**
 * @brief Sets the values of the note command.
 *
 * This function sets the pitch, intensity, and delay of the note command.
 *
 * @param note Pointer to the note command.
 * @param pitch The pitch of the note.
 * @param intensity The intensity of the note.
 * @param delay The delay before playing the note.
 */
void setNoteCmd(noteCmd *note, uint8_t pitch, uint8_t intensity, uint16_t delay);

/**
 * @brief Fills the note command with a byte.
 *
 * This function will set reSyncFlag to true if the byte is 0xff.
 * This function fills the note command with a byte value, aligning the pitch, intensity, and delay fields.
 *
 * @param note Pointer to the note command.
 * @param byte The byte value to fill the note command with.
 * 对于delay每个字节的最高位（MSB）用于指示是否有后续字节：
 * 最高位为 1 表示后面还有更多字节。
 * 最高位为 0 表示这是最后一个字节。
 * More than 2 bytes are not allowed.
 * @return True if the note command was filled to full, false otherwise.
 */
bool noteCmdAlignedFill(noteCmd *note, const uint8_t byte);

/**
 * @brief Splits the note command into a byte.
 *
 * This function splits the note command into a byte value, extracting the pitch, intensity, and delay fields.
 *
 * @param note Pointer to the note command.
 * @param byte Pointer to store the split byte value.
 * @return True if the note command was splited to empty, false otherwise.
 */
bool noteCmdSplit(noteCmd *note, uint8_t *byte);

extern bool reSyncFlag; /**< Flag indicating if a resynchronization is needed. */

#endif // NOTE_H
