/*!
 * MIT License
 *
 * Copyright (c) 2019 Eric Yonng<120453674@qq.com>
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
 * @file  : PollerDefs.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2019/12/29
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Defs_PollerDefs_H__
#define __Frame_Include_FrightenStone_Common_Net_Defs_PollerDefs_H__
#pragma once

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/component/Impl/FS_String.h"

// poller��������ʱʱ��
#undef __FS_POLLER_MONITOR_TIME_OUT_INTERVAL__ 
#define __FS_POLLER_MONITOR_TIME_OUT_INTERVAL__     20

FS_NAMESPACE_BEGIN

class BASE_EXPORT PollerDefs
{
public:
    enum PollerMoniterType
    {
        MonitorType_Begin = 0,              // ��Ч����
        MonitorType_IocpAcceptor = 1,       // iocpacceptor
        MonitorType_IocpTransfer = 2,       // iocptransfer
        MonitorType_EpollAcceptor = 3,      // epollacceptor
        MonitorType_EpollTransfer = 4,      // epolltransfer
        MonitorType_End,                    // ����
    };

    enum PollerEvClassificationProcessType
    {
        Class_None = 0,                     // ��Ч
        Class_RecvEvPoller = 1,             // ���ڴ��������� epoll�����Ŀɶ��¼�����dispatcher�׹����Ķ�ȡ�����¼�
        Class_SendEv = 2,                   // ���ڴ�������� epoll�����Ŀ�д�¼�����dispatcher�׹����ķ��������¼�
        Class_End,
    };

    static FS_String GetMoniterTypeStr(Int32 monitorType);

    static FS_String _unknownType;
};

class BASE_EXPORT SockEvFlagType
{
public:
    enum
    {
        Readable = 0,           // �ɶ�
        Writable = 1,           // ��д
    };

    static bool IsReadable(UInt32 flag);
    static bool IsWritable(UInt32 flag);
    static void SetFlag(UInt32 &flag, Int32 flagType);
    static void ClearFlag(UInt32 &flag, Int32 flagType);
};

FS_NAMESPACE_END

#endif
