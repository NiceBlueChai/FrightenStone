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
 * @file  : IFS_Connector.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_IFS_Connector_H__
#define __Base_Common_Net_Impl_IFS_Connector_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_ThreadPool;

class BASE_EXPORT IFS_Connector
{
public:
    IFS_Connector();
    virtual ~IFS_Connector();

public:
    virtual Int32 Start() = 0;
    virtual void Close() = 0;

//     /* TCP ������� */
//     #pragma region tcp normal operate
//     /*
//     * brief:
//     *       1. - InitSocket ��ʼ��Socket��
//     *       2. - Bind ��IP�Ͷ˿ں�
//     *       3. - Listen �����˿ں�
//     *       4. - Accept ���ܿͻ�������
//     *       5. - Start ���ݿ�ƽ̨�Ĳ�ͬ���в�ͬ��serverʵ�� ��Ҳ����Ƴ�ʹ�÷��͵�һ��ԭ��
//     *       6. - BeforeClose �ڹط�ǰ��Щ��������˳�iocp
//     *       7. - Close �رշ�����
//     */
// protected:
//     SOCKET _InitSocket();
//     Int32 _Bind(const Byte8 *ip, UInt16 port);
//     Int32 _Listen(Int32 unconnectQueueLen = SOMAXCONN);
//     SOCKET _Accept();
//     virtual void _BeforeClose();
//     void _Lock();
//     void _Unlock();
//     #pragma endregion
// 
//     /* ���� */
//     #pragma region misc
//     /*
//     * brief:
//     *       1. - _GetSocket ��ȡsocket
//     *       2. - _AddClientToFSServer ���ͻ��˼������
//     *       3. - _StatisticsMsgPerSecond ͳ�Ʋ����ÿ���յ���������Ϣ
//     */
// protected:
//     SOCKET _GetSocket();
// 
//     #pragma endregion

    /* ���ݳ�Ա */
//    #pragma region data member
// private:
//     // �߳� 
//     FS_ThreadPool *_threadPool;
//     // ��Ϣ��������ڲ��ᴴ���߳�
//     // ÿ����Ϣ��ʱ
//     Time _msgCountTime;
//     SOCKET _sock;
// 
// protected:
//     // �ͻ��˷��ͻ�������С
//     Int32 _sendBuffSize;
//     // �ͻ��˽��ջ�������С
//     Int32 _recvBuffSize;
//     // �ͻ�����������
//     Int32 _maxClient;
//     // SOCKET recv����
//     std::atomic<Int32> _recvCount;
//     // �յ���Ϣ����
//     std::atomic<Int32> _recvMsgCount;
//     // �ͻ����ܹ��������
//     std::atomic<Int32> _clientAcceptCnt;
//     // ��ǰ���������ѷ���ͻ��˼���
//     std::atomic<Int32> _clientJoinedCnt;
// 
//     Locker _locker;
//     UInt64 _clientMaxId;
//     const UInt64 _maxClientIdLimit;
//     Int64 _leaveClientCnt;
//     Int64 _joinClientCnt;
//     #pragma endregion
};

FS_NAMESPACE_END

#include "base/common/net/Impl/IFS_ConnectorImpl.h"

#endif
