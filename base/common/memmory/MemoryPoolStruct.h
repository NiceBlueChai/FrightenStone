#ifndef MEMORY_POOL_STRUCT_H_
#define MEMORY_POOL_STRUCT_H_

#pragma once

#include <BaseCode/MemoryPoolApi.h>
class CMemPage2;


struct MEMORY_POOL_API MemBlock
{
	MemBlock()
	:atszPageID(0), atszBlockID(0), pAddrBegin(NULL), pAddrEnd(NULL)
	{
		szErrBuffer[0] = 0;
	}

	atmc_sizet atszPageID{ 0 };			//ҳ���
	atmc_sizet atszBlockID{ 0 };			//����
	atmc_memaddr	pAddrBegin{ NULL };			//��ʼ��ַ
	atmc_memaddr	pAddrEnd{ NULL };			//������ַ
	BUFFER256		szErrBuffer;				//�ڴ�й¶��Ϣ
	msize_t GetSize()
	{
		if (pAddrBegin == 0 || pAddrEnd == 0)
			return 0;

		void *pBegin = pAddrBegin;
		void *pEnd = pAddrEnd;

		return msize_t(pEnd) - msize_t(pBegin);
	}
};

//�ڴ���ܿ���ͷ
struct MEMORY_POOL_API MemPoolControlHead
{
	atmc_sizet atszTotalPageNums{ 0 };				//�ڴ�ҳ��
	atmc_sizet atszTotalMemSize{ 0 };				//�ڴ��ܴ�С
	atmc_sizet atszCurUsed{ 0 };					//�����ڴ��С
};

typedef MEMORY_POOL_API gtool::CLitPtrMap<MemBlock, U32, DEL_WAY_DELETE>	BusyMemBlockAreaMap;		//ʹ�����ڴ��map key���ÿ��ڴ����ʼ��ַ, val:MemBlock
typedef MEMORY_POOL_API gtool::TPtrList<MemBlock, DEL_WAY_DELETE, false>	BusyMemBlockAreaList;		//ʹ�����ڴ������
typedef MEMORY_POOL_API gtool::TPtrList<MemBlock, DEL_WAY_DELETE, false>	WaitForAllocAreaList;		//���������ڴ������
typedef MEMORY_POOL_API gtool::CLitPtrMap<MemBlock, U32, DEL_WAY_DELETE>	RecycleMemBlockAreaMap;		//������map key���ÿ��ڴ����ʼ��ַ, val:MemBlock
typedef MEMORY_POOL_API gtool::CLitPtrMap<MemBlock, U32, DEL_WAY_DELETE>	TempMemBlockShiftMap;		//���������ں������ɣ�Ӧ����Ϊ�ֲ����󣬱�����Դ����
typedef MEMORY_POOL_API gtool::CLitPtrMap<MemBlock, U32, DEL_WAY_DELETE>	MemBlockMergeAreaMap;		//�ڴ���ں���map key���ÿ��ڴ����ʼ��ַ, val:MemBlock


#endif

