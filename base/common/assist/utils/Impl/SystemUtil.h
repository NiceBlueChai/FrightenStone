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
    // ��ȡ���õ��ڴ��С
    static UInt64 GetAvailPhysMemSize();
    // ��ȡ�ڴ��С
    static UInt64 GetTotalPhysMemSize();
    // �ڴ�ʹ����
    static ULong GetMemoryLoad();
    // ����ռ���ڴ���Ϣ
    static bool GetProcessMemInfo(HANDLE processHandle, ProcessMemInfo &info);

    /* �����߳� */
    // ��ȡ����Ŀ¼
    static Int32 GetProgramPath(bool isCurrentProcess, FS_String &processPath, ULong pid = 0);
    // ��ȡ��ǰ������
    static FS_String GetCurProgramName();
    // �������̿��գ�����������أ�
    static HANDLE CreateProcessSnapshot();
    // ��ȡ��һ������id
    static ULong GetFirstProcessPid(HANDLE &hSnapshot);
    // ��ȡ��һ������id
    static ULong GetNextProcessPid(HANDLE &hSnapshot);
    // ��ȡ�߳�id
    static ULong GetCurrentThreadId();
    // ��ȡ����id
    static Int32 GetCurProcessId();
    // ��ȡ���̾��
    static HANDLE GetCurProcessHandle();
    // ��������
    static Int32 CloseProcess(ULong processId, ULong *lastError = NULL);

    /* ���� */
    // ��ȡ��ǰ�����߳����ڵ�cpu�����Ϣ
    static void GetCallingThreadCpuInfo(UInt16 &cpuGroup, Byte8 &cpuNumber);
    // ͨ������ID��ȡ���ھ��
    static HWND GetWindowHwndByPID(DWORD dwProcessID);
    // ���������ö���
    static void BringWindowsToTop(HWND curWin);
    // ���������ж�ĳ�����Ƿ��ڽ����б�
    static bool IsProcessExist(const FS_String &processName);
    // ����
    static void MessageBoxPopup(const FS_String &title, const FS_String &content);

    /* ����̨ */
    static void LockConsole();
    static void UnlockConsole();
    // ���ÿ���̨��ɫ
    static Int32 SetConsoleColor(Int32 color);
    // ��ȡ����̨��ɫ
    static Int32 GetConsoleColor();
    // ���������̨
    static void OutputToConsole(const FS_String &outStr);
};

FS_NAMESPACE_END

#include "base/common/assist/utils/Impl/SystemUtilImpl.h"

#endif
