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
 * @file  : FS_IocpAcceptor.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/12/08
 * @brief :
 * 
 *
 * 
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_FS_IocpAcceptor_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_FS_IocpAcceptor_H__

#pragma once


#ifdef _WIN32

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/net/Impl/IFS_Acceptor.h"
#include "FrightenStone/common/component/Impl/FS_Delegate.h"
#include "FrightenStone/common/component/Impl/FS_ThreadPool.h"
#include "FrightenStone/common/asyn/asyn.h"
#include "FrightenStone/common/component/Impl/FS_String.h"

FS_NAMESPACE_BEGIN

class IFS_Session;
class FS_IniFile;
class FS_Iocp;
struct IoDataBase;
struct BriefListenAddrInfo;

class BASE_EXPORT FS_IocpAcceptor : public IFS_Acceptor
{
public:
    FS_IocpAcceptor(Locker &sessionLocker
                    ,Int32 &curSessionCnt
                    ,Int32 &maxSessionQuantityLimit
                    ,UInt64 &curMaxSessionId
                    ,const UInt64 &maxSessionIdLimit, FS_NetEngine *netEngine);
    virtual ~FS_IocpAcceptor();

public:
    virtual Int32 BeforeStart();
    virtual Int32 Start();
    virtual void BeforeClose();
    virtual void Close();
    virtual void OnDisconnected(IFS_Session *session);
    // ������ַ
    virtual void SetListenAddrInfo(const BriefListenAddrInfo &listenAddrInfo);
    /* TCP ������� */
    #pragma region tcp normal operate
    /*
    * brief:
    *       1. - InitSocket ��ʼ��Socket��
    *       2. - Bind ��IP�Ͷ˿ں�
    *       3. - Listen �����˿ں�
    */
private:
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
    void _OnConnected(SOCKET sock, const sockaddr_in *addrInfo, FS_Iocp *iocpListener);
    void _OnIocpMonitorTask(FS_ThreadPool *threadPool);
    void _PreparePostAccept(FS_Iocp *listenIocp, char **&bufArray, IoDataBase **&ioDataArray);
    void _FreePrepareAcceptBuffers(char **&bufArray, IoDataBase **&ioDataArray);

    #pragma endregion

    /* ���ݳ�Ա */
    #pragma region data member
private:
    // �߳� 
    FS_ThreadPool *_threadPool;
    SOCKET _sock;

    // �����¼��ص�
    IDelegate<void> *_closeIocpDelegate;

    // �ͻ�����������
    Locker &_locker;
    Int32 &_curSessionCnt;
    Int32 &_maxSessionQuantityLimit;
    UInt64 &_curMaxSessionId;
    const UInt64 &_maxSessionIdLimit;
    std::set<UInt64> _sucConnectedSessionIds;

    BriefListenAddrInfo *_listenAddrInfo;

#pragma endregion
};

FS_NAMESPACE_END

#endif

#endif
