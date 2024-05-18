#include "UARTui.h"
#include "inc/hw_memmap.h"
#include "driverlib/uart.h"

uint8_t analyzeCommandReceiver(char *command);
uint8_t analyzeCommandTransmitter(char *command);

void UARTStringPut(uint32_t ui32Base, char *cMessage)
{
    while (*cMessage != '\0')
    {
        UARTCharPut(ui32Base, *cMessage);
        cMessage++;
    }
}

const char errorCommand[] = "Error Command\n";
const char longCommand[] = "Command too long\n";

uint8_t processUARTInput(void)
{
    static char buffer[16];
    static uint8_t bufferIndex = 0;
    static uint8_t command = 0;

    while (UARTCharsAvail(UART2_BASE))
    {
        if (bufferIndex >= 14)
        {
            bufferIndex = 0;
            UARTStringPut(UART2_BASE, longCommand);
        }
        buffer[bufferIndex] = UARTCharGet(UART2_BASE);
        bufferIndex++;
        if (buffer[bufferIndex - 1] == '\n' || buffer[bufferIndex - 1] == '\r')
        {
            buffer[bufferIndex - 1] = '\0';
            bufferIndex = 0;
#ifdef RECEIVER
            command = analyzeCommandReceiver(buffer);
#endif
#ifdef TRANSMITTER
            command = analyzeCommandTransmitter(buffer);
#endif
            if (command == 0b00111111)
            {
                UARTStringPut(UART2_BASE, errorCommand);
            }
            return command;
        }
    }
    return 0;
}

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