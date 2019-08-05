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

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_Client
{
public:
    FS_Client(Int64 clientId
              , SOCKET sockfd = INVALID_SOCKET
              , int sendSize = SEND_BUFF_SZIE
              , int recvSize = RECV_BUFF_SZIE);
    ~FS_Client();

    void Init();
    void Destory();
    SOCKET GetSocket();
    int RecvData()
    {
        return _recvBuff.Read4socket(_sockfd);
    }

    bool hasMsg()
    {
        return _recvBuff.hasMsg();
    }

    netmsg_DataHeader* front_msg()
    {
        return (netmsg_DataHeader*)_recvBuff.data();
    }

    void pop_front_msg()
    {
        if(hasMsg())
            _recvBuff.pop(front_msg()->dataLength);
    }

    bool needWrite()
    {
        return _sendBuff.needWrite();
    }

    //���������ͻ����������ݷ��͸��ͻ���
    int SendDataReal()
    {
        resetDTSend();
        return _sendBuff.write2socket(_sockfd);
    }

    //�������Ŀ��Ƹ���ҵ������Ĳ��������
    //��������
    int SendData(netmsg_DataHeader* header)
    {
        return SendData((const char*)header, header->dataLength);
    }

    int SendData(const char* pData, int len)
    {
        if(_sendBuff.push(pData, len))
        {
            return len;
        }
        return SOCKET_ERROR;
    }

    void ResetDTHeart()
    {
        _heartDeadTime = 0;
    }

    void ResetDTSend()
    {
        _lastSendTime = 0;
    }

    //�������
    bool checkHeart(time_t dt)
    {
        _dtHeart += dt;
        if(_dtHeart >= CLIENT_HREAT_DEAD_TIME)
        {
            CELLLog_Info("checkHeart dead:s=%d,time=%ld", _sockfd, _dtHeart);
            return true;
        }
        return false;
    }

    //��ʱ������Ϣ���
    bool checkSend(time_t dt)
    {
        _dtSend += dt;
        if(_dtSend >= CLIENT_SEND_BUFF_TIME)
        {
            //CELLLog_Info("checkSend:s=%d,time=%d", _sockfd, _dtSend);
            //���������ͻ����������ݷ��ͳ�ȥ
            SendDataReal();
            //���÷��ͼ�ʱ
            resetDTSend();
            return true;
        }
        return false;
    }

#ifdef FS_USE_IOCP
    IO_DATA_BASE* makeRecvIoData()
    {
        if(_isPostRecv)
            return nullptr;
        _isPostRecv = true;
        return _recvBuff.makeRecvIoData(_sockfd);
    }
    void recv4iocp(int nRecv)
    {
        if(!_isPostRecv)
            CELLLog_Error("recv4iocp _isPostRecv is false");
        _isPostRecv = false;
        _recvBuff.read4iocp(nRecv);
    }

    IO_DATA_BASE* makeSendIoData()
    {
        if(_isPostSend)
            return nullptr;
        _isPostSend = true;
        return _sendBuff.makeSendIoData(_sockfd);
    }

    void send2iocp(int nSend)
    {
        if(!_isPostSend)
            CELLLog_Error("send2iocp _isPostSend is false");
        _isPostSend = false;
        _sendBuff.write2iocp(nSend);
    }

    bool isPostIoAction()
    {
        return _isPostRecv || _isPostSend;
    }
#endif // FS_USE_IOCP

    //////////���ڵ��Եĳ�Ա����
public:
    Int64 _id = -1;
    // ����serverid
    int _serverId = -1;
    // ���Խ��շ��߼���
    // ����server�����յ�����ϢID�Ƿ�����
    int _recvMsgId = 1;
    // ���Խ��շ��߼���
    // ����client�����յ�����ϢID�Ƿ�����
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
    Time _heartDeadTime;
    // �ϴη�����Ϣ���ݵ�ʱ�� 
    Time _lastSendTime;
#ifdef FS_USE_IOCP
    bool _isPostRecv = false;
    bool _isPostSend = false;
#endif
};

FS_NAMESPACE_END

#include "base/common/net/Impl/FS_ClientImpl.h"

#endif
