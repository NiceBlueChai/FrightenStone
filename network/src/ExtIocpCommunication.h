#ifndef EXT_IOCP_COMMUNICATION_H_
#define EXT_IOCP_COMMUNICATION_H_

#pragma once

#pragma MEM_ALIGN_BEGIN(1)

NAMESPACE_BEGIN(EXTIOCP_NETDATA)

//�ⲿ��iocpͨѶЭ��
struct ExtIocpNetData
{
	SOCKET_IDD idSocket;					//socketid
	TRAN_PROTOCOL::TRAN_TYPE nTranType;		//Э������
	NET_DATA_SIZE nszLen;					//���ݳ���
	char rStream[0];						//����
};

NAMESPACE_END(EXTIOCP_NETDATA)

#pragma MEM_ALIGN_END(1)

#endif 

