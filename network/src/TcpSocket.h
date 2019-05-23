#ifndef TCP_SOCKET_H_
#define TCP_SOCKET_H_
#pragma once
#include <BaseCode/NetworkServiceApi.h>
#include <Net/SocketProtocol.h>

//�����ɹ�����ֵΪ0; ���򷵻�ֵΪSOCKET_ERROR������ͨ������WSAGetLastError��ȡ������롣

//��һ�����̵߳Ļ����£�WSACleanup()��ֹ��Windows Sockets�������߳��ϵĲ���.
//�����ַ��Ϣ�ṹ��


//�ͻ��ˣ� ��������ʹ������ʽ���������Ӧ�÷�����socketʹ�÷�����ʽ
//createsocket�ӿ�
//connect
//send
//recv

//����� ������ʽ
//ʹ��iocp
//ֻ�г����send��recv�ӿ�

class CSenderThread;

class NETWORK_SERVICE_API CTcpServerMgr
{
	NO_COPY_NO_MOVE(CTcpServerMgr);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CTcpServerMgr);

	//�ӿڹ���
public:
	bool Start();	//����iocp
	bool Send(const SOCKET_IDD idSocket, const void *&pData, const U64 u64DataLen, TRAN_PROTOCOL::TRAN_TYPE eTranType = TRAN_PROTOCOL::TRAN_TYPE_MSG);	//��������
	bool Recv();	//pData���Ϊ���������ڴ���Ҫ������ָ�����
	bool Close(const SOCKET_IDD idSocket);
	bool FinishIocp();
	bool IsInitIocp();
	bool IsInit() const { return m_bIsInit; }
	bool IsDestroy() const { return m_bIsDestroy; }

	//��������
public:
	const void *GetSenderThread() const { ASSERTEX(m_pSenderThread); return m_pSenderThread; }

	//���ݴ���
protected:
	bool HandlerRecvData(SocketMsgInfo& rMsgInfo);

	//����ͳ��
protected:
	boost::atomic_bool	m_bIsInit{ false };
	static boost::atomic_bool	m_bIsDestroy;

	//һ���߳����� ���ͺͽ���iocp����
	gtool::CSmartPtr<CThreadPool> m_pThreadPool = NULL;		//�����߳� recvһ����senderһ��
	boost::atomic_bool				m_bIsThreadWork{ false };
	CSenderThread *				m_pSenderThread = NULL;				//������
};

typedef NETWORK_SERVICE_API gtool::CSingleton<CTcpServerMgr> CTcpServerMgrInstance;
extern NETWORK_SERVICE_API CTcpServerMgr *g_pTcpServerMgr;
#define TcpServerMgr  (ASSERT(g_pTcpServerMgr), *g_pTcpServerMgr)


////////////////////////////////////////////////////////////////

//IPC
#endif