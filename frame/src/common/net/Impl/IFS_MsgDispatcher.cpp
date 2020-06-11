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
 * @file  : IFS_MsgDispatcher.cpp
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */

#include "stdafx.h"
#include "FrightenStone/common/net/Impl/IFS_MsgDispatcher.h"
#include "FrightenStone/common/net/Impl/IFS_NetEngine.h"
#include "FrightenStone/common/net/Impl/IFS_Acceptor.h"
#include "FrightenStone/common/net/Defs/FS_ConnectInfo.h"
#include "FrightenStone/common/net/Defs/NetMessageBlock.h"
#include "FrightenStone/common/net/Impl/FS_SessionFactory.h"
#include "FrightenStone/common/net/Defs/IFS_NetBuffer.h"
#include "FrightenStone/common/net/Defs/CancelIoReason.h"

#include "FrightenStone/common/crashhandle/CrashHandle.h"

#pragma region platform relation
#ifdef _WIN32
#include "FrightenStone/common/net/Defs/IocpDefs.h"
#else

#endif
#pragma endregion

#include "FrightenStone/common/memleak/memleak.h"
#include "FrightenStone/common/component/Impl/FS_ThreadPool.h"
#include "FrightenStone/common/component/Impl/TimeWheel/TimeWheel.h"

FS_NAMESPACE_BEGIN
IFS_MsgDispatcher::MessageQueueHandler IFS_MsgDispatcher::_msgBlockHandler[NetMessageType::End] = { NULL };

IFS_MsgDispatcher::~IFS_MsgDispatcher()
{
    STLUtil::DelMapContainer(_sessions);
    Fs_SafeFree(_pool);
    Fs_SafeFree(_timeWheel);
    Fs_SafeFree(_cfgs);
    Fs_SafeFree(_printAlloctorOccupiedInfo);
    Fs_SafeFree(_sessionBufferAlloctor);
    if (_recvMsgBlocks)
        STLUtil::DelListContainer(*_recvMsgBlocks);
    Fs_SafeFree(_recvMsgBlocks);

    for (auto &iterDelegateInfo : _sessionIdRefUserDisconnected)
        STLUtil::DelListContainer(iterDelegateInfo.second);
    _sessionIdRefUserDisconnected.clear();

    //     _CrtMemCheckpoint(&s2);
    //     if(_CrtMemDifference(&s3, &s1, &s2))
    //         _CrtMemDumpStatistics(&s3);
}

Int32 IFS_MsgDispatcher::BeforeStart(const NetEngineTotalCfgs &cfgs)
{
    Int32 st = IFS_EngineComp::BeforeStart(cfgs);
    if (st != StatusDefs::Success)
    {
        g_Log->e<IFS_MsgDispatcher>(_LOGFMT_("IFS_EngineComp::BeforeStart fail st[%d]"), st);
        return st;
    }

    // ��ʼ����Ϣ������
    _InitMsgHandlers();

    _cfgs = new DispatcherCfgs;
    *_cfgs = cfgs._dispatcherCfgs;
    _timeWheel = new TimeWheel(_cfgs->_dispatcherResolutionInterval);
    _pool = new FS_ThreadPool(0, 1);

    // ���������
    _sessionBufferAlloctor = new MemoryAlloctorWithLimit(FS_BUFF_SIZE_DEF
        , _cfgs->_prepareBufferPoolCnt
        , _cfgs->_maxAlloctorBytesPerDispatcher);
    _sessionBufferAlloctor->InitMemory();
    _printAlloctorOccupiedInfo = DelegatePlusFactory::Create(this, &IFS_MsgDispatcher::_PrintAlloctorOccupiedInfo);

    // ������Ϣ����
    _recvMsgBlocks = new std::vector<std::list<FS_MessageBlock *> *>;
    auto generatorQuatity = _concurrentMq->GetGeneratorQuality();
    _recvMsgBlocks->resize(generatorQuatity);
    for (UInt32 i = 0; i < generatorQuatity; ++i)
        (*_recvMsgBlocks)[i] = new std::list<FS_MessageBlock *>;

    // ҵ���߼���ʼ��
    if (_logic)
    {
        _logic->SetDispatcher(this);
        _logic->SetTimeWheel(_timeWheel);
        auto st = _logic->BeforeStart();
        if (st != StatusDefs::Success)
        {
            g_Log->e<IFS_MsgDispatcher>(_LOGFMT_("_logic->BeforeStart error st[%d]"), st);
            return st;
        }
    }

    return StatusDefs::Success;
}


Int32 IFS_MsgDispatcher::Start()
{
    Int32 st = IFS_EngineComp::Start();
    if (st != StatusDefs::Success)
    {
        g_Log->e<IFS_MsgDispatcher>(_LOGFMT_("IFS_EngineComp::Start fail st[%d]"), st);
        return st;
    }

    if (_logic)
    {
        auto st = _logic->Start();
        if (st != StatusDefs::Success)
        {
            g_Log->e<IFS_MsgDispatcher>(_LOGFMT_("_logic->Start error st[%d]"), st);
            return st;
        }
    }

    return StatusDefs::Success;
}

void IFS_MsgDispatcher::AfterStart()
{
    auto task = DelegatePlusFactory::Create(this, &IFS_MsgDispatcher::_OnBusinessProcessThread);
    if (!_pool->AddTask(task, true))
    {
        g_Log->e<IFS_MsgDispatcher>(_LOGFMT_("add task fail"));
    }

    g_Log->sys<IFS_MsgDispatcher>(_LOGFMT_("dispatcher start suc..."));
}

void IFS_MsgDispatcher::WillClose()
{
    if (_isClose)
        return;

    _concurrentMq->NotifyConsumerByGenerator(_consumerId);

    // �߳��˳�
    _pool->Close();
}

void IFS_MsgDispatcher::BeforeClose()
{
    if (_isClose)
        return;

    // ��Ҫ����sessions
    if (_logic)
    {
        _logic->WillClose();
        _logic->BeforeClose();
    }

    IFS_EngineComp::BeforeClose();
}

void IFS_MsgDispatcher::Close()
{
    if (_isClose)
        return;

    _isClose = true;

    if (_logic)
        _logic->Close();

    if (!_recvMsgBlocks->empty())
    {
        UInt32 generatorSize = static_cast<UInt32>(_recvMsgBlocks->size());
        UInt64 blockCount = 0;
        for (UInt32 i = 0; i < generatorSize; ++i)
        {
            auto msgQueue = _recvMsgBlocks->at(i);
            if (!msgQueue)
                continue;

            for (auto &recvMsgBlock : *msgQueue)
            {
                FS_NetMsgBlock *netMsg = recvMsgBlock->CastTo<FS_NetMsgBlock>();
                g_Log->net<IFS_MsgDispatcher>("netmsg _compId[%u] _generatorId[%u] _netMessageBlockType[%d]is unhandled"
                    , netMsg->_compId
                    , netMsg->_generatorId
                    , netMsg->_messageType);
                ++blockCount;
                Fs_SafeFree(recvMsgBlock);
            }
            msgQueue->clear();
        }

        g_Log->w<IFS_MsgDispatcher>(_LOGFMT_("msg block queue has unhandled msgs[%llu]"), blockCount);
    }

    STLUtil::DelListContainer(*_recvMsgBlocks);
    Fs_SafeFree(_recvMsgBlocks);
    for (auto &iterDelegateInfo : _sessionIdRefUserDisconnected)
        STLUtil::DelListContainer(iterDelegateInfo.second);
    _sessionIdRefUserDisconnected.clear();

    IFS_EngineComp::Close();
}

void IFS_MsgDispatcher::AfterClose()
{
    // �ڴ������ռ�������ӡ����
    MemleakMonitor::GetInstance()->UnRegisterMemPoolPrintCallback(_transferThreadId, _printAlloctorOccupiedInfo);

    if (_sessionBufferAlloctor)
        _sessionBufferAlloctor->FinishMemory();
}
// 
// void IFS_MsgDispatcher::SendData(UInt64 sessionId, NetMsg_DataHeader *msg)
// {
//     if (_isClose)
//         return;
// 
//     auto session = GetSession(sessionId);
//     if (!session || !session->CanPost())
//         return;
// 
//     if (!session->PushMsgToSend(msg))
//     {
//         g_Log->w<IFS_MsgDispatcher>(_LOGFMT_("sessionid[%llu] send msg cmd[%hu] len[%hu] fail")
//             , sessionId
//             , msg->_cmd);
//     }
// 
//     if (session->IsPostSend())
//     {
//         auto iterSendQueue = _toPostSend.find(session);
//         g_Log->w<IFS_MsgDispatcher>(_LOGFMT_("session is already post send sessionId[%llu] %s")
//             , sessionId, iterSendQueue == _toPostSend.end() ? "not in post send queue" : "in post send queue");
//         return;
//     }
// 
//     //if(_DoPostSend(session))
//     // send�Ӻ���,�����Ż�
//     _toPostSend.insert(session);
// }

Int32 IFS_MsgDispatcher::AsynConnect(const FS_ConnectInfo &connectInfo)
{
    auto comps = _engine->GetCompByType(EngineCompType::Connector);
    if (!comps || comps->empty())
    {
        g_Log->e<IFS_MsgDispatcher>(_LOGFMT_("lose connectors"));
        return StatusDefs::NetModule_ConnectorsNotExists;
    }

    auto pos = _compId % comps->size();
    auto connector = comps->at(pos)->CastTo<IFS_Connector>();
    return connector->AsynConnect(_consumerId, connectInfo);
}

void IFS_MsgDispatcher::_OnBusinessProcessThread(FS_ThreadPool *pool)
{
    // _timeWheel->GetModifiedResolution(_resolutionInterval);
    const auto &resolutionIntervalSlice = _cfgs->_dispatcherResolutionInterval;
    bool hasMsg = false;

    // ��ʼ������
    _transferThreadId = SystemUtil::GetCurrentThreadId();
    MemleakMonitor::GetInstance()->RegisterMemPoolPrintCallback(_transferThreadId, _printAlloctorOccupiedInfo);

    // �ȴ��������ready
    MaskReady(true);
    g_Log->sys<IFS_MsgDispatcher>(_LOGFMT_("business process monitor is working..."));
    EngineCompsMethods::WaitForAllCompsReady(_engine);
    // ���ط�����׼������
    _OnLocalServerReady();
    // startup
    _OnWillStartup();

    while (pool->IsPoolWorking() || hasMsg)
    {
        // 1.�ȴ���Ϣ����
        _concurrentMq->WaitForPoping(_consumerId, _recvMsgBlocks, hasMsg, static_cast<ULong>(resolutionIntervalSlice.GetTotalMilliSeconds()));

        // 2.��ִ�ж�ʱ���¼�
        _timeWheel->RotateWheel();

        // 3.�������
        _CheckHeartbeat();

        // ���첽��Ϣ����ȡ���첽������ɷ����¼� TODO: ��Ҫ���첽������У������߳�����
        // ������ɵ��첽�¼� TODO:

        // 4.��ִ��ҵ���¼�
        if (hasMsg)
            _OnBusinessProcessing();

        // 5. Ͷ����Ϣ
        _PostEvents();

        // 6.�Ƴ�sessions
        _WillRemoveSessions();

        // Ͷ��ҵ��������첽�����¼� TODO: ��������������Ϊ���ܻ����������Ҫ�ڵ�
        // _timeWheel->GetModifiedResolution(_resolutionInterval);
    }

    _concurrentMq->PopImmediately(_consumerId, _recvMsgBlocks, hasMsg);
    _RunAFrame(hasMsg);

    MaskReady(false);
#ifndef _WIN32
    _CancelAllSessionIosAndWaitForTransferIoFinish();
#endif

    // ����Ự
     _ClearAllSessions();

    // ϵͳ�ط�
    _CloseLogicSvc();
    g_Log->sys<IFS_MsgDispatcher>(_LOGFMT_("dispatcher process thread end compId[%u],threadId[%llu]")
        , _compId, SystemUtil::GetCurrentThreadId());
}

void IFS_MsgDispatcher::_RunAFrame(bool hasMsg)
{
    // 2.��ִ�ж�ʱ���¼�
    _timeWheel->RotateWheel();

    // 3.�������
    _CheckHeartbeat();

    // ���첽��Ϣ����ȡ���첽������ɷ����¼� TODO: ��Ҫ���첽������У������߳�����
    // ������ɵ��첽�¼� TODO:

    // 4.��ִ��ҵ���¼�
    if (hasMsg)
        _OnBusinessProcessing();

    // 5. Ͷ����Ϣ
    _PostEvents();

    // 6.�Ƴ�sessions
    _WillRemoveSessions();
}

void IFS_MsgDispatcher::_CheckHeartbeat()
{
    _curTime.FlushTime();
    for (auto iterSession = _sessionHeartbeatQueue.begin(); iterSession != _sessionHeartbeatQueue.end();)
    {
        auto session = *iterSession;
        if (session->GetHeartBeatExpiredTime() > _curTime)
            break;

        _engine->HandleCompEv_HeartBeatTimeOut();
        iterSession = _sessionHeartbeatQueue.erase(iterSession);
        session->MaskClose();
        _toRemove.insert(session);

#ifndef _WIN32
      g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("session heartbeat time out: %s"), session->ToString().c_str());
#endif
    }
}

void IFS_MsgDispatcher::_OnBusinessProcessing()
{
    // ����������ת�Ƶ�������
    FS_NetMsgBlock *netMsgBlock = NULL;
    UInt64 sessionId = 0;
    for (auto iterGeneratorQueue = _recvMsgBlocks->begin(); iterGeneratorQueue != _recvMsgBlocks->end(); ++iterGeneratorQueue)
    {
        auto *blockQueue = *iterGeneratorQueue;
        if (blockQueue->empty())
            continue;

        for (auto iterBlock = blockQueue->begin(); iterBlock != blockQueue->end();)
        {
            netMsgBlock = (*iterBlock)->CastTo<FS_NetMsgBlock>();
            (this->*_msgBlockHandler[netMsgBlock->_messageType])(netMsgBlock);
            Fs_SafeFree(netMsgBlock);
            iterBlock = blockQueue->erase(iterBlock);
        }
    }
}

void IFS_MsgDispatcher::_PostEvents()
{
    // post recv
    FS_Session *session = NULL;
    for (auto iterRecv = _toPostRecv.begin(); iterRecv != _toPostRecv.end();)
    {
        session = *iterRecv;
        if (session->CanPost() &&
            !session->IsPostRecv())
            _DoPostRecv(session);

        iterRecv = _toPostRecv.erase(iterRecv);
    }

    // post send
    for (auto iterSend = _toPostSend.begin(); iterSend != _toPostSend.end();)
    {
        session = *iterSend;
        if (!session->IsPostSend() &&
            session->HasMsgToSend())
            _DoPostSend(session);

        iterSend = _toPostSend.erase(iterSend);
    }
}

void IFS_MsgDispatcher::_WillRemoveSessions()
{
    FS_Session *session = NULL;
    for (auto iterSession = _toRemove.begin(); iterSession != _toRemove.end();)
    {
        session = *iterSession;
        _RemoveFromPostRecvQueue(session);
        _RemoveFromPostSendQueue(session);

        // ׼���Ƴ�,�ȴ�transfer������ʽ�Ƴ�session
        _RemoveSessionGracefully(session);
        iterSession = _toRemove.erase(iterSession);
    }
}

void IFS_MsgDispatcher::_CloseLogicSvc()
{
    if (_logic)
    {
        _logic->WillClose();
        _logic->BeforeClose();
        _logic->Close();
        _logic->AfterClose();
    }
}

void IFS_MsgDispatcher::_ClearAllSessions()
{
#ifdef _WIN32
    for (auto iterSession = _sessions.begin(); iterSession != _sessions.end();)
    {
        auto session = iterSession->second;
        session->EnableDisconnect();
        _OnSessionDisconnected(session);
        iterSession = _sessions.erase(iterSession);
    }

#else
    // linux �²���_OnSessionDisconnected��Ϊ�����ط��Ѿ�����
    for (auto iterSession = _sessions.begin(); iterSession != _sessions.end();)
    {
        Fs_SafeFree(iterSession->second);
        iterSession = _sessions.erase(iterSession);
    }
#endif
}

void IFS_MsgDispatcher::_OnLocalServerReady()
{
    if (_logic)
        _logic->OnLocalServerReady();
}

void IFS_MsgDispatcher::_OnWillStartup()
{
    if (_logic)
        _logic->WillStartup();
}

#pragma region platform relation
#ifndef _WIN32
void IFS_MsgDispatcher::_CancelAllSessionIosAndWaitForTransferIoFinish()
{
    for (auto iterSession = _sessions.begin(); iterSession != _sessions.end(); ++iterSession)
    {
        auto session = iterSession->second;
        session->EnableDisconnect();
        session->CancelRecvIoAndMaskClose(CancelIoReason::CloseDispatcher, false);
        session->CancelSendIoAndMaskClose(CancelIoReason::CloseDispatcher, false);
        _OnSessionDisconnectedNoFree(session);
    }

    // ���ȴ����ܻ���ֻ��汻�ͷ�,���Ұָ��
    EngineCompsMethods::WaitForAllTransferIoFinish(_engine);
}

#endif
#pragma endregion

bool IFS_MsgDispatcher::_DoPostRecv(FS_Session *session)
{// �ⲿ�����ж�canpost
    // session����post�����Ѿ�post��ɲ����ظ�post
    if (session->IsPostRecv())
    {
        g_Log->w<IFS_MsgDispatcher>(_LOGFMT_("sessionId[%llu] is already posted recv."), session->GetSessionId());
        return true;
    }

    Int32 st = session->PostRecv();
    if (st != StatusDefs::Success)
    {
        g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("PostRecv fail st[%d] sessionId[%llu]"), session->GetSessionId());
        session->MaskClose();
        _toRemove.insert(session);
        return false;
    }

#ifndef _WIN32
   // g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("suc post a recv msg sessionId[%llu]"), session->GetSessionId());
#endif
    return true;
}

bool IFS_MsgDispatcher::_DoPostSend(FS_Session *session)
{// �ⲿ�����ж�canpost
    // �ж��Ƿ����post,�Ƿ��Ѿ�postsend,�Ƿ�����Ϣ���� �ⲿȥ�ж�,�������
//     if (session->IsPostSend()
//         || !session->HasMsgToSend())
//     {
//         g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("sessionId[%llu] already posted or has no msg to send"), session->GetSessionId());
//         return true;
//     }

    Int32 st = session->PostSend();
    if (st != StatusDefs::Success)
    {
        g_Log->w<IFS_MsgDispatcher>(_LOGFMT_("PostSend fail st[%d] sessionId[%llu]"), session->GetSessionId());
        session->MaskClose();
        _toRemove.insert(session);
        return false;
    }

#ifndef _WIN32
    // g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("suc post a send msg sessionId[%llu]"), session->GetSessionId());
#endif
    return true;
}

void IFS_MsgDispatcher::_RemoveSessionGracefully(FS_Session *session)
{// ����close��socket�����ȴ�transfer����, ondisconnected���������ĶϿ�
    const auto sessionId = session->GetSessionId();
    g_Log->net<IFS_MsgDispatcher>("sessionId[%llu] sock[%llu] disconnected address<%s> prepare remove"
        , sessionId
        , session->GetSocket()
        , session->GetAddr()->ToString().c_str());

#ifndef _WIN32
//     g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("sessionId[%llu] sock[%llu] disconnected address<%s> prepare remove")
//         , sessionId
//         , session->GetSocket()
//         , session->GetAddr()->ToString().c_str());
#endif

    session->MaskClose();
    if (!session->CanDisconnect())
    {// ����post event��û���ز��������ر�Ӧ����cancel,��֤�Ѿ���ɵ�io�¼���������
#ifdef _WIN32
        session->CancelPostedEventsAndMaskClose();
#else
        // ����post event��û���ز��������ر�Ӧ����cancel,��֤�Ѿ���ɵ�io�¼���������
        session->CancelRecvIoAndMaskClose(CancelIoReason::ForceClosed, true);
        session->CancelSendIoAndMaskClose(CancelIoReason::ForceClosed, true);
#endif
    }
    else
    {
        // close����ڻ�����ȫ���,����post��δ���������ֶ��������
        _OnSessionDisconnected(session);
        _sessions.erase(sessionId);
    }
}

#pragma region net msg handle

void IFS_MsgDispatcher::_OnMsgBlockConnectionFailure(FS_NetMsgBlock *msgBlock)
{
    auto connectedMsgBlock = msgBlock->CastTo<PostConnectionFailureToDispatcherMsgBlock>();
    if (_logic)
        _logic->OnConnectFailureRes(connectedMsgBlock->_connectInfo, connectedMsgBlock->_resultCode);
}

void IFS_MsgDispatcher::_OnSessionDisconnectedNoFree(FS_Session *session)
{
    // ��session������ʽ�Ƴ�session
    const auto sessionId = session->GetSessionId();
    g_Log->net<IFS_MsgDispatcher>("sessionId[%llu] sock[%llu] disconnected address<%s>"
        , sessionId
        , session->GetSocket()
        , session->GetAddr()->ToString().c_str());

#ifndef _WIN32
//     g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("sessionId[%llu] sock[%llu] disconnected address<%s>")
//         , sessionId
//         , session->GetSocket()
//         , session->GetAddr()->ToString().c_str());
#endif

    // �����ص�
    std::list<IDelegate<void, UInt64> *> *delagates = NULL;
    auto iterDisconnected = _sessionIdRefUserDisconnected.find(sessionId);
    if (iterDisconnected != _sessionIdRefUserDisconnected.end())
        delagates = &iterDisconnected->second;

    // ҵ���Ͽ��Ự
    if (_logic)
        _logic->OnSessionDisconnected(sessionId, delagates);

    if (delagates)
    {
        if (!delagates->empty())
            STLUtil::DelListContainer(*delagates);

        _sessionIdRefUserDisconnected.erase(iterDisconnected);
    }

    // ����ص�
    _engine->HandleCompEv_Disconnected(sessionId
        , session->GetSrcAcceptorCompId()
        , session->GetTransferCompId());

    // session�Ͽ�
    session->Close();
    session->OnDisconnect();
    session->OnDestroy();

    // �Ƴ���Դ
    _sessionHeartbeatQueue.erase(session);
    _toPostRecv.erase(session);
    _toPostSend.erase(session);
    --_curSessionQuatity;
}

void IFS_MsgDispatcher::_OnSessionConnected(SessionConnectedNetMsgBlock *connectedMsg)
{
#ifndef _WIN32
//     g_Log->i<IFS_MsgDispatcher>(_LOGFMT_(" new session is coming. _OnSessionConnected connectedMsg info:\n[%s]")
//         , connectedMsg->ToString().c_str());
#endif

    const UInt64 sessionId = connectedMsg->_sessionId;
    ++_curSessionQuatity;

    // ����session���󲢳�ʼ��
    BuildSessionInfo buldData;
    buldData._sessionId = sessionId;
    buldData._transferCompId = connectedMsg->_compId;
    buldData._acceptorCompId = connectedMsg->_acceptorCompId;
    buldData._connectorCompId = connectedMsg->_connectorCompId;
    buldData._protocolPort = connectedMsg->_protocolPort;
    buldData._srcCompType = connectedMsg->_srcCompType;
    buldData._sock = connectedMsg->_sock;
    buldData._addrInfo = &(connectedMsg->_addrInfo);
    buldData._memAlloctor = _sessionBufferAlloctor;
    buldData._heartbeatIntervalMicroSeconds = _cfgs->_heartbeatDeadTimeMsInterval * Time::_microSecondPerMilliSecond;
    auto newSession = FS_SessionFactory::Create(buldData);
    _sessions.insert(std::make_pair(sessionId, newSession));
    _UpdateSessionHeartbeat(newSession);

    // ���ò���
#ifdef _WIN32
    newSession->Bind(reinterpret_cast<FS_Iocp *>(connectedMsg->_iocp));
#else
    newSession->Bind(reinterpret_cast<FS_EpollTransferPoller *>(connectedMsg->_poller));
#endif

    newSession->OnConnect();
    auto addr = newSession->GetAddr();
    g_Log->net<IFS_MsgDispatcher>("new session connected: id<%llu>,socket<%llu>,remote ip[%s:%hu]"
        " protocol port[%hu], src comp type[%d:%s]"
        , sessionId
        , newSession->GetSocket()
        , addr->GetAddr().c_str()
        , addr->GetPort()
        , newSession->GetProtocolPort()
        , newSession->GetSrcCompType()
        , EngineCompType::GetStr(newSession->GetSrcCompType()).c_str());

#ifndef _WIN32
//     g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("new session connected: id<%llu>,socket<%llu>,remote ip[%s:%hu]"
//         " protocol port[%hu], src comp type[%d:%s]")
//         , sessionId
//         , newSession->GetSocket()
//         , addr->GetAddr().c_str()
//         , addr->GetPort()
//         , newSession->GetProtocolPort()
//         , newSession->GetSrcCompType()
//         , EngineCompType::GetStr(newSession->GetSrcCompType()).c_str());
#endif

    // ���öϿ��ص�
    auto userDisconnectedDelegates = connectedMsg->_onUserDisconnectedRes;
    if (userDisconnectedDelegates)
    {
        auto iterDisconnected = _sessionIdRefUserDisconnected.find(sessionId);
        if (iterDisconnected == _sessionIdRefUserDisconnected.end())
            iterDisconnected = _sessionIdRefUserDisconnected.insert(std::make_pair(sessionId, std::list<IDelegate<void, UInt64>*>())).first;
        iterDisconnected->second.push_back(userDisconnectedDelegates);
        connectedMsg->_onUserDisconnectedRes = NULL;
    }

    const Int32 st = _logic->OnSessionConnected(sessionId, connectedMsg->_stub);
    if (st != StatusDefs::Success)
    {
        g_Log->w<IFS_MsgDispatcher>(_LOGFMT_("_logic->OnSessionConnected fail st[%d] sessionId[%llu]"), st, sessionId);

        // ʧ�����Ƴ�session
        _DelayCloseSession(newSession);
        return;
    }

    // post recv�п��ܻ�Ͽ�����
    if (newSession->IsValid())
    {
        if (!_DoPostRecv(newSession))
            g_Log->w<IFS_MsgDispatcher>(_LOGFMT_("_DoPostRecv fail sessionId[%llu]"), sessionId);
    }
}

void IFS_MsgDispatcher::_OnSessionMsgHandle(FS_Session *session)
{
    session->InvokeMsgHandler();
    if(session->IsValid())
    {
        // ��Ϣ�������������Ϣ��Ҫ������ʱ����
        if (!session->IsPostSend() &&
            session->HasMsgToSend())
            _DoPostSend(session);
    }

//     auto recvBuffer = session->CastToRecvBuffer();
//     const Byte8 *buffer = NULL;
//     const UInt64 sessionId = session->GetSessionId();
// 
//     // TODO:������logic����,��ΪЭ���Ӧ�ò�ϢϢ��أ��ײ㲻����������
//     _logic->OnSessionMsgHandle(session);
//     while (recvBuffer->HasMsg())
//     {
//         if (!_msgDecoder->Decode(recvBuffer->GetData()))
//         {
//             g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("Decode error sessionId[%llu]")
//                 , sessionId);
//             _DelayCloseSession(session);
//             break;
//         }
// 
//         g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("cmd[%d] msg iscoming len[%u]")
//             , _msgDecoder->GetCmd(), _msgDecoder->GetMsgLen());
// 
//         // TODO:����һ��Ϣҵ���߼�����
//         if (_logic)
//             _logic->OnMsgDispatch(sessionId, _msgDecoder);
// 
//         _engine->HandleCompEv_RecvMsgAmount();
//         recvBuffer->PopFront(_msgDecoder->GetMsgLen());
// 
//         if (session->IsDelayClose())
//             break;
//     }
}
#ifdef _WIN32

void IFS_MsgDispatcher::_OnMsgArrived(IocpMsgArrivedMsgBlock *arrivedMsg)
{
    IoEvent *ioEv = arrivedMsg->_ioEv;
    IoDataBase *ioData = reinterpret_cast<IoDataBase *>(ioEv->_ioData);
    Int32 errorCode = arrivedMsg->_errorCode;
    const Int64 transferBytes = ioEv->_bytesTrans;
    if (errorCode != StatusDefs::Success)
    {
        g_Log->w<IFS_MsgDispatcher>(_LOGFMT_("arrived msg errorCode[%d]"), errorCode);
        return;
}

    if (ioData->_sessionId == static_cast<UInt64>(IocpDefs::IO_QUIT))
    {// transfer�����˳�
        return;
    }

    const UInt64 sessionId = ioEv->_sessionId;
    auto session = GetSession(sessionId);
    if (!session)
    {// ���ݶ�ʧ,���������Ѿ��Ͽ������ˣ�������
//         g_Log->net<FS_IocpMsgTransfer>("sessionId[%llu] is removed before.\n stack trace back:\n%s"
//                                      , sessionId, CrashHandleUtil::FS_CaptureStackBackTrace().c_str());
        return;
    }

    // 2.��������뷢�� TODO:�������ɻص����飬�������
    const Int32 ioType = ioData->_ioType;
    if (IocpDefs::IO_RECV == ioType)
    {
        if (transferBytes <= 0)
        {// �ͻ��˶Ͽ�����
            g_Log->net<IFS_MsgDispatcher>("sessionId[%llu] sock[%llu] IO_TYPE::RECV bytesTrans[%llu] will disconnect IsDelayClose[%d]"
                , sessionId
                , session->GetSocket()
                , transferBytes
                , session->IsDelayClose());

            session->ResetPostRecvMask();
            session->CancelPostedEventsAndMaskClose();
            _toRemove.insert(session);
            return;
        }

        session->OnRecvSuc(transferBytes, ioData);

        // �յ���Ϣ�ص�
        _engine->HandleCompEv_RecvMsg(transferBytes);
        if (session->IsValid())
        {
            _OnSessionMsgHandle(session);
            _UpdateSessionHeartbeat(session);
        }

        // ���½�������
        if (session->CanPost())
            _toPostRecv.insert(session);
    }
    else if (IocpDefs::IO_SEND == ioType)
    {
        if (transferBytes <= 0)
        {// �ͻ��˶Ͽ�����
            g_Log->net<IFS_MsgDispatcher>("sessionId[%llu] sock[%llu] IO_TYPE::IO_SEND bytesTrans[%lld] disconnected IsDelayClose[%d]"
                , sessionId
                , session->GetSocket()
                , transferBytes
                , session->IsDelayClose());
            session->ResetPostSendMask();
            session->CancelPostedEventsAndMaskClose();
            _toRemove.insert(session);
            return;
        }

        session->OnSendSuc(transferBytes, ioData);

        // ��Ϣ���ͻص�
        _engine->HandleCompEv_SendMsg(transferBytes);

        if (session->IsValid())
        {
            // TODO:�б���񷢰��ķ��գ����������������Э��+Ƶ����������+�����ɹ�ʱ����ʱ���
            _UpdateSessionHeartbeat(session);
        }

        // ����Ͷ�ݷ���
        if (session->CanPost() &&
            !session->IsPostSend() &&
            session->HasMsgToSend())
            _toPostSend.insert(session);
    }
    else
    {
        session->EnableDisconnect();
        session->Close();
        _toRemove.insert(session);
        g_Log->e<IFS_MsgDispatcher>(_LOGFMT_("undefine io type[%d]."), ioType);
    }
}



#else
void IFS_MsgDispatcher::_OnMsgRecv(EpollRecvDataMsgBlock *recvMsg)
{
//     g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("a recv io msg from transfer generatorId[%u] compId[%u] is coming handle sessionId[%llu] transferBytes[%lld]")
//         , recvMsg->_generatorId, recvMsg->_compId, recvMsg->_sessionId, recvMsg->_transferBytes);

    EpollIoData *ioData = reinterpret_cast<EpollIoData *>(recvMsg->_ioData);
    const UInt64 sessionId = recvMsg->_sessionId;
    const Int64 transferBytes = recvMsg->_transferBytes;
    const bool isRegisterInMonitor = recvMsg->_isSessionInRecverMonitor;

    auto session = GetSession(sessionId);
    if (!session)
    {// ���ݶ�ʧ,���������Ѿ��Ͽ������ˣ�������
        g_Log->net<IFS_MsgDispatcher>("_OnMsgRecv sessionId[%llu] is removed before.\n stack trace back:\n%s"
            , sessionId, CrashHandleUtil::FS_CaptureStackBackTrace().c_str());

//         g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("_OnMsgRecv sessionId[%llu] is removed before.\n stack trace back:\n%s")
//             , sessionId, CrashHandleUtil::FS_CaptureStackBackTrace().c_str());
        return;
    }

    session->ResetPostRecvMask();
    if (session->IsDelayClose() || transferBytes <= 0)
    {// �ͻ��˶Ͽ�����
        g_Log->net<IFS_MsgDispatcher>("_OnMsgRecv sessionId[%llu] sock[%llu] bytesTrans[%llu] will disconnect IsDelayClose[%d] sessionInfo:\n%s"
            , sessionId
            , session->GetSocket(),
            transferBytes,
            session->IsDelayClose()
            , session->ToString().c_str());
//         g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("_OnMsgRecv sessionId[%llu] sock[%llu] bytesTrans[%llu] will disconnect IsDelayClose[%d] sessionInfo:\n%s")
//             , sessionId
//             , session->GetSocket(),
//             transferBytes,
//             session->IsDelayClose()
//             , session->ToString().c_str());

        if (!isRegisterInMonitor) // �Ѿ�����recvע������п��Բ���cancel
            session->SetRecvIoCanceled();
        _toRemove.insert(session);
        return;
    }

    session->OnRecvSuc(transferBytes, ioData);

    // �յ���Ϣ�ص�
    _engine->HandleCompEv_RecvMsg(transferBytes);

    if (session->IsValid())
    {
        _OnSessionMsgHandle(session);
        _UpdateSessionHeartbeat(session);
    }

    // ���½�������
    if (session->CanPost())
        _toPostRecv.insert(session);
}

void IFS_MsgDispatcher::_OnMsgSendSuc(EpollSendSucMsgBlock *sendSucMsg)
{
//     g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("a send suc io msg from transfer generatorId[%u] compId[%u] is coming handle sessionId[%llu] transferBytes[%lld]")
//         , sendSucMsg->_generatorId, sendSucMsg->_compId, sendSucMsg->_sessionId, sendSucMsg->_transferBytes);

    EpollIoData *ioData = reinterpret_cast<EpollIoData *>(sendSucMsg->_ioData);
    const UInt64 sessionId = sendSucMsg->_sessionId;
    const Int64 transferBytes = sendSucMsg->_transferBytes;
    const bool isRegisterInMonitor = sendSucMsg->_isSessionInSenderMonitor;

    auto session = GetSession(sessionId);
    if (!session)
    {// ���ݶ�ʧ,���������Ѿ��Ͽ������ˣ�������
        g_Log->net<IFS_MsgDispatcher>("sessionId[%llu] is removed before.\n stack trace back:\n%s"
            , sessionId, CrashHandleUtil::FS_CaptureStackBackTrace().c_str());

//         g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("sessionId[%llu] is removed before.\n stack trace back:\n%s")
//             , sessionId, CrashHandleUtil::FS_CaptureStackBackTrace().c_str());
        return;
    }

    session->ResetPostSendMask();
    if (session->IsDelayClose() || transferBytes <= 0)
    {// �ͻ��˶Ͽ�����
        g_Log->net<IFS_MsgDispatcher>("_OnMsgSendSuc sessionId[%llu] sock[%llu] bytesTrans[%lld] disconnected IsDelayClose[%d], sessionInfo:\n%s"
            , sessionId
            , session->GetSocket()
            , transferBytes
            , session->IsDelayClose()
            , session->ToString().c_str());

//         g_Log->i<IFS_MsgDispatcher>(_LOGFMT_("_OnMsgSendSuc sessionId[%llu] sock[%llu] bytesTrans[%lld] disconnected IsDelayClose[%d], sessionInfo:\n%s")
//             , sessionId
//             , session->GetSocket()
//             , transferBytes
//             , session->IsDelayClose()
//             , session->ToString().c_str());

        if (!isRegisterInMonitor)
            session->SetSendIoCanceled();

        _toRemove.insert(session);
        return;
    }

    session->OnSendSuc(transferBytes, ioData);

    // ��Ϣ���ͻص�
    _engine->HandleCompEv_SendMsg(transferBytes);

    if (session->IsValid())
    {
        // TODO:�б���񷢰��ķ��գ����������������Э��+Ƶ����������+�����ɹ�ʱ����ʱ���
        _UpdateSessionHeartbeat(session);
    }

    // ����Ͷ�ݷ���
    if (session->CanPost() &&
        !session->IsPostSend() &&
        session->HasMsgToSend())
        _toPostSend.insert(session);
}
#endif
#pragma endregion

void IFS_MsgDispatcher::_PrintAlloctorOccupiedInfo()
{
    FS_String memInfo;
    memInfo.AppendFormat("\n��dispatcher alloctor occupied info��\n");
    memInfo.AppendFormat("dispatcher compId[%u] threadId[%llu] alloctor occupied info:[", _compId, _transferThreadId);
    _sessionBufferAlloctor->MemInfoToString(memInfo);
    memInfo.AppendFormat(" ]");
    memInfo.AppendFormat("\n��+++++++++++++++++++++++++ End ++++++++++++++++++++++++++��\n");
    g_Log->mempool("%s", memInfo.c_str());
}

void IFS_MsgDispatcher::_InitMsgHandlers()
{
    _msgBlockHandler[NetMessageType::SessionConnected] = &IFS_MsgDispatcher::_OnMsgBlockNetSessionConnected;
    _msgBlockHandler[NetMessageType::PostConnectionFailureToDispatcher] = &IFS_MsgDispatcher::_OnMsgBlockConnectionFailure;

#ifdef _WIN32
    _msgBlockHandler[NetMessageType::IocpMsgArrived] = &IFS_MsgDispatcher::_OnMsgBlockNetMsgArrived;
#else
    _msgBlockHandler[NetMessageType::RecvDataEv] = &IFS_MsgDispatcher::_OnMsgBlockRecvData;
    _msgBlockHandler[NetMessageType::SendSucEv] = &IFS_MsgDispatcher::_OnMsgBlockSendSuc;
#endif
}

FS_NAMESPACE_END

