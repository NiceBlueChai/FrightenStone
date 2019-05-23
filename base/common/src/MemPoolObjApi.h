#ifndef MEM_POOL_OBJ_API_H_
#define MEM_POOL_OBJ_API_H_

#pragma once



//��������
#ifndef MEM_POOL_OBJ_API
#ifdef  MEM_POOL_OBJ_DLL
#define MEM_POOL_OBJ_API _declspec(dllexport)
#else
#define MEM_POOL_OBJ_API _declspec(dllimport)
#endif
#endif

//�����ļ���
#ifndef _DEBUG

#ifdef _UNICODE
#define MEM_POOL_OBJ_DLL_NAME				TEXT("MemPoolObj.dll")		//�������
#else
#define MEM_POOL_OBJ_DLL_NAME				"MemPoolObj.dll"				//�������
#endif

#else
#ifdef _UNICODE
#define MEM_POOL_OBJ_DLL_NAME				TEXT("MemPoolObjD.dll")		//�������
#else
#define MEM_POOL_OBJ_DLL_NAME				"MemPoolObjD.dll"			//�������
#endif
#endif

#ifndef MEM_POOL_OBJ_DLL
//�����ӿ�
#include <PoolObj/xByte.h>
#include <PoolObj/xUByte.h>

#endif



#endif

