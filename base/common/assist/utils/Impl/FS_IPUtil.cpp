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
 * @file  : FS_IPUtil.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#pragma region winsock 2
#include <winsock2.h>
// #include <MSWSock.h>
#include<ws2tcpip.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#pragma comment(lib,"ws2_32.lib")
#pragma endregion

#include "base/common/component/Impl/FS_String.h"
#include "base/common/assist/utils/Defs/FS_IPUtilDefs.h"
#include "base/common/assist/utils/Impl/FS_IPUtil.h"
#include "base/common/status/status.h"

FS_NAMESPACE_BEGIN

bool FS_IPUtil::GetLocalIP(FS_String &ip, Int32 netCardNo, bool isToBind, bool isStreamSock, bool isIpv4)
{
    // ��ñ���������+
    BUFFER128 szBuffer = {0};
    char hostname[MAX_PATH] = {0};
    if(gethostname(hostname, MAX_PATH) != 0)
        return false;

    //��ȡip
    char *buffer = szBuffer;
    const char *pHost = hostname;
    const char *pService = NULL;
    if(!GetIPByDomain(pHost, pService
                      , pBuffer
                      , sizeof(szBuffer)
                      , netCardType
                      , bUseToBind ? NET_SYS::AI_FLAGS_TYPE_AI_PASSIVE : NET_SYS::AI_FLAGS_TYPE_NONE
                      , bSockStream ? NET_SYS::SOCK_TYPE_SOCK_STREAM : NET_SYS::SOCK_TYPE_SOCK_DGRAM
                      , bIpv4 ? NET_SYS::FAMILY_TYPE_AF_INET : NET_SYS::FAMILY_TYPE_AF_INET6
                      , NET_SYS::PROTOCOL_TYPE_IPPROTO_IP
    ))
        return false;

    strcpy(ip, szBuffer);

    return strlen(ip) > 0;
    return false;
}

bool FS_IPUtil::GetIPByDomain(
    P_IN const char *&domain /* ������������ */
    , P_IN const char *&service /* �˿ںš�80���ȡ������� ��"ftp", "http"�� */
    , P_OUT char *&ip /* ��� */
    , P_OUT const Int32 ipSize /* �������Ĵ�С */
    , Int32 netCardCount /*= 0 /* ����ȡ���Ǳ��صĵ�ַ��Ϊѡ����ַ�б��ĳһ����ַ */
    , Int32 eFlags /* = FS_IPUtilDefs::AI_FLAGS_TYPE_AI_PASSIVE /* FS_IPUtilDefs::AI_FLAGS_TYPE ����λ�����
                   Ĭ��AI_PASSIVE ������bind�� ������������connect */
    , FS_IPUtilDefs::SOCK_TYPE eSockType /*= FS_IPUtilDefs::SOCK_TYPE_SOCK_STREAM /* Ĭ�������� */
    , FS_IPUtilDefs::FAMILY_TYPE eFamily /*= FS_IPUtilDefs::FAMILY_TYPE_AF_INET /* Ĭ��ipv4 */
    , FS_IPUtilDefs::PROTOCOL_TYPE eProtocol /*= FS_IPUtilDefs::PROTOCOL_TYPE_IPPROTO_IP /* Ĭ������Э�� ��ipЭ�� */)
{
    // ����ͬʱΪNULL
    if(!domain && !service)
        return false;
    
    // ��ñ���������+
    struct addrinfo hints = {0};
    struct addrinfo *res = NULL, *cur = NULL;
    struct sockaddr_in *addr = NULL;
    BUFFER128 szBuffer = {0};

    // ��ȡ��ַ�Ĳ�������
    hints.ai_family = eFamily;
    hints.ai_flags = eFlags;
    hints.ai_protocol = eProtocol;
    hints.ai_socktype = eSockType;

    if(getaddrinfo(domain, service, &hints, &res) != 0)
    {
        throw std::logic_error("getaddrinfo failed");
        return false;
    }

    Int32 cnt = -1;
    for(cur = res; cur != NULL; cur = cur->ai_next)
    {
        addr = (struct sockaddr_in *)cur->ai_addr;
        if(!addr)
            continue;

        ++cnt;

        szBuffer[0] = 0;
        sprintf(szBuffer, "%d.%d.%d.%d"
                , (*addr).sin_addr.S_un.S_un_b.s_b1
                , (*addr).sin_addr.S_un.S_un_b.s_b2
                , (*addr).sin_addr.S_un.S_un_b.s_b3
                , (*addr).sin_addr.S_un.S_un_b.s_b4);

        if(netCardCount == cnt)
            break;
    }

    //�ͷ���Դ
    freeaddrinfo(res);

    //��������
    auto nLen = strlen(szBuffer);
    if(nLen == 0)
    {
        throw std::logic_error("szBuffer is null");
        return false;
    }

    if(ipSize <= nLen)
        return false;

    strcpy(ip, szBuffer);

    return strlen(ip) > 0;
}

Int32 FS_IPUtil::GetPeerAddr(UInt64 sSocket, P_OUT char *&ip, const UInt64 sizeIp, P_OUT UInt16 &port)
{
    if(!sSocket || sSocket == INVALID_SOCKET || !sizeIp)
        return StatusDefs::Socket_ParamError;

    //����
    struct sockaddr_in dstadd_in;
    memset(&dstadd_in, 0, sizeof(dstadd_in));
    socklen_t  len = sizeof(dstadd_in);

    if(getpeername(sSocket, (struct sockaddr*)&dstadd_in, &len) >= 0)
        return WSAGetLastError();

    if(inet_ntop(dstadd_in.sin_family, &dstadd_in.sin_addr.s_addr, ip, sizeIp) == NULL)
        return -1;

    port = ntohs(dstadd_in.sin_port);

    return StatusDefs::Success;
}
