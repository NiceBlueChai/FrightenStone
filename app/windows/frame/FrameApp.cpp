#include "stdafx.h"
#include <FrameApp.h>
#include<AppRoot.h>

//64λ��ȡ·������
#include <tlhelp32.h>  
#include <Psapi.h>  
#pragma comment (lib,"Psapi.lib")  

//appʵ�������ڴ�
CFrameApp theApp;

BEGIN_MESSAGE_MAP(CFrameApp, CWinApp)
END_MESSAGE_MAP()

boost::atomic_bool CFrameApp::m_bFreeResource{ false };
BOOL CFrameApp::InitInstance()
{
	CWinApp::InitInstance();

	//��������
	AfxOleInit();
	AfxInitRichEdit2();
	InitCommonControls();
	AfxEnableControlContainer();

	//ͬһĿ¼ֻ����һ��
	I32 nPidExists = 0;
	if (!AppNotStartedWithSamePath(nPidExists))
	{
		//�����Ѵ��ڵ�app�����ٵ�ǰapp
		if (nPidExists)
		{
			auto hExistsWnd = gtool::GetWindowHwndByPID(nPidExists);
			if(hExistsWnd)
			{
				::BringWindowToTop(hExistsWnd);
				::ShowWindow(hExistsWnd, SW_SHOW);
			}
		}

		return false;
	}
	

	//����Ŀ¼
	TCHAR szDirectory[MAX_PATH] = TEXT("");
	GetWorkDirectory(szDirectory, CountArray(szDirectory));

	//���ñ���
	SetRegistryKey(TEXT("Lab-WinApp"));

	//����Ŀ¼
	SetCurrentDirectory(szDirectory);

	//��������
	WNDCLASS WndClasss;
	ZeroMemory(&WndClasss, sizeof(WndClasss));

	//ע�ᴰ��
	WndClasss.style = CS_DBLCLKS;
	WndClasss.hIcon = LoadIcon(IDI_ICON1);
	WndClasss.lpfnWndProc = DefWindowProc;
	WndClasss.lpszClassName = TEXT("Lab-WinApp");
	WndClasss.hInstance = AfxGetInstanceHandle();
	WndClasss.hCursor = LoadStandardCursor((const char*)(MAKEINTRESOURCE(IDC_ARROW)));
	if (AfxRegisterClass(&WndClasss) == false) AfxThrowResourceException();
	m_pMainWnd = &m_DlgRoot;
	m_bFreeResource = false;

	SEH_TRY
	{
		auto nRet = m_DlgRoot.DoModal();

		//�����̳߳�
		m_DlgRoot.DestroyPool();

		ASSERTEX((nRet >= 0));
		ASSERTEX(FreeResource());		
	}
	SEH_FULLINFO_CATCH("BOOL CFrameApp::InitInstance()")
	{
		ASSERTEX(AppRootObj.Fini(false));
		LOGCRASH(GET_SEHCATCH_BUFFER_STRING());
		ASSERTEX(FreeResource());		
		return false;
	}
	SEH_CATCH_END

	return true;
}

int CFrameApp::ExitInstance()
{
	ASSERTEX(FreeResource());

	return CWinApp::ExitInstance();
}

bool CFrameApp::FreeResource()
{
	if (m_bFreeResource)
		return true;

	SEH_TRY
	{
		ASSERTEX(THREAD_TRANSFER::FiniThreadPostModule());				//��Ϣ����
		LOGSYS_FMT_C4096("ThreadPost Module Fini.");
	}
		SEH_FULLINFO_CATCH("int CFrameApp::ExitInstance() FreeResource FiniThreadPostModule")
	{
		LOGCRASH(GET_SEHCATCH_BUFFER_STRING());
	}
	SEH_CATCH_END

	SEH_TRY
	{
		gtool::MemPoolFiniModule();					//�ڴ��
		LOGSYS_FMT_C4096("MemPoolFiniModule Module Fini.");
	}
		SEH_FULLINFO_CATCH("int CFrameApp::ExitInstance() FreeResource MemPoolFiniModule")
	{
		LOGCRASH(GET_SEHCATCH_BUFFER_STRING());
	}
	SEH_CATCH_END	

	SEH_TRY
	{
		IF_NOT(gtool::FinishLogModule())			//log����ͷ�
		LOGGEN("gtool::FinishLogModule() ERR!");
	}
	SEH_FULLINFO_CATCH("int CFrameApp::ExitInstance() FreeResource FinishLogModule")
	{
		ASSERT(false);
		//LOGCRASH(GET_SEHCATCH_BUFFER_STRING());
	}
	SEH_CATCH_END

	m_bFreeResource = true;

	return true;
}

//����Ŀ¼
bool CFrameApp::GetWorkDirectory(TCHAR szWorkDirectory[], WORD wBufferCount)
{
	memset(szWorkDirectory, 0, wBufferCount * sizeof(TCHAR));

	//ģ��·��
	TCHAR szModulePath[MAX_PATH] = TEXT("");
	GetModuleFileName(AfxGetInstanceHandle(), szModulePath, CountArray(szModulePath));

	//�����ļ�
	for (INT i = lstrlen(szModulePath); i >= 0; i--)
	{
		if (szModulePath[i] == TEXT('\\'))
		{
			szModulePath[i] = 0;
			break;
		}
	}

	//���ý��
	ASSERT(szModulePath[0] != 0);
	lstrcpyn(szWorkDirectory, szModulePath, wBufferCount);

	return true;
}

bool CFrameApp::AppNotStartedWithSamePath(I32& nPidExists)
{
	nPidExists = 0;

	//������ͬһ��Ŀ¼App������
	BUFFER512 szPath = { 0 };
	CHECKF_NL(gtool::GetProcPath(true, _getpid(), szPath, sizeof(szPath)));

	CHECKF_NL(strlen(szPath) > 0);

	//��������ӳ����
	auto hProcModule = gtool::CreateProcessSnapshot();
	CHECKF_NL(hProcModule);
	BUFFER512 szPathLoop = { 0 };
	auto nPid = gtool::GetFirstProcessPid(hProcModule);
	bool bFirst = true;
	for (; bFirst? bFirst:(nPid > 0); nPid = gtool::GetNextProcessPid(hProcModule))
	{
		bFirst = false;
		memset(szPathLoop, 0, sizeof(szPathLoop));
		//ASSERT(nPid != 11868);
		if(!gtool::GetProcPath(false, nPid, szPathLoop, sizeof(szPathLoop)))
			continue;

		if (strcmp(szPath, szPathLoop) == 0
			&& nPid!=_getpid())
		{
			nPidExists = nPid;
			return false;
		}			
	}

	return true;
}


