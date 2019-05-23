#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#pragma once

// #include "BaseCode/MyDataType.h"
// #include <BaseCode/SmartPtr.hpp>
// #include <BaseCode/TPtrList.hpp>

//�������
class CTaskBase;
class CDataLocker;


#if defined(NEED_WINDOWS_COMMON_API)

//�㲥״̬
// #define CAST_UN_DO		0											//�޶���
// #define CASTED_ON		1											//֪ͨ
// #define CASTED_OFF		2											//�ر�

//�̴߳���������
#define		TIME_SLEEP_PER_TASK			10		//ms �߳̿���˯��ʱ��
#define		NO_TASK_BREAK_COUNT			10		//������������������

//���߳�֧��
#include <process.h> 
//#include<afxmt.h> �ٽ���


//�̳߳�
class  CThreadPool
{
	NO_COPY_NEED_RVALUE(CThreadPool);

	//���Ͷ���
public:
	typedef gtool::TPtrList<CTaskBase, DEL_WAY_RELEASE, boost::is_class<CTaskBase>::value> TASK_LIST;
	typedef char STR_BUF[1024];
#define POOL_DEF_CACHE_SIZE (128*1024)
#define POOL_DEF_CACHE_RE_PAUSE_PERCENTAGE	2000			//��ֱ�

	//ʵ�����ӿ�
protected:
	//���캯��
	CThreadPool(int nMinNum, int nMaxNum);
	//���캯��
	CThreadPool();
	//��������
	virtual ~CThreadPool();
public:
	static CThreadPool* CreateNew(int nMinNum, int nMaxNum) { return new CThreadPool(nMinNum, nMaxNum); }
	static CThreadPool* CreateNew() { return new CThreadPool(); }
	//�ͷŶ���
	virtual int Release() { ClearPool(); delete this; return 0; }

	//�̳߳ز���
protected:
	boost::atomic<I32>									m_nMinNum{ 0 };						//��С�߳���
	boost::atomic<I32>									m_nMaxNum{ 0 };						//����߳���
	boost::atomic<I32>									m_nCurNum{ 0 };						//��ǰ�ܵ��߳���
	boost::atomic<I32>									m_nWaitNum{0};						//�ȴ��߳���
	boost::atomic_bool									m_bIsDestroy{ false };					//�̳߳��Ƿ�����

																						//����
protected:
	boost::atomic_bool									m_bIsStopAdd{ false };					//ֹͣ�������
	volatile	bool									m_bEnableCache=false;					//ʹ�û��岻����ִ�л���
	volatile	U64										m_nCacheUsed=0;					//����ʹ�����
	volatile	U64										m_nCacheTopLimit=0;				//��󻺳�ߴ�
	volatile	U16										m_nRePauseCachePercentage=0;		//ִ�к��ٴ�ֹͣ������� ��ֱ�
	boost::atomic_bool									m_bClearPool{ false };
	gtool::CSmartPtr<CDataLocker>							m_pCSynLockerObj=std::move(gtool::CSmartPtr<CDataLocker>());				//���߳��������ͬ��
	mutable gtool::CSmartPtr<TASK_LIST>								m_plistTasklist=std::move(gtool::CSmartPtr<TASK_LIST>(NULL));				//�������
																								//list<stThreadPara *>								m_listThreadHandle;				//�߳���Դ��ʶ

	//����
protected:
	//CDataLocker *										m_pRecycleThreadObj;			//�̻߳���ͬ��
	void ClearPool();


	//�̲߳���
public:
	//ֻ�������޷�����
	void SetThreadNumLimit(const I32& nMin, const I32& nMax);
	//�����߳� ��Ҫ��������
	bool CreateThread(int nThreadNum);
	//�̴߳�����
	static unsigned __stdcall ThreadHandler(void* pThreadData);

	//����
public:
	//�����̳߳�
	void Destroy();
	//�������
	bool AddTask(CTaskBase &rTask, int iThreadPerNum = 1, const bool bCreateNewThread=false);
	//ֹͣ�������
	int StopAdd();
	//ʹ���������
	int EnAbleAdd();
	void DeductCacheByte(U64 u64Byte);
	void AddCacheByte(U64 u64Byte);
	const U64 GetCurCacheUsed();
	const U64 GetCacheTopLimit();
	void SetCacheTopLimit(U64 u64Byte);
	void SetEnableCache(bool bEnable = false);
	bool GetEnableCache();
	U16 GetRePauseCachePercentage();

	//����
protected:
	int CheckTask(bool &bDestroy, int &iThreadPerNum);

	//ͬ������
public:
	//�߳�ͬ�������
	CDataLocker *GetSynLockerObj() { return m_pCSynLockerObj; }
	TASK_LIST* GetTaskList() const;

	bool Lock();
	bool UnLock();
};


#endif


#endif
