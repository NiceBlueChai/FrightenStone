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
 * @file  : Example.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/11
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/Iocp/Example.h"

#include <windows.h>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
#include<MSWSock.h>
#pragma comment (lib, "MSWSock.lib")
FS_NAMESPACE_BEGIN

#define IO_DATA_BUFF_SIZE 1024
#define CLIENT_QUANTITY 10

class IO_Defs
{
public:
    enum IO_TYPE
    {
        IO_ACCEPT = 10,
        IO_RECV,
        IO_SEND,
    };
};

struct IO_DATA_BASE
{
    // �ص���
    OVERLAPPED _overlapped;    // ʹ���ص�����Թ�����iodatabase
    SOCKET _sock;
    char _buff[IO_DATA_BUFF_SIZE];
    Int32 _length;
    IO_Defs::IO_TYPE _ioType;
};

void PostAccept(SOCKET sockServer, IO_DATA_BASE *ioData)
{
    ioData->_ioType = IO_Defs::IO_ACCEPT;
    ioData->_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(!AcceptEx(sockServer
                 , ioData->_sock
                 , ioData->_buff
                 , 0
                 , sizeof(sockaddr_in) + 16
                 , sizeof(sockaddr_in) + 16
                 , NULL
                 , &ioData->_overlapped)) // �������Զ���Ľṹ��
    {
        auto error = WSAGetLastError();
        if(error != ERROR_IO_PENDING)
        {
            printf("AcceptEx failed error[%d]", error);
            return;
        }
    }
}

// Ͷ�ݽ�������
bool PostRecv(IO_DATA_BASE *ioData)
{
    ioData->_ioType = IO_Defs::IO_RECV;
    WSABUF wsBuff = {};
    wsBuff.buf = ioData->_buff;
    wsBuff.len = sizeof(ioData->_buff);
    DWORD flags = 0;
    memset(&ioData->_overlapped, 0, sizeof(ioData->_overlapped));
    if(SOCKET_ERROR == WSARecv(ioData->_sock, &wsBuff, 1, NULL, &flags, &ioData->_overlapped, NULL))
    {
        auto error = WSAGetLastError();
        if(error != ERROR_IO_PENDING)
        {
            printf("WSARecv failed error[%d]", error);
            return false;
        }
    }

    return true;
}

bool PostSend(IO_DATA_BASE *ioData)
{
    ioData->_ioType = IO_Defs::IO_SEND;
    WSABUF wsBuff = {};
    wsBuff.buf = ioData->_buff;
    wsBuff.len = ioData->_length;
    DWORD flags = 0;
    memset(&ioData->_overlapped, 0, sizeof(ioData->_overlapped));
    if(SOCKET_ERROR == WSASend(ioData->_sock, &wsBuff, 1, NULL, flags, &ioData->_overlapped, NULL))
    {
        auto error = WSAGetLastError();
        if(error != ERROR_IO_PENDING)
        {
            printf("WSASend failed error[%d]", error);
            return false;
        }
    }

    return true;
}

int Example::Run()
{
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);

    // 1. ��������socket
    SOCKET sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   // Ĭ���ڲ��������ص���־

    // 2.�󶨶˿�
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);    // ת�������ֽ���
    _sin.sin_addr.s_addr = INADDR_ANY;  // ����ip
    if(SOCKET_ERROR == bind(sockServer, reinterpret_cast<sockaddr *>(&_sin), sizeof(_sin)))
    {
        const auto err = GetLastError();
        printf("���󣬰�����Ͽ�ʧ��error<%d>...\n", err);
        return err;
    }
    else
    {
        printf("��������˿ڳɹ�\n");
    }

    // 3.�����˿�
    if(SOCKET_ERROR == listen(sockServer, 64))
    {// ���64���ȴ������Ķ��У�
        printf("����,��������˿�ʧ��...");
    }
    else
    {
        printf("��������˿ڳɹ�...\n");
    }

    // ������ɶ˿�IOCP NumberOfConcurrentThreads=0��ʾĬ��cpu����
    auto _completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if(!_completionPort)
    {
        auto err = GetLastError();
        printf("CreateIoCompletionPort failed error code<%d>", err);
        return err;
    }

    // ����IOCP �� ServerSocket
    // completionKey�����һ����ֵ�����ʱ��ԭ��������; NumberOfConcurrentThreads��������ڹ�����ɶ˿�ʱ������
    auto ret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(sockServer), _completionPort, (ULONG_PTR)(sockServer), 0); // completekey�������Զ���Ľṹ��ָ������������ݵ�ָ�룬���ڻ�ȡ���״̬ʱ��ʶ��
    if(!ret)
    {
        auto err = GetLastError();
        printf("CreateIoCompletionPort associated failure error code<%d>", err);
        return err;
    }

    // ��IOCPͶ�ݽ��ܿͻ������ӵ�����
    // sAcceptSocketԤ�ȴ�����socket������socket������ϵͳ��Դ��socket��Դ���޹���Ҫ����һ��socket�أ�������Դ��������
    // dwReceiveDataLength ��Ч�������ݳ��ȣ���Ϊ0��ʾ����ʱ���صȴ��ͻ��˷�������acceptex����ɣ�����ֵ��ʾ��Ҫ�ȴ��ͻ��˷������ݲ����
    // lpdwBytesReceived���ؽ������ݳ��ȣ�������ȴ��ͻ��˷������ݣ�����ط�������0
    IO_DATA_BASE ioData[CLIENT_QUANTITY] = {};
    for(Int32 i = 0; i < CLIENT_QUANTITY; ++i)
        PostAccept(sockServer, &ioData[i]);

    Int32 msgCount = 0;
    while(true)
    {
        // ��ȡ��ɶ˿�״̬
        DWORD bytesTrans = 0;
        SOCKET sock = INVALID_SOCKET;
        IO_DATA_BASE *ioDataPtr = NULL;
        // �ؼ����� completekey(����iocp�˿�ʱ������Զ�����ɼ�)
        // �Լ��ص��ṹioDataPtr ���ڻ�ȡ����
        if(FALSE == GetQueuedCompletionStatus(_completionPort, &bytesTrans, (PULONG_PTR)&sock, (LPOVERLAPPED *)&ioDataPtr, INFINITE))
        {
            const Int32 error = GetLastError();
            printf("GetQueuedCompletionStatus failed with error %d\n", error);
            if(ERROR_NETNAME_DELETED == error)
            {
                printf("�ͻ��˶Ͽ����ر� sockfd=%d\n", static_cast<Int32>(ioDataPtr->_sock));
                closesocket(ioDataPtr->_sock);
                continue;
            }
            break;
        }

        // ����������
        if(ioDataPtr->_ioType == IO_Defs::IO_ACCEPT)
        {
            printf("�¿ͻ������� sockfd=%d\n", static_cast<Int32>(ioDataPtr->_sock));

            // clientsocket������ɶ˿�
            auto associateRet = CreateIoCompletionPort(reinterpret_cast<HANDLE>(ioDataPtr->_sock), _completionPort, (ULONG_PTR)(ioDataPtr->_sock), 0);
            if(!associateRet)
            {
                auto err = GetLastError();
                printf("CreateIoCompletionPort associated clientsock[%d] failure error code<%d>", static_cast<Int32>(ioDataPtr->_sock), err);
                closesocket(ioDataPtr->_sock);
                continue;
            }

            // Ͷ�ݽ�������
            if(!PostRecv(ioDataPtr))
            {
                printf("post recv fail sock[%d]\n", static_cast<Int32>(ioDataPtr->_sock));
                closesocket(ioDataPtr->_sock);
                continue;
            }

            // Ͷ�ݽ�������
//             for(Int32 i=0;i<10;++i)
//                 if(!PostRecv(ioDataPtr))
//                 {
//                     printf("post recv fail sock[%d]\n", static_cast<Int32>(ioDataPtr->_sock));
//                     closesocket(ioDataPtr->_sock);
//                     continue;
//                 }
        }
        else if(ioDataPtr->_ioType == IO_Defs::IO_RECV)
        {
            if(bytesTrans <= 0)
            {
                printf("recv error socket[%d], bytesTrans[%d]\n"
                       , static_cast<Int32>(ioDataPtr->_sock), bytesTrans);
                closesocket(ioDataPtr->_sock);
                continue;
            }

            // ��ӡ���յ�������
            printf("recv data :socket[%d], bytesTrans[%d] msgCount[%d]\n"
                   , static_cast<Int32>(ioDataPtr->_sock), bytesTrans, ++msgCount);

            // ��ͣ�Ľ�������
            ioDataPtr->_length = bytesTrans;
            PostSend(ioDataPtr);
        }
        else if(ioDataPtr->_ioType == IO_Defs::IO_SEND)
        {
            // �ͻ��˶Ͽ�����
            if(bytesTrans <= 0)
            {
                printf("send error socket[%d], bytesTrans[%d]\n"
                       , static_cast<Int32>(ioDataPtr->_sock), bytesTrans);
                closesocket(ioDataPtr->_sock);
                continue;
            }

            // ��ӡ���͵�����
            printf("send data :socket[%d], bytesTrans[%d] msgCount[%d]\n"
                   , static_cast<Int32>(ioDataPtr->_sock), bytesTrans, msgCount);

            PostRecv(ioDataPtr);
        }
        else
        {
            printf("δ������Ϊ sockefd=%d", static_cast<Int32>(sock));
        }
        // ����Ƿ����¼���������selet,epoll_wait����
        // �������� ���
        // �������� ��� completion
        // �������� ���
        // ��IOCP Ͷ�ݽ�����������
    }

    // ------------ IOCP end ------------ //
    // �ر�clientsocket
    for(Int32 i = 0; i < CLIENT_QUANTITY; ++i)
        closesocket(ioData[i]._sock);
    // �ر�serversocket
    closesocket(sockServer);
    // �ر���ɶ˿�
    CloseHandle(_completionPort);

    // ���windows socket����
    WSACleanup();
    return 0;
}

FS_NAMESPACE_END
