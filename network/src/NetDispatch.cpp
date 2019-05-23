#include "NetDispatch.h"
#include "Message.hpp"
#include <Net/IocpServer.h>
#include <Net/connection.h>

CNetDispatch::CNetDispatch()
{
}

CNetDispatch::~CNetDispatch()
{

}

void CNetDispatch::Cleans()
{
}


void CNetDispatch::OnWrite(const Connection* conn, std::size_t bytes_transferred)
{
	CHECK(conn);
	//�����첽��ȡ�ͻ���
	ASSERTEX(IocpMgr.Lock());
	IocpMgr.AsyncRead(conn);
	ASSERTEX(IocpMgr.Unlock());
}

void CNetDispatch::OnRead(const Connection* conn, void* data, std::size_t size)
{
	CHECK(conn);

	auto variable_conn = const_cast<Connection*>(conn);

	MesageHeader *mh = nullptr;
	int msgPos = variable_conn->GetMsgPos();
	memmove(variable_conn->GetMsgBuffer() + msgPos, variable_conn->GetReadBuffer(), size);
	variable_conn->SetMsgPos(msgPos + size);

	LOGNET_FMT512("���յ����� %I64u", size);

	while (variable_conn->GetMsgPos() >= sizeof(MesageHeader))
	{
		mh = (MesageHeader*)variable_conn->GetMsgBuffer();
		IF_NOT(mh)
			break;

		//��������
		if (mh->lenth <= variable_conn->GetMsgPos() && (mh->lenth>=0))
		{
			//�ƶ�λ�� �۳���ǰ��������Ϣ����
			variable_conn->SetMsgPos(variable_conn->GetMsgPos() - mh->lenth);

			//������Ϣ
			ProcessMessage(mh, variable_conn);

			//����δ������Ϣ��ǰ��
			if(variable_conn->GetMsgPos()>0)
				memmove(variable_conn->GetMsgBuffer(), variable_conn->GetMsgBuffer() + mh->lenth, variable_conn->GetMsgPos());
			
		}
		else{
			ASSERTNET("OnRead ���ݽ������� mh->lenth > variable_conn->GetMsgPos() && (mh->lenth>=0)"," mh->lenth:" mh->lenth);
			break;
		}
	}

	ASSERTEX(IocpMgr.Lock());
	IocpMgr.AsyncRead(conn);
	ASSERTEX(IocpMgr.Unlock());

}

void CNetDispatch::OnClientDisconnected(const Connection& conn)
{
#if 0
	if (conn == NULL)
		int mm = 0;
	auto variable_conn = const_cast<Connection*>(conn);
	SOCKET clifd = variable_conn->GetSocket();
	fprintf(stderr, "client:%d �Ͽ�\n", clifd);
#endif
	ASSERTNET(IocpMgr.Lock());
	SOCKET  s = conn.GetSocket();
	IF_OK(IS_VALID_SOCKET_IDD(s))
		IocpMgr.DeductClientCount(s);
	LOGNET("�ͻ��˶Ͽ�", s);
	ASSERTNET(IocpMgr.Unlock());

}

void CNetDispatch::Init()
{
	//�󶨻ص�����
	IocpMgr.SetOnConnectedCallback(std::bind(&CNetDispatch::OnClientConnected, this, std::placeholders::_1));
	IocpMgr.SetOnDisconnectedCallback(std::bind(&CNetDispatch::OnClientDisconnected, this, std::placeholders::_1));
	IocpMgr.SetOnReadCallback(std::bind(&CNetDispatch::OnRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	IocpMgr.SetOnWriteCallback(std::bind(&CNetDispatch::OnWrite, this, std::placeholders::_1, std::placeholders::_2));

}

void CNetDispatch::OnClientConnected(const Connection* conn)
{
//test03 ���δ�ӡ��Ϣ
	//fprintf(stderr, "�¿ͻ�������\n");

//test05
	LOGNET("�ͻ�������.");
	IocpMgr.AddClientCount();
	ASSERTEX(IocpMgr.Lock());
	IocpMgr.AsyncRead(conn);
	ASSERTEX(IocpMgr.Unlock());
}


void CNetDispatch::ProcessMessage(MesageHeader* mh,Connection* conn)
{
	CHECK(conn&&mh);
	SOCKET sSocket = conn->GetSocket();

	MesageHeader *sendMsg = nullptr;
	switch (mh->type)
	{
	case T_Login:
	{
#if 1
			IocpMgr.AddClientCount();
		//if (_iocpServer._chrono_timer.GetSecInterval() > 1.0)
		//{
		//	fprintf(stderr, "client count<%d> msg count<%d>\n", _iocpServer._client_count, _iocpServer._msg_count);
		//	_iocpServer._chrono_timer.FlushTime();
		//	_iocpServer._msg_count = 0;
		//}
		//return;
#endif
		Login* lg = (Login*)mh;
		LOGNET_FMT1K("client socket<%d> T_Login name(%s),passwrod(%s) ���ݳ���:%d\n", sSocket, lg->name, lg->password, mh->lenth);
		sendMsg = new Login_Result();
		strcpy(((Login_Result *)sendMsg)->name, lg->name);
		auto pBusinessThread = IocpMgr.GetBusinessLogicMainThread();
		IF_OK(pBusinessThread)
		{
			POST_OBJ_TYPE_MAIN_THREAD;
			ASSERTNET(THREAD_TRANSFER::Post2Thread(pBusinessThread, POST_OBJ_TYPE_MAIN_THREAD, (U64)conn, mh, sizeof(Login)));
		}
		//IocpMgr.AsyncRead(conn);
	}
		break;
	case T_Logout:
	{
		Logout* lg = (Logout*)mh;
		LOGNET_FMT1K("client socket<%d> T_Logout name(%s),���ݳ���:%d\n", sSocket, lg->name, mh->lenth);
		sendMsg = new Logout_Result();
	}
		break;
	case T_ERROR:
		LOGNET_FMT1K("T_ERROR\n");
		break;
	default:
		fprintf(stderr, "δ֪��Ϣ\n");
		break;
	}
	if (sendMsg != nullptr)
	{
		ASSERTEX(IocpMgr.Lock());
		IocpMgr.AsyncWrite(conn, sendMsg, sendMsg->lenth);
		ASSERTEX(IocpMgr.Unlock());
	}		
}