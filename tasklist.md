# Task 1

## 寻找合适曲谱并编码

1. 选择一首简单且广为人知的曲谱，如《小星星》、《两只老虎》等，当然太皮的也不是不行，需要考虑每秒钟只能传约60个音符。
2. 用类似MIDI音符指令的方式编码曲谱，如：`{60, 100, 500}`表示音高60、强度100、延时500ms。
3. 音高表和MIDI音符相同，可以参见[音高表](https://www.cnblogs.com/520mus/p/15665271.html)。

|      | C  | C# | D  | D# | E  | F  | F# | G  | G# | A  | A# | B  |
|------|----|----|----|----|----|----|----|----|----|----|----|----|
|  0   | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  | 8  | 9  | 10 | 11 |
|  1   | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |
|  2   | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 |
|  3   | 36 | 37 | 38 | 39 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 |
|  4   | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 |
|  5   | 60 | 61 | 62 | 63 | 64 | 65 | 66 | 67 | 68 | 69 | 70 | 71 |
|  6   | 72 | 73 | 74 | 75 | 76 | 77 | 78 | 79 | 80 | 81 | 82 | 83 |
|  7   | 84 | 85 | 86 | 87 | 88 | 89 | 90 | 91 | 92 | 93 | 94 | 95 |
|  8   | 96 | 97 | 98 | 99 |100 |101 |102 |103 |104 |105 |106 |107 |
|  9   |108 |109 |110 |111 |112 |113 |114 |115 |116 |117 |118 |119 |
|  10  |120 |121 |122 |123 |124 |125 |126 |127 |    |    |    |    |

4. 强度在0-127，0表示停音。
5. 延时单位为ms，0表示立即生效，延时是相对于上一个指令的时间。
6. 如果可以，最好写一个自动转化脚本，脚本接受标准乐谱文件并输出编码后的曲谱。如果无法实现，至少提供一份具体的手动编码曲谱。

### 例子

```plaintext
{60, 100, 500}
{62, 100, 0}
{64, 100, 0}
{65, 100, 0}
{60, 0, 500}
```

表示：先延时500ms，然后，60、62、64、65音符同时响起，500ms后60音符停音。

# Task 2

## 实现一个函数

### 函数原型

```c
uint8_t analyzeCommand(char *command);
```

### 输入

- `command`是一个以'\0'结尾的字符串，表示一个控制指令。

### 输出

- 函数返回一个8位无符号整数，表示解析后的指令。

### 命令字节格式

- **Transmitter Command**
  - `0b11000xxx`: Set song to play 对应字符串“play %d”，%d表示一个整数，为曲谱编号，应写入返回值的低3位。
  - `0b11001xxx`: pause song 对应字符串“pause”
  - `0b11010xxx`: resume song 对应字符串“resume”
  - `0b11011xxx`: Turn on/off the FM transmitter 对应字符串“transmitter %d”，%d表示一个整数，为0或1，on为001，off为000。
  - `0b11100xxx`: Turn on/off the buzzer 对应字符串“buzzer %d”，%d表示一个整数，为0或1，on为001，off为000。

- **Receiver Command**
  - `0b10000xxx`: Record on/off 对应字符串“record %d”，%d表示一个整数，为0或1，on为001，off为000。
  - `0b10001xxx`: Play the recorded song 对应字符串“playrecord”
  - `0b10010xxx`: Play from FM receiver 对应字符串“playfm”

- **Error Command**
  - `0b00111111`: Error Command 当`command`不符合上述格式时，或者`command`为`NULL`时，或者`command`只有部分符合格式时。
  - `0b00000000`: No Command

### 输入示例

- `"play 1"` -> `0b11000001`
- `"pause"` -> `0b11001000`
- `"transmitter 1"` -> `0b11011001`
- `"buzzer 0"` -> `0b11100000`
- `"record 1"` -> `0b10000001`

### 错误示例

- `"play"` -> `0b00111111`
- `"transmit"` -> `0b00111111`
- `NULL` -> `0b00111111`

# Task 3

Finished

## `scoreRec` 库文档

### 概述

`scoreRec` 库用于管理和记录音乐指令。该库定义了一个 `scoreRecorder` 结构体，用于保存音符指令列表，以及一系列函数来初始化、记录和操作这些指令。

### 数据结构

#### `scoreRecorder`

```c
typedef struct {
    noteCmd *noteCmdList; // 音符指令列表的指针
    uint32_t maxSize;     // 音符指令列表的最大容量
    uint32_t cursor;      // 当前指令的索引
} scoreRecorder;
```

- `noteCmdList`: 指向 `noteCmd` 结构体数组的指针，用于存储音符指令。
- `maxSize`: 音符指令列表的最大容量。
- `cursor`: 当前记录位置的索引，指示下一个要记录的位置。

### 函数接口

#### `void initScoreRecorder(scoreRecorder *rec, noteCmd *noteCmdList, uint32_t maxSize)`

初始化 `scoreRecorder` 结构体。

- **参数**
  - `rec`: 指向 `scoreRecorder` 结构体的指针。
  - `noteCmdList`: 用于存储音符指令的 `noteCmd` 数组的指针。
  - `maxSize`: `noteCmdList` 数组的最大容量。

- **示例**

```c
noteCmd myNoteCmdList[100];
scoreRecorder myRecorder;
initScoreRecorder(&myRecorder, myNoteCmdList, 100);
```

#### `void recordNoteCmd(scoreRecorder *rec, noteCmd *noteCmd)`

向 `scoreRecorder` 中记录一个新的音符指令。

- **参数**
  - `rec`: 指向 `scoreRecorder` 结构体的指针。
  - `noteCmd`: 要记录的 `noteCmd` 指令。

- **注意**
  - 如果记录器已满，该函数不会记录新的指令。

- **示例**

```c
noteCmd newNote = {60, 100, 500};
recordNoteCmd(&myRecorder, &newNote);
```

#### `bool isRecFull(scoreRecorder *rec)`

检查 `scoreRecorder` 是否已满。

- **参数**
  - `rec`: 指向 `scoreRecorder` 结构体的指针。

- **返回值**
  - `true` 如果记录器已满。
  - `false` 如果记录器未满。

- **示例**

```c
if (isRecFull(&myRecorder)) {
    // 处理记录器已满的情况
}
```

#### `void resetRec(scoreRecorder *rec)`

重置 `scoreRecorder`，清空所有记录的音符指令。

- **参数**
  - `rec`: 指向 `scoreRecorder` 结构体的指针。

- **示例**

```c
resetRec(&myRecorder);
```

#### `bool isCmdLeft(scoreRecorder *rec)`

检查是否还有未读取的音符指令。

- **参数**
  - `rec`: 指向 `scoreRecorder` 结构体的指针。

- **返回值**
  - `true` 如果有未读取的音符指令。
  - `false` 如果没有未读取的音符指令。

- **示例**

```c
if (isCmdLeft(&myRecorder)) {
    // 处理有未读取指令的情况
}
```

#### `void getNoteCmd(scoreRecorder *rec, noteCmd *noteCmd)`

获取下一个未读取的音符指令，并将指针移动到下一条指令。

- **参数**
  - `rec`: 指向 `scoreRecorder` 结构体的指针。
  - `noteCmd`: 指向 `noteCmd` 结构体的指针，用于存储读取到的指令。

- **注意**
  - 调用此函数前应确保 `isCmdLeft(rec)` 返回 `true`。

- **示例**

```c
noteCmd currentNote;
if (isCmdLeft(&myRecorder)) {
    getNoteCmd(&myRecorder, &currentNote);
}
```

### 使用示例

```c
#include "scoreRec.h"
#include "note.h"

int main() {
    // 定义音符指令列表和记录器
    noteCmd myNoteCmdList[100];
    scoreRecorder myRecorder;

    // 初始化记录器
    initScoreRecorder(&myRecorder, myNoteCmdList, 100);

    // 记录一些音符指令
    noteCmd note1 = {60, 100, 500};
    recordNoteCmd(&myRecorder, &note1);

    noteCmd note2 = {62, 100, 500};
    recordNoteCmd(&myRecorder, &note2);

    // 检查记录器是否已满
    if (isRecFull(&myRecorder)) {
        // 处理记录器已满的情况
    }

    // 重置记录器
    resetRec(&myRecorder);

    // 检查是否有未读取的指令并获取
    if (isCmdLeft(&myRecorder)) {
        noteCmd currentNote;
        getNoteCmd(&myRecorder, &currentNote);
    }

    return 0;
}
```

### 注意事项

- 在调用 `recordNoteCmd` 前，确保记录器未满。
- 在调用 `getNoteCmd` 前，确保有未读取的指令。
- 使用 `resetRec` 重置记录器后，所有记录的指令将被清除。

### 依赖

`note.h` 文件定义了 `noteCmd`

结构体。确保在使用本库前包含该文件。

# Task 4

## `scoreBuf` 库文档

### 概述

`scoreBuf` 库用于管理和处理音符指令的循环缓冲区。它提供了一个 `scoreBuffer` 结构体来存储音符指令，并包含一系列函数来操作缓冲区，包括更新时间计数器、检查缓冲区状态、添加和获取音符指令。

### 数据结构

#### `scoreBuffer`

- `buffer`: 固定大小的音符指令缓冲区，用于存储音符指令。
- `head`: 缓冲区头指针，指示下一个可用位置用于插入新指令。
- `tail`: 缓冲区尾指针，指示下一个要读取的指令位置。
- `timeSinceLastCmd`: 自上次指令以来经过的时间，以某种格式记录。

### 函数接口

#### `void updateBufCounter(scoreBuffer *buf)`

更新缓冲区计数器，记录自上次指令以来经过的时间。

#### `uint8_t getBufRemainingSize(scoreBuffer *buf)`

获取缓冲区剩余空间大小。

#### `bool isBufFull(scoreBuffer *buf)`

检查缓冲区是否已满。

#### `void addNoteToBuf(scoreBuffer *buf, noteCmd *note)`

将音符指令添加到缓冲区。

#### `bool isCmdAvail(scoreBuffer *buf)`

检查是否有可用的指令。即检查最新的指令的延时是否小于等于 `timeSinceLastCmd`。

#### `void getCmdFromBuf(scoreBuffer *buf, noteCmd *note)`

从缓冲区中获取指令，并将其存储在给定的 `noteCmd` 结构体中。之后`head`指针移动到下一个指令,并且更新时间计数器`timeSinceLastCmd`。

### 注意事项

- `updateBufCounter` 函数用于更新时间计数器，以记录自上次指令以来经过的时间。
- `head` 和 `tail` 指针分别用于插入和读取指令，请确保了解它们的功能，并正确使用它们。
- 在使用缓冲区函数时，请注意检查缓冲区是否已满或是否有可用的指令，以避免意外行为。
