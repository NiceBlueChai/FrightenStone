#include "stdafx.h"
#include <boost/scoped_array.hpp>
#include <BaseCode/WarningControl.h>

#include"FileDirSys.h"
#include<errno.h>
#include<stdlib.h>
#include<stdio.h>
#include <time.h>
#include <sys/stat.h>  //_stat����
#include<iostream>
#include<string>
#include <string.h>
#include <BaseCode/MyBaseMacro.h>

#if defined(linux)|defined(__CYGWIN__)
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#else
#include<direct.h>	//mkdir����
#include<io.h>//access����

//�����ļ���֧��
#include "shlwapi.h"
#pragma comment(lib,"shlwapi.lib")
#endif

NAMESPACE_BEGIN(gtool)
/////////////////////////////////////////////////////////
CFileDirSys::CFileDirSys(CFileDirSys&&)
{
}

// 
// #ifdef _WIN64
// 
// 
// //�ַ�ת��
// bool CFileDirSys::UnicodeToAnsi(LPTSTR szUnicodeSrc, char *pAnsiStrOut, int nAnsiStrSize, int *pAnsiOutStrlenByte /*= NULL*/)
// {
// 	//����У��
// 	if (!(szUnicodeSrc&&pAnsiStrOut&&nAnsiStrSize > 0))
// 	{
// 		ASSERT((szUnicodeSrc&&pAnsiStrOut&&nAnsiStrSize > 0));
// 		return false;
// 	}
// 	
// 
// 	ZeroMemory(pAnsiStrOut, nAnsiStrSize);
// 
// 	//��ȡת�������ֽ��� 
// 	auto nLength = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(szUnicodeSrc), -1, NULL, 0, NULL, NULL);
// 
// 	//ת���ռ�Ϸ����ж�
// 	if (nLength > nAnsiStrSize)
// 	{
// 		//������ֽ���
// 		if (pAnsiOutStrlenByte)	*pAnsiOutStrlenByte = nLength;
// 
// 		return false;
// 	}
// 
// 	int nTransFormResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)szUnicodeSrc, -1, pAnsiStrOut, nAnsiStrSize, NULL, NULL);//UnicodeתANSI����һ��������ǰϵͳANSI,�ڶ���������Ĭ��0�����ڶ����ǲ���ת�������滻���ַ������һ����Ĭ��true
// 
// 	if (nTransFormResult == 0) return false;
// 
// 	//ת���ֽ�������
// 	if (pAnsiOutStrlenByte)	*pAnsiOutStrlenByte = nTransFormResult;
// 
// 	return true;
// }
// 
// //�ַ�ת��	20170114	SHY
// bool CFileDirSys::AnsiToUnicode(char *pAnsiStrIn, LPTSTR szUnicodeStrOut, int nUnicodeCacheSize, int *nUnicodeStrLenOutCalculateZeroTail)
// {
// 	//����У��
// 	if (!(pAnsiStrIn && szUnicodeStrOut && nUnicodeCacheSize > 0))
// 	{
// 		ASSERT((pAnsiStrIn && szUnicodeStrOut && nUnicodeCacheSize > 0));
// 		return false;
// 	}
// 
// 	//������
// 	ZeroMemory(szUnicodeStrOut, nUnicodeCacheSize);
// 
// 	//��ȡת��������ַ�����
// 	auto nLength = MultiByteToWideChar(CP_ACP, 0, pAnsiStrIn, -1, NULL, 0);	//ccWideByteΪ0�����������
// 
// 																			//ת���ռ�Ϸ����ж�
// 	if (nLength * sizeof(TCHAR) > nUnicodeCacheSize)
// 	{
// 		//������ַ�����
// 		if (nUnicodeStrLenOutCalculateZeroTail)	*nUnicodeStrLenOutCalculateZeroTail = nLength;
// 
// 		return false;
// 	}
// 
// 	//ת��
// 	auto nTransFormResult = MultiByteToWideChar(CP_ACP, 0, pAnsiStrIn, -1, LPWSTR(szUnicodeStrOut), nUnicodeCacheSize);
// 	if (nTransFormResult == 0) return false;
// 
// 	//ת���ֽ�������
// 	if (nUnicodeStrLenOutCalculateZeroTail)	*nUnicodeStrLenOutCalculateZeroTail = nTransFormResult;
// 
// 	return true;
// }
// 
// 
// //utf8->ansi
// char * CFileDirSys::Utf8Convert2Ansi(const char *pUtf8)
// {
// 	if (!pUtf8)
// 	{
// 		ASSERT(pUtf8);
// 		return NULL;
// 	}
// 
// 	//utf8->unicode
// 	int n = MultiByteToWideChar(CP_UTF8, 0, pUtf8, -1, NULL, 0);
// 	IF_NOT_NL(n > 0)
// 		return NULL;
// 
// 	boost::scoped_array<char> pusResult(new char[2 * (n + 1)]);
// 	if (!pusResult) return NULL;
// 	memset(pusResult.get(), 0, 2 * sizeof(char) *(n + 1));
// 
// 	::MultiByteToWideChar(CP_UTF8, 0, pUtf8, -1, (LPWSTR)pusResult.get(), 2 * sizeof(char) *(n + 1));
// 
// 	n = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pusResult.get(), -1, 0, 0, 0, 0);
// 	char *pAnsi = new char[n + 1];
// 	if (!pAnsi) return NULL;
// 	memset(pAnsi, 0, n + 1);
// 	::WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pusResult.get(), -1, pAnsi, n + 1, 0, 0);
// 
// 	return pAnsi;
// }
// 
// //ansi->utf8
// char * CFileDirSys::AnsiConvert2Utf8(const char *pAnsi)
// {
// 	if (!pAnsi)
// 	{
// 		ASSERT(pAnsi);
// 		return NULL;
// 	}
// 	int n = MultiByteToWideChar(CP_ACP, 0, pAnsi, -1, NULL, 0);
// 	IF_NOT_NL(n > 0)
// 		return NULL;
// 
// 	boost::scoped_array<char> pusResult(new char[2 * (n + 1)]);
// 	if (!pusResult) return NULL;
// 	memset(pusResult.get(), 0, 2 * sizeof(char) *(n + 1));
// 
// 	::MultiByteToWideChar(CP_ACP, 0, pAnsi, -1, (LPWSTR)pusResult.get(), 2 * (n + 1));
// 
// 	n = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)pusResult.get(), -1, 0, 0, 0, 0);
// 
// 	int nAnsiLen = n + 1;
// 	char *pAnsiRet = new char[nAnsiLen];
// 	if (!pAnsiRet) return NULL;
// 	memset(pAnsiRet, 0, nAnsiLen);
// 	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)pusResult.get(), -1, (char*)pAnsiRet, nAnsiLen, 0, 0);
// 
// 	return pAnsiRet;
// }
// 
// #endif

bool CFileDirSys::SaveDataAsFile(const char *szFileName, const char *szString, const U64 nStrByte)
{
	if (!szFileName&&szString)
	{
		ASSERT(szFileName&&szString);
		return false;
	}

	FILE *fp = NULL;

	fp=OpenFile(szFileName, true, "wb");
	if (!fp)
	{
		ASSERT(!"�ļ�����ʧ��");
		return false;
	}

	//д�����ݣ�
	auto u64WriteLen = WriteFile(*fp, (const void *)szString, nStrByte);
	if (u64WriteLen != nStrByte)
	{
		ASSERT(CloseFile(*fp));
		DelFile(szFileName);
		return false;
	}

	ASSERT(FlushFile(*fp));
	ASSERT(CloseFile(*fp));
	return true;
}

// int CWordFileDirHandleSys::GenUnicode2Ansi(U16 unicodeKey, U16 *pAnsiKey)
// {
// 	if (!pAnsiKey)
// 	{
// 		ASSERT(!pAnsiKey);
// 		return -1;
// 	}
// 
// 	U32 first = 0, end, mid = 0;
// 
// 	end = sizeof(uni_ansi_table) / sizeof(UNICODE_ANSI_T) - 1;
// 	while (first <= end)
// 	{
// 		mid = (first + end) / 2;
// 
// 		if (uni_ansi_table[mid].unicode == unicodeKey)
// 		{
// 			*pAnsiKey = uni_ansi_table[mid].ansi;
// 			return 0;
// 		}
// 		else if (uni_ansi_table[mid].unicode > unicodeKey)
// 		{
// 			end = mid - 1;
// 		}
// 		else
// 		{
// 			first = mid + 1;
// 		}
// 	}
// 	return -1;
// }
// 
// #ifdef _WINNT_
// 
// 
// //�ַ�ת��
// bool CFileDirSys::UnicodeToAnsi(LPTSTR szUnicodeSrc, char *pAnsiStrOut, int nAnsiStrSize, int *pAnsiOutStrlenByte /*= NULL*/)
// {
// 	//����У��
// 	CHECKF(szUnicodeSrc&&pAnsiStrOut&&nAnsiStrSize > 0);
// 
// 	ZeroMemory(pAnsiStrOut, nAnsiStrSize);
// 
// 	//��ȡת�������ֽ��� 
// 	auto nLength = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(szUnicodeSrc), -1, NULL, 0, NULL, NULL);
// 
// 	//ת���ռ�Ϸ����ж�
// 	if (nLength > nAnsiStrSize)
// 	{
// 		//������ֽ���
// 		if (pAnsiOutStrlenByte)	*pAnsiOutStrlenByte = nLength;
// 
// 		return false;
// 	}
// 
// 	int nTransFormResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)szUnicodeSrc, -1, pAnsiStrOut, nAnsiStrSize, NULL, NULL);//UnicodeתANSI����һ��������ǰϵͳANSI,�ڶ���������Ĭ��0�����ڶ����ǲ���ת�������滻���ַ������һ����Ĭ��true
// 
// 	if (nTransFormResult == 0) return false;
// 
// 	//ת���ֽ�������
// 	if (pAnsiOutStrlenByte)	*pAnsiOutStrlenByte = nTransFormResult;
// 
// 	return true;
// }
// 
// //�ַ�ת��	20170114	SHY
// bool CFileDirSys::AnsiToUnicode(char *pAnsiStrIn, LPTSTR szUnicodeStrOut, int nUnicodeCacheSize, int *nUnicodeStrLenOutCalculateZeroTail)
// {
// 	//����У��
// 	CHECKF(pAnsiStrIn && szUnicodeStrOut && nUnicodeCacheSize > 0);
// 
// 	//������
// 	ZeroMemory(szUnicodeStrOut, nUnicodeCacheSize);
// 
// 	//��ȡת��������ַ�����
// 	auto nLength = MultiByteToWideChar(CP_ACP, 0, pAnsiStrIn, -1, NULL, 0);	//ccWideByteΪ0�����������
// 
// 																			//ת���ռ�Ϸ����ж�
// 	if (nLength * sizeof(TCHAR) > nUnicodeCacheSize)
// 	{
// 		//������ַ�����
// 		if (nUnicodeStrLenOutCalculateZeroTail)	*nUnicodeStrLenOutCalculateZeroTail = nLength;
// 
// 		return false;
// 	}
// 
// 	//ת��
// 	auto nTransFormResult = MultiByteToWideChar(CP_ACP, 0, pAnsiStrIn, -1, LPWSTR(szUnicodeStrOut), nUnicodeCacheSize);
// 	if (nTransFormResult == 0) return false;
// 
// 	//ת���ֽ�������
// 	if (nUnicodeStrLenOutCalculateZeroTail)	*nUnicodeStrLenOutCalculateZeroTail = nTransFormResult;
// 
// 	return true;
// }
// 
// #endif



//ɾ���ļ�
void CFileDirSys::DelFile(const char *szfile)
{
	std::string strDelCmd = "del ";
	strDelCmd += szfile;
	size_t i32FindPos = 0;
	int nCount = 0;
	const auto strCount = strDelCmd.length();
	while ( (i32FindPos = strDelCmd.find_first_of('/', i32FindPos))!=std::string::npos)
	{
		DEAD_LOOP_BREAK(nCount, strCount);
		strDelCmd[i32FindPos] = '\\';
	}
	strDelCmd += " /f/s/q";

	system(strDelCmd.c_str());
}

bool CFileDirSys::DelFileCStyle(const char *szfile)
{
	return remove(szfile) == 0;
}

FILE * CFileDirSys::GenTmpFile()
{
	return tmpfile();
}

char * CFileDirSys::GenRandFileName(char szRandName[L_tmpnam])
{
	return tmpnam(szRandName);
}

char * CFileDirSys::GenRandFileNameNoDir(char szRandName[L_tmpnam])
{
	if (tmpnam(szRandName))
	{
		auto strFile = GetFileNameInPath(szRandName);
		if (strFile.length() > 0)
		{
			szRandName[0] = 0;
			auto nLen = sprintf(szRandName, "%s", strFile.c_str());
			nLen = ((nLen < L_tmpnam) ? gtool::Max(nLen, 0) : (L_tmpnam - 1));
			szRandName[nLen] = 0;
			return szRandName;
		}
	}

	return NULL;
}

//�����ļ���
bool CFileDirSys::CreateSubDir(const std::string& strDir)
{
	if (strDir.length() <= 0)
	{
		ASSERT(strDir.length() > 0);
		return false;
	}

#if defined(linux)|defined(__CYGWIN__)
	if (mkdir(strDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
	{
		//�ļ����Ҳ���
		if (access(strDir.c_str(), 0) == -1)
		{
			return false;	//0�ɹ�
		}
	}
	
#else
	if (mkdir(strDir.c_str()) != 0)
	{
		//�ļ����Ҳ���
		if (access(strDir.c_str(), 0) == -1)
		{
			return false;	//0�ɹ�
		}
	}
#endif


	return true;
}

//�ݹ鴴��Ŀ¼
bool CFileDirSys::CreateRecursiveDir(const std::string& strAbsolutePath)
{
	//��ȡ�̷�
	std::string strRoot, strSubPath;
	auto nStartPos=strAbsolutePath.find(":", 0);
	if (nStartPos == std::string::npos)
	{
		strRoot = "";
		strSubPath += "\\";
		strSubPath += strAbsolutePath;
	}
	else
	{
		strRoot = strAbsolutePath.substr(nStartPos - 1, 1);
		strRoot += ":\\";
		nStartPos = strAbsolutePath.find("\\", nStartPos + strlen(":"));
		if (nStartPos == std::string::npos)
		{
			ASSERT(!"nStartPos == std::string::npos");
			return false;
		}
		strSubPath = strAbsolutePath.substr(nStartPos, strAbsolutePath.length() - nStartPos);
		
	}

	return CreateRecursiveSubDir(strRoot, strSubPath);
}

//�ݹ鴴����Ŀ¼
bool CFileDirSys::CreateRecursiveSubDir(const std::string& strMasterDir, const std::string& strSubDir)
{
	std::string dir = "";
	std::string strtocreate="";
	std::string strMasterPath = strMasterDir;
	int iStartPos = 0;
	int iStartPosTmp = 0;
	int iEndPos = 0;
	int iEndPosTmp = 0;

	int iFinalStartPos = 0, iFinalCount = 0;
	int nCount = 0;
	const size_t nStrCount = strMasterPath.length() + strSubDir.length();
	int nSpritLen = int(strlen("\\"));
	int nRevSpritLen = int(strlen("/"));
	while (true)
	{
		DEAD_LOOP_BREAK(nCount, nStrCount);
		iStartPos = int(strSubDir.find("\\", iStartPos));
		iStartPosTmp = int(strSubDir.find("/", iStartPosTmp));
		if(iStartPos == std::string::npos&&iStartPosTmp == std::string::npos) break;

		if (iStartPos == std::string::npos || iStartPosTmp == std::string::npos)
		{
			if (iStartPos != std::string::npos)
			{
				iEndPos = int(strSubDir.find("\\", iStartPos + nSpritLen));
				if (iEndPos == std::string::npos) break;
				iFinalStartPos = iStartPos+ nSpritLen;
				iFinalCount = iEndPos - iStartPos - nSpritLen;
				iStartPos += nSpritLen;
			}

			if (iStartPosTmp != std::string::npos)
			{
				iEndPosTmp = int(strSubDir.find("/", iStartPosTmp + nRevSpritLen));
				if (iEndPosTmp == std::string::npos) break;
				iFinalStartPos = iStartPosTmp + nRevSpritLen;
				iFinalCount = iEndPosTmp - iStartPosTmp - nRevSpritLen;
				iStartPosTmp += nRevSpritLen;
			}
		}
		else if (iStartPos < iStartPosTmp)
		{
			//"\\"

			iEndPos = int(strSubDir.find("\\", iStartPos + nSpritLen));
			iEndPosTmp = int(strSubDir.find("/", iStartPos + nSpritLen));
			if (iEndPos == std::string::npos&&iEndPosTmp == std::string::npos) break;
			dir.clear();
			if (iEndPos == std::string::npos || iEndPosTmp == std::string::npos)
			{
				if (iEndPos != std::string::npos)
				{
					iFinalStartPos = iStartPos + nSpritLen;
					iFinalCount = iEndPos - iStartPos - nSpritLen;
				}
				if (iEndPosTmp != std::string::npos)
				{
					iFinalStartPos = iStartPos + nSpritLen;
					iFinalCount = iEndPosTmp - iStartPos - nSpritLen;
				}
			}
			else if (iEndPos < iEndPosTmp)
			{
				//"\\"
				iFinalStartPos = iStartPos + nSpritLen;
				iFinalCount = iEndPos - iStartPos - nSpritLen;
				//dir = strMasterPath + strSubDir.substr(iStartPos + strlen("\\"), iEndPos - iStartPos - strlen("\\"));
			}
			else
			{
				//"/"
				iFinalStartPos = iStartPos + nSpritLen;
				iFinalCount = iEndPosTmp - iStartPos - nSpritLen;
				//dir = strMasterPath + strSubDir.substr(iStartPos + strlen("\\"), iEndPos - iStartPos - strlen("\\"));
			}

			iStartPos += nSpritLen;
			
		}
		else
		{
			//"/"
			iFinalStartPos = iStartPosTmp + nRevSpritLen;
			iEndPos = int(strSubDir.find("\\", iStartPosTmp + nRevSpritLen));
			iEndPosTmp = int(strSubDir.find("/", iStartPosTmp + nRevSpritLen));
			if (iEndPos == std::string::npos&&iEndPosTmp == std::string::npos) break;
			if (iEndPos == std::string::npos || iEndPosTmp == std::string::npos)
			{
				if (iEndPos != std::string::npos)
				{					
					iFinalCount = iEndPos - iFinalStartPos;
				}
				if (iEndPosTmp != std::string::npos)
				{
					iFinalCount = iEndPosTmp - iFinalStartPos;
				}
			}
			else if (iEndPos < iEndPosTmp)
			{
				//"\\"
				iFinalCount = iEndPos - iFinalStartPos;
				//dir = strMasterPath + strSubDir.substr(iStartPos + strlen("\\"), iEndPos - iStartPos - strlen("\\"));
			}
			else
			{
				//"/"
				iFinalCount = iEndPosTmp - iFinalStartPos;
				//dir = strMasterPath + strSubDir.substr(iStartPos + strlen("\\"), iEndPos - iStartPos - strlen("\\"));
			}

			iStartPosTmp += nRevSpritLen;
		}

		//������Ŀ¼
		dir.clear();
		strtocreate.clear();
		dir = strMasterPath; 
		if (dir.length()!=0) dir += "/";
		dir+=strSubDir.substr(iFinalStartPos, iFinalCount);
		strtocreate = dir;
		strtocreate += "/";
		if (strcmp((char *)strtocreate.c_str(), "./") != 0)
		{
			ASSERT(CreateSubDir(strtocreate));
		}

		strMasterPath.clear();
		strMasterPath = dir;
	}

	return true;
}

//���ļ�
FILE * CFileDirSys::OpenFile(const char *szFileName,  bool bCreate/*=false*/, const char *pOpenType /*= (char *)"rb+"*/)
{
	//����У��
	if (szFileName== 0 || pOpenType == NULL)
	{
		ASSERT(!"�ļ�·��Ϊ�ջ����ļ���ģʽΪ��");
		return NULL;
	}

	FILE *fp = NULL;
	fp = fopen(szFileName, pOpenType);
	if(!fp)
	{
		if (bCreate)
		{
			fp = fopen(szFileName, "wb+");
			if (!fp)
			{
				ASSERT(fp);
				return NULL;
			}
		}
		else
		{
			ASSERT(!"���ļ�ʧ��");
			return NULL;
		}

	}
	
	//����������ض�λ�������ʼ��
	clearerr(fp);
	rewind(fp);

	return fp;
}

//�����ļ�
bool CFileDirSys::CopyFIle(const char* szSrcFile, const char * szDestFile)
{
	//����У��
	CHECKF_NL(szSrcFile&&szDestFile);
	auto fpSrc=OpenFile(szSrcFile);
	CHECKF_NL(fpSrc);
	auto fpDest = OpenFile(szDestFile, true, "wb+");
	CHECKF_NL(fpDest);

// 	clearerr(fpSrc);
// 	clearerr(fpDest);
// 	rewind(fpSrc);
// 	rewind(fpDest);

	unsigned char get_c = 0;
	char cCount = 0, cWrCount=0;
	const auto nSize=GetFileSize(*fpSrc);	//�ļ��ߴ�
	long lLoopCount = 0;
	while (!feof(fpSrc))
	{
		if(nSize>=0)
			DEAD_LOOP_BREAK(lLoopCount, nSize);
		
		get_c = 0;
		cCount = char(fread(&get_c, 1, 1, fpSrc));
		if(cCount != 1) break;	//�ж϶�ȡ�Ƿ������ߵ��ļ���βfeof���ӳ�
		cWrCount = char(fwrite(&get_c, 1, 1, fpDest));
		if (cWrCount != 1)
		{
			ASSERT(!("д�����nWrCount != 1"));
			break;
		}
		FlushFile(*fpDest);
	}

	CloseFile(*fpSrc);
	CloseFile(*fpDest);

	return true;
}

//��ȡһ��
U64 CFileDirSys::ReadOneLine(FILE& fp, void *& pBuffer, const U64& u64ReadDataLen)
{
	CHECKF_NL(pBuffer&&u64ReadDataLen != 0);
	
	unsigned char get_c = 0;
	unsigned char *pBufferTmp = (unsigned char *)pBuffer;
	memset(pBufferTmp, 0, u64ReadDataLen);

	U64 u64Count = 0;
	U64 u64LoopCount = 0;
	while (true)
	{
		DEAD_LOOP_BREAK(u64LoopCount, u64ReadDataLen);

		get_c = 0;
		if (fread(&get_c, sizeof(get_c), 1, &fp) == 1)
		{
#if (defined(_WIN32)|defined(_WIN64))
			//if (get_c == '\r') ASSERT(0);
			if (get_c != '\n')
			{
				*pBufferTmp = get_c;
				pBufferTmp++;
				u64Count++;

				if (u64ReadDataLen <= u64Count) break;	//��ֹ���
			}
			else
			{
				ASSERT(SetFileCursor(fp, FILE_CURSOR_POS_CUR, 1));
				break;
			}

#else

			if (get_c != '\n')
			{
				*pBufferTmp = get_c;
				pBufferTmp++;
				u64Count++;

				if (u64ReadDataLen <= u64Count) break;	//��ֹ���
			}
			else
			{
				//fread(&get_c, sizeof(get_c), 1, fpOutCache);
				break;
			}

#endif

		}
		else
		{
			break;
			//�ļ���β
		}
	}

	return u64Count;

}

//��ȡ�ļ�
U64 CFileDirSys::ReadFile(FILE&fp, void *&pBuffer, const U64& u64ReadDataLen)
{
	CHECKF_NL(pBuffer);

	U64 u64Read = 0;
	unsigned char *pBufferTmp = (unsigned char *)pBuffer;
	unsigned char get_c = 0;
	while (!feof(&fp))
	{
		get_c = 0;
		if (fread(&get_c, sizeof(get_c), 1, &fp) == 1)
		{
			*pBufferTmp = get_c;
			pBufferTmp++;
			u64Read++;

			//��������
			if(u64Read>=u64ReadDataLen) break;
		}
		else
		{
			break;
			//�ļ���β
		}
	}

	return u64Read;
}

//д���ļ�
U64 CFileDirSys::WriteFile(FILE& fp, const void *pBuffer, const U64& u64WriteDataLen)
{
	CHECKF_NL(pBuffer);

	U64 u64Count = 0;
	unsigned char *pData = (unsigned char *)pBuffer;
	U64 u64LoopCount = 0;
	while (true)
	{
		DEAD_LOOP_BREAK(u64LoopCount, u64WriteDataLen);
		if (fwrite(pData, 1, 1, &fp) == 1)
		{
			pData++;
			u64Count++;
		}
		else
		{
			break;
		}
		if (u64Count >= u64WriteDataLen)
		{
			break;
		}
	}

	if (u64WriteDataLen != u64Count)
	{
		ASSERT(!("�ļ�д�����"));
	}
	return u64Count;
}

//�ر��ļ�
bool CFileDirSys::CloseFile(FILE& fp)
{
	clearerr(&fp);
	if (!(fclose(&fp) == 0))
	{
		return false;
	}

	return true;
}


//�ļ��Ƿ����
bool CFileDirSys::IsFileExist(const char * szFileName)
{
	CHECKF_NL(szFileName);

	//�ļ�
	if (access(szFileName, 0) == -1)
	{
		return false;
	}

	return true;
}

//·���н�ȡ�ļ�
std::string CFileDirSys::GetFileNameInPath(const std::string& strPath)
{
	if (!(strPath.length() > 0))
	{
		ASSERT(!"·������");
		return "";
	}

	char c = 0;
	int i = 0;
	for (i = I32(strPath.length() - 1); i >= 0; --i)
	{
		c = strPath.at(i);
		if (c == '\\'|| c=='/')
		{
			i++;
			break;
		}
	}

	if (i < 0)
	{
		ASSERT(!"i<0");
		return "";
	}

	return strPath.substr(i, strPath.length() - i);
}

std::string CFileDirSys::GetFileNameInPath(const char * szPath)
{
	std::string strPath = szPath;

	if (!(strPath.length() > 0))
	{
		ASSERT(!"·������");
		return "";
	}

	char c = 0;
	int i = 0;
	for (i = I32(strPath.length() - 1); i >= 0; --i)
	{
		c = strPath.at(i);
		if (c == '\\' || c == '/')
		{
			i++;
			break;
		}
	}

	if (i < 0)
	{
		ASSERT(!"i<0");
		return "";
	}

	return strPath.substr(i, strPath.length() - i);

}

std::string CFileDirSys::GetFilePathInWholePath(const std::string& strPath)
{
	if (!(strPath.length() > 0))
	{
		ASSERT(!"·������");
		return "";
	}

	char c = 0;
	int i = 0;
	for (i = I32(strPath.length() - 1); i >= 0; --i)
	{
		c = strPath.at(i);
		if (c == '\\' || c == '/')
		{
			i++;
			break;
		}
	}

	if (i < 0)
	{
		ASSERT(!"i<0");
		return "";
	}

	return strPath.substr(0, i);
}

std::string CFileDirSys::GetFilePathInWholePath(const char *szPath)
{
	std::string strPath = szPath;
	if (!(strPath.length() > 0))
	{
		ASSERT(!"·������");
		return "";
	}

	char c = 0;
	int i = 0;
	for (i = I32(strPath.length() - 1); i >= 0; --i)
	{
		c = strPath.at(i);
		if (c == '\\' || c == '/')
		{
			i++;
			break;
		}
	}

	if (i < 0)
	{
		ASSERT(!"i<0");
		return "";
	}

	return strPath.substr(0, i);
}

bool CFileDirSys::SetFileCursor(FILE& fp, FILE_CURSOR_POS ePos, long lOffset)
{
	return fseek(&fp, lOffset, ePos)==0;
}

void CFileDirSys::ResetFileCursor(FILE& fp)
{
	clearerr(&fp);
	rewind(&fp);
}

bool CFileDirSys::FlushFile(FILE &fp)
{
	return fflush(&fp)==0;
}

long CFileDirSys::GetFileSize(FILE& fp)
{
	auto lCurPos=ftell(&fp);
	if (lCurPos < 0) return -1;
	if(!SetFileCursor(fp, FILE_CURSOR_POS_END, 0L)) return -1;

	auto nSize = ftell(&fp);
	if (nSize < 0)
	{
		ASSERT(SetFileCursor(fp, FILE_CURSOR_POS_SET, lCurPos));
		return -1;
	}

	ASSERT(SetFileCursor(fp, FILE_CURSOR_POS_SET, lCurPos));

	return nSize;
}

//_stat ����0�ɹ�
I64 CFileDirSys::GetFileSizeEx(const char * filepath)
{
	struct _stat info;
	if (_stat(filepath, &info) != 0)
		return -1;

	return info.st_size;
}



NAMESPACE_END(gtool)


