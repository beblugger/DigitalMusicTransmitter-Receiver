#include "transmitter.h"

#define UARTMBaseFreq 2000
#define SysTickResloution 120
#define _1ms UARTMBaseFreq *SysTickResloution / 1000
#define RESLUTTION_TIME 10
#define _1s UARTMBaseFreq *SysTickResloution

static uint32_t SystemClkFrequency = 0;

volatile static VUARTStreamBuffer txBuffer;

static uint8_t counter = 0;

static scoreBuffer PF3Buffer, FMBuffer;
static playerState PF3player;
static scoreRecorder myChineseHeartRecorder, myChineseHeartRecFM;

static volatile bool ResumePause, FMState, PF3State;

int main(void)
{
    static noteCmd tmpNote;
    static uint8_t command;
    // initalize
    InitGPIO();
    SystemClkFrequency = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                            120000000);
    VUARTInitTransmitter(&txBuffer, 1, 1);
    writeResyncToTransmitter(&txBuffer);

    InitSysTick(SystemClkFrequency, UARTMBaseFreq * SysTickResloution);
    InitUART(SystemClkFrequency, UARTMBaseFreq);
    ResumePause = true;
    FMState = true;
    PF3State = false;
    replay(myChineseHeart, &PF3Buffer, &FMBuffer, &PF3player, &myChineseHeartRecorder, &myChineseHeartRecFM, 110);

    while (true)
    {
        if (isAbleToWriteNoteToTransmitter(&txBuffer) && isCmdAvail(&FMBuffer))
        {
            getCmdFromBuf(&FMBuffer, &tmpNote);
            writeNoteToTransmitter(&txBuffer, &tmpNote);
        }
        if (isCmdLeft(&myChineseHeartRecFM) && !isBufFull(&FMBuffer))
        {
            getNoteCmd(&myChineseHeartRecFM, &tmpNote);
            addNoteToBuf(&FMBuffer, &tmpNote);
        }
        if (isCmdLeft(&myChineseHeartRecorder) && !isBufFull(&PF3Buffer))
        {
            getNoteCmd(&myChineseHeartRecorder, &tmpNote);
            addNoteToBuf(&PF3Buffer, &tmpNote);
        }
        command = processUARTInput();
        if ((command & TRANSMITTER_CMD_MASK) == TRANSMITTER_CMD_MASK)
        {
            switch (command & 0b11111000)
            {
            case SET_SONG_CMD:
                replay(myChineseHeart, &PF3Buffer, &FMBuffer, &PF3player, &myChineseHeartRecorder, &myChineseHeartRecFM, 110);
                break;
            case PAUSE_SONG_CMD:
                ResumePause = false;
                break;
            case RESUME_SONG_CMD:
                ResumePause = true;
                break;
            case FM_TRANSMITTER_CMD:
                FMState = command & 0b00000111;
                break;
            case BUZZER_CMD:
                PF3State = command & 0b00000111;
                break;
            default:
                break;
            }
        }
        if (!isCmdLeft(&myChineseHeartRecorder))
        {
            replay(myChineseHeart, &PF3Buffer, &FMBuffer, &PF3player, &myChineseHeartRecorder, &myChineseHeartRecFM, 110);
        }
    }
    return 0;
}

void SysTick_Handler(void)
{
    static volatile uint16_t _1msCounter = 0;
    static volatile uint8_t resCounter = 0;
    static volatile uint32_t _1sCounter = 0;
    static volatile noteCmd tmpNote;
    resCounter++;
    _1msCounter++;
    _1sCounter++;

    updateP0State();
    // updateK0State();

    if (resCounter >= RESLUTTION_TIME)
    {
        resCounter = 0;
        if (PF3State && ResumePause)
            updateF3State();
    }
    if (_1msCounter >= _1ms)
    {
        _1msCounter = 0;
        if (ResumePause)
        {
            PF3Buffer.timeSinceLastCmd++;
            FMBuffer.timeSinceLastCmd++;
            while (isCmdAvail(&PF3Buffer))
            {
                clearPlayerState(&PF3player);
                getCmdFromBuf(&PF3Buffer, &tmpNote);
                setCommandNote(&PF3player, &tmpNote);
            }
        }
    }
    if (_1sCounter >= _1s)
    {
        _1sCounter = 0;
        writeResyncToTransmitter(&txBuffer);
    }
}

void replay(noteCmd *noteList, scoreBuffer *LocalBuffer, scoreBuffer *FMBuffer, playerState *Localplayer, scoreRecorder *LocalRec, scoreRecorder *FMRec, uint32_t maxSize)
{
    noteCmd tmpNote;
    clearPlayerState(Localplayer);
    initBuf(LocalBuffer);
    initBuf(FMBuffer);
    initScoreRecorder(LocalRec, noteList, maxSize);
    initScoreRecorder(FMRec, noteList, maxSize);

    while (!isBufFull(LocalBuffer) && isCmdLeft(LocalRec))
    {
        getNoteCmd(LocalRec, &tmpNote);
        addNoteToBuf(LocalBuffer, &tmpNote);
        addNoteToBuf(FMBuffer, &tmpNote);
    }

    FMRec->cursor = LocalRec->cursor;
    FMBuffer->timeSinceLastCmd = 1024;
}

void updateF3State(void)
{
    static volatile uint32_t time = 0;
    static volatile bool PF3State = 0;
    static bool PF3StateNew;
    PF3StateNew = getOutputIntensityBasic(&PF3player, time);
    if (PF3StateNew != PF3State)
    {
        PF3State = PF3StateNew;
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, PF3State ? GPIO_PIN_3 : 0);
    }
    time++;
}

void updateP0State(void)
{
    static volatile uint16_t SysTickPhaseCounter = 0;
    static volatile bool bitWritingtoP0 = 1;
    // convert VUART -> UARTM, txBuffer -> P0

    if (SysTickPhaseCounter >= SysTickResloution)
    {
        SysTickPhaseCounter = 0;
        bitWritingtoP0 = VUARTReadBitFromTransmitter(&txBuffer);
        if (FMState)
            GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_0, GPIO_PIN_0);
    }
    else if (SysTickPhaseCounter == SysTickResloution * 2 / 3)
    {
        if (bitWritingtoP0)
        {
            GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_0, 0);
        }
    }
    else if (SysTickPhaseCounter == SysTickResloution / 3)
    {
        if (!bitWritingtoP0)
        {
            GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_0, 0);
        }
    }
    SysTickPhaseCounter++;
}

void updateK0State(void)
{
    static volatile int16_t activatedLength = 0;
    static volatile uint16_t counterSinceLastUpdateK0 = 0;
    // convert UARTM -> UART, P1 -> K0

    // After waiting too long for a bit, put 0 to K0
    counterSinceLastUpdateK0++;
    if (counterSinceLastUpdateK0 > SysTickResloution * 2)
    {
        counterSinceLastUpdateK0 = 0;
        activatedLength = 0;
        GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, 0);
    }

    // update K0 if a bit is received
    if (GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_1) == 0)
    {
        activatedLength++;
    }
    else
    {
        if (activatedLength > SysTickResloution / 2)
        {
            counterSinceLastUpdateK0 = 0;
            activatedLength = 0;
            GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, 0);
        }
        else if (activatedLength > SysTickResloution / 6)
        {
            counterSinceLastUpdateK0 = 0;
            activatedLength = 0;
            GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, GPIO_PIN_0);
        }
        activatedLength--;
        if (activatedLength < 0)
        {
            activatedLength = 0;
        }
    }
}

bool isAbleToWriteNoteToTransmitter(volatile VUARTStreamBuffer *tx)
{
    return VUARTGetBufferRemainingSize(tx->head, tx->tail) > 12 * 2 + 11 * 4;
}

void writeNoteToTransmitter(volatile VUARTStreamBuffer *tx, noteCmd *note)
{
    if (!isAbleToWriteNoteToTransmitter(tx))
    {
        return;
    }
    uint8_t byte;
    bool finished = false;
    while (!finished)
    {
        finished = noteCmdSplit(note, &byte);
        VUARTWriteByteToTransmitter(tx, byte);
    }
}

void writeResyncToTransmitter(volatile VUARTStreamBuffer *tx)
{
    if (VUARTGetBufferRemainingSize(tx->head, tx->tail) < 12 * 2)
    {
        return;
    }

    VUARTAddStopBit(tx, 12);
    VUARTWriteByteToTransmitter(tx, 0xff);
}