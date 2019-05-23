#ifndef LOGGER_H_
#define LOGGER_H_

/*	CLogger ��־��													*/
/*	Author:shy														*/	
/*	date:201709														*/
/*	doc:															*/
/*		_out_os log����ĵط���Ĭ����std::cout,����ֵʱ�����ȫ�ֱ���*/
//__func__ ��ǰ�ĺ�����
//__VA_ARGS__,�ɱ�κ�
//#define log(...) CLogger::log(__DATE__,' ',__TIME__ ':', __FILE__,' ',__LINE__,' ',__func__,"():",__VA_ARGS__) __TIME__û������ȥ __FUNCTION__
//#define Fyclog(filename, ...) gtool:: CLogger::log( filename, __FILE__,__LINE__,__FUNCSIG__,#__VA_ARGS__, __VA_ARGS__)	//��ǰ��ӡ�ĺ�����ĺ�����__func__
//#define log(...) CLogger::log(__FILE__,' ',__LINE__,' ',__VA_ARGS__)  

//��ǰ��ӡ�ĺ�����ĺ�����__func__
//�˴����������жϲ����ĸ����������һ�������logHelper(ostream& os, const T& t1)
//�������������logHelper(ostream& os, const T& t1, const Tn&... args)��������ݹ���ã�ֱ��ֻ��һ����������
//logHelper(ostream& os, const T& t1)��Ϊ���ڳ���
//���ʹ����C++11�Ŀɱ��������ģ������ԣ�˵���ˣ����ǵݹ����

#pragma once

//#include <string>
//#include <ios>
//#include <fstream>
//#include <iostream>
//#include <time.h>
#include "BaseCode/WinLocker.h"
#include "BaseCode/FmtIOString.h"
#include <BaseCode/FileDirSys.h>
#include <string>
#include <map>

#include <BaseCode/File2.h>
#include <BaseCode/LitPtrMap.hpp>
#include <BaseCode/SmartPtr.hpp>
#include <BaseCode/Singleton.hpp>

//static FILE *g_fp = NULL;
//��
#if !(defined(linux)|defined(__CYGWIN__))
extern CDataLocker  g_SuperLogLocker;
extern CDataLocker  g_SimpleLogLocker;
extern CDataLocker  g_GenLogLocker;
extern CDataLocker  g_ProgCrashLogLocker;
extern CDataLocker  g_ProgErrLogLocker;
#endif

typedef gtool::CLitPtrMap<CFile2, std::string> StrKeyFilePtrValLitMap;

namespace gtool
{
#if !(defined(linux)|defined(__CYGWIN__))

	class CSafeLogMap
	{
		NO_COPY(CSafeLogMap);
	public:
		CSafeLogMap();
		virtual ~CSafeLogMap();

	public:
		static CSafeLogMap * CreateNew() { return new CSafeLogMap(); }
		int Release() { delete this; return 0; }

		inline bool Lock()
		{
			return m_SafeLocker.Lock()== RES_SUCCESS;
		}
		inline bool UnLock()
		{
			return m_SafeLocker.UnLock() == RES_SUCCESS;
		}

		gtool::CSmartPtr<StrKeyFilePtrValLitMap>	m_pLogFileMap;			//key:�ļ����� val���ļ�ָ��
	protected:
		CDataLocker m_SafeLocker;
	
	};


	class  CLogger
	{
		NO_COPY_NEED_RVALUE(CLogger);
	private:
		CLogger();
		~CLogger();
	public:
		//log interface
		template<typename... Aargs>
		static bool logFmt(std::string filename, const char *szFmtLog, const char *szFmt, const Aargs&... args);

		template<typename... Aargs>
		static bool logNoFmt(std::string filename, const Aargs&... args);

		static void makeLoggerOn();
		static void makeLoggerOff();
		static const bool loggerState();
		static bool FlushLog(const char *szLogFile);
		static bool CloseLog(const char *szLogFile);

//internal function
	public:

#if  defined(_WIN32) || defined(_WIN64)
		//windowsƽ̨���뼶
		static void InsertCurrentTime(CFmtIOString& fos);								//ʱ���
		static void FileInsertTime(std::string &strFileName);						//����־�������
#else
		static void FileInsertTime(string &strFileName);						//����־�������
		//ͨ�ð��뼶
		static void InsertCurrentTime(CFmtIOString& fos);							//ʱ���
#endif

		//data member
	protected:
		
		static volatile bool _logger_is_on;		//log����		
	};

	//��־����
	typedef gtool::CSingleton<CSafeLogMap> SingleSafeLogFileMapInstance;
	#define SafeLogFileMap2  (*SingleSafeLogFileMapInstance::GetInstance())
}

	/////////////////////////////////////////////////////[ ʵ �� ]////////////////////////////////////////////////////////

	template<typename... Aargs>
	bool gtool::CLogger::logFmt(std::string filename, const char *szFmtLog, const char *szFmt,const Aargs&... args)
	{
		auto pMap = &(SafeLogFileMap2);
		if (!pMap)
		{
			ASSERT(pMap);
			return false;
		}

		pMap->Lock();
		auto bOpen = _logger_is_on;
		pMap->UnLock();
		if (bOpen)
		{
			if (!szFmtLog)
			{
				ASSERT(szFmtLog);
				return false;
			}
			pMap->Lock();
			if (!pMap->m_pLogFileMap)
			{
				ASSERT(pMap->m_pLogFileMap);
				pMap->m_pLogFileMap = StrKeyFilePtrValLitMap::CreateNew();
			}
			pMap->UnLock();

			if (filename.length() <= 0)
			{
				ASSERT(!"filename.length()<=0");				
				return false;
			}
			CFmtIOString rFmtBuffer;

			FileInsertTime(filename);
			InsertCurrentTime(rFmtBuffer);

			std::string strFmt = szFmtLog;
			strFmt += szFmt;

			char *szBufferFmt = (char *)rFmtBuffer.FormatEx(strFmt.c_str(), args...);
			if (!szBufferFmt||strlen(szBufferFmt) <= 0)
			{
				ASSERT(!"�ַ�����ʽ������");
				return false;
			}

			rFmtBuffer << CFmtIOString::IOEndl();
			
			//���ļ�
			FILE *fpLog = NULL;
			pMap->Lock();
			auto IterMap= pMap->m_pLogFileMap->find(filename);
			if (IterMap == pMap->m_pLogFileMap->end())
			{
				gtool::CSmartPtr<CFile2> fpLog2 = CFile2::CreateNew();
				if (!fpLog2)
				{
					ASSERT(fpLog2);
					pMap->UnLock();
					return false;
				}

				fpLog2->m_fptr = CFileDirSys::OpenFile(filename, !CFileDirSys::IsFileExist(filename), "ab+");
				if (!fpLog2->m_fptr)
				{
					ASSERT(fpLog2->m_fptr);
					pMap->UnLock();
					return false;
				}

				CFile2 *pFile2 = NULL;
				pMap->m_pLogFileMap->insert(std::make_pair(filename, pFile2=fpLog2.pop()));
				if (pMap->m_pLogFileMap->find(filename) == pMap->m_pLogFileMap->end())
				{
					ASSERT(!"map �����ļ�ָ��ʧ��");
					pMap->UnLock();
					return false;
				}

				if (pFile2)
				{
					fpLog = pFile2->m_fptr;
				}
			}
			else
			{
				auto pFile2=IterMap->second;
				if (pFile2)
				{
					fpLog = pFile2->m_fptr;
				}
			}
			if (!fpLog)
			{
				filename += (char *)"��־�ļ���ʧ��";
				ASSERT(!filename.c_str());
				pMap->UnLock();
				return false;
			}

			ASSERT( CFileDirSys::WriteFile(*fpLog, rFmtBuffer.c_str(), long(rFmtBuffer.GetLength()))>0 );

			pMap->UnLock();
			
		}
		
		return true;
	}

	template<typename... Aargs>
	bool gtool::CLogger::logNoFmt(std::string filename,  const Aargs&... args)
	{
		auto pMap = &(SafeLogFileMap2);
		if (!pMap)
		{
			ASSERT(pMap);
			return false;
		}

		pMap->Lock();
		auto bOpen = _logger_is_on;
		pMap->UnLock();
		if (bOpen)
		{
			if (filename.length() <= 0)
			{
				ASSERT(!"filename.length()<=0");
					return false;
			}

			pMap->Lock();
			if (!pMap->m_pLogFileMap)
			{
				ASSERT(pMap->m_pLogFileMap);
				pMap->m_pLogFileMap = StrKeyFilePtrValLitMap::CreateNew();
			}
			pMap->UnLock();

			CFmtIOString rFmtBuffer;

			FileInsertTime(filename);
			InsertCurrentTime(rFmtBuffer);

			char *szBufferFmt = (char *)rFmtBuffer.MulArgInput(args...);
			if (!szBufferFmt||strlen(szBufferFmt) <= 0)
			{
				ASSERT(!"�ַ����������");
				return false;
			}

			rFmtBuffer << CFmtIOString::IOEndl();

			//���ļ�
			FILE *fpLog = NULL;
			pMap->Lock();
			auto IterMap = pMap->m_pLogFileMap->find(filename);
			if (IterMap == pMap->m_pLogFileMap->end())
			{
				gtool::CSmartPtr<CFile2> fpLog2 = CFile2::CreateNew();
				if (!fpLog2)
				{
					ASSERT(fpLog2);
					pMap->UnLock();
					return false;
				}

				fpLog2->m_fptr = CFileDirSys::OpenFile(filename, !CFileDirSys::IsFileExist(filename), "ab+");
				if (!fpLog2->m_fptr)
				{
					ASSERT(fpLog2->m_fptr);
					pMap->UnLock();
					return false;
				}

				CFile2* pFile2 = NULL;
				pMap->m_pLogFileMap->insert(std::make_pair(filename, pFile2=fpLog2.pop()));
				if (pMap->m_pLogFileMap->find(filename) == pMap->m_pLogFileMap->end())
				{
					ASSERT(!"map �����ļ�ָ��ʧ��");
					pMap->UnLock();
					return false;
				}

				if (pFile2)
				{
					fpLog = pFile2->m_fptr;
				}
		
			}
			else
			{
				auto pFile2 = IterMap->second;
				if (pFile2)
				{
					fpLog = pFile2->m_fptr;
				}
			}

			if (!fpLog)
			{
				filename += (char *)"��־�ļ���ʧ��";
				ASSERT(!filename.c_str());
				pMap->UnLock();
				return false;
			}

			ASSERT(CFileDirSys::WriteFile(*fpLog, rFmtBuffer.c_str(), long(rFmtBuffer.GetLength())) <= 0);
			pMap->UnLock();
		}
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

#if !(defined(linux)|defined(__CYGWIN__))
	/************************************************* [ ��׼�� ] *************************************************/
	#undef SUPERLOG
	#define SUPERLOG(filename, szFmt, ...)  (g_SuperLogLocker.Lock(), gtool:: CLogger::logFmt( filename,"%s %d %s ", szFmt, __FILE__, __LINE__, __FUNCSIG__, ##__VA_ARGS__), g_SuperLogLocker.UnLock())		

	#undef SIMPLELOG
	#define SIMPLELOG(filename, szFmt, ...)	(g_SimpleLogLocker.Lock(), gtool::CLogger::logFmt(filename,"", szFmt, ##__VA_ARGS__),g_SimpleLogLocker.UnLock())

	#undef GENLOG
	#define GENLOG(filename,szFmt, ...)		(g_GenLogLocker.Lock(),gtool::CLogger::logFmt(filename,"%s ", szFmt, __FUNCSIG__, ##__VA_ARGS__),g_GenLogLocker.UnLock())

#undef GENLOGNOFMT
#define GENLOGNOFMT(filename,...)		(g_GenLogLocker.Lock(),gtool::CLogger::logNoFmt(filename, __FUNCSIG__, ##__VA_ARGS__),g_GenLogLocker.UnLock())

	/************************************************* [ E N D ] *************************************************/

	//��չ��
	#undef GENLOG2
	#define GENLOG2(FunNameView,filename,szFmt, ...)		(gtool::CLogger::logFmt(filename,"%s%s", szFmt, ((FunNameView)?(__FUNCTION__):" "),((FunNameView)?("()"):" "), ##__VA_ARGS__))

	//����������
	#undef GENLOG4
	#define GENLOG4(filename,szFmt, ...)		(GENLOG2(0,filename,szFmt, ##__VA_ARGS__))

	//��������
	#undef  GENLOG5
	#define GENLOG5(szFmt, ...)				(GENLOG2(1,".\\TESTLOG\\CodeTest.log",szFmt, ##__VA_ARGS__))

	//crash
	#undef PROG_CRASH_LOG
#define PROG_CRASH_LOG(szFmt, ...)			(gtool::CLogger::logFmt(".\\prg-crash.log","%s %d %s ", szFmt, __FILE__, __LINE__, __FUNCSIG__,  ##__VA_ARGS__ ))

	//����
	#undef PROG_ERR_LOG
	#define PROG_ERR_LOG(szFmt, ...)			(gtool::CLogger::logFmt(".\\prog-err.log", "%s %d %s ", szFmt, __FILE__, __LINE__, __FUNCSIG__, ##__VA_ARGS__ ))

	//����
	#undef LOGTESTTITLE_FUN
	#define LOGTESTTITLE_FUN(...)		(gtool::CLogger::logNoFmt(".\\TESTLOG\\CodeTest.log", "\n[--------------------------------------------------" ,__FUNCSIG__,##__VA_ARGS__, "--------------------------------------------------]"))

	//��������������
	#undef LOGTESTTITLE
	#define LOGTESTTITLE(...)		(gtool::CLogger::logNoFmt(".\\TESTLOG\\CodeTest.log","\n[--------------------------------------------------" , ##__VA_ARGS__, "--------------------------------------------------]"))

	//����
	#undef LOGCODETEST
	#define LOGCODETEST(szFmt, ...)	(GENLOG4(".\\TESTLOG\\CodeTest.log",szFmt, ##__VA_ARGS__))
#else

	/************************************************* [ ��׼�� ] *************************************************/
#undef SUPERLOG
#define SUPERLOG(filename,szFmt, ...)  ()		

#undef SIMPLELOG
#define SIMPLELOG(filename, szFmt, ...)	()

#undef GENLOG
#define GENLOG(filename,szFmt, ...)		()

	/************************************************* [ E N D ] *************************************************/

	//��չ��
#undef GENLOG2
#define GENLOG2(FunNameView,filename,szFmt, ...)		("")

	//����������
#undef GENLOG4
#define GENLOG4(filename,szFmt, ...)		(GENLOG2(0,filename,szFmt, ##__VA_ARGS__))

	//��������
#undef  GENLOG5
#define GENLOG5(szFmt, ...)				(GENLOG2(1,".\\TESTLOG\\CodeTest.log", szFmt, ##__VA_ARGS__))

	//crash
#undef PROG_CRASH_LOG
#define PROG_CRASH_LOG(szFmt, ...)			("")

	//����
#undef PROG_ERR_LOG
#define PROG_ERR_LOG(szFmt, ...)			("")

	//����
#undef LOGTESTTITLE_FUN
#define LOGTESTTITLE_FUN( ...)		("")

	//��������������
#undef LOGTESTTITLE
#define LOGTESTTITLE( ...)		("")

	//����
#undef LOGCODETEST
#define LOGCODETEST(szFmt,...)	(GENLOG4(".\\TESTLOG\\CodeTest.log",szFmt, ##__VA_ARGS__))

#endif




#endif





