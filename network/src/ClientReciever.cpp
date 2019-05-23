#include <Net/ClientReciever.h>
#include <Net/TcpClient.h>
#include <BaseCode/TaskBase.h>

MEMPOOL_CREATE_IMPLEMENTATION(CTcpClientReciever, m_MemObj)

CTcpClientReciever::CTcpClientReciever()
{
}

CTcpClientReciever::~CTcpClientReciever()
{
}

void CTcpClientReciever::Cleans()
{
}

//���������ӷ�����
int CTcpClientReciever::Run()
{
	//���ɶ�
	SOCKET nSock = INVALID_SOCKET;
	I32 nErr = 0;
	
	
	CHECKF(TcpClientMgr.IsInit());
	CHECKF(TcpClientMgr.GetConnectStatus());

	TRAN_PROTOCOL::NetCache rNetCache;
	long nSizeBuffer = 0;
	BUFFER1024 szBuffer = { 0 };
	BUFFER512 szAddr = { 0 };
	gtool::CFmtIOString rIoStr;
	while (!TcpClientMgr.IsDestroy())
	{
		nSock = TcpClientMgr.GetSocketFD();
		IF_NOT(IS_VALID_SOCKET_IDD(nSock))
			continue;

		//��ȡ����	
		TcpClientMgr.Lock();
		auto&rReadFd = TcpClientMgr.GetReadFdSet();
		auto&rWriteFd = TcpClientMgr.GetWriteFdSet();
		if (gtool::IsDetectTimeOut(nSock, rReadFd, rWriteFd, 0, 5000, true, false, &nErr))
		{
			TcpClientMgr.Unlock();
			continue;
		}

		IF_NOT(TcpClientMgr.RecvDataFrom(rNetCache.m_ptr, nSizeBuffer))
		{
			TcpClientMgr.Unlock();
			continue;
		}
		TcpClientMgr.Unlock();			
		
		//����
		if (rNetCache.m_ptr)
		{
			rIoStr.Clear();
			ZERO_CLEAN_PTR(szAddr, sizeof(szAddr));
			TcpClientMgr.GetSvrAddr(szAddr, sizeof(szAddr));
			ZERO_CLEAN_PTR(szBuffer, sizeof(szBuffer));
			_snprintf((char *)szBuffer, sizeof(szBuffer) - 1, "�յ����ݣ�[%s] ", szAddr);
			rIoStr << szBuffer;
			ZERO_CLEAN_PTR(szBuffer, sizeof(szBuffer));
			memmove(szBuffer, rNetCache.m_pstr, nSizeBuffer);
			SAFE_DELETE_MULTI(rNetCache.m_pstr);
			rIoStr << szBuffer;

			LOGSYS(rIoStr.c_str());
		}
	}

	return 0;
	
	//��ȡ����
	//����
}
