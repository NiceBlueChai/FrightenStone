#ifndef CLIENT_RECIEVER_H_
#define CLIENT_RECIEVER_H_

#pragma once
class CTaskBase;

class CTcpClientReciever : public CTaskBase
{
	NO_COPY(CTcpClientReciever);
	MEMPOOL_CREATE(m_MemObj);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CTcpClientReciever);

public:
	//����ִ����
	virtual int Run();
};

#endif

