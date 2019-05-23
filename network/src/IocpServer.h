#ifndef _IocpServer_h_
#define _IocpServer_h_
#include <WinSock2.h>
#include <Windows.h>
#include <io.h>
#include <thread>
#include <iostream>
#include "connection.h"
#include "callbacks.h"
#include "SocketExFnsHunter.h"
#include <atomic>
#pragma comment(lib,"ws2_32.lib")

typedef const void* (*pfnGetBusinessLogicMainThread)();

class CSenderThread;
class CNetDispatch;

class IocpServer :public CallbacksExecutor
{
	friend class Workers;
	NO_COPY(IocpServer);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(IocpServer);

public:
	bool Init();
	void Fini();

	bool AsyncRead(const Connection* conn);

	bool AsyncWrite(const Connection* conn, const void* data, const std::size_t size);

	bool Lock();
	bool Unlock();

	//��������
public:
	bool IsDestroy() const;	//�Ƿ�����
	bool IsInit() const;	//�Ƿ��ʼ��
	void SetPort(const U16& nPort) { m_nPort = nPort; }		// ���ü����˿�
	void SetIp(const char * ip) { m_strIP = ip; }

	auto& GetIocpPort() {	return _completion_port; }
	void AddClientCount() { ++_client_count; }
	void DeductClientCount(const SOCKET_IDD sSocket);
	void AddMsgCount() { ++_msg_count; }
	void DeductMsgCount() { --_msg_count; }

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

	void GetBindAddrStr(void *pBuffer, const U16 u16SizeBuffer) const;
	const void *GetSenderThread() const { ASSERTEX(m_pSenderThread); return m_pSenderThread; }
	bool HandleError(const Connection* conn, OVERLAPPED* & pOverlapped, const DWORD& dwErr);

protected:
	bool InitWorkers(const I32 nWorkNum);
	bool WinSockInit();

	bool InitListenSocket();

	bool Accept(bool bSvrListen = false );

	bool Bind();

	bool Listen(unsigned int nListen);

protected:
	gtool::CSmartPtr<CDataLocker>		m_pSvrLocker = NULL;         // ����Worker�߳�ͬ���Ļ�����
	gtool::CFmtIOString                 m_strIP;                       // �������˵�IP��ַ
	U16                          m_nPort;                       // �������˵ļ����˿�
	std::atomic_bool m_bInit{ false };			//�Ƿ��ʼ��
	std::atomic<I32> m_nCurTaskNum{ 0 };	//��ǰ�����߳���
	static std::atomic_bool m_bDestroy;		//�Ƿ�����
	HANDLE _completion_port = NULL;			//��ɶ˿�
	SOCKET _socketSvrListen = INVALID_SOCKET;	//����socket
	std::atomic_int _client_count{ 0 };		//�ͻ�����
	std::atomic_int _msg_count{ 0 };		//��Ϣ��
	LPFN_ACCEPTEX _acceptex_func = NULL;	//acceptex
	gtool::CSmartPtr<CThreadPool> m_pThreadPool;			//�����̳߳�

	boost::atomic<pfnGetBusinessLogicMainThread> m_pfnGetBusinessLogicMainThread{ NULL };	//ҵ���߼�ģ���߳�
	gtool::CLitUnmanagedPtr<CSenderThread>				m_pSenderThread;				//������
	boost::atomic_bool									m_bIsThreadWork{ false };

	typedef gtool::CLitPtrMap<Connection, SOCKET_IDD, DEL_WAY_DELETE> ConnectionMap;	//�ͻ�������
	gtool::CSmartPtr<ConnectionMap> m_pConnectionMap;
	gtool::CSmartPtr<CNetDispatch>	m_pNetDispatch;
};

typedef NETWORK_SERVICE_API gtool::CSingleton<IocpServer> IocpServerInstance;
extern NETWORK_SERVICE_API IocpServer *g_pIocpMgr;
#define IocpMgr  ( g_pIocpMgr=IocpServerInstance::GetInstance(), ASSERT(g_pIocpMgr), *g_pIocpMgr)

#endif