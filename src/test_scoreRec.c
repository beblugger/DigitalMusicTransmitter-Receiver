#include "unity/unity.h"
#include "scoreRec.h"
#include "note.h"

void setUp(void) {}

void tearDown(void) {}

void test_initScoreRecorder(void)
{
    noteCmd noteCmdList[100];
    scoreRecorder recorder;

    initScoreRecorder(&recorder, noteCmdList, 100);

    TEST_ASSERT_EQUAL_PTR(noteCmdList, recorder.noteCmdList);
    TEST_ASSERT_EQUAL(100, recorder.maxSize);
    TEST_ASSERT_EQUAL(0, recorder.cursor);
}

void test_recordNoteCmd(void)
{
    noteCmd note1 = {60, 100, 500};
    noteCmd note2 = {62, 100, 500};

    noteCmd noteCmdList[2];
    scoreRecorder recorder;
    initScoreRecorder(&recorder, noteCmdList, 2);

    recordNoteCmd(&recorder, &note1);
    recordNoteCmd(&recorder, &note2);

    TEST_ASSERT_EQUAL(2, recorder.cursor);
    TEST_ASSERT_EQUAL_UINT8(note1.pitch, recorder.noteCmdList[0].pitch);
    TEST_ASSERT_EQUAL_UINT8(note1.intensity, recorder.noteCmdList[0].intensity);
    TEST_ASSERT_EQUAL_UINT16(note1.delay, recorder.noteCmdList[0].delay);
    TEST_ASSERT_EQUAL_UINT8(note2.pitch, recorder.noteCmdList[1].pitch);
    TEST_ASSERT_EQUAL_UINT8(note2.intensity, recorder.noteCmdList[1].intensity);
    TEST_ASSERT_EQUAL_UINT16(note2.delay, recorder.noteCmdList[1].delay);
}

void test_isRecFull(void)
{
    noteCmd note = {60, 100, 500};
    noteCmd noteCmdList[1];
    scoreRecorder recorder;
    initScoreRecorder(&recorder, noteCmdList, 1);

    TEST_ASSERT_FALSE(isRecFull(&recorder));
    recordNoteCmd(&recorder, &note);
    TEST_ASSERT_TRUE(isRecFull(&recorder));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_initScoreRecorder);
    RUN_TEST(test_recordNoteCmd);
    RUN_TEST(test_isRecFull);
    return UNITY_END();
}
