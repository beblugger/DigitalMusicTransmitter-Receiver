#ifndef UARTUI_H
#define UARTUI_H

#include <stdint.h>
#include <stdbool.h>

#define TRANSMITTER_CMD_MASK 0b11000000
#define RECEIVER_CMD_MASK 0b10000000
#define ERROR_CMD_MASK 0b00000000

// Command Byte Masks
#define SET_SONG_CMD 0b11000000
#define PAUSE_SONG_CMD 0b11001000
#define RESUME_SONG_CMD 0b11010000
#define FM_TRANSMITTER_CMD 0b11011000
#define BUZZER_CMD 0b11100000
#define RECORD_CMD 0b10000000
#define PLAYRECORD_CMD 0b10001000
#define PLAYFM_CMD 0b10010000
#define ERROR_CMD 0b00111111
#define NO_CMD 0b00000000

/**
 * @brief Processes the UART input and returns the command byte.
 *
 * @return The command byte received from UART input.
 */
uint8_t processUARTInput(void);

/**
 * @brief Analyzes the command string and returns the corresponding command byte.
 *
 * @param command The command string to be analyzed.
 * @return The command byte corresponding to the command string.
 */
uint8_t analyzeCommand(char *command);

/**
 * @brief Sends a string message through UART.
 *
 * @param ui32Base The base address of the UART module.
 * @param cMessage The string message to be sent.
 */
void UARTStringPut(uint32_t ui32Base, const char *cMessage);

#endif /* UARTUI_H */

/*
Command Byte:
0b11xxxxx: Transmitter Command
0b10xxxxx: Receiver Command
0b00xxxxx: Error Command/No Command

Transmitter Command:
0b11000xxx: Set song to play
0b11001xxx: pause song
0b11010xxx: resume song
0b11011xxx: Turn on/off the FM transmitter
0b11100xxx: Turn on/off the buzzer

Receiver Command:
0b10000xxx: Record on/off
0b10001xxx: Play the recorded song
0b10010xxx: Play from FM receiver

Error Command:
0b00111111: Error Command
0b00000000: No Command
*/