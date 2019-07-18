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
FS_Iocp::FS_Iocp(SOCKET listenSocket)
    :_listenSocket(listenSocket)
{

}

FS_Iocp::~FS_Iocp()
{

}

Int32 FS_Iocp::Create()
{
    if(_completionPort)
        return StatusDefs::Success;

    // 创建完成端口IOCP NumberOfConcurrentThreads=0表示默认cpu核数
    auto _completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
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
    // 关闭监听socket
    if(_listenSocket != INVALID_SOCKET)
        closesocket(_listenSocket);
    _listenSocket = INVALID_SOCKET;

    // 关闭完成端口
    if(LIKELY(_completionPort))
        CloseHandle(_completionPort);
    _completionPort = NULL;
}

Int32 FS_Iocp::Reg(SOCKET sockfd)
{
    // 关联IOCP 与 sockfd
    // completionKey传入的一个数值，完成时会原样传回来; NumberOfConcurrentThreads这个参数在关联完成端口时被忽略
    // completekey可以是自定义的结构体指针或者其他数据的指针，便于获取完成状态时候识别 当处于关联时numofthread会被忽略
    HANDLE ret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(sockfd), _completionPort, (ULONG_PTR)(sockfd), 0); 
    if(!ret)
    {
        auto err = GetLastError();
        g_Log->e<FS_Iocp>(_LOGFMT_("Reg sockfd[%llu] to completionport failed windows error<%d> status[%d]")
                          ,sockfd, err, StatusDefs::IOCP_RegSocketToCompletionPortFail);
        return StatusDefs::IOCP_RegSocketToCompletionPortFail;
    }

    return StatusDefs::Success;
}

Int32 FS_Iocp::LoadAcceptEx()
{
    if(_listenSocket == INVALID_SOCKET)
    {
        g_Log->e<FS_Iocp>(_LOGFMT_("load acceptex fail: listen socket is invalid statuscode[%d]")
                          , StatusDefs::IOCP_LoadAcceptExFailForListenSocketIsInvalid);
        return StatusDefs::IOCP_LoadAcceptExFailForListenSocketIsInvalid;
    }

    if(_fnAcceptEx)
        return StatusDefs::Success;

    DWORD dwBytes = 0;
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    if(WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
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
                     , sizeof(sockaddr_in) + 16     // msdn指定参数
                     , sizeof(sockaddr_in) + 16     // msdn指定参数
                     , NULL
                     , &ioData->_overlapped)) // 重叠体的地址必须是自定义结构体的初始位置，必须是自定义结构体的第一个成员可以是自定义的结构体
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

FS_NAMESPACE_END


