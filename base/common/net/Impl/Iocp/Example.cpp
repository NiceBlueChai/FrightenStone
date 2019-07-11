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

FS_NAMESPACE_BEGIN

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
    auto ret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(sockServer), _completionPort, (ULONG_PTR)(sockServer), 0);
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
    auto acceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[1024] = {};
//    OVERLAPPED overLapped;
//     if(!AcceptEx(sockServer
//                  , acceptSocket
//                  , buffer
//                  , 0
//                  , sizeof(sockaddr_in) + 16
//                  , sizeof(sockaddr_in) + 16
//                  , NULL
//                  , &overLapped))
//     {
//         auto error = WSAGetLastError();
//         if(error != ERROR_IO_PENDING)
//         {
//             printf("AcceptEx failed error[%d]", error);
//         }
//     }
    while(true)
    {
        // ��ȡ��ɶ˿�״̬
        // ����Ƿ����¼���������selet,epoll_wait����
        // �������� ���
        // �������� ��� completion
        // �������� ���
        // ��IOCP Ͷ�ݽ�����������
    }

    // ------------ IOCP end ------------ //
    // �ر�clientsocket
    // �ر�serversocket
    closesocket(sockServer);
    // �ر���ɶ˿�
    CloseHandle(_completionPort);

    // ���windows socket����
    WSACleanup();
    return 0;
}

FS_NAMESPACE_END
