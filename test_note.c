#include <stdio.h>
#include <stdbool.h>
#include "note.h"

void test_clearNoteCmd()
{
    noteCmd note;
    clearNoteCmd(&note);
    // Verify that all fields are set to their default values
    if (note.pitch == 0xff && note.intensity == 0xff && note.delay == 0xffff)
    {
        printf("clearNoteCmd test passed.\n");
    }
    else
    {
        printf("clearNoteCmd test failed.\n");
    }
}

void test_setNoteCmd()
{
    noteCmd note;
    setNoteCmd(&note, 0x80, 0x7f, 1000);
    // Verify that the note fields are set correctly
    if (note.pitch == 0x80 && note.intensity == 0x7f && note.delay == 1000)
    {
        printf("setNoteCmd test passed.\n");
    }
    else
    {
        printf("setNoteCmd test failed.\n");
    }
}

void test_noteCmdAlignedFill()
{
    noteCmd note;
    bool result;

    // Test case 1: byte is 0xff
    result = noteCmdAlignedFill(&note, 0xff);
    // Verify that note fields are cleared and reSyncFlag is set to true
    if (note.pitch == 0xff && note.intensity == 0xff && note.delay == 0xffff && result == false)
    {
        printf("noteCmdAlignedFill test case 1 passed.\n");
    }
    else
    {
        printf("noteCmdAlignedFill test case 1 failed.\n");
    }

    // Test case 2: note->pitch is 0xff
    result = noteCmdAlignedFill(&note, 0x80);
    // Verify that note->pitch is set and result is false
    if (note.pitch == 0x80 && result == false)
    {
        printf("noteCmdAlignedFill test case 2 passed.\n");
    }
    else
    {
        printf("noteCmdAlignedFill test case 2 failed.\n");
    }

    // Test case 3: note->intensity is 0xff
    result = noteCmdAlignedFill(&note, 0x7f);
    // Verify that note->intensity is set and result is false
    if (note.intensity == 0x7f && result == false)
    {
        printf("noteCmdAlignedFill test case 3 passed.\n");
    }
    else
    {
        printf("noteCmdAlignedFill test case 3 failed.\n");
    }

    // Test case 4: note->delay is 0xffff
    result = noteCmdAlignedFill(&note, 0x01);
    // Verify that note->delay is set and result is true
    if (note.delay == 0x01 && result == true)
    {
        printf("noteCmdAlignedFill test case 4 passed.\n");
    }
    else
    {
        printf("noteCmdAlignedFill test case 4 failed.\n");
    }

    // Test case 5: note->delay & 0x0080 is true
    note.delay = 0xffff;
    result = noteCmdAlignedFill(&note, 0x81);
    result = noteCmdAlignedFill(&note, 0x20);
    // Verify that note->delay is updated and result is true
    if (note.delay == 0x1001 && result == true)
    {
        printf("noteCmdAlignedFill test case 5 passed.\n");
    }
    else
    {
        printf("noteCmdAlignedFill test case 5 failed.\n");
    }

    // Test case 6: none of the conditions are met
    note.pitch = 0x80;
    note.intensity = 0x7f;
    note.delay = 0x01;
    result = noteCmdAlignedFill(&note, 0x01);
    // Verify that result is true
    if (result == true)
    {
        printf("noteCmdAlignedFill test case 6 passed.\n");
    }
    else
    {
        printf("noteCmdAlignedFill test case 6 failed.\n");
    }
}

void test_noteCmdSplit()
{
    noteCmd note;
    uint8_t byte, byte2;
    bool result, result2;

    // Test case 1: note->pitch is not 0xff
    note.pitch = 0x80;
    result = noteCmdSplit(&note, &byte);
    // Verify that byte is set to note->pitch and result is false
    if (byte == 0x80 && result == false)
    {
        printf("noteCmdSplit test case 1 passed.\n");
    }
    else
    {
        printf("noteCmdSplit test case 1 failed.\n");
    }

    // Test case 2: note->intensity is not 0xff
    note.intensity = 0x7f;
    result = noteCmdSplit(&note, &byte);
    // Verify that byte is set to note->intensity and result is false
    if (byte == 0x7f && result == false)
    {
        printf("noteCmdSplit test case 2 passed.\n");
    }
    else
    {
        printf("noteCmdSplit test case 2 failed.\n");
    }

    // Test case 3: (uint8_t)note->delay is not 0xff
    note.delay = 0x01;
    result = noteCmdSplit(&note, &byte);
    // Verify that byte is set to (uint8_t)note->delay & 0x7f and result is false
    if (byte == 0x01 && result == true)
    {
        printf("noteCmdSplit test case 3 passed.\n");
    }
    else
    {
        printf("noteCmdSplit test case 3 failed.\n");
    }

    // Test case 4: none of the conditions are met
    note.delay = 0xffff;
    result = noteCmdSplit(&note, &byte);
    // Verify that byte is set to note->delay >> 8 and result is true
    if (byte == 0xff && result == true)
    {
        printf("noteCmdSplit test case 4 passed.\n");
    }
    else
    {
        printf("noteCmdSplit test case 4 failed.\n");
    }

    // Test case 5: note->delay & 0x0080 is true
    note.delay = 0x1001;
    result = noteCmdSplit(&note, &byte);
    result2 = noteCmdSplit(&note, &byte2);
    if (byte == 0x81 && result == false && byte2 == 0x20 && result2 == true)
    {
        printf("noteCmdSplit test case 5 passed.\n");
    }
    else
    {
        printf("noteCmdSplit test case 5 failed.\n");
    }
}

int main()
{
    test_clearNoteCmd();
    test_setNoteCmd();
    test_noteCmdAlignedFill();
    test_noteCmdSplit();

    return 0;
}