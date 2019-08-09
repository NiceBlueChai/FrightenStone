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
 * @file  : FS_TcpServer.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/08/03
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_FS_TcpServer_H__
#define __Base_Common_Net_Impl_FS_TcpServer_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/net/Interface/INetEvent.h"
#include "base/common/component/Impl/Time.h"
#include "base/common/net/Defs/FS_NetDefs.h"
#include "base/common/component/Impl/FS_Delegate.h"
#include "base/common/component/Impl/FS_ThreadPool.h"
#include "base/common/asyn/asyn.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_Client;

class BASE_EXPORT FS_TcpServer : public INetEvent
{
    OBJ_POOL_CREATE(FS_TcpServer, _objPoolHelper);
public:
    FS_TcpServer();
    virtual ~FS_TcpServer();
    virtual void Release();

    /* TCP ������� */
    #pragma region tcp normal operate
    /*
    * brief:
    *       1. - InitSocket ��ʼ��Socket��
    *       2. - Bind ��IP�Ͷ˿ں�
    *       3. - Listen �����˿ں�
    *       4. - Accept ���ܿͻ�������
    *       5. - Start ���ݿ�ƽ̨�Ĳ�ͬ���в�ͬ��serverʵ�� ��Ҳ����Ƴ�ʹ�÷��͵�һ��ԭ��
    *       6. - BeforeClose �ڹط�ǰ��Щ��������˳�iocp
    *       7. - Close �رշ�����
    */
public:
    SOCKET InitSocket();
    Int32 Bind(const Byte8 *ip, UInt16 port);
    Int32 Listen(Int32 unconnectQueueLen = SOMAXCONN);
    SOCKET Accept();
    template<class ServerT>
    void Start(Int32 msgTransferSvrQuantity);
    virtual void BeforeClose();
    void Close();
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
    */
public:
    virtual void OnNetJoin(FS_Client *client);
    virtual void OnNetLeave(FS_Client *client);
    virtual Int32 OnNetMsg(FS_Server *server, FS_Client *client, NetMsg_DataHeader *header);
    virtual void OnPrepareNetRecv(FS_Client *client);
protected:
    virtual void _OnNetMonitorTask(const FS_ThreadPool *threadPool) = 0;
    #pragma endregion

    /* ���� */
    #pragma region misc
    /*
    * brief:
    *       1. - _GetSocket ��ȡsocket
    *       2. - _AddClientToFSServer ���ͻ��˼������
    *       3. - _StatisticsMsgPerSecond ͳ�Ʋ����ÿ���յ���������Ϣ
    */
protected:
    SOCKET _GetSocket();
    void _AddClientToFSServer(FS_Client *client);
    void _StatisticsMsgPerSecond();
    #pragma endregion

    /* ���ݳ�Ա */
    #pragma region data member
private:
    // �߳� 
    FS_ThreadPool *_threadPool;
    // ��Ϣ��������ڲ��ᴴ���߳�
    std::vector<FS_Server *> _fsServers;
    // ÿ����Ϣ��ʱ
    Time _msgCountTime;
    SOCKET _sock;

protected:
    // �ͻ��˷��ͻ�������С
    Int32 _sendBuffSize;
    // �ͻ��˽��ջ�������С
    Int32 _recvBuffSize;
    // �ͻ�����������
    Int32 _maxClient;
    // SOCKET recv����
    std::atomic<Int32> _recvCount;
    // �յ���Ϣ����
    std::atomic<Int32> _recvMsgCount;
    // �ͻ��˼���
    std::atomic<Int32> _clientAcceptCnt;
    // �ѷ���ͻ��˼���
    std::atomic<Int32> _clientJoinedCnt;

    Locker _locker;
    std::atomic<Int64> _clientMaxId;
    #pragma endregion
};


FS_NAMESPACE_END

#include "base/common/net/Impl/FS_TcpServerImpl.h"

#endif
