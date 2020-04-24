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
 * @file  : FS_EpollSession.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/01/31
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_FS_EpollSession_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_FS_EpollSession_H__

#pragma once

#ifndef _WIN32

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/net/Impl/IFS_Session.h"
#include "FrightenStone/common/net/Defs/FS_NetDefs.h"

FS_NAMESPACE_BEGIN

class FS_EpollPoller;
class FS_Epoll;
class FS_EpollBuffer;
struct EpollIoData;
class IMemoryAlloctor;

class BASE_EXPORT FS_EpollSession : public IFS_Session
{
    OBJ_POOL_CREATE_DEF(FS_EpollSession);

public:
    FS_EpollSession(UInt64 sessionId
                    , UInt32 transferCompId
                    , UInt32 acceptorCompId
                    , SOCKET sock
                    , const sockaddr_in *addrInfo
                    , IMemoryAlloctor *memAlloctor
                    , Int64 heartbeatIntervalMicroSeconds);
    ~FS_EpollSession();

    // ����
public:
    virtual void Close();
    void ResetPostRecvMask();
    void ResetPostSendMask();
    void SetRecvIoCanceled();
    void SetSendIoCanceled();
    void EnableDisconnect();
    void Bind(FS_EpollPoller *transferPoller);
    virtual FS_String ToString() const;

    FS_EpollBuffer *CastToRecvBuffer();

    // post
    Int32 PostRecv();   // ��Ҫ�ж�canpost,�Լ�ispostrecv
    Int32 PostSend();
    void CancelRecvIoAndMaskClose(Int32 reason, bool giveRes = false);
    void CancelSendIoAndMaskClose(Int32 reason, bool giveRes = false);

    // �ͻ�������
    virtual void OnConnect();

    // ״̬
public:
    bool IsPostIoChange() const;
    bool IsPostSend() const;
    bool IsPostRecv() const;
    bool IsRecvIoCanceled() const;
    bool IsSendIoCaceled() const;
    bool CanPost() const;
    // ��Ҫ����session��Ҫ�ŵ��ӳٶ����У�����Ҫ��closesocketȻ��ȵ�����io���
    virtual bool CanDestroy() const;
    virtual bool CanDisconnect() const;

    // �¼�
public:
    void OnSendSuc(Int64 transferBytes, EpollIoData *ioData);
    void OnRecvSuc(Int64 transferBytes, EpollIoData *ioData);

private:
    bool _isPostRecv;
    bool _isPostSend;
    bool _isRecvIoCanceled;
    bool _isSendIoCanceled;
    FS_EpollPoller *_transferPoller;
    FS_Epoll *_epoll;
};
FS_NAMESPACE_END

#include "FrightenStone/common/net/Impl/FS_EpollSessionImpl.h"

#endif


#endif
