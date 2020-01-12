/*!
 * MIT License
 *
 * Copyright (c) 2019 ericyonng<120453674@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file  : LogData.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/6/13
 * @brief :
 * 
 *
 * 
 */
#ifndef __Frame_Include_FrightenStone_Common_Log_Defs_LogData_H__
#define __Frame_Include_FrightenStone_Common_Log_Defs_LogData_H__
#pragma once
#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/component/Impl/FS_String.h"
#include "FrightenStone/common/component/Impl/Time.h"
#include "FrightenStone/common/memorypool/memorypool.h"
#include "FrightenStone/common/objpool/objpool.h"

FS_NAMESPACE_BEGIN
class FS_String;
class Time;

class BASE_EXPORT LogLevel
{
public:
    enum LevelEnums
    {
        Begin = 0,          // 开始
        Warning = Begin,
        Debug,
        Info,
        Error,

        Crash,
        Net,
        Memleak,
        MemPool,
        ObjPool,
        Sys,
        Any,
        Custom,

        End,
    };

    static const char *GetDescription(Int32 level);
};

struct BASE_EXPORT LogData
{
    OBJ_POOL_CREATE_DEF(LogData);

public:
    Time _logTime;                  // 时间 generate inside
    FS_String _logToWrite;          // 格式化的日志字符串 %s<%s>[%s][%s]line:%d %s
};

FS_NAMESPACE_END

#endif
