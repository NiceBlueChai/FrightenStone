
#include <stdarg.h>
//#include <BaseCode/FmtIOString.h>

//���߳̽ӿ�
void gtool::SetTimerFlush(int nTimeDiff)
{
	LogMgr2.SetTimerFlush(nTimeDiff);
}

void gtool::FlushLog()
{
	//����flushlog
	LogMgr2.LockRes();
	LogMgr2.FlushLog();
	LogMgr2.UnLockRes();
}

bool gtool::AddLog(const I32 nFileIndex, const char *szLogPath, const char *szFileName)
{
	if (!LogMgr2.IsInit())
	{
		CHECKF_NL(LogMgr2.Init());
	}

	return LogMgr2.AddLog(nFileIndex, szLogPath, szFileName);
}

bool gtool::InitLogModule()
{
	if (LogMgr2.IsInit())
		return true;

	CHECKF_NL(LogMgr2.Init());

	return true;
}

bool gtool::FinishLogModule()
{
	return LogMgr2.Fini();
}

bool gtool::IsLogModuleInit()
{
	return LogMgr2.IsInit();
}

bool gtool::IsLogModuleFniDone()
{
	ASSERT(LogMgr2.LockRes());
	auto bFini = LogMgr2.IsFiniDone();
	ASSERT(LogMgr2.UnLockRes());
	return bFini;
}

// CFmtIOString* pData ��Ϊ���������Ҳ�������
bool gtool::WriteLogString(const I32 nFileIndex, CFmtIOString* pData, const time_t tCurData)
{
	bool bRelease = false;
	auto bSuc = LogMgr2.AddLogTask(nFileIndex, pData, tCurData, bRelease);
	if (bRelease)
	{
		SAFE_RELEASE_NL(pData);
	}
	//CHECKF_NL(IsLogModuleFniDone()==false);
	return bSuc;
}

// bool gtool::WriteFmtLog(const char *szLogFullName, const char *szFmtSys, const char *fmt, ...)
// {
// 	va_list args;
// 	CFmtIOString strFmt;
// 	gtool::InsertCurrentTime(strFmt);
// 	strFmt << szFmtSys;
// 	strFmt << fmt;
// 	strFmt << "\r\n";
// 
// 	//�����ת��Ϊ�ַ���
// 	va_start(args, fmt);
// 	
// 	//��ʽռλ������
// 	ptrdiff_t nCountFmt=std::count(strFmt.GetStrObj().begin(), strFmt.GetStrObj().end(), '%');
// 
// 	//����������軺��ռ�	
// 	int nMemneeded = MAX_INTEGER_STR_WIDTH * nCountFmt+ strFmt.GetLength()+1;
// 	gtool::CSmartPtrC<char> pBuffer = new char[nMemneeded];
// 	CHECKF_NL(pBuffer);
// 
// 	vsprintf((char *)pBuffer, strFmt.c_str(), args);
// 	va_end(args);
// 	pBuffer[nMemneeded-1] = 0;
// 
// 
// 	const char *pszStr = pBuffer;
// 	return LogMgr2.AddLogTask(pszStr, szLogFullName);
// }
