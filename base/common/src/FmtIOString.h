#ifndef CFMT_IO_STRING_HPP_
#define CFMT_IO_STRING_HPP_

#pragma once
//��÷���cpp�б���������

// #include <BaseCode/InlineFun.hpp>
// #include <BaseCode/SmartPtrC.hpp>
// #include <boost/format.hpp>
//#include <BaseCode/MyDataType.h>
//#include <BaseCode/myclassmacro.h>
//#include <boost/format.hpp>
//#include <BaseCode/MyBaseMacro.h>

//#pragma MEM_ALIGN_BEGIN(1)

NAMESPACE_BEGIN(gtool)

class CFmtIOString
{
		NO_COPY_NEED_RVALUE(CFmtIOString);

public:
	CFmtIOString();
	CFmtIOString(const char * str);
	CFmtIOString(char * str);
	virtual ~CFmtIOString();

public:
//	CFmtIOString &operator = (const std::string &strVal) = delete;
	CFmtIOString &operator = (const char * strVal);
	CFmtIOString &operator = (char * strVal);
	CFmtIOString &operator << (const gtool::CFmtIOString &str);			//�ַ������
	CFmtIOString &operator << (CFmtIOString &&str);			//�ַ������
	CFmtIOString &operator << (const bool& bVal);					//����ֵ���
	CFmtIOString &operator << (const char& cVal);					//�ַ����
	CFmtIOString &operator << (const unsigned char& hhuVal);		//�ַ����
	CFmtIOString &operator << (const short& hdVal);				//���������
	CFmtIOString &operator << (const unsigned short& huVal);		//���������
	CFmtIOString &operator << (const int& dVal);					//�������
	CFmtIOString &operator << (const unsigned int& uVal);			//�������
	CFmtIOString &operator << (const long& ldVal);				//���������
	CFmtIOString &operator << (const unsigned long& luVal);		//���������
	CFmtIOString &operator << (const std::thread::id& idVal);		//�߳�id 

#if (defined(_WIN32)|defined(_WIN64))
	CFmtIOString &operator << (const __int64& I64dVal);			//64λ���
	CFmtIOString &operator << (const unsigned __int64& I64uVal);	//64λ���
#else
	CFmtIOString &operator << (const long long& lldVal);			//64λ���
	CFmtIOString &operator << (const unsigned long long& lluVal);	//64λ���
#endif
	CFmtIOString &operator << (const float& fVal);	//������
	CFmtIOString &operator << (const double& lfVal);	//˫����
	CFmtIOString &operator << (const char * sVal);	//�ַ���
	CFmtIOString &operator << (const void * vpVal);	//��ӡ��ַ
	CFmtIOString &operator << (const void *const& vpVal);	//��ӡ��ַ
	operator CFmtIOString && ();
	operator bool();

	//����
public:
	const char *c_str() const;
	auto& GetBuffer() { return m_szBuffer; }
	U64 GetLength();
	void Clear();
	static CFmtIOString* CreateNew(){ return new CFmtIOString(); }
	int Release() { Clear(); delete this; return 0; } 
	std::string& GetStrObj() { return m_strBuffer; }
	void Swap(gtool::CFmtIOString&& rRightVal);
	void UpdateLastModifyTimestamp(const time_t tLast = 0);
	time_t GetLastModifyTimestamp() { return m_tLastModify; }

	//fmt����
public:
	//�󶨵Ĳ������ɳ����ṩ�Ŀɱ�������� ��ͨ�汾
	template<typename... Args>
	const char * FormatLowEx(const char *szFmt, const Args&... rest)
	{
		//m_strFmt.clear_binds();
		//m_strFmt.parse(szFmt);
		boost::format fmtObj(szFmt);
		const auto nBindArgs = fmtObj.remaining_args();
		const auto CurArgs = sizeof...(rest);
		const auto nFmtLen = strlen(szFmt);
		if (nBindArgs > CurArgs) return NULL;

		CFmtIOString rActualMore;

		if (nBindArgs <= 0)
		{
			if (szFmt&&nFmtLen > 0)
			{
				rActualMore << (char *)szFmt;
			}
		}

		Append(fmtObj, rActualMore, rest...);

		if (nBindArgs > 0)
		{
			m_strBuffer += fmtObj.str().c_str();
		}
		
		if (CurArgs > nBindArgs || nBindArgs<=0)
		{
			m_strBuffer += rActualMore.c_str();
		}
		
		return m_strBuffer.c_str();
	}

	//�󶨵Ĳ������ɳ����ṩ�Ŀɱ�������� �����ܰ汾
	template<typename... Args>
	const char * FormatBoostEx(boost::format& fmtObj, const Args&... rest)
	{
		fmtObj.clear_binds();
		const auto nBindArgs = fmtObj.remaining_args();
		const auto CurArgs = sizeof...(rest);
		if (nBindArgs <= 0 || nBindArgs > CurArgs) return NULL;

		CFmtIOString rActualMore;

		Append(fmtObj, rActualMore, rest...);

		m_strBuffer += fmtObj.str().c_str();

		if (CurArgs > nBindArgs)
		{
			m_strBuffer += rActualMore.c_str();
		}

		return m_strBuffer.c_str();
	}

	//�󶨵Ĳ������ɳ����ṩ�Ŀɱ�������� �����ܰ汾
	template<typename... Args>
	const char * FormatPro(std::string& strFmt, const Args&... rest)
	{
		//����
		//const auto CurArgs = sizeof...(rest);
		CFmtIOString rActualMore;
		U64 u64FirstIndex = 0, u64NextIndex = 0;

		ASSERT(com_algorithm::GetFirstValidFmtString(strFmt, u64FirstIndex, m_strBuffer));
		com_algorithm::NextFmtString(strFmt, u64FirstIndex, u64NextIndex);

		if (u64FirstIndex!=std::string::npos)
		{			
			//����
			AppendPro(strFmt, u64FirstIndex, u64NextIndex, rActualMore,rest...);
			m_strBuffer += rActualMore.c_str();
		}
		else
		{
			//���ý��������ַ�
			NoFmtInput(rest...);
		}

		return m_strBuffer.c_str();
	}
	//c���Է��ɱ���� �����������Ҫȷ�� �����������ʽ���Ʒ�����һ��
//	const char *FormatExHighCStyle(const char *szFmt, ...);

	//�޸�ʽ���Ʒ�
	template< typename... Args>
	const char * NoFmtInput(const Args&... rest)
	{
		AppendNoFmt(rest...);
		return m_strBuffer.c_str();
	}

	//�ڲ��ӿ�
protected:
	template<typename T>
	void Append(boost::format&FmtObj, CFmtIOString&rOs, const T &tVal)	//��������ģ����ֹ�ݹ�
	{
		if (FmtObj.remaining_args() > 0) {
			FmtObj%tVal;
		}
		else {
			rOs << tVal;
		}
	}

	template<typename T, typename... Args>
	void Append(boost::format&FmtObj, CFmtIOString&rOs, const T&tVal, const Args&... rest)	//Args&...�� ������Args������չ���õ�������������rest
	{
		auto nRemain = FmtObj.remaining_args();
		if (nRemain > 0) {
			FmtObj%tVal;
		}
		else {
			rOs << tVal;
		}

		Append(FmtObj, rOs, rest...);	//��rest����������������չ
	}

	//u64StartFmtIndex ��ҪΪ��ǰ��ʽ���Ʒ�λ�ã�std::string::nposΪ��ʽ���Ʒ�������
	template<typename T>
	void AppendPro(std::string& strFmtLeft, U64& u64StartFmtIndex, U64& u64NextFmtIndex, CFmtIOString&rOs, const T &tVal)	//��������ģ����ֹ�ݹ�
	{
		DoAnalysisFmt(strFmtLeft, u64StartFmtIndex, u64NextFmtIndex, rOs, tVal);
		ASSERT(u64StartFmtIndex == std::string::npos);
	}

	template<typename T, typename... Args>
	void AppendPro(std::string& strFmtLeft, U64& u64StartFmtIndex, U64& u64NextFmtIndex, CFmtIOString&rOs, const T&tVal, const Args&... rest)	//Args&...�� ������Args������չ���õ�������������rest
	{
		DoAnalysisFmt(strFmtLeft, u64StartFmtIndex, u64NextFmtIndex, rOs, tVal);
		AppendPro(strFmtLeft, u64StartFmtIndex, u64NextFmtIndex, rOs, rest...);	//��rest����������������չ
	}

	template<typename T>
	void DoAnalysisFmt(std::string& strFmtLeft, U64& u64StartFmtIndex, U64& u64NextFmtIndex, CFmtIOString&rOs, const T&tVal)
	{
		if (u64StartFmtIndex != std::string::npos)
		{
			//���и�ʽ���Ʒ�
			auto u64StartFmtIndexTmp = u64StartFmtIndex;
			u64StartFmtIndex = u64NextFmtIndex;	//��һ������λ��

			std::string strFmtObj;
			if (u64NextFmtIndex != std::string::npos)
			{
				strFmtObj = strFmtLeft.substr(u64StartFmtIndexTmp, u64NextFmtIndex - u64StartFmtIndexTmp).c_str();
			}
			else
			{
				strFmtObj = strFmtLeft.substr(u64StartFmtIndexTmp).c_str();
			}

			//����
			{
				auto nFmtPartLen = strFmtObj.length();

				//buffer����
				U64 u64BufferSize = nFmtPartLen + 1;
				//std::cout << typeid(tVal).name() << std::endl;
				u64BufferSize += GetBufferAddapterSize<T>::GetBufferNeeded(tVal);

				//�ϳ��ַ���
				CSmartPtrC<char, char, DEL_WAY_DELETE_MULTI> pszBuffer = new char[u64BufferSize];
				CHECK_NL(pszBuffer);
				//memset(pszBuffer, 0, u64BufferSize);
				pszBuffer[0] = 0;
				auto nLen=sprintf((char*)pszBuffer, (const char *)strFmtObj.c_str(), tVal);
				auto nIndex = ((nLen < u64BufferSize) ? gtool::Max(nLen, 0) : (u64BufferSize - 1));
				pszBuffer[I32(nIndex)] = 0;
				m_strBuffer += (char *)pszBuffer;
			}

			//ȡ��һ����ʽ�����ַ���Ƭ��
			com_algorithm::NextFmtString(strFmtLeft, u64StartFmtIndex, u64NextFmtIndex);

		}
		else
		{
			//ʣ�����
			rOs << tVal;
			//u64NextFmtIndex = std::string::npos;
		}
	}

	template<typename T>
	void AppendNoFmt(const T &tVal)	//��������ģ����ֹ�ݹ�
	{
		(*this) << tVal;
	}

	template<typename T, typename... Args>
	void AppendNoFmt(const T&tVal, const Args&... rest)	//Args&...�� ������Args������չ���õ�������������rest
	{
		(*this) << tVal;
		AppendNoFmt(rest...);	//��rest����������������չ
	}

	//���Ͷ���
public:

#if(defined(_WIN32)|defined(_WIN64))
	static const char * endl;
#else
	static const char * endl;
#endif
	
// 	class IOEndl
// 	{
// 		NO_COPY_NEED_RVALUE(IOEndl);
// 	public:
// 		IOEndl() {}
// 		virtual ~IOEndl() {}
// 		operator const char*()
// 		{
// #if(defined(_WIN32)|defined(_WIN64))
// 			return "\r\n";
// #else
// 			return "\n";
// #endif
// 		}
// 	};
	
protected:
	volatile time_t		m_tLastModify = 0;		//����޸��ַ���ʱ���
	std::string m_strBuffer;		//�̲߳���ȫ	��Ҫ�ⲿ����
//	boost::format m_strFmt;
	BUFFER32	m_szBuffer = { 0 };	//ͬһ�����̲߳���ȫ	��Ҫ�ⲿ����
};

NAMESPACE_END(gtool)

//#pragma MEM_ALIGN_END(1)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





#endif
