#ifndef NET_DATA_PACK_H_
#define NET_DATA_PACK_H_

#pragma once

class NETWORK_SERVICE_API IPackager
{
public:
	virtual int	Release() = 0;		//
	virtual bool IsMemPoolSwitchActive() = NULL;	//��Ҫ�ڴ�ش���
	virtual bool PackNext(void *pDataOut, U64& u64SizeOut) = NULL;						//���
	virtual bool Unpack(void *&pDataOut, U64& u64SizeOut, TRAN_PROTOCOL::PackAccess * & pAccessOut, I64& rIdKey)=NULL;						//���
	IPackager() {}
	virtual ~IPackager() {}

private:
	IPackager(const IPackager&) = delete;
	const IPackager& operator=(const IPackager&) = delete;
};


class NETWORK_SERVICE_API CNetDataPack :IPackager
{
	NO_COPY_NO_MOVE(CNetDataPack);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CNetDataPack);
	MEMPOOL_CREATE(m_MemPoolObj);

public:
	I64 GetSectionSize(const I64 u64Size);	//���Ƶ�������ü������ݴ�С
	bool Init(const void *pData, const U64 u64Size, bool bIsPack = true, const U64 u64Timestamp = 0, const U64 u64RandFeature = 0,  TRAN_PROTOCOL::TRAN_TYPE eTranType = TRAN_PROTOCOL::TRAN_TYPE_MSG, TRAN_PROTOCOL::PACK_TYPE ePackType = TRAN_PROTOCOL::PACK_TYPE_SEND, const I64 nPackBatchID=-1 );
	bool PackNext(void *pDataOut, U64& u64SizeOut);	//��� u64SizeOut����pDataOut��С��������ʵ���ݴ�С
	I64 GetCurPackModeSeqNum();
	bool GetCurPack(void *&pDataOut, U64& u64SizeOut);
	bool Unpack( void *&pDataOut, U64& u64SizeOut, TRAN_PROTOCOL::PackAccess * & pAccessOut, I64& rIdKey);	//������������������ڸ���NetDataPack ��Ҫ��ʱ�������� ��� u64SizeOut����pDataOut��С��������ʵ���ݴ�С
	void ResetBuffer();
	I64 GetCurDataSize() { return m_i64SizeOfData; }
	bool IsInit() const { return  m_bInit; }
	I64 GetCurPackModeTotalNum() { return m_PackAccess.nTotalNum; }
	TRAN_PROTOCOL::TRAN_TYPE GetCurTranType() { return TRAN_PROTOCOL::TRAN_TYPE(m_PackAccess.nTranType); }

protected:
	bool MakeMd5Str(char * Buffer, const U64 u64SizeOutIn, TRAN_PROTOCOL::PackHead& rHead, TRAN_PROTOCOL::PackAccess& rAccess, TRAN_PROTOCOL::PackData* pPackData, TRAN_PROTOCOL::PackTail& rTail);

protected:
	bool		m_bInit = false;			//��ʼ��
	BUFFER2048	m_szBuffer = { 0 };			//����
	U16			m_nBufferUsed = 0;			//������ʹ��
	void *		m_pData = NULL;				//Ҫ���/���������
	I64			m_i64SizeOfData = 0;		//Ҫ���/������ݴ�С
	I64			m_i64HandledDataSize = 0;	//�Ѿ���������ݳ���
	bool		m_bPack = true;				//�Ƿ���ģʽ
	TRAN_PROTOCOL::PackAccess m_PackAccess;	//���������
};

// class NETWORK_SERVICE_API NetDataPackMgr
// {
// 	NO_COPY_NO_MOVE(NetDataPackMgr);
// 	HEAP_CREATE_NEED_CLEANS_PUBLIC(NetDataPackMgr);
// 	MEMPOOL_CREATE(m_MemPoolObj);
// 
// public:
// 	bool AddPack(const void*pData, const U64 u64Size);
// 	bool Unpack(void *&pData, U64& u64Size);
// 
// 	typedef gtool::CLitPtrMap<CNetDataPack, U32, DEL_WAY_RELEASE> NetDataPackMap;	//key:SeqNo
// 	gtool::CSmartPtr<NetDataPackMap> m_pNetDataPackMap;	//���ݰ�map
// };



#endif

