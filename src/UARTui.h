#ifndef UARTUI_H
#define UARTUI_H

#include <stdint.h>
#include <stdbool.h>

uint8_t processUARTInput(void);

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