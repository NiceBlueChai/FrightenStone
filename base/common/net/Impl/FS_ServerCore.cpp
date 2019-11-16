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
 * @file  : FS_ServerCore.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/FS_ServerCore.h"
#include "base/common/net/Impl/IFS_ServerConfigMgr.h"
#include "base/common/net/Impl/FS_ServerConfigMgrFactory.h"
#include "base/common/net/Impl/FS_ConnectorFactory.h"
#include "base/common/net/Impl/FS_MsgTransferFactory.h"
#include "base/common/net/Impl/FS_MsgDispatcherFactory.h"
#include "base/common/net/Impl/FS_SessionMgr.h"
#include "base/common/net/Impl/IFS_Connector.h"
#include "base/common/net/Impl/IFS_MsgTransfer.h"
#include "base/common/net/Impl/IFS_MsgDispatcher.h"
#include "base/common/net/Impl/IFS_Connector.h"
#include "base/common/net/Impl/IFS_Session.h"
#include "base/common/net/Impl/FS_Addr.h"
#include "base/common/net/Impl/IFS_BusinessLogic.h"
#include "base/common/net/protocol/protocol.h"
#include "base/common/net/Impl/FS_IocpSession.h"
#include "base/common/net/Defs/FS_IocpBuffer.h"
#include "base/common/net/Defs/BriefSessionInfo.h"

#include "base/common/status/status.h"
#include "base/common/log/Log.h"
#include "base/common/assist/utils/utils.h"
#include "base/common/crashhandle/CrashHandle.h"
#include "base/common/socket/socket.h"
#include "base/common/component/Impl/FS_CpuInfo.h"
#include "base/common/component/Impl/FS_ThreadPool.h"
#include "base/common/memleak/memleak.h"

fs::FS_ServerCore *g_ServerCore = NULL;
fs::IFS_ServerConfigMgr *g_SvrCfg = NULL;
fs::IFS_MsgDispatcher *g_Dispatcher = NULL;
fs::IFS_BusinessLogic *g_Logic = NULL;
fs::ConcurrentMessageQueue *g_net2LogicMessageQueue = NULL;

FS_NAMESPACE_BEGIN
FS_ServerCore::FS_ServerCore()
    :_serverConfigMgr(NULL)
    ,_cpuInfo(new FS_CpuInfo)
    ,_connector(NULL)
    ,_pool(NULL)
    ,_curSessionConnecting{0}
    ,_sessionConnectedBefore{0}
    ,_sessionDisconnectedCnt{0}
    ,_recvMsgCountPerSecond{0}
    ,_recvMsgBytesPerSecond{0}
    ,_sendMsgBytesPerSecond{0}
    ,_heartbeatTimeOutDisconnected{0}
    ,_isInit(false)
{
    g_ServerCore = this;
}

FS_ServerCore::~FS_ServerCore()
{
    STLUtil::DelVectorContainer<decltype(_logics), AssistObjsDefs::SelfRelease>(_logics);
    Fs_SafeFree(_pool);
    Fs_SafeFree(_connector);
    STLUtil::DelVectorContainer(_msgDispatchers);
    STLUtil::DelVectorContainer(_msgTransfers);
    Fs_SafeFree(_cpuInfo);
    Fs_SafeFree(_serverConfigMgr);
    g_ServerCore = NULL;
}

#pragma region api
Int32 FS_ServerCore::Init()
{
    if(_isInit)
        return StatusDefs::Success;

    // 1.ʱ��
    TimeUtil::SetTimeZone();

    // 2.����ʶ��
    SmartVarRtti::InitRttiTypeNames();

    // 3.��ʼ���ֲ߳̾��洢���
    Int32 ret = FS_TlsUtil::CreateUtilTlsHandle();
    if(ret != StatusDefs::Success)
    {
        FS_String str;
        str.AppendFormat("CreateUtilTlsHandle fail ret[%d]", ret);
        ASSERTBOX(str.c_str());
        return ret;
    }

    // 4.log��ʼ��
    ret = g_Log->InitModule(NULL);
    if(ret != StatusDefs::Success)
    {
        FS_String str;
        str.AppendFormat("log init fail ret[%d]", ret);
        ASSERTBOX(str.c_str());
        return ret;
    }

    // 5. crash dump switch start
    ret = fs::CrashHandleUtil::InitCrashHandleParams();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("init crash handle params fail ret[%d]"), ret);
        return ret;
    }

    // cpu��Ϣ��ʼ��
    if(!_cpuInfo->Initialize())
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("Initialize cpuinfo fail"));
        return StatusDefs::Failed;
    }

    // 6.Socket����
    ret = SocketUtil::InitSocketEnv();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("InitSocketEnv fail ret[%d]"), ret);
        return ret;
    }

    // 7. ��ȡ����
    ret = _ReadConfig();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("_ReadConfig fail ret[%d]"), ret);
        return ret;
    }

    _isInit = true;

    return StatusDefs::Success;
}

Int32 FS_ServerCore::Start(std::vector<IFS_BusinessLogic *> &businessLogic)
{
    if(!_isInit)
        return StatusDefs::NotInit;

    // 8.�ڴ���
    g_MemleakMonitor->Start();

    // 9.���������
    _pool = new FS_ThreadPool(0, 1);
    auto task = DelegatePlusFactory::Create(this, &FS_ServerCore::_OnSvrRuning);
    if(!_pool->AddTask(task, true))
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("add task fail"));
        return StatusDefs::FS_ServerCore_StartFailOfSvrRuningTaskFailure;
    }
    
    // 10.ҵ���߼�
    _logics = businessLogic;

    // 11.����������ģ��
    Int32 ret = _CreateNetModules();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("_CreateNetModules fail ret[%d]"), ret);
        return ret;
    }

    // 12.ע��ӿ�
    _RegisterToModule();

    // 13.����ǰ...
    ret = _BeforeStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("_BeforeStart fail ret[%d]"), ret);
        return ret;
    }

    // 14.start ģ��
    ret = _StartModules();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("_StartModules fail ret[%d]"), ret);
        return ret;
    }

    // 15.onstart
    ret = _OnStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("_OnStart fail ret[%d]"), ret);
        return ret;
    }

    // 16._AfterStart
    ret = _AfterStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("_AfterStart fail ret[%d]"), ret);
        return ret;
    }

    return StatusDefs::Success;
}

void FS_ServerCore::Wait()
{
    _waitForClose.Lock();
    _waitForClose.Wait();
    _waitForClose.Unlock();
}

void FS_ServerCore::Close()
{
    if(!_isInit)
        return;

    _isInit = false;

    // �Ͽ���������
    _WillClose();

    // �����Ը�ģ���Ƴ���Դ
    _BeforeClose();

    // �Ƴ�����������ģ��
    _connector->Close();
    for(auto &msgTransfer : _msgTransfers)
        msgTransfer->Close();

    for(auto &dispatcher : _msgDispatchers)
        dispatcher->Close();

    _messageQueue->BeforeClose();
    const Int32 senderMqSize = static_cast<Int32>(_senderMessageQueue.size());
    for(Int32 i = 0; i < senderMqSize; ++i)
        _senderMessageQueue[i]->BeforeClose();

    _messageQueue->Close();
    for(Int32 i = 0; i < senderMqSize; ++i)
        _senderMessageQueue[i]->Close();

    _pool->Close();

    // ���һ��ɨ��
    TimeSlice timeSlice;
    timeSlice = Time::_microSecondPerSecond;
    _PrintSvrLoadInfo(timeSlice);

    // �����
    _AfterClose();

    // STLUtil::DelVectorContainer(_logics);
    SocketUtil::ClearSocketEnv();

    STLUtil::DelVectorContainer<decltype(_logics), AssistObjsDefs::SelfRelease>(_logics);
    Fs_SafeFree(_pool);
    Fs_SafeFree(_connector);
    STLUtil::DelVectorContainer(_msgDispatchers);
    STLUtil::DelVectorContainer(_msgTransfers);
    Fs_SafeFree(_cpuInfo);
    Fs_SafeFree(_serverConfigMgr);
    g_ServerCore = NULL;

    // ��ǰ��־ȫ������
    g_MemleakMonitor->Finish();
    g_Log->FlushAllFile();
    g_Log->FinishModule();
    g_MemoryPool->FinishPool();
}

#pragma endregion

/* �����¼� */
#pragma region
void FS_ServerCore::_OnConnected(const BriefSessionInfo &sessionInfo)
{// ֻ��connector���ýӿ�

    // ���෨����session���䵽������Ϣ����ģ��
    const auto sessionHash = sessionInfo._sessionId % _msgTransfers.size();
    auto minTransfer = _msgTransfers[sessionHash];

    // ͳ��session����
    ++_curSessionConnecting;
    ++_sessionConnectedBefore;

    minTransfer->OnConnect(sessionInfo);
}

void FS_ServerCore::_OnDisconnected(IFS_Session *session)
{
    --_curSessionConnecting;
    ++_sessionDisconnectedCnt;

    // _msgDispatcher->OnDisconnected(session);
    _connector->OnDisconnected(session);
}

void FS_ServerCore::_OnHeartBeatTimeOut(IFS_Session *session)
{
    ++_heartbeatTimeOutDisconnected;
}

void FS_ServerCore::_OnRecvMsg(IFS_Session *session, Int64 transferBytes)
{
    _recvMsgBytesPerSecond += transferBytes;
    // Int64 incPackets = 0;
    // _msgDispatcher->OnRecv(session, incPackets);
    // _recvMsgCountPerSecond += incPackets;
}

void FS_ServerCore::_OnSvrRuning(FS_ThreadPool *threadPool)
{
    Time nowTime;
    _lastStatisticsTime.FlushTime();
    while(threadPool->IsPoolWorking())
    {
        // ÿ��100����ɨ��һ��
        Sleep(100);
        nowTime.FlushTime();
        const auto &timeSlice = nowTime - _lastStatisticsTime;
        if(timeSlice >= _statisticsInterval)
        {
            _lastStatisticsTime = nowTime;
            _PrintSvrLoadInfo(timeSlice);

            // ���ò���
            _recvMsgCountPerSecond = 0;
            _recvMsgBytesPerSecond = 0;
            _sendMsgBytesPerSecond = 0;
        }
    }
}

void FS_ServerCore::_PrintSvrLoadInfo(const TimeSlice &dis)
{
    const auto &sendSpeed = SocketUtil::ToFmtSpeedPerSec(_sendMsgBytesPerSecond);
    const auto &recvSpeed = SocketUtil::ToFmtSpeedPerSec(_recvMsgBytesPerSecond);
    g_Log->custom("<%lld ms> transfercnt<%d>, dispatcher<%d>, "
                  "online<%lld> historyonline<%lld>, timeout<%lld> offline<%lld>, "
                  "Recv[%lld pps], RecvSpeed<%s>, SendSpeed<%s>"
                  , dis.GetTotalMilliSeconds(), (Int32)(_msgTransfers.size()), (Int32)(_msgDispatchers.size())
                  , (Int64)(_curSessionConnecting), (Int64)(_sessionConnectedBefore)
                  ,(Int64)(_heartbeatTimeOutDisconnected), (Int64)(_sessionDisconnectedCnt)
                  , Int64(_recvMsgCountPerSecond), recvSpeed.c_str()
                  , sendSpeed.c_str());
}

#pragma endregion

#pragma region inner api
Int32 FS_ServerCore::_ReadConfig()
{
    _serverConfigMgr = FS_ServerConfigMgrFactory::Create();
    auto ret = _serverConfigMgr->Init();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("server config init fail ret[%d]"), ret);
        return ret;
    }

    _statisticsInterval = IOCP_STATISTIC_INTERVAL * Time::_microSecondPerMilliSecond;

    return StatusDefs::Success;
}

Int32 FS_ServerCore::_CreateNetModules()
{
    // TODO:��ͻ��˵����ӵ�ֻ����һ������Ϊ�˿�ֻ��һ��
    _connector = FS_ConnectorFactory::Create();

    //const Int32 cpuCnt = _cpuInfo->GetCpuCoreCnt();
    const Int32 transferCnt = g_SvrCfg->GetTransferCnt();
    const UInt32 dispatcherCnt = static_cast<UInt32>(_logics.size());

    if((transferCnt % dispatcherCnt) != 0)
    {
        g_Log->w<FS_ServerCore>(_LOGFMT_("transfer cnt[%d] cant divide by dispatcher cnt[%u], it will break balance!")
                                , transferCnt, dispatcherCnt);
    }

    _msgTransfers.resize(transferCnt);
    _messageQueue = new ConcurrentMessageQueue(transferCnt, dispatcherCnt);
    _senderMessageQueue.resize(transferCnt);
    g_net2LogicMessageQueue = _messageQueue;
    for(Int32 i = 0; i < transferCnt; ++i)
    {
        _senderMessageQueue[i] = new MessageQueue();
        _msgTransfers[i] = FS_MsgTransferFactory::Create(i);
        _msgTransfers[i]->AttachMsgQueue(_messageQueue, i);
        _msgTransfers[i]->AttachSenderMsgQueue(_senderMessageQueue[i]);
    }

    _msgDispatchers.resize(dispatcherCnt);
    for(UInt32 i = 0; i < dispatcherCnt; ++i)
    {
        _msgDispatchers[i] = FS_MsgDispatcherFactory::Create(i);
        _msgDispatchers[i]->AttachRecvMessageQueue(_messageQueue);
    }

    return StatusDefs::Success;
}

Int32 FS_ServerCore::_StartModules()
{
    Int32 ret = StatusDefs::Success;
    ret = _messageQueue->Start();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("messageQueue start fail ret[%d]"), ret);
        return ret;
    }

    const Int32 senderMqSize = static_cast<Int32>(_senderMessageQueue.size());
    for(Int32 i = 0; i < senderMqSize; ++i)
    {
        ret = _senderMessageQueue[i]->Start();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_ServerCore>(_LOGFMT_("senderMessageQueue start fail i[%d] ret[%d]"), i, ret);
            return ret;
        }
    }

    ret = _connector->Start();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("connector start fail ret[%d]"), ret);
        return ret;
    }

    for(auto &msgTransfer : _msgTransfers)
    {
        ret = msgTransfer->Start();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_ServerCore>(_LOGFMT_("msgTransfer start fail ret[%d]"), ret);
            return ret;
        }
    }

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
    {
        ret = _msgDispatchers[i]->Start();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_ServerCore>(_LOGFMT_("msgHandler start fail ret[%d] i[%d]"), ret, i);
            return ret;
        }
    }

    return StatusDefs::Success;
}

Int32 FS_ServerCore::_BeforeStart()
{
    Int32 ret = StatusDefs::Success;
    ret = _messageQueue->BeforeStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("messageQueue BeforeStart fail ret[%d]"), ret);
        return ret;
    }

    const Int32 senderMqSize = static_cast<Int32>(_senderMessageQueue.size());
    for(Int32 i=0;i<senderMqSize;++i)
    {
        ret = _senderMessageQueue[i]->BeforeStart();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_ServerCore>(_LOGFMT_("senderMessageQueue BeforeStart fail i[%d] ret[%d]"), i, ret);
            return ret;
        }
    }

    ret = _connector->BeforeStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("connector BeforeStart fail ret[%d]"), ret);
        return ret;
    }

    for(auto &msgTransfer : _msgTransfers)
    {
        ret = msgTransfer->BeforeStart();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_ServerCore>(_LOGFMT_("msgTransfer BeforeStart fail ret[%d]"), ret);
            return ret;
        }
    }

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
    {
        ret = _msgDispatchers[i]->BeforeStart();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_ServerCore>(_LOGFMT_("msgHandler BeforeStart fail ret[%d] i[%d]"), ret, i);
            return ret;
        }
    }

    return StatusDefs::Success;
}

Int32 FS_ServerCore::_OnStart()
{
    return StatusDefs::Success;
}

Int32 FS_ServerCore::_AfterStart()
{
    Int32 ret = StatusDefs::Success;
    ret = _connector->AfterStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ServerCore>(_LOGFMT_("connector AfterStart fail ret[%d]"), ret);
        return ret;
    }

    for(auto &msgTransfer : _msgTransfers)
    {
        ret = msgTransfer->AfterStart();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_ServerCore>(_LOGFMT_("msgTransfer AfterStart fail ret[%d]"), ret);
            return ret;
        }
    }

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
    {
        ret = _msgDispatchers[i]->AfterStart();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_ServerCore>(_LOGFMT_("msgHandler AfterStart fail ret[%d] i[%d]"), ret, i);
            return ret;
        }
    }

    return StatusDefs::Success;
}

void FS_ServerCore::_WillClose()
{
    _connector->WillClose();

    for(auto &msgTransfer : _msgTransfers)
        msgTransfer->WillClose();

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
        _msgDispatchers[i]->WillClose();
}

void FS_ServerCore::_BeforeClose()
{
    _connector->BeforeClose();
    for(auto &msgTransfer : _msgTransfers)
        msgTransfer->BeforeClose();

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
        _msgDispatchers[i]->BeforeClose();
}

void FS_ServerCore::_AfterClose()
{
    _connector->AfterClose();

    for(auto &msgTransfer : _msgTransfers)
        msgTransfer->AfterClose();

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
        _msgDispatchers[i]->AfterClose();
}

void FS_ServerCore::_RegisterToModule()
{
    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
    {
        _logics[i]->SetDispatcher(_msgDispatchers[i]);
        _msgDispatchers[i]->BindBusinessLogic(_logics[i]);
    }
}

#pragma endregion

FS_NAMESPACE_END
