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
 * @file  : FS_Client.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/5
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_FS_Client_H__
#define __Base_Common_Net_Impl_FS_Client_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/net/Defs/FS_NetDefs.h"
#include "base/common/component/Impl/Time.h"
#include "base/common/net/Defs/FS_NetBuffer.h"
#include "base/common/net/protocol/protocol.h"
#include "base/common/component/Impl/TimeSlice.h"
#include "base/common/net/Defs/FS_NetBufferArray.h"
#include "base/common/assist/utils/Impl/STLUtil.h"
#include "base/common/component/Impl/FS_Delegate.h"

FS_NAMESPACE_BEGIN
class BASE_EXPORT  FS_Packet;
class BASE_EXPORT FS_Client
{
    OBJ_POOL_CREATE(FS_Client, _objPoolHelper);
public:
    FS_Client(UInt64 clientId
              , SOCKET sockfd = INVALID_SOCKET
              , int sendSize = SEND_BUFF_SZIE
              , int recvSize = RECV_BUFF_SZIE);
    ~FS_Client();

public:
    // �������� �������Ŀ��Ƹ���ҵ������Ĳ��������
    void SendData(NetMsg_DataHeader *header);
    // ����ֵ len �� SOCKET_ERROR
    void SendData(const char *data, Int32 len);
    void SetSender(IDelegatePlus<void, FS_Packet *> *sendFunc);

    NetMsg_DataHeader *FrontRecvMsg();
    void PopRecvFrontMsg();

    void UpdateHeartBeatExpiredTime();

    #ifdef FS_USE_IOCP
    IO_DATA_BASE *MakeRecvIoData();

    bool IsPostIoChange() const;
    bool IsPostSend() const;
    bool IsPostRecv() const;
    void ResetPostRecv();
    #endif // FS_USE_IOCP

private:
    void _SendData(const char *data, Int32 len);

    /* ���� */
    #pragma region misc
public:
    SOCKET GetSocket() const;
    UInt64 GetId() const;
    bool HasRecvMsg() const;
    bool IsDestroy() const;
    const Time &GetHeartBeatExpiredTime() const;
    void Close();

    #pragma endregion

    //////////���ڵ��Եĳ�Ա����
public:
    // ����serverid
    int _serverId = -1;
    // ���Խ��շ��߼���
    // ����server�����յ�����ϢID�Ƿ����� ÿ�յ�һ���ͻ�����Ϣ������1�Ա���ͻ��˵�msgidУ�飬��ƥ���򱨴���˵�������ȣ�
    int _recvMsgId = 1;
    // ���Խ��շ��߼���
    // ����client�����յ�����ϢID�Ƿ����� ÿ����һ����Ϣ������1�Ա���ͻ��˵�sendmsgidУ�飬��ƥ����ͻ��˱���˵�������ȣ�
    int _sendMsgId = 1;
    ///////////////////////////////////

private:
    // ÿ��FS_Server Ψһid
    UInt64 _id;
    // socket fd_set  file desc set
    SOCKET _sockfd;
    // �ڶ������� ������Ϣ������
    FS_Packet *_recvBuff;
    // ���ͻ�����
    IDelegatePlus<void, FS_Packet *> *_sendFunc;
    // ��������ʱ��
    Time _heartBeatExpiredTime; // ��������ʱ��

#ifdef FS_USE_IOCP
    bool _isPostRecv = false;
    bool _isPostSend = false;
#endif
};

FS_NAMESPACE_END

#include "base/common/net/Impl/FS_ClientImpl.h"

#endif
