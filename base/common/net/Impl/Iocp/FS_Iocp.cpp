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
 * @file  : FS_Iocp.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/18
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/Iocp/FS_Iocp.h"
#include "base/common/log/Log.h"
#include "base/common/status/status.h"
#include "base/common/net/Defs/IocpDefs.h"

#pragma region windows api
#include <windows.h>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
// #pragma comment (lib, "MSWSock.lib")
#pragma endregion

FS_NAMESPACE_BEGIN
FS_Iocp::FS_Iocp()
{

}

FS_Iocp::~FS_Iocp()
{

}

Int32 FS_Iocp::Create()
{
    if(_completionPort)
        return StatusDefs::Success;

    // ������ɶ˿�IOCP NumberOfConcurrentThreads=0��ʾĬ��cpu����
    _completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if(!_completionPort)
    {
        auto err = GetLastError();
        g_Log->e<FS_Iocp>(_LOGFMT_("Create failed windows error<%d> status[%d]")
                          , err,  StatusDefs::IOCP_CreateCompletionPortFail);
        return StatusDefs::IOCP_CreateCompletionPortFail;
    }

    return StatusDefs::Success;
}

void FS_Iocp::Destroy()
{
    // �ر���ɶ˿�
    if(LIKELY(_completionPort))
        CloseHandle(_completionPort);
    _completionPort = NULL;
}

Int32 FS_Iocp::Reg(SOCKET sockfd)
{
    // ����IOCP �� sockfd
    // completionKey�����һ����ֵ�����ʱ��ԭ��������; NumberOfConcurrentThreads��������ڹ�����ɶ˿�ʱ������
    // completekey�������Զ���Ľṹ��ָ������������ݵ�ָ�룬���ڻ�ȡ���״̬ʱ��ʶ�� �����ڹ���ʱnumofthread�ᱻ����
    HANDLE ret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(sockfd), _completionPort, ULONG_PTR(sockfd), 0); 
    if(!ret)
    {
        auto err = GetLastError();
        g_Log->e<FS_Iocp>(_LOGFMT_("Reg sockfd[%llu] to completionport failed windows error<%d> status[%d]")
                          ,sockfd, err, StatusDefs::IOCP_RegSocketToCompletionPortFail);
        return StatusDefs::IOCP_RegSocketToCompletionPortFail;
    }

    return StatusDefs::Success;
}

Int32 FS_Iocp::LoadAcceptEx(SOCKET listenSocket)
{
    if(_fnAcceptEx)
        return StatusDefs::Success;

    DWORD dwBytes = 0;
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    if(WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                &guidAcceptEx, sizeof(guidAcceptEx),
                &_fnAcceptEx, sizeof(_fnAcceptEx),
                &dwBytes, NULL, NULL) != 0)
    {
        auto error = WSAGetLastError();
        g_Log->e<FS_Iocp>(_LOGFMT_("load acceptex fail windows error[%d] statuscode[%d]")
                          , error, StatusDefs::IOCP_LoadAcceptExError);
        return StatusDefs::IOCP_LoadAcceptExError;
    }

    return StatusDefs::Success;
}

Int32 FS_Iocp::PostAccept(SOCKET listenSocket, IO_DATA_BASE *ioData)
{
    ioData->_ioType = IocpDefs::IO_ACCEPT;
    ioData->_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(!_fnAcceptEx(listenSocket
                     , ioData->_sock
                     , ioData->_buff
                     , 0
                     , sizeof(sockaddr_in) + 16     // msdnָ������
                     , sizeof(sockaddr_in) + 16     // msdnָ������
                     , NULL
                     , &ioData->_overlapped)) // �ص���ĵ�ַ�������Զ���ṹ��ĳ�ʼλ�ã��������Զ���ṹ��ĵ�һ����Ա�������Զ���Ľṹ��
    {
        auto error = WSAGetLastError();
        if(error != ERROR_IO_PENDING)
        {
            g_Log->e<FS_Iocp>(_LOGFMT_("PostAccept listenSocket[%llu] ioSocket[%llu] to completionport failed windows error<%d> status[%d]")
                              , listenSocket,ioData->_sock, error, StatusDefs::IOCP_PostAcceptFail);
            return StatusDefs::IOCP_PostAcceptFail;
        }
    }

    return StatusDefs::Success;
}

Int32 FS_Iocp::PostRecv(IO_DATA_BASE *ioData)
{
    ioData->_ioType = IocpDefs::IO_RECV;
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
            g_Log->e<FS_Iocp>(_LOGFMT_("PostRecv ioSocket[%llu] to completionport failed windows error<%d> status[%d]")
                              , ioData->_sock, error, StatusDefs::IOCP_PostRecvFail);
            return StatusDefs::IOCP_PostRecvFail;
        }
    }

    return StatusDefs::Success;
}

Int32 FS_Iocp::PostSend(IO_DATA_BASE *ioData)
{
    ioData->_ioType = IocpDefs::IO_SEND;
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
            g_Log->e<FS_Iocp>(_LOGFMT_("PostSend ioSocket[%llu] to completionport failed windows error<%d> status[%d]")
                              , ioData->_sock, error, StatusDefs::IOCP_PostSendFail);
            return StatusDefs::IOCP_PostSendFail;
        }
    }

    return StatusDefs::Success;
}

Int32 FS_Iocp::WaitForCompletion(IO_EVENT &ioEvent, ULong millisec)
{
    // ��ȡ��ɶ˿�״̬
    // �ؼ����� completekey(����iocp�˿�ʱ������Զ�����ɼ�)
    // �Լ��ص��ṹioDataPtr ���ڻ�ȡ����
    ioEvent._bytesTrans = 0;
    ioEvent._ioData = NULL;
    ioEvent._socket = INVALID_SOCKET;
    if(FALSE == GetQueuedCompletionStatus(_completionPort
                                          , &ioEvent._bytesTrans
                                          , reinterpret_cast<PULONG_PTR>(&ioEvent._socket)
                                          , reinterpret_cast<LPOVERLAPPED *>(&ioEvent._ioData)
                                          , millisec))
    {
        const Int32 error = GetLastError();
        if(WAIT_TIMEOUT == error)
        {
            g_Log->net("WaitForMessage time out error<%d> status[%d]"
                       , error, StatusDefs::IOCP_WaitTimeOut);
            return StatusDefs::IOCP_WaitTimeOut;
        }

        if(ERROR_NETNAME_DELETED == error)
        {
            g_Log->net("WaitForMessage client closed sockfd=%llu\n error<%d> status[%d]"
                       , ioEvent._ioData->_sock, error, StatusDefs::IOCP_IODisconnect);
            // ��ʱioevent�����ݱ���ȷ����䣬ֻ��ioEvent._bytesTrans<=0����¼�������recv�¼�������
            // closesocket(ioEvent._ioData->_sock);
            // return StatusDefs::IOCP_IODisconnect;
            return StatusDefs::Success;
        }

        g_Log->e<FS_Iocp>(_LOGFMT_("WaitForMessage other error error<%d> status[%d]")
                          , error, StatusDefs::IOCP_PostSendFail);
        return StatusDefs::IOCP_WaitOtherError;
    }

    return StatusDefs::Success;
}


FS_NAMESPACE_END

