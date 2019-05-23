#ifndef ENUM_TYPE_H_
#define ENUM_TYPE_H_
#pragma once

enum DEL_WAY
{
	DEL_WAY_DELETE=0,
	DEL_WAY_DELETE_MULTI=1,	//delete []
	DEL_WAY_RELEASE=2,
	DEL_WAY_NOT_OWNER=3,
};

enum WIN_WAIT_EVENT
{
	WIN_WAIT_EVENT_FAILED=-1,
	WIN_WAIT_EVENT_SUC=0,
	WIN_WAIT_EVENT_TIMEOUT=1,
};

enum BUFFER_LEN_TYPE
{
	BUFFER_LEN_TYPE_ZERO=0,
	BUFFER_LEN_TYPE_32,
	BUFFER_LEN_TYPE_64,
	BUFFER_LEN_TYPE_128,
	BUFFER_LEN_TYPE_256,
	BUFFER_LEN_TYPE_512,
	BUFFER_LEN_TYPE_1024,
	BUFFER_LEN_TYPE_2048,
	BUFFER_LEN_TYPE_4096,
	BUFFER_LEN_TYPE_8192,
	BUFFER_LEN_TYPE_9216,
	BUFFER_LEN_TYPE_10240,	//10K
	BUFFER_LEN_TYPE_64K,	//64K
	BUFFER_LEN_TYPE_128K,	//128K
	BUFFER_LEN_TYPE_256K,	//256K
	BUFFER_LEN_TYPE_512K,	//512K
	BUFFER_LEN_TYPE_1M,		//1MB
};

//������������
enum BUFFER_DATA_TYPE
{
	BUFFER_DATA_TYPE_NULL=0,
	BUFFER_DATA_TYPE_CONST_CHAR_PTR,
	BUFFER_DATA_TYPE_CHAR_PTR,
	BUFFER_DATA_TYPE_CONST_CHAR_ARRAY,
	BUFFER_DATA_TYPE_CHAR_ARRAY,
	BUFFER_DATA_TYPE_FLOAT,
	BUFFER_DATA_TYPE_DOUBLE,
	BUFFER_DATA_TYPE_OTHER_INTEGER,
};

enum DUMPER_BUFFER_TYPE
{
	DUMPER_BUFFER_TYPE_NONE=0,
	DUMPER_BUFFER_TYPE_SMALLER,						//��С��Ϣ
	DUMPER_BUFFER_TYPE_NORMAL,						//������Ϣ����Ϣ�㹻
	DUMPER_BUFFER_TYPE_MOST_DETAILED,				//����ϸ����Ϣ����Ϣ�Ƚϴ�
};

enum NETWORK_CARD_TYPE
{
	NETWORK_CARD_TYPE_0=0,	//0������
	NETWORK_CARD_TYPE_END= NETWORK_CARD_MAX_NUM+ NETWORK_CARD_TYPE_0 -1,	//�������
};

//cpu ��ʱ����
enum TIMER_ACCURACY_TYPE
{
	TIMER_ACCURACY_TYPE_BEGIN=0,	//��
	TIMER_ACCURACY_TYPE_SEC,		//��
	TIMER_ACCURACY_TYPE_MILL_SEC,	//����
	TIMER_ACCURACY_TYPE_MICRO_SEC,	//΢��
	TIMER_ACCURACY_TYPE_NANO_SEC,	//����
	TIMER_ACCURACY_TYPE_END,
};

//�ڴ�ؿ���
enum POOL_SWITCH_TYPE
{
	POOL_SWITCH_TYPE_CLOSE,		//Ĭ�ϲ�ʹ���ڴ��
	POOL_SWITCH_TYPE_OPEN,		//�����ڴ�� ��Ҫ����ڴ�ع���dllʹ��
};


//��ַЭ��
NAMESPACE_BEGIN(NET_SYS)

//������
enum FAMILY_TYPE
{
	FAMILY_TYPE_UNSPEC=0,		//Э���޹�
	FAMILY_TYPE_AF_INET=2,		//IPV4
	FAMILY_TYPE_AF_INET6=23,	//IPV6
};

//Э������
enum PROTOCOL_TYPE
{
	PROTOCOL_TYPE_IPPROTO_IP=0,		//ipЭ��
	PROTOCOL_TYPE_IPPROTO_IPV4 =4,	//ipv4
	PROTOCOL_TYPE_IPPROTO_IPV6 =41,	//IPV6
	PROTOCOL_TYPE_IPPROTO_UDP=17,	//udp
	PROTOCOL_TYPE_IPPROTO_TCP=6,	//tcp
};

//������׽�������
enum SOCK_TYPE
{
	SOCK_TYPE_SOCK_STREAM=1,		//��
	SOCK_TYPE_SOCK_DGRAM=2,			//���ݱ���
};

enum AI_FLAGS_TYPE
{
	AI_FLAGS_TYPE_NONE=0,			//��ʼ
	AI_FLAGS_TYPE_AI_PASSIVE=1,		//�����ģ�����bind��ͨ������server socket
	AI_FLAGS_TYPE_AI_CANONNAME=2,	//���ڷ��������Ĺ淶����
	AI_FLAGS_TYPE_AI_NUMERICHOST=4,	//��ַΪ���ִ�
	AI_FLAGS_TYPE_END=8,			//����ai_flagsֵ��[0,8);
};

// ���ϱ���ʾ��ai_flags��ֵ�ķ�ΧΪ0~7��ȡ���ڳ����������3����־λ����������ai_flagsΪ��AI_PASSIVE | AI_CANONNAME����ai_flagsֵ��Ϊ3�����������ĺ���ֱ�Ϊ��
// (1)AI_PASSIVE���˱�־��λʱ����ʾ�����߽���bind()����������ʹ�÷��صĵ�ַ�ṹ�����˱�־����λʱ����ʾ����connect()����������ʹ�á�
// ���ڵ���λNULL���Ҵ˱�־��λ���򷵻صĵ�ַ����ͨ���ַ��
// ����ڵ���NULL���Ҵ˱�־����λ���򷵻صĵ�ַ���ǻػ���ַ��
// (2)AI_CANNONAME���˱�־��λʱ���ں��������صĵ�һ��addrinfo�ṹ�е�ai_cannoname��Ա�У�Ӧ�ð���һ���Կ��ַ���β���ַ������ַ����������ǽڵ�������������
// (3)AI_NUMERICHOST���˱�־��λʱ���˱�־��ʾ�����еĽڵ���������һ�����ֵ�ַ�ַ�����



NAMESPACE_END(NET_SYS)


//�������
enum NET_IO_ERR_TYPE
{
	NET_IO_ERR_TYPE_NONE = 0,		//�޴���
	NET_IO_ERR_TYPE_PACKET_OVER_MAX = 1,			//��������������� MAX_BUFFER_LEN
	NET_IO_ERR_TYPE_MEMORY_NOT_ENOUGH = 2,		//�ڴ治��
	NET_IO_ERR_TYPE_POST_SEND = 3,				//Ͷ��sendʧ��
	NET_IO_ERR_TYPE_SOCKET_NOT_EXIST = 4,		//socket������
	NET_IO_ERR_TYPE_OTHER,						//��������
};

//threadpost����
enum POST_OBJ_TYPE
{
	POST_OBJ_TYPE_NONE = 0,
	POST_OBJ_TYPE_MAIN_THREAD,				//ҵ�����߼��߳�
	POST_OBJ_TYPE_NETWORK_SENDER,			//���緢��ģ��
	POST_OBJ_TYPE_END,
};

#endif

