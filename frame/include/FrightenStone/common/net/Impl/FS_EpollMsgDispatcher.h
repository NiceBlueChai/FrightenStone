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
 * @file  : FS_EpollMsgDispatcher.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/01/26
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_FS_EpollMsgDispatcher_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_FS_EpollMsgDispatcher_H__

#pragma once

#ifndef _WIN32

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/net/Impl/IFS_MsgDispatcher.h"
#include "FrightenStone/common/net/Defs/NetCfgDefs.h"
#include "FrightenStone/common/component/Impl/FS_Delegate.h"
#include "FrightenStone/common/net/Defs/HeartBeatComp.h"
#include "FrightenStone/common/net/Defs/FS_NetMessageBlock.h"
#include "FrightenStone/common/net/Defs/FS_EpollEvMessageBlock.h"

#include "FrightenStone/common/asyn/asyn.h"
#include <set>
#include "FrightenStone/common/component/Impl/TimeSlice.h"
#include "FrightenStone/common/objpool/objpool.h"

FS_NAMESPACE_BEGIN

class IFS_Session;
class FS_EpollSession;
class IFS_NetEngine;
class IFS_BusinessLogic;
struct NetEngineTotalCfgs;
struct NetMsg_DataHeader;
struct FS_MessageBlock;
class TimeWheel;
class FS_ThreadPool;
class NetMsgDecoder;
class IMemoryAlloctor;
struct FS_EpollEvMessageBlock;

class BASE_EXPORT FS_EpollMsgDispatcher : public IFS_MsgDispatcher
{
    OBJ_POOL_CREATE_DEF(FS_EpollMsgDispatcher);
public:
    FS_EpollMsgDispatcher(IFS_NetEngine *netEngine, UInt32 compId);
    virtual ~FS_EpollMsgDispatcher();

public:
    virtual Int32 BeforeStart(const NetEngineTotalCfgs &cfgs);
    virtual Int32 Start();
    virtual void AfterStart();
    virtual void WillClose();
    virtual void BeforeClose();
    virtual void Close();
    virtual void AfterClose();
    virtual void BindBusinessLogic(IFS_BusinessLogic *businessLogic);
    virtual Int32 GetSessionCnt() const;
    virtual TimeWheel *GetTimeWheel();
    virtual const IFS_Session *GetSession(UInt64 sessionId) const;
    // ������Ϣmsg���ڲ��ᱻ������������
    virtual void SendData(UInt64 sessionId, NetMsg_DataHeader *msg);
    virtual void SendData(UInt64 sessionId, const Byte8 *msgBuffer, UInt64 msgLen);
    virtual void CloseSession(UInt64 sessionId);

private:
    // ҵ���߳�
    void _OnBusinessProcessThread(FS_ThreadPool *pool);
    void _RunAFrame(bool hasMsg);
    // �������
    void _CheckHeartbeat();
    // ҵ����Ϣ����
    void _OnBusinessProcessing();
    // ͳһ����Ͷ����Ϣ
    void _PostEvents();
    // �ӳ��Ƴ��Ͽ��ĻỰ
    void _WillRemoveSessions();
    // �������лỰ���ȴ�transferֹͣ����
    void _CancelAllSessionIosAndWaitForTransferIoFinish();
    // �������лỰ���ȴ�transferֹͣ����
    void _ClearAllSessions();
    // �ȴ�transferֹͣ
    // ֪ͨ����ϵͳ�ط� ֻ����ҵ���߳���
    void _CloseLogicSvc();

    /* �����Ự */
    bool _DoPostRecv(FS_EpollSession *session);
    bool _DoPostSend(FS_EpollSession *session);
    void _RemoveSessionGracefully(FS_EpollSession *session);
    void _RemoveFromPostRecvQueue(FS_EpollSession *session);
    void _RemoveFromPostSendQueue(FS_EpollSession *session);
    void _UpdateSessionHeartbeat(IFS_Session *session);

    /* ���� */
private:
    FS_EpollSession *_GetSession(UInt64 sessionId);
    const FS_EpollSession *_GetSession(UInt64 sessionId) const;
    void _PrintAlloctorOccupiedInfo();

    /* ��Ϣ���д��� */
private:
    // �յ������
    void _OnMsgBlockRecvData(FS_EpollEvMessageBlock *msgBlock);
    // ���ͳɹ��¼�
    void _OnMsgBlockSendSuc(FS_EpollEvMessageBlock *msgBlock);
    // �Ự����
    void _OnMsgBlockNetSessionConnected(FS_EpollEvMessageBlock *msgBlock);
    // �Ự�Ͽ�
    // void _OnMsgBlockNetSessionDisconnected(FS_EpollEvMessageBlock *msgBlock);
    // ���������Ӳ������,�����������������Ӷ��ѳɹ�
    void _OnMsgBlockConnectorConnectOpFinish(FS_EpollEvMessageBlock *msgBlock);

    /* �����¼� */
private:
    // �Ự�Ͽ�
    void _OnSessionDisconnectedNoFree(FS_EpollSession *session);
    // �Ự�Ͽ�
    void _OnSessionDisconnected(FS_EpollSession *session);
    // �Ự����
    void _OnSessionConnected(EpollEvSessionWillConnectMsg *connectedMsg);
    // Զ�˻Ự��Ϣ����
    void _OnMsgRecv(EpollRecvDataMessageBlock *recvMsg);
    // ��Ϣ���ͳɹ�
    void _OnMsgSendSuc(EpollSendSucMessageBlock *sendSucMsg);
    // �Ự��Ϣ����
    void _OnSessionMsgHandle(FS_EpollSession *session);

private:
    void _MaskCloseSession(FS_EpollSession *session);

private:
    DispatcherCfgs *_cfgs;
    std::atomic<bool> _isClose;
    FS_ThreadPool *_pool;

    /* ҵ�����Դ */
    TimeWheel *_timeWheel;
    IFS_BusinessLogic *_logic;
    std::map<UInt64, std::list<IDelegate<void, UInt64> *>> _sessionIdRefUserDisconnected;
    std::vector<std::list<FS_MessageBlock *> *> *_recvMsgBlocks;       // ��Ҫת���� FS_NetMsgBufferBlock

    /* �Ự */
    IMemoryAlloctor *_sessionBufferAlloctor;
    IDelegate<void> *_printAlloctorOccupiedInfo;
    UInt64 _transferThreadId;

    std::atomic<Int32> _curSessionQuatity;
    std::map<UInt64, FS_EpollSession *> _sessions;  // key:sessionId
    Time _curTime;
    std::set<IFS_Session *, HeartBeatComp> _sessionHeartbeatQueue;  // ��������
    std::set<FS_EpollSession *> _toPostRecv;
    std::set<FS_EpollSession *> _toPostSend;
    std::set<FS_EpollSession *> _toRemove;

    /* Э����� */
    NetMsgDecoder *_msgDecoder;

    // ��Ϣ���д�����
    typedef void (FS_EpollMsgDispatcher::*MessageQueueHandler)(FS_EpollEvMessageBlock *msgBlock);
    static MessageQueueHandler _msgBlockHandler[FS_EpollEvMessageType::End];
};

FS_NAMESPACE_END

#include "FrightenStone/common/net/Impl/FS_EpollMsgDispatcherImpl.h"

#endif

#endif
