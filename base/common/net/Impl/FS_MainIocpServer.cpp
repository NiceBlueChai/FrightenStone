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
 * @file  : FS_MainIocpServer.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/8
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/FS_MainIocpServer.h"
#include "base/common/net/Impl/FS_IocpMsgTransferServer.h"
#include "base/common/net/Impl/FS_Iocp.h"
#include "base/common/log/Log.h"
#include "base/common/socket/socket.h"
#include "base/common/status/status.h"
#include "base/common/net/Defs/IocpDefs.h"
#include "base/common/net/Impl/FS_Client.h"
#include "base/common/assist/assistobjs/Impl/SmartPtr.h"

FS_NAMESPACE_BEGIN

OBJ_POOL_CREATE_IMPL(FS_MainIocpServer, _objPoolHelper, __DEF_OBJ_POOL_OBJ_NUM__)

FS_MainIocpServer::FS_MainIocpServer()
    :_closeIocpDelegate(NULL)
{
}

FS_MainIocpServer::~FS_MainIocpServer()
{
    Fs_SafeFree(_closeIocpDelegate);
}

#pragma region start close
void FS_MainIocpServer::Start(Int32 msgTransferServerCnt)
{
    FS_TcpServer::Start<FS_IocpMsgTransferServer>(msgTransferServerCnt);
}
#pragma endregion

void FS_MainIocpServer::_OnNetMonitorTask(const FS_ThreadPool *pool)
{
    const auto listenSock = _GetSocket();

    // �������󶨼����˿�
    SmartPtr<FS_Iocp> listenIocp = new FS_Iocp;
    listenIocp->Create();
    listenIocp->Reg(listenSock);
    listenIocp->LoadAcceptEx(listenSock);

    // ����ر�iocp
    auto __quitIocpFunc = [&listenIocp]()->void
    {
        listenIocp->PostQuit();
    };

    // ���˳�iocp
    _closeIocpDelegate = DelegatePlusFactory::Create<decltype(__quitIocpFunc), void>(__quitIocpFunc);

    // const int len = 2 * (sizeof(sockaddr_in) + 16);
    // ����Ҫ�ͻ��������Ӻ������������ݵ��������ͳ���len
    const int len = 1024;
    char buf[len] = {};

    IoDataBase ioData = {};
    ioData._wsaBuff.buf = buf;
    ioData._wsaBuff.len = len;

    listenIocp->PostAccept(listenSock, &ioData);
    IO_EVENT ioEvent = {};
    while(!pool->IsClearingPool())
    {
        // ͳ����Ϣ����
        _StatisticsMsgPerSecond();

        // ����iocp
        auto ret = listenIocp->WaitForCompletion(ioEvent, 1);
        if(ret == StatusDefs::IOCP_WaitTimeOut)
            continue;

        // ����
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_MainIocpServer>(_LOGFMT_("OnNetMonitorTask.WaitForCompletion error ret[%d]"), ret);
            break;
        }

        // ����iocp�˳�
        if(ioEvent._data._code == IocpDefs::IO_QUIT)
        {
            g_Log->sys<FS_MainIocpServer>(_LOGFMT_("iocp�˳� code=%lld"), ioEvent._data._code);
            break;
        }

        // �������� ���
        if(IocpDefs::IO_ACCEPT == ioEvent._ioData->_ioType)
        {
            // CELLLog_Info("�¿ͻ��˼��� sockfd=%d", ioEvent.pIoData->sockfd);
            _OnIocpAccept(ioEvent._ioData->_sock);

            // ������IOCPͶ�ݽ�����������
            listenIocp->PostAccept(listenSock, &ioData);
        }
    }

    _mainLocker.Lock();
    Fs_SafeFree(_closeIocpDelegate);
    _mainLocker.Unlock();
}

SOCKET FS_MainIocpServer::_OnIocpAccept(SOCKET sock)
{
    // 4 accept �ȴ����ܿͻ�������
    // sockaddr_in clientAddr = {};
    // int nAddrLen = sizeof(sockaddr_in);
    if(INVALID_SOCKET == sock)
    {
        g_Log->e<FS_MainIocpServer>(_LOGFMT_("_IocpAccept.sock invalid socket"));
    }
    else
    {
        _locker.Lock();
        UInt64 curMaxId = _clientMaxId;
        if(_clientAcceptCnt < _maxClient && curMaxId < _maxClientIdLimit)
        {
            curMaxId = ++_clientMaxId;
            _locker.Unlock();
            ++_clientAcceptCnt;
            SocketUtil::MakeReUseAddr(sock);

            // ���¿ͻ��˷�����ͻ��������ٵ�FS_Server
            _AddClientToFSServer(new FS_Client(curMaxId, sock, _sendBuffSize, _recvBuffSize));

            // ��ȡIP��ַ inet_ntoa(clientAddr.sin_addr)
        }
        else {
            _locker.Unlock();
            // ��ȡIP��ַ inet_ntoa(clientAddr.sin_addr)
            SocketUtil::DestroySocket(sock);
            g_Log->w<FS_MainIocpServer>(_LOGFMT_("Accept to MaxClient[%d] or curMaxId[%llu] too large"), _maxClient, curMaxId);
            g_Log->net<FS_MainIocpServer>("Accept to MaxClient[%d]or curMaxId[%llu] too large", _maxClient, curMaxId);
        }
    }

    return sock;
}

FS_NAMESPACE_END
