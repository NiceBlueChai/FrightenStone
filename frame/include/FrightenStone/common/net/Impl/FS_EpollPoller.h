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
 * @file  : FS_EpollPoller.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/01/18
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_FS_EpollPoller_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_FS_EpollPoller_H__

#pragma once

#ifndef _WIN32

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/net/Impl/IFS_BasePoller.h"
#include "FrightenStone/common/net/Defs/PollerDefs.h"
#include "FrightenStone/common/net/Defs/FS_EpollEvMessageBlock.h"
#include "FrightenStone/common/net/Defs/FS_NetDefs.h"

#include "FrightenStone/common/asyn/asyn.h"
#include "FrightenStone/common/objpool/objpool.h"

struct sockaddr_in;

FS_NAMESPACE_BEGIN

class FS_Epoll;
class FS_ThreadPool;
class ConcurrentMessageQueueNoThread;
class IFS_EngineComp;
class IFS_NetEngine;
class FS_CpuInfo;
class MessageQueueNoThread;
class FS_String;

class FS_EpollPoller : public IFS_BasePoller
{
    OBJ_POOL_CREATE_DEF(FS_EpollPoller);
public:
    FS_EpollPoller(IFS_EngineComp *engineComp, Int32 monitorType);
    ~FS_EpollPoller();

public:
    virtual Int32 BeforeStart();
    virtual Int32 Start();
    virtual void AfterStart();
    virtual void WillClose();
    virtual void BeforeClose();
    virtual void Close();

    // ����ģ��iocp,epoll
    virtual IFS_NetModule *GetNetModuleObj();
    FS_Epoll *GetEpollObj();

    // transferpoller����
public:
    bool PushRecvMsg(EpollIoData *ioData);
    bool PushSendMsg(EpollIoData *ioData);
    void CancelRecvIo(UInt64 sessionId, Int32 reason, bool giveRes);
    void CancelSendIo(UInt64 sessionId, Int32 reason, bool giveRes);
    virtual bool OnWillConnect(SOCKET sock, UInt64 sessionId);
    bool IsIoFinish() const;

private:
    // ��������
    void _OnAcceptorEvMonitorPoller(FS_ThreadPool *pool);
    void _OnPreHandleAcceptEvPoller(FS_ThreadPool *pool);

    // �������ݴ���
    void _OnTransferEvMonitorPoller(FS_ThreadPool *pool);
    void _OnPreDistributeEvPoller(FS_ThreadPool *pool);
    void _OnReadPoller(FS_ThreadPool *pool);
    void _OnSendPoller(FS_ThreadPool *pool);

    // ���ݴ������Ϣ����
    void _OnEpollReadableEv(FS_EpollEvMessageBlock *messageBlock);
    void _OnEpollWritableEv(FS_EpollEvMessageBlock *messageBlock);
    void _OnBePostedRecv(FS_EpollEvMessageBlock *messageBlock);
    void _OnBePostedSend(FS_EpollEvMessageBlock *messageBlock);
    void _OnBePostedCancleIoToRecver(FS_EpollEvMessageBlock *messageBlock);
    void _OnBePostedCancleIoToSender(FS_EpollEvMessageBlock *messageBlock);

    // �շ�
    Int32 _OnRecv(SOCKET sock, Byte8 *buff, Int64 buffLen, Int32 &sockErrorCode);
    Int32 _OnSend(SOCKET sock, Byte8 *buff, Int64 buffLen, Int32 &sockErrorCode);

    // �����¼�����
    void _OnRecvRemoveSession(SOCKET sock, UInt64 sessionId);
    void _OnSendRemoveSession(SOCKET sock, UInt64 sessionId);
    void _PostCancelToRecver(UInt64 sessionId, Int32 reason, bool giveRes);
    void _PostCancelToSender(UInt64 sessionId, Int32 reason, bool giveRes);
    void _PostDisconnectToDispatcher(UInt64 sessionId);
    void _PostRecvMsgToDispatcher(UInt64 sessionId, Int64 transferBytes, EpollIoData *ioData);
    void _PostSendSucMsgToDispatcher(UInt64 sessionId, Int64 transferBytes, EpollIoData *ioData);

    void _AddCount(const FS_String &name);

private:
    Int32 _mainType;
    FS_CpuInfo *_cpuInfo;
    FS_ThreadPool *_pool;
    Int32 _maxThreadCnt;
    FS_Epoll *_epoll;
    MessageQueueNoThread *_innerEvThread2MsgPreHandleMq;      // �����¼��߳���Ϣ����ϢԤ�����߳�
    ConditionLocker _preHandlerGuard;
    std::atomic_bool _isInit;
    std::atomic_bool _isWillClose;
    UInt32 _generatorId;

    // ��Ծ�׽���
    std::map<SOCKET, UInt32> _sockRefActiveFlag;    // �׽��ֵĻ�Ծ��־

    // acceptor����
    std::atomic_bool _isEventArrived;

    typedef void (FS_EpollPoller::*MonitorHandler)(FS_ThreadPool *pool);
    static MonitorHandler _monitorHandler[PollerDefs::MonitorType_End];

    typedef void (FS_EpollPoller::*MsgPreHandler)(FS_ThreadPool *pool);
    static MsgPreHandler _msgPreHandler[PollerDefs::MonitorType_End];

    // ��Ϣ���� ע��recvermailbox����sendermailbox������¼�����Ҫ������������Դ����������
    typedef void (FS_EpollPoller::*MessageBlockHandler)(FS_EpollEvMessageBlock *messageBlock);
    static MessageBlockHandler _msgBlockHandler[FS_EpollEvMessageType::End];

    // ��������
    mutable Locker _recvLock;
    std::map<UInt64, bool> _sessionIdRefRecverEvStatus;             // �Ƿ��Ǵ���״̬
    std::map<UInt64, EpollIoData *> _sessionIdRefRecverEvIoData;    // ����������
    MessageQueueNoThread *_recverMailBox;                           // ������Ϣ������

    // ��������
    mutable Locker _sendLock;
    std::map<UInt64, bool> _sessionIdRefSenderEvStatus;             // �Ƿ��Ǵ���״̬
    std::map<UInt64, EpollIoData *> _sessionIdRefSenderIoData;      // ����������
    MessageQueueNoThread *_senderMailBox;                           // ������Ϣ������
};

FS_NAMESPACE_END

#include "FrightenStone/common/net/Impl/FS_EpollPollerImpl.h"

#endif

#endif
