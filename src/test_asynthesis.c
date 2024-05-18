#include "unity/unity.h"
#include "asynthesis.h"
#include "note.h"

// Mock implementation for pitchPeriod array for testing purposes
// uint16_t pitchPeriod[128];

void setUp()
{
    // Set up initial conditions for each test case, if needed
    // for (int i = 0; i < 128; i++)
    // {
    //     pitchPeriod[i] = 100 + i; // Example values for testing
    // }
}

void tearDown()
{
    // Clean up after each test case, if needed
}

void test_clearPlayerState_should_reset_player_state()
{
    playerState player;
    player.playerTime = 100;
    player.activatedNoteNum = 5;

    clearPlayerState(&player);

    TEST_ASSERT_EQUAL_INT(0, player.playerTime);
    TEST_ASSERT_EQUAL_INT(0, player.activatedNoteNum);
}

void test_setCommandNote_should_add_note()
{
    playerState player;
    noteCmd note = {60, 100, 500}; // Example note

    clearPlayerState(&player);
    setCommandNote(&player, &note);

    TEST_ASSERT_EQUAL_INT(1, player.activatedNoteNum);
    TEST_ASSERT_EQUAL_INT(60, player.noteList[0].pitch);
    TEST_ASSERT_EQUAL_INT(100, player.noteList[0].intensity);
    TEST_ASSERT_EQUAL_INT(500, player.noteList[0].delay);
}

void test_setCommandNote_should_update_existing_note()
{
    playerState player;
    noteCmd note1 = {60, 100, 500}; // Example note
    noteCmd note2 = {60, 50, 500};  // Updated intensity

    clearPlayerState(&player);
    setCommandNote(&player, &note1);
    setCommandNote(&player, &note2);

    TEST_ASSERT_EQUAL_INT(1, player.activatedNoteNum);
    TEST_ASSERT_EQUAL_INT(60, player.noteList[0].pitch);
    TEST_ASSERT_EQUAL_INT(50, player.noteList[0].intensity); // Updated intensity
}

void test_setCommandNote_should_remove_note()
{
    playerState player;
    noteCmd note1 = {60, 100, 500}; // Example note
    noteCmd note2 = {60, 0, 500};   // Remove note

    clearPlayerState(&player);
    setCommandNote(&player, &note1);
    setCommandNote(&player, &note2);

    TEST_ASSERT_EQUAL_INT(0, player.activatedNoteNum);
}

void test_getOutputIntensityBasic_should_return_correct_intensity()
{
    playerState player;
    noteCmd note = {60, 100, 500}; // Example note

    clearPlayerState(&player);
    setCommandNote(&player, &note);

    uint16_t intensity = getOutputIntensityBasic(&player, 250);
    TEST_ASSERT_EQUAL_INT(1, intensity); // Based on the mock implementation
}

void test_getOutputIntensity_should_return_correct_intensity()
{
    playerState player;
    noteCmd note1 = {60, 100, 500}; // Example note
    noteCmd note2 = {62, 50, 500};  // Another example note

    clearPlayerState(&player);
    setCommandNote(&player, &note1);
    setCommandNote(&player, &note2);

    uint16_t intensity = getOutputIntensity(&player, 250);
    TEST_ASSERT_GREATER_THAN(0, intensity); // Should be greater than 0 based on the mock implementation
}

void test_maintainPlayerState_should_clear_expired_notes()
{
    playerState player;
    noteCmd note = {60, 100, 500}; // Example note

    clearPlayerState(&player);
    setCommandNote(&player, &note);
    player.clearCounter[0] = -1; // Simulate an expired note

    maintainPlayerState(&player);

    TEST_ASSERT_EQUAL_INT(0, player.activatedNoteNum); // The note should be cleared
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_clearPlayerState_should_reset_player_state);
    RUN_TEST(test_setCommandNote_should_add_note);
    RUN_TEST(test_setCommandNote_should_update_existing_note);
    RUN_TEST(test_setCommandNote_should_remove_note);
    RUN_TEST(test_getOutputIntensityBasic_should_return_correct_intensity);
    RUN_TEST(test_getOutputIntensity_should_return_correct_intensity);
    RUN_TEST(test_maintainPlayerState_should_clear_expired_notes);

    return UNITY_END();
}
