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
 * @file  : FS_IocpMsgTransfer.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */

#include "stdafx.h"
#include "base/common/net/Impl/FS_IocpMsgTransfer.h"
#include "base/common/net/Impl/IFS_Session.h"
#include "base/common/net/Impl/FS_IocpSession.h"
#include "base/common/net/Defs/FS_IocpBuffer.h"
#include "base/common/net/Defs/IocpDefs.h"
#include "base/common/net/Impl/FS_Iocp.h"
#include "base/common/net/protocol/protocol.h"
#include "base/common/net/Impl/FS_SessionFactory.h"
#include "base/common/net/Defs/BriefSessionInfo.h"

#include "base/common/status/status.h"
#include "base/common/component/Impl/FS_ThreadPool.h"
#include "base/common/log/Log.h"
#include "base/common/crashhandle/CrashHandle.h"
#include "base/common/socket/socket.h"
#include "base/common/net/Impl/FS_Addr.h"
#include "base/common/component/Impl/MessageQueue/MessageQueue.h"
#include "base/common/memleak/memleak.h"

FS_NAMESPACE_BEGIN
FS_IocpMsgTransfer::FS_IocpMsgTransfer(Int32 id)
    :_threadPool(NULL)
    ,_iocp(NULL)
    ,_ioEvent(NULL)
    ,_sessionCnt{0}
    ,_hasNewSessionLinkin(false)
    // ,_isSendCacheDirtied(false)
    ,_id(id)
    ,_messageQueue(NULL)
    ,_senderMsgs(NULL)
    ,_senderMessageQueue(NULL)
    ,_generatorId(0)
    ,_recvMsgList(NULL)
    ,_sessionBufferAlloctor(NULL)
    ,_canCreateNewNodeForAlloctor(true)
    ,_maxAlloctorBytes(0)
    ,_curAlloctorOccupiedBytes(0)
    ,_updateAlloctorOccupied(NULL)
    ,_transferThreadId(0)
    ,_printAlloctorOccupiedInfo(NULL)
{
/*        _CrtMemCheckpoint(&s1);*/
}

FS_IocpMsgTransfer::~FS_IocpMsgTransfer()
{
    Fs_SafeFree(_threadPool);
    Fs_SafeFree(_iocp);
    Fs_SafeFree(_ioEvent);
    STLUtil::DelListContainer(*_senderMsgs);
    STLUtil::DelListContainer(*_recvMsgList);
    Fs_SafeFree(_senderMsgs);
    Fs_SafeFree(_recvMsgList);
    Fs_SafeFree(_sessionBufferAlloctor);
    Fs_SafeFree(_updateAlloctorOccupied);
    Fs_SafeFree(_printAlloctorOccupiedInfo);

//     _CrtMemCheckpoint(&s2);
//     if(_CrtMemDifference(&s3, &s1, &s2))
//         _CrtMemDumpStatistics(&s3);
}

Int32 FS_IocpMsgTransfer::BeforeStart()
{
    size_t blockAmount = g_SvrCfg->GetPrepareBufferPoolCnt();
    if(blockAmount <= 0)
    {
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("prepare buffer pool block amount is zero."));
        return StatusDefs::IocpMsgTransfer_CfgError;
    }
    _maxAlloctorBytes = g_SvrCfg->GetMaxMemPoolBytesPerTransfer();
    _updateAlloctorOccupied = DelegatePlusFactory::Create(this, &FS_IocpMsgTransfer::_UpdateCanCreateNewNodeForAlloctor);
    _sessionBufferAlloctor = new MemoryAlloctor(FS_BUFF_SIZE_DEF, blockAmount, _updateAlloctorOccupied, &_canCreateNewNodeForAlloctor);
    _sessionBufferAlloctor->InitMemory();
    _printAlloctorOccupiedInfo = DelegatePlusFactory::Create(this, &FS_IocpMsgTransfer::_PrintAlloctorOccupiedInfo);

    _threadPool = new FS_ThreadPool(0, 1);
    _iocp = new FS_Iocp;
    _ioEvent = new IO_EVENT;
    _senderMsgs = new std::list<FS_MessageBlock *>;
    _recvMsgList = new std::list<FS_MessageBlock *>;
    const Int32 st = _iocp->Create();
    if(st != StatusDefs::Success)
    {
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("create iocp fail st[%d]"), st);
        return st;
    }

    return StatusDefs::Success;
}

Int32 FS_IocpMsgTransfer::Start()
{
    auto task = DelegatePlusFactory::Create(this, &FS_IocpMsgTransfer::_OnMoniterMsg);
    if(!_threadPool->AddTask(task, this))
    {
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("addtask fail"));
        return StatusDefs::IocpMsgTransfer_StartFailOfMoniterMsgFailure;
    }

    return StatusDefs::Success;
}

void FS_IocpMsgTransfer::BeforeClose()
{
    // close���лỰ��ʹ��Ͷ�ݵ���Ϣ���Ϸ���
    for(auto &session : _sessions)
    {
        session.second->MaskClose();
        session.second->Close();
    }
}

void FS_IocpMsgTransfer::Close()
{
    // �ȴ����лỰ���Ƴ�
    while(true)
    {
        if(_sessionCnt <= 0)
            break;
        Sleep(1000);
    }

    // Ͷ��iocpquit�¼�
    const auto st = _iocp->PostQuit();
    if(st != StatusDefs::Success)
    {
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("post quit error st[%d]"), st);
    }

    // �߳��˳�
    _threadPool->Close();

    if(!_senderMsgs->empty())
    {
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sender has [%llu] unhandled msgs"), _senderMsgs->size());
        STLUtil::DelListContainer(*_senderMsgs);
        Fs_SafeFree(_senderMsgs);
    }
}

void FS_IocpMsgTransfer::AfterClose()
{
    // �ڴ������ռ�������ӡ����
    g_MemleakMonitor->UnRegisterMemPoolPrintCallback(_transferThreadId);

    if(_sessionBufferAlloctor)
        _sessionBufferAlloctor->FinishMemory();

    Fs_SafeFree(_updateAlloctorOccupied);
}

void FS_IocpMsgTransfer::OnConnect(const BriefSessionInfo  &sessionInfo)
{
    _connectorGuard.Lock();
    _pendingNewSessionInfos.push_back(sessionInfo);
    _hasNewSessionLinkin = true;
    ++_sessionCnt;
    _connectorGuard.Unlock();
}

void FS_IocpMsgTransfer::OnDestroy()
{

}

void FS_IocpMsgTransfer::OnHeartBeatTimeOut(IFS_Session *session)
{

}

void FS_IocpMsgTransfer::_OnMoniterMsg(FS_ThreadPool *pool)
{// iocp ��closesocket������Ϸ�������Ͷ�ݵ��¼������Բ���������postδ����ʱ���ͷ�session����

    // ��ʼ������
    _transferThreadId = SystemUtil::GetCurrentThreadId();
    g_MemleakMonitor->RegisterMemPoolPrintCallback(_transferThreadId, _printAlloctorOccupiedInfo);

    ULong waitTime = 1;   // TODO���Ե��ڣ���Ҫ����������⣩
    Int32 ret = StatusDefs::Success;
    while(pool->IsPoolWorking() || _sessionCnt > 0)
    {
        // 1.�������session����
        _LinkCacheToSessions();

        // 3.�ж��Ƿ���session
        if(_sessions.empty())
        {
            _ClearSenderMessageQueue();
            SocketUtil::Sleep(1);
            continue;
        }

        // 4.������ʱ��⣨���ڵķ���toremove���У�
        _CheckSessionHeartbeat();

        // postsend ���ıȽϴ�
        _AsynSendFromDispatcher();

        // 5.post session to iocp
        _PostEventsToIocp();

        // 6.donetevent ���Ͽ��Ŀͻ��˷���toremove����,��sessions����torecv��tosend����
        ret = _DoEvents();
        if(ret != StatusDefs::Success)
        {
            if(ret != StatusDefs::IocpMsgTransfer_IocpQuit)
                g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("do events error ret[%d]"), ret);
            else
            {
                g_Log->net<FS_IocpMsgTransfer>("transfer id[%d] threadId[%lu] iocp quit", _id, SystemUtil::GetCurrentThreadId());
            }

            _RemoveSessions(true);
            break;
        }

        // 6.domsg ���ıȽϴ��ڸ߲�������¿��ܻᵼ�³�ʱ
        _OnMsgArrived();

        // 7.removesessions
        _RemoveSessions();
    }

    g_Log->sys<FS_IocpMsgTransfer>(_LOGFMT_("transfer id[%d] threadId[%lu] end"), _id, SystemUtil::GetCurrentThreadId());

    // ����sessions ��ʱҪ��֤handlerҪ��û�ر�
    _iocp->Destroy();
    _ClearSessionsWhenClose();
}

Int32 FS_IocpMsgTransfer::_HandleNetEvent()
{
    auto ret = _iocp->WaitForCompletion(*_ioEvent, 1);
    if(ret != StatusDefs::Success)
        return ret;

    // ����iocp�˳�
    if(_ioEvent->_data._code == IocpDefs::IO_QUIT)
    {
        g_Log->sys<FS_IocpMsgTransfer>(_LOGFMT_("FS_IocpMsgTransfer iocp�˳� threadId<%lu> code=%lld")
                                     , SystemUtil::GetCurrentThreadId(), _ioEvent->_data._code);
        return StatusDefs::IocpMsgTransfer_IocpQuit;
    }

    // 1.�жϻỰ�Ƿ����
    const UInt64 sessionId = _ioEvent->_data._sessionId;
    auto session = _GetSession(sessionId);
    if(!session)
    {// ���ݶ�ʧ,���������Ѿ��Ͽ������ˣ�������
//         g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] is removed before.\n stack trace back:\n%s"
//                                      , sessionId, CrashHandleUtil::FS_CaptureStackBackTrace().c_str());
        return StatusDefs::Success;
    }

    // 2.��������뷢��
    if(IocpDefs::IO_RECV == _ioEvent->_ioData->_ioType)
    {
        if(_ioEvent->_bytesTrans <= 0)
        {// �ͻ��˶Ͽ�����
            g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] IO_TYPE::RECV bytesTrans[%lu] will disconnect"
                                           , session->GetSessionId()
                                           , session->GetSocket(),
                                           _ioEvent->_bytesTrans);

             session->ResetPostRecvMask();
             _OnDelayDisconnected(session);
             _toRemove.insert(session);
            return StatusDefs::Success;
        }

        session->OnRecvSuc(_ioEvent->_bytesTrans, _ioEvent->_ioData);

        // ��Ϣ���ջص�
        g_ServerCore->_OnRecvMsg(session, _ioEvent->_bytesTrans);
        // _OnMsgArrived(session);

        _msgArriviedSessions.push_back(session);
        _UpdateSessionHeartbeat(session);

        // ���½�������
        if(session->CanPost())
            _toPostRecv.insert(session);
    }
    else if(IocpDefs::IO_SEND == _ioEvent->_ioData->_ioType)
    {
        if(_ioEvent->_bytesTrans <= 0)
        {// �ͻ��˶Ͽ�����
            g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] IO_TYPE::IO_SEND bytesTrans[%lu] disconnected"
                                           , session->GetSessionId()
                                           , session->GetSocket(),
                                           _ioEvent->_bytesTrans);
             session->ResetPostSendMask();
             _OnDelayDisconnected(session);
             _toRemove.insert(session);
            return StatusDefs::Success;
        }

        session->OnSendSuc(_ioEvent->_bytesTrans, _ioEvent->_ioData);

        // ��Ϣ���ͻص�
        g_ServerCore->_OnSendMsg(session, _ioEvent->_bytesTrans);

        _UpdateSessionHeartbeat(session);

        // ����Ͷ�ݽ���
        if(session->CanPost() &&
           !session->IsPostSend() &&
           session->HasMsgToSend())
            _toPostSend.insert(session);
    }
    else
    {
        session->EnableDisconnect();
        session->Close();
        _toRemove.insert(session);
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("undefine io type[%d]."), _ioEvent->_ioData->_ioType);
    }

    // �ͻ����Ѿ��Ͽ�����
//     if(session->IsClose())
//     {
//         g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("session is closed sessionId[%llu]."), sessionId);
//         // iocpSession->EnableDisconnect();
//         _toRemove.insert(session);
//     }

    return StatusDefs::Success;
}

void FS_IocpMsgTransfer::_OnMsgArrived()
{
    if(_msgArriviedSessions.empty())
        return;

    // ������Ϣ����
    FS_IocpSession *session = NULL;
    UInt64 sessionId = 0;
    for(auto iterSession = _msgArriviedSessions.begin(); iterSession != _msgArriviedSessions.end();)
    {
        session = *iterSession;
        sessionId = session->GetSessionId();
        auto recvBuffer = session->CastToRecvBuffer();
        while(recvBuffer->HasMsg())
        {
            auto frontMsg = recvBuffer->CastToData<NetMsg_DataHeader>();

            // ������Ϣ
            FS_NetMsgBufferBlock *newBlock = new FS_NetMsgBufferBlock;
            newBlock->_generatorId = _id;
            newBlock->_sessionId = sessionId;
            newBlock->_mbType = MessageBlockType::MB_NetMsgArrived;
            g_MemoryPool->Lock();
            newBlock->_buffer = g_MemoryPool->Alloc<Byte8>(frontMsg->_packetLength);
            g_MemoryPool->Unlock();
            ::memcpy(newBlock->_buffer, frontMsg, frontMsg->_packetLength);

            _recvMsgList->push_back(newBlock);
            g_ServerCore->_OnRecvMsgAmount(frontMsg);
            recvBuffer->PopFront(frontMsg->_packetLength);
        }

        iterSession = _msgArriviedSessions.erase(iterSession);
    }

    _messageQueue->PushLock(_generatorId);
    if(!_messageQueue->Push(_generatorId, _recvMsgList))
    {
        if(!_recvMsgList->empty())
            g_Log->memleak("_OnMsgArrived mem leak FS_MessageBlock cnt[%llu]", _recvMsgList->size());
        STLUtil::DelListContainer(*_recvMsgList);
    }
    _messageQueue->PushUnlock(_generatorId);

}

void FS_IocpMsgTransfer::_OnMsgArrived(FS_IocpSession *session)
{
    // TODO:�Ͽ��˵�ͬʱ�յ���Ϣ������Ŀǰ��ʱ������
    if(_toRemove.find(session) != _toRemove.end())
        return;

    {
        auto sessionId = session->GetSessionId();
        auto recvBuffer = session->CastToRecvBuffer();
        while(recvBuffer->HasMsg())
        {
            auto frontMsg = recvBuffer->CastToData<NetMsg_DataHeader>();

            // ������Ϣ
            FS_NetMsgBufferBlock *newBlock = new FS_NetMsgBufferBlock;
            newBlock->_generatorId = _id;
            newBlock->_sessionId = sessionId;
            newBlock->_mbType = MessageBlockType::MB_NetMsgArrived;
            g_MemoryPool->Lock();
            newBlock->_buffer = g_MemoryPool->Alloc<Byte8>(frontMsg->_packetLength);
            g_MemoryPool->Unlock();
            ::memcpy(newBlock->_buffer, frontMsg, frontMsg->_packetLength);

            _recvMsgList->push_back(newBlock);
            g_ServerCore->_OnRecvMsgAmount(frontMsg);
            recvBuffer->PopFront(frontMsg->_packetLength);
        }
    }

    _messageQueue->PushLock(_generatorId);
    if(!_messageQueue->Push(_generatorId, _recvMsgList))
    {
        if(!_recvMsgList->empty())
            g_Log->memleak("_OnMsgArrived mem leak FS_MessageBlock cnt[%llu]", _recvMsgList->size());
        STLUtil::DelListContainer(*_recvMsgList);
    }
    _messageQueue->PushUnlock(_generatorId);

}

void FS_IocpMsgTransfer::_OnDelayDisconnected(FS_IocpSession *session)
{
    session->MaskClose();
    CancelIoEx(HANDLE(session->GetSocket()), NULL);
}

void FS_IocpMsgTransfer::_OnDisconnected(FS_IocpSession *session)
{
    const auto sessionId = session->GetSessionId();
    g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] disconnected address<%s>"
                                   , sessionId
                                   , session->GetSocket(), session->GetAddr()->ToString().c_str());

    // servercore�յ��Ͽ��ص� ��Ҫ�ص�����ģ���̰߳�ȫ�Ľӿڱ�������
    _sessionHeartbeatQueue.erase(session);
    g_ServerCore->_OnDisconnected(session);

    // ���ͻỰ�Ͽ���Ϣ
    _NtySessionDisConnectMsg(sessionId);

    // �Ƴ��Ự
     _sessions.erase(sessionId);
    _toPostRecv.erase(session);
    _toPostSend.erase(session);
    _DestroySession(session);
    --_sessionCnt;
}

void FS_IocpMsgTransfer::_DestroySession(FS_IocpSession *session)
{
    session->Close();
    session->OnDisconnect();
    session->OnDestroy();
    Fs_SafeFree(session);
}

bool FS_IocpMsgTransfer::_DoPostSend(FS_IocpSession *session)
{
    auto ioData = session->MakeSendIoData();
    if(ioData)
    {
        Int32 st = _iocp->PostSend(session->GetSocket(), ioData);
        if(st != StatusDefs::Success)
        {
            session->ResetPostSendMask();
            _OnDelayDisconnected(session);
            if(st != StatusDefs::IOCP_ClientForciblyClosed)
            {
                g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] socket[%llu] post send fail st[%d]")
                                             , session->GetSessionId(), session->GetSocket(), st);
            }

            _toRemove.insert(session);
            return false;
        }
    }

    return true;
}

bool FS_IocpMsgTransfer::_DoPostRecv(FS_IocpSession *session)
{
    auto ioData = session->MakeRecvIoData();
    if(ioData)
    {
        Int32 st = _iocp->PostRecv(session->GetSocket(), ioData);
        if(st != StatusDefs::Success)
        {
            session->ResetPostRecvMask();
            _OnDelayDisconnected(session);
            if(st != StatusDefs::IOCP_ClientForciblyClosed)
            {
                g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] socket[%llu] post recv fail st[%d]")
                                             , session->GetSessionId(), session->GetSocket(), st);
            }
            
            _toRemove.insert(session);
                //_OnGracefullyDisconnect(session);
            return false;
        }
    }

    return true;
}

void FS_IocpMsgTransfer::_NtySessionConnectedMsg(UInt64 sessionId)
{
    FS_NetMsgBufferBlock *newMsgBlock = new FS_NetMsgBufferBlock;
    newMsgBlock->_generatorId = _id;
    newMsgBlock->_mbType = MessageBlockType::MB_NetSessionConnected;
    newMsgBlock->_sessionId = sessionId;
    _messageQueue->PushLock(_id);
    if(!_messageQueue->Push(_id, newMsgBlock))
        Fs_SafeFree(newMsgBlock);

    _messageQueue->Notify(_id);
    _messageQueue->PushUnlock(_id);
}

void FS_IocpMsgTransfer::_NtySessionDisConnectMsg(UInt64 sessionId)
{
    FS_NetMsgBufferBlock *newMsgBlock = new FS_NetMsgBufferBlock;
    newMsgBlock->_generatorId = _id;
    newMsgBlock->_mbType = MessageBlockType::MB_NetSessionDisconnect;
    newMsgBlock->_sessionId = sessionId;
    _messageQueue->PushLock(_id);
    if(!_messageQueue->Push(_id, newMsgBlock))
        Fs_SafeFree(newMsgBlock);
    _messageQueue->Notify(_id);
    _messageQueue->PushUnlock(_id);
}

void FS_IocpMsgTransfer::_ClearSessionsWhenClose()
{
    for(auto &iterSession : _sessions)
    {
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] not destroy when thread ending")
                                     , iterSession.first);
        _DestroySession(iterSession.second);
    }
}

void FS_IocpMsgTransfer::_CheckSessionHeartbeat()
{
    _curTime.FlushTime();
    for(auto iterSession = _sessionHeartbeatQueue.begin(); 
        iterSession != _sessionHeartbeatQueue.end(); 
        )
    {
        auto session = (*iterSession)->CastTo<FS_IocpSession>();
        if(session->GetHeartBeatExpiredTime() > _curTime)
            break;

        g_ServerCore->_OnHeartBeatTimeOut(session);
        iterSession = _sessionHeartbeatQueue.erase(iterSession);
        session->MaskClose();
        _toRemove.insert(session);
//        g_Log->any<FS_IocpMsgTransfer>("nowTime[%lld][%s] sessionId[%llu] expiredtime[%llu][%s] heartbeat time out."
//                                       , _curTime.GetMicroTimestamp(), _curTime.ToStringOfMillSecondPrecision().c_str()
//                                       ,session->GetSessionId()
//                                       , session->GetHeartBeatExpiredTime().GetMicroTimestamp()
//                                       , session->GetHeartBeatExpiredTime().ToStringOfMillSecondPrecision().c_str());

        // �������� ��ondisconnected�������Ƴ�
        // OnHeartBeatTimeOut(session);
    }
}

void FS_IocpMsgTransfer::_PostEventsToIocp()
{
    FS_IocpSession *session = NULL;
    for(auto iterRecv = _toPostRecv.begin(); iterRecv != _toPostRecv.end();)
    {
        session = *iterRecv;
        if(session->CanPost())
            _DoPostRecv(session);

        iterRecv = _toPostRecv.erase(iterRecv);
    }

    for(auto iterSend = _toPostSend.begin(); iterSend != _toPostSend.end();)
    {
        session = *iterSend;
        if(!session->CanPost()||session->IsPostSend())
        {
            iterSend = _toPostSend.erase(iterSend);
            continue;
        }
        
        _DoPostSend(session);
        iterSend = _toPostSend.erase(iterSend);
    }
}

void FS_IocpMsgTransfer::_AsynSendFromDispatcher()
{
    _senderMessageQueue->PopLock();
    _senderMessageQueue->PopImmediately(_senderMsgs);
    _senderMessageQueue->PopUnlock();

    // TODO:��Ϣ����ʱ�������ѭ���У����������ٶȣ���Ҫ�Ȱ�sessionId�������һ�������У�����Ҫ����Ϣ�����ж��Ʒ��෽���ص���ʹ����Ϸ�ʽ����
    // ���أ�std::map<UInt64, std::list<FS_NetMsgBufferBlock *>> *����
    for(auto iterBlock = _senderMsgs->begin(); iterBlock != _senderMsgs->end();)
    {
        FS_NetMsgBufferBlock *sendMsgBufferBlock = static_cast<FS_NetMsgBufferBlock *>(*iterBlock);
        auto session = _GetSession(sendMsgBufferBlock->_sessionId);
        if(session && session->CanPost())
        {
            NetMsg_DataHeader *header = reinterpret_cast<NetMsg_DataHeader *>(sendMsgBufferBlock->_buffer);
            if(!session->PushMsgToSend(header))
            {
                g_Log->w<FS_IocpMsgTransfer>(_LOGFMT_("sessionid[%llu] send msg cmd[%s:%hu] len[%hu] fail")
                                             , sendMsgBufferBlock->_sessionId, ProtocolCmd::GetStr(header->_cmd)
                                             , header->_cmd, header->_packetLength);
            }

            if(!session->IsPostSend())
            {
                _DoPostSend(session);
                _toPostSend.erase(session);
            }
        }

        Fs_SafeFree(sendMsgBufferBlock);
        iterBlock = _senderMsgs->erase(iterBlock);
    }
}

void FS_IocpMsgTransfer::_ClearSenderMessageQueue()
{
    _senderMessageQueue->PopLock();
    _senderMessageQueue->PopImmediately(_senderMsgs);
    _senderMessageQueue->PopUnlock();

    // ��ʾ��n����Ϣ���ڶԶ˹رն��޷�����ȥ
    if(!_senderMsgs->empty())
    {
        g_Log->net<FS_IocpMsgTransfer>("it has %llu messages unhandled which will send to remote session!", _senderMsgs->size());
        STLUtil::DelListContainer(*_senderMsgs);
    }
}

void FS_IocpMsgTransfer::_FreeSendList(std::list<NetMsg_DataHeader *> *sendQueue)
{
    for(auto &msg : *sendQueue)
    {
        g_MemoryPool->Lock();
        g_MemoryPool->Free(msg);
        g_MemoryPool->Unlock();
    }
    Fs_SafeFree(sendQueue);
}

void FS_IocpMsgTransfer::_LinkCacheToSessions()
{
    if(_hasNewSessionLinkin)
    {
        _connectorGuard.Lock();
        for(auto iterSession = _pendingNewSessionInfos.begin(); iterSession != _pendingNewSessionInfos.end();)
        {
            auto &newSessionInfo = *iterSession;
            auto newSession = FS_SessionFactory::Create(newSessionInfo, _sessionBufferAlloctor)->CastTo<FS_IocpSession>();
            _sessions.insert(std::make_pair(newSessionInfo._sessionId, newSession));
            newSession->OnConnect();
            auto addr = newSession->GetAddr();
            g_Log->net<FS_IocpMsgTransfer>("new session connected: id<%llu>,socket<%llu>,remote ip[%s:%hu]"
                                         , newSession->GetSessionId()
                                         , newSession->GetSocket()
                                         , addr->GetAddr().c_str()
                                         , addr->GetPort());
            // ��iocp
            auto st = _iocp->Reg(newSessionInfo._sock, newSessionInfo._sessionId);
            if(st != StatusDefs::Success)
            {
                g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("reg socket[%llu] sessionId[%llu] fail st[%d]")
                                             , newSessionInfo._sock, newSessionInfo._sessionId, st);
            }

            // ���ͻỰ������Ϣ
            _NtySessionConnectedMsg(newSessionInfo._sessionId);

            // Ͷ�ݽ�������
            if(!_DoPostRecv(newSession))
            {
                g_Log->w<FS_IocpMsgTransfer>(_LOGFMT_("post recv fail sessionId[%llu] serverId[%d]"), newSessionInfo._sessionId, _id);
                _toRemove.insert(newSession);
            }
            else
            {
                _UpdateSessionHeartbeat(newSession);
            }

            iterSession = _pendingNewSessionInfos.erase(iterSession);
        }
        _hasNewSessionLinkin = false;
        _connectorGuard.Unlock();
    }
}


FS_NAMESPACE_END


