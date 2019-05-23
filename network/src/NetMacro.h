#ifndef NET_MACRO_H_
#define NET_MACRO_H_

#pragma once

// ���������� (1024*8)
// ֮����Ϊʲô����8K��Ҳ��һ�������ϵľ���ֵ
// ���ȷʵ�ͻ��˷�����ÿ�����ݶ��Ƚ��٣���ô�����õ�СһЩ��ʡ�ڴ�
#ifndef MAX_BUFFER_LEN
#define MAX_BUFFER_LEN        8192
#endif


// Ĭ�϶˿�
#ifndef DEFAULT_PORT
#define DEFAULT_PORT          12345  
#endif

#ifndef INVALID_SOCKET_IDD
#define INVALID_SOCKET_IDD	U64(~0)		//��Ч�׽���
#endif

#ifndef SEND_FALE_COUNT_MAX
#define SEND_FALE_COUNT_MAX 5			//����5��ʧ���򲻷���
#endif

#ifndef IS_VALID_SOCKET_IDD
#define IS_VALID_SOCKET_IDD(x)  (((x)!=INVALID_SOCKET_IDD)&&((x)!=0))		//�Ƿ���Ч��socketIDD
#endif

#ifndef SET_INVALID_SOCKET
#define  SET_INVALID_SOCKET(x)	(x)=INVALID_SOCKET_IDD
#endif

#ifndef EXIT_CODE
#define EXIT_CODE						NULL	// ���ݸ�Worker�̵߳��˳��ź�
#endif

#define WORKER_THREADS_PER_PROCESSOR	2		// ÿһ���������ϲ������ٸ��߳�(Ϊ������޶ȵ��������������ܣ���������ĵ�)
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}	// �ͷž����
#define RELEASE_SOCKET(x)               {if(IS_VALID_SOCKET_IDD(x)) {  ASSERTNET(closesocket(x)==0);x=INVALID_SOCKET;}}	// �ͷ�Socket��

#endif

