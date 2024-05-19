#include "receiver.h"

#define UARTMBaseFreq 2000
#define SysTickResloution 120
#define _1ms UARTMBaseFreq *SysTickResloution / 1000
#define RESLUTTION_TIME 10
#define _2s UARTMBaseFreq *SysTickResloution * 2

static uint32_t SystemClkFrequency = 0;

static uint8_t counter = 0;

static scoreBuffer PF3Buffer;
static playerState PF3player;
static scoreRecorder recorder;
static volatile noteCmd recorderList[256];
static volatile bool recordState;
static volatile bool singalSource; // 0: FM, 1: recorder
static volatile bool syncState;

static volatile uint32_t _2sCounter = 0;

// static char debugString[100] = "Debug: Received command: 000\n";

int main(void)
{
    static noteCmd tmpNote;
    static uint8_t command;
    syncState = false;
    static volatile bool noteFilled = false;
    clearNoteCmd(&tmpNote);
    // initalize
    InitGPIO();
    SystemClkFrequency = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                            120000000);

    InitSysTick(SystemClkFrequency, UARTMBaseFreq * SysTickResloution);
    InitUART(SystemClkFrequency, UARTMBaseFreq);

    initScoreRecorder(&recorder, recorderList, 0);

    while (true)
    {
        if (singalSource == 0)
        {
            if (reSyncFlag)
            {
                _2sCounter = 0;
                reSyncFlag = false;
                if (!syncState)
                {
                    clearPlayerState(&PF3player);
                }
                syncState = true;
                UARTStringPut(UART0_BASE, "Sync received!\n");
            }
            if (_2sCounter >= _2s)
            {
                _2sCounter = 0;
                syncState = false;
                UARTStringPut(UART0_BASE, "Sync timeout.\n");
                initBuf(&PF3Buffer);
            }
        }

        switch (singalSource)
        {
        case 0:
            if (syncState)
            {
                if (UARTCharsAvail(UART2_BASE) && !noteFilled)
                {
                    command = UARTCharGet(UART2_BASE);
                    // debugString[25] = command / 100 + '0';
                    // debugString[26] = (command / 10) % 10 + '0';
                    // debugString[27] = command % 10 + '0';
                    // UARTStringPut(UART0_BASE, debugString);
                    noteFilled = noteCmdAlignedFill(&tmpNote, command);
                }
                if (noteFilled && !isBufFull(&PF3Buffer))
                {
                    addNoteToBuf(&PF3Buffer, &tmpNote);
                    if (recordState && recorder.maxSize < 256)
                    {
                        recordNoteCmd(&recorder, &tmpNote);
                    }
                    clearNoteCmd(&tmpNote);
                    noteFilled = false;
                }
            }
            else
            {
                if (UARTCharsAvail(UART2_BASE))
                {
                    command = UARTCharGet(UART2_BASE);
                    noteFilled = noteCmdAlignedFill(&tmpNote, command);
                }
            }
            break;
        case 1:
            if (isCmdLeft(&recorder) && !isBufFull(&PF3Buffer))
            {
                getNoteCmd(&recorder, &tmpNote);
                addNoteToBuf(&PF3Buffer, &tmpNote);
            }
            break;
        }

        command = processUARTInput();
        if ((command & RECEIVER_CMD_MASK) == RECEIVER_CMD_MASK)
        {
            switch (command & 0b10111000)
            {
            case RECORD_CMD:
                recordState = command & 0b00000111;
                if (recordState)
                {
                    resetRec(&recorder);
                    recorder.maxSize = 0;
                }
                break;
            case PLAYRECORD_CMD:
                syncState = true;
                singalSource = 1;
                replay(&PF3Buffer, &PF3player, &recorder);
                break;
            case PLAYFM_CMD:
                syncState = false;
                recordState = false;
                singalSource = 0;
                clearNoteCmd(&tmpNote);
                clearPlayerState(&PF3player);
                initBuf(&PF3Buffer);
                break;
            default:
                break;
            }
        }
    }
    return 0;
}

void SysTick_Handler(void)
{
    static volatile uint16_t _1msCounter = 0;
    static volatile uint8_t resCounter = 0;
    static volatile noteCmd tmpNote;
    resCounter++;
    _1msCounter++;
    _2sCounter++;

    updateK0State();

    if (resCounter >= RESLUTTION_TIME)
    {
        resCounter = 0;
        updateF3State();
    }
    if (_1msCounter >= _1ms)
    {
        _1msCounter = 0;
        PF3Buffer.timeSinceLastCmd++;
        while (isCmdAvail(&PF3Buffer))
        {
            clearPlayerState(&PF3player);
            getCmdFromBuf(&PF3Buffer, &tmpNote);
            setCommandNote(&PF3player, &tmpNote);
        }
        if (!syncState && PF3Buffer.head == PF3Buffer.tail)
        {
            clearPlayerState(&PF3player);
        }
        maintainPlayerState(&PF3player);
    }
}

void replay(scoreBuffer *LocalBuffer, playerState *Localplayer, scoreRecorder *LocalRec)
{
    noteCmd tmpNote;
    clearPlayerState(Localplayer);
    initBuf(LocalBuffer);
    resetRec(LocalRec);

    while (!isBufFull(LocalBuffer) && isCmdLeft(LocalRec))
    {
        getNoteCmd(LocalRec, &tmpNote);
        addNoteToBuf(LocalBuffer, &tmpNote);
    }
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

static volatile int16_t activatedLength = 0;
static volatile uint16_t counterSinceLastUpdateK0 = 0;

void updateK0State(void)
{
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