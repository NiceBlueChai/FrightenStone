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
 * @file  : IFS_NetEngine.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/12/28
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "FrightenStone/common/net/Impl/IFS_NetEngine.h"
#include "FrightenStone/common/net/Impl/IFS_MsgTransfer.h"
#include "FrightenStone/common/net/Impl/IFS_Acceptor.h"
#include "FrightenStone/common/net/Impl/IFS_Connector.h"
#include "FrightenStone/common/net/Impl/IFS_MsgDispatcher.h"
#include "FrightenStone/common/net/Impl/FS_AcceptorFactory.h"
#include "FrightenStone/common/net/Impl/FS_ConnectorFactory.h"
#include "FrightenStone/common/net/Impl/FS_MsgDispatcherFactory.h"
#include "FrightenStone/common/net/Impl/FS_MsgTransferFactory.h"
#include "FrightenStone/common/net/Impl/FS_ConfigMgrFactory.h"
#include "FrightenStone/common/net/Impl/IFS_ConfigMgr.h"
#include "FrightenStone/common/net/Defs/ServerCompsDef.h"
#include "FrightenStone/common/net/Defs/EngineCompDefs.h"

#include "FrightenStone/common/component/Impl/SmartVar/SmartVar.h"
#include "FrightenStone/common/assist/utils/Impl/FS_TlsUtil.h"
#include "FrightenStone/common/log/Log.h"
#include "FrightenStone/common/crashhandle/CrashHandle.h"
#include "FrightenStone/common/assist/utils/Impl/SystemUtil.h"
#include "FrightenStone/common/component/Impl/FS_CpuInfo.h"
#include  "FrightenStone/common/socket/socket.h"
#include "FrightenStone/common/memleak/memleak.h"
#include "FrightenStone/common/memoryhelper/MemoryHelper.h"
#include "FrightenStone/common/component/Impl/FS_ThreadPool.h"
#include "FrightenStone/common/component/Impl/MessageQueue/MessageQueue.h"

FS_NAMESPACE_BEGIN
OBJ_POOL_CREATE_DEF_IMPL(IFS_NetEngine, 2);

IFS_NetEngine::~IFS_NetEngine()
{
    Fs_SafeFree(_pool);
    Fs_SafeFree(_connector);
    STLUtil::DelVectorContainer(_acceptors);
    STLUtil::DelVectorContainer(_msgDispatchers);
    STLUtil::DelVectorContainer(_msgTransfers);
    Fs_SafeFree(_cpuInfo);

    Fs_SafeFree(_concurrentMq);
    Fs_SafeFree(_totalCfgs);
    STLUtil::DelVectorContainer(_compConsumerMq);
}

Int32 IFS_NetEngine::Init()
{
    if(_isInit)
        return StatusDefs::Success;

    // 0. ������
    _appName = SystemUtil::GetApplicationName();

    // 1.ʱ��
    TimeUtil::SetTimeZone();

    // 2.���ܱ���������ʶ��
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

    // 4.log��ʼ�� NULLĬ���Գ�����Ϊ��׼����Ŀ¼
    ret = g_Log->InitModule(NULL);
    if(ret != StatusDefs::Success)
    {
        FS_String str;
        str.AppendFormat("log init fail ret[%d]", ret);
        ASSERTBOX(str.c_str());
        return ret;
    }

    // 5. crash dump switch start
    ret = CrashHandleUtil::InitCrashHandleParams();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("init crash handle params fail ret[%d]"), ret);
        return ret;
    }

    // 6.��С���жϣ�������ֻ֧��x86��С���ֽ����cpu
    if(!SystemUtil::IsLittleEndian())
    {
        ret = StatusDefs::SystemUtil_NotLittleEndian;
        g_Log->e<IFS_NetEngine>(_LOGFMT_("appname[%s] the endianness of current machine is bigendian. This app support little endian only.!ret[%d]")
                                , _appName.c_str(), ret);
        return ret;
    }

    // cpu��Ϣ��ʼ��
    _cpuInfo = new FS_CpuInfo;
    if(!_cpuInfo->Initialize())
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("Initialize cpuinfo fail"));
        return StatusDefs::Failed;
    }

    // 6.Socket����
    ret = SocketUtil::InitSocketEnv();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("InitSocketEnv fail ret[%d]"), ret);
        return ret;
    }

    // 7. ��ȡ����
    ret = _ReadConfig();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_ReadConfig fail ret[%d]"), ret);
        return ret;
    }

    // 8.��־�Ƿ����
    if(!_totalCfgs)
    {
        ret = StatusDefs::Error;
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_totalCfgs total cfgs not init [%d]"), ret);
        return ret;
    }

    // 8.��ʼ����������
    ret = _OnInitFinish();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_InitFinish fail ret[%d]"), ret);
        return ret;
    }

    _isInit = true;

    return StatusDefs::Success;
}

Int32 IFS_NetEngine::Start()
{
    if(!_isInit)
        return StatusDefs::NotInit;

    // 1.�ڴ�����
    g_MemoryHelper = MemoryHelper::GetInstance();
    g_MemoryHelper->Start(_totalCfgs->_objPoolCfgs._maxAllowObjPoolBytesOccupied, _totalCfgs->_mempoolCfgs._maxAllowMemPoolBytesOccupied);
    g_MemleakMonitor->BeforeStart(_totalCfgs->_commonCfgs._memoryMonitorPrintIntervalSeconds);

    // 2.�ڴ���
    if(_totalCfgs->_commonCfgs._isOpenMemoryMonitor)
    {
        g_MemleakMonitor->Start();
    }

    // 3.��ʼ���̶߳���
    _pool = new FS_ThreadPool(0, 1);

    // 4.����������ģ��
    Int32 ret = _CreateNetModules();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_CreateNetModules fail ret[%d]"), ret);
        return ret;
    }

    // 6.����ǰ...
    ret = _BeforeStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_BeforeStart fail ret[%d]"), ret);
        return ret;
    }

    // 7.start ģ��
    ret = _StartModules();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_StartModules fail ret[%d]"), ret);
        return ret;
    }

    // 8.onstart
    ret = _OnStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_OnStart fail ret[%d]"), ret);
        return ret;
    }

    // 9._AfterStart
    _AfterStart();

    // 10.�����̼߳�����
    auto task = DelegatePlusFactory::Create(this, &IFS_NetEngine::_Monitor);
    if(!_pool->AddTask(task, true))
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("add task fail"));
        return StatusDefs::FS_ServerCore_StartFailOfSvrRuningTaskFailure;
    }

    g_Log->sys<IFS_NetEngine>(_LOGFMT_("net engine start suc..."));
    return StatusDefs::Success;
}

void IFS_NetEngine::Close()
{
    if(!_isInit)
        return;

    _isInit = false;

    // �Ͽ���������
    _WillClose();

    // �����Ը�ģ���Ƴ���Դ
    _BeforeClose();

    // �رս�������
    for(auto &acceptor : _acceptors)
        acceptor->Close();

    // �Ƴ�����������ģ��
    _connector->Close();
    for(auto &msgTransfer : _msgTransfers)
        msgTransfer->Close();

    for(auto &dispatcher : _msgDispatchers)
        dispatcher->Close();

    _concurrentMq->BeforeClose();
    _concurrentMq->Close();

    STLUtil::DelVectorContainer(_compConsumerMq);
    Fs_SafeFree(_concurrentMq);

    _pool->Close();

    // ���һ��ɨ��
    TimeSlice timeSlice;
    timeSlice = Time::_microSecondPerSecond;
    _PrintInfo(timeSlice);

    // �����
    _AfterClose();

    g_Log->custom("waiting for all comps close...");
    EngineCompsMethods::WaitForAllCompsFinish(this);
    g_Log->custom("all comps close now!\nnetengine will close");

    SocketUtil::ClearSocketEnv();

    Fs_SafeFree(_pool);
    Fs_SafeFree(_connector);
    STLUtil::DelVectorContainer(_msgDispatchers);
    STLUtil::DelVectorContainer(_msgTransfers);
    Fs_SafeFree(_cpuInfo);

    // ��ǰ��־ȫ������
    g_MemleakMonitor->Finish();
    g_MemoryHelper->Finish();
    g_Log->FlushAllFile();
    g_Log->FinishModule();
    g_MemoryPool->FinishPool();

    Fs_SafeFree(_totalCfgs);
}

bool IFS_NetEngine::IsCompAllReady() const
{
    if(!_isInit)
        return false;

    if(_compIdRefComps.empty())
        return false;

    for(auto &iterComp : _compIdRefComps)
    {
        if(!iterComp.second->IsReady())
            return false;
    }

    return true;
}

bool IFS_NetEngine::IsCompAllFinish() const
{
    for(auto &iterComp : _compIdRefComps)
    {
        if(iterComp.second->IsReady())
        {
            g_Log->custom("comp info:[%s] is not finish", iterComp.second->ToString().c_str());
            return false;
        }
    }

    return true;
}

bool IFS_NetEngine::IsAllTransferFinish() const
{
    for(auto transfer : _msgTransfers)
    {
        if(transfer->IsReady())
            return false;
    }

    return true;
}

bool IFS_NetEngine::IsAllTransferIoFinish() const
{
    for(auto transfer : _msgTransfers)
    {
        if(!transfer->IsIoFinish())
            return false;
    }

    return true;
}

bool IFS_NetEngine::IsServicePort(UInt16 port) const
{
    return _totalCfgs->_commonCfgs._serviceNodePort == port;
}

bool IFS_NetEngine::IsRealUserPort(UInt16 port) const
{
    return _totalCfgs->_commonCfgs._realUserPort == port;
}

void IFS_NetEngine::Connect(const FS_ConnectInfo &info)
{
    Int32 st = _connector->Connect(info);
    if(st != StatusDefs::Success)
        g_Log->w<IFS_NetEngine>(_LOGFMT_("st[%d] connect to<%s:%hu> fail..."), st, info._ip.c_str(), info._port);
}

void IFS_NetEngine::PostConnectFinish()
{
    _connector->PostConnectOpFinish();
}

bool IFS_NetEngine::_HandleCompEv_WillConnect(BriefSessionInfo *newSessionInfo)
{
    // transfer session����
    const Int32 transferQuantity = static_cast<Int32>(_msgTransfers.size());
    IFS_MsgTransfer *minTransfer = _msgTransfers[0];
    for(Int32 i = 0; i < transferQuantity; ++i)
    {
        if(minTransfer->GetSessionCnt() > _msgTransfers[i]->GetSessionCnt())
            minTransfer = _msgTransfers[i];
    }

    // ͳ��session����
    ++_curSessionConnecting;
    ++_sessionConnectedBefore;

    if(!minTransfer->OnWillConnect(newSessionInfo))
    {
        --_sessionConnectedBefore;
        --_curSessionConnecting;
        return false;
    }

    return true;
}

void IFS_NetEngine::_HandleCompEv_Disconnected(UInt64 sessionId,  UInt32 acceptorCompId, UInt32 transferCompId)
{
    --_curSessionConnecting;
    ++_sessionDisconnectedCnt;

    // ���ӶϿ�
    if(acceptorCompId)
    {
        auto acceptorComp = _GetComp(acceptorCompId);
        acceptorComp->OnSessionDisconnected(sessionId);
    }

    auto transferComp = _GetComp(transferCompId);
    transferComp->OnSessionDisconnected(sessionId);
}

void IFS_NetEngine::_HandleCompEv_ConnectorPostConnectOpFinish()
{
    // TODO:����ȡ��һ��transfer�����Կ����·�������ֱ����ô������Ϊ�п���dispatch�Ƕ��Ҫ��Ҫ����ÿ��transfer����һ��,ò���ֲ�������Ϊ�����ϢֻҪ��һ�ξ͹���
    _msgTransfers[0]->OnPostConnectOpFinish();
}

void IFS_NetEngine::_Monitor(FS_ThreadPool *threadPool)
{
    g_Log->custom("waiting for all comps ready...");
    EngineCompsMethods::WaitForAllCompsReady(this);
    g_Log->custom("all comps ready now!\nserver suc start");

    Time nowTime;
    _lastStatisticsTime.FlushTime();
    while(threadPool->IsPoolWorking())
    {
        // ÿ��100����ɨ��һ��
        SystemUtil::Sleep(100);
        nowTime.FlushTime();
        const auto &timeSlice = nowTime - _lastStatisticsTime;
        if(timeSlice >= _statisticsInterval)
        {
            _lastStatisticsTime = nowTime;

            #ifdef _WIN32
            _PrintInfo(timeSlice);
            #endif

            // ���ò���
            _recvMsgCountPerSecond = 0;
            _recvMsgBytesPerSecond = 0;
            _sendMsgBytesPerSecond = 0;
        }
    }
}

void IFS_NetEngine::_PrintInfo(const TimeSlice &dis)
{
    const auto &sendSpeed = SocketUtil::ToFmtSpeedPerSec(_sendMsgBytesPerSecond);
    const auto &recvSpeed = SocketUtil::ToFmtSpeedPerSec(_recvMsgBytesPerSecond);

    // ���߳���static�ǰ�ȫ��
    static FS_String transferBalanceInfo;
    static FS_String dispatcherBalanceInfo;
    static FS_String toPrintInfo;
    static Int32 serviceId = _totalCfgs->_dispatcherCfgs._logicServiceId;

    transferBalanceInfo.Clear();
    dispatcherBalanceInfo.Clear();
    for(auto &transfer : _msgTransfers)
        transferBalanceInfo.AppendFormat("%d,", transfer->GetSessionCnt());
    for(auto &dispatcher : _msgDispatchers)
        dispatcherBalanceInfo.AppendFormat("%d,", dispatcher->GetSessionCnt());

    // Ҫ��ӡ������
    toPrintInfo.Clear();
    toPrintInfo.AppendFormat("<%lld ms> service<id=%d,%s> transfercnt<%d,balance info:%s>, dispatcher<%d,balance info:%s>, "
                             "online<%lld> historyonline<%lld>, timeout<%lld> offline<%lld>, "
                             "Recv[%lld pps], RecvSpeed<%s>, SendSpeed<%s>"
                             , dis.GetTotalMilliSeconds(), serviceId, _serviceName.c_str(), (Int32)(_msgTransfers.size()), transferBalanceInfo.c_str()
                             , (Int32)(_msgDispatchers.size()), dispatcherBalanceInfo.c_str()
                             , (Int64)(_curSessionConnecting), (Int64)(_sessionConnectedBefore)
                             , (Int64)(_heartbeatTimeOutDisconnected), (Int64)(_sessionDisconnectedCnt)
                             , Int64(_recvMsgCountPerSecond), recvSpeed.c_str()
                             , sendSpeed.c_str());

    // �Զ����ӡ
    _ModifyCustomPrintInfo(toPrintInfo);

    g_Log->custom(toPrintInfo.c_str());
}

Int32 IFS_NetEngine::_ReadConfig()
{
    auto ret = _OnReadCfgs();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("FS_NetEngine _OnReadCfgs fail ret[%d]"), ret);
        return ret;
    }

    _statisticsInterval = IOCP_STATISTIC_INTERVAL * Time::_microSecondPerMilliSecond;

    return StatusDefs::Success;
}

Int32 IFS_NetEngine::_CreateNetModules()
{
    const auto generatorQuatity = _totalCfgs->_commonCfgs._transferQuantity;
    const auto consumerQuatity = _totalCfgs->_commonCfgs._dispatcherQuantity;

    // ������Ϣ���� ��transfer=>dispatcher֮��ͨ��
    _concurrentMq = new ConcurrentMessageQueueNoThread(generatorQuatity, consumerQuatity);

    // ������
    _connector = FS_ConnectorFactory::Create(this
                                             , _GenerateCompId()
                                             , _sessionlocker
                                             , _curSessionCnt
                                             , _totalCfgs->_commonCfgs._maxSessionQuantityLimit
                                             , _curMaxSessionId);
    _AddNewComp(_connector->GetCompId(), _connector);

    // ������
    const UInt32 acceptorQuantity = _totalCfgs->_commonCfgs._acceptorQuantityLimit;
    _acceptors.resize(acceptorQuantity);
    const auto &acceptorCfgs = _totalCfgs->_acceptorCfgs;
    for(UInt32 i = 0; i < acceptorQuantity; ++i)
    {
         auto newAcceptor = FS_AcceptorFactory::Create(_GenerateCompId(),
                                                   _sessionlocker
                                                   , _curSessionCnt
                                                   , _totalCfgs->_commonCfgs._maxSessionQuantityLimit
                                                   , _curMaxSessionId
                                                   , acceptorCfgs[i]
                                                   , this);
         _AddNewComp(newAcceptor->GetCompId(), newAcceptor);
         _acceptors[i] = newAcceptor;
    }

    //const Int32 cpuCnt = _cpuInfo->GetCpuCoreCnt();
    // const UInt32 dispatcherCnt = static_cast<UInt32>(logics.size());

    const auto transferQuatity = _totalCfgs->_commonCfgs._transferQuantity;
    const auto dispatcherQuatity = _totalCfgs->_commonCfgs._dispatcherQuantity;
    if((transferQuatity % dispatcherQuatity) != 0)
    {
        g_Log->w<IFS_NetEngine>(_LOGFMT_("transfer cnt[%u] cant divide by dispatcher cnt[%u], it will break balance!")
                               , transferQuatity, dispatcherQuatity);
    }

    // ���ݴ����
    _msgTransfers.resize(transferQuatity);
    for(UInt32 i = 0; i < transferQuatity; ++i)
    {
        auto newTransfer = FS_MsgTransferFactory::Create(_GenerateCompId(), this);
        newTransfer->BindConcurrentParams(_concurrentMq->GenerateGeneratorId(), 0, _concurrentMq);
        _AddNewComp(newTransfer->GetCompId(), newTransfer);
        _msgTransfers[i] = newTransfer;
    }

    // ҵ���
    std::vector<IFS_BusinessLogic *> logics;
    _GetLogics(logics);

    // ��Ϣת����
    _msgDispatchers.resize(dispatcherQuatity);
    for(UInt32 i = 0; i < dispatcherQuatity; ++i)
    {
        auto newDispatcher = FS_MsgDispatcherFactory::Create(_GenerateCompId(), this);
        _msgDispatchers[i] = newDispatcher;
        newDispatcher->BindConcurrentParams(0, _concurrentMq->GenerateConsumerId(), _concurrentMq);
        _AddNewComp(newDispatcher->GetCompId(), newDispatcher);

        if(!logics.empty() && i < logics.size())
            newDispatcher->BindBusinessLogic(logics[i]);
    }

    // �����Ϣ���� �ɵ����������
    _compConsumerMq.resize(_curMaxCompId + 1);  // ��compid����������compId��1��ʼ�ʶ�һ���ռ�0����λΪNULL
    const UInt32 compConsumerMqSize = static_cast<UInt32>(_compConsumerMq.size());
    for(UInt32 i = 1; i < compConsumerMqSize; ++i)
    {
        auto newCompMq = new MessageQueueNoThread();
        auto comp = _GetComp(i);
        _compConsumerMq[i] = newCompMq;
        comp->BindCompMq(newCompMq);
        comp->AttachAllCompMq(&_compConsumerMq);
    }

    return StatusDefs::Success;
}

Int32 IFS_NetEngine::_StartModules()
{
    Int32 ret = StatusDefs::Success;
    ret = _concurrentMq->Start();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_concurrentMq start fail ret[%d]"), ret);
        return ret;
    }

//     const Int32 compMqSize = static_cast<Int32>(_compConsumerMq.size());
//     for(Int32 i = 1; i < compMqSize; ++i)
//     {
//         ret = _compConsumerMq[i]->Start();
//         if(ret != StatusDefs::Success)
//         {
//             g_Log->e<IFS_NetEngine>(_LOGFMT_("_compConsumerMq BeforeStart fail i[%d] ret[%d]"), i, ret);
//             return ret;
//         }
//     }

    ret = _connector->Start();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("connector start fail ret[%d]"), ret);
        return ret;
    }

    const auto acceptoreQuantity = static_cast<Int32>(_acceptors.size());
    for(Int32 i = 0; i < acceptoreQuantity; ++i)
    {
        ret = _acceptors[i]->Start();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<IFS_NetEngine>(_LOGFMT_("_acceptors start fail i[%u] ret[%d]"), i, ret);
            return ret;
        }
    }

    for(auto &msgTransfer : _msgTransfers)
    {
        ret = msgTransfer->Start();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<IFS_NetEngine>(_LOGFMT_("msgTransfer start fail ret[%d]"), ret);
            return ret;
        }
    }

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
    {
        ret = _msgDispatchers[i]->Start();
        if(ret != StatusDefs::Success)
        {
            g_Log->e<IFS_NetEngine>(_LOGFMT_("msgHandler start fail ret[%d] i[%d]"), ret, i);
            return ret;
        }
    }

    return ret;
}

Int32 IFS_NetEngine::_BeforeStart()
{
    Int32 ret = StatusDefs::Success;
    ret = _concurrentMq->BeforeStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_concurrentMq BeforeStart fail ret[%d]"), ret);
        return ret;
    }

//     const Int32 compMqSize = static_cast<Int32>(_compConsumerMq.size());
//     for(Int32 i = 1; i < compMqSize; ++i)
//     {
//         ret = _compConsumerMq[i]->BeforeStart();
//         if(ret != StatusDefs::Success)
//         {
//             g_Log->e<IFS_NetEngine>(_LOGFMT_("_compConsumerMq BeforeStart fail i[%d] ret[%d]"), i,  ret);
//             return ret;
//         }
//     }

    const Int32 acceptorQuatity = static_cast<Int32>(_acceptors.size());
    for(Int32 i = 0; i < acceptorQuatity; ++i)
    {
        ret = _acceptors[i]->BeforeStart(*_totalCfgs);
        if(ret != StatusDefs::Success)
        {
            g_Log->e<IFS_NetEngine>(_LOGFMT_("_acceptors BeforeStart fail i[%d] ret[%d]"), i, ret);
            return ret;
        }
    }

    ret = _connector->BeforeStart(*_totalCfgs);
    if(ret != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("connector BeforeStart fail ret[%d]"), ret);
        return ret;
    }

    for(auto &msgTransfer : _msgTransfers)
    {
        ret = msgTransfer->BeforeStart(*_totalCfgs);
        if(ret != StatusDefs::Success)
        {
            g_Log->e<IFS_NetEngine>(_LOGFMT_("msgTransfer BeforeStart fail ret[%d]"), ret);
            return ret;
        }
    }

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
    {
        ret = _msgDispatchers[i]->BeforeStart(*_totalCfgs);
        if(ret != StatusDefs::Success)
        {
            g_Log->e<IFS_NetEngine>(_LOGFMT_("msgHandler BeforeStart fail ret[%d] i[%d]"), ret, i);
            return ret;
        }
    }

    return ret;
}

Int32 IFS_NetEngine::_OnStart()
{
    return StatusDefs::Success;
}

void IFS_NetEngine::_AfterStart()
{
    const Int32 acceptorQuatity = static_cast<Int32>(_acceptors.size());
    for(Int32 i = 0; i < acceptorQuatity; ++i)
        _acceptors[i]->AfterStart();

    _connector->AfterStart();
    for(auto &msgTransfer : _msgTransfers)
        msgTransfer->AfterStart();

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
        _msgDispatchers[i]->AfterStart();
}

void IFS_NetEngine::_WillClose()
{
    // �رս�������
    for(auto &acceptor : _acceptors)
        acceptor->WillClose();

    _connector->WillClose();

    for(auto &msgTransfer : _msgTransfers)
        msgTransfer->WillClose();

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
        _msgDispatchers[i]->WillClose();
}

void IFS_NetEngine::_BeforeClose()
{
    // �رս�������
    for(auto &acceptor : _acceptors)
        acceptor->BeforeClose();

    _connector->BeforeClose();
    for(auto &msgTransfer : _msgTransfers)
        msgTransfer->BeforeClose();

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
        _msgDispatchers[i]->BeforeClose();
}

void IFS_NetEngine::_AfterClose()
{
    // �رս�������
    for(auto &acceptor : _acceptors)
        acceptor->AfterClose();

    _connector->AfterClose();

    for(auto &msgTransfer : _msgTransfers)
        msgTransfer->AfterClose();

    const Int32 dispatcherSize = static_cast<Int32>(_msgDispatchers.size());
    for(Int32 i = 0; i < dispatcherSize; ++i)
        _msgDispatchers[i]->AfterClose();
}

Int32 IFS_NetEngine::_ReadBaseCfgs(IFS_ConfigMgr *cfgMgr)
{
    _totalCfgs = new fs::NetEngineTotalCfgs;
    auto &commonConfig = _totalCfgs->_commonCfgs;
    commonConfig._maxSessionQuantityLimit = cfgMgr->GetMaxSessionQuantityLimit();
    commonConfig._acceptorQuantityLimit = cfgMgr->GetAcceptorQuantity();
    commonConfig._dispatcherQuantity = cfgMgr->GetDispatcherCnt();
    commonConfig._transferQuantity = cfgMgr->GetTransferCnt();
    commonConfig._isOpenMemoryMonitor = cfgMgr->GetIsOpenMemoryMonitor();
    commonConfig._memoryMonitorPrintIntervalSeconds = cfgMgr->GetMemoryMonitorPrintIntervalSeconds();
    commonConfig._realUserPort = cfgMgr->GetRealUserPort();
    commonConfig._serviceNodePort = cfgMgr->GetServiceNodePort();

    auto &connectorCfg = _totalCfgs->_connectorCfgs;
    connectorCfg._connectTimeOutMs = cfgMgr->GetConnectorConnectTimeOutMs();

    // һ�������
    _totalCfgs->_acceptorCfgs = new fs::AcceptorCfgs[commonConfig._acceptorQuantityLimit];
    auto &acceptorCfg = _totalCfgs->_acceptorCfgs;
    std::vector<std::pair<fs::FS_String, UInt16>> addrInfos;
    cfgMgr->GetListenAddr(addrInfos);
    for(UInt32 i = 0; i < commonConfig._acceptorQuantityLimit; ++i)
    {
        acceptorCfg[i]._ip = addrInfos[i].first;
        acceptorCfg[i]._port = addrInfos[i].second;
    }

    auto &transferCfg = _totalCfgs->_transferCfgs;

    auto &dispatcherCfg = _totalCfgs->_dispatcherCfgs;
    dispatcherCfg._heartbeatDeadTimeMsInterval = cfgMgr->GetHeartbeatDeadTimeIntervalMs();
    dispatcherCfg._dispatcherResolutionInterval = cfgMgr->GetDispatcherResolutionIntervalMs()*fs::Time::_microSecondPerMilliSecond;
    dispatcherCfg._maxAlloctorBytesPerDispatcher = cfgMgr->GetMaxAllowAlloctorBytesPerDispatcher();
    dispatcherCfg._prepareBufferPoolCnt = cfgMgr->GetPrepareBufferCnt();
    dispatcherCfg._logicServiceId = cfgMgr->GetLogicServiceId();

    auto &objPoolCfgs = _totalCfgs->_objPoolCfgs;
    objPoolCfgs._maxAllowObjPoolBytesOccupied = cfgMgr->GetMaxAllowObjPoolBytesOccupied();

    auto &mempoolCfgs = _totalCfgs->_mempoolCfgs;
    mempoolCfgs._maxAllowMemPoolBytesOccupied = cfgMgr->GetMaxAllowMemPoolBytesOccupied();

    return StatusDefs::Success;
}

Int32 IFS_NetEngine::_OnReadCfgs()
{
    auto cfgMgr = _PreparConfigFile();
    if(!cfgMgr)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_PreparConfigFile fail"));
        return StatusDefs::Failed;
    }

    Int32 st = _ReadBaseCfgs(cfgMgr);
    if(st != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_ReadBaseCfgs fail st[%d]"), st);
        return st;
    }

    st = _ReadCustomCfgs();
    if(st != StatusDefs::Success)
    {
        g_Log->e<IFS_NetEngine>(_LOGFMT_("_ReadCustomCfgs fail st[%d]"), st);
        return st;
    }

    g_Log->i<IFS_NetEngine>(_LOGFMT_("read cfgs suc cfg info:\n%s"), cfgMgr->ToString().c_str());
    return StatusDefs::Success;
}

FS_NAMESPACE_END
