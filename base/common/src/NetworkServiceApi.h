#ifndef NETWORK_SERVICE_API_H_
#define NETWORK_SERVICE_API_H_

#pragma once



//��������
#ifndef NETWORK_SERVICE_API
#ifdef  NETWORK_SERVICE_DLL
#define NETWORK_SERVICE_API _declspec(dllexport)
#else
#define NETWORK_SERVICE_API _declspec(dllimport)
#endif
#endif

//�����ļ���
#ifndef _DEBUG

#ifdef _UNICODE
#define NETWORK_SERVICE_DLL_NAME				TEXT("NetworkService.dll")		//�������
#else
#define NETWORK_SERVICE_DLL_NAME				"NetworkService.dll"				//�������
#endif

#else
#ifdef _UNICODE
#define NETWORK_SERVICE_DLL_NAME				TEXT("NetworkServiceD.dll")		//�������
#else
#define NETWORK_SERVICE_DLL_NAME				"NetworkServiceD.dll"			//�������
#endif
#endif

#ifndef NETWORK_SERVICE_DLL
//�����ӿ�
 //#include <Net/NetConfig.h>
// #include <Net/IOCPModel.h>
#include <Net/SocketProtocol.h>
#include <Net/NetMacro.h>
#include <Net/NetEnum.h>
#include<Net/NetWorkModule.h>

#endif



#endif

