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

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_NetBuffer;

class BASE_EXPORT FS_Client
{
    OBJ_POOL_CREATE(FS_Client, _objPoolHelper);
public:
    FS_Client(Int64 clientId
              , SOCKET sockfd = INVALID_SOCKET
              , int sendSize = SEND_BUFF_SZIE
              , int recvSize = RECV_BUFF_SZIE);
    ~FS_Client();

public:
    Int32 RecvData();
    // ���������ͻ����������ݷ��͸��ͻ���
    Int32 SendDataReal();
    // �������� �������Ŀ��Ƹ���ҵ������Ĳ��������
    Int32 SendData(NetMsg_DataHeader *header);
    // ����ֵ len �� SOCKET_ERROR
    Int32 SendData(const char *data, Int32 len);
    NetMsg_DataHeader *FrontMsg();
    void PopFrontMsg();

    void ResetDTHeart();
    void ResetDTSend();
    // ������� ���������Ҫһ��set����
    bool CheckHeart(const TimeSlice &slice);
    // ��ʱ������Ϣ���
    bool CheckSend(const TimeSlice &dt);

    #ifdef FS_USE_IOCP
    IO_DATA_BASE *MakeRecvIoData();
    void OnRecvFromIocp(Int32 rcvBytes);

    IO_DATA_BASE *MakeSendIoData();
    void OnSend2iocp(Int32 snd);

    bool IsPostIoChange() const;
    #endif // FS_USE_IOCP

    /* ���� */
    #pragma region misc
public:
    void Destroy();
    SOCKET GetSocket() const;
    bool HasRecvMsg() const;
    bool NeedWrite() const;
    bool IsDestroy() const;
    #pragma endregion

    //////////���ڵ��Եĳ�Ա����
public:
    Int64 _id;
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
    // socket fd_set  file desc set
    SOCKET _sockfd;
    // �ڶ������� ������Ϣ������
    FS_NetBuffer *_recvBuff;
    // ���ͻ�����
    FS_NetBuffer *_sendBuff;
    // ����������ʱ
    TimeSlice _heartDeadSlice; // ���������Ż�ʹ��ʱ���TODO
    // �ϴη�����Ϣ���ݵ�ʱ�� 
    TimeSlice _lastSendSlice; // ���ڶ�ʱ����
#ifdef FS_USE_IOCP
    bool _isPostRecv = false;
    bool _isPostSend = false;
#endif
};

FS_NAMESPACE_END

#include "base/common/net/Impl/FS_ClientImpl.h"

#endif
