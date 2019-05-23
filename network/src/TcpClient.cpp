#include <Net/TcpClient.h>
#include <Net/NetDataPack.h>
#include <Net/IOCPModel.h>
#include <Net/ClientReciever.h>

#undef SEND_ERR_MAX_COUNT			
#define SEND_ERR_MAX_COUNT			3		//����ʧ��������
// 
// CTcpClient::CTcpClient()
// {
// }
// 
// CTcpClient::~CTcpClient()
// {
// }
// 
// void CTcpClient::Cleans()
// {
// 	ASSERTEX(Close());
// }
// 
// bool CTcpClient::SetAddr(const char *domain, const U16 nPort)
// {
// 	m_addrRemote.SetAddrInfo(AF_INET, NULL, domain, false, &nPort, false);
// 	return true;
// }
// 
// bool CTcpClient::Create()
// {
// 	//socket����
// 	CHECKF(gtool::LoadSocketEnvLib());
// 
// 	//�Ƿ��Ѵ����׽���
// 	CHECKF(!IS_VALID_SOCKET_IDD(m_sRemote));
// 
// 	//����socket
// 	m_sRemote = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
// 	CHECKF(IS_VALID_SOCKET_IDD(m_sRemote));
// 
// 	return true;
// }
// 
// bool CTcpClient::Connect()
// {
// 	//�Ϸ���֤
// 	CHECKF(IS_VALID_SOCKET_IDD(m_sRemote));
// 
// 	const sockaddr_in *sin = m_addrRemote;
// 	CHECKF(sin);
// 
// 	//�л��׽���ģʽ
// 	CHECKF(gtool::SetSocketNoBlockMode(m_sRemote));
// 
// 	//����
// 	bool bConnectTimeOut = true;
// 	WORD i = 0;
// 	for (i = 0; i < TIME_OUT_COUNT; i++)
// 	{
// 		ASSERTEX((SOCKET_ERROR !=connect(m_sRemote, (struct sockaddr *)sin, sizeof(sockaddr_in))) || (WSAGetLastError() != WSAEWOULDBLOCK));
// 
// 		int ret = 1;
// 		if (gtool::IsDetectTimeOut(m_sRemote, m_fs_read, m_fs_write, 0, 500000, false, true, &ret, false)){
// 			continue;	//��ʱΪ500����
// 		}
// 
// 		//�ж��Ƿ����ӳɹ�
// 		int optval = -1;
// 		int optlen = sizeof(optval);
// 		int retget = getsockopt(m_sRemote, SOL_SOCKET, SO_ERROR, (char*)(&optval), &optlen);
// 		if (retget != 0 || optval != 0)
// 			continue;
// 
// 		//��γ�ʱ
// 		if (ret <= 0 && (i == (TIME_OUT_COUNT - 1)))
// 			continue;
// 
// 		//��ͨ
// 		if (ret > 0)
// 		{
// 			bConnectTimeOut = false;
// 			break;
// 		}
// 	}
// 
// 	//�жϳ�ʱ
// 	CHECKF(!bConnectTimeOut);
// 
// 	m_bConnected = true;
// 	return true;
// }
// 
// bool CTcpClient::Send(const void *pData, const I64 nDataSize)
// {
// 	//�Ϸ���У��
// 	CHECKF(m_bConnected);
// 	CHECKF(IS_VALID_SOCKET_IDD(m_sRemote));
// 	CHECKF(pData&&nDataSize > 0);
// 
// 	//��������
// 	I64 nRet = 0;
// 	nRet = send(m_sRemote, (char *)pData, nDataSize, 0);
// 	CHECKF((nRet != -1) && (nRet == nDataSize));
// 
// 	return true;
// }
// 
// bool CTcpClient::Recv(void *pData, I64& nDataSizeOut)
// {
// 	CHECKF(m_bConnected);
// 	CHECKF(IS_VALID_SOCKET_IDD(m_sRemote));
// 
// 	//�Ϸ���֤
// 	CHECKF(pData && (nDataSizeOut > 0));
// 	ZERO_CLEAN_PTR(pData, nDataSizeOut);
// 
// 	//����
// 	I64 nRecvByte = 0;
// 	nRecvByte = recv(m_sRemote, (char *)pData, nDataSizeOut, 0);
// 	CHECKF(nRecvByte <= 0);
// 	nDataSizeOut = nRecvByte;
// 
// 	return true;
// }
// 
// bool CTcpClient::Close()
// {
// 	m_bConnected = false;
// 	if (IS_VALID_SOCKET_IDD(m_sRemote))
// 		return true;
// 
// 	if (closesocket(m_sRemote) != 0)
// 	{
// 		SET_INVALID_SOCKET(m_sRemote);
// 		return false;
// 	}
// 
// 	SET_INVALID_SOCKET(m_sRemote);
// 	return true;
// }
// 
// 
// 
// CClientSocketMgr *g_pClientSocketMgr = CClientSocketMgrInstance::GetInstance();
// 
// CClientSocketMgr::CClientSocketMgr()
// {
// 	//m_pNetDataBufferList = NetDataBufferList::CreateNew();
// 	m_pSendCacheAllNetworkDataCacheMap = AllNetworkDataCacheMap::CreateNew();
// 	m_pSendCacheLocker = CDataLocker::CreateNew();
// 	m_pRecvCacheAllNetworkDataCacheMap = AllNetworkDataCacheMap::CreateNew();
// 	m_pRecvCacheLocker = CDataLocker::CreateNew();
// 	m_pThreadPool = CThreadPool::CreateNew();
// }
// 
// CClientSocketMgr::~CClientSocketMgr()
// {
// }
// 
// void CClientSocketMgr::Cleans()
// {
// }
// 
// #undef ADDCLIENT_SOCKETTASK
// #define ADDCLIENT_SOCKETTASK(classname, status_param)\
// gtool::CSmartPtr<CTaskBase> pTask_##classname = new classname(status_param);\
// CHECKF(pTask_##classname);\
// CHECKF(m_pThreadPool->AddTask(*pTask_##classname, 1, true));\
// pTask_##classname.pop();
// 
// bool CClientSocketMgr::Init()
// {
// //	CHECKF(m_pNetDataBufferList);
// 	CHECKF(m_pThreadPool);
// 	CHECKF(m_pSendCacheAllNetworkDataCacheMap);
// 	CHECKF(m_pSendCacheLocker);
// 	CHECKF(m_pRecvCacheAllNetworkDataCacheMap);
// 	CHECKF(m_pRecvCacheLocker);
// 	m_pThreadPool->SetThreadNumLimit(0, 2);
// 
// 	//��ʼ�����ͺͽ����߳�
// 	ADDCLIENT_SOCKETTASK(CClientSender, m_bSenderThreadWork);
// 	ADDCLIENT_SOCKETTASK(CClientReciever, m_bRecieverThreadWork);
// 	CHECKF(m_bSenderThreadWork);
// 	CHECKF(m_bRecieverThreadWork);
// 
// 	m_bInit = true;
// 	return true;
// }
// 
// bool CClientSocketMgr::Fini()
// {
// 	m_bInit = false;
// 
// 	if (m_bDestroy)
// 		return true;
// 
// 	m_bDestroy = true;
// 
// 	//�����̻߳���
// 	IF_OK(m_pClientSender)
// 	{
// 		if (m_bSenderThreadWork)
// 		{
// 			THREAD_TRANSFER::Post2Thread(m_pClientSender, POST_OBJ_TYPE_NETWORK_CLIENT_SENDER, 0, 0, 0);
// 			THREAD_TRANSFER::WaitForFini(m_pClientSender, POST_OBJ_TYPE_NETWORK_CLIENT_SENDER);
// 		}
// 		m_pClientSender = NULL;
// 	}
// 
// 	//�����̻߳���
// 	IF_OK(m_pClientReciever)
// 	{
// 		if (m_bSenderThreadWork)
// 		{
// 			THREAD_TRANSFER::Post2Thread(m_pClientReciever, POST_OBJ_TYPE_NETWORK_CLIENT_RECIEVER, 0, 0, 0);
// 			THREAD_TRANSFER::WaitForFini(m_pClientReciever, POST_OBJ_TYPE_NETWORK_CLIENT_RECIEVER);
// 		}
// 		m_pClientReciever = NULL;
// 	}
// 
// 	//�����̳߳�
// 	IF_OK(m_pThreadPool)
// 		m_pThreadPool->Destroy();
// 
// 	//��������
// 	IF_OK(m_pSendCacheAllNetworkDataCacheMap)
// 	{
// 		ASSERTNET(m_pSendCacheAllNetworkDataCacheMap->empty());
// 		m_pSendCacheAllNetworkDataCacheMap->Cleans();
// 	}
// 
// 	IF_OK(m_pRecvCacheAllNetworkDataCacheMap)
// 	{
// 		ASSERTNET(m_pRecvCacheAllNetworkDataCacheMap->empty());
// 		m_pRecvCacheAllNetworkDataCacheMap->Cleans();
// 	}
// }
// 
// bool CClientSocketMgr::SendData(const void *pData, const I64 nDataSize)
// {
// 	CHECKF(m_pClientSender);
// 	CHECKF(m_bInit);
// 
// 	//���
// 	CHECKF(IS_VALID_SOCKET_IDD(m_sRemote));
// 	NET_IO_ERR_TYPE eErrType = NET_IO_ERR_TYPE_NONE;
// 
// 	//���
// 	CNetDataPack NetDataPack;
// 	gtool::CRandGen<> gRandor;
// 	++m_nSendPackBatchID;
// 	NetDataPack.Init(pData, nDataSize, true, gtool::GetMilliSecTimestamp(), gRandor
// 		, TRAN_PROTOCOL::TRAN_TYPE_MSG, TRAN_PROTOCOL::PACK_TYPE_SEND, m_nSendPackBatchID);
// 	char *pCache = NULL;
// 	U64 nPackSize = 0;
// 	const auto nTotalNum = NetDataPack.GetCurPackModeTotalNum();
// 
// 	CHECKF(LockSendCache());
// 	while (NetDataPack.PackNext(pCache, nPackSize))
// 	{
// 		IF_NOT(pCache&&nPackSize)
// 			continue;
// 
// 		//ѹ�뻺��
// 		auto pXByteCache = QuerySendCache(m_nSendPackBatchID, NetDataPack.GetCurTranType()
// 			, NetDataPack.GetCurPackModeSeqNum(), true, nPackSize);
// 
// 		CHECKF(pXByteCache);
// 		memmove(pXByteCache, pCache, nPackSize);
// 
// 		auto pMapInfo = QueryPackCacheMapInfo(m_nSendPackBatchID, NetDataPack.GetCurTranType());
// 		IF_NOT(pMapInfo)
// 			continue;
// 
// 		pMapInfo->m_nTotalByte += nPackSize;
// 	}
// 
// 	//post���ݸ����������߳�
// 	THREAD_TRANSFER::Post2Thread(m_pClientSender, POST_OBJ_TYPE_NETWORK_CLIENT_SENDER, m_sRemote, (void *)1, 1);	//1�� 1 ��ʵ���������Ϊ���뻽���߳������֣�client��sender�̵߳� pData��datasize���κ�ʵ������
// 
// 	return true;
// }
// 
// bool CClientSocketMgr::RecvData(void *&pData, I64& nDataSize, bool bSocketBlockMode/*=false*/, bool bNeedSocketBlockSet/*=true*/)
// {
// 	CHECKF(IS_VALID_SOCKET_IDD(m_sRemote));
// //	CHECKF(m_pNetDataBufferList);
// 	CleanResource();
// 
// 	//��������ģʽ
// 	if (bNeedSocketBlockSet)
// 	{
// 		CHECKF(gtool::SetSocketNoBlockMode(m_sRemote, !bSocketBlockMode));
// 	}
// 
// 	I64 nSizeRecvTotalByte = 0;
// 	while (true)
// 	{
// 		//����������
// 		gtool::CSmartPtr<NetDataBuffer> pBuffer = NetDataBuffer::CreateNew();
// 		CHECKF(pBuffer);
// 		I64 nSizeBuffer = sizeof(pBuffer->szBuffer);
// 		if(!Recv(pBuffer->szBuffer, nSizeBuffer))
// 			break;
// 
// 		IF_NOT(nSizeBuffer>0)
// 			continue;
// 
// 		nSizeRecvTotalByte += nSizeBuffer;
// 		pBuffer->nSizeData = nSizeBuffer;
// 
// 		//ѹ�����
// //		ASSERTNET(m_pNetDataBufferList->PushBack(pBuffer.pop()));
// 	}
// 
// //	CHECKF(!m_pNetDataBufferList->IsEmpty());
// 
// 	//��������
// // 	gtool::CSmartPtrC<CXByte, CXByte, DEL_WAY_DELETE_MULTI> pByte = new CXByte[nSizeRecvTotalByte];
// // 	CHECKF(pByte);
// // 	CXByte *pByteTmp = pByte;
// // 	auto IterList = m_pNetDataBufferList->NewEnum();
// // 	while (IterList.Next())
// // 	{
// // 		NetDataBuffer *pBuffer = IterList;
// // 		IF_NOT(pBuffer)
// // 			continue;
// // 
// // 		IF_NOT(pBuffer->nSizeData > 0)
// // 			continue;
// // 			
// // 		memmove(pByteTmp, pBuffer->szBuffer, pBuffer->nSizeData);
// // 		pByteTmp += pBuffer->nSizeData;
// // 	}
// // 	pData = pByte.pop();
// // 	nDataSize = nSizeRecvTotalByte;	
// // 	CHECKF(nDataSize > 0);
// // 	CHECKF(pData);
// 	
// 	return true;
// }
// 
// bool CClientSocketMgr::LockSendCache()
// {
// 	CHECKF(m_pSendCacheLocker);
// 	return m_pSendCacheLocker->Lock();
// }
// 
// TimestampXByte* CClientSocketMgr::QueryTimestampSendStream(const PACK_UNIQUE_ID idPackBatch, TRAN_PROTOCOL::TRAN_TYPE eTransType, const I32 nSeqNum, bool bCreate /*= false*/)
// {
// 	CHECKF(m_pSendCacheAllNetworkDataCacheMap);
// 	auto pTransTypePackCacheMap = m_pSendCacheAllNetworkDataCacheMap->GetObj(idPackBatch);
// 	if (pTransTypePackCacheMap || !bCreate)
// 	{
// 		if (!pTransTypePackCacheMap)
// 			return NULL;
// 
// 		//Э������
// 		PackCacheMapInfo* pMapInfo = pTransTypePackCacheMap->GetObj(eTransType);
// 		if (pMapInfo || !bCreate)
// 		{
// 			if (!pMapInfo)
// 				return NULL;
// 
// 			CHECKF(pMapInfo->m_pXByteStreamMap);
// 
// 			//����������
// 			if (pMapInfo->m_nTotalByte > 0 || !bCreate)
// 			{
// 				if (pMapInfo->m_nTotalByte <= 0)
// 					return NULL;
// 
// 				//���
// 				auto pXByte = pMapInfo->m_pXByteStreamMap->GetObj(nSeqNum);
// 				if (pXByte || !bCreate)
// 				{
// 					if (!pXByte)
// 						return NULL;
// 
// 					return pXByte;
// 				}
// 			}
// 		}
// 	}
// 
// 	//����
// 	if (!pTransTypePackCacheMap)
// 	{
// 		gtool::CSmartPtr<TransTypePackCacheMap> pMap = TransTypePackCacheMap::CreateNew();
// 		CHECKF(pMap);
// 		ASSERTNET(m_pSendCacheAllNetworkDataCacheMap->AddObj(idPackBatch, pMap.pop()));
// 		pTransTypePackCacheMap = m_pSendCacheAllNetworkDataCacheMap->GetObj(idPackBatch);
// 	}
// 	CHECKF(pTransTypePackCacheMap);
// 
// 	//mapinfo
// 	auto pMapInfo = pTransTypePackCacheMap->GetObj(eTransType);
// 	if (!pMapInfo)
// 	{
// 		gtool::CSmartPtr<PackCacheMapInfo> pMapInfoTmp = new PackCacheMapInfo;
// 		CHECKF(pMapInfoTmp);
// 		ASSERTNET(pTransTypePackCacheMap->AddObj(eTransType, pMapInfoTmp.pop()));
// 		pMapInfo = pTransTypePackCacheMap->GetObj(eTransType);
// 	}
// 	CHECKF(pMapInfo);
// 	CHECKF(pMapInfo->m_pXByteStreamMap);
// 	auto &XByteStreamMap = *pMapInfo->m_pXByteStreamMap;
// 
// 	//����ʱ�������� ʱ�����sender�߳���侫ȷ������
// 	auto pXByte = XByteStreamMap.GetObj(nSeqNum);
// 	if (!pXByte)
// 	{
// 		gtool::CSmartPtrC<TimestampXByte, TimestampXByte, DEL_WAY_DELETE> pXByteTmp = new TimestampXByte;
// 		CHECKF(pXByteTmp);
// 		ASSERTNET(XByteStreamMap.AddObj(nSeqNum, pXByteTmp.pop()));
// 		pXByte = XByteStreamMap.GetObj(nSeqNum);
// 	}
// 	CHECKF(pXByte);
// 
// 	return pXByte;
// }
// 
// CXByte* CClientSocketMgr::QuerySendCache(const PACK_UNIQUE_ID idPackBatch, TRAN_PROTOCOL::TRAN_TYPE eTransType, const I32 nSeqNum, bool bCreate /*= false*/, const I64 nCreateByte /*= 0*/)
// {
// 	auto pTimestampXByte = QueryTimestampSendStream(idPackBatch, eTransType, nSeqNum, false);
// 	if (pTimestampXByte || !bCreate)
// 	{
// 		if (!pTimestampXByte)
// 			return NULL;
// 
// 		if (pTimestampXByte->pdata_stream || !bCreate)
// 		{
// 			if (!pTimestampXByte->pdata_stream)
// 				return NULL;
// 
// 			IF_NOT(pTimestampXByte->nSizeStream == nCreateByte)
// 			{
// 				pTimestampXByte->pdata_stream.Release();
// 			}
// 			pTimestampXByte->nSizeStream = nCreateByte;
// 			pTimestampXByte->nTimestamp = 0;
// 
// 			return pTimestampXByte->pdata_stream;
// 		}			
// 	}
// 
// 	CHECKF(nCreateByte);
// 	pTimestampXByte = QueryTimestampSendStream(idPackBatch, eTransType, nSeqNum, true);
// 	CHECKF(pTimestampXByte);
// 	IF_NOT(!pTimestampXByte->pdata_stream)
// 	{
// 		pTimestampXByte->pdata_stream.Release();
// 	}
// 	pTimestampXByte->nSizeStream = 0;
// 	pTimestampXByte->nTimestamp = 0;
// 
// 	pTimestampXByte->pdata_stream = new CXByte[nCreateByte];
// 	CHECKF(pTimestampXByte->pdata_stream);
// 	pTimestampXByte->nSizeStream = nCreateByte;
// 
// 	return pTimestampXByte->pdata_stream;
// }
// 
// PackCacheMapInfo* CClientSocketMgr::QueryPackCacheMapInfo(const PACK_UNIQUE_ID idPackBatch, TRAN_PROTOCOL::TRAN_TYPE eTransType)
// {
// 	CHECKF(m_pSendCacheAllNetworkDataCacheMap);
// 	auto pTransTypePackCacheMap = m_pSendCacheAllNetworkDataCacheMap->GetObj(idPackBatch);
// 	if (!pTransTypePackCacheMap)
// 		return NULL;
// 
// 	return pTransTypePackCacheMap->GetObj(eTransType);
// }
// 
// bool CClientSocketMgr::UnlockSendCache()
// {
// 	CHECKF(m_pSendCacheLocker);
// 	return m_pSendCacheLocker->UnLock();
// }
// 
// bool CClientSocketMgr::DividePack(void *&pData, I64&nDataSize)
// {
// 
// }
// 
// void CClientSocketMgr::CleanResource()
// {
// // 	IF_OK(m_pNetDataBufferList)
// // 		m_pNetDataBufferList->Clear();
// }



/////////////////////////////////////////////////////


////////////////////////////////////////////

CTcpSocket::CTcpSocket()
{
	m_bConnected = FALSE;
	m_nSocket = INVALID_SOCKET;
	m_bAutoCloseSocket = true;
	m_bSocketReadable = false;
	m_bSocketWriteable = false;
	m_pLocker = CDataLocker::CreateNew();
}

CTcpSocket:: ~CTcpSocket()
{

}

void CTcpSocket::Cleans()
{
	if (m_bAutoCloseSocket)
		Close();
}

//����socket����
bool CTcpSocket::InitSocketEnvironment()
{
	try
	{
		if (!gtool::IsLoadSocketEnvLib())
		{
			gtool::LoadSocketEnvLib();
		}
	}
	catch (...)
	{
	}

	return TRUE;
}

//������
bool CTcpSocket::CleanSocketEnvironment()
{
	int iRet = 0;
	try
	{
		if (gtool::IsLoadSocketEnvLib())
		{
			gtool::UnLoadSocketEnvLib();
		}
	}
	catch (...)
	{
		iRet = -1;
	}


	//һ�����̵���һ�ξ͹�����ʹ�ù����б�����������쳣

	return iRet == 0 ? true : false;
}

//�׽���
bool CTcpSocket::CreateSocket()
{
	//�ȶϿ�
	Close();

	m_nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);			//�����׽���
	IF_NOT(IS_VALID_SOCKET_IDD(m_nSocket))
	{
		m_bConnected = false;
		return false;
	}

	m_bSocketReadable = true;
	m_bSocketWriteable = true;

	return true;
}


//�Ͽ�����
bool CTcpSocket::Close()
{
	m_bConnected = false;
	m_bSocketReadable = false;
	m_bSocketWriteable = false;

	SOCKET nSock = m_nSocket;
	m_nSocket = INVALID_SOCKET;
	if(!IS_VALID_SOCKET_IDD(nSock))
		return true;

	CHECKF(closesocket(nSock) == 0);
	return true;
}

//���ŵĶϿ�����
bool CTcpSocket::GraciousClose(int iHowto)
{
	if (m_nSocket <0 || INVALID_SOCKET == m_nSocket) return true;

	if (shutdown(m_nSocket, iHowto) != 0)
	{
		return false;
	}

	switch (iHowto)
	{
	case SD_RECEIVE:
		{
			m_bSocketReadable = false;
			break;
		}
	case SD_SEND:
		{
			m_bSocketWriteable = false;
			break;
		}
	case SD_BOTH:
		{
			m_bSocketReadable = false;
			m_bSocketWriteable = false;
			break;
		}
	}


	return true;
}

//��������
bool CTcpSocket::SendData(void *pData, long *plDataLen)
{
	CHECKF(IS_VALID_SOCKET_IDD(m_nSocket));
	CHECKF(pData&&plDataLen);

	//���ɷ���
	if (!m_bSocketWriteable) return false;

	//��������
	long lSendRet = 0;
	lSendRet = send(m_nSocket, (char *)pData, *plDataLen, 0);
	if (lSendRet == -1)
	{
		*plDataLen = -1;
		return false;
	}
	else if (lSendRet<*plDataLen)
	{
		*plDataLen = lSendRet;
		return false;
	}

	*plDataLen = lSendRet;

	//����ͳ��
	//m_DataFlowCount.lSendTotal += lSendRet;
	return true;
}

//��������
bool CTcpSocket::RecvData(void *pData, long *plDataLen)
{
	CHECKF(IS_VALID_SOCKET_IDD(m_nSocket));
	CHECKF(pData&&plDataLen);

	//���ɽ���
	CHECKF (m_bSocketReadable);

	//����
	long lRecvByte = 0;

	//��������
	memset(pData, 0, *plDataLen);
	lRecvByte = recv(m_nSocket, (char *)pData, *plDataLen, 0);
	if (lRecvByte <= 0)
	{
		//Close();
		*plDataLen = lRecvByte;
		return false;
	}

	//���
	*plDataLen = lRecvByte;

	//����ͳ��
	//m_DataFlowCount.lRecvTotal += lRecvByte;
	return true;
}


bool CTcpSocket::Init()
{
	CHECKF(m_pLocker);

	return true;
}

//���õ�ַ
void CTcpSocket::SetDestAddr(void *pAddr)
{
	if (pAddr)
		SetAddrInfo(*((CAddr *)pAddr));
}

//���õ�ַ
bool CTcpSocket::SetAddrInfo(CAddr &addr)
{
	m_SocketAddr.Reset();
	m_SocketAddr = addr;

	return	true;
}

//���õ�ַ
bool CTcpSocket::SetAddrInfo(const char *domain, int domainlen, unsigned short port, unsigned short family, bool bUseBind, bool bSocketStream)
{
	m_SocketAddr.Reset();
	m_SocketAddr.SetAddrInfo(family, NULL, domain, false, &port, bUseBind, bSocketStream);
	return true;
}

//���õ�ַ
bool CTcpSocket::SetAddrInfo(const char *ip, int iplen, unsigned short port, unsigned short family, bool bUseBind, bool bSocketStream, bool bIp)
{
	m_SocketAddr.Reset();
	m_SocketAddr.SetAddrInfo(family, ip, NULL, true, &port, bUseBind, bSocketStream);

	return	true;
}

//����
VOID CTcpSocket::Clear()
{
	if (IS_VALID_SOCKET_IDD(m_nSocket)) Close();
	m_nSocket = INVALID_SOCKET;
	m_SocketAddr.Reset();
	m_bConnected = false;
}

//����ģʽ
bool CTcpSocket::SetBlockMode()
{
	SOCKET sSocket = m_nSocket;
	return gtool::SetSocketNoBlockMode(sSocket, false);
}

//������ģʽ
bool CTcpSocket::SetNoBlockMode()
{
	SOCKET sSocket = m_nSocket;
	return gtool::SetSocketNoBlockMode(sSocket, true);
}

//�׽��ֳ�ʱ���
bool CTcpSocket::DelayMonitorSocket(long tv_sec, long tv_usec, bool bReadableDetect, bool bWriteableDetect, int *pErrOut, bool bSetOneAtLeast, bool bInfiniteWaiting)
{
	SOCKET sSocket = m_nSocket;
	return gtool::IsDetectTimeOut(sSocket, m_fs_read, m_fs_write, tv_sec, tv_usec, bReadableDetect, bWriteableDetect, pErrOut, bSetOneAtLeast, bInfiniteWaiting);

}

//���socket������
bool CTcpSocket::ClearSocketCache()
{
	//��������
	char clr[8] = { 0 };
	long lClrLen = 1;
	int iErr = 0;

	while (true)
	{
		memset(clr, 0, sizeof(clr));
		if (DelayMonitorSocket(1, 0, true, false, &iErr))
		{
			//�쳣
			if (iErr < 0) return false;

			break;
		}

		RecvData(clr, &lClrLen);
	}

	return TRUE;
}

//�����׽���
VOID CTcpSocket::SetSocketFD(SOCKET hSock)
{
	if (IS_VALID_SOCKET_IDD(hSock))
	{
		m_bSocketReadable = true;
		m_bSocketWriteable = true;
	}
	else
	{
		m_bSocketReadable = false;
		m_bSocketWriteable = false;
	}

	m_nSocket = hSock;
}

//��ȡ�Է�ip�˿�
int CTcpSocket::GetPeerIP(char *peerip, int iPeerIPSize, unsigned short *peerport)
{
	if (!peerip || !peerport || iPeerIPSize <= 0) return -1;
	return gtool::GetPeerAddr(m_nSocket, peerip, iPeerIPSize, *peerport);
}

//��ȡ���ص�ַ�˿�
int CTcpSocket::GetLocalIP(char *localipaddr, int ipaddrsize, unsigned short *localport)
{
	IF_NOT(localipaddr&&localport && (ipaddrsize > 0))
		return -1;

	int ret = 0;
	struct sockaddr_in dstadd_in;
	socklen_t  len = sizeof(struct sockaddr_in);
	memset(&dstadd_in, 0, sizeof(struct sockaddr_in));
	if (ret = getsockname(m_nSocket, (struct sockaddr*)&dstadd_in, &len) < 0)
	{
		return WSAGetLastError();
		//err_err("getpeername()");
	}
	else
	{
		if (ipaddrsize < strlen(inet_ntoa(dstadd_in.sin_addr))) return -1;
		sprintf(localipaddr, "%s", inet_ntoa(dstadd_in.sin_addr));
		*localport = ntohs(dstadd_in.sin_port);
	}
	return ret;
}


///////////////////////////////////////////////////////////////////////////
//��ʼ��socket����
CNetworkService::CNetworkService()
{
	//m_BusinessLayerCache = NULL;
	m_ServerPort = 0;
	m_bIsDomainName = false;
	m_bSetTransferSecurity = false;
}

CNetworkService::~CNetworkService()
{
	//��ն���
	ClearAllDataCache();
}

void CNetworkService::Cleans()
{
	ClearAllDataCache();
}

//�����������
bool CNetworkService::StartNetService(const bool bClearDataCache /*= true*/)
{
	//��ն���
	if (bClearDataCache)
		ASSERTEX(ClearAllDataCache());

	//����socket
	return CreateSocket();
}



//�Ͽ�����
bool CNetworkService::CloseNetService()
{
	ASSERTEX(Close());

	//��ն���
	ASSERTEX(ClearAllDataCache());

	return true;
}

//ҵ���߼����������
bool CNetworkService::AddDataToSend(const void *pData, const long lDataLen, BusinessDataCacheList& rCacheList, I64& nBusinessTotalSize)
{
	//����У��
	CHECKF(pData);

	//��������

	//�Ƿ����ð�ȫ����
	if (m_bSetTransferSecurity)
	{
		CHECKF(false);
// 		//����
// 		//��Կkey
// 		BYTE pcbKey[DES_KEY_BIT + 1] = { 0 };
// 		BYTE cbKeyCipher[RSA_KEY_STR_LEN / 8] = { 0 };	//key����
// 		BYTE *pKeyCipherTmp = cbKeyCipher;				//key����
// 		unsigned long KeyCipherLen = RSA_KEY_STR_LEN / 8;
// 		DWORD dwKeyLen = DES_KEY_BIT + 1;
// 		DWORD dwDataLenTmp = lDataLen;		//���ĳ���
// 		CDESEngine DesSystem;
// 		CRsaEngine RsaSystem;
// 		//����rsa��Կ
// 		RsaSystem.SetPublicKey((unsigned char *)m_strRsaPublicKey.c_str());
// 		DWORD dwNeeded = DesSystem.EncrypteMemoryNeeded((BYTE *)pcbKey, &dwKeyLen, (BYTE *)pData, dwDataLenTmp);
// 		//int iKeyLentmpd = dwKeyLen;
// 		RsaSystem.Encrypte(pcbKey, dwKeyLen, &pKeyCipherTmp, &KeyCipherLen);
// 		pCache->pData = new char[dwNeeded + KeyCipherLen];
// 		if (!pCache->pData)
// 		{
// 			delete pCache;
// 			return FALSE;
// 		}
// 		memset(pCache->pData, 0, dwNeeded + KeyCipherLen);
// 		DesSystem.Encrypte(pcbKey, &dwKeyLen, (BYTE *)pData, dwDataLenTmp, (BYTE *)pCache->pData, &dwNeeded);	//ҵ�������
// 		unsigned char *pDataTmp = (unsigned char *)pCache->pData;
// 		pDataTmp += dwNeeded;									//ƫ��
// 		memmove(pDataTmp, pKeyCipherTmp, KeyCipherLen);			//���ܺ����Կ
// 		pCache->lDataLen = dwNeeded + KeyCipherLen;				//ʵ�ʼ��ܺ�ҵ��㳤��
	}
	else
	{
		const auto nCacheSize = sizeof(TRAN_PROTOCOL::BusinessDataCache) + lDataLen;
		gtool::CSmartPtrC<CUXByte, CUXByte, DEL_WAY_DELETE_MULTI> pBusinessDataCache = new CUXByte[nCacheSize];
		TRAN_PROTOCOL::NetCache NetCachePtr(pBusinessDataCache);
		CHECKF(NetCachePtr.m_ptr);
		ZERO_CLEAN_PTR(pBusinessDataCache, nCacheSize);
		memmove(NetCachePtr.m_pBusinessDataCache->rData, pData, lDataLen);
		NetCachePtr.m_pBusinessDataCache->nLen = lDataLen;
		CHECKF(rCacheList.PushBack(NetCachePtr.m_pUXByte));
		pBusinessDataCache.pop();
		nBusinessTotalSize += lDataLen;
	}


	//��־
	//char rizhi[8192] = { 0 };
	//	sprintf(rizhi, "ҵ������ģ�pCache->pData:%s, ���ȣ�%d;����ҵ�����ݳ��ȣ�%d\r\n���ܺ����Կ��%s, ���ȣ�%d\r\n����ǰ����Կ��%s, ���ȣ�%d;\r\n", pDataTmp, pCache->lDataLen, dwNeeded, pKeyCipherTmp, KeyCipherLen, pcbKey, iKeyLentmpd);
	//CWriteDailyLog::WriteLog(rizhi, "yewuceng.txt");

	//��������
	//pCache->pData = new char[lDataLen];
	//if (!pCache->pData)
	//{
	//	delete pCache;
	//	return FALSE;
	//}
	//memset(pCache->pData, 0, lDataLen);
	//memmove(pCache->pData, pData, lDataLen);
	//pCache->lDataLen = lDataLen;

	//TCHAR szRizhi[100] = { 0 };
	//TCHAR szResult[200] = { 0 };
	//CWriteDailyLog::AnsiToUnicode((char *)pCache->pData, szRizhi, 100*sizeof(TCHAR));
	//_sntprintf(szResult, 200, TEXT("%s, len:%d, %d"), szRizhi, pCache->lDataLen, m_lCacheDataSize);
	//MessageBox(NULL, szResult, NULL, 0);
	//CWriteDailyLog::WriteDialyLog((char *)pCache->pData);

	//����
	//if (!DataEncrypt(pData, &lDataLen, &pCache->pData, &pCache->lDataLen))
	//{
	//	if (pCache) delete pCache;
	//	return FALSE;
	//}

	return true;
}

//������
bool CNetworkService::CreateSocketPacketToSend(BusinessDataCacheMap& rSendMap, BusinessDataCacheList& rCacheList, const I64 nTotalSize, long &lPackCount)
{
	gtool::CSmartPtr<CNetDataPack> pNetDataPack = CNetDataPack::CreateNew();
	CHECKF(pNetDataPack);

	//��������
	long lPackCountTmp = 0;


	//���ռ�
	gtool::CSmartPtrC<CUXByte, CUXByte, DEL_WAY_DELETE_MULTI> pCacheTmp = new CUXByte[nTotalSize];
	CHECKF(pCacheTmp);
	TRAN_PROTOCOL::NetCache rNetCachePtr(pCacheTmp);
	CHECKF(rNetCachePtr.m_ptr);

	//�ӻ�����ת������
	auto IterList = rCacheList.NewEnum();
	while (IterList.Next())
	{
		CUXByte* pCacheData = IterList;
		IF_NOT(pCacheData)
			continue;

		TRAN_PROTOCOL::NetCache rListDataTmp(pCacheData);
		IF_NOT(rListDataTmp.m_ptr)
			continue;

		memmove(rNetCachePtr.m_pUXByte, rListDataTmp.m_pBusinessDataCache->rData, rListDataTmp.m_pBusinessDataCache->nLen);
		rNetCachePtr.m_pUXByte += rListDataTmp.m_pBusinessDataCache->nLen;
	}
	rCacheList.Clear();
	rNetCachePtr.m_ptr = pCacheTmp;

	const auto nTimestamp = 0;
	gtool::CRandGen<U64> rRandNum;
	pNetDataPack->ResetBuffer();
	CHECKF(pNetDataPack->Init(rNetCachePtr.m_ptr, nTotalSize, true, gtool::GetMilliSecTimestamp(), rRandNum, TRAN_PROTOCOL::TRAN_TYPE_MSG, TRAN_PROTOCOL::PACK_TYPE_SEND, m_nPackBatch));

	//ѭ����
	BUFFER2048 szCacheData = { 0 };
	U64 nSizeDataCache = sizeof(szCacheData);
	while (pNetDataPack->PackNext(szCacheData, nSizeDataCache))
	{

		//һ�������ѹ�������������
		gtool::CSmartPtrC<CUXByte, CUXByte, DEL_WAY_DELETE_MULTI> pCacheMove = new CUXByte[nSizeDataCache];
		CHECKF(pCacheMove);
		TRAN_PROTOCOL::NetCache rMoveNetCache(pCacheMove);
		rMoveNetCache.m_pBusinessDataCache->nLen = nSizeDataCache;
		memmove(rMoveNetCache.m_pBusinessDataCache->rData, szCacheData, nSizeDataCache);
		CHECKF(rSendMap.AddObj(pNetDataPack->GetCurPackModeSeqNum(), rMoveNetCache.m_pUXByte));
		ASSERTEX(pCacheMove.pop());
	}

	//������
	lPackCount = rSendMap.size();

	m_nPackBatch++;

	return true;
}

//�������ݰ�
bool CNetworkService::SendDataToDest(BusinessDataCacheList& rCacheList, const I64 nTotalSize, const bool bSocketBlockMode /*= false*/, const bool bNeedSocketBlockSet /*= true*/, const bool bNeedReturnPacket /*= true*/, const long lTvPerSec /*= 1*/, const long lTvPerUsec /*= 0*/, const int iTimeOutCountSet /*= TIME_OUT_COUNT*/)
{
	//����У��
	//�Ƿ��Ѿ�����socket
	CHECKF(IS_VALID_SOCKET_IDD(m_nSocket));
	long lPackCount = 0;

	gtool::CSmartPtr<BusinessDataCacheMap> pCacheMap = BusinessDataCacheMap::CreateNew();
	CHECKF(pCacheMap);

	//��������װ��(����ҵ�������)
	CHECKF (CreateSocketPacketToSend(*pCacheMap, rCacheList,nTotalSize,lPackCount));

	//��������Ҫ����
	CHECKF(!pCacheMap->empty());

	//��������ģʽ
	if (bNeedSocketBlockSet)
	{
		if (bSocketBlockMode)
		{
			CHECKF (SetBlockMode());
		}
		else
		{
			CHECKF (SetNoBlockMode());
		}
	}


	//��������
	int iTimeOutCount = iTimeOutCountSet;
	bool bRecvReturn = false;

	//����
	BusinessDataCacheMap::iterator IterMap = pCacheMap->begin();
	for (IterMap = pCacheMap->begin(); IterMap != pCacheMap->end(); ++IterMap)
	{
		//ȡ��
		TRAN_PROTOCOL::NetCache rCache(IterMap->second);
		CHECKF(rCache.m_ptr);

		//��ʱ��������
		//iTimeOutCount = iTimeOutCountSet;
		iTimeOutCount = 1;
		bRecvReturn = false;

		//����һ����			
		while ((iTimeOutCount--)>0)
		{
			bRecvReturn = false;

			//����
			long nSendSize = rCache.m_pBusinessDataCache->nLen;
			CHECKF(SendData(rCache.m_pBusinessDataCache->rData, &nSendSize));

			//�Ƿ���Ҫ�ɹ����հ�Ӧ��
			if (!bNeedReturnPacket) break;

			//��ʱ�ȴ�	500ms
			if (DelayMonitorSocket(0, 500000)) continue;

			if (bRecvReturn = IsDestRecvSuccess(IterMap->first, bSocketBlockMode, !bNeedSocketBlockSet))
			{
				break;
			}
		}

		//����ʧ���ж�
		IF_NOT(iTimeOutCount > 0 || bRecvReturn || !bNeedReturnPacket)
			return false;
	}

	return true;

	//(�Ӵ���������ȡһ��������send������ˣ��ȴ�����˷��ؽ������Ӧ���ٷ���һ��������1s��δ�ظ��ط������ѭ��ֱ���Ѱ����꣬�����ն���)
	//��������
}

//�Է��Ƿ���ճɹ�ȷ��
bool CNetworkService::IsDestRecvSuccess(/*��ȷ�ϵİ����*/long lCurrentPackSeq, const bool bSocketBlockMode, const bool bNeedSocketBlockSet)
{
	//�ɹ�����Ӧ������ܺ� FUNID_RIGHT_RECV
	TRAN_PROTOCOL::NetCache rNetCache(NULL);
	long lRecvPackSize = 0;
	bool bLastAReplyPack = false;

	gtool::CSmartPtr<BusinessDataCacheMap> pDataCacheMap = BusinessDataCacheMap::CreateNew();
	gtool::CSmartPtr<BusinessDataCacheList> pDataOutCacheList = BusinessDataCacheList::CreateNew();
	I64 nRecvTotalSize = 0;
	CHECKF(pDataOutCacheList);
	CHECKF(pDataCacheMap);

	CHECKF (RecvDataFromSrc(*pDataOutCacheList,*pDataCacheMap, bLastAReplyPack, nRecvTotalSize, bSocketBlockMode, bNeedSocketBlockSet, false));

	if (!bLastAReplyPack)
	{
		IF_NOT(GetOutRecvData(*pDataOutCacheList, rNetCache.m_ptr, lRecvPackSize, nRecvTotalSize))
		{
			SAFE_DELETE_MULTI(rNetCache.m_pUXByte);
			rNetCache.m_ptr = NULL;
			return bLastAReplyPack;
		}

		if (lRecvPackSize)
			CHECKF(rNetCache.m_ptr);
	}


	//�ж�
	SAFE_DELETE_MULTI(rNetCache.m_pUXByte);

	return bLastAReplyPack;
}

//�������ݰ�
//1.��һ�����ݰ�����������������ݰ���
//2.��λ��ͷ��β��һ������ͷ�����ֽ�Ϊ��ͷ��һ�����ĺ������ֽ�Ϊ��β��
//3.��λ�����ݳ��ȣ������յ��ֽ���Ӧ����֮�����(�ϰ�����Ȳ�����գ����г�ʱ���յȴ�,���޶�ʱ����δ���յ���������Ͽ����ӽ���ʧ�ܲ���������)
//4.��λ����ź��ܰ��������ڼ������մ�����
//5.ά��һ��ȥ�ذ����У��ѽ��յİ�����Ҫ�ٽ��գ�����ʾ���ճɹ�

//ѹ������ջ����������ؽ��ճɹ�Ӧ���

//�˳�while
//�����հ������ԣ����ĸ�����
//���в����ƴ��ҵ��㣬ƴ����ɺ����ҵ��㣬����ȴ�����ҵ��㻺����
//��ɽ���
bool CNetworkService::RecvDataFromSrc(BusinessDataCacheList&rDataOutList, BusinessDataCacheMap&rCacheMap, bool& bLastPackReplyPack, I64&rRecvCacheDataSize, const bool bSocketBlockMode /*= false*/, const bool bNeedSocketBlockSet /*= false*/, const bool bNeedSendSuccessRecvReturnPack /*= TRUE*/, const long lTvPerSec /*= 1*/, const long lTvPerUsec /*= 0*/,const int iTimeOutCountSet /*= TIME_OUT_COUNT*/)
{
	CHECKF(IS_VALID_SOCKET_IDD(m_nSocket));
	gtool::CSmartPtr<CNetDataPack> pNetDataPack = CNetDataPack::CreateNew();
	CHECKF(pNetDataPack);

	//��������ģʽ
	if (bNeedSocketBlockSet)
	{
		if (bSocketBlockMode)
		{
			CHECKF (SetBlockMode());
		}
		else
		{
			CHECKF (SetNoBlockMode());
		}
	}


	//���հ�
	BUFFER2048 szBufferRecv = { 0 };
	long lRecvDataLen = 0;
	bool bRecvSuccess = false;
	int iCountTimeOut = iTimeOutCountSet;
	//	int iCountSuccessRecv = 0;		//�ɹ���������
	unsigned long ulPacketNum = 0;
	unsigned long ulPacketSeq = 0;

	CNetDataPack rDataPackReply;
	BUFFER2048 szReplyBuffer = { 0 };
	gtool::CRandGen<U64> gRandor;


	rRecvCacheDataSize = 0;		//��

	long lCountSuccess = 0;
	long lCountLen = 0;
	TRAN_PROTOCOL::NetCache rNetCache(NULL);
	while (iCountTimeOut-->0)
	{
		//����
		lRecvDataLen = PACKAGE_SIZE_MAX;
		bRecvSuccess = false;
		ulPacketNum = 0;
		ulPacketSeq = 0;
		ZERO_CLEAN_PTR(szBufferRecv, sizeof(szBufferRecv));

		//��ʱ����
		if (DelayMonitorSocket(lTvPerSec, lTvPerUsec)) continue;
		bRecvSuccess = RecvData(szBufferRecv, &lRecvDataLen);
		IF_NOT (bRecvSuccess) continue;

		IF_NOT(pNetDataPack->Init(szBufferRecv, lRecvDataLen, false))
			continue;

		U64 nSizeRecvOut = 0;
		TRAN_PROTOCOL::PackAccess *pAsOut = NULL;
		I64 nBatchKey = 0;
		rNetCache.m_ptr = NULL;	//���ǻظ�ȷ�ϰ���û������
		IF_NOT(pNetDataPack->Unpack(rNetCache.m_ptr, nSizeRecvOut, pAsOut, nBatchKey))
			continue;

		IF_NOT(pAsOut)
			continue;

		//IF_NOT(nSizeRecvOut)
		//	continue;

		//У�������
		//�����سɹ�Ӧ����ͻ��˻ᳬʱ�ط�
		IF_NOT (pAsOut->nPackSize == lRecvDataLen) continue;

		//������
		ulPacketNum = pAsOut->nTotalNum;

		//�����
		ulPacketSeq = pAsOut->nSeqNum;

		//����Ƿ��ذ�
		auto pDataCacheToRecv = rCacheMap.GetObj(ulPacketSeq);
		IF_NOT(!pDataCacheToRecv)
		{
			if (bNeedSendSuccessRecvReturnPack)
			{
				rDataPackReply.ResetBuffer();
				IF_NOT(rDataPackReply.Init(NULL, 0, true, gtool::GetMilliSecTimestamp(), gRandor, TRAN_PROTOCOL::TRAN_TYPE_MSG, TRAN_PROTOCOL::PACK_TYPE_REPLY, pAsOut->nPackBatchID))
					continue;
				
				U64 u64Size2Reply = 0;
				u64Size2Reply = sizeof(szReplyBuffer);
				IF_NOT(rDataPackReply.PackNext(szReplyBuffer, u64Size2Reply))
					continue;
				
				gtool::CSmartPtr<BusinessDataCacheList> pCacheList = BusinessDataCacheList::CreateNew();	//ҵ������ݻ���
				CHECKF(pCacheList);

				I64 nBusinessTotalSize = 0;
				IF_NOT(AddDataToSend(szReplyBuffer, u64Size2Reply, *pCacheList, nBusinessTotalSize))
						continue;

				IF_NOT(SendDataToDest(*pCacheList, nBusinessTotalSize, false, !bNeedSocketBlockSet, false))
					continue;
			}

			//��λ��ʱ����
			iCountTimeOut = iTimeOutCountSet;
			continue;
		}

		//���ռ䱣��ҵ���߼�������
		const I32 nSaveSize = sizeof(TRAN_PROTOCOL::BusinessDataCache) + nSizeRecvOut;
		gtool::CSmartPtrC<CUXByte, CUXByte, DEL_WAY_DELETE_MULTI> pDataCacheSave = new CUXByte[nSaveSize];
		CHECKF(pDataCacheSave);
		TRAN_PROTOCOL::NetCache rNetDataSaveCache(pDataCacheSave);
		CHECKF(rNetDataSaveCache.m_ptr);
		ZERO_CLEAN_PTR(rNetDataSaveCache.m_ptr, nSaveSize);
		rNetDataSaveCache.m_pBusinessDataCache->nLen = nSizeRecvOut;
		if(nSizeRecvOut)
			memmove(rNetDataSaveCache.m_pBusinessDataCache->rData, rNetCache.m_ptr, nSizeRecvOut);
		bLastPackReplyPack = (pAsOut->nPackType == TRAN_PROTOCOL::PACK_TYPE_REPLY);

		//ѹ�������socket����
		IF_OK(!rCacheMap.IsExist(ulPacketSeq))
		{
			rRecvCacheDataSize += rNetDataSaveCache.m_pBusinessDataCache->nLen;
			IF_OK(rCacheMap.AddObj(ulPacketSeq, rNetDataSaveCache.m_pUXByte))
			{
				ASSERTEX(pDataCacheSave.pop());

				bool  bRet = false;
				if (bNeedSendSuccessRecvReturnPack)
				{
					rDataPackReply.ResetBuffer();
					IF_NOT(rDataPackReply.Init(NULL, 0, true, gtool::GetMilliSecTimestamp(), gRandor, TRAN_PROTOCOL::TRAN_TYPE_MSG, TRAN_PROTOCOL::PACK_TYPE_REPLY, pAsOut->nPackBatchID))
						continue;

					U64 u64Size2Reply = 0;
					u64Size2Reply = sizeof(szReplyBuffer);
					IF_NOT(rDataPackReply.PackNext(szReplyBuffer, u64Size2Reply))
						continue;

					gtool::CSmartPtr<BusinessDataCacheList> pCacheList = BusinessDataCacheList::CreateNew();	//ҵ������ݻ���
					CHECKF(pCacheList);

					I64 nBusinessTotalSize = 0;

					IF_NOT(AddDataToSend(szReplyBuffer, u64Size2Reply, *pCacheList, nBusinessTotalSize))
						continue;

					IF_NOT(SendDataToDest(*pCacheList, nBusinessTotalSize, false, !bNeedSocketBlockSet, false))
						continue;
				}

				//���ճɹ�����
				iCountTimeOut = iTimeOutCountSet;
			}

			//����Ƿ�������
			if (rCacheMap.size() == ulPacketNum)
				break;
		}
	}

	CHECKF(bRecvSuccess);

	//ƴ�ӽ��յ���ҵ�������
	IF_NOT(CreateBussnessLayerToGet(rDataOutList, rCacheMap, rRecvCacheDataSize))
	{
		rCacheMap.Cleans();
		return false;
	}

	//����socket�ȴ����ջ�����
	//	ClearSocketPackCacheToRecv();

	return true;
}

//���(������)��socket�����ջ���������ƴ�ӳ�ҵ������� �õ�һ��ҵ���߼�������
bool CNetworkService::CreateBussnessLayerToGet(BusinessDataCacheList&rDataOutList, BusinessDataCacheMap&rCacheMap, I64&rRecvCacheDataSize)
{
	//����У��
	if (rRecvCacheDataSize <= 0)
	{
		rCacheMap.Cleans();
		return true;
	}
	CHECKF(!rCacheMap.empty() && rRecvCacheDataSize);

	//���ռ�
	gtool::CSmartPtrC<CUXByte, CUXByte, DEL_WAY_DELETE_MULTI> pBusinessLayerData = new CUXByte[rRecvCacheDataSize];
	CHECKF(pBusinessLayerData);
	TRAN_PROTOCOL::NetCache rNetCache(pBusinessLayerData);
	CHECKF(rNetCache.m_ptr);
	ZERO_CLEAN_PTR(pBusinessLayerData, rRecvCacheDataSize);

	//��˳��ȡ���� size����0��ʼ��size
	long lBusinessTotalSize = 0;
	for (BusinessDataCacheMap::iterator IterMap = rCacheMap.begin(); IterMap != rCacheMap.end(); ++IterMap)
	{
		CUXByte *pCacheTmp = IterMap->second;
		CHECKF(pCacheTmp);
		TRAN_PROTOCOL::NetCache rCacheTmp(pCacheTmp);
		CHECKF(rCacheTmp.m_ptr);
		if(rCacheTmp.m_pBusinessDataCache->nLen)
			memmove(rNetCache.m_ptr, rCacheTmp.m_pBusinessDataCache->rData, rCacheTmp.m_pBusinessDataCache->nLen);
		rNetCache.m_pUXByte += rCacheTmp.m_pBusinessDataCache->nLen;
		lBusinessTotalSize += rCacheTmp.m_pBusinessDataCache->nLen;
	}
	rCacheMap.Cleans();

	//У������������
	IF_NOT(lBusinessTotalSize == rRecvCacheDataSize)
		return false;

	if (m_bSetTransferSecurity)
	{
		CHECKF(false);
// 		//����
// 		CRsaEngine RsaSystem;
// 		//����˽Կ
// 		RsaSystem.SetPrivateKey((unsigned char *)m_strRsaPrivateKey.c_str());
// 		CDESEngine DesSystem;
// 		//Ѱ����Կ ���� CIPHER_TEXT_OUT_ENCODE_LEN ����
// 		BYTE *pcbKeyCipher = (BYTE *)(((char *)pBusinessLayerData) + m_lRecvCacheDataSize - CIPHER_TEXT_OUT_ENCODE_LEN);
// 		BYTE cbKeyPlain[DES_KEY_BIT + 1] = { 0 };
// 		BYTE *pcbKeyPlainTmp = cbKeyPlain;
// 		unsigned long ulKeyPlainSize = DES_KEY_BIT + 1;
// 		//������Կ
// 		RsaSystem.Decrypte(pcbKeyCipher, CIPHER_TEXT_OUT_ENCODE_LEN, &pcbKeyPlainTmp, &ulKeyPlainSize);
// 
// 		//���ռ�Ž��ܺ��������
// 		unsigned char *pPlainDataTmp = new unsigned char[m_lRecvCacheDataSize - CIPHER_TEXT_OUT_ENCODE_LEN];
// 		if (!pPlainDataTmp)
// 		{
// 			delete[]pBusinessLayerData;
// 			return FALSE;
// 		}
// 		memset(pPlainDataTmp, 0, m_lRecvCacheDataSize - CIPHER_TEXT_OUT_ENCODE_LEN);
// 		DWORD dwPlainDataLen = m_lRecvCacheDataSize - CIPHER_TEXT_OUT_ENCODE_LEN;
// 
// 		//��������
// 		DesSystem.Decrypte(cbKeyPlain, ulKeyPlainSize, (BYTE *)pBusinessLayerData, m_lRecvCacheDataSize - CIPHER_TEXT_OUT_ENCODE_LEN, pPlainDataTmp, &dwPlainDataLen);
// 
// 		//���ռ��Ž��ܺ������
// 		pBusinessDataCache->pData = new char[dwPlainDataLen];
// 		if (!pBusinessDataCache->pData)
// 		{
// 			delete[]pBusinessLayerData;
// 			delete[]pPlainDataTmp;
// 			return FALSE;
// 		}
// 		memset(pBusinessDataCache->pData, 0, dwPlainDataLen);
// 		memmove(pBusinessDataCache->pData, pPlainDataTmp, dwPlainDataLen);
// 		pBusinessDataCache->lDataLen = dwPlainDataLen;
// 		delete[]pPlainDataTmp;
	}
	else
	{
		//�޼���
// 		pBusinessDataCache->pData = new char[m_lRecvCacheDataSize];
// 		if (!pBusinessDataCache->pData)
// 		{
// 			delete[]pBusinessLayerData;
// 			return FALSE;
// 		}
// 		memset(pBusinessDataCache->pData, 0, m_lRecvCacheDataSize);
// 		memmove(pBusinessDataCache->pData, pBusinessLayerData, m_lRecvCacheDataSize);
// 		pBusinessDataCache->lDataLen = m_lRecvCacheDataSize;
	}
	rNetCache.m_ptr = pBusinessLayerData;

	//����������ҵ��㻺����
	IF_OK(rNetCache.m_ptr)
	{
		CHECKF(rDataOutList.PushBack(rNetCache.m_pUXByte));
		ASSERTEX(pBusinessLayerData.pop());
	}

	return true;
}

//ȡ�����յ�������
bool CNetworkService::GetOutRecvData(BusinessDataCacheList&rDataOutList, void *&pData, long &lDataLen, I64&rRecvCacheDataSize)
{
	//����У��
	if (rRecvCacheDataSize <= 0)
	{
		rDataOutList.Clear();
		return true;
	}		

	CHECKF(!rDataOutList.IsEmpty());
	pData = new CUXByte[rRecvCacheDataSize];
	CHECKF(pData);
	lDataLen = 0;

	//ȡ����
	//���ռ�
	//����Ǩ��
	//����ռ�
	//���
	TRAN_PROTOCOL::NetCache rCachePtr(pData);
	CHECKF(rCachePtr.m_ptr);
	auto IterList = rDataOutList.NewEnum();
	while (IterList.Next())
	{
		CUXByte* pDataCache = IterList;
		CHECKF(pDataCache);
		TRAN_PROTOCOL::NetCache rDataCache(pDataCache);
		memmove(rCachePtr.m_ptr, rDataCache.m_pBusinessDataCache->rData, rDataCache.m_pBusinessDataCache->nLen);
		rCachePtr.m_pUXByte += rDataCache.m_pBusinessDataCache->nLen;
		lDataLen += rDataCache.m_pBusinessDataCache->nLen;
	}
	rDataOutList.Clear();
	return true;
}

//���ù�Կ
void CNetworkService::SetRsaPublicKey(unsigned char *ucKey)
{
	if (!ucKey) return;
	m_bSetTransferSecurity = true;
	m_strRsaPublicKey.clear();
	m_strRsaPublicKey = (char *)ucKey;
}

//����˽Կ
void CNetworkService::SetRsaPrivateKey(unsigned char *ucKey)
{
	if (!ucKey) return;
	m_bSetTransferSecurity = true;
	m_strRsaPrivateKey.clear();
	m_strRsaPrivateKey = (char *)ucKey;
}


//��ջ��������ݶ��� �������������
bool CNetworkService::ClearAllDataCache()
{
	m_bIsDomainName = true;


	return true;
}

/////////////////////////////////////////////////////////////
//tcp�ͻ���
CNetworkTcpClient *g_pTcpClientMgr = NULL;
boost::atomic_bool CNetworkTcpClient::m_bDestroy{ false };
CNetworkTcpClient::CNetworkTcpClient()
{
	m_pThreadPool = CThreadPool::CreateNew(0, 1);
}

CNetworkTcpClient::~CNetworkTcpClient()
{
}

void CNetworkTcpClient::Cleans()
{
}


//���ӷ�����
bool CNetworkTcpClient::Link(const bool bClearDataCache)
{
	//�����׽���
	//����

	//�����������
	CHECKF(StartNetService(bClearDataCache));

	return Connect();
}

//�ر�����
bool CNetworkTcpClient::CloseLink()
{
	return CloseNetService();
}

//���������������
bool CNetworkTcpClient::SendDataTo(const void *pData, const long lDataLen)
{
	//����У��
	CHECKF(pData);

	gtool::CSmartPtr<BusinessDataCacheList> pCacheList = BusinessDataCacheList::CreateNew();	//ҵ������ݻ���
	CHECKF(pCacheList);

	I64 nBusinessTotalSize = 0;
	CHECKF(AddDataToSend(pData, lDataLen, *pCacheList, nBusinessTotalSize));

	bool bSend = SendDataToDest(*pCacheList, nBusinessTotalSize,false, true, true);

	return bSend;
}

//�ӷ�������ȡ����
bool CNetworkTcpClient::RecvDataFrom(void *&pData, long &lDataLen)
{
	//����У��
	bool bLastAReplyPack = false;
	gtool::CSmartPtr<BusinessDataCacheMap> pDataCacheMap = BusinessDataCacheMap::CreateNew();
	gtool::CSmartPtr<BusinessDataCacheList> pDataOutCacheList = BusinessDataCacheList::CreateNew();
	I64 nRecvTotalSize = 0;
	CHECKF(pDataOutCacheList);
	CHECKF(pDataCacheMap);
	CHECKF(RecvDataFromSrc(*pDataOutCacheList, *pDataCacheMap,bLastAReplyPack, nRecvTotalSize));

	return GetOutRecvData(*pDataOutCacheList, pData, lDataLen, nRecvTotalSize);
}

//�������ã�����Ͽ����ӣ�
bool CNetworkTcpClient::RefreshSvrAddr(const char *host/*����*/, int hostlen/*����*/, unsigned short port/*�˿�*/, bool bIsDomainName/*�Ƿ�����*/)
{
	CHECKF(host&&hostlen > 0);
	if (!m_strServerAddr.empty()) m_strServerAddr.clear();
	ASSERTEX(CloseNetService());
	m_strServerAddr = host;
	m_ServerPort = port;
	m_bIsDomainName = bIsDomainName;
	
	if (m_bIsDomainName)
	{
		CHECKF(SetAddrInfo(host, strlen(host), port, AF_INET, true, true));
	}
	else
	{
		CHECKF(SetAddrInfo(host, strlen(host), port, AF_INET, true, true, true));
	}	

	return true;
}


void CNetworkTcpClient::GetSvrAddr(void *pBuffer, const U16 nBufferSize)
{
	CHECK(pBuffer&&nBufferSize);
	const char *ip = m_SocketAddr;
	const U16 nPort = m_SocketAddr;
	_snprintf((char *)pBuffer, nBufferSize - 1, "%s:%hu", ip, nPort);
}

bool CNetworkTcpClient::Init()
{
	CHECKF(CTcpSocket::Init());
	CHECKF(InitSocketEnvironment());

	m_bInit = true;
	m_bDestroy = false;
	return true;
}

void CNetworkTcpClient::Fini()
{
	m_bInit = false;
	if (m_bDestroy)
		return;

	m_bDestroy = true;

	//�����̳߳�
	IF_OK(m_pThreadPool)
		m_pThreadPool->Destroy();

	//�ر��׽���
	CloseLink();
}

bool CNetworkTcpClient::StartMonitor()
{
	CHECKF(m_pThreadPool);
	CHECKF(m_bInit);

	gtool::CSmartPtr<CTaskBase> pMonitorTask = CTcpClientReciever::CreateNew();
	CHECKF(pMonitorTask);
	CHECKF(m_pThreadPool->AddTask(*pMonitorTask));
	ASSERTEX(pMonitorTask.pop());

	return true;
}

bool CNetworkTcpClient::Lock()
{
	CHECKF(m_pLocker);
	return m_pLocker->Lock();
}

bool CNetworkTcpClient::Unlock()
{
	CHECKF(m_pLocker);
	return m_pLocker->UnLock();
}

//����Զ��
bool CNetworkTcpClient::Connect()
{
	//�Ϸ���֤
	CHECKF(IS_VALID_SOCKET_IDD(m_nSocket));
	sockaddr_in *sin = (m_SocketAddr);
	CHECKF(sin);

	//�л��׽���ģʽ
	CHECKF(SetNoBlockMode());

	//����
	bool bConnectTimeOut = true;
	WORD i = 0;
	for (i = 0; i < TIME_OUT_COUNT; i++)
	{
		if (!(SOCKET_ERROR == connect(m_nSocket, (struct sockaddr *)sin, sizeof(sockaddr_in)) && WSAGetLastError() == WSAEWOULDBLOCK))
		{
		}

		int ret = 1;
		bConnectTimeOut = true;
		if (DelayMonitorSocket(0, 500000, false, true, &ret, false))
			continue;	//��ʱΪ500����

		//�ж��Ƿ����ӳɹ�
		int optval = -1;
		int optlen = sizeof(optval);
		int retget = getsockopt(m_nSocket, SOL_SOCKET, SO_ERROR, (char*)(&optval), &optlen);
		if (retget != 0 || optval != 0)
		{
			bConnectTimeOut = true;
		}

		if (ret <= 0 && (i == (TIME_OUT_COUNT - 1)))
			break;
			
		if (ret > 0)
		{
			bConnectTimeOut = false;
			break;
		}
	}

	//�л��׽���ģʽ
	//SetBlockMode();

	//�жϳ�ʱ
	if (bConnectTimeOut)
	{
		if (m_bAutoCloseSocket)
			Close();

		return false;
	}

	m_bConnected = true;

	return m_bConnected;
}

/////////////////////////////////////////////////////////////
