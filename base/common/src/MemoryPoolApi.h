#ifndef MEMORY_POOL_API_H_
#define MEMORY_POOL_API_H_

#pragma once


//��������
#ifndef MEMORY_POOL_API
	#ifdef  MEMORY_POOL_DLL
		#define MEMORY_POOL_API _declspec(dllexport)
	#else
		#define MEMORY_POOL_API _declspec(dllimport)
	#endif
#endif

//�����ļ���
#ifndef _DEBUG

	#ifdef _UNICODE
		#define MEMORY_POOL_DLL_NAME				TEXT("MemoryPool.dll")		//�������
	#else
		#define MEMORY_POOL_DLL_NAME				"MemoryPool.dll"				//�������
	#endif

#else
	#ifdef _UNICODE
		#define MEMORY_POOL_DLL_NAME				TEXT("MemoryPoolD.dll")		//�������
	#else
		#define MEMORY_POOL_DLL_NAME				"MemoryPoolD.dll"			//�������
	#endif
#endif

#ifndef MEMORY_POOL_DLL
//�����ӿ�
//#include <BaseCode/BaseCode.inl>
#include <PoolSrc/UseMemPool.h>
#include <PoolSrc/MemPoolModule.h>
#include <PoolSrc/MemPoolSwich.hpp>


#endif

#endif