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
#include "base/common/net/Impl/FS_SessionFactory.h"
#include "base/common/net/Impl/FS_Addr.h"
#include "base/common/net/Defs/IocpDefs.h"

#include "base/common/status/status.h"
#include "base/common/log/Log.h"
#include "base/common/socket/socket.h"
#include "base/common/net/Impl/IFS_Session.h"
#include "base/common/net/Impl/FS_SessionMgr.h"
#include "base/common/assist/utils/Impl/SystemUtil.h"
#include "base/common/component/Impl/File/FS_IniFile.h"

FS_NAMESPACE_BEGIN
FS_IocpConnector::FS_IocpConnector()
    :_threadPool(NULL)
    ,_sock(INVALID_SOCKET)
    , _onConnected(NULL)
    ,_closeIocpDelegate(NULL)
    ,_curSessionCnt(0)
    , _maxSessionQuantityLimit(0)
    ,_curMaxSessionId(0)
    ,_maxSessionIdLimit((std::numeric_limits<UInt64>::max)())
    ,_linkConfig(NULL)
{
    // TODO:��ȡ����
     _maxSessionQuantityLimit = FD_SETSIZE;
}

FS_IocpConnector::~FS_IocpConnector()
{
    Fs_SafeFree(_closeIocpDelegate);
    Fs_SafeFree(_onConnected);
    Fs_SafeFree(_threadPool);
    Fs_SafeFree(_linkConfig);
}

Int32 FS_IocpConnector::BeforeStart()
{
    // TODO:��ȡ���ó�ʼ������
    auto st = _ReadConfig();
    if(st != StatusDefs::Success)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("_ReadConfig fail st[%d]"), st);
        return st;
    }

    _threadPool = new FS_ThreadPool(0, 1);

    // ��ʼ��
    auto sock = _InitSocket();
    if(sock == INVALID_SOCKET)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("init listen socket fail"));
        return StatusDefs::IocpConnector_InitListenSocketFail;
    }

    BUFFER256 buffer = {};
    char *ptr = buffer;
    _linkConfig->ReadStr(_segmentName.c_str(), _ipKey.c_str(), "", ptr, sizeof(buffer));
    UInt16 port = static_cast<UInt32>(_linkConfig->ReadInt(_segmentName.c_str(), _portKey.c_str(), 0));

    st = _Bind(buffer, port);
    if(st != StatusDefs::Success)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("listen sock[%llu] bind ip[%s:%hu] fail st[%d]")
                                   , _sock, buffer, port, st);
        return st;
    }

    st = _Listen();
    if(st != StatusDefs::Success)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("listen sock[%llu] listen ip[%s:%hu] fail st[%d]")
                                   , _sock, buffer, port, st);
        return st;
    }

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
    // ��������Ҫ�ٴ�ʱ�ر����
    _closeIocpDelegate->Invoke();
    _threadPool->Clear();
}

void FS_IocpConnector::Close()
{
}

void FS_IocpConnector::RegisterConnected(IDelegate<void, IFS_Session *> *callback)
{
    Fs_SafeFree(_onConnected);
    _onConnected = callback;
}

void FS_IocpConnector::OnDisconnected(IFS_Session *session)
{
    _locker.Lock();
    --_curSessionCnt;
    _locker.Unlock();
}

Int32 FS_IocpConnector::_ReadConfig()
{
    _segmentName = "Listener";
    _listenerFileName = "./ServerCfg.ini";
    _ipKey = "ip";
    _portKey = "port";

    if(!_linkConfig)
    {
        _linkConfig = new FS_IniFile();
        if(!_linkConfig->SetPath(_listenerFileName.c_str()))
        {
            g_Log->e<FS_IocpConnector>(_LOGFMT_("create cfg ini fail"));
            return StatusDefs::IocpConnector_CreateCfgIniFail;
        }

        // ��ʼ�������ļ�
        auto st = _InitDefCfgs();
        if(st != StatusDefs::Success)
        {
            g_Log->e<FS_IocpConnector>(_LOGFMT_("_InitDefCfgs fail st[%d]"), st);
            return st;
        }
    }

    return StatusDefs::Success;
}

Int32 FS_IocpConnector::_InitDefCfgs()
{
    _linkConfig->WriteStr(_segmentName.c_str(), _ipKey.c_str(), "127.0.0.1");
    _linkConfig->WriteStr(_segmentName.c_str(), _portKey.c_str(), "4567");

    // ����Ƿ�д����ȷ
    BUFFER256 buffer = {};
    char *ptr = buffer;
    _linkConfig->ReadStr(_segmentName.c_str(), _ipKey.c_str(), "", ptr, sizeof(buffer));
    if(strcmp(buffer, "127.0.0.1") != 0)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("_InitDefCfgs fail ip not match"));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 port = _linkConfig->ReadInt(_segmentName.c_str(), _portKey.c_str(), 0);
    if(port != 4567)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("_InitDefCfgs fail port not match"));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    return StatusDefs::Success;
}

SOCKET FS_IocpConnector::_InitSocket()
{
    if(INVALID_SOCKET != _sock)
    {
        g_Log->w<FS_IocpConnector>(_LOGFMT_("initSocket has already init socket<%d>..."), static_cast<Int32>(_sock));
        return _sock;
    }

    // ipv4 ��������
    _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == _sock)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("create socket failed..."));
        return _sock;
    }

    // �׽�������
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
    // TODO:��ȡ���ñ�
    // 1.bind �����ڽ��ܿͻ������ӵ�����˿�
    sockaddr_in sin = {};
    sin.sin_family = AF_INET;
    // host to net unsigned short
    sin.sin_port = htons(port);

#ifdef _WIN32
    if(ip) {
        inet_pton(sin.sin_family, ip, &(sin.sin_addr));// �Ƚ��µĺ����Ա�inet_addr
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
    // 1.listen ��������˿�
    int ret = listen(_sock, unconnectQueueLen);
    if(SOCKET_ERROR == ret)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("listen socket<%llu> failed..."), _sock);
        return StatusDefs::Socket_Error;
    }

    g_Log->net<FS_IocpConnector>("listen port socket<%llu> success...", _sock);
    return StatusDefs::Success;
}

void FS_IocpConnector::_OnConnected(SOCKET sock, const sockaddr_in *addrInfo)
{
    if(INVALID_SOCKET == sock)
    {
        g_Log->e<FS_IocpConnector>(_LOGFMT_("_IocpAccept.sock invalid socket"));
        return;
    }

    _locker.Lock();
    if(_curSessionCnt < _maxSessionQuantityLimit && _curMaxSessionId < _maxSessionIdLimit)
    {
        ++_curSessionCnt;
        _locker.Unlock();

        ++_curMaxSessionId;
        SocketUtil::MakeReUseAddr(sock);

        // TODO:���ӻص� 
        IFS_Session *newSession = FS_SessionFactory::Create(_curMaxSessionId, sock, addrInfo);
        newSession->OnConnect();
        _onConnected->Invoke(newSession);

        auto sessionAddr = newSession->GetAddr();
        g_Log->net<FS_IocpConnector>("new session connected: id<%llu>,socket<%llu>,remote ip[%s:%hu]"
                                     , newSession->GetSessionId()
                                     , newSession->GetSocket()
                                     , sessionAddr->GetAddr().c_str()
                                     , sessionAddr->GetPort());
        g_Log->any<FS_IocpConnector>("new session connected: id<%llu>,socket<%llu>,remote ip[%s:%hu]"
                                     , newSession->GetSessionId()
                                     , newSession->GetSocket()
                                     , sessionAddr->GetAddr().c_str()
                                     , sessionAddr->GetPort());

        // Ͷ�ݽ�������
        
        // ��ȡIP��ַ inet_ntoa(clientAddr.sin_addr)
    }
    else {
        _locker.Unlock();

        // ��ȡIP��ַ inet_ntoa(clientAddr.sin_addr)
        SocketUtil::DestroySocket(sock);
        g_Log->w<FS_IocpConnector>(_LOGFMT_("Accept to MaxClient[%d] or curMaxId[%llu] too large"), _maxSessionQuantityLimit, _curMaxSessionId);
    }
}

void FS_IocpConnector::_OnDisconnected(IFS_Session *session)
{
    _locker.Lock();
    --_curSessionCnt;
    _locker.Unlock();
}

void FS_IocpConnector::_OnIocpMonitorTask(const FS_ThreadPool *threadPool)
{
    // 1.�������󶨼����˿�
    SmartPtr<FS_Iocp> listenIocp = new FS_Iocp;
    listenIocp->Create();
    listenIocp->Reg(_sock);
    listenIocp->LoadAcceptEx(_sock);

    // 2.����ر�iocp
    auto __quitIocpFunc = [this, &listenIocp]()->void {
        // �ȹر�listensocket
        SocketUtil::DestroySocket(_sock);
        listenIocp->PostQuit();
    };

    // 3.���˳�iocp
    _closeIocpDelegate = DelegatePlusFactory::Create<decltype(__quitIocpFunc), void>(__quitIocpFunc);

    // const int len = 2 * (sizeof(sockaddr_in) + 16);
    // ����Ҫ�ͻ��������Ӻ������������ݵ��������ͳ���len

    // 4.Ԥ��Ͷ��FD_SETSIZE��accept ��������
    char **bufArray = NULL;
    IoDataBase **ioDataArray = NULL;
    _PreparePostAccept(listenIocp, bufArray, ioDataArray);

    // 5.������������
    IO_EVENT ioEvent = {};
    while(!threadPool->IsClearingPool())
    {
        // ����iocp
        auto ret = listenIocp->WaitForCompletion(ioEvent);
        if(ret == StatusDefs::IOCP_WaitTimeOut)
            continue;

        // ����
        if(ret != StatusDefs::Success)
        {
            g_Log->e<FS_IocpConnector>(_LOGFMT_("_OnIocpMonitorTask.WaitForCompletion error ret[%d]"), ret);
            break;
        }

        // ����iocp�˳�
        if(ioEvent._data._code == IocpDefs::IO_QUIT)
        {
            g_Log->sys<FS_IocpConnector>(_LOGFMT_("connector iocp�˳� threadId<%lu> code=%lld")
                                         , SystemUtil::GetCurrentThreadId(), ioEvent._data._code);
            break;
        }

        // �������� ���
        if(IocpDefs::IO_ACCEPT == ioEvent._ioData->_ioType)
        {
            // TODO:��getacceptAddrInfoʱ����Ҫ�����߳��Ƿ�ȫ
            sockaddr_in *clientAddrInfo = NULL;
            listenIocp->GetClientAddrInfo(ioEvent._ioData->_wsaBuff.buf, clientAddrInfo);
            _OnConnected(ioEvent._ioData->_sock, clientAddrInfo);

            // ������IOCPͶ�ݽ�����������
            const auto st = listenIocp->PostAccept(_sock, ioEvent._ioData);
            if(st != StatusDefs::Success)
            {
                g_Log->e<FS_IocpConnector>(_LOGFMT_("post accept fail whnen new session create listen sock[%llu], iodata addr[%p] st[%d]")
                                           , _sock, ioEvent._ioData, st);
            }
        }
    }

    listenIocp->Destroy();
    _FreePrepareAcceptBuffers(bufArray, ioDataArray);
}

void FS_IocpConnector::_PreparePostAccept(FS_Iocp *listenIocp, char **&bufArray, IoDataBase **&ioDataArray)
{
    // Ԥ�ȴ���n������������ӹ���
    g_MemoryPool->Lock();
    bufArray = g_MemoryPool->Alloc<char *>(FD_SETSIZE * sizeof(char *));
    g_MemoryPool->Unlock();

    for(Int32 i = 0; i < FD_SETSIZE; ++i)
    {
        g_MemoryPool->Lock();
        bufArray[i] = g_MemoryPool->Alloc<char>(IOCP_CONNECTOR_BUFFER);
        g_MemoryPool->Unlock();
    }

    // Ԥ�ȴ���n��iodata
    g_MemoryPool->Lock();
    ioDataArray = g_MemoryPool->Alloc<IoDataBase *>(sizeof(IoDataBase *)*FD_SETSIZE);
    g_MemoryPool->Unlock();

    Int32 st = StatusDefs::Success;
    for(Int32 i = 0; i < FD_SETSIZE; ++i)
    {
        g_MemoryPool->Lock();
        ioDataArray[i] = g_MemoryPool->Alloc<IoDataBase>(sizeof(IoDataBase));
        g_MemoryPool->Unlock();

        ioDataArray[i]->_wsaBuff.buf = bufArray[i];
        ioDataArray[i]->_wsaBuff.len = IOCP_CONNECTOR_BUFFER;

        st = listenIocp->PostAccept(_sock, ioDataArray[i]);
        if(st != StatusDefs::Success)
        {
            g_Log->e<FS_IocpConnector>(_LOGFMT_("prepare post accept fail sock[%llu] st[%d]")
                                       , _sock, st);
        }
    }

    // TODO:���������ٶȣ�һ����Ͷ�ݱ��������ӵ�������������õ�һ���ٲ���һ��
}

void FS_IocpConnector::_FreePrepareAcceptBuffers(char **&bufArray, IoDataBase **&ioDataArray)
{
    g_MemoryPool->Lock();
    for(Int32 i = 0; i < FD_SETSIZE; ++i)
    {
        g_MemoryPool->Free(bufArray[i]);
        g_MemoryPool->Free(ioDataArray[i]);
    }
    g_MemoryPool->Free(bufArray);
    g_MemoryPool->Free(ioDataArray);
    g_MemoryPool->Unlock();
}

FS_NAMESPACE_END


