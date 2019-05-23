/*********************************
/*	FileName: FileDirSys.h
/*	Auther:	Feng Yanchen
/*	Creation Date: 2016-05-04
/*	Date Updated : 2016-05-04
*********************************/

#ifndef FILE_DIR_SYS_H_
#define FILE_DIR_SYS_H_

#pragma once 
//#include <BaseCode/BaseCodeApi.h>

//#include <BaseCode/myclassmacro.h>


#ifdef _WIN64


#endif

#ifndef FILE_DIR_SYS_H_FILE_CURSOR_POS_TYPE_
#define  FILE_DIR_SYS_H_FILE_CURSOR_POS_TYPE_

enum FILE_CURSOR_POS
{
	FILE_CURSOR_POS_SET=0,
	FILE_CURSOR_POS_CUR=1,
	FILE_CURSOR_POS_END=2,
};

#endif
///////////////////////////////////////////////////////////////////////

//���ݲ���
#define BYTE_UNIT_BITS		0x8											//���ֽ�����λ��
#define GET_WDATA_HIGH_BYTE(x)	((( (x)&0x0FFFF )>>BYTE_UNIT_BITS)&0x0FF)		//ȡWORD���ݵĸ��ֽ�
#define GET_WDATA_LOW_BYTE(x)	(( (x)&0x0FFFF)&0x0FF)						//ȡWORD���ݵĵ��ֽ�

///////////////////////////////////////////

NAMESPACE_BEGIN(gtool)

//�ļ�����
class CFileDirSys
{
	NO_COPY_NEED_RVALUE(CFileDirSys);
	private:
	CFileDirSys(){}
	~CFileDirSys(){}

public:
	static bool SaveDataAsFile(const char *szFileName, const char *szString, const U64 nStrByte);

// #ifdef _WIN64
// 
// 	//�ַ�ת��
// 	static bool UnicodeToAnsi(LPTSTR szUnicodeSrc, char *pAnsiStrOut, int nAnsiStrSize, int *pAnsiOutStrlenByte = NULL);
// 
// 	//�ַ�ת��
// 	static bool AnsiToUnicode(char *pAnsiStrIn, LPTSTR szUnicodeStrOut, int nUnicodeCacheSize, int *nUnicodeStrLenOutCalculateZeroTail = NULL);
// 
// 	//utf8->ansi ��windows api
// 	static char * Utf8Convert2Ansi(const char *pUtf8);	//�����ɶ����������ַ���
// 	//ansi->utf8 ��windows api
// 	static char * AnsiConvert2Utf8(const char *pAnsi);	//�����ɶ����������ַ���
// #endif



	//�ļ�api
public:
	//ɾ���ļ�
	static void DelFile(const char *szfile);

	//ɾ���ļ�
	static bool DelFileCStyle(const char *szfile);

	//������ʱ�ļ� ��w+b��ģʽ�򿪷�����ʱ�ļ�����ָ��
	static FILE *GenTmpFile();

	//��ȡ�����Ψһ���ļ��� ���鳤������Ϊ L_tmpnam 
	static char *GenRandFileName(char szRandName[L_tmpnam]);

	static char *GenRandFileNameNoDir(char szRandName[L_tmpnam]);

	//�ݹ鴴��Ŀ¼
	static bool CreateRecursiveDir(const std::string& strAbsolutePath);

	//���ļ�
	static FILE *OpenFile(const char *szFileName, bool bCreate=false, const char *pOpenType = (char *)"rb+");

	//�����ļ�
	static bool CopyFIle(const char* szSrcFile, const char * szDestFile);

	//��ȡһ��
	static U64 ReadOneLine(FILE& fp, void *& pBuffer, const U64& u64ReadDataLen);

	//��ȡ�ļ�
	static U64 ReadFile(FILE&fp, void *&pBuffer, const U64& u64ReadDataLen);

	//д���ļ�
	static U64 WriteFile(FILE& fp, const void *pBuffer, const U64& u64WriteDataLen);

	//�ر��ļ�
	static bool CloseFile(FILE& fp);

	//�ļ��Ƿ����
	static bool IsFileExist(const char * szFileName);

	//·���н�ȡ�ļ�
	static std::string GetFileNameInPath(const std::string& strPath);
	static std::string GetFileNameInPath(const char * szPath);
	static std::string GetFilePathInWholePath(const std::string& strPath);
	static std::string GetFilePathInWholePath(const char *szPath);

	static bool SetFileCursor(FILE& fp, FILE_CURSOR_POS ePos, long lOffset);
	static void ResetFileCursor(FILE& fp);
	static bool FlushFile(FILE &fp);
	static long GetFileSize(FILE& fp);		//�ļ��ߴ���ҪС��2GB long��ȡֵ��Χ�����⺯��������-1
	static I64 GetFileSizeEx(const char * filepath);


// #ifdef _WINNT_
// 
// 	//utf8->ansi ��windows api
// 	static std::string Utf8Convert2Ansi(const char *pUtf8);
// 	//ansi->utf8 ��windows api
// 	static std::string AnsiConvert2Utf8(const char *pAnsi);
// #endif

	//Internal fun
protected:
	//�����ļ���
	static bool CreateSubDir(const std::string& strDir);

	//�ݹ鴴����Ŀ¼
	static bool CreateRecursiveSubDir(const std::string& strMasterDir, const std::string& strSubDir);
};

NAMESPACE_END(gtool)

#endif