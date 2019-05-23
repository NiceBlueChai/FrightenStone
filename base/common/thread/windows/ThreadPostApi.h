#ifndef THREAD_POST_API_H
#define THREAD_POST_API_H

#pragma once


//��������
#ifndef THREAD_POST_API
#ifdef  THREAD_POST_DLL
#define THREAD_POST_API _declspec(dllexport)
#else
#define THREAD_POST_API _declspec(dllimport)
#endif
#endif

//�����ļ���
#ifndef _DEBUG

#ifdef _UNICODE
#define THREAD_POST_API_DLL_NAME				TEXT("ThreadPost.dll")		//�������
#else
#define THREAD_POST_API_DLL_NAME				"ThreadPost.dll"				//�������
#endif

#else
#ifdef _UNICODE
#define THREAD_POST_API_DLL_NAME				TEXT("ThreadPostD.dll")		//�������
#else
#define THREAD_POST_API_DLL_NAME				"ThreadPostD.dll"			//�������
#endif
#endif

//////////////////////////////////////////////////////////////////////////////////

//�����ļ�
#ifndef THREAD_POST_DLL







#endif



NAMESPACE_BEGIN(THREAD_TRANSFER)

extern bool THREAD_POST_API InitThreadPostModule();

extern bool THREAD_POST_API FiniThreadPostModule();

//pRecvObj ֻ������һ�Σ�����󲻻�ı� Ҫ��֤ pRecvObj ��post����������ʼ����Ч������������Ϊ����ȫ�� pData����Ϊ���������������ձ�����ָ��������
extern bool THREAD_POST_API Post2Thread(const void * pRecvObj, POST_OBJ_TYPE eType, const U64 u64Param, const void *pData, const I64 nSize); //�̰߳�ȫ

extern bool THREAD_POST_API IsMsgQueueEmpty(const void * pRecvObj, POST_OBJ_TYPE eThreadType);	//�̰߳�ȫ

extern bool THREAD_POST_API RecverWaitAndHandle(const void *pRecvObj, POST_OBJ_TYPE eThreadType); //�̰߳�ȫ

extern bool THREAD_POST_API SetThreadAwakeStatus(const void *pRecvObj, POST_OBJ_TYPE eThreadType, bool bAwake); //�̰߳�ȫ

extern bool THREAD_POST_API WaitForFini(const void *pRecvObj, POST_OBJ_TYPE eThreadType, const bool bBreakLoopFinal); //�̰߳�ȫ
extern void THREAD_POST_API SetEnablePost(const void *pRecvObj, POST_OBJ_TYPE eThreadType, const bool bEnable); //ʹ��Post


//extern bool THREAD_POST_API SetDisplayFun(const void *pRecvObj, POST_OBJ_TYPE eThreadType, void* DisplayFunPtr);

NAMESPACE_END(THREAD_TRANSFER)

#endif

