#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include "Workers.h"
#include "overlapped.h"
#include "SocketExFnsHunter.h"
#include <BaseCode/TaskBase.h>
#include<Net/IocpServer.h>

Workers::Workers(std::atomic<I32>& rCurThreadNum)
	:m_nCurThreadNum(rCurThreadNum)
{

}

I32 Workers::Run()
{
	DWORD bytes_transferred;
	ULONG_PTR completion_key;
	DWORD Flags = 0;
	Overlapped* overlapped = NULL;

	LOGSYSFMT_C512("IOCP �������߳� �߳�id[%d] ����. ��ǰ�߳���[%d]", std::this_thread::get_id(), ++m_nCurThreadNum);
	
	DOTRY_B
	{
		while (!IocpMgr.IsDestroy())
		{
			BOOL bRet = GetQueuedCompletionStatus(IocpMgr.GetIocpPort(),
				&bytes_transferred,
				&completion_key,
				reinterpret_cast<LPOVERLAPPED*>(&overlapped),
				INFINITE);

			if (!completion_key || !overlapped)
				continue;

			// 		// ����յ������˳���־����ֱ���˳�
			// 		if (EXIT_CODE == (DWORD)completion_key && !overlapped)
			// 			break;

			// ����յ������˳���־����ֱ���˳�
			if (EXIT_CODE == (DWORD)completion_key)
				break;

			if (bRet == FALSE)
			{

				//�ͷ���ֱ���˳���û�е���closesocket�����ر�����
				auto nErr = GetLastError();
				if (nErr != WAIT_TIMEOUT && ERROR_NETNAME_DELETED != nErr)
					continue;

				if (!overlapped)
					continue;

				if (overlapped->connection == NULL)
					continue;

				//�ͻ��˶Ͽ�,���ûص�����
				if (IocpMgr.OnDisconnected)
					IocpMgr.OnDisconnected(*overlapped->connection);	//�Ѿ��ͷ�

				IF_NOT(overlapped->type != Overlapped::Accept_type)
				{
					IF_NOT(IocpMgr.Accept())
						continue;
				}

				continue;
			}

			if ((bytes_transferred == 0) &&
				(overlapped->type == Overlapped::Type::Read_type ||
					overlapped->type == Overlapped::Type::Write_type))
			{
				//�Ͽ�
				if (overlapped->connection != NULL)
				{
					//�ͻ��˶Ͽ�
					if (IocpMgr.OnDisconnected)
						IocpMgr.OnDisconnected(*overlapped->connection); //�Ѿ��ͷ�

																		 //overlapped = NULL;
																		 //SAFE_DELETE(overlapped->connection);

					IF_NOT(overlapped->type != Overlapped::Accept_type)
					{
						IF_NOT(IocpMgr.Accept())
							continue;
					}
				}

				continue;
			}
			else
			{
				switch (overlapped->type)
				{
				case Overlapped::Accept_type:
					{
						//acceptex����˲������������ǻ�Ҫ�����������ɶ˿ڡ�
						//�����Ȳ����죬�Ⱥ������ǻ�����Ż�����
						//����Ҳ������Ӷ��accept����ɶ˿�
						IF_NOT(IocpMgr.Accept())
							continue;

						//�¿ͻ�������
						if (IocpMgr.OnConnected)
							IocpMgr.OnConnected(overlapped->connection);
					}
					break;
				case Overlapped::Type::Read_type:
					{
						// �첽�����
						if (IocpMgr.OnRead)
							IocpMgr.OnRead(overlapped->connection, overlapped->connection->GetReadBuffer(), bytes_transferred);
					}
					break;
				case Overlapped::Type::Write_type:
					{
						auto conn = overlapped->connection;
						IF_NOT(conn)
							continue;
						conn->SetSentBytes(conn->GetSentBytes() + bytes_transferred);

						//�ж��Ƿ�ֻ������һ����
						if (conn->GetSentBytes() < conn->GetTotalBytes())
						{
							//��ʣ�ಿ���ٷ���
							overlapped->wsa_buf.len = conn->GetTotalBytes() - conn->GetSentBytes();
							overlapped->wsa_buf.buf = reinterpret_cast<CHAR*>(conn->GetWriteBuffer()) + conn->GetSentBytes();

							auto send_result = WSASend(conn->GetSocket(),
								&overlapped->wsa_buf, 1, &bytes_transferred,
								0, reinterpret_cast<LPWSAOVERLAPPED>(overlapped),
								NULL);

							ASSERTNET((send_result == NULL || (send_result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING)));
						}
						else
						{
							//�������
							//AsyncRead(overlapped->connection);
							LOGNET("�������.");
							if (IocpMgr.OnWrite)
								IocpMgr.OnWrite(overlapped->connection, bytes_transferred);
						}
					}
					break;
				default:
					LOGNET_FMT512("����� overlapped->type[%d].", overlapped->type);
					ASSERTNET(false);
					break;
				}
			}
		}

	}
	DOCATCH_E1("Workers::Run()")
	{
		LOGCRASH("Network Module Workers::Run()");
	}
	
	//�߳��˳�
	LOGSYSFMT_C512("IOCP �������߳� �߳�id[%d] �˳�. ��ǰ�߳���[%d]", std::this_thread::get_id(), --m_nCurThreadNum);

	return 0;
}

Workers::~Workers()
{
}

void Workers::Cleans()
{
}

