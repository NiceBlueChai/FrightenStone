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
 * @file  : FS_NetEngine.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/12/7
 * @brief :
 * 
 *
 * 
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_FS_NetEngine_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_FS_NetEngine_H__

#pragma once

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/asyn/asyn.h"
#include "FrightenStone/common/component/Impl/Time.h"
#include "FrightenStone/common/component/Impl/TimeSlice.h"
#include <FrightenStone/common/status/status.h>

FS_NAMESPACE_BEGIN

class  IFS_Session;
class  IFS_ServerConfigMgr;
class  IFS_Connector;
class  IFS_Acceptor;        // TODO
class  IFS_MsgTransfer;
class  IFS_MsgDispatcher;
class  FS_SessionMgr;
class  FS_CpuInfo;
class  FS_ThreadPool;
class  TimeSlice;
class  Time;
class  IFS_BusinessLogic;
class  ConcurrentMessageQueue;
struct NetMsg_DataHeader;
class  MessageQueue;
struct BriefSessionInfo;

class BASE_EXPORT FS_NetEngine
{
    friend class FS_IocpAcceptor;
public:
    FS_NetEngine();
    virtual ~FS_NetEngine();

    /* ����ӿ� */
    #pragma region api
public:
    virtual Int32 Init();
    virtual Int32 Start();
    virtual void Wait();
    virtual void Close();
    #pragma endregion
    
    /* �����¼� */
    #pragma region net event
    /*
    * brief: 
    *       1. 4 ����̴߳��� ����ȫ ���ֻ����1��FS_Server���ǰ�ȫ��
    *       2. _OnNetMonitorTask ������������ OnRun(�ɰ�) ��������߳�ȥ��monitor�����ǵ����̣߳���ɶ˿ڵ�get���̰߳�ȫ��
    *       3. OnNetJoin ��Ҽ��� �̲߳���ȫ
    *       4. OnNetLeave ��ҵ��� �̲߳���ȫ
    *       5. OnNetMsg �����Ϣ��������Ϣ�Ǵ�FS_Server��_HandleNetMsg���룩�̲߳���ȫ NetMsg_DataHeader ת���������߳���Ҫ����������Ϣ������
    *       6. OnNetRecv ���յ����� �̲߳���ȫ
            7. �ӿ�ֻ����msgtransfer���ȣ������̲߳���ȫ msghandler������fs_session����ֻ�ܴ���user����ʹ��userId��sessionId�������Ҳ�����session���������ݣ�ֻ����sessionId,���⵼���̲߳���ȫ
    */
protected:
    void _OnConnected(BriefSessionInfo *sessionInfo);
    void _OnDisconnected(IFS_Session *session);
    void _OnHeartBeatTimeOut(IFS_Session *session);
    // ÿ����һ������������һ��
    void _OnRecvMsg(IFS_Session *session, Int64 transferBytes);
    void _OnRecvMsgAmount(NetMsg_DataHeader *msgArrived);
    // ����ֻ��ͳ���ֽ����������޷�֧��ͳ��
    void _OnSendMsg(IFS_Session *session, Int64 transferBytes);

protected:
    // �������������
    virtual void _OnSvrRuning(FS_ThreadPool *threadPool);
    void _PrintSvrLoadInfo(const TimeSlice &dis);
    #pragma endregion

    // ��������д
protected:
    // ��ȡ����λ��
    virtual Int32 _OnReadCfgs() = 0;
    // ��ȡҵ���,�Ա�󶨵�dispatcher��
    virtual void _GetLogics(std::vector<IFS_BusinessLogic *> &logics) {}
    // ��ʼ������ʱ
    virtual Int32 _InitFinish() { return StatusDefs::Success; }

    /* �ڲ����� */
    #pragma region inner api
private:
    Int32 _ReadConfig();
    Int32 _CreateNetModules();
    Int32 _StartModules();

    Int32 _BeforeStart();
    Int32 _OnStart();
    Int32 _AfterStart();

    void _WillClose();
    void _BeforeClose();
    void _AfterClose();

    // �������Ľӿ�ע�ᵽģ����
    void _RegisterToModule();

    std::vector<MessageQueue *> &_GetSenderMq();
    #pragma endregion

private:
    std::atomic_bool _isInit;
    FS_CpuInfo *_cpuInfo;                               // cpu��Ϣ
    //IFS_ServerConfigMgr *_serverConfigMgr;            // ����������,�������������������,���ɱ�����

    Locker _locker;
    IFS_Connector * _connector;                         // ������
    UInt32 _acceptorQuantity;                            // ������������
    std::vector<IFS_Acceptor *> _acceptors;             // ֧�ּ�����˿ڣ����忴��������������
    UInt32 _transferCnt;                                 // ��Ϣ�շ�������
    std::vector<IFS_MsgTransfer *> _msgTransfers;       // ���߳���Ϣ�շ���
    ConcurrentMessageQueue *_messageQueue;              // ��Ϣ����
    std::vector<MessageQueue *> _senderMessageQueue;    // ������Ϣ����

    UInt32 _dispatcherCnt;
    Int64 _dispatcherResolutionInterval;                // ��Ϣ��������ʱ�����̾��� microseconds Ĭ��1ms
    std::vector<IFS_MsgDispatcher *> _msgDispatchers;   // ҵ����Ϣ������ ҵ���̴߳���,֧�ֶ��̲߳�������
    // std::vector<IFS_BusinessLogic *> _logics;        // ��ҵ���߼����� logicӦ�ò���ÿ��dispatcher��

    // TODO:sessionmgr������Ҫ�Ƴ���������ͻ
    ConditionLocker _waitForClose;                      // һ�������̣߳����������ȴ��������
    FS_ThreadPool *_pool;

    // ͳ������
    Time _lastStatisticsTime;                       // ���һ��ͳ�Ƶ�ʱ��
    TimeSlice _statisticsInterval;                  // ͳ�Ƶ�ʱ����
    std::atomic<Int64> _curSessionConnecting;       // �����ӵĻỰ����
    std::atomic<Int64> _sessionConnectedBefore;     // ��������ĻỰ����
    std::atomic<Int64> _sessionDisconnectedCnt;     // �Ͽ����ӵĻỰ����
    std::atomic<Int64> _recvMsgCountPerSecond;      // ÿ���յ��İ�����
    std::atomic<Int64> _recvMsgBytesPerSecond;      // ÿ���յ��İ����ֽ���
    std::atomic<Int64> _sendMsgBytesPerSecond;      // ÿ�뷢�͵İ��ֽ���
    std::atomic<Int64> _heartbeatTimeOutDisconnected;   // ������ʱ�Ͽ��Ự��

   // ���ӵĻỰ����
    Locker _sessionlocker;
    Int32 _curSessionCnt;
    Int32 _maxSessionQuantityLimit;
    UInt64 _curMaxSessionId;
    const UInt64 _maxSessionIdLimit;
    Int32 _connectTimeOutMs;
};

FS_NAMESPACE_END

#include "FrightenStone/common/net/Impl/FS_NetEngineImpl.h"

// extern BASE_EXPORT fs::FS_ServerCore *g_ServerCore;                         // �������
// extern BASE_EXPORT fs::IFS_ServerConfigMgr *g_SvrCfg;                       // ����������
// extern BASE_EXPORT fs::IFS_MsgDispatcher *g_Dispatcher;                     // ������ַ���
// extern BASE_EXPORT fs::IFS_BusinessLogic *g_Logic;                          // ҵ���߼���
// extern BASE_EXPORT fs::ConcurrentMessageQueue *g_net2LogicMessageQueue;     // ����㵽ҵ������Ϣ����


#endif
