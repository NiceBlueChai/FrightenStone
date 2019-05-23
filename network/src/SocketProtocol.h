#ifndef SOCKET_PROTOCOL_H_
#define SOCKET_PROTOCOL_H_

#pragma once
#include <BaseCode/NetworkServiceApi.h>
#include <openssl/md5.h>
#include <PoolObj/xByte.h>
#include <PoolObj/xUByte.h>

//Э��
//��ʽ
//

#pragma MEM_ALIGN_BEGIN(1)

NAMESPACE_BEGIN(TRAN_PROTOCOL)

typedef I32 PROTOCOL_VER;

#define NETPACK_ORDERID_LEN	64		//�������ַ�������

#define PACKAGE_SIZE_MAX I64(1024)		//���������ƴ�С1024

#undef TIMEOUT_RESEND_COUNT
#define  TIMEOUT_RESEND_COUNT 10			//��ʱ�ط�����

//Э������
enum TRAN_TYPE
{
	TRAN_TYPE_NONE=0,		//δ֪��
	TRAN_TYPE_MSG,			//��Ϣ���� ֧�ֳ�����
	TRAN_TYPE_FILE,			//�����ļ�
	TRAN_TYPE_HTTP,			//http����
	TRAN_TYPE_HTTPS,		//https
	TRAN_TYPE_FTP,			//FTP
	TRAN_TYPE_SEC_MSG,		//��ȫ����ϢЭ��
	TRAN_TYPE_END,
};

//����ֵ
enum FEATURE_NUM
{
	FEATURE_NUM_PACK_HEADER=0x0404,	//��ͷ����ֵ
	FEATURE_NUM_PACK_TAIL=0x0505,	//��β����ֵ
};

enum PACK_TYPE
{
	PACK_TYPE_NULL = -1,	//��Ч
	PACK_TYPE_SEND=0,		//���Ͱ�
	PACK_TYPE_REPLY=1,		//���ͺ��Ӧ��� ����������һ����
	PACK_TYPE_END,
};

//У�飺PackHead+PackAccess+PackData+PackTail+key

//��ͷ
struct PackHead
{
	U16 nMark;		//����ֵ
};

//һ��������Ψһ��ʶ��nTranType-nPackType-nTimestamp-nPackBatchID-nTotalNum-nRandFeature
//У�������nMark+nTranType+nTotalNum+nPackBatchID+nSeqNum+nPackSize+nPackType+nTimestamp+nRandFeature+Len+Stream+nMark
//�����
struct PackAccess
{
	U16			nTranType;	//Э������ TRAN_TYPE *
	U32			nTotalNum;	//������
	I64			nPackBatchID;	//�������� ��������Ϊsocket������ ÿһ������ �ɰ�����ά������ socket������Ψһ ֻ����
	U32			nSeqNum;	//��ǰ��� ��ű����1��ʼ
	U16			nPackSize;	//��ǰ��������С
	U8			nPackType;	//������ PACK_TYPE *
	U64			nTimestamp;	//ʱ����Ժ���Ϊ��λ *
	U64			nRandFeature;	//����ֵ boost��������������� *
	U8			Md5Str[MD5_VERIFY_SIZE];	//md5�ַ���
};

//��ʵ������
struct PackData
{
	U16 nLen;		//���������� *
	U8	Stream[0];	//�ɱ䳤������������ *
};

//��β
struct PackTail
{
	U16 nMark;		//����ֵ
};

//������
struct NETWORK_SERVICE_API NetDataStream
{
	U16 nLen;
	char rData[0];			//����������� �ڷ��������յ�ʱ�����Ϊ PackData.Stream
};

struct  BusinessDataCache
{
	U16 nLen;
	char rData[0];			//����������� �ڷ��������յ�ʱ�����Ϊ PackData.Stream
};

//typedef NetDataStream BusinessDataCache;

// struct NetCache
// {
// 	union
// 	{
// 		void *m_ptr;
// 		BusinessDataCache *m_pBusinessDataCache;
// 		CXByte *m_pXByte;
// 		CUXByte m_pUXByte;
// 	};
// 	NetCache(void *ptr) :m_ptr(ptr){}
// };

DECLEAR_UNION_PTR_BEGIN(NetCache)
ADD_UNION_PTR_VAR(BusinessDataCache, m_pBusinessDataCache)
ADD_UNION_PTR_VAR(CXByte, m_pXByte)
ADD_UNION_PTR_VAR(CUXByte, m_pUXByte)
ADD_UNION_PTR_VAR(char, m_pstr)
DECLEAR_UNION_PTR_END(NetCache)

// struct NetCache
// {
// 	union
// 	{
// 		void*				m_ptr;
// 		NetDataStream *		m_pNetDataStream;
// 		CXByte *			m_pXByte;
// 	};
// 	NetCache(void* pBuf = NULL) :m_ptr((void*)pBuf) {}
// };

//����ͳ��
typedef  struct DataFlowCount
{
	long	lRecvTotal;				//�ܵĽ�����	byte
	long	lSendTotal;				//�ܵķ�����	byte
}DATA_FLOW_COUNT_T;

NAMESPACE_END(TRAN_PROTOCOL)
#define S_DATA_FLOW_COUNT_T		sizeof(TRAN_PROTOCOL::DATA_FLOW_COUNT_T)
#pragma MEM_ALIGN_END(1)


#endif