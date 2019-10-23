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

#include "base/common/status/status.h"
#include "base/common/component/Impl/FS_ThreadPool.h"
#include "base/common/log/Log.h"
#include "base/common/crashhandle/CrashHandle.h"
#include "base/common/socket/socket.h"
#include "base/common/net/Impl/FS_Addr.h"

FS_NAMESPACE_BEGIN
FS_IocpMsgTransfer::FS_IocpMsgTransfer()
    :_threadPool(NULL)
    ,_iocp(NULL)
    ,_ioEvent(NULL)
    ,_serverCoreDisconnectedCallback(NULL)
    ,_serverCoreRecvSucCallback(NULL)
    ,_serverCoreSendSucCallback(NULL)
    ,_serverCoreHeartBeatTimeOutCallback(NULL)
    ,_sessionCnt{0}
    ,_hasNewSessionLinkin(false)
    ,_isSendCacheDirtied(false)
{
}

FS_IocpMsgTransfer::~FS_IocpMsgTransfer()
{
    Fs_SafeFree(_serverCoreSendSucCallback);
    Fs_SafeFree(_serverCoreRecvSucCallback);
    Fs_SafeFree(_serverCoreDisconnectedCallback);
    Fs_SafeFree(_serverCoreHeartBeatTimeOutCallback);
    Fs_SafeFree(_threadPool);
    Fs_SafeFree(_iocp);
    Fs_SafeFree(_ioEvent);
}

Int32 FS_IocpMsgTransfer::BeforeStart()
{
    _threadPool = new FS_ThreadPool(0, 1);
    _iocp = new FS_Iocp;
    _ioEvent = new IO_EVENT;
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
        session.second->Close();
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
    _threadPool->Clear();
    _iocp->Destroy();
}

void FS_IocpMsgTransfer::AfterClose()
{
    Fs_SafeFree(_serverCoreDisconnectedCallback);
}

void FS_IocpMsgTransfer::OnConnect(IFS_Session *session)
{
    _connectorGuard.Lock();
    _linkSessionCache.push_back(session);
    _hasNewSessionLinkin = true;
    _connectorGuard.Unlock();
    ++_sessionCnt;
}

void FS_IocpMsgTransfer::OnDestroy()
{

}

void FS_IocpMsgTransfer::OnHeartBeatTimeOut(IFS_Session *session)
{
//     g_Log->any<FS_IocpMsgTransfer>("OnHeartBeatTimeOut sessionId[%llu] sock[%llu] _OnGracefullyDisconnect"
//                                    , session->GetSessionId()
//                                    , session->GetSocket());
//     g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] _OnGracefullyDisconnect"
//                                    , session->GetSessionId()
//                                    , session->GetSocket());

    _serverCoreHeartBeatTimeOutCallback->Invoke(session);
    auto iocpSession = session->CastTo<FS_IocpSession>();
    iocpSession->ResetPostRecvMask();
    iocpSession->ResetPostSendMask();

    // _OnGracefullyDisconnect(session);
    _OnDisconnected(session);
}

void FS_IocpMsgTransfer::AsynSend(UInt64 sessionId, NetMsg_DataHeader *msg)
{
    _asynSendGuard.Lock();
    auto iterQueue = _asynSendQueueCache.find(sessionId);
    if(iterQueue == _asynSendQueueCache.end())
        iterQueue = _asynSendQueueCache.insert(std::make_pair(sessionId, new std::list<NetMsg_DataHeader *>)).first;
    auto queueList = iterQueue->second;
    queueList->push_back(msg);
    _isSendCacheDirtied = true;
    _asynSendGuard.Unlock();
}

void FS_IocpMsgTransfer::RegisterDisconnected(IDelegate<void, IFS_Session *> *callback)
{
    Fs_SafeFree(_serverCoreDisconnectedCallback);
    _serverCoreDisconnectedCallback = callback;
}

void FS_IocpMsgTransfer::RegisterRecvSucCallback(IDelegate<void, IFS_Session *, Int64> *callback)
{
    Fs_SafeFree(_serverCoreRecvSucCallback);
    _serverCoreRecvSucCallback = callback;
}

void FS_IocpMsgTransfer::RegisterSendSucCallback(IDelegate<void, IFS_Session *, Int64> *callback)
{
    Fs_SafeFree(_serverCoreSendSucCallback);
    _serverCoreSendSucCallback = callback;
}

void FS_IocpMsgTransfer::RegisterHeatBeatTimeOutCallback(IDelegate<void, IFS_Session *> *callback)
{
    Fs_SafeFree(_serverCoreHeartBeatTimeOutCallback);
    _serverCoreHeartBeatTimeOutCallback = callback;
}

Int32 FS_IocpMsgTransfer::GetSessionCnt()
{
    return _sessionCnt;
}

void FS_IocpMsgTransfer::_OnMoniterMsg(const FS_ThreadPool *pool)
{// iocp ��closesocket������Ϸ�������Ͷ�ݵ��¼������Բ���������postδ����ʱ���ͷ�session����

    ULong waitTime = 1;   // TODO���Ե��ڣ���Ҫ����������⣩
    std::set<UInt64> timeoutSessionIds;
    std::set<UInt64> sessinsToRemove;
    std::set<UInt64> toPostRecv;
    std::set<UInt64> toPostSend;
    while(!pool->IsClearingPool() || _sessionCnt > 0)
    {
        _LinkCacheToSessions();

        // û�лỰ����
        if(_sessions.empty())
        {
            SocketUtil::Sleep(1);
            continue;
        }

        // �ȴ�������Ϣ
        timeoutSessionIds.clear();
        sessinsToRemove.clear();
        toPostRecv.clear();
        toPostSend.clear();
        _CheckSessionHeartbeat(timeoutSessionIds);
        sessinsToRemove = timeoutSessionIds;
        _PostSessions();

        Int32 ret = StatusDefs::Success;
        while(true)
        {
            ret = _iocp->WaitForCompletion(*_ioEvent, waitTime);
            if(ret != StatusDefs::Success)
            {
                if(ret == StatusDefs::IOCP_WaitTimeOut)
                    ret = StatusDefs::Success;
                break;
            }

            _HandleNetEvent(sessinsToRemove, toPostRecv, toPostSend);
        }

        // post recv
        for(auto &sessionId : toPostRecv)
        {
            if(!_DoPostRecv(_GetSession(sessionId)->CastTo<FS_IocpSession>(), false))
                sessinsToRemove.insert(sessionId);
        }

        // post send
        for(auto &sessionId : toPostSend)
        {
            if(!_DoPostSend(_GetSession(sessionId)->CastTo<FS_IocpSession>(), false))
                sessinsToRemove.insert(sessionId);
        }
        
        if(ret != StatusDefs::Success)
        {
            _OnHeartbeatTimeOut(timeoutSessionIds, sessinsToRemove);
            g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("_OnMoniterMsg.wait nothing but ret[%d] monitor end."), ret);
            break;
        }

        // ����iocp�˳�
        if(_ioEvent->_data._code == IocpDefs::IO_QUIT)
        {
            // TODO:�˳�ǰ��Ҫ��������Ϣ�������
            // �������κ���Ϣ���ȴ���Ϣ������ �ȹر�socket�Ķ���
            g_Log->sys<FS_IocpMsgTransfer>(_LOGFMT_("iocp�˳� code=%lld"), _ioEvent->_data._code);
            break;
        }

//        g_Log->any<FS_IocpMsgTransfer>("iocp wake up of ret[%d] sessionId[%llu]"
//                                       , ret, _ioEvent->_data._sessionId);


        
        _OnHeartbeatTimeOut(timeoutSessionIds, sessinsToRemove);
        _RemoveSessions(sessinsToRemove);
    }

    g_Log->sys<FS_IocpMsgTransfer>(_LOGFMT_("transfer thread end"));

    // ����sessions ��ʱҪ��֤handlerҪ��û�ر�
    _ClearSessionsWhenClose();
}

void FS_IocpMsgTransfer::_HandleNetEvent(std::set<UInt64> &sessionIdsToRemove, std::set<UInt64> &toPostRecv, std::set<UInt64> &toPostSend)
{
    const UInt64 sessionId = _ioEvent->_data._sessionId;

    // 1.�жϻỰ�Ƿ����
    auto session = _GetSession(sessionId);
    if(!session)
    {// ���ݶ�ʧ,���������Ѿ��Ͽ������ˣ�������
        g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] is removed before.\n stack trace back:\n%s"
                                     , sessionId, CrashHandleUtil::FS_CaptureStackBackTrace().c_str());

        // ȡ������io�������ر��׽���
//         auto ioData = _ioEvent->_ioData;
//         if(ioData)
//         {
//             if(ioData->_sock != INVALID_SOCKET)
//             {
//                 CancelIoEx(HANDLE(ioData->_sock), NULL);    // ȡ������δ����io����
//                 SocketUtil::DestroySocket(ioData->_sock);
//             }
//         }
        return;
    }

    auto iocpSession = session->CastTo<FS_IocpSession>();

    // 2.��������뷢��
    if(IocpDefs::IO_RECV == _ioEvent->_ioData->_ioType)
    {
        if(_ioEvent->_bytesTrans <= 0)
        {// �ͻ��˶Ͽ�����
//            g_Log->any<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] IO_TYPE::RECV bytesTrans[%lu] will disconnect"
//                                           , iocpSession->GetSessionId()
//                                           , iocpSession->GetSocket(),
//                                           _ioEvent->_bytesTrans);
            g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] IO_TYPE::RECV bytesTrans[%lu] will disconnect"
                                           , iocpSession->GetSessionId()
                                           , iocpSession->GetSocket(),
                                           _ioEvent->_bytesTrans);

            iocpSession->ResetPostRecvMask();
            sessionIdsToRemove.insert(sessionId);
            toPostRecv.erase(sessionId);
            toPostSend.erase(sessionId);
            return;
        }

        iocpSession->OnRecvSuc(_ioEvent->_bytesTrans, _ioEvent->_ioData);

        // ��Ϣ���ջص�
        _serverCoreRecvSucCallback->Invoke(session, _ioEvent->_bytesTrans);

        _UpdateSessionHeartbeat(session);

//        g_Log->any<FS_IocpMsgTransfer>("sessionId[%llu] transfer[%lu] recv msg", sessionId, _ioEvent->_bytesTrans);

        // ����Ͷ�ݽ���
        if(iocpSession->CanPost())
        {
            toPostRecv.insert(sessionId);
//             if(!_DoPostRecv(iocpSession, false))
//             {
//                 sessionIdsToRemove.insert(sessionId);
//                 return;
//             }

//            g_Log->any<FS_IocpMsgTransfer>("sessionId[%llu] re post recv ", sessionId);

        }
    }
    else if(IocpDefs::IO_SEND == _ioEvent->_ioData->_ioType)
    {
        if(_ioEvent->_bytesTrans <= 0)
        {// �ͻ��˶Ͽ�����
//             g_Log->any<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] IO_TYPE::IO_SEND bytesTrans[%lu] disconnected"
//                                            , session->GetSessionId()
//                                            , session->GetSocket(),
//                                            _ioEvent->_bytesTrans);
            g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] IO_TYPE::IO_SEND bytesTrans[%lu] disconnected"
                                           , session->GetSessionId()
                                           , session->GetSocket(),
                                           _ioEvent->_bytesTrans);
            iocpSession->ResetPostSendMask();
            sessionIdsToRemove.insert(sessionId);
            toPostSend.erase(sessionId);
            toPostRecv.erase(sessionId);
            return;
        }

        auto iocpSession = session->CastTo<FS_IocpSession>();
        iocpSession->OnSendSuc(_ioEvent->_bytesTrans, _ioEvent->_ioData);
//        g_Log->any<FS_IocpMsgTransfer>("send suc sessionId[%llu] transfer[%lu] ", sessionId, _ioEvent->_bytesTrans);
        // ��Ϣ���ͻص�
        _serverCoreSendSucCallback->Invoke(session, _ioEvent->_bytesTrans);

        _UpdateSessionHeartbeat(session);

        // ����Ͷ�ݽ���
        if(iocpSession->CanPost() && iocpSession->HasMsgToSend())
        {
            toPostSend.insert(sessionId);
//             if(!_DoPostSend(iocpSession, false))
//             {
//                 sessionIdsToRemove.insert(sessionId);
//                 return;
//             }

//            g_Log->any<FS_IocpMsgTransfer>("sessionId[%llu] re post recv ", sessionId);
        }
    }
    else
    {
        iocpSession->EnableDisconnect();
        sessionIdsToRemove.insert(sessionId);
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("undefine io type[%d]."), _ioEvent->_ioData->_ioType);
    }

    // �ͻ����Ѿ��Ͽ�����
    if(iocpSession->IsClose())
    {
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("session is closed sessionId[%llu]."), sessionId);
        // iocpSession->EnableDisconnect();
        sessionIdsToRemove.insert(sessionId);
    }
}

void FS_IocpMsgTransfer::_RemoveSessions(const std::set<UInt64> &sessionIds)
{
    for(auto &sessionId : sessionIds)
    {
        auto session = _GetSession(sessionId);
        if(!session)
            continue;

        // _OnDisconnected(session);
        _OnGracefullyDisconnect(session);
    }
}

void FS_IocpMsgTransfer::_OnHeartbeatTimeOut(const std::set<UInt64> &timeoutSessionIds, std::set<UInt64> &leftSessionIdsToRemove)
{
    for(auto &sessionId : timeoutSessionIds)
    {
        leftSessionIdsToRemove.erase(sessionId);
        auto session = _GetSession(sessionId);
        if(!session)
            continue;

        OnHeartBeatTimeOut(session);
    }
}

void FS_IocpMsgTransfer::_OnDelayDisconnected(IFS_Session *session)
{
    session->MaskClose();
    CancelIoEx(HANDLE(session->GetSocket()), NULL);
//     if(!session->IsClose())
//         session->Close();
}

void FS_IocpMsgTransfer::_OnDisconnected(IFS_Session *session)
{
//     g_Log->any<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] disconnected"
//                                    , session->GetSessionId()
//                                    , session->GetSocket());
    g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] disconnected address<%s>"
                                   , session->GetSessionId()
                                   , session->GetSocket(), session->GetAddr()->ToString().c_str());

    // ȡ������δ����io����
    //CancelIoEx(HANDLE(session->GetSocket()), NULL);
    session->Close();
    _sessionHeartbeatQueue.erase(session);

    // servercore�յ��Ͽ��ص� ��Ҫ�ص�����ģ���̰߳�ȫ�Ľӿڱ�������
    _serverCoreDisconnectedCallback->Invoke(session);

    session->OnDisconnect();
    session->OnDestroy();

    // �Ƴ��Ự
    _sessions.erase(session->GetSessionId());
    Fs_SafeFree(session);
    --_sessionCnt;
}

bool FS_IocpMsgTransfer::_DoPostSend(FS_IocpSession *session, bool removeIfFail)
{
    auto ioData = session->MakeSendIoData();
    if(ioData)
    {
        Int32 st = _iocp->PostSend(session->GetSocket(), ioData);
        if(st != StatusDefs::Success)
        {
            session->ResetPostSendMask();
            if(st != StatusDefs::IOCP_ClientForciblyClosed)
            {
                g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] socket[%llu] post send fail st[%d]")
                                             , session->GetSessionId(), session->GetSocket(), st);
            }

            if(removeIfFail)
                _OnGracefullyDisconnect(session);
            return false;
        }
    }

    return true;
}

bool FS_IocpMsgTransfer::_DoPostRecv(FS_IocpSession *session, bool removeIfFail)
{
    auto ioData = session->MakeRecvIoData();
    if(ioData)
    {
        Int32 st = _iocp->PostRecv(session->GetSocket(), ioData);
        if(st != StatusDefs::Success)
        {
            session->ResetPostRecvMask();
            if(st != StatusDefs::IOCP_ClientForciblyClosed)
            {
                g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] socket[%llu] post recv fail st[%d]")
                                             , session->GetSessionId(), session->GetSocket(), st);
            }

            if(removeIfFail)
                _OnGracefullyDisconnect(session);
            return false;
        }
    }

    return true;
}

IFS_Session *FS_IocpMsgTransfer::_GetSession(UInt64 sessionId)
{
    auto iterSession = _sessions.find(sessionId);
    if(iterSession == _sessions.end())
        return NULL;

    return iterSession->second;
}

void FS_IocpMsgTransfer::_ClearSessionsWhenClose()
{
    for(auto &iterSession : _sessions)
    {
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] not destroy when thread ending")
                                     , iterSession.first);
        _OnDisconnected(iterSession.second);
    }
}

void FS_IocpMsgTransfer::_UpdateSessionHeartbeat(IFS_Session *session)
{
    _sessionHeartbeatQueue.erase(session);
    session->UpdateHeartBeatExpiredTime();
    _sessionHeartbeatQueue.insert(session);
}

void FS_IocpMsgTransfer::_CheckSessionHeartbeat(std::set<UInt64> &timeoutSessionIds)
{
    _curTime.FlushTime();
    for(auto iterSession = _sessionHeartbeatQueue.begin(); 
        iterSession != _sessionHeartbeatQueue.end(); 
        ++iterSession)
    {
        auto session = *iterSession;
        if(session->GetHeartBeatExpiredTime() > _curTime)
            break;

        timeoutSessionIds.insert(session->GetSessionId());
//        g_Log->any<FS_IocpMsgTransfer>("nowTime[%lld][%s] sessionId[%llu] expiredtime[%llu][%s] heartbeat time out."
//                                       , _curTime.GetMicroTimestamp(), _curTime.ToStringOfMillSecondPrecision().c_str()
//                                       ,session->GetSessionId()
//                                       , session->GetHeartBeatExpiredTime().GetMicroTimestamp()
//                                       , session->GetHeartBeatExpiredTime().ToStringOfMillSecondPrecision().c_str());

        // �������� ��ondisconnected�������Ƴ�
        // OnHeartBeatTimeOut(session);
        // iterSession = _sessionHeartbeatQueue.erase(iterSession);
    }
}

void FS_IocpMsgTransfer::_PostSessions()
{
    if(_isSendCacheDirtied)
    {
        std::list<NetMsg_DataHeader *> *msgs = NULL;
        _asynSendGuard.Lock();
        _isSendCacheDirtied = false;
        for(auto iterQueueCache = _asynSendQueueCache.begin();iterQueueCache!= _asynSendQueueCache.end();)
        {
            auto iterMsgs = _asynSendQueue.find(iterQueueCache->first);
            if(iterMsgs == _asynSendQueue.end())
                iterMsgs = _asynSendQueue.insert(std::make_pair(iterQueueCache->first, new std::list<NetMsg_DataHeader *>)).first;

            iterMsgs->second = iterQueueCache->second;
            iterQueueCache = _asynSendQueueCache.erase(iterQueueCache);
        }
        _asynSendGuard.Unlock();

        msgs = NULL;
        for(auto iterMsgs= _asynSendQueue.begin();iterMsgs!=_asynSendQueue.end();)
        {
            auto iterSession = _sessions.find(iterMsgs->first);
            if(iterSession == _sessions.end())
            {
                g_Log->w<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] has already disconnected or is not existed before"), iterMsgs->first);
                _FreeSendList(iterMsgs->second);
                iterMsgs = _asynSendQueue.erase(iterMsgs);
                continue;
            }

            auto iocpSession = iterSession->second->CastTo<FS_IocpSession>();
            auto msgList = iterMsgs->second;
            for(auto &msg : *msgList)
            {
                if(iocpSession->CanPost())
                {
                    if(!iocpSession->Send(msg))
                    {
                        g_Log->w<FS_IocpMsgTransfer>(_LOGFMT_("sessionid[%llu] send msg fail"), iterMsgs->first);
                    }
                }

                // �ͷ�
                g_MemoryPool->Lock();
                g_MemoryPool->Free(msg);
                g_MemoryPool->Unlock();
            }

            Fs_SafeFree(msgList);
            iterMsgs = _asynSendQueue.erase(iterMsgs);
        }
    }
}

void fs::FS_IocpMsgTransfer::_FreeSendList(std::list<NetMsg_DataHeader *> *sendQueue)
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
        _linkSessionSwitchCache = _linkSessionCache;
        _linkSessionCache.clear();
        _hasNewSessionLinkin = false;
        _connectorGuard.Unlock();

        for(auto &session : _linkSessionSwitchCache)
        {
            _sessions.insert(std::make_pair(session->GetSessionId(), session));
            auto iocpSession = session->CastTo<FS_IocpSession>();
            auto sender = DelegatePlusFactory::Create(this, &FS_IocpMsgTransfer::_DoPostSend);
            iocpSession->BindSender(sender);

            // ��iocp
            auto st = _iocp->Reg(iocpSession->GetSocket(), iocpSession->GetSessionId());
            if(st != StatusDefs::Success)
            {
                g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("reg socket[%llu] sessionId[%llu] fail st[%d]")
                                             , session->GetSocket(), session->GetSessionId(), st);
            }

            // Ͷ�ݽ�������
            if(!_DoPostRecv(iocpSession))
            {
                g_Log->w<FS_IocpMsgTransfer>(_LOGFMT_("post recv fail"));
            }

        }

        _linkSessionSwitchCache.clear();
    }

}

FS_NAMESPACE_END

