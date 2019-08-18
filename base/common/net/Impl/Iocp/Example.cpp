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
#include "base/common/net/Impl/FS_Iocp.h"
#include "base/common/status/status.h"
#include "base/common/log/Log.h"
#include "base/common/net/Defs/IocpDefs.h"

#include <windows.h>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
#include<MSWSock.h>
#pragma comment (lib, "MSWSock.lib")
FS_NAMESPACE_BEGIN

#define IO_DATA_BUFF_SIZE 1024
#define CLIENT_QUANTITY 10

// LPFN_ACCEPTEX __g_fnAccept = NULL;
// 
// class IO_Defs
// {
// public:
//     enum IO_TYPE
//     {
//         IO_ACCEPT = 10,
//         IO_RECV,
//         IO_SEND,
//     };
// };
// 
// struct IO_DATA_BASE
// {
//     // �ص���
//     OVERLAPPED _overlapped;    // ʹ���ص�����Թ�����iodatabase
//     SOCKET _sock;
//     char _buff[IO_DATA_BUFF_SIZE];
//     Int32 _length;
//     IO_Defs::IO_TYPE _ioType;
// };
// 
// void LoadAcceptEx(SOCKET listenSocket)
// {
//     DWORD dwBytes = 0;
//     GUID guidAcceptEx = WSAID_ACCEPTEX;
//     if(WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
//              &guidAcceptEx, sizeof(guidAcceptEx),
//              &__g_fnAccept, sizeof(__g_fnAccept),
//              &dwBytes, NULL, NULL)!=0)
//     {
//         auto error = WSAGetLastError();
//         printf("load acceptex fail error[%d]", error);
//     }
// }
// 
// void PostAccept(SOCKET sockServer, IO_DATA_BASE *ioData)
// {
//     ioData->_ioType = IO_Defs::IO_ACCEPT;
//     ioData->_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//     if(!__g_fnAccept(sockServer
//                  , ioData->_sock
//                  , ioData->_buff
//                  , 0
//                  , sizeof(sockaddr_in) + 16
//                  , sizeof(sockaddr_in) + 16
//                  , NULL
//                  , &ioData->_overlapped)) // �������Զ���Ľṹ��
//     {
//         auto error = WSAGetLastError();
//         if(error != ERROR_IO_PENDING)
//         {
//             printf("AcceptEx failed error[%d]", error);
//             return;
//         }
//     }
// }
// 
// // Ͷ�ݽ�������
// bool PostRecv(IO_DATA_BASE *ioData)
// {
//     ioData->_ioType = IO_Defs::IO_RECV;
//     WSABUF wsBuff = {};
//     wsBuff.buf = ioData->_buff;
//     wsBuff.len = sizeof(ioData->_buff);
//     DWORD flags = 0;
//     memset(&ioData->_overlapped, 0, sizeof(ioData->_overlapped));
//     if(SOCKET_ERROR == WSARecv(ioData->_sock, &wsBuff, 1, NULL, &flags, &ioData->_overlapped, NULL))
//     {
//         auto error = WSAGetLastError();
//         if(error != ERROR_IO_PENDING)
//         {
//             printf("WSARecv failed error[%d]", error);
//             return false;
//         }
//     }
// 
//     return true;
// }
// 
// bool PostSend(IO_DATA_BASE *ioData)
// {
//     ioData->_ioType = IO_Defs::IO_SEND;
//     WSABUF wsBuff = {};
//     wsBuff.buf = ioData->_buff;
//     wsBuff.len = ioData->_length;
//     DWORD flags = 0;
//     memset(&ioData->_overlapped, 0, sizeof(ioData->_overlapped));
//     if(SOCKET_ERROR == WSASend(ioData->_sock, &wsBuff, 1, NULL, flags, &ioData->_overlapped, NULL))
//     {
//         auto error = WSAGetLastError();
//         if(error != ERROR_IO_PENDING)
//         {
//             printf("WSASend failed error[%d]", error);
//             return false;
//         }
//     }
// 
//     return true;
// }

int Example::Run()
{
    //g_Log->InitModule("IOCP_TEST");
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
    FS_Iocp iocp;
    iocp.Create();
//     auto _completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
//     if(!_completionPort)
//     {
//         auto err = GetLastError();
//         printf("CreateIoCompletionPort failed error code<%d>", err);
//         return err;
//     }

    // ����IOCP �� ServerSocket
    // completionKey�����һ����ֵ�����ʱ��ԭ��������; NumberOfConcurrentThreads��������ڹ�����ɶ˿�ʱ������
    iocp.Reg(sockServer);
//     auto ret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(sockServer), _completionPort, (ULONG_PTR)(sockServer), 0); // completekey�������Զ���Ľṹ��ָ������������ݵ�ָ�룬���ڻ�ȡ���״̬ʱ��ʶ��
//     if(!ret)
//     {
//         auto err = GetLastError();
//         printf("CreateIoCompletionPort associated failure error code<%d>", err);
//         return err;
//     }

    // ��IOCPͶ�ݽ��ܿͻ������ӵ�����
    // sAcceptSocketԤ�ȴ�����socket������socket������ϵͳ��Դ��socket��Դ���޹���Ҫ����һ��socket�أ�������Դ��������
    // dwReceiveDataLength ��Ч�������ݳ��ȣ���Ϊ0��ʾ����ʱ���صȴ��ͻ��˷�������acceptex����ɣ�����ֵ��ʾ��Ҫ�ȴ��ͻ��˷������ݲ����
    // lpdwBytesReceived���ؽ������ݳ��ȣ�������ȴ��ͻ��˷������ݣ�����ط�������0
    iocp.LoadAcceptEx(sockServer);
    // LoadAcceptEx(sockServer);
    IO_DATA_BASE ioData[CLIENT_QUANTITY] = {};
    for(Int32 i = 0; i < CLIENT_QUANTITY; ++i)
        iocp.PostAccept(sockServer, &ioData[i]);

    Int32 msgCount = 0;
    IO_EVENT ioEvent = {};
    while(true)
    {
        // �ȴ�io���
        const Int32 st = iocp.WaitForCompletion(ioEvent);
        if(st != StatusDefs::Success)
        {
//             if(st== StatusDefs::IOCP_IODisconnect)
//                 g_Log->sys(_LOGFMT_("�ͻ��˶Ͽ����� sockfd=%llu"), ioEvent._ioData->_sock);
            continue;
        }

        // ����iocp�˳�
        if(ioEvent._data._code == IocpDefs::IO_QUIT)
        {
            g_Log->sys<Example>(_LOGFMT_("iocp�˳� code=%lld"), ioEvent._data._code);
            break;
        }

        // ����������
        if(ioEvent._ioData->_ioType == IocpDefs::IO_ACCEPT)
        {
            g_Log->sys<Example>(_LOGFMT_("�¿ͻ������� sockfd=%llu"), ioEvent._ioData->_sock);

            // �¿ͻ��˹�����ɶ˿�
            if(StatusDefs::Success != iocp.Reg(ioEvent._ioData->_sock))
                continue;

            // Ͷ�ݽ�������
            if(StatusDefs::Success != iocp.PostRecv(ioEvent._ioData))
            {
                g_Log->e<Example>(_LOGFMT_("post recv fail sock[%llu]"), ioEvent._ioData->_sock);
                closesocket(ioEvent._ioData->_sock);
                iocp.PostAccept(sockServer, ioEvent._ioData);
                continue;
            }

            // ����������һ������ ���ж���Ͷ�ݵ����Ӳ��ɴ�������
            // iocp.PostAccept(sockServer, )
        }
        else if(ioEvent._ioData->_ioType == IocpDefs::IO_RECV)
        {
            if(ioEvent._bytesTrans <= 0)
            {
                // �ͻ��˶Ͽ�
                // g_Log->e<Example>(_LOGFMT_("recv error socket[%llu], bytesTrans[%d]"), ioEvent._ioData->_sock, ioEvent._bytesTrans);
                g_Log->sys<Example>(_LOGFMT_("�ͻ��˶Ͽ����� sockfd=%llu bytestrans[%lu]"), ioEvent._ioData->_sock, ioEvent._bytesTrans);
                closesocket(ioEvent._ioData->_sock);
                iocp.PostAccept(sockServer, ioEvent._ioData);
                continue;
            }

            // ��ӡ���յ�������
            g_Log->sys<Example>(_LOGFMT_("recv data :socket[%llu], bytesTrans[%d] msgCount[%d]")
                       , ioEvent._ioData->_sock, ioEvent._bytesTrans, ++msgCount);

            // ��ͣ�Ľ�������
            ioEvent._ioData->_wsaBuff.len = ioEvent._bytesTrans;
            auto st = iocp.PostSend(ioEvent._ioData);
            if(st == StatusDefs::IOCP_ClientForciblyClosed)
            {// Զ�˹ر�
                closesocket(ioEvent._ioData->_sock);
                iocp.PostAccept(sockServer, ioEvent._ioData);
            }
        }
        else if(ioEvent._ioData->_ioType == IocpDefs::IO_SEND)
        {
            // �ͻ��˶Ͽ�����
            if(ioEvent._bytesTrans <= 0)
            {
                g_Log->sys<Example>(_LOGFMT_("�ͻ��˶Ͽ����� sockfd=%llu bytestrans[%lu]"), ioEvent._ioData->_sock, ioEvent._bytesTrans);
                closesocket(ioEvent._ioData->_sock);
                iocp.PostAccept(sockServer, ioEvent._ioData);
                continue;
            }

            // ��ӡ���͵�����
            g_Log->sys<Example>(_LOGFMT_("send data :socket[%llu], bytesTrans[%d] msgCount[%d]")
                       , ioEvent._ioData->_sock, ioEvent._bytesTrans, msgCount);

            // ioEvent._ioData->_wsaBuff.len = 
            auto st = iocp.PostRecv(ioEvent._ioData);
            if(st == StatusDefs::IOCP_ClientForciblyClosed)
            {// Զ�˹ر�
                closesocket(ioEvent._ioData->_sock);
                iocp.PostAccept(sockServer, ioEvent._ioData);
            }
        }
        else
        {
            g_Log->e<Example>(_LOGFMT_("δ������Ϊ sockefd=%llu"), ioEvent._data._socket);
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
    iocp.Destroy();
    // CloseHandle(_completionPort);

    // ���windows socket����
    WSACleanup();
    return 0;
}

FS_NAMESPACE_END
