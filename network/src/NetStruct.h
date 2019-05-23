#ifndef NET_STRUCT_H_
#define NET_STRUCT_H_

#pragma once

#include <BaseCode/NetworkServiceApi.h>
//#include <BaseCode/MemPoolObj.h>

#pragma MEM_ALIGN_BEGIN(1)

#ifndef _WS2DEF_

typedef struct _WSABUF {
	ULONG len;     /* the length of the buffer */
	_Field_size_bytes_(len) CHAR FAR *buf; /* the pointer to the buffer */
} WSABUF, FAR * LPWSABUF;

#endif

typedef I64 PACK_UNIQUE_ID;		//��Ψһ��ʶ nPackBatchID ��������
struct SocketMsgInfo
{
	SOCKET_IDD idSocket;		//�׽���Ψһid
	SOCKET_STATUS_TYPE eType;	//�׽���״̬
	TRAN_PROTOCOL::TRAN_TYPE eTranType;	//Э������
	I64		 nPackBatchID;		//����
};

typedef gtool::TPtrList<SocketMsgInfo, DEL_WAY_DELETE, false> SocketMsgInfoList;	//�׽�����Ϣ��Ϣ����

//��ַ��Ϣ��
struct NETWORK_SERVICE_API CAddr
{
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CAddr)
	MEMPOOL_CREATE(m_MemPoolAlloctor)

	//��Դ�������ͷ�
public:
	CAddr(char *szip, unsigned short usPort, unsigned short usFamily = AF_INET);
	CAddr(const SOCKADDR_IN& rOther);
	void Reset();	//��λ

	//���ú���
public:
	//���ظ�ֵ�����
	CAddr & operator =(const CAddr &addr);	//
	CAddr & operator =(CAddr &addr);
	 operator const struct sockaddr *() const;	//��ȡ�ɰ��ַ
	 operator const struct sockaddr_in *() const;	//��ȡ�°��ַ
	 operator struct sockaddr *();	//��ȡ�ɰ��ַ
	 operator struct sockaddr_in *();	//��ȡ�°��ַ
	 operator const char *() const;			//��ȡip
	 operator const U16() const;			//��ȡ�˿ں�
	
	 //����ֵ
public:
	void SetAddrInfoEx(const struct sockaddr_in &stAddr_Info);	//���õ�ַ��ṹ��
	void SetAddrInfo(const unsigned short family, const char *szip = NULL
		,const char *domainname = NULL, bool bUserIp = true
		, const unsigned short* port = NULL, bool bUseBind =true
		, bool bUserSockStream = true);		//��ַ

	//��ȡ��Ϣ
public:	
	const char *GetDomainName() const;					//����	
	U16 GetFamily() const;				//��ȡ����Э������	

protected:
	bool GetAddrInfo(bool bReload = false) const;

protected:
	mutable SOCKADDR_IN				m_stAddr_Info;						//��ַ����Ϣ�ṹ���°�
	BUFFER1024						m_DomainName;						//����
	mutable BUFFER128				m_szCache;							//���� ������
	U16								m_uCache;							//���� ������
};

//====================================================================================
//
//				��IO���ݽṹ�嶨��(����ÿһ���ص������Ĳ���)
//
//====================================================================================

//��ö�������
typedef NETWORK_SERVICE_API struct _RW_CONTEXT
{
	OVERLAPPED     m_Overlapped;                               // ÿһ���ص�����������ص��ṹ(���ÿһ��Socket��ÿһ����������Ҫ��һ��)              
	WSABUF         m_wsaBuf;                                   // WSA���͵Ļ����������ڸ��ص�������������
	char           m_szBuffer[MAX_BUFFER_LEN];                 // �����WSABUF�������ַ��Ļ�����
	U64			   m_u64DataToHandle;							//����������ݳ���
	U16			   m_u16FailHandleCount;						//ʧ�ܴ���
	OPERATION_TYPE m_OpType;                                   // ��ʶ�������������(��Ӧ�����ö��)
	void *			m_pParrentObj = NULL;							//��ǰ�ڵ������� _PER_SOCKET_CONTEXT
	PACK_UNIQUE_ID  m_idPackBatchID = -1;							//������
	I64				m_nSeqNum = -1;								//�����
	TRAN_PROTOCOL::TRAN_TYPE m_nTranType = TRAN_PROTOCOL::TRAN_TYPE_MSG;	//��Э��

	//�������ͷ�
	_RW_CONTEXT();
	~_RW_CONTEXT();
	bool operator==(const _RW_CONTEXT& Other);	//����������

	
	void ResetBuffer();	// ���û���������
	SOCKET GetSocket();	//��ȡ�׽���
	const SOCKADDR_IN* GetAddr();	//��ȡ�׽��ֵ�ַ��Ϣ
	const CAddr* GetAddrObj();
	const char *GetIp();
	const U16 GetPort();
	MEMPOOL_CREATE(m_MemPoolAlloctor);	//�������ڴ��

} PER_RW_CONTEXT,  *PPER_RW_CONTEXT;




//====================================================================================
//
//				��������ݽṹ�嶨��(����ÿһ����ɶ˿ڣ�Ҳ����ÿһ��Socket�Ĳ���)
//
//====================================================================================


typedef gtool::TPtrList<_RW_CONTEXT,DEL_WAY_DELETE, false> NetRWContextList;	//�׽��ֶ�д������List

typedef gtool::CLitPtrMap<char, I32, DEL_WAY_DELETE_MULTI> PackDataMap;		//�������� ����ʹ�� NetDataStream key:nSeqNum

struct PackDataMapInfo
{
	PackDataMapInfo()
	{
		m_pPackDataMap = PackDataMap::CreateNew();
		ASSERTEX(m_pPackDataMap);
	}
	~PackDataMapInfo() {}

	I32 Release()
	{
		IF_OK(m_pPackDataMap)
			m_pPackDataMap->Cleans();

		return 0;
	}

	gtool::CSmartPtr<PackDataMap> m_pPackDataMap = NULL;
	I64 m_nTotalByte = 0;
};

typedef gtool::CLitPtrMap<PackDataMapInfo, U16, DEL_WAY_RELEASE> TransTypePackDataMap;	//key: TRAN_TYPE val:PackDataMap
typedef gtool::CLitPtrMap<TransTypePackDataMap, PACK_UNIQUE_ID, DEL_WAY_RELEASE> AllPackDataWaitHandleMap;		//������������������� key:PACK_UNIQUE_ID socket��������Ψһ

typedef NETWORK_SERVICE_API struct  _PER_SOCKET_CONTEXT
{
protected:
	SOCKET      m_Socket;                                  // ÿһ���ͻ������ӵ�Socket
	CAddr		m_Addrinfo;									//��ַ��Ϣ

public:	
	gtool::CSmartPtr<NetRWContextList> m_pListReadContext;             // ��ͻ��˶��������������ݣ�
	gtool::CSmartPtr<NetRWContextList> m_pListWriteContext;				//��ͻ���д��������������

// 	gtool::CSmartPtr<AllPackDataWaitHandleMap> m_pAllPackDataRecvMap;			//������������ݽ�����ϵ�
// 	gtool::CSmartPtr<AllPackDataWaitHandleMap> m_pAllPackDataSendMap;			//��������������ѷ���ȴ����͵�

protected:
	bool						m_bCloseSocket;					//�ͷ�ʱ���Ƿ�ر��׽���
// 	fd_set						m_fs_read;							//�ɶ����
// 	fd_set						m_fs_write;							//��д���
// 	boost::atomic<I64>			m_nSendPackBatchID{ -1 };			//���͵�����id

	//����
public:
	_PER_SOCKET_CONTEXT();
	~_PER_SOCKET_CONTEXT() {}
	int Release();

	//������
	bool operator==(const _PER_SOCKET_CONTEXT& Other);

	// io�������
public:
	_RW_CONTEXT* GetNewReadContext();	//��ȡ�µĴ�IOCp���յ����ݻ���
	_RW_CONTEXT* GetNewWriteContext();	//��ȡ�µĴ�iocp���͵����ݻ���
// 	PackDataMapInfo *GetNetDataRecvMap(const PACK_UNIQUE_ID& Id, const U16 nTransType, bool bCreate = false);	//��ȡ������ϰ�����
// 	PackDataMapInfo *GetNetDataSendMap(const PACK_UNIQUE_ID& Id, const U16 nTransType, bool bCreate = false);	//��ȡ�����ϰ�����
// 	bool DelNetDataSendMap(const PACK_UNIQUE_ID& Id, const U16 nTransType);
// 	bool DelNetDataRecvMap(const PACK_UNIQUE_ID& Id, const U16 nTransType);
	void DelContext(_RW_CONTEXT* pContext);			// ���������Ƴ�һ��ָ����IoContext
	void DelReadContext(_RW_CONTEXT* pContext);
	void DelWriteContext(_RW_CONTEXT* pContext);
	_RW_CONTEXT* FrontToSend();
	_RW_CONTEXT* GetReadFront();
	SOCKET GetSocket() const { return m_Socket; }
	SOCKET& GetSocketObj() { return m_Socket; }
	void SetSocket(const SOCKET sNetSocket) {m_Socket = sNetSocket;}
	const CAddr& GetAddrInfo() const { return m_Addrinfo; }
	template<typename T>
	void SetAddrInfo(const T tObj)
	{
		m_Addrinfo.SetAddrInfoEx(tObj);
	}

// 	bool IsExistNetPackDataRecv(const PACK_UNIQUE_ID& Id);
// 	bool IsExistNetPackDataRecv(const PACK_UNIQUE_ID& Id, const U16 nTransType, const I32 nSeqNum);
// 	bool IsExistNetPackDataSend(const PACK_UNIQUE_ID& Id);
// 	bool IsExistNetPackDataSend(const PACK_UNIQUE_ID& Id, const U16 nTransType, const I32 nSeqNum);

	//����
protected:
//	TransTypePackDataMap * GetTransTypeSendMap(const PACK_UNIQUE_ID& Id);



	//����
public:
	void SetSocketCloseFinal(bool bClose);
//	bool IsDetectTimeOut(long tv_sec, long tv_usec, bool bReadableDetect = true, bool bWriteableDetect = false, int *pErrOut = NULL, bool bSetOneAtLeast = false, bool bInfiniteWaiting = false);
	const char *GetIp();
	const U16 GetPort();
//	I64 GetCurPackBatchID() { return m_nSendPackBatchID; }
//	void SetCurPackBatchID(const I64 nPackBatchID) { m_nSendPackBatchID = nPackBatchID; }

	MEMPOOL_CREATE(m_MemPoolAlloctor);

} PER_SOCKET_CONTEXT, *PPER_SOCKET_CONTEXT;

struct NetDataBuffer
{
	HEAP_CREATE_NEED_CLEANS_PUBLIC(NetDataBuffer);
	MEMPOOL_CREATE(m_MemObj);

public:
	BUFFER8192 szBuffer;
	I64 nSizeData = 0;
};

//���ͻ��߽��ջ�����������
struct NetCacheData
{
	NET_CACHE_TYPE ePackCacheType;		//������ ������ �ظ���

};

//�������绺��2 ʹ�����ڴ�ظ���Ч
struct TimestampXByte
{
	I64		nTimestamp = 0;			//�ɷ����߳���� ��ȷ������
	I64		nSizeStream = 0;		//��������С
	gtool::CSmartPtrC<CXByte, CXByte, DEL_WAY_DELETE_MULTI> pdata_stream = NULL;
};

typedef gtool::CLitPtrMap<TimestampXByte, I32, DEL_WAY_DELETE> XByteStreamMap;		//�������� ����ʹ�� NetDataStream key:nSeqNum
struct PackCacheMapInfo
{
	MEMPOOL_CREATE(m_MemObj);

public:
	PackCacheMapInfo()
	{
		m_pXByteStreamMap = XByteStreamMap::CreateNew();
		ASSERTEX(m_pXByteStreamMap);
	}
	~PackCacheMapInfo() {}

	I32 Release()
	{
		IF_OK(m_pXByteStreamMap)
			m_pXByteStreamMap->Cleans();

		return 0;
	}

	gtool::CSmartPtr<XByteStreamMap> m_pXByteStreamMap = NULL;
	I64 m_nTotalByte = 0;
	U64 m_Timestamp = 0;	//������
};

typedef gtool::TPtrList<NetDataBuffer, DEL_WAY_RELEASE, false> NetDataBufferList;


#pragma MEM_ALIGN_END(1)

///////////////////////////////////////////////////////////////////////////////////////

#endif

