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
 * @file  : FS_IocpConnector.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_FS_IocpConnector_H__
#define __Base_Common_Net_Impl_FS_IocpConnector_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/net/Impl/IFS_Connector.h"
#include "base/common/component/Impl/FS_Delegate.h"
#include "base/common/component/Impl/FS_ThreadPool.h"
#include "base/common/asyn/asyn.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT IFS_Session;

class BASE_EXPORT FS_IocpConnector : public IFS_Connector
{
public:
    FS_IocpConnector();
    virtual ~FS_IocpConnector();

public:
    virtual Int32 BeforeStart();
    virtual Int32 Start();
    virtual void BeforeClose();
    virtual void Close();
    virtual void RegisterConnected(IDelegate<void, IFS_Session *> *callback);
   
    /* TCP ������� */
    #pragma region tcp normal operate
    /*
    * brief:
    *       1. - InitSocket ��ʼ��Socket��
    *       2. - Bind ��IP�Ͷ˿ں�
    *       3. - Listen �����˿ں�
    */
private:
    void _ReadConfig();
    SOCKET _InitSocket();
    Int32 _Bind(const Byte8 *ip, UInt16 port);
    Int32 _Listen(Int32 unconnectQueueLen = SOMAXCONN);
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
private:
    void _OnConnected(SOCKET sock, const sockaddr_in *addrInfo);
    void _OnIocpMonitorTask(const FS_ThreadPool *threadPool);
    #pragma endregion

    /* ���ݳ�Ա */
    #pragma region data member
private:
    // �߳� 
    FS_ThreadPool *_threadPool;
    SOCKET _sock;

    // �����¼��ص�
    IDelegate<void, IFS_Session *> *_onConnected;
    IDelegate<void> *_closeIocpDelegate;

    // �ͻ�����������
    Locker _locker;
    Int32 _curSessionCnt;
    Int32 _maxSessionQuantityLimit;
    UInt64 _curMaxSessionId;
    const UInt64 _maxSessionIdLimit;
#pragma endregion
};

FS_NAMESPACE_END

#endif
