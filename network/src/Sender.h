#ifndef SENDER_H_
#define SENDER_H_

#pragma once

#include <TDPoster/ThreadPostInterfaceClass.hpp>

class CSenderThread :public INetworkSenderThread
{
	MEMPOOL_CREATE(m_MemObj);
public:
	CSenderThread(boost::atomic_bool& bThreadWork) :m_bThreadWork(bThreadWork) {}
	virtual ~CSenderThread() {}

	//����ִ����
	virtual int Run();

	//�ͷ�
	virtual int Release();

	virtual bool HandleRecvMsg(const U64 u64ConnectionPtr, const void *pData, const I64 nSize);	 //pData ���ݲ������ߴ� nSize���ݳߴ�

protected:
	boost::atomic_bool&		m_bThreadWork;		//�������m_bThreadWork����
};


#endif

