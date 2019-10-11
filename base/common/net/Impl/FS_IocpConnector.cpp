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
 * @file  : FS_IocpConnector.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/FS_IocpConnector.h"
#include "base/common/net/Impl/FS_ServerCore.h"
#include "base/common/net/Impl/FS_Iocp.h"

#include "base/common/status/status.h"
#include "base/common/log/Log.h"
#include "base/common/socket/socket.h"
#include "base/common/net/Impl/FS_Session.h"
#include "base/common/net/Impl/FS_SessionMgr.h"

FS_NAMESPACE_BEGIN
FS_IocpConnector::FS_IocpConnector()
    :_threadPool(NULL)
    ,_sock(INVALID_SOCKET)
    , _onConnected(NULL)
    ,_closeIocpDelegate(NULL)
    ,_clientAcceptCnt(0)
    , _maxClient(0)
    ,_clientMaxId(0)
    ,_maxClientIdLimit((std::numeric_limits<UInt64>::max)())
{

}

FS_IocpConnector::~FS_IocpConnector()
{
    Fs_SafeFree(_closeIocpDelegate);
    Fs_SafeFree(_onConnected);
    Fs_SafeFree(_threadPool);
}

Int32 FS_IocpConnector::BeforeStart()
{
    // TODO:读取配置初始化变量
    _threadPool = new FS_ThreadPool(0, 1);
    return StatusDefs::Success;
}

Int32 FS_IocpConnector::Start()
{
    auto monitorTask = DelegatePlusFactory::Create(this, &FS_IocpConnector::_OnIocpMonitorTask);
    if(!_threadPool->AddTask(monitorTask, true))
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("add monitor task fail"));
        return StatusDefs::IocpConnector_StartFailOfMonitorTaskFailure;
    }

    return StatusDefs::Success;
}

void FS_IocpConnector::BeforeClose()
{
    _closeIocpDelegate->Invoke();
}

void FS_IocpConnector::Close()
{
}

void FS_IocpConnector::RegisterConnected(IDelegate<void, FS_Session *> *callback)
{
    Fs_SafeFree(_onConnected);
    _onConnected = callback;
}

SOCKET FS_IocpConnector::_InitSocket()
{
    if(INVALID_SOCKET != _sock)
    {
        g_Log->w<FS_IocpConnector>(_LOGFMT_("initSocket has already init socket<%d>..."), static_cast<Int32>(_sock));
        return _sock;
    }

    // ipv4 的流数据
    _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == _sock)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("create socket failed..."));
        return _sock;
    }

    // 套接字重用
    auto st = SocketUtil::MakeReUseAddr(_sock);
    if(st != StatusDefs::Success)
    {
        g_Log->w<FS_IocpConnector>(_LOGFMT_("make reuse of socket failed sock[%llu]"), _sock);
    }

    g_Log->net<FS_IocpConnector>("create socket<%llu> success...", _sock);
    return _sock;
}

Int32 FS_IocpConnector::_Bind(const Byte8 *ip, UInt16 port)
{
    // TODO:读取配置表
    // 1.bind 绑定用于接受客户端连接的网络端口
    sockaddr_in sin = {};
    sin.sin_family = AF_INET;
    // host to net unsigned short
    sin.sin_port = htons(port);

#ifdef _WIN32
    if(ip) {
        inet_pton(sin.sin_family, ip, &(sin.sin_addr));// 比较新的函数对比inet_addr
    }
    else {
        sin.sin_addr.S_un.S_addr = INADDR_ANY;
    }
#else
    if(ip) {
        sin.sin_addr.s_addr = inet_addr(ip);
    }
    else {
        sin.sin_addr.s_addr = INADDR_ANY;
    }
#endif

    int ret = bind(_sock, (sockaddr *)&sin, sizeof(sin));
    if(SOCKET_ERROR == ret)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("bind port<%hu> failed..."), port);
        return StatusDefs::Socket_Error;
    }

    g_Log->net<FS_IocpConnector>("bind port<%hu> success...", port);
    return StatusDefs::Success;
}

Int32 FS_IocpConnector::_Listen(Int32 unconnectQueueLen)
{
    // 1.listen 监听网络端口
    int ret = listen(_sock, unconnectQueueLen);
    if(SOCKET_ERROR == ret)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("listen socket<%llu> failed..."), _sock);
        return StatusDefs::Socket_Error;
    }

    g_Log->net<FS_IocpConnector>("listen port socket<%llu> success...", _sock);
    return StatusDefs::Success;
}

void FS_IocpConnector::_OnIocpAccept(SOCKET sock)
{
    // 1. accept 等待接受客户端连接
 // sockaddr_in clientAddr = {};
 // int nAddrLen = sizeof(sockaddr_in);
    if(INVALID_SOCKET == sock)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("_IocpAccept.sock invalid socket"));
        return;
    }

    if(_clientAcceptCnt < _maxClient && _clientMaxId < _maxClientIdLimit)
    {
        ++_clientMaxId;
        ++_clientAcceptCnt;
        SocketUtil::MakeReUseAddr(sock);

        // TODO:连接回调
        FS_Session *newSession = NULL;
        _onConnected->Invoke(newSession);
        // 获取IP地址 inet_ntoa(clientAddr.sin_addr)
    }
    else {
        // 获取IP地址 inet_ntoa(clientAddr.sin_addr)
        SocketUtil::DestroySocket(sock);
        g_Log->w<FS_IocpConnector>(_LOGFMT_("Accept to MaxClient[%d] or curMaxId[%llu] too large"), _maxClient, _clientMaxId);
    }
}

void FS_IocpConnector::_OnIocpMonitorTask(const FS_ThreadPool *threadPool)
{
    // 创建并绑定监听端口
    SmartPtr<FS_Iocp> listenIocp = new FS_Iocp;
    listenIocp->Create();
    listenIocp->Reg(_sock);
    listenIocp->LoadAcceptEx(_sock);

    // 定义关闭iocp
    auto __quitIocpFunc = [&listenIocp]()->void {
        listenIocp->PostQuit();
    };

    // 绑定退出iocp
    _closeIocpDelegate = DelegatePlusFactory::Create<decltype(__quitIocpFunc), void>(__quitIocpFunc);

    // const int len = 2 * (sizeof(sockaddr_in) + 16);
    // 不需要客户端再连接后立即发送数据的情况下最低长度len
    const int len = 1024;
    char buf[len] = {};

    IO_DATA_BASE ioData = {};
    ioData._wsaBuff.buf = buf;
    ioData._wsaBuff.len = len;

    listenIocp->PostAccept(_sock, &ioData);
    IO_EVENT ioEvent = {};
    while(!threadPool->IsClearingPool())
    {
        // 监听iocp
        auto ret = listenIocp->WaitForCompletion(ioEvent, 1);
        if(ret == StatusDefs::IOCP_WaitTimeOut)
            continue;

        // 出错
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_IocpConnector>(_LOGFMT_("_OnIocpMonitorTask.WaitForCompletion error ret[%d]"), ret);
            break;
        }

        // 处理iocp退出
        if(ioEvent._data._code == IocpDefs::IO_QUIT)
        {
            g_Log->sys<FS_IocpConnector>(_LOGFMT_("iocp退出 code=%lld"), ioEvent._data._code);
            break;
        }

        // 接受连接 完成
        if(IocpDefs::IO_ACCEPT == ioEvent._ioData->_ioType)
        {
            // CELLLog_Info("新客户端加入 sockfd=%d", ioEvent.pIoData->sockfd);
            _OnIocpAccept(ioEvent._ioData->_sock);

            // 继续向IOCP投递接受连接任务
            listenIocp->PostAccept(_sock, &ioData);
        }
    }
}

FS_NAMESPACE_END
