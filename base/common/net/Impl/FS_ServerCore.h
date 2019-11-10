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
 * @file  : FS_ServerCore.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_FS_ServerCore_H__
#define __Base_Common_Net_Impl_FS_ServerCore_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/asyn/asyn.h"
#include "base/common/component/Impl/Time.h"
#include "base/common/component/Impl/TimeSlice.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT IFS_Session;
class BASE_EXPORT IFS_ServerConfigMgr;
class BASE_EXPORT IFS_Connector;
class BASE_EXPORT IFS_MsgTransfer;
class BASE_EXPORT IFS_MsgDispatcher;
class BASE_EXPORT FS_SessionMgr;
class BASE_EXPORT FS_CpuInfo;
class BASE_EXPORT FS_ThreadPool;
class BASE_EXPORT TimeSlice;
class BASE_EXPORT Time;
class BASE_EXPORT IFS_BusinessLogic;
class BASE_EXPORT ConcurrentMessageQueue;
struct BASE_EXPORT NetMsg_DataHeader;
class BASE_EXPORT MessageQueue;

class BASE_EXPORT FS_ServerCore
{
    friend class FS_IocpMsgTransfer;
    friend class FS_IocpConnector;
    friend class FS_IocpMsgDispatcher;

public:
    FS_ServerCore();
    virtual ~FS_ServerCore();

    /* ����ӿ� */
    #pragma region api
public:
    virtual Int32 Start(IFS_BusinessLogic *businessLogic);
    virtual void Wait();
    virtual void Close();
    #pragma endregion
    
    /* �����¼� */
    #pragma region net event
    /*
    * brief: 
    *       1. FS_Server 4 ����̴߳��� ����ȫ ���ֻ����1��FS_Server���ǰ�ȫ��
    *       2. _OnNetMonitorTask ������������ OnRun(�ɰ�) ��������߳�ȥ��monitor�����ǵ����̣߳���ɶ˿ڵ�get���̰߳�ȫ��
    *       3. OnNetJoin ��Ҽ��� �̲߳���ȫ
    *       4. OnNetLeave ��ҵ��� �̲߳���ȫ
    *       5. OnNetMsg �����Ϣ��������Ϣ�Ǵ�FS_Server��_HandleNetMsg���룩�̲߳���ȫ NetMsg_DataHeader ת���������߳���Ҫ����������Ϣ������
    *       6. OnNetRecv ���յ����� �̲߳���ȫ
            7. �ӿ�ֻ����msgtransfer���ȣ������̲߳���ȫ msghandler������fs_session����ֻ�ܴ���user����ʹ��userId��sessionId�������Ҳ�����session���������ݣ�ֻ����sessionId,���⵼���̲߳���ȫ
    */
protected:
    void _OnConnected(IFS_Session *session);
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
    FS_CpuInfo *_cpuInfo;                            // cpu��Ϣ
    IFS_ServerConfigMgr *_serverConfigMgr;          // ����������

    Locker _locker;
    IFS_Connector * _connector;                         // ������
    std::vector<IFS_MsgTransfer *> _msgTransfers;       // ���߳���Ϣ�շ���
    ConcurrentMessageQueue *_messageQueue;              // ��Ϣ����
    std::vector<MessageQueue *> _senderMessageQueue;    // ������Ϣ����

    IFS_MsgDispatcher *_msgDispatcher;                 // ��Ϣ������ ҵ���̴߳���
    IFS_BusinessLogic *_logic;                      // ҵ���߼����

    // TODO:sessionmgr������Ҫ�Ƴ���������ͻ
    ConditionLocker _waitForClose;                  // һ�������̣߳����������ȴ��������
    FS_ThreadPool *_pool;

    // ͳ����
    Time _lastStatisticsTime;                       // ���һ��ͳ�Ƶ�ʱ��
    TimeSlice _statisticsInterval;                  // ͳ�Ƶ�ʱ����
    std::atomic<Int64> _curSessionConnecting;       // �����ӵĻỰ����
    std::atomic<Int64> _sessionConnectedBefore;     // ��������ĻỰ����
    std::atomic<Int64> _sessionDisconnectedCnt;     // �Ͽ����ӵĻỰ����
    std::atomic<Int64> _recvMsgCountPerSecond;      // ÿ���յ��İ�����
    std::atomic<Int64> _recvMsgBytesPerSecond;      // ÿ���յ��İ����ֽ���
    std::atomic<Int64> _sendMsgBytesPerSecond;      // ÿ�뷢�͵İ��ֽ���
    std::atomic<Int64> _heartbeatTimeOutDisconnected;   // ������ʱ�Ͽ��Ự��
};

FS_NAMESPACE_END

#include "base/common/net/Impl/FS_ServerCoreImpl.h"

extern BASE_EXPORT fs::FS_ServerCore *g_ServerCore;                         // �������
extern BASE_EXPORT fs::IFS_ServerConfigMgr *g_SvrCfg;                       // ����������
extern BASE_EXPORT fs::IFS_MsgDispatcher *g_Dispatcher;                     // ������ַ���
extern BASE_EXPORT fs::IFS_BusinessLogic *g_Logic;                          // ҵ���߼���
extern BASE_EXPORT fs::ConcurrentMessageQueue *g_net2LogicMessageQueue;     // ����㵽ҵ������Ϣ����

#endif
