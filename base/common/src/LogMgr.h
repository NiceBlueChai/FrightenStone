#ifndef LOG_MGR_H_
#define LOG_MGR_H_
#pragma once

#include <BaseCode/MyBaseMacro.h>
#include <BaseCode/LitPtrMap.hpp>
#include <BaseCode/SmartPtr.hpp>
#include <BaseCode/TPtrList.hpp>
#include <BaseCode/FmtIOString.h>
#include <boost/timer.hpp>

class CFile2;
class CThreadPool;
class CDataLocker;

struct LogDataInfo
{
	std::string strFullLogName;			//�ļ���
	gtool::CFmtIOString strData;		//����
};

typedef gtool::CLitPtrMap<CDataLocker, std::string> FileNameKeyLockerMap;	//key:filename value: locker
typedef gtool::CLitPtrMap<CFile2, std::string> StrKeyFilePtrValLitMap;		//key:filename value: FilePtr
typedef gtool::TPtrList<LogDataInfo, DEL_WAY_DELETE> StringList;

#define TIMER_FLUSH_DIFF_DEF		1			//Ĭ��ÿ1s�����һ��

//ʹ��һ���̵߳��첽��ʽ
//һ���ļ�����Ӧһ����
//д�ļ����ļ�����ʹ��map�ȹ�����Դ����һ����
//LogMgr��Ҫȫ��Ψһ��ʹ�õ��� ���߳���־

class CLogMgr
{
	NO_COPY(CLogMgr);
public:
	CLogMgr();
	virtual ~CLogMgr();

	bool Init();
	bool Fini();
	bool LockRes();
	bool UnLockRes();
	void YieldSchedule();
	bool AddTask(gtool::CFmtIOString& strData, const char *& strFileName);
	bool IsInit();

public:
	StrKeyFilePtrValLitMap* GetFilePtrMap();
	FileNameKeyLockerMap* GetFileLockerMap();
	StringList *GetLogDataList();

	//api�ӿ�
public:
	void SetTimerFlush(int nTimeDiff);																//����ˢ��ʱ�䣨�����ʱ�䣩
	void FlushLog();																				//�����(�̳߳�����)
	bool AddLog(const char *szLogPath, const char *szFileName, FILE* fp);										//��־�ļ���ʼ��

	template<typename... Aargs> 	//��װ���ַ����������̳߳���д�ļ�������������̣�
	bool WriteLog(bool bNoFmt, const char *szLogFullName, const char *fmt, const Aargs&... rest)
	{
		gtool::CFmtIOString strData;
		if (bNoFmt)
		{
			strData.MulArgInput(rest);
		}
		else
		{
			strData.FormatEx(fmt, rest);
		}
		CHECKF_NL(AddTask(strData, szLogFullName));

		return true;
	}

private:
	gtool::CSmartPtr<FileNameKeyLockerMap>	m_pFileNameKeyLockerMap;			//�ļ�����Ӧ����map
	gtool::CSmartPtr<StrKeyFilePtrValLitMap> m_pFilePtrValStrKeyMap;		//�ļ������ļ�ָ��map
	gtool::CSmartPtr<CDataLocker>			m_pDataSafeGuard;				//���ݱ���
	gtool::CSmartPtr<CThreadPool>			m_pThreadPool;					//�̳߳�	
	gtool::CSmartPtr<StringList>			m_pStringList;					//��־����
	volatile bool							m_bInit = false;
	volatile int							m_nTimeDiff = TIMER_FLUSH_DIFF_DEF;				//��λs <=0��ʾ�ֶ�����̣�>0��ʾ��ʱ����� Ĭ��1s��һ�δ���
	boost::timer							m_Timer;
};

typedef gtool::CSingleton<CLogMgr> LogMgrInstance;
#define LogMgr2 (*LogMgrInstance::GetInstance())


#endif
