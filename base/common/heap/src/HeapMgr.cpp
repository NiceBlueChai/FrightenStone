#include "stdafx.h"
#include <HeapMgr.h>
#include<tchar.h>
#include <intrin.h>
#include <process.h>
#include<thread>
#include <iostream>
#include <BaseCode/DataCheckMacro.h>


#define HEAP_MGR_APPLY_OBJ_INFO_FMT_STR		"[err]:ģ�����ƣ�[%s] �������ڽ���id:[%d] ���������߳�id��[%lu] ������[%s] ��ǰ����������׵�ַ��[%x] �����ڴ��С��[%I64u]Byte ���һ�εĽ���id:[%d] ���һ��ģ������[%s] ���һ�ε����ߵ�ַ��[%x] ���һ�������뺯����:[%s]"
#define ANSI_NAME_MAX	(MAX_PATH*2)

gtool::CHeapMgr::CHeapMgr(const char *szClassName /*= ""*/)
{
	ASSERTEX(m_SafeGuardLocker.Lock());
	if (!m_bHeepInit)
	{
		m_bHeepInit = true;
		ASSERTEX(GetModuleCallerInfo(m_pCallerAddress, m_hCurModuleHandle, m_strModuleName));
		m_ClassName = szClassName;
		m_PtrMemInfoMap.clear();
		m_ProcessId = _getpid();
	}
	ASSERTEX(m_SafeGuardLocker.Unlock());
}

gtool::CHeapMgr::~CHeapMgr()
{
	Cleans();
}

void* gtool::CHeapMgr::ApplyMem(size_t szApply)
{
	ASSERTEX(m_SafeGuardLocker.Lock());
	auto pApply = malloc(szApply);
	if (pApply)
	{
		ObjMemInfo stCreateInfo;
		stCreateInfo.idThread = GetCurrentThreadId();
		stCreateInfo.u32ObjBaseAddress = reinterpret_cast<U32>(pApply);
		stCreateInfo.szObjSize = szApply;
		stCreateInfo.strLastUserModuleName = m_strModuleName.c_str();
		stCreateInfo.u32LastUserModuleAddress = 0;	
		stCreateInfo.i32LastUserProcessId = m_ProcessId;
		stCreateInfo.strLastUserDetailName += m_ClassName.c_str();
		stCreateInfo.strLastUserDetailName += ":CHeapMgr";
		stCreateInfo.strLastUserDetailName += "| ApplyMem(size_t szApply)";
		stCreateInfo.u32LastUserModuleAddress = reinterpret_cast<U32>(m_pCallerAddress);

		auto IterMap=m_PtrMemInfoMap.find(stCreateInfo.u32ObjBaseAddress);
		if (IterMap != m_PtrMemInfoMap.end())
		{
			std::string FmtObj(HEAP_MGR_APPLY_OBJ_INFO_FMT_STR);
			CFmtIOString strErr;
			strErr << "����������������������ǰ���ڴ���һ���Ѿ��������Ķ��� ֮ǰ������ܴ���й¶���գ���������������������" << CFmtIOString::endl;
			strErr << "[֮ǰ������Ϣ]:";
			auto& rBefore = IterMap->second;
			strErr.FormatPro(FmtObj, m_strModuleName.c_str(), m_ProcessId, rBefore.idThread, m_ClassName.c_str(), rBefore.u32ObjBaseAddress
				, rBefore.szObjSize, rBefore.i32LastUserProcessId,  rBefore.strLastUserModuleName.c_str()
				, rBefore.u32LastUserModuleAddress, rBefore.strLastUserDetailName.c_str());

			strErr << "\r\n";

			strErr << "[���ڴ����Ķ�����Ϣ]:";
			strErr.FormatPro(FmtObj
				, m_strModuleName.c_str(), m_ProcessId, stCreateInfo.idThread, m_ClassName.c_str(), stCreateInfo.u32ObjBaseAddress
				, stCreateInfo.szObjSize, stCreateInfo.i32LastUserProcessId, stCreateInfo.strLastUserModuleName.c_str()
				, stCreateInfo.u32LastUserModuleAddress, stCreateInfo.strLastUserDetailName.c_str());

			strErr << "��������������������[END] "<< m_ClassName.c_str()<< " ��������������������"<< CFmtIOString::endl;

			ASSERTEX( !strErr.c_str() );
			m_PtrMemInfoMap.erase(stCreateInfo.u32ObjBaseAddress);
		}

		m_PtrMemInfoMap.insert(std::make_pair(stCreateInfo.u32ObjBaseAddress, stCreateInfo));

		IterMap=m_PtrMemInfoMap.find(stCreateInfo.u32ObjBaseAddress);
		ASSERTEX(IterMap != m_PtrMemInfoMap.end());
	}
	ASSERTEX(m_SafeGuardLocker.Unlock());

	return pApply;
}

void gtool::CHeapMgr::FreeMem(void* ptr)
{
	CHECK(ptr);
	U32 u32Ptr = reinterpret_cast<U32>(ptr);
	free(ptr);
	ASSERTEX(m_SafeGuardLocker.Lock());
	auto IterMap = m_PtrMemInfoMap.find(u32Ptr);
	if(IterMap == m_PtrMemInfoMap.end())
	{
	
		LOGMEMLEAK("ģ����:[%s] ����id:[%d] �߳�id[%lu] ��ǰ�ͷ�һ������:��ַ:[%p] �ö����Ѿ�����Heap�й���"
			, m_strModuleName.c_str(), m_ProcessId, GetCurrentThreadId(), ptr);

		ASSERTEX("��ǰ�ͷ�һ�����󣬸ö����Ѿ�����Heap�й���");
		ASSERTEX(m_SafeGuardLocker.Unlock());
		return;
	}
	m_PtrMemInfoMap.erase(u32Ptr);
	ASSERTEX(m_SafeGuardLocker.Unlock());
}

bool gtool::CHeapMgr::IsValidPt(void* p)
{
	if (p == NULL)return false;

	ASSERTEX(m_SafeGuardLocker.Lock());
	U32 u32Ptr = reinterpret_cast<U32>(p);
	auto IterMap = m_PtrMemInfoMap.find(u32Ptr);
	if (IterMap == m_PtrMemInfoMap.end())
	{
		ASSERTEX(m_SafeGuardLocker.Unlock());
		return false;
	}
	ASSERTEX(m_SafeGuardLocker.Unlock());

	return true;
}

inline bool gtool::CHeapMgr::GetModuleCallerInfo(void *&pCallerAddress, void *&pModuleHandle, std::string& strName)
{
	pCallerAddress = NULL;
	pModuleHandle = NULL;

	//��ȡ��ǰ����ģ����Ϣ
	HMODULE hCallerModule = NULL;
	TCHAR szModuleName[MAX_PATH] = _T("");
	void *callerAddress = _ReturnAddress();
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)callerAddress, &hCallerModule))
	{
		GetModuleFileName(hCallerModule, szModuleName, ARRAYSIZE(szModuleName));
		pModuleHandle = (void *)(hCallerModule);
		pCallerAddress = callerAddress;

		strName = szModuleName;
		return true;
	}

	return false;
}

//���һ��ʹ�ø����λ����Ϣ ��ʹ����֮ǰ������ʹ�øõ����Ա����й¶λ��
inline void gtool::CHeapMgr::SetLastUserInfo(void *ObjAddress, const I32 i32ProcessId, const U32 u32CallerAddress,  const char *szCallerModuleName, const char *szClassName, const char * szFuncName)
{
	U32 u32ObjAddress = reinterpret_cast<U32>(ObjAddress);
	ASSERTEX(m_SafeGuardLocker.Lock());
	auto IterMap = m_PtrMemInfoMap.find(u32ObjAddress);
	IF_OK(IterMap != m_PtrMemInfoMap.end())
	{
		auto& rInfo = IterMap->second;
		rInfo.strLastUserModuleName = szCallerModuleName;
		rInfo.i32LastUserProcessId= i32ProcessId;
		rInfo.u32LastUserModuleAddress = U32(u32CallerAddress);
		rInfo.strLastUserDetailName += szClassName;
		rInfo.strLastUserDetailName += "|";
		rInfo.strLastUserDetailName += szFuncName;
	}
	ASSERTEX(m_SafeGuardLocker.Unlock());
}

void gtool::CHeapMgr::Cleans()
{
	//й¶�ڴ浼����־
	ASSERTEX(m_SafeGuardLocker.Lock());
	if (!m_PtrMemInfoMap.empty())
	{
		std::string FmtObj(HEAP_MGR_APPLY_OBJ_INFO_FMT_STR);
		CFmtIOString strErr;
		strErr << "��������������������		�����ڴ�й¶��		��������������������"<< CFmtIOString::endl;
		U64 u64TotalByte = 0, nObjCount=0;
		for (auto IterMap = m_PtrMemInfoMap.begin(); IterMap != m_PtrMemInfoMap.end(); ++IterMap)
		{
			auto&rInfo = IterMap->second;
			strErr.FormatPro(FmtObj
				, m_strModuleName.c_str(), m_ProcessId, rInfo.idThread, m_ClassName.c_str(), rInfo.u32ObjBaseAddress
				, rInfo.szObjSize, rInfo.i32LastUserProcessId, rInfo.strLastUserModuleName.c_str()
				, rInfo.u32LastUserModuleAddress, rInfo.strLastUserDetailName.c_str());
			u64TotalByte += rInfo.szObjSize;
			++nObjCount;
			strErr << CFmtIOString::endl;
		}
		strErr <<"й¶����["<< m_ClassName.c_str()<<"] й¶�������["<< nObjCount<< "] й¶���ֽ���[" << u64TotalByte << "] Byte" << CFmtIOString::endl;
		strErr << "��������������������	[END]		��������������������" << CFmtIOString::endl;
		LOGMEMLEAK("%s", strErr.c_str());
		m_PtrMemInfoMap.clear();
	}


	ASSERTEX(m_SafeGuardLocker.Unlock());
}
