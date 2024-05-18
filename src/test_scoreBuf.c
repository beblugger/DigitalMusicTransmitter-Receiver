#include "scoreBuf.h"
#include <string.h>
#include "unity\unity.h"
void setUp(void)
{
}

void tearDown(void)
{
}

// 测试 scoreBuf 库中的函数
void test_scoreBuf_operations(void)
{
    // 创建 scoreBuffer 实例
    scoreBuffer buf;
    memset(&buf, 0, sizeof(buf)); // 初始化缓冲区

    // 定义测试音符指令
    noteCmd testNote = {60, 100, 500};

    // 测试更新缓冲区计数器函数
    updateBufCounter(&buf);
    TEST_ASSERT_EQUAL(1, buf.timeSinceLastCmd);

    // 测试获取缓冲区剩余空间大小函数
    TEST_ASSERT_EQUAL(SCORE_BUFFER_MASK, getBufRemainingSize(&buf));

    // 测试检查缓冲区是否已满函数
    TEST_ASSERT_FALSE(isBufFull(&buf));

    // 测试将音符指令添加到缓冲区函数
    addNoteToBuf(&buf, &testNote);
    TEST_ASSERT_EQUAL(0, buf.head);
    TEST_ASSERT_EQUAL(1, buf.tail);

    // 测试检查是否有可用的指令函数
    TEST_ASSERT_FALSE(isCmdAvail(&buf));

    for (int i = 0; i < 503; i++)
    {
        updateBufCounter(&buf);
    }

    // 测试从缓冲区中获取指令函数
    noteCmd retrievedNote;
    getCmdFromBuf(&buf, &retrievedNote);
    TEST_ASSERT_EQUAL(60, retrievedNote.pitch);
    TEST_ASSERT_EQUAL(100, retrievedNote.intensity);
    TEST_ASSERT_EQUAL(500, retrievedNote.delay);
    TEST_ASSERT_EQUAL(1, buf.head);
    TEST_ASSERT_EQUAL(1, buf.tail);
    TEST_ASSERT_EQUAL(4, buf.timeSinceLastCmd);
    buf.head = 4;
    buf.tail = 3;
    TEST_ASSERT_EQUAL(0, getBufRemainingSize(&buf));
}

// Unity 测试程序入口
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_scoreBuf_operations);
    return UNITY_END();
}
