#ifndef LOG_FILE_CONTAINER_H_
#define LOG_FILE_CONTAINER_H_
#pragma once

NAMESPACE_BEGIN(gtool)

//�������� �̰߳�ȫ
class CLogFileContainer
{
	NO_COPY_NO_MOVE(CLogFileContainer);
	HEAP_CREATE_NEED_CLEANS_PUBLIC(CLogFileContainer);

public:
	bool Init();	//ֻ���ڵ��߳�����ɣ����߳�Init���������
	bool Fini() throw ();
	bool IsInit();

	//��ȡ�����ַ�������
public:
	//�����fmt�Ƕ��������Ķ���
	const gtool::CFmtIOString& GetLogFileStingObj(const std::string& strFileName);

private:
	typedef CLitPtrMap<CFmtIOString, std::string, DEL_WAY_DELETE> FileContainerMap;		//�������ĸ�ʽ�����ַ���
	bool m_bInit = false;
	
	CSmartPtr<CDataLocker> m_pShareLocker;
	CSmartPtr<FileContainerMap> m_pFileContainerMap;
	CFmtIOString m_ErrIOString="";
};

NAMESPACE_END(gtool)

//��Ҫ��Fini֮ǰʹ�ã�����crash
typedef gtool::CSingleton<gtool::CLogFileContainer> LogFileContainerInstance;
#define LogFileContainerObj (*LogFileContainerInstance::GetInstance())

#endif

