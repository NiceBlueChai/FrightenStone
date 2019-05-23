#ifndef HEAP_EX_API_H_
#define HEAP_EX_API_H_

#pragma once


//��������
#ifndef HEAP_EX_API
#ifdef  HEAP_EX_DLL
#define HEAP_EX_API _declspec(dllexport)
#else
#define HEAP_EX_API _declspec(dllimport)
#endif
#endif

//�����ļ���
#ifndef _DEBUG

#ifdef _UNICODE
#define HEAP_DLL_NAME				TEXT("HeapEx.dll")		//�������
#else
#define HEAP_DLL_NAME				"HeapEx.dll"				//�������
#endif

#else
#ifdef _UNICODE
#define HEAP_DLL_NAME				TEXT("HeapExD.dll")		//�������
#else
#define HEAP_DLL_NAME				"HeapExD.dll"			//�������
#endif
#endif

#ifndef HEAP_EX_DLL
#include <Heap/HeapMgr.h>
#endif


#endif

