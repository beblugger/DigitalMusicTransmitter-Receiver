#include "UARTui.h"
#include "inc/hw_memmap.h"
#include "driverlib/uart.h"

#include <string.h>
// #include <stdio.h>

uint8_t analyzeCommandReceiver(char *command);
uint8_t analyzeCommandTransmitter(char *command);
uint8_t analyzeCommand(char *command);

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
            command = analyzeCommand(buffer);
            if (command == 0b00111111)
            {
                UARTStringPut(UART2_BASE, errorCommand);
            }
            return command;
        }
    }
    return 0;
}

#define TRANSMITTER_CMD_MASK 0b11000000
#define RECEIVER_CMD_MASK 0b10000000
#define ERROR_CMD_MASK 0b00000000

// 定义命令字节
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

uint8_t analyzeCommand(char *command)
{
    if (command == NULL)
    {
        return ERROR_CMD;
    }

    char *token = strtok(command, " ");
    if (token == NULL)
    {
        return ERROR_CMD;
    }

    if (strcmp(token, "play") == 0)
    {
        token = strtok(NULL, " ");
        if (token == NULL)
        {
            return ERROR_CMD;
        }
        int songNumber = atoi(token);
        return SET_SONG_CMD | (songNumber & 0b00000111);
    }
    else if (strcmp(token, "pause") == 0)
    {
        return PAUSE_SONG_CMD;
    }
    else if (strcmp(token, "resume") == 0)
    {
        return RESUME_SONG_CMD;
    }
    else if (strcmp(token, "transmitter") == 0)
    {
        token = strtok(NULL, " ");
        if (token == NULL)
        {
            return ERROR_CMD;
        }
        int transmitterState = atoi(token);
        return FM_TRANSMITTER_CMD | ((transmitterState == 1) ? 0b000001 : 0);
    }
    else if (strcmp(token, "buzzer") == 0)
    {
        token = strtok(NULL, " ");
        if (token == NULL)
        {
            return ERROR_CMD;
        }
        int buzzerState = atoi(token);
        return BUZZER_CMD | ((buzzerState == 1) ? 0b000001 : 0);
    }
    else if (strcmp(token, "record") == 0)
    {
        token = strtok(NULL, " ");
        if (token == NULL)
        {
            return ERROR_CMD;
        }
        int recordState = atoi(token);
        return RECORD_CMD | ((recordState == 1) ? 0b000001 : 0);
    }
    else if (strcmp(token, "playrecord") == 0)
    {
        return PLAYRECORD_CMD;
    }
    else if (strcmp(token, "playfm") == 0)
    {
        return PLAYFM_CMD;
    }
    else
    {
        return ERROR_CMD;
    }
}