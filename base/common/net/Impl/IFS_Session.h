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
 * @file  : IFS_Session.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/9/30
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_IFS_Session_H__
#define __Base_Common_Net_Impl_IFS_Session_H__
#pragma once
#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/objpool/objpool.h"
#include "base/common/component/Impl/Time.h"
#include "base/common/net/Defs/FS_NetDefs.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_SessionMgr;
class BASE_EXPORT IFS_Packet;
class BASE_EXPORT FS_Addr;
struct BASE_EXPORT NetMsg_DataHeader;
class BASE_EXPORT IFS_Buffer;

// TODO: IFS_Session, FS_IocpSession, FS_EpollSession

class BASE_EXPORT IFS_Session
{
    OBJ_POOL_CREATE_DEF(IFS_Session);
public:
    explicit IFS_Session(UInt64 sessionId, SOCKET sock, const sockaddr_in *addrInfo);
    virtual ~IFS_Session();

    // ��ȡ������״̬
public:
    UInt64 GetSessionId() const;
    SOCKET GetSocket() const;
    const FS_Addr *GetAddr() const;
    bool HasMsgToRead() const;      // Ҫ�ȴ���Ϣ�������������
    bool HasMsgToSend() const;      // Ҫ�ȴ���Ϣ�������������
    bool IsListen() const;          // �Ƿ�����׽���
    void SetListener(bool isListen);
    template<typename ObjType>
    ObjType *CastTo();
    template<typename ObjType>
    const ObjType *CastTo() const;
    bool IsClose() const;
    const Time &GetHeartBeatExpiredTime() const;
    virtual bool CanDestroy() const;
    virtual bool CanDisconnect() const;
    void MaskClose(); // ������post��������maskdestroy����������ʱ��������destroy��
    bool IsDelayClose() const;
    IFS_Buffer *GetRecvBuffer();

    // ����
public:
    void Close();
    void Lock();
    void Unlock();
    void UpdateHeartBeatExpiredTime();

    // һ��sessionֻͶ��һ��send�������ټ�������һ������شӶ���ͷ��ʼͶ��
    bool Send(NetMsg_DataHeader *header);   // ���ⲿ���õ�ʱ����ؼ���

    /* �¼� */
public:
    // �ͻ�������
    void OnDestroy();
    // �ͻ�������
    void OnConnect();
    void OnDisconnect();
    // �������ӳ�ʱ
    void OnHeartBeatTimeOut();
    void OnMsgArrived();

    // �ڲ�
private:
    void _Destroy();
    
protected:
    bool _isDestroy;
    bool _maskClose;
    Locker _lock;
    UInt64 _sessionId;
    FS_Addr *_addr;
    SOCKET _sock;
    std::atomic<Int32> _lastErrorReason;
    bool _isListen;
    Time _heartBeatExpiredTime; // ��������ʱ��
    IFS_Buffer *_recvBuffer;
    std::list<IFS_Buffer *> _toSend;

    // ����server�����յ�����ϢID�Ƿ����� ÿ�յ�һ���ͻ�����Ϣ������1�Ա���ͻ��˵�msgidУ�飬��ƥ���򱨴���˵�������ȣ�
    Int32 _recvMsgId;
    // ���Խ��շ��߼���
    // ����client�����յ�����ϢID�Ƿ����� ÿ����һ����Ϣ������1�Ա���ͻ��˵�sendmsgidУ�飬��ƥ����ͻ��˱���˵�������ȣ�
    Int32 _sendMsgId;
};

FS_NAMESPACE_END

#include "base/common/net/Impl/IFS_SessionImpl.h"

#endif
