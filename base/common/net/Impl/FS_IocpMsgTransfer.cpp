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

FS_NAMESPACE_BEGIN
FS_IocpMsgTransfer::FS_IocpMsgTransfer()
    :_threadPool(NULL)
    ,_iocp(NULL)
    ,_ioEvent(NULL)
    ,_serverCoreDisconnectedCallback(NULL)
    ,_serverCoreRecvSucCallback(NULL)
    ,_serverCoreSendSucCallback(NULL)
{
}

FS_IocpMsgTransfer::~FS_IocpMsgTransfer()
{
    Fs_SafeFree(_serverCoreDisconnectedCallback);
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
    _locker.Lock();
    for(auto &session : _sessions)
        session.second->Close();
    _locker.Unlock();
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
    _locker.Lock();
    _sessions.insert(std::make_pair(session->GetSessionId(), session));
    auto iocpSession = session->CastTo<FS_IocpSession>();
    auto sender = DelegatePlusFactory::Create(this, &FS_IocpMsgTransfer::_DoSend);
    iocpSession->BindSender(sender);
    ++_sessionCnt;
    _locker.Unlock();
}

void FS_IocpMsgTransfer::OnDestroy()
{

}

void FS_IocpMsgTransfer::OnHeartBeatTimeOut(IFS_Session *session)
{
    g_Log->any<FS_IocpMsgTransfer>("OnHeartBeatTimeOut sessionId[%llu] sock[%llu] _OnGracefullyDisconnect"
                                   , session->GetSessionId()
                                   , session->GetSocket());
    g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] _OnGracefullyDisconnect"
                                   , session->GetSessionId()
                                   , session->GetSocket());

    _OnGracefullyDisconnect(session);
}

void FS_IocpMsgTransfer::OnSendData(UInt64 sessionId, NetMsg_DataHeader *msg)
{
    do 
    {
        _locker.Lock();
        auto iterSession = _sessions.find(sessionId);
        if(iterSession == _sessions.end())
        {
            g_Log->w<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] has already disconnected or is not existed before"), sessionId);
            _locker.Unlock();
            break;
        }

        auto iocpSession = iterSession->second->CastTo<FS_IocpSession>();
        if(iocpSession->CanPost())
        {
            if(!iocpSession->Send(msg))
            {
                g_Log->w<FS_IocpMsgTransfer>(_LOGFMT_("sessionid[%llu] send msg fail"), sessionId);
            }
        }
        _locker.Unlock();
    } while(0);

    g_MemoryPool->Lock();
    g_MemoryPool->Free(msg);
    g_MemoryPool->Unlock();
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

Int32 FS_IocpMsgTransfer::GetSessionCnt()
{
    return _sessionCnt;
}

void FS_IocpMsgTransfer::_OnMoniterMsg(const FS_ThreadPool *pool)
{// iocp ��closesocket������Ϸ�������Ͷ�ݵ��¼������Բ���������postδ����ʱ���ͷ�session����

    ULong waitTime = 1000;   // TODO���Ե��ڣ���Ҫ����������⣩
    while(!pool->IsClearingPool() || _sessionCnt > 0)
    {
        // �ȴ�������Ϣ
        const Int32 ret = _iocp->WaitForCompletion(*_ioEvent, waitTime);
        
        // ��������
        _CheckSessionHeartbeat();

        if(ret != StatusDefs::Success)
        {
            if(ret == StatusDefs::IOCP_WaitTimeOut)
                continue;

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

        const UInt64 sessionId = _ioEvent->_data._sessionId;

        // 1.�жϻỰ�Ƿ����
        _locker.Lock();
        auto session = _GetSession(sessionId);
        if(!session)
        {// ���ݶ�ʧ��������
            g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] is removed before.\n stack trace back:\n%s")
                                         , sessionId, CrashHandleUtil::FS_CaptureStackBackTrace().c_str());
            _locker.Unlock();
            continue;
        }

        auto iocpSession = session->CastTo<FS_IocpSession>();

        // 2.��������뷢��
        if(IocpDefs::IO_RECV == _ioEvent->_ioData->_ioType)
        {
            if(_ioEvent->_bytesTrans <= 0)
            {// �ͻ��˶Ͽ�����
                g_Log->any<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] IO_TYPE::RECV bytesTrans[%lu] disconnected"
                                               , iocpSession->GetSessionId()
                                               , iocpSession->GetSocket(),
                                               _ioEvent->_bytesTrans);
                g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] IO_TYPE::RECV bytesTrans[%lu] disconnected"
                                               , iocpSession->GetSessionId()
                                               , iocpSession->GetSocket(),
                                               _ioEvent->_bytesTrans);
                _OnGracefullyDisconnect(session);
                _locker.Unlock();
                continue;
            }

            iocpSession->OnRecvSuc(_ioEvent->_bytesTrans, _ioEvent->_ioData);

            // ��Ϣ���ջص�
            _serverCoreRecvSucCallback->Invoke(session, _ioEvent->_bytesTrans);

            _UpdateSessionHeartbeat(session);

            // ����Ͷ�ݽ���
            if(iocpSession->CanPost())
            {
                auto ioData = iocpSession->MakeRecvIoData();
                if(ioData)
                {
                    Int32 st = _iocp->PostRecv(iocpSession->GetSocket(), ioData);
                    if(st != StatusDefs::Success)
                    {
                        iocpSession->ResetPostRecvMask();
                        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] socket[%llu] post recv fail st[%d]")
                                                     , iocpSession->GetSessionId(), iocpSession->GetSocket(), st);
                        _OnGracefullyDisconnect(iocpSession);
                        _locker.Unlock();
                        continue;
                    }
                }
            }
        }
        else if(IocpDefs::IO_SEND == _ioEvent->_ioData->_ioType)
        {
            if(_ioEvent->_bytesTrans <= 0)
            {// �ͻ��˶Ͽ�����
                g_Log->any<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] IO_TYPE::IO_SEND bytesTrans[%lu] disconnected"
                                               , session->GetSessionId()
                                               , session->GetSocket(),
                                               _ioEvent->_bytesTrans);
                g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] sock[%llu] IO_TYPE::IO_SEND bytesTrans[%lu] disconnected"
                                               , session->GetSessionId()
                                               , session->GetSocket(),
                                               _ioEvent->_bytesTrans);
                _OnGracefullyDisconnect(session);
                _locker.Unlock();
                continue;
            }

            auto iocpSession = session->CastTo<FS_IocpSession>();
            iocpSession->OnSendSuc(_ioEvent->_bytesTrans, _ioEvent->_ioData);

            // ��Ϣ���ͻص�
            _serverCoreSendSucCallback->Invoke(session, _ioEvent->_bytesTrans);

            _UpdateSessionHeartbeat(session);

            // ����Ͷ�ݽ���
            if(iocpSession->CanPost() && iocpSession->HasMsgToSend())
            {
                if(!_DoSend(iocpSession))
                {
                    _locker.Unlock();
                    continue;
                }
            }
        }
        else
        {
            g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("undefine io type[%d]."), _ioEvent->_ioData->_ioType);
        }

        // �ͻ����Ѿ��Ͽ�����
        if(iocpSession->IsClose())
            _OnGracefullyDisconnect(iocpSession);

        _locker.Unlock();
    }

    // ����sessions ��ʱҪ��֤handlerҪ��û�ر�
    _ClearSessionsWhenClose();
}

void FS_IocpMsgTransfer::_OnDelayDisconnected(IFS_Session *session)
{
    session->MaskClose();
    if(!session->IsClose())
        session->Close();
}

void FS_IocpMsgTransfer::_OnDisconnected(IFS_Session *session)
{
    session->Close();

    // servercore�յ��Ͽ��ص�
    _serverCoreDisconnectedCallback->Invoke(session);

    session->OnDisconnect();
    session->OnDestroy();

    // �Ƴ��Ự
    Fs_SafeFree(session);
    _sessions.erase(session->GetSessionId());
    --_sessionCnt;
}

bool FS_IocpMsgTransfer::_DoSend(FS_IocpSession *session)
{
    auto ioData = session->MakeSendIoData();
    if(ioData)
    {
        Int32 st = _iocp->PostSend(session->GetSocket(), ioData);
        if(st != StatusDefs::Success)
        {
            session->ResetPostSendMask();
            g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] socket[%llu] post send fail st[%d]")
                                         , session->GetSessionId(), session->GetSocket(), st);
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
    _locker.Lock();
    for(auto &iterSession : _sessions)
    {
        g_Log->e<FS_IocpMsgTransfer>(_LOGFMT_("sessionId[%llu] not destroy when thread ending")
                                     , iterSession.first);
        _OnDisconnected(iterSession.second);
    }
    _locker.Unlock();
}

void FS_IocpMsgTransfer::_UpdateSessionHeartbeat(IFS_Session *session)
{
    _sessionHeartbeatQueue.erase(session);
    session->UpdateHeartBeatExpiredTime();
    _sessionHeartbeatQueue.insert(session);
}

void FS_IocpMsgTransfer::_CheckSessionHeartbeat()
{
    _curTime.FlushTime();
    _locker.Lock();
    for(auto iterSession = _sessionHeartbeatQueue.begin(); 
        iterSession != _sessionHeartbeatQueue.end(); 
        )
    {
        auto session = *iterSession;
        if(session->GetHeartBeatExpiredTime() > _curTime)
            break;

        // ��������
        OnHeartBeatTimeOut(session);
        iterSession = _sessionHeartbeatQueue.erase(iterSession);
    }
    _locker.Unlock();
}
FS_NAMESPACE_END


