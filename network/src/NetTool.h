#ifndef NET_TOOL_H_
#define NET_TOOL_H_

#pragma once

NAMESPACE_BEGIN(gtool)


bool GetIPByDomain(
	P_IN const char *&domain												//������������
	, P_IN const char *& service											/*�˿ںš�80���ȡ������� ��"ftp", "http"��*/
	, P_OUT char *& ip														//���
	, P_OUT const I32 szIp													//�������Ĵ�С
	, const I32& nNetCardCount = 0											//����ȡ���Ǳ��صĵ�ַ��Ϊѡ����ַ�б��ĳһ����ַ
	, const I32& eFlags = NET_SYS::AI_FLAGS_TYPE_AI_PASSIVE					//NET_SYS::AI_FLAGS_TYPE ����λ����� Ĭ��AI_PASSIVE ������bind�� ������������connect
	, NET_SYS::SOCK_TYPE eSockType = NET_SYS::SOCK_TYPE_SOCK_STREAM			//Ĭ��������
	, NET_SYS::FAMILY_TYPE eFamily = NET_SYS::FAMILY_TYPE_AF_INET			//Ĭ��ipv4
	, NET_SYS::PROTOCOL_TYPE eProtocol = NET_SYS::PROTOCOL_TYPE_IPPROTO_IP //Ĭ������Э�� ��ipЭ��
);


//��ȡ�����ַ��Ϣ 0��ʾ�ɹ�����ֵΪ������Ϣ
I32 GetPeerAddr(const U64 sSocket, P_OUT char *&ip, const U64 szIp, P_OUT U16& port);


//ת��Ϊ�����ֽ���
bool FillTcpAddrInfo(P_OUT SOCKADDR_IN& AddrObj, P_IN const char *ip, P_IN  const U16 port, P_IN  const U16 family);

//ת��Ϊ������Ϣ
bool GetAddrInfoFromNetInfo(P_IN const SOCKADDR_IN& AddrObj, P_OUT char *& ip, P_OUTSIZE const U64 szip, P_OUT U16& port);

bool LoadSocketEnvLib();
bool UnLoadSocketEnvLib();
bool IsLoadSocketEnvLib();

bool SetSocketNoBlockMode(SOCKET& rSocket, bool bNoBlock = true);

bool IsDetectTimeOut(SOCKET& rSocket, fd_set&rReadableSet, fd_set&rWritableSet, long tv_sec, long tv_usec, bool bReadableDetect = true, bool bWriteableDetect = false, int *pErrOut = NULL, bool bSetOneAtLeast = false, bool bInfiniteWaiting = false);

//����socket��������С
bool SetSocketCacheSize(SOCKET& rSocket, SOCKET_CACHE_TYPE eType, const I64 nSize);

//��ȡsocket��������С
bool GetSocketCacheSize(SOCKET& rSocket, SOCKET_CACHE_TYPE eType, I64& nSize);

NAMESPACE_END(gtool)

#endif

