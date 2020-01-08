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
#include "FrightenStone/common/net/Impl/FS_ServerConfigMgrFactory.h"

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

IFS_NetEngine::~IFS_NetEngine()
{
    Fs_SafeFree(_pool);
    Fs_SafeFree(_connector);
    STLUtil::DelVectorContainer(_acceptors);
    STLUtil::DelVectorContainer(_msgDispatchers);
    STLUtil::DelVectorContainer(_msgTransfers);
    Fs_SafeFree(_cpuInfo);

    STLUtil::DelVectorContainer(_compIdRefConsumerMq);
    Fs_SafeFree(_concurrentMq);
    Fs_SafeFree(_totalCfgs);
}

Int32 IFS_NetEngine::Init()
{
    if(_isInit)
        return StatusDefs::Success;

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
        g_Log->e<IFS_NetEngine>(_LOGFMT_("not little endian ret[%d]"), ret);
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
        g_Log->e<FS_NetEngine>(_LOGFMT_("InitSocketEnv fail ret[%d]"), ret);
        return ret;
    }

    // 7. ��ȡ����
    ret = _ReadConfig();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_NetEngine>(_LOGFMT_("_ReadConfig fail ret[%d]"), ret);
        return ret;
    }

    // 8.��־�Ƿ����
    if(!_totalCfgs)
    {
        ret = StatusDefs::Error;
        g_Log->e<FS_NetEngine>(_LOGFMT_("_totalCfgs total cfgs not init [%d]"), ret);
        return ret;
    }

    // 8.��ʼ����������
    ret = _OnInitFinish();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_NetEngine>(_LOGFMT_("_InitFinish fail ret[%d]"), ret);
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
    g_MemoryHelper->Start(_totalCfgs->_objPoolCfgs._maxAllowObjPoolBytesOccupied, _totalCfgs->_mempoolCfgs._maxAllowMemPoolBytesOccupied);
    // 2.�ڴ���
    g_MemleakMonitor->Start();

    // 3.���������
    _pool = new FS_ThreadPool(0, 1);
    auto task = DelegatePlusFactory::Create(this, &IFS_NetEngine::_Monitor);
    if(!_pool->AddTask(task, true))
    {
        g_Log->e<FS_NetEngine>(_LOGFMT_("add task fail"));
        return StatusDefs::FS_ServerCore_StartFailOfSvrRuningTaskFailure;
    }

    // 4.����������ģ��
    Int32 ret = _CreateNetModules();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_NetEngine>(_LOGFMT_("_CreateNetModules fail ret[%d]"), ret);
        return ret;
    }

    // 6.����ǰ...
    ret = _BeforeStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_NetEngine>(_LOGFMT_("_BeforeStart fail ret[%d]"), ret);
        return ret;
    }

    // 7.start ģ��
    ret = _StartModules();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_NetEngine>(_LOGFMT_("_StartModules fail ret[%d]"), ret);
        return ret;
    }

    // 8.onstart
    ret = _OnStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_NetEngine>(_LOGFMT_("_OnStart fail ret[%d]"), ret);
        return ret;
    }

    // 9._AfterStart
    ret = _AfterStart();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_NetEngine>(_LOGFMT_("_AfterStart fail ret[%d]"), ret);
        return ret;
    }

    return StatusDefs::Success;
}

void IFS_NetEngine::Close()
{
}

void IFS_NetEngine::_HandleCompEv_WillConnect(BriefSessionInfo *newSessionInfo)
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

    minTransfer->OnWillConnect(newSessionInfo);
}

void IFS_NetEngine::_Monitor(FS_ThreadPool *threadPool)
{
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
            _PrintInfo(timeSlice);

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
    g_Log->custom("<%lld ms> transfercnt<%d>, dispatcher<%d>, "
                  "online<%lld> historyonline<%lld>, timeout<%lld> offline<%lld>, "
                  "Recv[%lld pps], RecvSpeed<%s>, SendSpeed<%s>"
                  , dis.GetTotalMilliSeconds(), (Int32)(_msgTransfers.size()), (Int32)(_msgDispatchers.size())
                  , (Int64)(_curSessionConnecting), (Int64)(_sessionConnectedBefore)
                  , (Int64)(_heartbeatTimeOutDisconnected), (Int64)(_sessionDisconnectedCnt)
                  , Int64(_recvMsgCountPerSecond), recvSpeed.c_str()
                  , sendSpeed.c_str());
}

Int32 IFS_NetEngine::_ReadConfig()
{
    auto ret = _OnReadCfgs();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_NetEngine>(_LOGFMT_("FS_NetEngine _OnReadCfgs fail ret[%d]"), ret);
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
    _connector = FS_ConnectorFactory::Create(_sessionlocker
                                             , _curSessionCnt
                                             , _totalCfgs->_commonCfgs._maxSessionQuantityLimit
                                             , _curMaxSessionId);

    // ��������
    const UInt32 acceptorQuantity = _totalCfgs->_commonCfgs._acceptorQuantityLimit;
    _acceptors.resize(acceptorQuantity);
    for(UInt32 i = 0; i < acceptorQuantity; ++i)
    {
         _acceptors[i] = FS_AcceptorFactory::Create(_GenerateCompId(),
                                                   _sessionlocker
                                                   , _curSessionCnt
                                                   , _totalCfgs->_commonCfgs._maxSessionQuantityLimit
                                                   , _curMaxSessionId
                                                   , this);
    }

    //const Int32 cpuCnt = _cpuInfo->GetCpuCoreCnt();
    // const UInt32 dispatcherCnt = static_cast<UInt32>(logics.size());

    const auto transferQuatity = _totalCfgs->_commonCfgs._transferQuantity;
    const auto dispatcherQuatity = _totalCfgs->_commonCfgs._dispatcherQuantity;
    if((transferQuatity % dispatcherQuatity) != 0)
    {
        g_Log->w<FS_NetEngine>(_LOGFMT_("transfer cnt[%u] cant divide by dispatcher cnt[%u], it will break balance!")
                               , transferQuatity, dispatcherQuatity);
    }

    _msgTransfers.resize(transferQuatity);
    for(UInt32 i = 0; i < transferQuatity; ++i)
    {
        auto newTransfer = FS_MsgTransferFactory::Create(_GenerateCompId(), this);
        newTransfer->BindConcurrentParams(_concurrentMq->GenerateGeneratorId(), 0, _concurrentMq);
        _msgTransfers[i] = newTransfer;
    }

    // ҵ���
    std::vector<IFS_BusinessLogic *> logics;
    _GetLogics(logics);
    _msgDispatchers.resize(dispatcherQuatity);
    for(UInt32 i = 0; i < dispatcherQuatity; ++i)
    {
        _msgDispatchers[i] = FS_MsgDispatcherFactory::Create(i, this);
        _msgDispatchers[i]->AttachRecvMessageQueue(_messageQueue);

        if(!logics.empty() && i < logics.size())
            _msgDispatchers[i]->BindBusinessLogic(logics[i]);
    }

    // �����Ϣ���� �ɵ����������
    _compConsumerMq.resize(_curMaxCompId + 1);  // ��compid����������compId��1��ʼ�ʶ�һ���ռ�0����λΪNULL
    const UInt32 compConsumerMqSize = static_cast<UInt32>(_compConsumerMq.size());
    for(UInt32 i = 1; i < compConsumerMqSize; ++i)
    {
        _compConsumerMq[i] = new MessageQueueNoThread();
        
    }

    return StatusDefs::Success;
}

FS_NAMESPACE_END
