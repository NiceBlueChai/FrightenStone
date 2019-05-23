/**
* @file TcpServer.cpp
* @author Huiya Song <120453674@qq.com>
* @date 2019/04/18
* @brief
*/
#include "pch.h"
#include <TcpPort/Server/Impl/TcpServer.h>
#include <TcpPort/Server/Impl/ClientSocket.h>

#pragma region windows����֧��
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifdef _WIN32
// #include<windows.h>
 #include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif // _WIN32
#pragma endregion

TcpServer::TcpServer()
{

}

TcpServer::~TcpServer()
{
    Close();
}

Int32 TcpServer::InitSocket(bool blockSocket)
{
    // 1.�����׽���
    if(_socket != MYINVALID_SOCKET)
        return StatusDefs::Repeat;

    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(_socket == MYINVALID_SOCKET)
    {
        printf("���󣬽���Socketʧ��...\n");
        return StatusDefs::Socket_CreateFailure;
    }

    // ��������ģʽ
    if(blockSocket)
        fs::SocketUtil::SetBlock(_socket);
    else
        fs::SocketUtil::SetNoBlock(_socket);

    printf("����Socket=<%llu>�ɹ�...\n", _socket);

    return StatusDefs::Success;
}

Int32 TcpServer::Bind(const char* ip, unsigned short port) const
{
    // 1. bind �����ڽ��ܿͻ������ӵ�����˿�
    sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);    //host to net unsigned short

    // 2. ip
#ifdef _WIN32
    if(ip) {
        sin.sin_addr.S_un.S_addr = inet_addr(ip);
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

    // 3.��
    auto ret = bind(_socket, reinterpret_cast<sockaddr *>(&sin), sizeof(sin));
    if(SOCKET_ERROR == ret)
    {
        printf("����,������˿�<%d>ʧ��...\n", port);
    }
    else {
        printf("������˿�<%d>�ɹ�...\n", port);
    }

    return ret;
}

Int32 TcpServer::Listen(Int32 backLog) const
{
    // 3 listen ��������˿�
    auto ret = listen(_socket, backLog);
    if(SOCKET_ERROR == ret)
    {
        printf("socket=<%llu>����,��������˿�ʧ��...\n", _socket);
    }
    else {
        printf("socket=<%llu>��������˿ڳɹ�...\n", _socket);
    }

    return ret;
}

MYSOCKET TcpServer::Accept()
{
    // 1. accept �ȴ����ܿͻ�������
    sockaddr_in clientAddr = {0};
    int addrLen = sizeof(sockaddr_in);
    MYSOCKET socket = MYINVALID_SOCKET;

    // 2.accept��������
#ifdef _WIN32
    socket = accept(_socket, reinterpret_cast<sockaddr *>(&clientAddr), &addrLen);
#else
    socket = accept(_socket, reinterpret_cast<sockaddr *>(&clientAddr), (socklen_t *)&addrLen);
#endif

    if(MYINVALID_SOCKET == socket)
    {
        printf("socket=<%d>����,���ܵ���Ч�ͻ���SOCKET...\n", static_cast<Int32>(_socket));
    }
    else
    {
        CreatePlayerNty userJoin;
        SendToAll(&userJoin);
        _clients.push_back(new ClientSocket(socket));
        printf("socket=<%llu>�¿ͻ��˼��룺socket = %llu,IP = %s \n", _socket, socket, inet_ntoa(clientAddr.sin_addr));
    }

    return socket;
}

void TcpServer::Close()
{
    if(_socket == MYINVALID_SOCKET)
        return;

#ifdef _WIN32
    for(Int32 n = static_cast<Int32>(_clients.size() - 1); n >= 0; --n)
    {
        ClientSocket *client = _clients[n];
        delete client;
    }
    // 8 �ر��׽���closesocket
    closesocket(_socket);
    //------------
#else
    for(int n = (int)_clients.size() - 1; n >= 0; n--)
    {
        delete _clients[n];
    }
    // 8 �ر��׽���closesocket
    close(_socket);
#endif
    _socket = MYINVALID_SOCKET;
    _clients.clear();
}

void TcpServer::CloseClients()
{
#ifdef _WIN32
    for(Int32 n = static_cast<Int32>(_clients.size() - 1); n >= 0; --n)
    {
        ClientSocket *client = _clients[n];
        delete client;
    }
    //------------
#else
    for(int n = (int)_clients.size() - 1; n >= 0; n--)
    {
        delete _clients[n];
    }
#endif
    _clients.clear();
}

Int32 TcpServer::RecvData(ClientSocket *client)
{
    // 1. ���տͻ�������
    auto nLen = recv(client->GetSocket(), _recv, SOCKET_CACHE_SIZE, 0);
    //printf("nLen=%d\n", nLen);
    if(nLen <= 0)
    {
        printf("�ͻ���<Socket=%llu>���˳������������\n", client->GetSocket());
        return -1;
    }

    // 2.����ȡ�������ݿ�������Ϣ������
    memcpy(client->GetMsgBuf() + client->GetLastPos(), _recv, nLen);
    // 3.��Ϣ������������β��λ�ú���
    client->SetLastPos(client->GetLastPos() + nLen);

    // 4.�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
    while(client->GetLastPos() >= sizeof(PacketHeader))
    {
        // ��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
        auto *header = reinterpret_cast<PacketHeader *>(client->GetMsgBuf());
        // �ж���Ϣ�����������ݳ��ȴ�����Ϣ����
        if(client->GetLastPos() >= header->_packetLength)
        {
            // ��Ϣ������ʣ��δ�������ݵĳ���
            Int32 nSize = client->GetLastPos() - header->_packetLength;
            // ����������Ϣ
            OnNetMsg(client->GetSocket(), header);
            // ����Ϣ������ʣ��δ��������ǰ��
            memcpy(client->GetMsgBuf(), client->GetMsgBuf() + header->_packetLength, nSize);
            // ��Ϣ������������β��λ��ǰ��
            client->SetLastPos(nSize);
        }
        else {
            // ��Ϣ������ʣ�����ݲ���һ��������Ϣ
            break;
        }
    }
    return 0;
}

Int32 TcpServer::SendData(MYSOCKET socket, PacketHeader *header)
{
    if(UNLIKELY(!IsRun() || !header))
        return SOCKET_ERROR;

    return send(socket, reinterpret_cast<const char *>(header), header->_packetLength, 0);
}

void TcpServer::SendToAll(PacketHeader* header)
{
    for(Int32 n = static_cast<Int32>(_clients.size() - 1); n >= 0; --n)
        SendData(_clients[n]->GetSocket(), header);
}

bool TcpServer::OnRun()
{
    if(UNLIKELY(!IsRun()))
        return false;

    //�������׽��� BSD socket
    fd_set fdRead;//��������socket�� ����
    fd_set fdWrite;
    fd_set fdExp;
    //������
    FD_ZERO(&fdRead);
    FD_ZERO(&fdWrite);
    FD_ZERO(&fdExp);

    //����������socket�����뼯��
    FD_SET(_socket, &fdRead);
    FD_SET(_socket, &fdWrite);
    FD_SET(_socket, &fdExp);
    MYSOCKET maxSock = _socket;

    for(Int32 n = static_cast<Int32>( _clients.size() - 1); n >= 0; n--)
    {
        FD_SET(_clients[n]->GetSocket(), &fdRead);
        if(maxSock < _clients[n]->GetSocket())
        {
            maxSock = _clients[n]->GetSocket();
        }
    }
    ///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
    ///���������ļ����������ֵ+1 ��Windows�������������д0
    timeval t = {1, 0};
    auto ret = select(static_cast<Int32>(maxSock + 1), &fdRead, &fdWrite, &fdExp, &t); //
    //printf("select ret=%d count=%d\n", ret, _nCount++);
    if(ret < 0)
    {
        printf("select���������\n");
        CloseClients();
        return false;
    }
    //�ж���������socket���Ƿ��ڼ�����
    if(FD_ISSET(_socket, &fdRead))
    {
        FD_CLR(_socket, &fdRead);
        Accept();
    }
    for(Int32 n = static_cast<Int32>(_clients.size() - 1); n >= 0; n--)
    {
        if(FD_ISSET(_clients[n]->GetSocket(), &fdRead))
        {
            if(-1 == RecvData(_clients[n]))
            {
                auto iter = _clients.begin() + n;//std::vector<SOCKET>::iterator
                if(iter != _clients.end())
                {
                    delete _clients[n];
                    _clients.erase(iter);
                }
            }
        }
    }

    return true;
}

bool TcpServer::IsRun() const
{
    return _socket != MYINVALID_SOCKET;
}

void TcpServer::OnNetMsg(MYSOCKET socket, PacketHeader *header)
{
    switch(header->_cmd)
    {
        case ProtocolCmd::LoginReq:
        {

            LoginReq *login = static_cast<LoginReq *>(header);
            printf("�յ��ͻ���<Socket=%llu>����LoginReq,���ݳ��ȣ�%d,userName=%s PassWord=%s\n", socket, login->_packetLength, login->_userName, login->_pwd);

            // ����
            LoginNty nty;
            memcpy(nty._userName, login->_userName, sizeof(login->_userName));
            memcpy(nty._pwd, login->_pwd, sizeof(login->_pwd));
            SendData(socket, &nty);

            // res
            LoginRes res;
            memcpy(res._userName, login->_userName, sizeof(login->_userName));
            SendData(socket, &res);
        }
        break;
        default:
        {
            printf("<socket=%llu>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", socket, header->_packetLength);
            //DataHeader ret;
            //SendData(cSock, &ret);
        }
        break;
    }
}
