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
 * @file  : SystemUtil.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/6/10
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Assist_Utils_Impl_SystemUtil_H__
#define __Base_Common_Assist_Utils_Impl_SystemUtil_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"

FS_NAMESPACE_BEGIN
class BASE_EXPORT FS_String;
struct BASE_EXPORT ProcessMemInfo;
class BASE_EXPORT SystemUtil
{
public:
    // 获取可用的内存大小
    static UInt64 GetAvailPhysMemSize();
    // 获取内存大小
    static UInt64 GetTotalPhysMemSize();
    // 内存使用率
    static ULong GetMemoryLoad();
    // 进程占用内存信息
    static bool GetProcessMemInfo(HANDLE processHandle, ProcessMemInfo &info);

    /* 进程线程 */
    // 获取程序目录
    static Int32 GetProgramPath(bool isCurrentProcess, FS_String &processPath, ULong pid = 0);
    // 获取当前进程名
    static FS_String GetCurProgramName();
    // 创建进程快照（遍历进程相关）
    static HANDLE CreateProcessSnapshot();
    // 获取第一个进程id
    static ULong GetFirstProcessPid(HANDLE &hSnapshot);
    // 获取下一个进程id
    static ULong GetNextProcessPid(HANDLE &hSnapshot);
    // 获取线程id
    static ULong GetCurrentThreadId();
    // 获取进程id
    static Int32 GetCurProcessId();
    // 获取进程句柄
    static HANDLE GetCurProcessHandle();
    // 结束进程
    static Int32 CloseProcess(ULong processId, ULong *lastError = NULL);

    /* 杂项 */
    // 获取当前调用线程所在的cpu编号信息
    static void GetCallingThreadCpuInfo(UInt16 &cpuGroup, Byte8 &cpuNumber);
    // 通过进程ID获取窗口句柄
    static HWND GetWindowHwndByPID(DWORD dwProcessID);
    // 将窗口设置顶层
    static void BringWindowsToTop(HWND curWin);
    // 遍历进程判断某进程是否在进程列表
    static bool IsProcessExist(const FS_String &processName);
    // 弹窗
    static void MessageBoxPopup(const FS_String &title, const FS_String &content);

    /* 控制台 */
    static void LockConsole();
    static void UnlockConsole();
    // 设置控制台颜色
    static Int32 SetConsoleColor(Int32 color);
    // 获取控制台颜色
    static Int32 GetConsoleColor();
    // 输出给控制台
    static void OutputToConsole(const FS_String &outStr);
    // 大小端判断
    static bool IsLittleEndian();
};

FS_NAMESPACE_END

#include "base/common/assist/utils/Impl/SystemUtilImpl.h"

#endif
