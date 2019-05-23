#include "stdafx.h"
#include <resource.h>
#include <AppRoot.h>
#include <DlgRoot.h>
#include <BaseCode/Win/IniFileIO.h>
//#include <BaseCode/ThreadPool.h>

//#include <BaseCode/MemoryPool.inl>
boost::atomic_bool CAppRoot::m_bDestroy{ false };
CAppRoot::CAppRoot()
{
}

CAppRoot::~CAppRoot()
{
}

void CAppRoot::Cleans()
{
}

bool CAppRoot::Init()
{	
	SEH_TRY
	{
		//log��ʼ��
		//CHECKF_NL(LogInitHelper<LOG_NUM_MAX>::InitLog());	//log��ʼ��
		IF_NOT_NL(InitLogModule())
		{
			TestDisPlay("Log Module ��ʼ��ʧ��.");
			return false;
		}
		TestDisPlay("Log Module ��ʼ���ɹ�.");
		LOGSYS_FMT_C4096("Log Module ��ʼ���ɹ�.");

		IF_NOT(gtool::MemPoolInitModule())				//�ڴ��	
		{
			TestDisPlay("MemPool Module ��ʼ��ʧ��.");
			return false;
		}
		TestDisPlay("MemPool Module ��ʼ���ɹ�.");
		LOGSYS_FMT_C4096("MemPool Module ��ʼ���ɹ�.");

		IF_NOT_NL(THREAD_TRANSFER::InitThreadPostModule())
		{
			TestDisPlay("ThreadPost Module ��ʼ��ʧ��.");
			return false;
		}
		TestDisPlay("ThreadPost Module ��ʼ���ɹ�.");
		LOGSYS_FMT_C4096("ThreadPost Module ��ʼ���ɹ�.");		

		//��������ַ����
		IF_NOT(ConfigSvrAddr())
		{
			TestDisPlay("ConfigSvrAddr ʧ��.");
			return false;
		}
		TestDisPlay("ConfigSvrAddr �ɹ�.");
		LOGSYS_FMT_C4096("ConfigSvrAddr �ɹ�.");

		IF_NOT(gtool::NetWorkInitModule())				//iocp����ģ���ʼ��
		{
			TestDisPlay("NetWork Module ��ʼ��ʧ��.");
			return false;
		}
		TestDisPlay("NetWork Module ��ʼ���ɹ�.");
		LOGSYS_FMT_C4096("NetWork Module ��ʼ���ɹ�.");

		IF_NOT(InitBusinessLogicModule())
		{
			TestDisPlay("BusinessLogic Module ��ʼ��ʧ��.");
			return false;
		}
		TestDisPlay("BusinessLogic Module ��ʼ���ɹ�.");
		LOGSYS_FMT_C4096("BusinessLogic Module ��ʼ���ɹ�.");

		//����ҵ���߼��߳�ʵ��
//		ASSERTEX(THREAD_TRANSFER::SetDisplayFun(GetBusinessLogicMainThread(), POST_OBJ_TYPE_MAIN_THREAD, &DlgDisplay));	//������ʾ
//		ASSERTEX(THREAD_TRANSFER::SetDisplayFun(gtool::NetWorkModulGetSenderThread(), POST_OBJ_TYPE_NETWORK_SENDER, &DlgDisplay)); //������ʾ
		gtool::NetWorkModulSetGetBusinessLogicMainThread(GetBusinessLogicMainThread());	//����ģ�����ҵ���߼�
		BusinessLogicSetNetworkSenderThread(gtool::NetWorkModulGetSenderThread());	//ҵ���߼�ģ���������ģ��

		m_bInit = true;
		m_bDestroy = false;
	}
	SEH_FULLINFO_CATCH("bool CAppRoot::Init()")
	{
		LOGCRASH(GET_SEHCATCH_BUFFER_STRING());
		m_bInit = false;
		return false;
	}
	SEH_CATCH_END

	return true;
}

//һϵ�е��ͷ� ���ʼ���෴���ȳ�ʼ���ĺ�finish
bool CAppRoot::Fini(bool bShowInWindow /*= true*/)
{
	SEH_TRY
	{
		if (m_bDestroy)
			return true;

		m_bDestroy = true;
		m_bInit = false;
		ASSERTEX(FiniBusinessLogicModule());					//ҵ��ģ���ͷ�
		if(bShowInWindow)
			TestDisPlay("BusinessLogic Module Fini.");

		LOGSYS_FMT_C4096("BusinessLogic Module Fini.");
	}
	SEH_FULLINFO_CATCH("bool CAppRoot::Fini() FiniBusinessLogicModule")
	{
		LOGCRASH(GET_SEHCATCH_BUFFER_STRING());
	}
	SEH_CATCH_END

	SEH_TRY
	{
		m_bInit = false;
		gtool::NetWorkFinishModule();					//����ģ���ͷ�
		if (bShowInWindow)
			TestDisPlay("NetWork Module Fini.");
		LOGSYS_FMT_C4096("NetWork Module Fini.");
	}
	SEH_FULLINFO_CATCH("bool CAppRoot::Fini() NetWorkFinishModule")
	{
		LOGCRASH(GET_SEHCATCH_BUFFER_STRING());
	}
	SEH_CATCH_END

	return true;
}

bool CAppRoot::ConfigSvrAddr()
{
	CHECKF_NL(gtool::IsLogModuleInit());

	//���÷�������ַ
	gtool::NetWorkModulSetIp("");
	BUFFER512 szConfigPath = { 0 };

	//��ȡ�����ļ�����·��
	CHECKF(gtool::GetProcPath(true, _getpid(), szConfigPath, sizeof(szConfigPath)));
	auto strConfig = gtool::CFileDirSys::GetFilePathInWholePath(szConfigPath);
	strConfig += "SvrConfig.ini";

	//�����ļ���ʼ��
	CIniFileIO IniConfig;
	CHECKF(IniConfig.SetFilePath(strConfig.c_str()));

	//��ȡ����
	BUFFER128 szip = { 0 };
	char *pip = szip;
	U32 nPort = 0;
	IniConfig.ReadString("AddrInfo", "ip", "", pip, sizeof(szip));
	nPort = IniConfig.ReadInt("AddrInfo", "port", 0);

	//����ip
	if (strlen(pip) > 0)
		gtool::NetWorkModulSetIp(pip);

	//���ö˿�
	if ((nPort > 0))
		gtool::NetWorkModulSetSvrPort(nPort);

	return true;
}

bool CAppRoot::InitLogModule()
{
	CHECKF_NL(gtool::InitLogModule());
	CHECKF_NL(LogInitHelper<LOG_NUM_MAX>::InitLog());

	return true;
}

