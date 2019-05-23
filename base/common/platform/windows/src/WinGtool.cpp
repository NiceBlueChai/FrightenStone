#include <BaseCode/Win/WinGtool.h>


#if defined(NEED_WINDOWS_COMMON_API)



NAMESPACE_BEGIN(gtool)

//windowsƽ̨���뼶
time_t InsertCurrentTime(gtool::CFmtIOString& fos)								//ʱ���
{
	//��ȡʱ��
	SYSTEMTIME tm_now = { 0 };
	GetLocalTime(&tm_now);

	auto nStrLen = sprintf(fos.GetBuffer(), "%hu.%02hu.%02hu %02hu:%02hu:%02hu.%03hu ",
		tm_now.wYear, tm_now.wMonth, tm_now.wDay, tm_now.wHour, tm_now.wMinute, tm_now.wSecond, tm_now.wMilliseconds);
	nStrLen = ((nStrLen < BUFFER_LEN32) ? gtool::Max(nStrLen, 0) : (BUFFER_LEN32 - 1));
	fos.GetBuffer()[nStrLen] = 0;
	fos << fos.GetBuffer();
	fos.GetBuffer()[0] = 0;

	tm	tmMkTime;
	memset(&tmMkTime, 0, sizeof(tm));
	tmMkTime.tm_year = tm_now.wYear - 1900;
	tmMkTime.tm_mon = tm_now.wMonth - 1;
	tmMkTime.tm_mday = tm_now.wDay;
	tmMkTime.tm_hour = tm_now.wHour;
	tmMkTime.tm_min = tm_now.wMinute;
	tmMkTime.tm_sec = tm_now.wSecond;

	return mktime(&tmMkTime);
}



//��ȡ�ڴ���Ϣ
void GetMemoryInfo(MEMORYSTATUS& MemInfo)
{
	MemInfo.dwLength = sizeof(MemInfo);
	GlobalMemoryStatus(&MemInfo);
}

msize_t GetMemPoolAlignByte(const msize_t&nByte, msize_t nPoolRestByte)
{
	//ʹ�������ڴ�ʣ���С
	if (nPoolRestByte < 0)
	{
		MEMORYSTATUS rMemInfo;
		gtool::GetMemoryInfo(rMemInfo);
		nPoolRestByte = msize_t(rMemInfo.dwAvailPhys) - PYSICAL_MEM_REMAIN_BYTE;
	}
	CHECKF_NL(nPoolRestByte >= 0);

	//�����ڴ�ʣ���С
	msize_t nNeededByte = ((nByte% MEMBLOCK_MINIUM_SIZE) != 0) ? ((nByte / MEMBLOCK_MINIUM_SIZE + 1)*MEMBLOCK_MINIUM_SIZE) : nByte;
	nNeededByte = ((nNeededByte > I64(nPoolRestByte)) ? (I64(nPoolRestByte) / MEMBLOCK_MINIUM_SIZE * MEMBLOCK_MINIUM_SIZE) : nNeededByte);
	return nNeededByte;
}

msize_t GetSysAlignByte(const msize_t&nByte)
{
	//�����ڴ�ʣ���С
	msize_t nNeededByte = ((nByte% MEMBLOCK_MINIUM_SIZE) != 0) ? ((nByte / MEMBLOCK_MINIUM_SIZE + 1)*MEMBLOCK_MINIUM_SIZE) : nByte;
	MEMORYSTATUS rMemInfo;
	gtool::GetMemoryInfo(rMemInfo);
	I64 nSysTemMemRest = I64(rMemInfo.dwAvailPhys) - I64(PYSICAL_MEM_REMAIN_BYTE);//�����ϵͳ�ڴ�������
	CHECKF_NL(nSysTemMemRest >= 0);

	nNeededByte = ((nNeededByte > I64(nSysTemMemRest)) ? (I64(nSysTemMemRest) / MEMBLOCK_MINIUM_SIZE * MEMBLOCK_MINIUM_SIZE) : nNeededByte);
	return nNeededByte;
}

typedef   void(WINAPI*   FunctionGlobalMemoryStatusEx)(LPMEMORYSTATUS);//��������ԭ��ָ��

msize_t GetAvailProcessSize()
{
	HMODULE hModule;
	FunctionGlobalMemoryStatusEx GlobalMemoryStatusEx;
	MEMORYSTATUS status;
	status.dwLength = sizeof(status);
	hModule = LoadLibrary("kernel32.dll");//���붯̬���ӿ�kernel32.dll���������ľ��
	IF_NOT_NL (hModule)//�ж�����dll�Ƿ�ɹ�
		return -1;

	//��kernel32.dll��������GlobalMemoryStatusEx���������ظĺ�����ָ��
	GlobalMemoryStatusEx = (FunctionGlobalMemoryStatusEx)GetProcAddress(hModule, "GlobalMemoryStatusEx");
	IF_NOT_NL (GlobalMemoryStatusEx)//�жϺ����Ƿ��ҵ�
		return -1;

	GlobalMemoryStatusEx(&status);//���ú���ȡ��ϵͳ���ڴ����
	FreeLibrary(hModule);//�ͷ����ӿ���
	return status.dwAvailVirtual;
}

msize_t GetTotalProcessSize()
{
	HMODULE hModule;
	FunctionGlobalMemoryStatusEx GlobalMemoryStatusEx;
	MEMORYSTATUS status;
	status.dwLength = sizeof(status);
	hModule = LoadLibrary("kernel32.dll");//���붯̬���ӿ�kernel32.dll���������ľ��
	IF_NOT_NL(hModule)//�ж�����dll�Ƿ�ɹ�
		return -1;

	//��kernel32.dll��������GlobalMemoryStatusEx���������ظĺ�����ָ��
	GlobalMemoryStatusEx = (FunctionGlobalMemoryStatusEx)GetProcAddress(hModule, "GlobalMemoryStatusEx");
	IF_NOT_NL(GlobalMemoryStatusEx)//�жϺ����Ƿ��ҵ�
		return -1;

	GlobalMemoryStatusEx(&status);//���ú���ȡ��ϵͳ���ڴ����
	FreeLibrary(hModule);//�ͷ����ӿ���
	return status.dwTotalVirtual;
}

//��ȡ����·��
bool GetProcPath(
	__in	bool	bCurrentProc,
	__in	MYULONG	ulPid,
	__inout LPTSTR	lpProcPath,
	__in	MYULONG	ulProcPathBufLen
)
{
	bool	bRet = false;

	HMODULE hModule = NULL;
	HANDLE	hProc = NULL;
	DWORD	dwProcPathLen = 0;
	I8		tchProcPathDev[MAX_PATH] = { 0 };
	I8		tchVolNameDev[MAX_PATH] = { 0 };
	I8		tchVolName[MAX_PATH] = { 0 };


	__try
	{
		if (!lpProcPath || !ulProcPathBufLen || (!bCurrentProc && !ulPid))
		{
			//ASSERT(!"[GetProcPath] : input parameter error \n");
			__leave;
		}

		if (bCurrentProc)
		{
			if (!GetModuleFileName(NULL, lpProcPath, ulProcPathBufLen / sizeof(I8)))
			{
				//ASSERT(!"[GetProcPath] : GetModuleFileName failed err[", GetLastError(), "].");
				ASSERT(!"[GetProcPath] : GetModuleFileName failed err[");
				__leave;
			}

			bRet = true;
			__leave;
		}

		hProc = OpenProcess(PROCESS_QUERY_INFORMATION, false, ulPid);
		if (!hProc)
		{
			//ASSERTEX("[GetProcPath] : OpenProcess failed err[", GetLastError(), "].");
			//ASSERT(!"[GetProcPath] : OpenProcess failed err[");
			__leave;
		}

		hModule = LoadLibrary(_T("Kernel32.dll"));
		if (!hModule)
		{
			//ASSERTEX("[GetProcPath] : LoadLibrary failed. err[", GetLastError(), "].");
			ASSERT(!"[GetProcPath] : LoadLibrary failed. err[");
			__leave;
		}

		if (GetProcAddress(hModule, "QueryFullProcessImageNameA"))	//��ȡQueryFullProcessImageNameA����
		{
			dwProcPathLen = ulProcPathBufLen / sizeof(I8);
			if (!QueryFullProcessImageName(hProc, 0, lpProcPath, &dwProcPathLen))
			{
				//ASSERTEX("[GetProcPath] : QueryFullProcessImageName failed. err[", GetLastError(), "].");
				ASSERT(!"[GetProcPath] : QueryFullProcessImageName failed. err[");
				__leave;
			}

			bRet = true;
			__leave;
		}

		if (!GetProcessImageFileName(hProc, tchProcPathDev, MAX_PATH))	//��ȡ����·��
		{
			//ASSERTEX("[GetProcPath] : GetProcessImageFileName failed err [", GetLastError(), "].");
			ASSERT(!"[GetProcPath] : GetProcessImageFileName failed err [");
			__leave;
		}

		_tcscat_s(tchVolName, MAX_PATH, _T("A:"));
		for (; *tchVolName <= _T('Z'); (*tchVolName)++)
		{
			memset(tchVolNameDev, 0, MAX_PATH * sizeof(I8));
			if (!QueryDosDevice(tchVolName, tchVolNameDev, MAX_PATH))	//��ȡ�̷�
			{
				if (GetLastError() == 2)
					continue;
				else
				{
					//ASSERT("[GetProcPath] : QueryDosDevice failed. err[", GetLastError(), "].");
					ASSERT(!"[GetProcPath] : QueryDosDevice failed. err[");
					__leave;
				}
			}

			if (_tcsncmp(tchProcPathDev, tchVolNameDev, _tcslen(tchVolNameDev)) == 0)	//����ӳ������
			{
				bRet = true;
				break;
			}
		}

		if (bRet)
		{
			_tcscat_s(lpProcPath, ulProcPathBufLen / sizeof(I8), tchVolName);	//�̷�
			_tcscat_s(lpProcPath, ulProcPathBufLen / sizeof(I8), tchProcPathDev + _tcslen(tchVolNameDev));	//�̷����·��
		}
	}
	__finally
	{
		if (hModule)
			FreeLibrary(hModule);

		if (hProc)
			CloseHandle(hProc);
	}

	return bRet;
}

//�����������
 HANDLE CreateProcessSnapshot()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		return NULL;
	}

	return hSnapshot;
}

I32 GetFirstProcessPid(HANDLE& hSnapshot)
{
	CHECKF_NL(hSnapshot);
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(pe);

	CHECKF_NL(Process32First(hSnapshot, &pe));

	return pe.th32ProcessID;
}

I32 GetNextProcessPid(HANDLE& hSnapshot)
{
	CHECKF_NL(hSnapshot);
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(pe);
	if (!Process32Next(hSnapshot, &pe))
		return 0;

	return pe.th32ProcessID;
}


///< ö�ٴ��ڻص�����
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	EnumWindowsArg *pArg = (EnumWindowsArg *)lParam;
	DWORD  dwProcessID = 0;
	// ͨ�����ھ��ȡ�ý���ID
	::GetWindowThreadProcessId(hwnd, &dwProcessID);
	if (dwProcessID == pArg->dwProcessID)
	{
		pArg->hwndWindow = hwnd;
		// �ҵ��˷���FALSE
		return FALSE;
	}
	// û�ҵ��������ң�����TRUE
	return TRUE;
}

///< ͨ������ID��ȡ���ھ��
HWND GetWindowHwndByPID(DWORD dwProcessID)
{
	HWND hwndRet = NULL;
	EnumWindowsArg ewa;
	ewa.dwProcessID = dwProcessID;
	ewa.hwndWindow = NULL;
	EnumWindows(EnumWindowsProc, (LPARAM)&ewa);
	if (ewa.hwndWindow)
	{
		hwndRet = ewa.hwndWindow;
	}
	return hwndRet;
}


NAMESPACE_END(gtool)

#endif
