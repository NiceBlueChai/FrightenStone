#ifndef HEAP_MGR_H_
#define HEAP_MGR_H_

#pragma once
#include <BaseCode/HeapExApi.h>

//#include <BaseCode/MyBaseMacro.h>
//#include <BaseCode/myclassmacro.h>
//#include <map>
//#include <BaseCode/MyDataType.h>
//#include <BaseCode/WinLocker.h>
class CCriticalSectionProtect;



//1�ֽڶ���
#pragma MEM_ALIGN_BEGIN(1)

//������� operator new delete
//ʹ��һ������static ��������ڴ�״�� validptr��������һ��map���жϸò����ڴ��Ƿ������

NAMESPACE_BEGIN(gtool)

struct HEAP_EX_API ObjMemInfo
{
	U32				u32ObjBaseAddress;				//�����׵�ַ
	size_t			szObjSize;						//������ռ�ڴ�ռ�
	unsigned long	idThread;						//�������߳�id
	std::string		strLastUserModuleName;			//���������һ�η����쳣��ģ������
	std::string		strLastUserDetailName;			//���� | ������
	U32				u32LastUserModuleAddress;		//���������һ�η����쳣�ĵ����ߵ�ַ
	I32				i32LastUserProcessId;			//�����������һ�η����쳣�Ľ���id
	ObjMemInfo()
	:u32ObjBaseAddress(0), szObjSize(0), idThread(0), u32LastUserModuleAddress(0), i32LastUserProcessId(0)
	{}
};

//Ҫȷ���̰߳�ȫ ������������Ҫ�Ծ�̬����ڣ��Ա��ڳ����˳�����ģ���ͷ�ʱ���ӡ�ڴ�й¶��Ϣ
class HEAP_EX_API CHeapMgr
{
public:
	CHeapMgr(const char *szClassName = "");
	virtual ~CHeapMgr();

public:
	void*		ApplyMem(size_t szApply);
	void		FreeMem(void* ptr);
	bool		IsValidPt(void* p);

public:
	inline bool GetModuleCallerInfo(void *&pCallerAddress, void *&pModuleHandle, std::string& strName);
	inline void SetLastUserInfo(void *ObjAddress, const I32 i32ProcessId, const U32 u32CallerAddress, const char *szCallerModuleName, const char *szClassName, const char * szFuncName);

public:
	void		Cleans();

protected:
	std::map<U32, ObjMemInfo>	m_PtrMemInfoMap;	//key�������׵�ַ�� value��ObjMemInfo		//����Heap�ͷ�ʱ����ڴ�й¶��Ϣ
	
	std::string					m_ClassName;		//��������
	void *						m_hCurModuleHandle = NULL;	//��ǰ����ģ����������=HANDLE��
	std::string					m_strModuleName;		//ģ����
	I32							m_ProcessId=0;			//����id
	void *						m_pCallerAddress=NULL;	//�����ߵ�ַ
	CCriticalSectionProtect		m_SafeGuardLocker;	//�̰߳�ȫ
	volatile	bool			m_bHeepInit=false;
};



NAMESPACE_END(gtool)

#pragma MEM_ALIGN_END(1)

#endif
