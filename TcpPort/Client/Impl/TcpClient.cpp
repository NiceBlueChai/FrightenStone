#include "stdafx.h"
#include <TcpPort/Client/Impl/TcpClient.h>

#pragma region windows����֧��
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifdef _WIN32
// #include<windows.h>
 #include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif // _WIN32
#pragma endregion

TcpClient::TcpClient()
{
    
}

TcpClient::~TcpClient()
{

}

#pragma region �׽��ֲ���
Int32 TcpClient::InitSocket(bool blockSocket)
{
    // 1.�����׽���
    if(_sock != MYINVALID_SOCKET)
        return StatusDefs::Repeat;

    _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(_sock == MYINVALID_SOCKET)
    {
        printf("���󣬽���Socketʧ��...\n");
        return StatusDefs::Socket_CreateFailure;
    }

    if(blockSocket)
        fs::SocketUtil::SetBlock(_sock);
    else
        fs::SocketUtil::SetNoBlock(_sock);

    printf("����Socket=<%llu>�ɹ�...\n", _sock);
    
    return StatusDefs::Success;
}

Int32 TcpClient::Connect(const char *ip, UInt16 port) const
{
    // 1.�ж�socket�Ƿ��ʼ��
    if(_sock == MYINVALID_SOCKET)
        return StatusDefs::Socket_NotInit;

    // 2.���ӷ�����
    sockaddr_in sin = {};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
#ifdef _WIN32
    sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
    sin.sin_addr.s_addr = inet_addr(ip);
#endif

    printf("<socket=%llu>�������ӷ�����<%s:%d>...\n", _sock, ip, port);
    int ret = connect(_sock, reinterpret_cast<sockaddr *>(&sin), sizeof(sockaddr_in));
    if(SOCKET_ERROR == ret)
    {
        printf("<socket=%llu>�������ӷ�����<%s:%d>ʧ��...\n", _sock, ip, port);
    }
    else {
        printf("<socket=%llu>���ӷ�����<%s:%d>�ɹ�...\n", _sock, ip, port);
    }

    return ret;
}

Int32 TcpClient::RecvData()
{
    // 1 ��������
    auto nLen = recv(_sock, _recv, SOCKET_CACHE_SIZE, 0);
    printf("nLen=%d\n", nLen);

    if(nLen <= 0)
    {
        printf("<socket=%llu>��������Ͽ����ӣ����������\n", _sock);
        return StatusDefs::Error;
    }

    // 2.����ȡ�������ݿ�������Ϣ������
    memcpy(_msgBuf + _lastPos, _recv, nLen);

    // 3.��Ϣ������������β��λ�ú���
    _lastPos += nLen;

    // 4.�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
    while(_lastPos >= sizeof(PacketHeader))
    {
        // 5.��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
        auto *header = reinterpret_cast<PacketHeader *>(_msgBuf);

        // 7.�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
        if(_lastPos >= header->_packetLength)    // ��Ҫ�ж���Ϣͷ�Ƿ�������TODO
        {
            // ��Ϣ������ʣ��δ�������ݵĳ���
            int nSize = _lastPos - header->_packetLength;

            // ����������Ϣ
            OnNetMsg(header);

            // ����Ϣ������ʣ��δ��������ǰ��
            memcpy(_msgBuf, _msgBuf + header->_packetLength, nSize);

            // ��Ϣ������������β��λ��ǰ��
            _lastPos = nSize;
        }
        else {
            // ��Ϣ������ʣ�����ݲ���һ��������Ϣ
            break;
        }
    }

    return StatusDefs::Success;
}

Int32 TcpClient::SendData(PacketHeader* header) const
{
    if(IsRun() && header)
    {
        return send(_sock, reinterpret_cast<char *>(header), header->_packetLength, 0);
    }

    return StatusDefs::Error;
}

void TcpClient::Close()
{
    if(_sock != MYINVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(_sock);
#else
        close(_sock);
#endif
        _sock = MYINVALID_SOCKET;
    }
}
#pragma endregion

#pragma region ��Ϣ����
bool TcpClient::OnRun()
{
    if(IsRun())
    {
        fd_set fdReads;
        FD_ZERO(&fdReads);
        FD_SET(_sock, &fdReads);
        timeval t = {0, 0};
        auto ret = select(static_cast<Int32>(_sock + 1), &fdReads, NULL, NULL, &t);
        // printf("select ret=%d count=%d\n", ret, _nCount++);
        if(ret < 0)
        {
            printf("<socket=%llu>select�������1\n", _sock);
            Close();
            return false;
        }
        if(FD_ISSET(_sock, &fdReads))
        {
            FD_CLR(_sock, &fdReads);

            if(-1 == RecvData())
            {
                printf("<socket=%llu>select�������2\n", _sock);
                Close();
                return false;
            }
        }
        return true;
    }
    return false;
}

bool TcpClient::IsRun() const
{
    return _sock != MYINVALID_SOCKET;
}

void TcpClient::OnNetMsg(PacketHeader *header)
{
    switch(header->_cmd)
    {
        case ProtocolCmd::LoginRes:
        {

            auto *res = static_cast<LoginRes *>(header);
            printf("<socket=%llu>�յ��������Ϣ��LoginRes,���ݳ��ȣ�%d\n userName[%s] status[%d]"
                   , _sock, res->_packetLength, res->_userName, res->_status);
        }
        break;
        case ProtocolCmd::LoginNty:
        {
            auto *loginNty = static_cast<LoginNty *>(header);
            printf("<socket=%llu>�յ��������Ϣ��LoginNty,���ݳ��ȣ�%d name[%s] pwd[%s]\n", _sock, loginNty->_packetLength, loginNty->_userName, loginNty->_pwd);
        }
        break;
        default:
        {
            printf("<socket=%llu>�յ�δ������Ϣ[%d],���ݳ��ȣ�%d\n", _sock, header->_cmd, header->_packetLength);
        }
    }
}
#pragma endregion
