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
 * @file  : NetMessageBlock.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/05/04
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Defs_NetMessageBlock_H__
#define __Frame_Include_FrightenStone_Common_Net_Defs_NetMessageBlock_H__

#pragma once

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/component/Impl/MessageQueue/MessageQueue.h"
#include "FrightenStone/common/net/Defs/FS_ConnectInfo.h"
#include "FrightenStone/common/net/Defs/FS_NetDefs.h"
#include "FrightenStone/common/net/Defs/IoEvDefs.h"
#include "FrightenStone/common/component/Impl/FS_Delegate.h"
#include "FrightenStone/common/net/Defs/EpollDefs.h"

#ifdef _WIN32
class IFS_NetModule;

#else
#pragma region linux
#include <unistd.h>
#include <sys/epoll.h>
#pragma endregion

class IFS_BasePoller;
#endif

FS_NAMESPACE_BEGIN

// ö�ٿ��ܻᶯ̬�仯���ɳ־û�
class BASE_EXPORT NetMessageType
{
public:
    enum 
    {
        None = 0,                       // ��Ч

        SessionConnected = 1,                   // ���� transfer -> dispatcher
        PostConnectionFailureToDispatcher = 2,  // ��dispatcher ������ʧ�ܵ���Ϣ������ִstub�ģ�
        PostAsynConnectionToConnector = 3,      // ��connector��������Ϣ

        #pragma region iocp
        IocpMsgArrived,                 // �յ������ Transfer to Dispatcher
        #pragma endregion

        #pragma region Epoll
        EpollEvArriveTriggers,      // �����¼�����

        EpollInTrigger,             // epoll�ɶ��¼�
        EpollOutTrigger,            // ��д�¼�

        PostRecv,                   // Ͷ�ݽ������� dispatcher -> transfer
        PostSend,                   // Ͷ�ݷ������� dispatcher -> transfer

        RecvDataEv,                 // �յ����� transfer -> dispatcher
        SendSucEv,                  // ���ͳɹ��¼� transfer -> dispatcher

        PostCancelIoToRecver,      // ��transfer�� recver��ȡ���Ựio��Ϣ
        PostCancelIoToSender,      // ��transfer�� sender��ȡ���Ựio����Ϣ
        #pragma endregion

        SessionDisconnected,        // ���ӶϿ�

        End,
    };
};


struct BASE_EXPORT FS_NetMsgBlock : public FS_MessageBlock
{
    OBJ_POOL_CREATE_DEF(FS_NetMsgBlock);

    FS_NetMsgBlock(UInt32 compId = 0, UInt32 generatorId = 0, Int32 messageType = NetMessageType::None);
    ~FS_NetMsgBlock();

    UInt32 _compId;                     // ���id һ����transfercompId(��Ϣ������)
    UInt32 _generatorId;                // ������id
    Int32 _messageType;                 // NetMessageType
};

// �Ự��Ϣ����
struct BASE_EXPORT SessionConnectedNetMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(SessionConnectedNetMsgBlock);

    SessionConnectedNetMsgBlock(UInt32 compId, UInt32 generatorId);
    ~SessionConnectedNetMsgBlock();

    FS_String ToString();

    UInt32 _acceptorCompId;             // ����compId
    UInt32 _connectorCompId;
    UInt16 _protocolPort;
    Int32 _srcCompType;
    UInt64 _sessionId;
    SOCKET _sock;
    sockaddr_in _addrInfo;

    // TODO:
    #ifdef _WIN32
    IFS_NetModule *_iocp;
    #else
    IFS_BasePoller *_poller;                // transfer poller
    #endif

    /* �������Ķ������ */
    IDelegate<void, UInt64> *_onUserDisconnectedRes;    // UInt64 :sessionId
    UInt64 _stub;   // ���
};

// ����ʧ��
struct BASE_EXPORT PostConnectionFailureToDispatcherMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(PostConnectionFailureToDispatcherMsgBlock);

    PostConnectionFailureToDispatcherMsgBlock(const FS_ConnectInfo &connectInfo, Int32 resultCode);

    FS_ConnectInfo _connectInfo;
    Int32 _resultCode;
};

// �첽���ӷ���
struct BASE_EXPORT PostAsynConnectionToConnectorMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(PostAsynConnectionToConnectorMsgBlock);

    PostAsynConnectionToConnectorMsgBlock(const FS_ConnectInfo &connectInfo);

    FS_ConnectInfo _connectInfo;
    UInt32 _dispatcherMsgQueueConsumerId;   // dispatcher��Ϊ��Ϣ���е�������
};

// ���ӶϿ�
struct BASE_EXPORT SessionDisconnectedMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(SessionDisconnectedMsgBlock);

    SessionDisconnectedMsgBlock(UInt64 sessionId);
    UInt64 _sessionId;
};

#ifdef _WIN32

// �Ự��Ϣ����
struct BASE_EXPORT IocpMsgArrivedMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(IocpMsgArrivedMsgBlock);
    IocpMsgArrivedMsgBlock();
    ~IocpMsgArrivedMsgBlock();

    IoEvent *_ioEv;                     // ����ش���
    Int32 _errorCode;                   // ���������
};

#else

struct BASE_EXPORT EpollEvArriveMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(EpollEvArriveMsgBlock);

    EpollEvArriveMsgBlock();
    ~EpollEvArriveMsgBlock() {}

    epoll_event _ev[__FS_MAX_EVENT_NUM_TO_HANDLE_ONCE_DEF__];   // �������¼�
    Int32 _triggerAmount;                                       // �¼��������������
};

struct BASE_EXPORT EpollEvEpollInEvMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(EpollEvEpollInEvMsgBlock);
    EpollEvEpollInEvMsgBlock();
    ~EpollEvEpollInEvMsgBlock() {}

    UInt64 _sessionId;
};

struct BASE_EXPORT EpollEvEpollOutEvMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(EpollEvEpollOutEvMsgBlock);
    EpollEvEpollOutEvMsgBlock();
    ~EpollEvEpollOutEvMsgBlock() {}

    UInt64 _sessionId;
};

struct BASE_EXPORT PostRecvMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(PostRecvMsgBlock);

    PostRecvMsgBlock();

    UInt64 _sessionId;
    EpollIoData *_ioData;
};

struct BASE_EXPORT PostSendMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(PostSendMsgBlock);

    PostSendMsgBlock();

    UInt64 _sessionId;
    EpollIoData *_ioData;
};

struct BASE_EXPORT EpollRecvDataMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(EpollRecvDataMsgBlock);

    EpollRecvDataMsgBlock(UInt32 compId, UInt32 generatorId);

    UInt64 _sessionId;
    Int64 _transferBytes;
    EpollIoData *_ioData;
    bool _isSessionInRecverMonitor;
};

struct BASE_EXPORT EpollSendSucMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(EpollSendSucMsgBlock);

    EpollSendSucMsgBlock(UInt32 compId, UInt32 generatorId);

    UInt64 _sessionId;
    Int64 _transferBytes;
    EpollIoData *_ioData;
    bool _isSessionInSenderMonitor;
};


struct BASE_EXPORT PostCancelIoToRecverMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(PostCancelIoToRecverMsgBlock);

    PostCancelIoToRecverMsgBlock(UInt64 sessionId, Int32 reason, bool giveRes);

    UInt64 _sessionId;
    Int32 _reason;                  // �Ͽ�ԭ��
    bool _giveRes;                  // ������Ϣ
};

struct BASE_EXPORT PostCancelIoToSenderMsgBlock : public FS_NetMsgBlock
{
    OBJ_POOL_CREATE_DEF(PostCancelIoToSenderMsgBlock);

    PostCancelIoToSenderMsgBlock(UInt64 sessionId, Int32 reason, bool giveRes);

    UInt64 _sessionId;
    Int32 _reason;                  // �Ͽ�ԭ��
    bool _giveRes;                  // ������Ϣ
};
#endif

FS_NAMESPACE_END

#include "FrightenStone/common/net/Defs/NetMessageBlockImpl.h"

#endif
