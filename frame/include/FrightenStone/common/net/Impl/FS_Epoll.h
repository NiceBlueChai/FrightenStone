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
 * @file  : FS_Epoll.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/1/17
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_FS_Epoll_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_FS_Epoll_H__

#pragma once

#ifndef _WIN32

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/net/Impl/IFS_NetModule.h"
#include "FrightenStone/common/net/Defs/EpollDefs.h"
#include "FrightenStone/common/objpool/objpool.h"

#include <unistd.h>
#include <sys/epoll.h>

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_Epoll : public IFS_NetModule
{
    OBJ_POOL_CREATE_ANCESTOR(FS_Epoll);
public:
    FS_Epoll();
    ~FS_Epoll();

public:
    Int32 Create();
    Int32 Destroy();
    bool IsDestroy() const;
    // opType:EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
    Int32 Ctl(Int32 opType, Int32 fd, epoll_event *ev);
    // Int32 AddEvent(Int32 newFd, UInt32 eventFlags = __ADD_FS_EPOLL_EVENT_FLAGS_DEF__);
    Int32 AddEvent(Int32 newFd, UInt64 sessionId, UInt32 eventFlags);
    Int32 AddEvent(Int32 newFd, UInt32 eventFlags);
    Int32 ModifyEvent(Int32 modFd, UInt64 sessionId, UInt32 eventFlags);
    Int32 DelEvent(Int32 delFd, UInt64 sessionId, UInt32 eventFlagsToDel = __DEL_FS_EPOLL_EVENT_FLAGS_DEF__);
    // 0��ʾ��������,-1��ʾ�����ȴ�,>0��ʾ��ʱʱ�� Wait ���ص���epoll_wait ���ص�ֵ������status�е�״̬��
    Int32 Wait(Int32 millisecondsToWait = EpollWaitTimeOutType::BlockNoTimeOut);
    epoll_event *GetEvs();
    Int32 GetEvFd()const;

private:
    epoll_event *_evPtr;
    // epoll_event ��data��Ա��һ��������
    epoll_event _ev[__FS_MAX_EVENT_NUM_TO_HANDLE_ONCE_DEF__];   // epoll�������¼�������    ���Ա��������¼�
    std::atomic<Int32> _evFd;                                                // epoll�ļ�������
};

FS_NAMESPACE_END

#include "FrightenStone/common/net/Impl/FS_EpollImpl.h"

#endif

#endif
