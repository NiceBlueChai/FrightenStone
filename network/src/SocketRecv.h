#ifndef SOCKET_RECV_H_
#define SOCKET_RECV_H_

#pragma once

//�̲߳���ȫ�� CSocketRecv������������ֻ�ܰ�һ��socket
class CSocketRecv
{
	NO_COPY(CSocketRecv);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CSocketRecv);
	MEMPOOL_CREATE(m_MemObj);

public:
	bool Recv();	//��������
	bool Bind(const SOCKET& sbind, const I64 nCacheSizeSet = 0);	//��socket��
	I64 GetCacheSize() const { return m_nCacheSizeMax; }	//��ȡ��������С
	I64 GetCurDataSize() const { return m_nCurDataSize; }	//��ȡ��ǰ���ݴ�С
	const char *GetCurData() const { return m_pBuffer; }	//��ȡ��ǰ����
	void ResetCache();

protected:
	gtool::CLitUnmanagedPtr<SOCKET> m_ps = NULL;
	gtool::CSmartPtrC<char, char, DEL_WAY_DELETE_MULTI> m_pBuffer = NULL;
	I64			m_nCurDataSize = 0;		//��ǰ���ݴ�С
	I64			m_nCacheSizeMax = 0;	//��󻺳��С
	I32			m_nLastErr = 0;			//socket����
};

#endif

