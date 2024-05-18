/*
Task1:
寻找合适曲谱并编码：
1. 选择一首简单且广为人知的曲谱，如《小星星》、《两只老虎》等，当然太皮的也不是不行，需要考虑每秒钟只能传约60个音符
2. 用类似MIDI音符指令的方式编码曲谱，如：{60, 100, 500}表示音高60、强度100、延时500ms。
3. 音高表和MIDI音符相同，可以参见https://www.cnblogs.com/520mus/p/15665271.html等。
4. 强度在0-127，0表示停音。
5. 延时单位为ms， 0表示立即生效，延时是相对于上一个指令的时间。
6. 如果可以，最好写一个自动转化脚本，脚本接受标准乐谱文件并输出编码后的曲谱。如果无法实现，至少提供一份具体的手动编码曲谱。
例子：
{60, 100, 500}
{62, 100, 0}
{64, 100, 0}
{65, 100, 0}
{60, 0, 500}
表示：先延时500ms，然后，60、62、64、65音符同时响起，500ms后60音符停音。
*/

/*
实现一个函数：
uint8_t analyzeCommand(char *command);
command是一个以'\0'结尾的字符串，表示一个控制指令。
函数返回一个8位无符号整数，表示解析后的指令：

命令字节格式：
0b11xxxxx: Transmitter Command
0b10xxxxx: Receiver Command
0b00xxxxx: Error Command/No Command

Transmitter Command:
0b11000xxx: Set song to play                        对应字符串“play %d”，%d表示一个整数，为曲谱编号，应写入返回值的低3位。
0b11001xxx: pause song                              对应字符串“pause”
0b11010xxx: resume song                             对应字符串“resume”
0b11011xxx: Turn on/off the FM transmitter          对应字符串“transmitter %d”，%d表示一个整数，为0或1，on为001，off为000。
0b11100xxx: Turn on/off the buzzer                  对应字符串“buzzer %d”，%d表示一个整数，为0或1，on为001，off为000。

Receiver Command:
0b10000xxx: Record on/off                           对应字符串“record %d”，%d表示一个整数，为0或1，on为001，off为000。
0b10001xxx: Play the recorded song                  对应字符串“playrecord”
0b10010xxx: Play from FM receiver                   对应字符串“playfm”

Error Command:
0b00111111: Error Command                           当command不符合上述格式时，或者command为NULL时，或者command只有部分符合格式时。
0b00000000: No Command

输入示例：
"play 1" -> 0b11000001
"pause" -> 0b11001000
"transmitter 1" -> 0b11011001
"buzzer 0" -> 0b11100000
"record 1" -> 0b10000001

错误示例：
"play" -> 0b00111111
"transmit" -> 0b00111111
NULL -> 0b00111111
*/
