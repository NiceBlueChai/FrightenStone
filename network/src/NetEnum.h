#ifndef NET_ENUM_H_
#define NET_ENUM_H_

#pragma once

//////////////////////////////////////////////////////////////////
// ����ɶ˿���Ͷ�ݵ�I/O����������
typedef enum _OPERATION_TYPE
{
	NULL_POSTED=0,                       // ���ڳ�ʼ����������
	ACCEPT_POSTED,                     // ��־Ͷ�ݵ�Accept����
	SEND_POSTED,                       // ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV_POSTED,                       // ��־Ͷ�ݵ��ǽ��ղ���
//	CONNECT_POSTED,						//�������

}OPERATION_TYPE;

//�׽��ֻ�������
enum SOCKET_MACHINE_TYPE
{
	SOCKET_MACHINE_TYPE_SVR=0,	//�������׽���
	SOCKET_MACHINE_TYPE_CLIENT,	//�ͻ����׽���
};

enum SOCKET_STATUS_TYPE
{
	SOCKET_STATUS_TYPE_CLOSED=0,	//�׽��ֹر�
	SOCKET_STATUS_TYPE_RECVED=1,	//�׽��ֽ�������
};

enum SOCKET_CACHE_TYPE
{
	SOCKET_CACHE_TYPE_NONE=0,	//��Ч
	SOCKET_CACHE_TYPE_RECV,		//recv������
	SOCKET_CACHE_TYPE_SEND,		//send������
	SOCKET_CACHE_TYPE_END,
};

//�������ݻ�������
enum NET_CACHE_TYPE
{
	NET_CACHE_TYPE_NONE=0,
	NET_CACHE_TYPE_NORMAL = 1,		//������
	NET_CACHE_TYPE_REPLY = 2,		//�ظ���
	NET_CACHE_TYPE_END,
};

#endif

