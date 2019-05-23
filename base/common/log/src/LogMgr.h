#ifndef LOG_MGR_H_
#define LOG_MGR_H_
#pragma once
// #include <BaseCode/MyBaseMacro.h>
// #include <BaseCode/LitPtrMap.hpp>
// #include <BaseCode/SmartPtr.hpp>
// #include <BaseCode/TPtrList.hpp>

//#include <BaseCode/WinLocker.h>

class CFile2;
class CThreadPool;
class CLogFile;
//class CDataLocker;


typedef gtool::CLitPtrMap<CLogFile, I32> FilePtrMap;		//key:fileindex value: FilePtr
typedef gtool::TPtrList<gtool::CFmtIOString, DEL_WAY_RELEASE> LogDataList;
typedef gtool::CLitPtrMap<LogDataList, I32> LogDataListMap;	//key:fileindex value:LogDataList


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
	bool IsLock();
	bool TimeWait(MYULONG ulMilliseconds=TIMER_FLUSH_DIFF_DEF);
	void YieldSchedule();
	bool AddLogTask(const I32 nFileIndex, gtool::CFmtIOString* pData,const time_t tCurData, bool & bNeedDataRelease);
	bool IsInit();
	void FinishFlushTask() { m_bFlushStarted = false; }
	void StartFlushTask() { m_bFlushStarted = true; }

public:
	LogDataListMap * GetLogDataListMap();
	CLogFile* QueryFilePtr(const I32 nFileIndex);
	bool IsFinishLog() const;
	int GetFlushTimerDiff() { return m_nTimeDiff; }
	bool IsFiniDone()const;

	//api�ӿ�
public:
	//����ˢ��ʱ�䣨�����ʱ�䣩 �̰߳�ȫ
	void SetTimerFlush(int nTimeDiff);		

	//�����(�̳߳�����) �̰߳�ȫ
	void FlushLog();	

	//��־�ļ���ʼ�� �̰߳�ȫ ��ʼ��ʱ����ã�������������ʹ��
	bool AddLog(const I32 nFileIndex, const char *szLogPath, const char *szFileName);										

private:
	gtool::CSmartPtr<FilePtrMap>			m_pFilePtrMap;				//�ļ������ļ�ָ��map
	gtool::CSmartPtr<LogDataListMap>		m_pLogDataListMap;
	gtool::CSmartPtr<CDataLocker>			m_pDataSafeGuard;				//���ݱ���
	gtool::CSmartPtr<CThreadPool>			m_pThreadPool;					//�̳߳�	
	boost::atomic_bool						m_bInit{ false };
	boost::atomic<I32>						m_nTimeDiff{ TIMER_FLUSH_DIFF_DEF };				//��λs <=0��ʾ�ֶ�����̣�>0��ʾ��ʱ����� Ĭ��1s��һ�δ���
	boost::atomic_bool						m_bFiniLog{ false };
	static boost::atomic_bool				m_bFiniDone ;
	boost::atomic_bool						m_bFlushStarted{ false };
};

typedef gtool::CSingleton<CLogMgr> LogMgrInstance;

extern CLogMgr * g_pLogMgr;
#define LogMgr2 (ASSERT(g_pLogMgr), *g_pLogMgr)


#endif
