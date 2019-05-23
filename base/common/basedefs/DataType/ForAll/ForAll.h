#ifndef __Base_Common_BaseDefs_DataType_ForAll_ForAll_H__
#define __Base_Common_BaseDefs_DataType_ForAll_ForAll_H__
/**
* @file ForAll.h
* @auther Huiya Song <120453674@qq.com>
* @date 2019/04/25
* @brief
*/

#pragma once

// defs ...

#pragma region 基本数据类型
typedef char Byte8;
typedef unsigned char U8;
typedef short Int16;
typedef unsigned short UInt16;
typedef int Int32;
typedef unsigned int UInt32;
typedef long Long;
typedef unsigned long ULong;
typedef long long Int64;
typedef unsigned long long UInt64;
typedef float Float;
typedef double Double;
#pragma endregion

// 缓冲
#define BUFFER_LEN4         4
#define BUFFER_LEN8         8
#define BUFFER_LEN16        16
#define BUFFER_LEN32        32
#define BUFFER_LEN64        64
#define BUFFER_LEN128       128
#define BUFFER_LEN256       256
#define BUFFER_LEN512       512
#define BUFFER_LEN1024      1024
typedef Byte8 BUFFER8[BUFFER_LEN8];
typedef Byte8 BUFFER16[BUFFER_LEN16];
typedef Byte8 BUFFER32[BUFFER_LEN32];
typedef Byte8 BUFFER64[BUFFER_LEN64];
typedef Byte8 BUFFER128[BUFFER_LEN128];
typedef Byte8 BUFFER256[BUFFER_LEN256];
typedef Byte8 BUFFER512[BUFFER_LEN512];
typedef Byte8 BUFFER1024[BUFFER_LEN1024];

class FileCursorOffsetType
{
public:
    enum FILE_CURSOR_POS
    {
        FILE_CURSOR_POS_SET = 0,
        FILE_CURSOR_POS_CUR = 1,
        FILE_CURSOR_POS_END = 2,
    };
};

#endif // !__Base_Common_BaseDefs_DataType_InnerDataType_InnerDataType_H__
