#ifndef LOG_EX_API_H_
#define LOG_EX_API_H_
#pragma once
#include <BaseCode/MyBaseMacro.h>
//#include<BaseCode/FmtIOString.h>

//��������
#ifndef LOG_EX_API
	#ifdef  LOG_EX_API_DLL
		#define LOG_EX_API _declspec(dllexport)
	#else
		#define LOG_EX_API _declspec(dllimport)
	#endif
#endif

//�����ļ���
#ifndef _DEBUG

	#ifdef _UNICODE
		#define LOG_EX_API_DLL_NAME				TEXT("LogEx.dll")		//�������
	#else
		#define LOG_EX_API_DLL_NAME				"LogEx.dll"				//�������
	#endif
	
#else
	#ifdef _UNICODE
		#define LOG_EX_API_DLL_NAME				TEXT("LogExD.dll")		//�������
	#else
		#define LOG_EX_API_DLL_NAME				"LogExD.dll"			//�������
	#endif
#endif

//////////////////////////////////////////////////////////////////////////////////

//�����ļ�
// #ifndef COMMON_MODUL_DLL
// 
// #endif



//////////////////////////////////////////////////////////////////////////////////////


// struct  LogInitInfo
// {
// 	const char* log_path;
// 	const char* log_name;
// 	LogInitInfo(const char* _log_path, const char* _log_name, const char* _log_title) :log_path(_log_path), log_name(_log_name) {}
// };

// #define DECLEAR_LOGNAME(log_path, log_name)	const int _##log_path##_##log_name =xlog::AddLogFile(#log_path, #log_name);
// #define LOGNAME_DELIMI ;

template <int i>
struct LogInitHelper
{
	static bool InitLog() { return true; }
};

template<int i>
struct ExcludeCommentHelper
{
	enum { realvalue = ExcludeCommentHelper<i - 1>::realvalue };
};

template<>
struct ExcludeCommentHelper<0>
{
	enum { realvalue = 0 };
};


#define DECLEAR_LOG_BEGIN(LOG_BEGIN, line, logindexbase) \
	enum {LOG_BEGIN = logindexbase};\
	template<>\
struct ExcludeCommentHelper<line>\
{\
	enum {realvalue = logindexbase-1};\
};\


#define ADD_LOG(log_path, log_name, base, line) \
	template<>\
struct ExcludeCommentHelper<line>\
{\
	enum {realvalue = ExcludeCommentHelper<line-1>::realvalue + 1};\
};\
	static const int _##log_path##_##log_name##_ = ExcludeCommentHelper<line>::realvalue;\
	template<>\
struct LogInitHelper<_##log_path##_##log_name##_>\
{\
	static bool InitLog() \
	{ \
		if (_##log_path##_##log_name##_ > base)\
			CHECKF_NL(LogInitHelper<_##log_path##_##log_name##_-1>::InitLog()); \
		CHECKF_NL(gtool::AddLog(_##log_path##_##log_name##_,".\\"#log_path"\\", #log_name".log"));\
		return true;\
	} \
};

#define DECLEAR_LOG_END(LOG_NUMBER, line) \
	enum {LOG_NUMBER = ExcludeCommentHelper<line>::realvalue};

#define DECLEAR_GENERAL_LOG_BEGIN() DECLEAR_LOG_BEGIN(LOG_NUM_BEGIN,  __LINE__, 0)
#define GENERAL_ADDLOG(log_path, log_name) ADD_LOG(log_path, log_name, LOG_NUM_BEGIN, __LINE__)
#define DECLEAR_GENERAL_LOG_END() DECLEAR_LOG_END(LOG_NUM_MAX, __LINE__)

// #define DECLEAR_MODULE_LOG_BEGIN() DECLEAR_LOG_BEGIN(MODULE_BEGIN, __LINE__+UTILITY_MAX+1, UTILITY_MAX+1)
// #define MODULE_ADDLOG(log_path, log_name, log_title) ADD_LOG(log_path, log_name, MODULE_BEGIN, __LINE__+UTILITY_MAX+1)
// #define DECLEAR_MODULE_LOG_END() DECLEAR_LOG_END(MODULE_MAX, __LINE__+UTILITY_MAX+1)

//////////////////////////////////////////////////////////////////////////////////////

//#ifndef LOG_EX_API_DLL

// #ifdef __cplusplus	
// extern "C" {	//ʹ��c��������ģʽ
// #endif

NAMESPACE_BEGIN(gtool)

extern LOG_EX_API void SetTimerFlush(int nTimeDiff);			//���ˢ��
extern LOG_EX_API void FlushLog();
//extern LOG_EX_API bool AddLog(const I32 nFileIndex, const char *szLogPath, const char *szFileName);	//������
extern LOG_EX_API bool AddLog(const I32 nFileIndex, const char *szLogPath, const char *szFileName);	//������
//extern LOG_EX_API bool  WriteLog(bool bNoFmt, const char *szLogFullName, const char *szFmtSys, const char *fmt, ...);
extern LOG_EX_API bool WriteLogString(const I32 nFileIndex, CFmtIOString* pData, const time_t tCurData);	// CFmtIOString* pData ��Ϊ���������Ҳ�������

//�޸�ʽ���ַ��� ������
template<typename... Args>
static bool  WriteLog(const I32 nFileIndex, const Args&... rest)
{
	gtool::CSmartPtr<CFmtIOString> pData = CFmtIOString::CreateNew();
	CHECKF_NL(pData);
	auto nCurTime = gtool::InsertCurrentTime(*pData);
	pData->NoFmtInput(rest...);
	*pData << CFmtIOString::endl;
	
	return WriteLogString(nFileIndex, pData.pop(), nCurTime);
}

//�ϵ�������Ҫ�����ַ���
template<typename... Args>
static inline bool  WriteFmtLog(const I32 nFileIndex, const char *szFmtSys, const char *fmt, const Args&... rest)
{
	gtool::CSmartPtr<CFmtIOString> pData = CFmtIOString::CreateNew();
	CHECKF_NL(pData);
	
	std::string strFmt = szFmtSys;
	strFmt += fmt;

	auto nCurTime = gtool::InsertCurrentTime(*pData);
	pData->FormatPro(strFmt, rest...);
	*pData << CFmtIOString::endl;

	return WriteLogString(nFileIndex, pData.pop(), nCurTime);
}

//�ϸ�����
template<BUFFER_LEN_TYPE BUFFER_TYPE>
static inline bool WriteLogCStyle(const I32 nFileIndex, const char *szSysFmt, const char *fmt, ...)
{
	gtool::CSmartPtr<CFmtIOString> pData = CFmtIOString::CreateNew();
	CHECKF_NL(pData);

	//��ʽ���ַ���
	std::string strFmtObj;
	strFmtObj += szSysFmt;
	strFmtObj += fmt;

	//c �������б�
	va_list rVAList = NULL;
	va_start(rVAList, fmt);

	gtool::BufferAdapterCustomLen<BUFFER_TYPE> Buffer;
	auto nByte = vsnprintf(Buffer.Buffer, sizeof(Buffer.Buffer) - 1, strFmtObj.c_str(), rVAList);
	va_end(rVAList);
	CHECKF_NL(nByte >= 0);

	auto nCurTime = gtool::InsertCurrentTime(*pData);
	*pData << Buffer.Buffer << CFmtIOString::endl;

	return WriteLogString(nFileIndex, pData.pop(), nCurTime);
}

//�и�ʽ���ַ����������
//bool LOG_EX_API  WriteFmtLog(const char *szLogFullName, const char *szFmtSys, const char *fmt, ...);

extern LOG_EX_API bool InitLogModule();
extern LOG_EX_API bool FinishLogModule();
extern LOG_EX_API bool IsLogModuleInit();

extern LOG_EX_API bool IsLogModuleFniDone();

// extern void SetTimerFlush(int nTimeDiff);			//���ˢ��
// extern void FlushLog();
// extern bool AddLog(const char *szLogPath, const char *szFileName);
// 
// template<typename... Args>
// extern bool WriteLog(bool bNoFmt, const char *szLogFullName, const char *fmt, const Args&... rest);
// 
// extern bool FinishLogModule();

NAMESPACE_END(gtool)

//#endif




// 
//  #ifdef __cplusplus
//  	}
// #endif


#endif

