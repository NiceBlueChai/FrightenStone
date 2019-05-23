#ifndef FMT_ANALYSIS_H_
#define FMT_ANALYSIS_H_
#pragma once

class CDataLocker;

NAMESPACE_BEGIN(gtool)

//�������� �̰߳�ȫ
class CFmtAnalysis
{
	NO_COPY_NO_MOVE(CFmtAnalysis);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CFmtAnalysis);

public:
	bool Init();	//ֻ���ڵ��߳�����ɣ����߳�Init���������
	bool Fini() throw ();
	bool IsInit();

	//��ȡ�����ַ�������
public:
	//�����fmt�Ƕ��������Ķ���
	bool GetFmtConstObj(std::string& strFmt, boost::format *& pOutObj);

private:
	typedef CLitPtrMap<boost::format, std::string, DEL_WAY_DELETE> FmtStringAnalysisMap;		//�������ĸ�ʽ�����ַ���
	bool m_bInit = false;
	
	CSmartPtr<CDataLocker> m_pShareLocker;
	CSmartPtr<FmtStringAnalysisMap> m_pAnalysisMap;
	const boost::format m_ErrFmt;
};

NAMESPACE_END(gtool)

//��Ҫ��Fini֮ǰʹ�ã�����crash
typedef gtool::CSingleton<gtool::CFmtAnalysis> FmtAnalysisInstance;
#define FmtAnalysisObj (*FmtAnalysisInstance::GetInstance())

#endif

