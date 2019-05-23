#ifndef __Base_Common_Assist_Utils_Impl_FS_FileUtil_H__
#define __Base_Common_Assist_Utils_Impl_FS_FileUtil_H__
/**
* @file STLUtil.h
* @auther Huiya Song <120453674@qq.com>
* @date 2019/05/08
* @brief
*/

#pragma once

#include<base/exportbase.h>
#include "base/common/basedefs/DataType/DataType.h"
#include "base/common/basedefs/Macro/MacroDefs.h"
#include<stdio.h>

FS_NAMESPACE_BEGIN

class FS_String;
class Time;

class BASE_EXPORT FS_FileUtil
{
public:
    // ɾ���ļ�
    static void DelFile(const char *filePath);
    // ɾ���ļ�
    static bool DelFileCStyle(const char *filePath);
    // ������ʱ�ļ� ��w+b��ģʽ�򿪷�����ʱ�ļ�����ָ��
    static FILE *CreateTmpFile();
    // ��ȡ�����Ψһ���ļ��� ���鳤������Ϊ L_tmpnam 
    static const char *GenRandFileName(char randName[L_tmpnam]);
    static const char *GenRandFileNameNoDir(char randName[L_tmpnam]);
    // ���ļ�
    static FILE *OpenFile(const char *fileName, bool isCreate = false, const char *openType = "rb+");
    // �����ļ�
    static bool CopyFIle(const char *srcFile, const char *destFile);
    // ��ȡһ��
    static UInt64 ReadOneLine(FILE &fp, UInt64 bufferSize, char *&buffer);
    static UInt64 ReadOneLine(FILE &fp, FS_String &outBuffer);
    // ��ȡ�ļ�
    static UInt64 ReadFile(FILE &fp, UInt64 bufferSize, char *&buffer);
    static UInt64 ReadFile(FILE &fp, FS_String &outString, Int64 sizeLimit = -1);
    // д���ļ�
    static UInt64 WriteFile(FILE &fp, const char *buffer, UInt64 dataLenToWrite);
    static UInt64 WriteFile(FILE &fp, const FS_String &bitData);
    // �ر��ļ�
    static bool CloseFile(FILE &fp);
    // �ļ��Ƿ����
    static bool IsFileExist(const char *fileName);
    // �����ļ�ָ��λ��
    static bool SetFileCursor(FILE &fp, Int32 enumPos, long offset);
    // �����ļ�ָ��λ�õ��ļ�ͷ
    static void ResetFileCursor(FILE &fp);
    // �ļ���������
    static bool FlushFile(FILE &fp);
    // ��ȡ�ļ���С
    static long GetFileSize(FILE &fp);  // �ļ��ߴ���ҪС��2GB long��ȡֵ��Χ�����⺯��������-1
    static Int64 GetFileSizeEx(const char *filepath);       // 

    // ���ʱ��
    static void InsertFileTime(const FS_String &extensionName, const Time &timestamp, FS_String &fileName);

    // ���β����ʶ
    static void InsertFileTail(const FS_String &extensionName, const char *tail, FS_String &fileName);

    // ��ȡ��չ��
    static FS_String ExtractFileExtension(const FS_String &fileName);
};

FS_NAMESPACE_END

#endif
