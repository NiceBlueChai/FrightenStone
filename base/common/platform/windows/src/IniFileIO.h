#ifndef INI_FILE_IO_H_		//IniFileIO
#define INI_FILE_IO_H_

#pragma once



class  CIniFileIO
{
	NO_COPY_NO_MOVE(CIniFileIO);
public:
	CIniFileIO();
	virtual ~CIniFileIO() {}

	//�ļ�·��
public:
	//����·��(����·���������ļ���)
	bool SetFilePath(const char * szFilePath);
	//��ȡ·��
	const char *GetFilePath();
	
	//�ļ�IO
public:
	//��ȡ�ַ���
	const char * ReadString(const char * pszItem, const char * pszSubItem, const char * pszDefault, char *& pszString, const U16& u16MaxCount);
	//��ȡ����ֵ
	U32 ReadInt(const char *pszItem, const char *pszSubItem, const I32 &nDefault);
	//д���ַ���
	bool WriteString(const char *lpAppName, const char *lpKeyName, const char *lpString, const char *lpFileName);

protected:
	char  m_strFilePath[INI_FILE_NAME_LEN];
};
#endif