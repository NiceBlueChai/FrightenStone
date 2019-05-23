#include "StdAfx.h"

#include <BaseCode/Win/ThreadPool.h>
#include <BaseCode/Win/WinLocker.h>
#include<string.h>
#include <iostream>

#include<BaseCode/TaskBase.h>
#include <BaseCode/FileDirSys.h>
#include <BaseCode/CheckNoLog.h>
#define CLEAR_GROUP_WAIT_TIME_DEF	5000		//5ms�ȴ�������������

#if defined(NEED_WINDOWS_COMMON_API)

#include <thread>
//�̳߳�

//����
CThreadPool::CThreadPool(int nMinNum, int nMaxNum)
{
	//��ʼ������
	m_nMinNum = nMinNum;		//��ʼ�߳���
	m_nMaxNum = nMaxNum;		//����߳���
	m_bIsDestroy = false;				//�̳߳�����
	m_nCurNum = 0;					//��ǰ�߳���
	m_nWaitNum = 0;					//�����߳���
	m_bIsStopAdd = false;				//�Ƿ�ֹͣ��������
	m_nCacheUsed = 0;
	m_nCacheTopLimit = POOL_DEF_CACHE_SIZE;
	m_bEnableCache = false;
	m_nRePauseCachePercentage = POOL_DEF_CACHE_RE_PAUSE_PERCENTAGE;

	m_pCSynLockerObj = CDataLocker::CreateNew(true);
	m_plistTasklist = TASK_LIST::CreateNew();

	//��ʼ���ֳ�
	ASSERT(this->CreateThread(m_nMinNum));
}

CThreadPool::CThreadPool(CThreadPool&& rValue)
{
	m_pCSynLockerObj = std::move(rValue.m_pCSynLockerObj);
	m_plistTasklist = std::move(rValue.m_plistTasklist);
}

CThreadPool::CThreadPool()
{
	//��ʼ������
	m_nMinNum = 0;		//��ʼ�߳���
	m_nMaxNum = 0;		//����߳���
	m_bIsDestroy = false;				//�̳߳�����
	m_nCurNum = 0;					//��ǰ�߳���
	m_nWaitNum = 0;					//�����߳���
	m_bIsStopAdd = false;				//�Ƿ�ֹͣ��������
	m_nCacheUsed = 0;
	m_nCacheTopLimit = POOL_DEF_CACHE_SIZE;
	m_bEnableCache = false;
	m_nRePauseCachePercentage = POOL_DEF_CACHE_RE_PAUSE_PERCENTAGE;

	m_pCSynLockerObj = CDataLocker::CreateNew(true);
	m_plistTasklist = TASK_LIST::CreateNew();
}

//����
CThreadPool::~CThreadPool()
{

}

void CThreadPool::ClearPool()
{
	CHECK_NL(m_pCSynLockerObj&&m_plistTasklist);
	if (m_bClearPool)
		return;

	m_bClearPool = true;

	try
	{
		//ֹͣ�������
		StopAdd();							//ֹͣ�������

		//�ȴ��߳��������
		WIN_WAIT_EVENT nWaitRet = WIN_WAIT_EVENT_FAILED;
		bool bEmpty = false;
		const I64 nLoopMax = 1000;
		I64 nCountLoop = 0;
		while (true)
		{
			DEAD_LOOP_ASSERT(nCountLoop, nLoopMax);
			ASSERT(m_pCSynLockerObj->Lock());
			m_pCSynLockerObj->SinalOne();	//�����߳����������
			m_pCSynLockerObj->UnLock();
			Sleep(CLEAR_GROUP_WAIT_TIME_DEF);
			ASSERT(m_pCSynLockerObj->Lock());
			bEmpty = m_plistTasklist->IsEmpty();
			ASSERT(m_pCSynLockerObj->UnLock());
			if (bEmpty)
				break;
		}

		IF_NOT_NL(m_plistTasklist->IsEmpty())
			m_plistTasklist->Clear();

		//�߳��˳�
		m_bIsDestroy = true;

		bool bNoneThread = false;
		nCountLoop = 0;
		ASSERT(m_pCSynLockerObj->Broadcast());
// 		while (true)
// 		{
// 			DEAD_LOOP_ASSERT(nCountLoop, nLoopMax);
// 			ASSERT(m_pCSynLockerObj->Lock());
// 			m_pCSynLockerObj->SinalOne();
// 			nWaitRet = m_pCSynLockerObj->WaitEvent(CLEAR_GROUP_WAIT_TIME_DEF);		//5ms��ʱ
// 			bNoneThread = m_nCurNum <= 0;
// 			ASSERT(m_pCSynLockerObj->UnLock());
// 			if (bNoneThread)
// 				break;
// 		}	
	}
	catch (...)
	{
		ASSERT(!"�����̳߳ط���CRASH*******");
	}
}

void CThreadPool::SetThreadNumLimit(const I32& nMin, const I32& nMax)
{
	CHECK_NL(nMin <= nMax && (!m_bIsDestroy) && (!m_bIsStopAdd));

	I32 nMaxNum = m_nMaxNum;
	m_nMaxNum = gtool::Max(nMaxNum, nMax);		//����߳���

	auto nDiffMin = nMin - m_nMinNum;
	if (nDiffMin > 0)
	{
		m_nMinNum = nMin;		//��ʼ�߳���

		ASSERT(CreateThread(nDiffMin));
	}
}

//�����߳� ��Ҫ��������
bool CThreadPool::CreateThread(int nThreadNum)
{

	//У�����
	CHECKF_NL (nThreadNum>=0 && nThreadNum<=m_nMaxNum);

	//����
	unsigned int uThreadID;
	HANDLE ret;

	for (int i = 0; i < nThreadNum; i++)
	{
		//������ʼ��
		ret = (HANDLE)::_beginthreadex(NULL, 0, ThreadHandler, (void *)(this), 0, &uThreadID);	//�����߳�
		if (ret != INVALID_HANDLE_VALUE)
			ASSERT(CloseHandle(ret));	//�ͷ��ں���Դ
		else
		{
			ASSERT(ret != INVALID_HANDLE_VALUE);
		}

		//�����ж�
		if (ret == INVALID_HANDLE_VALUE)
		{
			IF_NOT_NL(i != 0)
				return false;
			else
				break;
		}

		m_nCurNum++;	//��ǰ�߳���
	}

	return true;
}

//�������
bool CThreadPool::AddTask(CTaskBase &rTask, int iThreadPerNum /*= 1*/, const bool bCreateNewThread/*=false*/)
{
	IF_NOT_NL(m_pCSynLockerObj)
		return false;

	IF_NOT_NL(iThreadPerNum>0)
		return false;

	bool bDestroy = false;
	IF_NOT_NL (CheckTask(bDestroy, iThreadPerNum) == RES_SUCCESS)
		return false;

	if (m_bIsStopAdd)
	{
		return false;
	}

	//�������
	if (!bDestroy)
	{
		IF_NOT_NL(m_pCSynLockerObj->Lock())
			return false;//---�ٽ���

		//�������
		if (m_plistTasklist->PushBack(&rTask))
		{
			m_nCacheUsed += rTask.GetCacheSize();
		}

		if (m_nWaitNum > 0&&!bCreateNewThread)
		{
			//�����߳�			
			m_pCSynLockerObj->SinalOne();
			ASSERT(m_pCSynLockerObj->UnLock());
		}
		else
		{
			//�����߳�
			if (m_nCurNum+ iThreadPerNum <= m_nMaxNum)
			{
				IF_NOT_NL(CreateThread(((iThreadPerNum > (m_nMaxNum - m_nCurNum)) ? (m_nMaxNum - m_nCurNum) : iThreadPerNum)))//�����߳�
				{
					ASSERT(m_pCSynLockerObj->UnLock());
					return false;
				}
			}
			ASSERT(m_pCSynLockerObj->UnLock());
		}
	}
	else
	{
		return false;
	}

	return true;
}

//�̴߳�����
unsigned __stdcall CThreadPool::ThreadHandler(void* pThreadData)
{
	//mfc���̷߳���
	CThreadPool *pPool = (CThreadPool *)(pThreadData);

	//����У��
	IF_NOT_NL(pPool)
	{
		//��ֹ�߳�
		_endthreadex(0L);
		return 0L;
	}

	try
	{
		auto pLock = pPool->GetSynLockerObj();
		auto pList = pPool->GetTaskList();

		//�߳���������
		while (!pPool->m_bIsDestroy)
		{
			ASSERT(pLock->Lock());

			if (pList->IsEmpty() != true)//�ж������Ƿ�Ϊ��
			{
				//����ִ��
				{
					gtool::CSmartPtr<CTaskBase> pTask = std::move(gtool::CSmartPtr<CTaskBase>(pList->PopFront()));//ȡ����

					ASSERT(pLock->UnLock());

					try
					{						
						pTask->Run();	//ִ������
					}
					catch (...)
					{
						ASSERT(!"TASK CRASH");
					}
				}
			}
			else
			{
				pPool->m_nWaitNum++;
				auto nWaitRet = pLock->WaitEvent();
				ASSERT(nWaitRet == WIN_WAIT_EVENT_SUC);
				pPool->m_nWaitNum--;
				ASSERT(pLock->UnLock());
			}
		}

		pPool->m_nCurNum--;
		auto bNoneThread = pPool->m_nCurNum <= 0;
		if (bNoneThread)
		{
			ASSERT(pLock->Lock());
			pLock->SinalOne();
			ASSERT(pLock->UnLock());
		}		
	}
	catch (...)
	{
		ASSERT(!"CRASH THREAD");
	}

	//��ֹ�߳�
	_endthreadex(0L);
	return 0L;
}

void CThreadPool::Destroy()
{
	ClearPool();
}

int CThreadPool::StopAdd()
{
	m_bIsStopAdd = true;
	return RES_SUCCESS;
}

int CThreadPool::EnAbleAdd()
{
	m_bIsStopAdd = false;
	return RES_SUCCESS;
}

void CThreadPool::DeductCacheByte(U64 u64Byte)
{
	m_nCacheUsed -= u64Byte;
}

void CThreadPool::AddCacheByte(U64 u64Byte)
{
	m_nCacheUsed += u64Byte;
}

const U64 CThreadPool::GetCurCacheUsed()
{
	auto nCurCache = m_nCacheUsed;

	return nCurCache;
}

const U64 CThreadPool::GetCacheTopLimit()
{
	auto nTopCache = m_nCacheTopLimit;

	return nTopCache;
}

void CThreadPool::SetCacheTopLimit(U64 u64Byte)
{
	m_nCacheTopLimit = u64Byte;
}

void CThreadPool::SetEnableCache(bool bEnable /*= false*/)
{
	m_bEnableCache = bEnable;
}

bool CThreadPool::GetEnableCache()
{
	auto bUsed = m_bEnableCache;

	return bUsed;
}

U16 CThreadPool::GetRePauseCachePercentage()
{
	auto nVal = m_nRePauseCachePercentage;

	return nVal;
}

int CThreadPool::CheckTask(bool &bDestroy, int &iThreadPerNum)
{
	if (!m_pCSynLockerObj)
	{
		ASSERT(m_pCSynLockerObj);
		return RES_FAILED;
	}

	//����У��
	bDestroy = m_bIsDestroy;
	if (m_bIsStopAdd == true)
	{
		return RES_FAILED;
	}

	if (iThreadPerNum+m_nCurNum > m_nMaxNum) iThreadPerNum = m_nMaxNum- m_nCurNum;

	return RES_SUCCESS;
}

CThreadPool::TASK_LIST* CThreadPool::GetTaskList() const
{
	return m_plistTasklist;
}

bool CThreadPool::Lock()
{
	if (!m_pCSynLockerObj)
	{
		ASSERT(!m_pCSynLockerObj);
		return false;
	}
	return  m_pCSynLockerObj->Lock();
}

bool CThreadPool::UnLock()
{
	if (!m_pCSynLockerObj)
	{
		ASSERT(!m_pCSynLockerObj);
		return false;
	}
	return  m_pCSynLockerObj->UnLock();
}


#endif



