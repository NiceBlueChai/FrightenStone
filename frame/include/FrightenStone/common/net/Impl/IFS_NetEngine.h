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
 * @file  : IFS_NetEngine.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/12/28
 * @brief :
 * 
 *
 * 
 */
#pragma once
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_IFS_NetEngine_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_IFS_NetEngine_H__

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/status/status.h"
#include "FrightenStone/common/asyn/asyn.h"
#include "FrightenStone/common/component/Impl/Time.h"
#include "FrightenStone/common/component/Impl/TimeSlice.h"

FS_NAMESPACE_BEGIN

class FS_ThreadPool;
class TimeSlice;
struct NetEngineTotalCfgs;
class IFS_Connector;
class IFS_Acceptor;
class IFS_MsgTransfer;
class ConcurrentMessageQueueNoThread;
class MessageQueueNoThread;
class FS_CpuInfo;
struct BriefSessionInfo;
class IFS_MsgDispatcher;

class BASE_EXPORT IFS_NetEngine
{
public:
    IFS_NetEngine();
    virtual ~IFS_NetEngine();

public:
    virtual Int32 Init();
    virtual Int32 Start();
    virtual void Wait();
    virtual void Sinal();
    virtual void Close();

    /* ����¼� */
protected:
    virtual void _HandleCompEv_WillConnect(BriefSessionInfo *newSessionInfo);

protected:
    virtual void _Monitor(FS_ThreadPool *threadPool);
    void _PrintInfo(const TimeSlice &dis);

    /* �����ط�ʱ����� */
protected:
    Int32 _ReadConfig();
    Int32 _CreateNetModules();
    Int32 _StartModules();

    Int32 _BeforeStart();
    Int32 _OnStart();
    Int32 _AfterStart();

    void _WillClose();
    void _BeforeClose();
    void _AfterClose();

    /* ���� */
protected:
    // ��ȡ����λ��
    virtual Int32 _OnReadCfgs() = 0;
    // ��ʼ������ʱ
    virtual Int32 _OnInitFinish() { return StatusDefs::Success; }

    /* ���� */
protected:
    // �������id
    UInt32 _GenerateCompId();
    // ��ȡtransfer->dispatcher������Ϣ����
    ConcurrentMessageQueueNoThread *_GetConcurrentMQ();

    void _AddNewComp(UInt32 compId, IFS_EngineComp *newComp);

protected:
    /* ��Ԫ */
    friend class FS_IocpPoller;
    friend class FS_IocpConnector;
    friend class FS_IocpAcceptor;
    
    NetEngineTotalCfgs *_totalCfgs;
    FS_CpuInfo *_cpuInfo;
    std::atomic_bool _isInit;

    Locker _locker;
    IFS_Connector * _connector;                         // ������
    std::vector<IFS_Acceptor *> _acceptors;             // ֧�ּ�����˿ڣ����忴��������������
    std::vector<IFS_MsgTransfer *> _msgTransfers;       // ���߳���Ϣ�շ���
    std::vector<IFS_MsgDispatcher *> _msgDispatchers;   // ҵ����Ϣ������ ҵ���̴߳���,֧�ֶ��̲߳�������
    UInt32 _curMaxCompId;                                // ���id���ֵ ��Сֵ��1��ʼ
    std::map<UInt32, IFS_EngineComp *> _compIdRefComps; // ���

    ConcurrentMessageQueueNoThread *_concurrentMq;                // ����,ֻ�ܴ������ߵ������� generatorid ��consumerId��Ҫ��������
    std::vector<MessageQueueNoThread *> _compConsumerMq;            // ��compId���±�����
    
    /* ͳ������ */ 
    ConditionLocker _waitForClose;                  // һ�������̣߳����������ȴ��������
    FS_ThreadPool *_pool;
    Time _lastStatisticsTime;                       // ���һ��ͳ�Ƶ�ʱ��
    TimeSlice _statisticsInterval;                  // ͳ�Ƶ�ʱ����
    std::atomic<Int64> _curSessionConnecting;       // �����ӵĻỰ����
    std::atomic<Int64> _sessionConnectedBefore;     // ��������ĻỰ����
    std::atomic<Int64> _sessionDisconnectedCnt;     // �Ͽ����ӵĻỰ����
    std::atomic<Int64> _recvMsgCountPerSecond;      // ÿ���յ��İ�����
    std::atomic<Int64> _recvMsgBytesPerSecond;      // ÿ���յ��İ����ֽ���
    std::atomic<Int64> _sendMsgBytesPerSecond;      // ÿ�뷢�͵İ��ֽ���
    std::atomic<Int64> _heartbeatTimeOutDisconnected;   // ������ʱ�Ͽ��Ự��
    
    /* ���ӵĻỰ���� */ 
    Locker _sessionlocker;
    Int32 _curSessionCnt;
    UInt64 _curMaxSessionId;
};


FS_NAMESPACE_END

#include "FrightenStone/common/net/Impl/IFS_NetEngineImpl.h"

#endif