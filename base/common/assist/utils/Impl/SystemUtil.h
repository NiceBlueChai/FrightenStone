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
class FS_String;
class BASE_EXPORT SystemUtil
{
public:
    // ��ȡ���õĽ����ڴ��С
    static UInt64 GetProcessAvailMemSize();
    // ��ȡ���̿ռ��ܵ��ڴ��С
    static UInt64 GetProcessTotalSize();
    // ��ȡ����Ŀ¼
    static bool GetProgramPath(bool isCurrentProcess, ULong pid, FS_String &processPath);
    // �������̿��գ�����������أ�
    static HANDLE CreateProcessSnapshot();
    // ��ȡ��һ������id
    static ULong GetFirstProcessPid(HANDLE &hSnapshot);
    // ��ȡ��һ������id
    static ULong GetNextProcessPid(HANDLE &hSnapshot);
//     // ö�ٴ��ڻص�����
//     static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
    // ͨ������ID��ȡ���ھ��
    static HWND GetWindowHwndByPID(DWORD dwProcessID);
};

FS_NAMESPACE_END

#include "base/common/assist/utils/Impl/SystemUtilImpl.h"

#endif
