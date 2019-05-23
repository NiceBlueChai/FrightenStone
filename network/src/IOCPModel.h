/*
==========================================================================

Purpose:

* �����CIOCPModel�Ǳ�����ĺ����࣬����˵��WinSock�������˱��ģ���е�
��ɶ˿�(IOCP)��ʹ�÷�������ʹ��MFC�Ի�����������������ʵ���˻�����
����������ͨ�ŵĹ��ܡ�

* ���е�PER_IO_DATA�ṹ���Ƿ�װ������ÿһ���ص������Ĳ���
PER_HANDLE_DATA �Ƿ�װ������ÿһ��Socket�Ĳ�����Ҳ��������ÿһ����ɶ˿ڵĲ���

* ��ϸ���ĵ�˵����ο� http://blog.csdn.net/PiggyXP

Notes:

* ���彲���˷������˽�����ɶ˿ڡ������������̡߳�Ͷ��Recv����Ͷ��Accept����ķ�����
���еĿͻ��������Socket����Ҫ�󶨵�IOCP�ϣ����дӿͻ��˷��������ݣ�����ʵʱ��ʾ��
��������ȥ��

Author:

* PiggyXP��С��

Date:

* 2009/10/04

==========================================================================
*/

#ifndef IOCPMODEL_H_
#define IOCPMODEL_H_


#pragma once

#include<BaseCode/NetworkServiceApi.h>

typedef struct _RW_CONTEXT PER_RW_CONTEXT, *PPER_RW_CONTEXT;
typedef struct _PER_SOCKET_CONTEXT PER_SOCKET_CONTEXT, *PPER_SOCKET_CONTEXT;

//
typedef const void* (*pfnGetBusinessLogicMainThread)();

class CThreadPool;
class CIocpTask;
class CSenderThread;
enum NET_IO_ERR_TYPE;


//�л���Э�������������л�ײ㲻�л�
// CIOCPModel��
NAMESPACE_BEGIN(gtool)

class NETWORK_SERVICE_API CIOCPModelMgr
{
	friend class CIocpTask;

	NO_COPY_NO_MOVE(CIOCPModelMgr);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CIOCPModelMgr);

	//ģ�����
public:	
	bool Init();	// ����������	
	void Fini();	//	ֹͣ������
	bool IsDestroy() const;	//�Ƿ�����
	bool IsMgrInit()const;
	//busines api��ַ
	void SetfnGetBusinessLogicMainThread(const void *pfn) {
		pfnGetBusinessLogicMainThread p = (pfnGetBusinessLogicMainThread)pfn;
		m_pfnGetBusinessLogicMainThread = p;
		ASSERTNET(m_pfnGetBusinessLogicMainThread);
	}
	const void *GetBusinessLogicMainThread()
	{
		CHECKF(m_pfnGetBusinessLogicMainThread);
		return m_pfnGetBusinessLogicMainThread;
	}

	//cp�����߳�ͬ������
public:
	bool Lock();	//iocp������Դ��
	bool IsLock();
	bool Unlock();
	bool Wait(const unsigned long ulMilliseconds = INFINITE);
	bool SinalOne();
	
	//��ѯsocket
	PER_SOCKET_CONTEXT* QueryClientContext(const SOCKET_IDD idSocket);

	//�ⲿ�ӿ�
public:
	_RW_CONTEXT * AddData(const SOCKET_IDD sDest, const void *pData, const U64 nSize, NET_IO_ERR_TYPE& errOut, bool bPutOutErrlog = false);
	bool SendData(const SOCKET_IDD sDest, _RW_CONTEXT& rDataContext, NET_IO_ERR_TYPE& errOut, bool bPutOutErrlog=false);	//�̲߳���ȫ��Ҫ����

//	bool RecvData(const SOCKET_IDD sDest, void *& pData, U64& nRecvSize);	//pData�����ض���������ָ����Ҫ������ָ��� nRecvSize ���ؽ��յ����ݴ�С
	bool Close(const SOCKET_IDD idSocket);
	void GetBindAddrStr(void *pBuffer, const U16 u16SizeBuffer) const;
	const void *GetSenderThread() const { ASSERTEX(m_pSenderThread); return m_pSenderThread; }
		
	//����
public:	
	void SetPort(const U16& nPort) { m_nPort = nPort; }		// ���ü����˿�
	void SetIp(const char * ip) { m_strIP = ip; }

	//iocp��ʼ��
protected:
	bool InitializeIOCP();		// ��ʼ��IOCP
	bool PreBindListenEnv();	//׼������������������
	void FreeResource();		// ����ͷ���Դ
	bool AssociateWithIOCP(PER_SOCKET_CONTEXT *pContext);	// socket����IOCp															
	bool HandleError(PER_SOCKET_CONTEXT *pSocketContext, OVERLAPPED* & pOverlapped, const DWORD& dwErr);	// ������ɶ˿��ϵĴ���

	//�������
protected:
	bool CreateSocket(_PER_SOCKET_CONTEXT& rSocketContext, bool bSetNoBlock = true);	//�����׽���
	bool DoAccpet(PER_RW_CONTEXT* pIoContext);	// ���пͻ��������ʱ�򣬽��д���																					
	bool DoRecv(PER_SOCKET_CONTEXT* pSocketContext, PER_RW_CONTEXT* pIoContext, const U64& nRecvByte, bool& bRecvNetData);	// ���н��յ����ݵ����ʱ�򣬽��д���
	//bool DoConnect(PER_SOCKET_CONTEXT* pSocketContext, PER_RW_CONTEXT* pIoContext, const U64& nRecvByte);	//�������
	bool DoSend(PER_SOCKET_CONTEXT& rSocketContext, PER_RW_CONTEXT& rIoContext, const U64& nSendByte);
	bool PostAccept(PER_SOCKET_CONTEXT& rSocketContext, PER_RW_CONTEXT* pIOContext = NULL);	//Ͷ��Accept����	
	bool PostRecv(PER_RW_CONTEXT* pIoContext);	// Ͷ�ݽ�����������
	bool PostSend(PER_RW_CONTEXT& rIoContext);	// Ͷ�ݷ�����������
	//bool PostConnect(PER_RW_CONTEXT* pIoContext);	//Ͷ����������
												
	bool AddToSocketMap(PER_SOCKET_CONTEXT *pSocketContext);	// ���ͻ��˵������Ϣ�洢��������																
	void RemoveFromSocketMap(PER_SOCKET_CONTEXT *pSocketContext);	// ���ͻ��˵���Ϣ���������Ƴ�
	void ClearSocketContainer();						// ��տͻ�����Ϣ
//	bool IsSocketAlive(PER_SOCKET_CONTEXT& rSocketContext);				// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ�	

												//�׽��ֳ�ʱ���
//	bool DelayMonitorSocket(long tv_sec, long tv_usec, bool bReadableDetect = true, bool bWriteableDetect = false, int *pErrOut = NULL, bool bSetOneAtLeast = false, bool bInfiniteWaiting = false);

	//����
protected:
	bool DoAcceptPostNewRecvSock(const SOCKET sSocket, const SOCKADDR_IN* pClientAddr);	//doaccept Ͷ��recv

private:
	boost::atomic_bool			m_bIsInitMgr{false};					//�Ƿ��ʼ�����
	HANDLE                      m_hIOCompletionPort = NULL;    // ��ɶ˿ڵľ��
	static boost::atomic_bool	m_bDestroy;			//����iocp
	CFmtIOString                 m_strIP;                       // �������˵�IP��ַ
	U16                          m_nPort;                       // �������˵ļ����˿�

	boost::atomic<I32>			m_CurWorkThreadNum{ 0 };		//��ǰ�����߳���
	CSmartPtr<CThreadPool>		m_pThreadPool = NULL;			//�̳߳�
	CSmartPtr<CDataLocker>		m_pContextLocer = NULL;         // ����Worker�߳�ͬ���Ļ�����
      
	typedef gtool::CLitPtrMap<PER_SOCKET_CONTEXT, U64, DEL_WAY_DELETE> NetPerSocketContextMap;	//key:PER_SOCKET_CONTEXT ADDR SOCKET_IDD, val:PER_SOCKET_CONTEXT
	CSmartPtr<NetPerSocketContextMap>	m_pClientSocketMap = NULL;		// �ͻ���Socket��Context��Ϣ  
	CSmartPtr<PER_SOCKET_CONTEXT>		m_pSvrListenSocket;             // ���ڼ�����Socket��Context��Ϣ


//	CSmartPtr<SocketMsgInfoList> m_pSocketMsgInfoRecvList;		//iocp2ext�� ��Ϣ����

	//����ָ��
	void *						m_lpfnAcceptEx;                // AcceptEx �� GetAcceptExSockaddrs �ĺ���ָ�룬���ڵ�����������չ����
	void *					    m_lpfnGetAcceptExSockAddrs;


private:
// 	CSmartPtr<CDataLocker>		m_pIocp2ExternalLock = NULL;		//����״̬�ź�	iocp�����ⲿ���ݽ���
// 	CSmartPtr<CDataLocker>		m_pIocp2ExternalLockIocpGuard = NULL;		//iocp���ػ� //iocp���ⲿ֪ͨ���ݵ���ʱ�ⲿ����iocp�ķ���

	boost::atomic_bool			m_bExternalRecvFromIocpAwake{ false };
	boost::atomic_bool			m_bExternalRecvFinish{ false };
	boost::atomic<pfnGetBusinessLogicMainThread> m_pfnGetBusinessLogicMainThread{ NULL };
	CSenderThread *				m_pSenderThread = NULL;				//������
	boost::atomic_bool				m_bIsThreadWork{ false };
};

//LPFN_ACCEPTEX
//LPFN_GETACCEPTEXSOCKADDRS
NAMESPACE_END(gtool)

typedef NETWORK_SERVICE_API gtool::CSingleton<gtool::CIOCPModelMgr> CIOCPModelInstance;
extern NETWORK_SERVICE_API gtool::CIOCPModelMgr *g_pIocpMgr;
#define IocpMgr  (ASSERT(g_pIocpMgr), *g_pIocpMgr)

#endif



