#include "StdAfx.h"
//#include <Net/NetConfig.h>
#include <Net/IOCPModel.h>
#include <BaseCode/Win/ThreadPool.h>
#include<BaseCode/TaskBase.h>
#include <thread>
#include <iostream>
#include <BaseCode/enumtype.h>
#include <Net/Sender.h>



#define WORKER_THREADS_PER_PROCESSOR	8		// ÿһ���������ϲ������ٸ��߳�(Ϊ������޶ȵ��������������ܣ���������ĵ�)
#define MAX_POST_ACCEPT					10		// ͬʱͶ�ݵ�Accept���������(���Ҫ����ʵ�ʵ�����������)
#define EXIT_CODE						NULL	// ���ݸ�Worker�̵߳��˳��ź�
#define JUDGE_SOCKET_ALIVE_DELAY_TIME	100		//100΢�� �жϿͻ����Ƿ���


#define RELEASE(x)                      {if(x != NULL ){delete x;x=NULL;}}	// �ͷ�ָ���
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}	// �ͷž����
#define RELEASE_SOCKET(x)               {if(x !=INVALID_SOCKET) { closesocket(x);x=INVALID_SOCKET;}}	// �ͷ�Socket��

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//����

//����
class CIocpTask :public CTaskBase
{
	NO_COPY(CIocpTask);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CIocpTask);

public:	
	virtual int Run();	//����ִ����

protected:
	bool NetHandler(gtool::CIOCPModelMgr& rIocpMgr, DWORD& rBytesTransfered, PER_SOCKET_CONTEXT& rSockteContext, PER_RW_CONTEXT& rRWContext, bool& bRecvData);	//�������
	bool NewPostAccept(gtool::CIOCPModelMgr& rIocpMgr);	//�µ�acceptͶ��
};

CIocpTask::CIocpTask()
{
}

CIocpTask::~CIocpTask()
{
}

bool CIocpTask::NewPostAccept(gtool::CIOCPModelMgr& rIocpMgr)
{
	CHECKF(gtool::IsLoadSocketEnvLib());
	if (rIocpMgr.IsDestroy())
		return true;

	gtool::CSmartPtrC<PER_SOCKET_CONTEXT> pNewSocket = new PER_SOCKET_CONTEXT;
	CHECKF(pNewSocket);

	//����Ͷ��
	CHECKF(rIocpMgr.PostAccept(*pNewSocket));

	CHECKF(rIocpMgr.AddToSocketMap(pNewSocket.pop()));

	return true;
}

//�������
bool CIocpTask::NetHandler(gtool::CIOCPModelMgr& rIocpMgr, DWORD& rBytesTransfered, PER_SOCKET_CONTEXT& rSockteContext, PER_RW_CONTEXT& rRWContext, bool& bRecvData)
{
	// �ж��Ƿ��пͻ��˶Ͽ���
	if ((0 == rBytesTransfered)
		&& (RECV_POSTED == rRWContext.m_OpType
			|| SEND_POSTED == rRWContext.m_OpType))
	{
		//rBuffer[0] = 0;
		LOGNET("�ͻ���", (const char *)(rSockteContext.GetAddrInfo()), ":", U16(rSockteContext.GetAddrInfo()), " �Ͽ�����.");
	
		// �ͷŵ���Ӧ����Դ
		rIocpMgr.RemoveFromSocketMap(&rSockteContext);

		//����Ͷ���µ�accpet
		IF_NOT(NewPostAccept(rIocpMgr)) {
			return false;
		}
	}
	else
	{
		switch (rRWContext.m_OpType)
		{
		case ACCEPT_POSTED:// Accept  
			{
				IF_NOT(rIocpMgr.DoAccpet(&rRWContext))
				{
					//�Ƴ�
					rIocpMgr.RemoveFromSocketMap(&rSockteContext);

					//����Ͷ���µ�accpet
					IF_NOT(NewPostAccept(rIocpMgr)) {
						return false;
					}
				}
			}
			break;
		case RECV_POSTED:	// RECV
			{
				IF_NOT(rIocpMgr.DoRecv(&rSockteContext, &rRWContext, rBytesTransfered, bRecvData)) {
					ASSERTNET("DoRecv����");
				}

				//��������
// 				BUFFER128 szBuffer = { 0 };
// 				sprintf(szBuffer, "hello��");
// 				NET_IO_ERR_TYPE eErrType = NET_IO_ERR_TYPE_NONE;
// 				IF_NOT(rIocpMgr.SendData(SOCKET_IDD(&rSockteContext), szBuffer, strlen(szBuffer) + 1, eErrType))
// 				{
// 					ASSERTNET("SendData����");
// 					LOGNET("SendData���� eErrType[", eErrType, "]");
// 				}
			}
			break;
		case SEND_POSTED:	// SEND
			{
				LOGNET("SEND_POSTED.");
				IF_NOT(rIocpMgr.DoSend(rSockteContext, rRWContext, rBytesTransfered)) {
					ASSERTNET("DoSend����");
				}
			}
			break;
// 		case CONNECT_POSTED:
// 			{
// 				LOGNET("CONNECT_POSTED.");
// 				IF_NOT(rIocpMgr.DoConnect(&rSockteContext, &rRWContext, rBytesTransfered)) {
// 					ASSERTNET(!"DoConnect ����");
// 				}
// 			}
// 			break;
		default:
			LOGNET("_WorkThread�е� pIoContext->m_OpType=[", rRWContext.m_OpType, "] �����쳣.");
			ASSERTNET(false);
			break;
		} //switch
	}//if

	return true;
}

int CIocpTask::Run()
{
	gtool::CIOCPModelMgr* pIOCPModel = (gtool::CIOCPModelMgr*)m_pArg;	//iocpģ��
	CHECKF(pIOCPModel);

	if (pIOCPModel->IsDestroy())
		return 0;

	//��������
	OVERLAPPED           *pOverlapped = NULL;
	PER_SOCKET_CONTEXT   *pSocketContext = NULL;
	DWORD                dwBytesTransfered = 0;
	
	//�߳̽���
	++(pIOCPModel->m_CurWorkThreadNum);	//�߳���Ŀ
	LOGNET("IOCP �������߳� ", std::this_thread::get_id(), "����.");

	// ѭ����������֪�����յ�Shutdown��ϢΪֹ
	while (!pIOCPModel->IsDestroy())
	{
		bool bReturn = GetQueuedCompletionStatus(
			pIOCPModel->m_hIOCompletionPort,		//cp����
			&dwBytesTransfered,						//������ֽ���
			(PULONG_PTR)&pSocketContext,			//�׽��ֵȣ���������ʱ��Ҫ�ͷ� pSocketContext��socket��һ����pOverlapped��socket��Ӧ��
			&pOverlapped,							//����send recv�����ݻ�����ص��ṹ���ڶ��� ��pSocketContext�еĻ������������� socket��Ҫ��pOverlapped
			INFINITE);

		if(!pSocketContext || !pOverlapped)
			continue;

		// ����յ������˳���־����ֱ���˳�
		if (EXIT_CODE == (DWORD)pSocketContext)
			break;

		//�ٽ���
		ASSERTNET(pIOCPModel->Lock());

		// �ж��׽����Ƿ�����˴���
		if (!bReturn)
		{
			DWORD dwErr = GetLastError();
			if (!pIOCPModel->HandleError(pSocketContext, pOverlapped, dwErr))
			{				
				PER_RW_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_RW_CONTEXT, m_Overlapped);
				
				char *ip = "";
				U16 u16Port = 0;
				if (pIoContext)
				{
					ip = const_cast<char *>(pIoContext->GetIp());
					u16Port = pIoContext->GetPort();
				}

				LOGNET("�׽��ֳ���.", ip, ":", u16Port, "dwErr[", dwErr, "]");

				// �رտͻ���
				pIOCPModel->RemoveFromSocketMap(pSocketContext);	//�Ƴ�socket

				//����Ͷ���µ�accpet
				IF_NOT(NewPostAccept(*pIOCPModel)){		//����Ͷ��accept
					pIOCPModel->Unlock();
					break;
				}
			}

			pIOCPModel->Unlock();
			continue;
		}

		// ��ȡ����Ĳ���
		PER_RW_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_RW_CONTEXT, m_Overlapped);	//ϵͳ���Ļ���������
		IF_NOT(pIoContext)
		{
			pIOCPModel->Unlock();
			continue;
		}			
		
		//�������
		bool bRecvData = false;
		IF_NOT(NetHandler(*pIOCPModel, dwBytesTransfered, *pSocketContext, *pIoContext, bRecvData))
		{
			ASSERTNET(pIOCPModel->Unlock());
			break;
		}

		ASSERTNET(pIOCPModel->Unlock());

		//֪ͨ��������
		if (bRecvData)
		{
		}

	}//while

	//�߳��˳�
	--(pIOCPModel->m_CurWorkThreadNum);
	if (pIOCPModel->m_CurWorkThreadNum <= 0)
	{
		ASSERTNET(pIOCPModel->Lock());
		pIOCPModel->SinalOne();
		ASSERTNET(pIOCPModel->Unlock());
	}
		

	LOGNET("IOCP �������߳� ", std::this_thread::get_id(),"�˳�.");
	return 0;
}

void CIocpTask::Cleans()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
gtool::CIOCPModelMgr *g_pIocpMgr = CIOCPModelInstance::GetInstance();
NAMESPACE_BEGIN(gtool)

//tcpsocket
boost::atomic_bool CIOCPModelMgr::m_bDestroy{ false };
CIOCPModelMgr::CIOCPModelMgr():
							m_hIOCompletionPort(NULL),
							m_strIP(DEFAULT_IP),
							m_nPort(DEFAULT_PORT),
							m_lpfnAcceptEx( NULL ),
							m_pSvrListenSocket( NULL )
{
	m_pThreadPool = CThreadPool::CreateNew();
	m_pContextLocer = CDataLocker::CreateNew(true);
	m_pClientSocketMap = NetPerSocketContextMap::CreateNew();
//	m_pClientContextList = NetPerSocketContextList::CreateNew();
	m_pSvrListenSocket = NULL;
}


CIOCPModelMgr::~CIOCPModelMgr(void)
{
	Cleans();
}

void CIOCPModelMgr::Cleans()
{
	// ȷ����Դ�����ͷ�
	this->Fini();
}

//====================================================================================
//
//				    ϵͳ��ʼ������ֹ
//
//====================================================================================

bool CIOCPModelMgr::IsMgrInit() const
{
	return m_bIsInitMgr;
}

////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
//	����������
bool CIOCPModelMgr::Init()
{
	CHECKF(m_pThreadPool && m_pContextLocer);
	CHECKF(m_pClientSocketMap);
	
	ASSERTNET(m_pContextLocer->Lock());

	m_pThreadPool->SetThreadNumLimit(0, gtool::CpuCoreCount()*WORKER_THREADS_PER_PROCESSOR + 1);

	//��ʼ�����绷��
	IF_NOT(gtool::LoadSocketEnvLib())
	{
		ASSERTNET(m_pContextLocer->UnLock());
		return false;
	}

	BUFFER128 szIpStr = { 0 };
	char *pStrIP = szIpStr;
	if (m_strIP.GetLength() == 0)
	{
		IF_NOT(gtool::GetLocalIp(pStrIP))
		{
			m_pContextLocer->UnLock();
			return false;
		}
	}
	else
	{
		_sntprintf(szIpStr, sizeof(szIpStr) - 1, "%s", m_strIP.c_str());
	}


	IF_NOT(strlen(szIpStr) > 0)
	{
		m_pContextLocer->UnLock();
		return false;
	}

	//��ַ
	m_strIP = szIpStr;


	// ��ʼ���̻߳�����

	// ����ϵͳ�˳����¼�֪ͨ
	//m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// ��ʼ��IOCP
	IF_NOT(InitializeIOCP())
	{
		ASSERTNET(m_pContextLocer->UnLock());
		return false;
	}

	//��ʼ�������߳�
	// ��ʼ��Socket
	IF_NOT(PreBindListenEnv())
	{
		ASSERTNET(m_pContextLocer->UnLock());
		return false;
	}

	//sender�߳�
	gtool::CSmartPtr<CSenderThread> pSenderThread = new CSenderThread(m_bIsThreadWork);
	CHECKF(pSenderThread);
	IF_NOT(m_pThreadPool->AddTask(*pSenderThread, 1, true))
	{
		IocpMgr.Fini();
		return false;
	}
	m_pSenderThread = pSenderThread.pop();

	ASSERTNET(m_pContextLocer->UnLock());

	m_bIsInitMgr = true;
	m_bDestroy = false;

	//��ʼ��

	return true;
}


////////////////////////////////////////////////////////////////////
//	��ʼ����ϵͳ�˳���Ϣ���˳���ɶ˿ں��߳���Դ
void CIOCPModelMgr::Fini()
{
	ASSERTNET(m_pContextLocer);
	m_bIsInitMgr = false;

	if(m_bDestroy)
		return;

	m_bDestroy = true;

	//�����ⲿrecv
	//ASSERTNET(PostStatus2External());
	//�����߳�
	IF_OK(m_pSenderThread)
	{
		if (m_bIsThreadWork)
		{
			THREAD_TRANSFER::Post2Thread(m_pSenderThread, POST_OBJ_TYPE_NETWORK_SENDER, 0, 0, 0);
		}

		//�ȴ��߳̽���
		THREAD_TRANSFER::WaitForFini(m_pSenderThread, POST_OBJ_TYPE_NETWORK_SENDER, false);

		m_pSenderThread = NULL;
	}

	if( m_pSvrListenSocket!=NULL && m_pSvrListenSocket->GetSocket()!=INVALID_SOCKET )
	{
		const I32 nCurThreadNum = m_CurWorkThreadNum;
		for (int i = 0; i < nCurThreadNum; ++i)
		{
			// ֪ͨ���е���ɶ˿ڲ����˳�
			PostQueuedCompletionStatus(m_hIOCompletionPort, 0, (DWORD)EXIT_CODE, NULL);
		}

		//�ȴ��߳�ȫ���˳�
		IF_OK(m_pContextLocer)
		{
			while (true)
			{
				ASSERTNET(Lock());
				ASSERTNET(Wait(1));
				ASSERTNET(Unlock());
				if(m_CurWorkThreadNum<=0)
					break;
			}
		}
		
		//���߳�

		// ����ͻ����б���Ϣ
		this->ClearSocketContainer();

		// �ͷ�������Դ
		this->FreeResource();

		ASSERTNET(gtool::UnLoadSocketEnvLib());

		LOGNET("ֹͣ����");
	}	
}

bool CIOCPModelMgr::IsDestroy() const
{
	return m_bDestroy;
}

bool CIOCPModelMgr::CreateSocket(_PER_SOCKET_CONTEXT& rSocketContext, bool bSetNoBlock)
{
	// Ϊ�Ժ�������Ŀͻ�����׼����Socket( ������봫ͳaccept�������� ) 
	rSocketContext.SetSocketCloseFinal(true);
	rSocketContext.SetSocket(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED));
	IF_NOT(INVALID_SOCKET != rSocketContext.GetSocket())
	{
		LOGNET("��������Accept��Socketʧ�ܣ��������: ", WSAGetLastError());
		return false;
	}

	ASSERTNET(gtool::SetSocketNoBlockMode(rSocketContext.GetSocketObj(), bSetNoBlock));

	return true;
}

////////////////////////////////
// ��ʼ����ɶ˿�
bool CIOCPModelMgr::InitializeIOCP()
{
	CHECKF(m_pThreadPool && !m_hIOCompletionPort);

	// ������һ����ɶ˿�
	m_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0 );

	CHECKF(m_hIOCompletionPort);

	//���ݱ��������������������Ӧ���߳�
	const auto nCoreCount = gtool::CpuCoreCount() * WORKER_THREADS_PER_PROCESSOR;
	for (MYULONG i = 0; i < nCoreCount; ++i)
	{
		gtool::CSmartPtrC<CIocpTask> pIocpTask = CIocpTask::CreateNew();
		IF_NOT(pIocpTask)
			continue;

		pIocpTask->SetArg(this);
		CTaskBase *pTaskBase = pIocpTask.pop();
		IF_NOT(pTaskBase)
			continue;

		ASSERTNET(m_pThreadPool->AddTask(*pTaskBase, 1, true));
	}

	return true;
}


/////////////////////////////////////////////////////////////////
// ��ʼ��Socket
bool CIOCPModelMgr::PreBindListenEnv()
{
	CHECKF(m_pClientSocketMap);

	// AcceptEx �� GetAcceptExSockaddrs ��GUID�����ڵ�������ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;  
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 

	// ��������ַ��Ϣ�����ڰ�Socket
	struct sockaddr_in ServerAddress;

	// �������ڼ�����Socket����Ϣ
	CSmartPtrC<PER_SOCKET_CONTEXT> pListenContext= new PER_SOCKET_CONTEXT;
	CHECKF(pListenContext);

	// ��Ҫʹ���ص�IO�������ʹ��WSASocket������Socket���ſ���֧���ص�IO����
	CHECKF(CreateSocket(*pListenContext));

	// ��Listen Socket������ɶ˿���
	IF_NOT(AssociateWithIOCP(pListenContext))
	{
		RELEASE_SOCKET(pListenContext->GetSocketObj());
		return false;
	}

	// ����ַ��Ϣ
	CHECKF(gtool::FillTcpAddrInfo(ServerAddress, m_strIP.c_str(), m_nPort, AF_INET));                        

	// �󶨵�ַ�Ͷ˿�
	CHECKF(SOCKET_ERROR != bind(pListenContext->GetSocket(), (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)));

	// ��ʼ���м���
	CHECKF(SOCKET_ERROR != listen(pListenContext->GetSocket(), SOMAXCONN));

	// ʹ��AcceptEx��������Ϊ���������WinSock2�淶֮���΢�������ṩ����չ����
	// ������Ҫ�����ȡһ�º�����ָ�룬
	// ��ȡAcceptEx����ָ��
	MYULONG dwBytes = 0;  
	IF_NOT(SOCKET_ERROR != WSAIoctl(
		pListenContext->GetSocket(),
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidAcceptEx, 
		sizeof(GuidAcceptEx), 
		&(m_lpfnAcceptEx),
		sizeof(m_lpfnAcceptEx), 
		&dwBytes, 
		NULL, 
		NULL))  
	{  
		this->FreeResource();
		return false;  
	}  

	// ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
	IF_NOT(SOCKET_ERROR != WSAIoctl(
		pListenContext->GetSocket(),
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs), 
		&m_lpfnGetAcceptExSockAddrs, 
		sizeof(m_lpfnGetAcceptExSockAddrs),   
		&dwBytes, 
		NULL, 
		NULL))  
	{  
		LOGERR("WSAIoctl δ�ܻ�ȡGuidGetAcceptExSockAddrs����ָ�롣�������:", WSAGetLastError());
		this->FreeResource();
		return false; 
	}  

	m_pSvrListenSocket = pListenContext.pop();
	ASSERTNET(m_pSvrListenSocket);

	// ΪAcceptEx ׼��������Ȼ��Ͷ��AcceptEx I/O����
	for( int i=0;i<MAX_POST_ACCEPT;i++ )
	{
		gtool::CSmartPtrC<PER_SOCKET_CONTEXT> pSocketNewListen = new PER_SOCKET_CONTEXT;
		IF_NOT(pSocketNewListen)
			continue;

		CHECKF(PostAccept(*pSocketNewListen));	//��Щsocket����Ҫ����ɶ˿ڣ�����ɶ˿ڼ���accept��socket ���� �͸�socket�󶨵� socket����ACCEPT�¼�
		
		CHECKF(this->AddToSocketMap(pSocketNewListen.pop()));
	}

	LOGNET("Ͷ��", MAX_POST_ACCEPT, " ��AcceptEx�������");
	//this->_ShowMessage( _T("Ͷ�� %d ��AcceptEx�������"),MAX_POST_ACCEPT );

	return true;
}

////////////////////////////////////////////////////////////
//	����ͷŵ�������Դ
void CIOCPModelMgr::FreeResource()
{
	IF_OK(m_pThreadPool)
		m_pThreadPool->Destroy();

	IF_OK(m_pSvrListenSocket)
		m_pSvrListenSocket.Release();

	// �ر�IOCP���
	RELEASE_HANDLE(m_hIOCompletionPort);
}


//====================================================================================
//
//				    Ͷ����ɶ˿�����
//
//====================================================================================

bool CIOCPModelMgr::PostAccept(PER_SOCKET_CONTEXT& rSocketContext, PER_RW_CONTEXT* pIOContext /*= NULL*/)
{
	CHECKF(m_lpfnAcceptEx);
	CHECKF(m_pSvrListenSocket);
	CHECKF(INVALID_SOCKET != m_pSvrListenSocket->GetSocket());
	CHECKF(rSocketContext.m_pListReadContext);

	//IO������
	if (!pIOContext)
	{
		pIOContext = rSocketContext.GetNewReadContext();
	}
	CHECKF(pIOContext);
	CHECKF(rSocketContext.m_pListReadContext->IsExist(pIOContext));	

	// �ص��ṹ������ ϵͳ��д
	DWORD dwBytes = 0;
	pIOContext->m_OpType = ACCEPT_POSTED;
	WSABUF *p_wbuf = &pIOContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIOContext->m_Overlapped;

	// Ϊ�Ժ�������Ŀͻ�����׼����Socket( ������봫ͳaccept�������� ) 	
	if (rSocketContext.GetSocket() == NULL
		|| rSocketContext.GetSocket() == INVALID_SOCKET)
	{
		IF_NOT(CreateSocket(rSocketContext))
		{
			rSocketContext.DelReadContext(pIOContext);
			return false;
		}
	}

	// Ͷ��AcceptEx
	LPFN_ACCEPTEX pAcceptex = (LPFN_ACCEPTEX)m_lpfnAcceptEx;
	if (!(pAcceptex(m_pSvrListenSocket->GetSocket(), rSocketContext.GetSocket(), p_wbuf->buf, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, p_ol)))
	{
		IF_NOT(WSA_IO_PENDING == WSAGetLastError())
		{
			LOGNET("Ͷ�� AcceptEx ����ʧ�ܣ��������: ", WSAGetLastError());

			rSocketContext.DelReadContext(pIOContext);
			return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////

//accept���룬����listensockt�ǰ���ɶ˿ڵģ�ʼ�����ڼ������룬��listensocket������socket��Ϊ�˽�ʡϵͳ����socket��ʱ�䣬�����԰���ɶ˿�
//����Ϊ��Щsocket��Ҫȥ���������д
bool CIOCPModelMgr::DoAccpet( PER_RW_CONTEXT* pIoContext )
{
	CHECKF(pIoContext);
	CHECKF(m_lpfnGetAcceptExSockAddrs);
	PER_SOCKET_CONTEXT *pCurSocketContainer = (PER_SOCKET_CONTEXT *) (pIoContext->m_pParrentObj);
	CHECKF(pCurSocketContainer);

	//׼������
	SOCKADDR_IN* pClientAddr = NULL;
	SOCKADDR_IN* pLocalAddr = NULL;  
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);  

	// 1. ����ȡ������ͻ��˵ĵ�ַ��Ϣ
	LPFN_GETACCEPTEXSOCKADDRS pGetAcceptexSockaddrs = (LPFN_GETACCEPTEXSOCKADDRS)m_lpfnGetAcceptExSockAddrs;
	pGetAcceptexSockaddrs(pIoContext->m_wsaBuf.buf, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&pLocalAddr, &localLen, (LPSOCKADDR*)&pClientAddr, &remoteLen);

	IF_OK(pClientAddr)
	{
		//��ӡ�ͻ�������
		BUFFER128 szBuffer = { 0 };
		szBuffer[0] = 0;
		U16 uPort = 0;
		char *pBuffer = szBuffer;
		gtool::GetAddrInfoFromNetInfo(*pClientAddr, pBuffer, sizeof(szBuffer), uPort);
		LOGNET("�ͻ��� ", szBuffer, ":", uPort, "����.");
	}

	//Ͷ��recv
	ASSERTNET(DoAcceptPostNewRecvSock(pIoContext->GetSocket(), pClientAddr));	

	// �����µ��׽��ֹ���listensocket����Ͷ��accept��������
	pIoContext->ResetBuffer();
	CHECKF(CreateSocket(*pCurSocketContainer));	//��Ҫʹ���µ��׽��� �Ҳ���Ҫ����ɶ˿ڣ�ֻҪ��listensokcet�Ϳ���
	return this->PostAccept(*(pCurSocketContainer), pIoContext);		//�����µ�clientsocket	
}

////////////////////////////////////////////////////////////////////
// Ͷ�ݽ�����������
bool CIOCPModelMgr::PostRecv( PER_RW_CONTEXT* pIoContext )
{
	CHECKF(pIoContext);
	auto sSocket = pIoContext->GetSocket();
	CHECKF(sSocket&&sSocket != INVALID_SOCKET);

	// ��ʼ������
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf   = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_Overlapped;

	pIoContext->ResetBuffer();
	pIoContext->m_OpType = RECV_POSTED;

	// ��ʼ����ɺ󣬣�Ͷ��WSARecv����
	int nBytesRecv = WSARecv( pIoContext->GetSocket(), p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL );

	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	const I32 nLastErr = WSAGetLastError();
	IF_NOT ( (SOCKET_ERROR != nBytesRecv) || (WSA_IO_PENDING == nLastErr) )
	{
		LOGNET("Ͷ��WSARecvʧ�ܣ�nLastErr:", nLastErr,"dest ip:", pIoContext->GetIp(), ":", pIoContext->GetPort());
		return false;
	}
	return true;
}

bool CIOCPModelMgr::PostSend(PER_RW_CONTEXT& rIoContext)
{
	auto sSocket = rIoContext.GetSocket();
	CHECKF(sSocket&&sSocket != INVALID_SOCKET);
	rIoContext.m_wsaBuf.len = rIoContext.m_u64DataToHandle;
	CHECKF(rIoContext.m_wsaBuf.len <= MAX_BUFFER_LEN);

	// ��ʼ������
	DWORD dwFlags = 0;
	DWORD nByteSend = 0;
	WSABUF *p_wbuf = &rIoContext.m_wsaBuf;
	OVERLAPPED *p_ol = &rIoContext.m_Overlapped;
	rIoContext.m_OpType = SEND_POSTED;

	// ��ʼ����ɺ󣬣�Ͷ��WSARecv����
	int nBytesSend = WSASend(rIoContext.GetSocket(), p_wbuf, 1, &nByteSend, dwFlags, p_ol, NULL);

	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	auto nLastErr = WSAGetLastError();
	IF_NOT( (SOCKET_ERROR != nBytesSend)
		|| (WSA_IO_PENDING == nLastErr) )
	{
		LOGNET("Ͷ��WSASendʧ�ܣ�nLastErr:", nLastErr, "dest ip:", rIoContext.GetIp(), ":", rIoContext.GetPort());
		return false;
	}
	return true;
}

// Return value
// If no error occurs, WSAConnect returns zero.Otherwise, it returns SOCKET_ERROR, and a specific error code can be retrieved by calling WSAGetLastError.On a blocking socket, the return value indicates success or failure of the connection attempt.
// With a nonblocking socket, the connection attempt cannot be completed immediately.In this case, WSAConnect will return SOCKET_ERROR, and WSAGetLastError will return WSAEWOULDBLOCK; the application could therefore :
// Use select to determine the completion of the connection request by checking if the socket is writeable.
// If your application is using WSAAsyncSelect to indicate interest in connection events, then your application will receive an FD_CONNECT notification when the connect operation is complete(successful or not).
// If your application is using WSAEventSelect to indicate interest in connection events, then the associated event object will be signaled when the connect operation is complete(successful or not).
// For a nonblocking socket, until the connection attempt completes all subsequent calls to WSAConnect on the same socket will fail with the error code WSAEALREADY.
// If the return error code indicates the connection attempt failed(that is, WSAECONNREFUSED, WSAENETUNREACH, WSAETIMEDOUT) the application can call WSAConnect again for the same socket.
// bool CIOCPModelMgr::PostConnect(PER_RW_CONTEXT* pIoContext)
// {
// 	CHECKF(pIoContext);
// 	auto sSocket = pIoContext->GetSocket();
// 	CHECKF(sSocket&&sSocket != INVALID_SOCKET);
// 
// 	// ��ʼ������
// 	DWORD dwFlags = 0;
// 	DWORD dwBytes = 0;
// 	pIoContext->ResetBuffer();
// 	WSABUF *p_wbuf = &pIoContext->m_wsaBuf;
// 	OVERLAPPED *p_ol = &pIoContext->m_Overlapped;
// 	pIoContext->ResetBuffer();
// 	pIoContext->m_OpType = CONNECT_POSTED;
// 	
// 	int nBytesConnect = WSAConnect(sSocket, (const struct sockaddr *)pIoContext->GetAddr(), sizeof(*pIoContext->GetAddr()), NULL, NULL, NULL, NULL);
// 
// 	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
// 	auto nErrCode = WSAGetLastError();
// 	IF_NOT((SOCKET_ERROR != nBytesConnect) || (WSA_IO_PENDING == nErrCode))
// 	{
// 		LOGNET("PostConnectʧ�ܣ�", "nErrCode=[", nErrCode, "]");
// 		return false;
// 	}
// 	return true;
// }

/////////////////////////////////////////////////////////////////
// ���н��յ����ݵ����ʱ�򣬽��д���
bool CIOCPModelMgr::DoRecv( PER_SOCKET_CONTEXT* pSocketContext, PER_RW_CONTEXT* pIoContext, const U64& nRecvByte, bool& bRecvNetData )
{
	CHECKF(pSocketContext);
	CHECKF(pIoContext);

	//���ݳ���
	pIoContext->m_u64DataToHandle = nRecvByte;

	// �Ȱ���һ�ε�������ʾ���֣�Ȼ�������״̬��������һ��Recv����
	auto nCacheLen = nRecvByte * 2 + 4;
	gtool::CSmartPtrC<CXByte, CXByte, DEL_WAY_DELETE_MULTI> pBuffer = new CXByte[nCacheLen];

	char *pCache = (char *)((void *)pBuffer);
	IF_OK(pBuffer)
	{
		memset(pCache, 0, nCacheLen);
		ASSERTNET(gtool::Conver2HexStr(pIoContext->m_wsaBuf.buf, nRecvByte, pCache, nCacheLen));
	
		//todo
		ASSERTNET(THREAD_TRANSFER::Post2Thread(GetBusinessLogicMainThread()
			, POST_OBJ_TYPE_MAIN_THREAD, (SOCKET_IDD)(pSocketContext), pIoContext->m_wsaBuf.buf, nRecvByte));

	}

	//�����µ�recv����
	auto pNewRecv = pSocketContext->GetNewReadContext();
	CHECKF(pNewRecv);

	// Ȼ��ʼͶ����һ��WSARecv���� ע����һ������
	auto bSucRecv = PostRecv(pNewRecv);
	
	LOGNET("�յ�", (const char *)(pSocketContext->GetAddrInfo()), ":", U16(pSocketContext->GetAddrInfo()), "���ݳ��ȣ�", nRecvByte,"ʮ���������ݣ�", (char *)(pCache), " ��Ϣ�ַ���:", pIoContext->m_wsaBuf.buf);
	
	//����ס����pIoContext

	return bSucRecv;
}


// 
// bool CIOCPModelMgr::DoConnect(PER_SOCKET_CONTEXT* pSocketContext, PER_RW_CONTEXT* pIoContext, const U64& nRecvByte)
// {
// 	CHECKF(pSocketContext);
// 	CHECKF(pIoContext);
// 
// 	// �Ȱ���һ�ε�������ʾ���֣�Ȼ�������״̬��������һ��Recv����
// 	auto nCacheLen = nRecvByte * 2 + 4;
// 	gtool::CSmartPtrC<char, char, DEL_WAY_DELETE_MULTI> pBuffer = new char[nCacheLen];
// 
// 	char *pCache = pBuffer;
// 	IF_OK(pBuffer)
// 	{
// 		memset(pCache, 0, nCacheLen);
// 		ASSERTNET(gtool::Conver2HexStr(pIoContext->m_wsaBuf.buf, nRecvByte, pCache, nCacheLen));
// 	}
// 
// 	LOGNET("DoConnect ���ӳɹ�.", (const char *)(pSocketContext->m_Addrinfo), ":", U16(pSocketContext->m_Addrinfo), "���ݳ��ȣ�", nRecvByte, "ʮ���������ݣ�", (char *)(pCache), " ��Ϣ�ַ���:", pIoContext->m_wsaBuf.buf);
// 
// 	// Ȼ��ʼͶ����һ��WSARecv���� ע����һ������
// 	return true;
// }

bool CIOCPModelMgr::DoSend(PER_SOCKET_CONTEXT& rSocketContext, PER_RW_CONTEXT& rIoContext, const U64& nSendByte)
{
	CHECKF(rSocketContext.m_pListWriteContext);

	// �Ȱ���һ�ε�������ʾ���֣�Ȼ�������״̬��������һ��Recv����
	auto nCacheLen = nSendByte * 2 + 4;
	gtool::CSmartPtrC<CXByte, CXByte, DEL_WAY_DELETE_MULTI> pBuffer = new CXByte[nCacheLen];

	char *pCache =(char*)(void *)pBuffer;
	IF_OK(pBuffer)
	{
		memset(pCache, 0, nCacheLen);
		ASSERTNET(gtool::Conver2HexStr(rIoContext.m_wsaBuf.buf, nSendByte, pCache, nCacheLen));
	}

	LOGNET("�������.", (const char *)(rSocketContext.GetAddrInfo()), ":", U16(rSocketContext.GetAddrInfo())," ���峤�ȣ�", rIoContext.m_u64DataToHandle, " ���ݳ��ȣ�", nSendByte, " ʮ���������ݣ�", (char *)(pCache), " ��Ϣ�ַ���:", rIoContext.m_wsaBuf.buf);

	//У�鷢����ɵĳ��Ⱥͻ�������ݳ��� ���·���
	IF_NOT (rIoContext.m_u64DataToHandle == nSendByte)
	{	
		++rIoContext.m_u16FailHandleCount;
		LOGNET("����������ݳ����뻺�����ݳ��Ȳ�һ�£�", rIoContext.GetIp(), ":"
			, rIoContext.GetPort(), "���ݳ��ȣ�", nSendByte, "ʮ���������ݣ�"
			, (char *)(pCache), " ��Ϣ�ַ���:", rIoContext.m_wsaBuf.buf
			, "����ʧ��. ʧ�ܴ���[", rIoContext.m_u16FailHandleCount, "]");

		IF_NOT(SEND_FALE_COUNT_MAX >= rIoContext.m_u16FailHandleCount)
			return false;

		return PostSend(rIoContext);
	}

	LOGNET("���ͳɹ�.", (const char *)(rSocketContext.GetAddrInfo()), ":", U16(rSocketContext.GetAddrInfo()), "���ݳ��ȣ�", nSendByte, "ʮ���������ݣ�", (char *)(pCache), " ��Ϣ�ַ���:", rIoContext.m_wsaBuf.buf);

	//��send�������Ƴ����������ݼ���postsend
	IF_OK(rSocketContext.m_pListWriteContext->IsExist(&rIoContext))
		rSocketContext.DelWriteContext(&rIoContext);

	if (rSocketContext.m_pListWriteContext->IsEmpty())
		return true;

	auto pNewIoContext = rSocketContext.FrontToSend();
	IF_NOT(pNewIoContext)
		return true;	

	// Ȼ��ʼͶ����һ��WSARecv���� ע����һ������
	return PostSend(*pNewIoContext);
}

/////////////////////////////////////////////////////
// �����(Socket)�󶨵���ɶ˿���
bool CIOCPModelMgr::AssociateWithIOCP( PER_SOCKET_CONTEXT *pContext )
{
	// �����ںͿͻ���ͨ�ŵ�SOCKET�󶨵���ɶ˿���
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pContext->GetSocket(), m_hIOCompletionPort, (DWORD)pContext, 0);

	IF_NOT (hTemp)
	{
		LOGNET(("ִ��CreateIoCompletionPort()���ִ���.������룺"), GetLastError());
		return false;
	}

	return true;
}




//====================================================================================
//
//				    ClientSocketMap ��ز���
//
//====================================================================================


//////////////////////////////////////////////////////////////
// ���ͻ��˵������Ϣ�洢��������
bool CIOCPModelMgr::AddToSocketMap( PER_SOCKET_CONTEXT *pSocketContext )
{
	CHECKF(pSocketContext);
	CHECKF(m_pContextLocer&&m_pClientSocketMap);

	IF_OK(!m_pClientSocketMap->IsExist(U64(pSocketContext)))
	{
		m_pClientSocketMap->insert(std::make_pair(U64(pSocketContext), pSocketContext));
		CHECKF(m_pClientSocketMap->IsExist(U64(pSocketContext)));
	}

	return true;
}

////////////////////////////////////////////////////////////////
//	�Ƴ�ĳ���ض���Context
void CIOCPModelMgr::RemoveFromSocketMap( PER_SOCKET_CONTEXT *pSocketContext )
{
	CHECK(m_pClientSocketMap);
	CHECK(pSocketContext);

	U64 addrSocketContext = U64(pSocketContext);
	ASSERTNET(m_pClientSocketMap->DelObj(addrSocketContext));
}

////////////////////////////////////////////////////////////////
// ��տͻ�����Ϣ
void CIOCPModelMgr::ClearSocketContainer()
{
	IF_OK(m_pClientSocketMap)
		m_pClientSocketMap->Cleans();
}
// 
// bool CIOCPModelMgr::DataSinal()
// {
// 	CHECKF(m_pDataStatusSinal);
// 	return m_pDataStatusSinal->SinalOne();
// }
// 
// bool CIOCPModelMgr::WaitForData(const unsigned long ulMilliseconds /*= INFINITE*/)
// {
// 	CHECKF(m_pDataStatusSinal);
// 	return m_pDataStatusSinal->WaitEvent(ulMilliseconds) != WIN_WAIT_EVENT_FAILED;
// }
// 
// bool CIOCPModelMgr::NetDataLock()
// {
// 	CHECKF(m_pDataStatusSinal);
// 	return m_pDataStatusSinal->Lock();
// }
// 
// bool CIOCPModelMgr::NetDataUnLock()
// {
// 	CHECKF(m_pDataStatusSinal);
// 	return m_pDataStatusSinal->UnLock();
// }


_RW_CONTEXT * CIOCPModelMgr::AddData(const SOCKET_IDD sDest, const void *pData, const U64 nSize, NET_IO_ERR_TYPE& errOut, bool bPutOutErrlog /*= false*/)
{
	CHECKF(m_bIsInitMgr);
	CHECKF(m_pContextLocer);
	CHECKF(m_pClientSocketMap);

	//��ʼ��
	errOut = NET_IO_ERR_TYPE_NONE;

	//�Ƿ����
	auto pSocketContext = m_pClientSocketMap->GetObj(sDest);
	IF_NOT(pSocketContext)
	{
		errOut = NET_IO_ERR_TYPE_SOCKET_NOT_EXIST;

		if (bPutOutErrlog)
			LOGNET("AddData nSize[", nSize, "].", " sDest[", sDest, "] NET_IO_ERR_TYPE_SOCKET_NOT_EXIST");

		return false;
	}

	//�������ܴ���MAX_BUFFER_LEN
	IF_NOT(MAX_BUFFER_LEN >= nSize)
	{
		errOut = NET_IO_ERR_TYPE_PACKET_OVER_MAX;

		if (bPutOutErrlog)
			LOGNET("AddData nSize[", nSize, "].", " sDest[", sDest, "]", pSocketContext->GetIp(), ":", pSocketContext->GetPort(), " NET_IO_ERR_TYPE_PACKET_OVER_MAX");

		return false;
	}

	//����д����
	auto pNewIOContext = pSocketContext->GetNewWriteContext();
	IF_NOT(pNewIOContext)
	{
		errOut = NET_IO_ERR_TYPE_MEMORY_NOT_ENOUGH;

		if (bPutOutErrlog)
			LOGNET("AddData nSize[", nSize, "].", " sDest[", sDest, "]", pSocketContext->GetIp(), ":", pSocketContext->GetPort(), " NET_IO_ERR_TYPE_MEMORY_NOT_ENOUGH");

		return false;
	}

	//����ת�� �Ż���������ָ��ת�ƶ������ݿ���
	pNewIOContext->m_u64DataToHandle = nSize;
	memmove(pNewIOContext->m_szBuffer, pData, pNewIOContext->m_u64DataToHandle);

	return pNewIOContext;		
}

//====================================================================================
//
//				       ����������������
//
//====================================================================================

bool CIOCPModelMgr::SendData(const SOCKET_IDD sDest, _RW_CONTEXT& rDataContext, NET_IO_ERR_TYPE& errOut, bool bPutOutErrlog/*=false*/)
{
	CHECKF(m_bIsInitMgr);
	CHECKF(m_pContextLocer);
	CHECKF(m_pClientSocketMap);

	//��ʼ��
	errOut = NET_IO_ERR_TYPE_NONE;

	//�Ƿ����
	auto pSocketContext = m_pClientSocketMap->GetObj(sDest);
	IF_NOT(pSocketContext)
	{
		errOut = NET_IO_ERR_TYPE_SOCKET_NOT_EXIST;

		if (bPutOutErrlog)
			LOGNET("SendData nSize[", rDataContext.m_u64DataToHandle, "].", " sDest[", sDest, "] NET_IO_ERR_TYPE_SOCKET_NOT_EXIST");

		return false;
	}

	//Ͷ��send
	IF_NOT(PostSend(rDataContext))
	{
		errOut = NET_IO_ERR_TYPE_POST_SEND;

		if (bPutOutErrlog)
			LOGNET("SendData PostSend nSize[", rDataContext.m_u64DataToHandle, "].", " sDest[", sDest, "]", rDataContext.GetIp(), ":", rDataContext.GetPort(), " NET_IO_ERR_TYPE_POST_SEND");

		IF_OK(pSocketContext->m_pListWriteContext->IsExist(&rDataContext))
			pSocketContext->DelWriteContext(&rDataContext);

		return false;
	}

	return true;
}

// nRecvSize ��0���Ի�ȡ���軺���С
// bool CIOCPModelMgr::RecvData(const SOCKET_IDD sDest, void *& pData, U64& nRecvSize)
// {
// 	pData = NULL;
// 	CHECKF(m_bIsInitMgr);
// 	CHECKF(m_pContextLocer);
// 	CHECKF(m_pClientSocketMap);
// 
// 	//�Ƿ����
// 	auto pSocketContext = m_pClientSocketMap->GetObj(sDest);
// 	CHECKF(pSocketContext);
// 
// 	//��û�����ݷ���
// 	auto pIOContext = pSocketContext->GetReadFront();
// 	if (!pIOContext)
// 		return false;
// 
// 	//����ת��
// 	gtool::CSmartPtrC<char, char, DEL_WAY_DELETE_MULTI> pCache = new char[pIOContext->m_u64DataToHandle];
// 	CHECKF(pCache);
// 	nRecvSize = pIOContext->m_u64DataToHandle;
// 	memmove((char *)pCache, pIOContext->m_szBuffer, pIOContext->m_u64DataToHandle);
// 	
// 	//�ͷſռ�
// 	pSocketContext->DelReadContext(pIOContext);
// 
// 	pData = pCache.pop();
// 	ASSERTEX(pData);
// 
// 	return true;
// }

bool CIOCPModelMgr::Close(const SOCKET_IDD idSocket)
{
	CHECKF(idSocket != INVALID_SOCKET_IDD);
	CHECKF(m_bIsInitMgr);
	CHECKF(m_pClientSocketMap);
	CHECKF(m_pContextLocer);

	RemoveFromSocketMap((PER_SOCKET_CONTEXT *)(idSocket));

	return true;
}

void CIOCPModelMgr::GetBindAddrStr(void *pBuffer, const U16 u16SizeBuffer) const
{
	_snprintf((char *)pBuffer, u16SizeBuffer - 1, "%s:%hu", m_strIP.c_str(), m_nPort);
}



////////////////////////////////////////////////////////////////////

bool CIOCPModelMgr::SinalOne()
{
	CHECKF(m_pContextLocer);
	return m_pContextLocer->SinalOne();
}


PER_SOCKET_CONTEXT* CIOCPModelMgr::QueryClientContext(const SOCKET_IDD idSocket)
{
	CHECKF(IS_VALID_SOCKET_IDD(idSocket));
	CHECKF(m_pClientSocketMap);

	return m_pClientSocketMap->GetObj(idSocket);
}

bool CIOCPModelMgr::Wait(const unsigned long ulMilliseconds /*= INFINITE*/)
{
	CHECKF(m_pContextLocer);
	return m_pContextLocer->WaitEvent(ulMilliseconds) != WIN_WAIT_EVENT_FAILED;
}

bool CIOCPModelMgr::Unlock()
{
	CHECKF(m_pContextLocer);
	return m_pContextLocer->UnLock();
}

bool CIOCPModelMgr::Lock()
{
	CHECKF(m_pContextLocer);
	return m_pContextLocer->Lock();
}

bool gtool::CIOCPModelMgr::IsLock()
{
	CHECKF(m_pContextLocer);
	return m_pContextLocer->Islock();
}

///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ���������һ����Ч��Socket��Ͷ��WSARecv����������쳣
// ʹ�õķ����ǳ��������socket�������ݣ��ж����socket���õķ���ֵ
// ��Ϊ����ͻ��������쳣�Ͽ�(����ͻ��˱������߰ε����ߵ�)��ʱ�򣬷����������޷��յ��ͻ��˶Ͽ���֪ͨ��
//��Э������л�
// bool CIOCPModelMgr::IsSocketAlive(PER_SOCKET_CONTEXT& rSocketContext)
// {
// 	ASSERTNET(SetSocketNoBlockMode(rSocketContext.m_Socket));
// 	int nByteSent=send(rSocketContext.m_Socket,"1", 1,0);
// 
// 	if (rSocketContext.IsDetectTimeOut(0, JUDGE_SOCKET_ALIVE_DELAY_TIME))
// 		return false;
// 
// 	return true;
// }

bool CIOCPModelMgr::DoAcceptPostNewRecvSock(const SOCKET sSocket, const SOCKADDR_IN* pClientAddr)
{
	// 2. ������Ҫע�⣬���ﴫ��������ListenSocket�ϵ�Context�����Context���ǻ���Ҫ���ڼ�����һ������
	// �����һ���Ҫ��ListenSocket�ϵ�Context���Ƴ���һ��Ϊ�������Socket�½�һ��SocketContext
	CSmartPtrC<PER_SOCKET_CONTEXT> pNewSocketContext = new PER_SOCKET_CONTEXT;
	CHECKF(pNewSocketContext);
	pNewSocketContext->SetSocket(sSocket);	//�ƽ�socket	���ڶ�д����
	IF_OK(pClientAddr)
		pNewSocketContext->SetAddrInfo(*pClientAddr);
	pNewSocketContext->SetSocketCloseFinal(true);

	// ����������ϣ������Socket����ɶ˿ڰ�(��Ҳ��һ���ؼ�����) ��д������Ҫ����ɶ˿ڣ�accpetֻҪlistensocket����ɶ˿ڼ��������Ĳ���Ҫ
	//acceptֻ��Ҫlistensockt����ɶ˿ڼ��ɣ���listensockt������socket��Ҫ���»ض�дpost��ʱ��Ű󶨶˿����ڼ��������д����
	CHECKF(AssociateWithIOCP(pNewSocketContext));

	// 3. �������������µ�IoContext�����������Socket��Ͷ��һ��Recv��������
	PER_RW_CONTEXT* pNewIoContext = pNewSocketContext->GetNewReadContext();
	CHECKF(pNewIoContext);

	pNewIoContext->m_OpType = RECV_POSTED;

	// �����֮�󣬾Ϳ��Կ�ʼ�����Socket��Ͷ�����������
	CHECKF(PostRecv(pNewIoContext));

	// 4. ���Ͷ�ݳɹ�����ô�Ͱ������Ч�Ŀͻ�����Ϣ�����뵽Socketmap��ȥ(��Ҫͳһ���������ͷ���Դ)
	PER_SOCKET_CONTEXT *pNetContext = NULL;
	IF_NOT(AddToSocketMap(pNetContext = pNewSocketContext.pop()))
	{
		SAFE_DELETE(pNetContext);
		return false;
	}

	return true;
}

// 
// //�׽��ֳ�ʱ���
// bool CIOCPModelMgr::DelayMonitorSocket(long tv_sec, long tv_usec, bool bReadableDetect, bool bWriteableDetect, int *pErrOut, bool bSetOneAtLeast, bool bInfiniteWaiting)
// {
// 	bool bTimeOut = false;
// 
// 	//��������
// 	fd_set *pFd_ReadOpt = NULL;
// 	fd_set *pFd_WriteOpt = NULL;
// 	memset(&m_fs_read, 0, sizeof(m_fs_read));
// 	memset(&m_fs_write, 0, sizeof(m_fs_write));
// 
// 	//�ɶ����
// 	if (bReadableDetect)
// 	{
// 		FD_ZERO(&m_fs_read);
// 		FD_SET(m_nSocket, &m_fs_read);
// 		pFd_ReadOpt = &m_fs_read;
// 	}
// 
// 	//��д���
// 	if (bWriteableDetect)
// 	{
// 		FD_ZERO(&m_fs_write);
// 		FD_SET(m_nSocket, &m_fs_write);
// 		pFd_WriteOpt = &m_fs_write;
// 	}
// 
// 	//��ʱ��ز�����ʼ��
// 	timeval timeout;
// 	memset(&timeout, 0, sizeof(timeout));
// 	timeout.tv_sec = tv_sec;	//���
// 	timeout.tv_usec = tv_usec;
// 
// 	int ret = SOCKET_ERROR;
// 	if (bInfiniteWaiting)
// 	{
// 		ret = select(m_nSocket + 1, pFd_ReadOpt, pFd_WriteOpt, NULL, NULL);	//0��ʾ��ʱ�����򷵻�SOCKET_ERROR ������Ϊ-1ʱ����������������0�� ������Ϊ����ʱ����ʾ�Ѿ�׼���õ�����������
// 	}
// 	else
// 	{
// 		ret = select(m_nSocket + 1, pFd_ReadOpt, pFd_WriteOpt, NULL, &timeout);	//0��ʾ��ʱ�����򷵻�SOCKET_ERROR ������Ϊ-1ʱ����������������0�� ������Ϊ����ʱ����ʾ�Ѿ�׼���õ�����������
// 	}
// 
// 
// 	if (ret == SOCKET_ERROR)
// 	{
// 		if (pErrOut) *pErrOut = SOCKET_ERROR;
// 		if (bReadableDetect) FD_CLR(m_nSocket, pFd_ReadOpt);
// 		if (bWriteableDetect) FD_CLR(m_nSocket, pFd_WriteOpt);
// 		return TRUE;
// 	}
// 	else
// 	{
// 		if (pErrOut) *pErrOut = ret;
// 	}
// 
// 	if (!bSetOneAtLeast)
// 	{
// 		if (bReadableDetect && !bWriteableDetect)
// 		{
// 			// �ж�socket����Ƿ�ɶ�  
// 			if (!FD_ISSET(m_nSocket, &m_fs_read))
// 			{
// 				if (bReadableDetect) FD_CLR(m_nSocket, pFd_ReadOpt);
// 				bTimeOut = TRUE;
// 			}
// 		}
// 		else if (bWriteableDetect && !bReadableDetect)
// 		{
// 			// �ж�socket����Ƿ��д  
// 			if (!FD_ISSET(m_nSocket, &m_fs_write))
// 			{
// 				if (bWriteableDetect) FD_CLR(m_nSocket, pFd_WriteOpt);
// 				bTimeOut = TRUE;
// 			}
// 		}
// 		else if (bWriteableDetect&&bWriteableDetect)
// 		{
// 			if ((!FD_ISSET(m_nSocket, &m_fs_read)) && (!FD_ISSET(m_nSocket, &m_fs_write)))
// 			{
// 				if (bReadableDetect) FD_CLR(m_nSocket, pFd_ReadOpt);
// 				if (bWriteableDetect) FD_CLR(m_nSocket, pFd_WriteOpt);
// 				bTimeOut = TRUE;
// 			}
// 		}
// 	}
// 	else
// 	{
// 		if (bWriteableDetect || bReadableDetect)
// 		{
// 			bTimeOut = FALSE;
// 			// �ж�socket����Ƿ�ɶ�  
// 			if (!FD_ISSET(m_nSocket, &m_fs_read))
// 			{
// 				if (bReadableDetect) FD_CLR(m_nSocket, pFd_ReadOpt);
// 				bTimeOut |= TRUE;
// 			}
// 
// 			// �ж�socket����Ƿ��д  
// 			if (!FD_ISSET(m_nSocket, &m_fs_write))
// 			{
// 				if (bWriteableDetect) FD_CLR(m_nSocket, pFd_WriteOpt);
// 				bTimeOut |= TRUE;
// 			}
// 		}
// 	}
// 
// 
// 	return bTimeOut;
// }

///////////////////////////////////////////////////////////////////
// ��ʾ��������ɶ˿��ϵĴ���
bool CIOCPModelMgr::HandleError( PER_SOCKET_CONTEXT *pSocketContext, OVERLAPPED* & pOverlapped,const DWORD& dwErr )
{
	CHECKF(pSocketContext);

	// ����ǳ�ʱ�ˣ����ټ����Ȱ�  
	if(WAIT_TIMEOUT == dwErr)  
	{  	
		// ȷ�Ͽͻ����Ƿ񻹻���...
// 		IF_NOT(IsSocketAlive(*pContext) )
// 		{
// 			LOGNET(!"��⵽�ͻ����쳣�˳���NotAlive");
// 		}
// 		else
// 		{
// 			LOGNET(!"���������ʱ��������...");
// 			return true;
// 		}
		CHECKF(pOverlapped);
		
		LOGNET("���������ʱ��������..., socket������Ҫ�л�");

		PER_RW_CONTEXT* pIoContext = CONTAINING_RECORD(pOverlapped, PER_RW_CONTEXT, m_Overlapped);
		CHECKF(pIoContext);

		switch (pIoContext->m_OpType)
		{
		case ACCEPT_POSTED:
			{}
			break;
		case SEND_POSTED:
			{
				++pIoContext->m_u16FailHandleCount;
				LOGNET("����������ݳ����뻺�����ݳ��Ȳ�һ�£�", pIoContext->GetIp(), ":"
					, pIoContext->GetPort(), "���ݳ��ȣ�", pIoContext->m_u64DataToHandle, "����ʧ��. ʧ�ܴ���[", pIoContext->m_u16FailHandleCount, "]");

				IF_NOT(SEND_FALE_COUNT_MAX >= pIoContext->m_u16FailHandleCount)
					break;

				PostSend(*pIoContext);
			}
		case RECV_POSTED:
			{}
			break;
		default:
			break;
		}	

		return true;
	}  

	// �����ǿͻ����쳣�˳���
	else if( ERROR_NETNAME_DELETED==dwErr )
	{
		LOGNET("��⵽�ͻ����쳣�˳��� ERROR_NETNAME_DELETED");
	}

	else
	{
		LOGNET("��ɶ˿ڲ������ִ����߳��˳����������:", dwErr );
	}

	return false;
}

NAMESPACE_END(gtool)


