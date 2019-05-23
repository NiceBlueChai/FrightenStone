#include "stdafx.h"
#include <Net/NetTool.h>


NAMESPACE_BEGIN(gtool)



bool GetIPByDomain(
	P_IN const char *&domain												//������������
	, P_IN const char *& service											/*�˿ںš�80���ȡ������� ��"ftp", "http"��*/
	, P_OUT char *& ip														//���
	, P_OUT const I32 szIp													//�������Ĵ�С
	, const I32& nNetCardCount											//����ȡ���Ǳ��صĵ�ַ��Ϊѡ����ַ�б��ĳһ����ַ
	, const I32& eFlags				//NET_SYS::AI_FLAGS_TYPE ����λ����� Ĭ��AI_PASSIVE ������bind�� ������������connect
	, NET_SYS::SOCK_TYPE eSockType		//Ĭ��������
	, NET_SYS::FAMILY_TYPE eFamily			//Ĭ��ipv4
	, NET_SYS::PROTOCOL_TYPE eProtocol //Ĭ������Э�� ��ipЭ��
)
{
	CHECKF_NL(domain || service);	//����ͬʱΪNULL

									// ��ñ���������+
	struct addrinfo hints = { 0 };
	struct addrinfo *res = NULL, *cur = NULL;
	struct sockaddr_in *addr = NULL;
	BUFFER128 szBuffer = { 0 };

	//��ȡ��ַ�Ĳ�������
	hints.ai_family = eFamily;
	hints.ai_flags = eFlags;
	hints.ai_protocol = eProtocol;
	hints.ai_socktype = eSockType;

	CHECKF_NL(getaddrinfo(domain, service, &hints, &res) == 0);
	I32 nCount = -1;
	for (cur = res; cur != NULL; cur = cur->ai_next)
	{
		addr = (struct sockaddr_in *)cur->ai_addr;
		IF_NOT_NL(addr)
			continue;

		++nCount;

		szBuffer[0] = 0;
		sprintf(szBuffer, "%d.%d.%d.%d"
			, (*addr).sin_addr.S_un.S_un_b.s_b1
			, (*addr).sin_addr.S_un.S_un_b.s_b2
			, (*addr).sin_addr.S_un.S_un_b.s_b3
			, (*addr).sin_addr.S_un.S_un_b.s_b4);

		if (nNetCardCount == nCount)
			break;
	}

	//�ͷ���Դ
	freeaddrinfo(res);

	//��������
	auto nLen = strlen(szBuffer);
	CHECKF_NL(nLen != 0);
	CHECKF_NL(szIp > nLen);

	strcpy(ip, szBuffer);

	return strlen(ip) > 0;
}


//��ȡ�����ַ��Ϣ 0��ʾ�ɹ�����ֵΪ������Ϣ
I32 GetPeerAddr(const U64 sSocket, P_OUT char *&ip, const U64 sizeIp, P_OUT U16& port)
{
	CHECKF_NL(sSocket&&sSocket != INVALID_SOCKET && sizeIp != 0);

	//����
	struct sockaddr_in dstadd_in;
	ZERO_CLEAN_OBJ(dstadd_in);
	socklen_t  len = sizeof(dstadd_in);

	IF_NOT_NL(getpeername(sSocket, (struct sockaddr*)&dstadd_in, &len) < 0) {
		return WSAGetLastError();
	}

	IF_NOT_NL(inet_ntop(dstadd_in.sin_family, &dstadd_in.sin_addr.s_addr, ip, sizeIp) != NULL)
		return -1;

	port = ntohs(dstadd_in.sin_port);

	return 0;
}


//ת��Ϊ�����ֽ���
bool FillTcpAddrInfo(P_OUT SOCKADDR_IN& AddrObj, P_IN const char *ip, P_IN  const U16 port, P_IN  const U16 family)
{
	CHECKF_NL(ip&&strlen(ip) > 0);

	ZERO_CLEAN_OBJ(AddrObj);
	AddrObj.sin_family = family;
	CHECKF_NL(inet_pton(AddrObj.sin_family, ip, &AddrObj.sin_addr.s_addr) > 0);
	AddrObj.sin_port = htons(port);

	return true;
}

//ת��Ϊ������Ϣ
bool GetAddrInfoFromNetInfo(P_IN const SOCKADDR_IN& AddrObj, P_OUT char *& ip, P_OUTSIZE const U64 szip, P_OUT U16& port)
{
	CHECKF_NL(inet_ntop(AddrObj.sin_family, &AddrObj.sin_addr.s_addr, ip, szip) != NULL);
	port = ntohs(AddrObj.sin_port);

	return true;
}

static boost::atomic_bool	m_bInitEnvironment{ false };				//�Ƿ��ʼ�����绷��

bool LoadSocketEnvLib()
{
	try
	{
		if (m_bInitEnvironment)
			return false;

		m_bInitEnvironment = true;

		//����socket ��ʼ������
		WSADATA wsaData;
		memset(&wsaData, 0, sizeof(WSADATA));
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);	//����winsock DLL ÿ�����̵���һ�ξ͹��� �����޷�ʹ��socket
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			WSACleanup();
			m_bInitEnvironment = false;
			return false;
		}

		IF_NOT(iResult == NO_ERROR)
		{
			m_bInitEnvironment = false;
			return false;
		}

	}
	catch (...)
	{
		m_bInitEnvironment = false;
		return false;
	}

	return true;
}

bool UnLoadSocketEnvLib()
{
	int iRet = NO_ERROR;
	try
	{
		if (!m_bInitEnvironment) return true;

		m_bInitEnvironment = false;

		if ((iRet = WSACleanup()) == SOCKET_ERROR)
		{
			if (WSACancelBlockingCall() == 0)//ȡ�������׽ӿ�
			{
				iRet = WSACleanup();
			}
		}

		IF_NOT(iRet == NO_ERROR)
		{
			m_bInitEnvironment = true;
			return false;
		}
	}
	catch (...)
	{
		iRet = -1;
	}


	//һ�����̵���һ�ξ͹�����ʹ�ù����б�����������쳣

	return iRet == NO_ERROR ? true : false;
}

bool IsLoadSocketEnvLib()
{
	return m_bInitEnvironment;
}

bool SetSocketNoBlockMode(SOCKET& rSocket, bool bNoBlock /*= true*/)
{
	CHECKF(IS_VALID_SOCKET_IDD(rSocket));

	//���÷�����ģʽ
	MYULONG ul = (bNoBlock ? 1 : 0);
	if (0 != ioctlsocket(rSocket, FIONBIO, &ul))
	{
		return false;
	}

	return true;
}

//�׽��ֳ�ʱ���
bool IsDetectTimeOut(SOCKET& rSocket, fd_set&rReadableSet, fd_set&rWritableSet, long tv_sec, long tv_usec, bool bReadableDetect, bool bWriteableDetect, int *pErrOut, bool bSetOneAtLeast, bool bInfiniteWaiting)
{
	//��������
	ZERO_CLEAN_OBJ(rReadableSet);
	ZERO_CLEAN_OBJ(rWritableSet);

	//����
	FD_ZERO(&rReadableSet);
	FD_ZERO(&rWritableSet);
	FD_SET(rSocket, &rReadableSet);
	FD_SET(rSocket, &rWritableSet);

	//��ʱ��ز�����ʼ��
	timeval timeout;
	ZERO_CLEAN_OBJ(timeout);
	timeout.tv_sec = tv_sec;	//�����
	timeout.tv_usec = tv_usec;	//���΢��

								//�����׽���״̬
	int ret = SOCKET_ERROR;
	if (bInfiniteWaiting) { //��������
		ret = select(rSocket + 1, &rReadableSet, &rWritableSet, NULL, NULL);	//0��ʾ��ʱ�����򷵻�SOCKET_ERROR ������Ϊ-1ʱ����������������0�� ������Ϊ����ʱ����ʾ�Ѿ�׼���õ�����������
	}
	else {
		ret = select(rSocket + 1, &rReadableSet, &rWritableSet, NULL, &timeout);	//0��ʾ��ʱ�����򷵻�SOCKET_ERROR ������Ϊ-1ʱ����������������0�� ������Ϊ����ʱ����ʾ�Ѿ�׼���õ�����������
	}

	//�������
	if (pErrOut) *pErrOut = ret;
	IF_NOT(ret != SOCKET_ERROR) {
		FD_CLR(rSocket, &rReadableSet);
		FD_CLR(rSocket, &rWritableSet);
		return true;
	}

	//�������
	bool bTimeOut = false, bReadTimeOut = false, bWriteTimeOut = false;

	// �ж�socket����Ƿ�ɶ�  
	if (!FD_ISSET(rSocket, &rReadableSet)) bReadTimeOut = true;

	// �ж�socket����Ƿ��д  
	if (!FD_ISSET(rSocket, &rWritableSet)) bWriteTimeOut = true;

	//���״̬
	FD_CLR(rSocket, &rReadableSet);
	FD_CLR(rSocket, &rWritableSet);

	//����״̬����
	if (bSetOneAtLeast) {
		bTimeOut = (bReadTimeOut || bWriteTimeOut);
	}
	else {
		if (bReadableDetect && !bWriteableDetect) {
			bTimeOut = (bReadTimeOut);
		}
		else if (!bReadableDetect && bWriteableDetect) {
			bTimeOut = (bWriteTimeOut);
		}
		else {
			bTimeOut = (bReadTimeOut && bWriteTimeOut);
		}
	}

	return bTimeOut;
}

bool SetSocketCacheSize(SOCKET& rSocket, SOCKET_CACHE_TYPE eType, const I64 nSize)
{
	CHECKF(IS_VALID_SOCKET_IDD(rSocket));
	switch (eType)
	{
	case SOCKET_CACHE_TYPE_RECV:
		{
			auto nRet = setsockopt(rSocket, SOL_SOCKET, SO_RCVBUF, (char *)(&nSize), sizeof(nSize));
			auto nErr = WSAGetLastError();
			IF_NOT(nRet == 0)
			{
				LOGERR("SetSocketCacheSize SOCKET_CACHE_TYPE_RECV nSize[", nSize, "]. err[", nErr, "].");
				return false;
			}
		}
		break;
	case SOCKET_CACHE_TYPE_SEND:
		{
			auto nRet = setsockopt(rSocket, SOL_SOCKET, SO_SNDBUF, (char *)(&nSize), sizeof(nSize));
			auto nErr = WSAGetLastError();
			IF_NOT(nRet == 0)
			{
				LOGERR("SetSocketCacheSize SOCKET_CACHE_TYPE_SEND nSize[", nSize, "]. err[", nErr, "].");
				return false;
			}
		}
		break;
	default:
		{
			ASSERT(false);
			LOGERR("SetSocketCacheSize unknown type[", eType, "]. nSize[" , nSize, "].");
		}
		return false;
	}

	return true;
}

bool GetSocketCacheSize(SOCKET& rSocket, SOCKET_CACHE_TYPE eType, I64& nSize)
{
	CHECKF(IS_VALID_SOCKET_IDD(rSocket));
	switch (eType)
	{
	case SOCKET_CACHE_TYPE_RECV:
		{
			I32 nSizeLen = sizeof(nSize);
			auto nRet = getsockopt(rSocket, SOL_SOCKET, SO_RCVBUF, (char *)(&nSize), &nSizeLen);
			auto nErr = WSAGetLastError();
			IF_NOT(nRet == 0)
			{
				LOGERR("GetSocketCacheSize SOCKET_CACHE_TYPE_RECV err[", nErr, "].");
				return false;
			}
		}
		break;
	case SOCKET_CACHE_TYPE_SEND:
		{
			I32 nSizeLen = sizeof(nSize);
			auto nRet = getsockopt(rSocket, SOL_SOCKET, SO_SNDBUF, (char *)(&nSize), &nSizeLen);
			auto nErr = WSAGetLastError();
			IF_NOT(nRet == 0)
			{
				LOGERR("GetSocketCacheSize SOCKET_CACHE_TYPE_SEND err[", nErr, "].");
				return false;
			}
		}
		break;
	default:
		{
			ASSERT(false);
			LOGERR("GetSocketCacheSize unknown type[", eType, "].");
		}
		return false;
	}

	return true;
}


NAMESPACE_END(gtool)






