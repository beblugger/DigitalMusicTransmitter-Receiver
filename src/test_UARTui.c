// test_analyzeCommand.c
#include "unity/unity.h"
#include "UARTui.h"

// 包含要测试的函数声明

void setUp(void)
{
}

void tearDown(void)
{
}
// 测试用例1：有效输入
void test_analyzeCommand_validInput(void)
{
    char command1[] = "play 1";
    char command2[] = "pause";
    char command3[] = "transmitter 1";
    char command4[] = "buzzer 0";
    char command5[] = "record 1";
    char command6[] = "playrecord";
    char command7[] = "playfm";

    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b11000001, analyzeCommand(command1), "Valid input test case 1 failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b11001000, analyzeCommand(command2), "Valid input test case 2 failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b11011001, analyzeCommand(command3), "Valid input test case 3 failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b11100000, analyzeCommand(command4), "Valid input test case 4 failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b10000001, analyzeCommand(command5), "Valid input test case 5 failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b10001000, analyzeCommand(command6), "Valid input test case 6 failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b10010000, analyzeCommand(command7), "Valid input test case 7 failed");
}

// 测试用例2：无效输入
void test_analyzeCommand_invalidInput(void)
{
    char *invalidCommand1 = NULL;
    char invalidCommand2[] = "";
    char invalidCommand3[] = "invalid";
    char invalidCommand4[] = "play";
    char invalidCommand5[] = "pause 1";

    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b00111111, analyzeCommand(invalidCommand1), "Invalid input test case 1 failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b00111111, analyzeCommand(invalidCommand2), "Invalid input test case 2 failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b00111111, analyzeCommand(invalidCommand3), "Invalid input test case 3 failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b00111111, analyzeCommand(invalidCommand4), "Invalid input test case 4 failed");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0b00111111, analyzeCommand(invalidCommand5), "Invalid input test case 5 failed");
}

// 主函数
int main()
{
    UNITY_BEGIN();
    // 运行所有测试
    RUN_TEST(test_analyzeCommand_validInput);
    RUN_TEST(test_analyzeCommand_invalidInput);
    return UNITY_END();
}
