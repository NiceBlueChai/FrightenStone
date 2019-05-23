#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

/*	�ڴ��													*/
/*	���ã���߳�������										*/
/*	���˼·��												*/
/*	1.�ڴ�ذ���һ���ܿ���ͷҳ��n�����ڴ�飬ÿ���ڴ���һ������ͷҳ��һ��δ�ָ�����list����һ������ʹ������list����һ����������list����һ���ں�����map��	��һ��ά���߳�*/
/*	2.�����ڴ��������ȴ�δ�ָ�����ʼ��δ�ָ�������ģ�������һ��ͬ�ȴ�С���µĴ��ڴ�飨���ݵ�ǰ�����ڴ������ڴ棩�������������ҳ������ǰ��,			*/
/*	3.ÿ����δ�ָ�����ȡ���ڴ棬������һ���ӱ�ź�һ������ţ�������һ��������ź��Ա����ɵ�ȫ��Ψһ��ʶid���ڿ��ٵķ���	*/
/*	4.����ʹ�������ڴ��ͷţ����ͷŵ��ڴ�����ڻ������ȴ��ں�				*/
/*	5.ά���̶߳�ʱ�����������Ƿ��пɻ��յ��ڴ�飬������ת�Ƶ��ں������ں���ֻ�ںϡ����ڵ��ڴ�顱ֱ����һ�������ڵĽ�ֹ����������ָ�������ַ�ϵ����ڣ����������ڵı���������Ż��������ں�����ڴ�Ӧ�ñ�pushback��δ�ָ������ڴ棬ѭ������								*/
/*	6.ά���̻߳���Ҫ���������ڴ��ҳ���õ���ҳ�������ǰ�ˣ�*/
/*	6.ÿ���ڴ�Ĳ�����Ӧ�ø�����Ӧ�Ŀ���ͷҳ��Ϣ*/
/*	7.�ڴ����ԭ�������ѭ�ڴ�����Լ���С���䵥Ԫԭ�򣬲���ҳ�������ǰ�˿�ʼ�����ڴ棬����δ�ָ�����ȡ���յ��ڴ�*/
/*	8.���䣺���������������ڴ棬ѡ��һ���ڴ�ҳ��ѡ��һ�����ʵ�δ�ָ����ڴ棬���з��䣬ÿ������ռ�ʱ��Ҫ�жϵ�ǰ�ڴ�ҳ�Ƿ�����꣬��������Ѹ�ҳ��ŵ��ڴ�ҳ�������������һ���ڵ�*/
/*	9.�ں����Կ����ʵ��ַ��Ϊkey*/

/*	�ڴ��ʹ��׼��															*/
/*	�����ṹ��������Ҫֱ�ӵ��� memorypool��Alloc								*/
/*	���ṹ��������Ҫ����new��delete������ �������е���memorypool��Alloc		*/

//Ϊ������ܣ������ܵļ���new free��ʱ�䣬newʱ��Ӧ�ж��߳��ͷ�lockס�ں�������ס���˽��ں����ڴ�ת�����ɷָ���
//free ʱ�����߳�Ӧ�жϻ��������Ƿ���ס�����������ٻ���ʱ���ʱ��Ƭ��ʱ��Ƭ����ʹ�ø߾���ʱ���


#pragma once

#include <BaseCode/MemoryPoolApi.h>
#include <PoolSrc/MemoryPoolStruct.h>

struct MemPageHead;
typedef gtool::TPtrList<MemPageHead, DEL_WAY_RELEASE, false>		MemPageList;				//�ڴ�ҳ����
class CThreadPool;

class MEMORY_POOL_API CMemPage2
{
	NO_COPY_NO_MOVE(CMemPage2);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CMemPage2);

public:
	//�����ڴ�ҳ
	bool Create(const msize_t nPageId, void *&pBegin, void *&pEnd, const msize_t nByte=MEMPAGE_MINIUM_SIZE);

	//�ڴ����
public:
	void *Alloc(const msize_t nByte, const char *& szErrBuffer);	//�����ڴ�ռ�
	void FreeAlloc(void *pObj);	//�ͷ��ڴ�
	bool IsValid(const void *& p);		//�Ƿ���Ч��ַ
	void *GetBeginOfBuffer();	//�ڴ���ʼ��ַ
	void *GetEndOfBuffer();	//�ڴ������ַ
	msize_t GetSize();	//��ȡ�ڴ�ҳ��С

	//�ں�
public:
	bool LockMerge();
	void MergeMem();
	bool UnlockMerge();
	bool IsLockMerge();
	bool LockBusyMap();
	bool UnlockBusyMap();

	//����
protected:
	bool MoveToMergeMap(MemBlock& rBeginMerge, RecycleMemBlockAreaMap::iterator&IterMap);


	//״̬��Ϣ
protected:
	//WaitForAllocAreaList::Iterator& GetAllocableMaxBlock();		//��ȡ��ǰ�����ڴ�����

private:
	boost::atomic_bool											m_bInit{ false };				//�Ƿ��Ѵ���
	boost::atomic_bool											m_bCleans{ false };				//����
	atmc_sizet											m_atszPageID{ 0 };				//��ǰҳ���
	atmc_sizet													m_atszMaxBlockID{ 0 };			//��ҳ�������ֵ
	boost::atomic_address										m_patszBufferBegin{ NULL };		//�ڴ���ʼ��ַ
	boost::atomic_address										m_patszBufferEnd{ NULL };		//�ڴ���ʼ��ַ
	gtool::CSmartPtr<CDataLocker>								m_pSafeLocker = NULL;			//���ں���������֤�ڴ����Ļ�����
	gtool::CSmartPtr<BusyMemBlockAreaMap>						m_pBusyMap = NULL;				//ʹ�����ڴ������ key���ÿ��ڴ����ʼ��ַ, val:MemBlock
	gtool::CSmartPtr<WaitForAllocAreaList>						m_pAllocAreaList = NULL;		//�������ڴ������
	gtool::CSmartPtr<RecycleMemBlockAreaMap>					m_pRecycleAreaMap = NULL;		//�����ڴ��������� key���ÿ��ڴ����ʼ��ַ, val:MemBlock
	gtool::CSmartPtr<CDataLocker>								m_pMergeMapLocker = NULL;		//�ں�����
	gtool::CSmartPtr<MemBlockMergeAreaMap>						m_pMergeAreaMap = NULL;			//���ں�����map key���ÿ��ڴ����ʼ��ַ, val:MemBlock
	atmc_sizet													m_atszBufferSize{ 0 };			//�ڴ�����С
	gtool::CSmartPtrC<char,char, DEL_WAY_DELETE_MULTI>			m_pPhysicBuffer = NULL;			//�����ڴ�����
};

struct MEMORY_POOL_API MemPageHead
{
	MemPageHead()
	{
		pPage2 = CMemPage2::CreateNew();
	}

	int Release()
	{
		IF_OK(pPage2)
			pPage2->Release();

		return 0;
	}
	atmc_sizet				atszPageID{ 0 };					//ҳ���Ψһ����
	gtool::CSmartPtr<CMemPage2>		pPage2 = NULL;						//�ڴ�ҳ������
	boost::atomic_address			atszAddrBegin{ 0 };					//�ڴ���ʼ
	boost::atomic_address			atszAddrEnd{ 0 };					//�ڴ����
};


class MEMORY_POOL_API CMemPoolMgr
{
	NO_COPY_NO_MOVE(CMemPoolMgr);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CMemPoolMgr);

	friend class CMaintainTask;
	//��ʼ������
public:
	//��ʼ���ڴ��
	bool Init();
	//�ͷ��ڴ��
	void Fini();

public:
	//�����ڴ�ռ�
	void *Alloc(const msize_t& nByte, const char *& szErrBuffer);
	void FreeAlloc(void *pObj);
	bool		IsValidPt(const void* p);

	//����
public:
	bool IsInit()const { return m_bInit; }
	bool IsDestroy()const { return m_bIsDestroy; }

	//�������ջ���
protected:
	bool GCLock();
	bool GCUnlock();
	bool Wait();
	bool Wakeup();
	void MemMergeAll();

protected:
	MemPageHead * CreateNewPage(const msize_t nByte);

private:
	gtool::CSmartPtr<CDataLocker>				m_pLocker = NULL;						//�ڴ��ҳ������
	gtool::CSmartPtr<CDataLocker>				m_pGarbageCollectorGuard = NULL;		//��������
	gtool::CSmartPtr<MemPageList>				m_pMemPageList = NULL;					//�ڴ�ҳ����
	atmc_sizet									m_atszMaxPageID{ 0 };					//���ҳ��id
	boost::atomic_bool							m_bInit{ false };						//��ʼ��״̬
	static boost::atomic_bool					m_bIsDestroy;							//�Ƿ��ͷ�
	boost::atomic<I32>							m_atm32GCPeriod{ MEM_POOL_GC_PERIOD };	//������������
	gtool::CSmartPtr<CThreadPool>				m_pThreadPool=NULL;
	boost::atomic<U64>							m_nAllocCount{ 0 };						//�����ڴ�������
	boost::atomic<U64>							m_nFreeCount{ 0 };						//�ͷŶ������
};

typedef MEMORY_POOL_API gtool::CSingleton<CMemPoolMgr> CCMemPoolMgrInstance;
extern MEMORY_POOL_API CMemPoolMgr *g_pMemPoolMgr;

#undef MemPoolMgr
#define MemPoolMgr (ASSERT(g_pMemPoolMgr), *g_pMemPoolMgr)

#endif



