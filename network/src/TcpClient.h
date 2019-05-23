#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#pragma once


#undef TIME_OUT_COUNT
#define TIME_OUT_COUNT				10		//��ʱ������ÿ��һ�룬һ��ȴ����ظ�������



/////////////////////////////////////////////////////////////////////////////

//WSAStartup��Ŀ���ǳ�ʼ��winsock DLL��ÿ��Ӧ�ó������һ�ξ����˰ɣ�����ÿ���̵߳���һ�ΰ�
//�����ɹ�����ֵΪ0; ���򷵻�ֵΪSOCKET_ERROR������ͨ������WSAGetLastError��ȡ������롣

//��һ�����̵߳Ļ����£�WSACleanup()��ֹ��Windows Sockets�������߳��ϵĲ���.
//�����ַ��Ϣ�ṹ��
class  CTcpSocket
{
	NO_COPY(CTcpSocket);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CTcpSocket);

	//�ӿڹ���
public:
	virtual bool Init();
	//���õ�ַ
	virtual void SetDestAddr(void *pAddr);
	//����
	virtual bool SendDataTo(const void *pData, const long lDataLen) { return false; }
	//����
	virtual bool RecvDataFrom(void **pData, long *plDataLen) { return false; }

	//��������
public:
	//���õ�ַ
	bool SetAddrInfo(CAddr &addr);
	//���õ�ַ
	bool SetAddrInfo(const char *domain, int domainlen, unsigned short port, unsigned short family, bool bUseBind, bool bSocketStream);
	//���õ�ַ
	bool SetAddrInfo(const char *ip, int iplen, unsigned short port, unsigned short family, bool bUseBind, bool bSocketStream, bool bIp);
	//���socket������
	bool ClearSocketCache();
	//�����׽���
	void SetSocketFD(SOCKET hSock);
	//��ȡ�׽���
	SOCKET GetSocketFD() { return m_nSocket; }
	//�����Զ��ر��׽���ģʽ
	void SetAutoCloseSocket(bool bAutoClose) { m_bAutoCloseSocket = bAutoClose; }
	//����ͳ��
	//TRAN_PROTOCOL::DATA_FLOW_COUNT_T &GetFlowCount() { return m_DataFlowCount; }

	//peer����ȡ����
public:
	//��ȡ�Է�ip�˿�
	int GetPeerIP(char *peerip, int iPeerIPSize, unsigned short *peerport);
	//��ȡ���ص�ַ�˿� 0��ʾ�ɹ� ����Ϊ����ֵ
	int GetLocalIP(char *localipaddr, int ipaddrsize, unsigned short *localport);

	//win32dll ���绷����ʼ��
public:
	//����socket����
	static bool InitSocketEnvironment();
	//������
	static bool CleanSocketEnvironment();

	//�׽��ֲ���
protected:
	//����ģʽ
	bool SetBlockMode();
	//������ģʽ
	bool SetNoBlockMode();
	//�׽��ֳ�ʱ���
	bool DelayMonitorSocket(long tv_sec, long tv_usec, bool bReadableDetect = true, bool bWriteableDetect = false, int *pErrOut = NULL, bool bSetOneAtLeast = false, bool bInfiniteWaiting = false);
	//����
	VOID Clear();

public:
	//����״̬
	bool GetConnectStatus() { return m_bConnected; }
	fd_set&GetReadFdSet() { return m_fs_read; }
	fd_set&GetWriteFdSet() { return m_fs_write; }

	//�������
protected:
	//�׽���
	virtual bool CreateSocket();

	//�Ͽ�����
	virtual bool Close();
	//���ŵĶϿ�����
	virtual bool GraciousClose(int iHowto);
	//��������
	virtual bool SendData(void *pData, long *plDataLen);
	//��������
	virtual bool RecvData(void *pData, long *plDataLen);

	//��������״̬
	bool GetNetConnectStatus() { return m_bConnected; }



protected:

																	//��Ҫ����
protected:
	boost::atomic<SOCKET>		m_nSocket{INVALID_SOCKET};			//�׽���
	boost::atomic_bool			m_bAutoCloseSocket{ true };			//�Զ��ر��׽���
	boost::atomic_bool			m_bConnected{ false };				//����״̬
	fd_set						m_fs_read;							//�ɶ����
	fd_set						m_fs_write;							//��д���
	boost::atomic_bool			m_bSocketReadable{ false };			//�ɶ�
	boost::atomic_bool			m_bSocketWriteable{ false };		//��д
	CAddr						m_SocketAddr;						//��ַ
	gtool::CSmartPtr<CDataLocker> m_pLocker;						//������

																	//����ͳ��
protected:
//	boost::atomic<TRAN_PROTOCOL::DATA_FLOW_COUNT_T>		m_DataFlowCount;					//����ͳ��

};

////////////////////////////////////////////////////////////////


//������� ֻ����һ����
class CNetworkService :public CTcpSocket
{
	NO_COPY(CNetworkService);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CNetworkService);
public:
	typedef gtool::TPtrList<CUXByte, DEL_WAY_DELETE_MULTI, false> BusinessDataCacheList;
	typedef gtool::CLitPtrMap<CUXByte, I32, DEL_WAY_DELETE_MULTI> BusinessDataCacheMap;	//key:nSeqNum �����

	//�ⲿ�ӿ�
public:
	//��ȫ����ʹ��
	void SetTransferSecurity(bool bSet) { m_bSetTransferSecurity = bSet; }
	//��ȫ��������
	bool GetTransferSecurityStatus() { return m_bSetTransferSecurity; }

	//�ڲ��ӿ�

	//����������
protected:
	//�����������
	virtual bool StartNetService(const bool bClearDataCache = true);

	//�Ͽ�����
	virtual bool	CloseNetService();

	//����ģ��
protected:
	//����ģ��
	//ҵ���߼���������� ���֮ǰ����ҵ�������
	virtual bool AddDataToSend(const void *pData, const long lDataLen, BusinessDataCacheList& rCacheList, I64& nBusinessTotalSize);
	//�������ݰ�
	virtual bool SendDataToDest(BusinessDataCacheList& rCacheList, const I64 nTotalSize, const bool bSocketBlockMode = false, const bool bNeedSocketBlockSet = true, const bool bNeedReturnPacket = true,const long lTvPerSec = 1, const long lTvPerUsec = 0, const int iTimeOutCountSet = TIME_OUT_COUNT);
	//�Է��Ƿ���ճɹ�ȷ��
	virtual bool IsDestRecvSuccess(/*��ȷ�ϵİ����*/long lCurrentPackSeq, const bool bSocketBlockMode, const bool bNeedSocketBlockSet);
	//������ һ�����е�ҵ��������� BUSINESS_DATA_LIMIT����
	virtual bool CreateSocketPacketToSend(BusinessDataCacheMap& rSendMap, BusinessDataCacheList& rCacheList, const I64 nTotalSize, long &lPackCount);

	//����ģ��
protected:
	//ȡ�����յ�������
	virtual bool GetOutRecvData(BusinessDataCacheList&rDataOutList, void *&pData, long &lDataLen, I64&rRecvCacheDataSize);
	//�������ݰ�
	virtual bool RecvDataFromSrc(BusinessDataCacheList&rDataOutList, BusinessDataCacheMap&rCacheMap, bool& bLastPackReplyPack, I64&rRecvCacheDataSize, const bool bSocketBlockMode = false, const bool bNeedSocketBlockSet = false, const bool bNeedSendSuccessRecvReturnPack = TRUE,const long lTvPerSec = 1,const long lTvPerUsec = 0, const int iTimeOutCountSet = TIME_OUT_COUNT);
	//���(������)��socket�����ջ���������ƴ�ӳ�ҵ������� �õ�һ��ҵ���߼�������
	virtual bool CreateBussnessLayerToGet(BusinessDataCacheList&rDataOutList, BusinessDataCacheMap&rCacheMap, I64&rRecvCacheDataSize);

	//rsa key����
protected:
	//���ù�Կ
	void SetRsaPublicKey(unsigned char *ucKey);
	//����˽Կ
	void SetRsaPrivateKey(unsigned char *ucKey);

	//������
protected:
	bool ClearAllDataCache();

	//rsa��Կ����
protected:
	std::string					m_strRsaPublicKey;				//��Կ
	std::string					m_strRsaPrivateKey;				//˽Կ
	bool					m_bSetTransferSecurity;			//�Ƿ����ô��䰲ȫ

															//ҵ�����������ݻ����� 1��
protected:
//	long					m_lCacheDataSize;				//ҵ���߼��������ܳ���
	boost::atomic<I64>		m_nPackBatch{ 0 };				//������
//	gtool::CSmartPtr<CNetDataPack> m_pNetDataPack;			//�����


//	gtool::CSmartPtr<BusinessDataCacheList> m_pBusinessDataCacheList;	//ҵ������ݻ���

															//ҵ�����������ݻ�����	2��
protected:
//	long					m_lRecvCacheDataSize;				//ҵ���߼��������ܳ���
//	gtool::CSmartPtr<BusinessDataCacheList> m_pRecvCacheDataQueue;	//ҵ������ݻ���		
	//CUXByte *					m_pRecvDataOut;						//ȡ������������Ҫ�ͷ� Ҫ����

																//socket������ 3��
protected:

//	gtool::CSmartPtr<BusinessDataCacheMap> m_pBusinessDataCacheToSendMap;	//���ͻ����� ���ڷ���ʧ�ܵ��ط� keyΪ�����
//	gtool::CSmartPtr<BusinessDataCacheMap> m_pBusinessDataCacheToRecvMap;	//���ջ����� ���ڽ������ݰ� keyΪ�����


																//Ŀ������
protected:
	std::string				m_strServerAddr;			//Ŀ�ĵ�ַ
	unsigned short		m_ServerPort;				//Ŀ�Ķ˿�
	bool				m_bIsDomainName;			//Ŀ���Ƿ�����

};

//�ͻ��� ��Ҫ���ù�˽Կ
class CNetworkTcpClient :public CNetworkService
{
	NO_COPY(CNetworkTcpClient);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CNetworkTcpClient);

	//�������
public:
	//���ӷ�����
	virtual bool Link(const bool bClearDataCache = true);
	//�ر�����
	virtual bool CloseLink();
	//����
	virtual bool SendDataTo(const void *pData, const long lDataLen);
	//���� ���պ���Ҫ�ֶ��ͷſռ�
	virtual bool RecvDataFrom(void *&pData, long &lDataLen);
	//�������ã�����Ͽ����ӣ�
	virtual bool RefreshSvrAddr(const char *host/*����*/, int hostlen/*����*/, unsigned short port/*�˿�*/, bool bIsDomainName/*�Ƿ�����*/);
	//rsa	key :���ر���server�˿��Խ��ܵļ��������Լ�һ��ר���ڿͻ��˵Ľ�����������˱���ͻ��˿��Խ��ܵļ������Լ�ר���ڷ���˵Ľ�����
	void GetSvrAddr(void *pBuffer, const U16 nBufferSize);	//��ַ�ַ���

	bool Init();
	void Fini();

	bool IsInit() { return m_bInit; }
	bool IsDestroy() { return m_bDestroy; }

	//������������Ϣ
	bool StartMonitor();
	bool Lock();
	bool Unlock();

protected:
	//����Զ��
	virtual bool Connect();
	
protected:
	boost::atomic_bool m_bInit{ false };
	static boost::atomic_bool m_bDestroy;

	gtool::CSmartPtr<CThreadPool> m_pThreadPool;
};

typedef gtool::CSingleton<CNetworkTcpClient> CNetworkTcpClientInstance;
extern CNetworkTcpClient *g_pTcpClientMgr;
#define TcpClientMgr  ( (g_pTcpClientMgr = CNetworkTcpClientInstance::GetInstance()), ASSERT(g_pTcpClientMgr), *g_pTcpClientMgr)

//������


#endif

