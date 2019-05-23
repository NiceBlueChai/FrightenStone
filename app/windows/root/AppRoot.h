#ifndef APP_ROOT_H_
#define APP_ROOT_H_

#pragma once

//1.һϵ�б�Ҫ�ĳ�ʼ��
//app����
//app�ȴ�
//app������һϵ�е��ͷţ�

//BusinessLogic api
typedef bool(*pfnInitBusinessLogicModule)();
typedef bool(*pfnFiniBusinessLogicModule)();
typedef const void* (*pfnGetBusinessLogicMainThread)();

class CAppRoot
{
	NO_COPY_NO_MOVE(CAppRoot);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CAppRoot);

public:
	bool Init();
	bool Fini(bool bShowInWindow = true);

	//����
protected:
	bool ConfigSvrAddr();
	bool InitLogModule();

private:
	boost::atomic_bool				m_bInit{ false };
	static boost::atomic_bool		m_bDestroy;

};

//����ģʽ
typedef gtool::CSingleton<CAppRoot> AppRootInstance;
#define AppRootObj (*AppRootInstance::GetInstance())


#endif

