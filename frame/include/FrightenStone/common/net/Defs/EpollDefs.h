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
 * @file  : EpollDefs.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/01/18
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Defs_EpollDefs_H__
#define __Frame_Include_FrightenStone_Common_Net_Defs_EpollDefs_H__
#pragma once

#ifndef _WIN32

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/net/Defs/BufferQueueNode.h"
#include "FrightenStone/common/component/Impl/FS_Delegate.h"
#include "FrightenStone/common/net/Defs/FS_NetDefs.h"

#undef __FS_MAX_EVENT_NUM_TO_HANDLE_ONCE_DEF__
#define __FS_MAX_EVENT_NUM_TO_HANDLE_ONCE_DEF__     128                         // ÿ��Ĭ�ϴ��������¼�����

 // epoll�ܼ������¼�������� linux 2.6.8֮��İ汾�������û������,����linux�������ֻ���ڴ��Լ��ļ������������й�ϵ,�Ƕ�̬��
#undef __FS_MAX_EPOLL_EVENT_NUM_LIMIT__
#define __FS_MAX_EPOLL_EVENT_NUM_LIMIT__            10000

#undef __ADD_FS_EPOLL_EVENT_FLAGS_DEF__
#define __ADD_FS_EPOLL_EVENT_FLAGS_DEF__    (EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR | EPOLLOUT)   // ��עepoll�ɶ�,�Ͽ�,�����Լ�ʹ��etģʽ

#undef __ADD_FS_EPOLL_ACCEPT_EVENT_FLAGS_DEF__
#define __ADD_FS_EPOLL_ACCEPT_EVENT_FLAGS_DEF__     (EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR)

#undef __DEL_FS_EPOLL_EVENT_FLAGS_DEF__
#define __DEL_FS_EPOLL_EVENT_FLAGS_DEF__    (EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR | EPOLLOUT)    // �����й�ע���¼����Ƴ�fd

FS_NAMESPACE_BEGIN

class BASE_EXPORT EpollWaitTimeOutType
{
public:
    enum
    {
        BlockNoTimeOut = -1,                // û�г�ʱ�������ȴ�
        WakeupImmediately = 0,              // ����������
    };
};

class BASE_EXPORT EpollDefs
{
public:
    enum
    {
        InvalidFd = -1,         // ��Ч�ļ�������
        EpollError = -1,        // epoll����
    };
};

// class BASE_EXPORT EpollIoType
// {
// public:
//     enum
//     {
//         IO_Accept = 10,
//         IO_Recv,
//         IO_Send,
//     };
// };

struct BASE_EXPORT EpollIoData
{
    OBJ_POOL_CREATE_DEF(EpollIoData);

    EpollIoData():_node(NULL), _data(NULL), _dateLen(0) {}
    ~EpollIoData();

    UInt64 _sessionId = 0;                   // �ỰΨһid
    SOCKET _sock = INVALID_SOCKET;
    union
    {
        BufferQueueNode *_node;         // packet���ڵĶ��нڵ�
    };
    IDelegate<void, Int64> *_callback = NULL; // ���ʱ�Ļص�

    // Ҫ���������
    Byte8 *_data;
    Int64 _dateLen;
};

FS_NAMESPACE_END

#endif

#endif
