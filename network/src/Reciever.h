#ifndef RECIEVER_H_
#define RECIEVER_H_

#pragma once

class CTaskBase;

class CReciever :public CTaskBase
{
	MEMPOOL_CREATE(m_MemObj);
public:
	CReciever() {}
	virtual ~CReciever() {}

	//����ִ����
	virtual int Run();

	//�ͷ�
	virtual int Release()
	{
		delete this;
		return 0;
	}
};


#endif

