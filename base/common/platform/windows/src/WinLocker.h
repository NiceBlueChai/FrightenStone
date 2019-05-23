#ifndef WIN_LOCKER_H_
#define WIN_LOCKER_H_

#pragma once
//#include <windows.h>
//#include <BaseCode/myclassmacro.h>
//#include<BaseCode/SmartPtr.hpp>
enum WIN_WAIT_EVENT;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(NEED_WINDOWS_COMMON_API)

//ͬһ�߳��ڶ�ε���lock��ѭ�����ߵݹ���ã����ᱻ��������ֻ���Ʋ�ͬ�̼߳䣬�ؼ��λ�ȡ��ֻ���ֲ�ͬ�߳�Ҫunlock��Ҫ�̻߳�ȡ��
//��Ҫ��ȫ����
class  CCriticalSectionProtect
{
	NO_COPY(CCriticalSectionProtect);

public:
	static CCriticalSectionProtect* CreateNew();
	//�ͷŶ���
	virtual int Release();
	CCriticalSectionProtect();

	virtual ~CCriticalSectionProtect();

	bool Lock();						
	bool Unlock();
	bool TryLock();
	bool Islock();
	bool IsInit();
	bool Init();
	void YieldScheduler();

private:
	typename CRITICAL_SECTION    m_CritSec;
	boost::atomic<bool>			m_bInit{ false };
};

//������
class CDataLocker
{
	NO_COPY(CDataLocker);
	
	//��������
protected:
	boost::atomic<I64>								m_nLockCount{ 0 };				//��������
	boost::atomic<I64>								m_nWaitCount{ 0 };				//��������
	CCriticalSectionProtect							m_CSLockerObj;					//�ٽ���
	//void *											m_hEvent;						//�¼����
	boost::atomic_address							m_hEvent;
	boost::atomic_bool								m_bSinal{ false };				//�Ƿ����ź�
			
	//��������
public:
	static CDataLocker* CreateNew(bool bCreateEvent = false);
	//�ͷŶ���
	virtual int	Release();
	//���캯��
	CDataLocker(bool bCreateEvent=false);
	//��������
	virtual ~CDataLocker();

	//��������
public:
	//��������
	bool LoopLock();
	//�������� 
	bool LoopUnLock();

	//��ѭ������
public:
	bool Lock();
	bool UnLock();

	//�¼��ȴ�
public:

	//�����¼����ֶ�����ʼ���������¼�
	bool CreateEventDef();	
	WIN_WAIT_EVENT WaitEvent(const unsigned long ulMilliseconds=INFINITE); //����΢�� �̲߳���ȫ��Ҫ��
	bool SinalOne();	//�̰߳�ȫ��
	bool Broadcast();	//���������߳�
	bool BroadcastSinal()=delete;	//��Ч
	bool CloseEvent();

	//��״̬
public:
	bool Islock();
	void YieldScheduler();

	//״̬����
public:
	//��������
	int GetLockCount();
};

#endif

#endif
