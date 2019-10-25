/*!
 * MIT License
 *
 * Copyright (c) 2019 ericyonng<120453674@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file  : TestServer.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/10
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "TestSuit/TestSuit/TestInst/TestServer.h"

FS_NAMESPACE_BEGIN
// 
// class User
// {
// public:
//     User(UInt64 sessionId, IFS_MsgDispatcher *dispatcher)
//         :_sessionId(sessionId)
//         ,_recvMsgId(1)
//         ,_sendMsgId(1)
//         ,_dispatcher(dispatcher)
//     {
//     }
//     ~User()
//     {
//     }
//     
//     void SendData(IFS_Session *session, NetMsg_DataHeader *msgData)
//     {
//         session->Send(msgData);
//     }
// 
//     UInt64 _sessionId;
//     // ����server�����յ�����ϢID�Ƿ����� ÿ�յ�һ���ͻ�����Ϣ������1�Ա���ͻ��˵�msgidУ�飬��ƥ���򱨴���˵�������ȣ�
//     Int32 _recvMsgId = 1;
//     // ���Խ��շ��߼���
//     // ����client�����յ�����ϢID�Ƿ����� ÿ����һ����Ϣ������1�Ա���ͻ��˵�sendmsgidУ�飬��ƥ����ͻ��˱���˵�������ȣ�
//     Int32 _sendMsgId = 1;
//     IFS_MsgDispatcher *_dispatcher;
// };
// 
// class MyLogic : public IFS_BusinessLogic
// {
// public:
//     std::map<UInt64, User *> _users;
// 
// public:
//     MyLogic() {}
//     virtual ~MyLogic() 
//     {
//         STLUtil::DelMapContainer(_users);
//     }
//     virtual void Release()
//     {
//         delete this;
//     }
// 
// public:
//     User *GetUser(UInt64 sessionId)
//     {
//         auto iterUser = _users.find(sessionId);
//         if(iterUser == _users.end())
//             return NULL;
// 
//         return iterUser->second;
//     }
// 
//     void RemoveUser(UInt64 sessionId)
//     {
//         auto iterUser = _users.find(sessionId);
//         if(iterUser == _users.end())
//             return;
// 
//         Fs_SafeFree(iterUser->second);
//         _users.erase(iterUser);
//     }
// 
//     User *NewUser(UInt64 sessionId)
//     {
//         auto user = new User(sessionId, _dispatcher);
//         _users.insert(std::make_pair(sessionId, user));
//         return user;
//     }
// 
//     virtual Int32 Start()
//     {
//         return StatusDefs::Success;
//     }
// 
//     void BeforeClose()
//     {
//         STLUtil::DelMapContainer(_users);
//         g_Log->sys<MyLogic>(_LOGFMT_("MyLogic before closed remove all users"));
//     }
// 
//     virtual void Close()
//     {
//          g_Log->sys<MyLogic>(_LOGFMT_("MyLogic closed"));
//     }
// 
//     virtual void OnMsgDispatch(fs::IFS_Session *session, NetMsg_DataHeader *msgData)
//     {
//         auto sessionId = session->GetSessionId();
//         auto user = GetUser(sessionId);
//         if(!user)
//             user = NewUser(sessionId);
// 
//         // g_Log->any<MyLogic>("sessionid[%llu] handle a msg", sessionId);
//         switch(msgData->_cmd)
//         {
//             case fs::ProtocolCmd::LoginReq:
//             {
//                 fs::LoginReq *login = static_cast<fs::LoginReq *>(msgData);
// 
//                 //     g_Log->any<MyLogic>("sessionid[%llu] login username[%s], pwd[%s] msgId[%d] user recvmsgid"
//                 //                         , sessionId, login->_userName, login->_pwd, login->_msgId, user->_recvMsgId);
//                      // �����ϢID
//                 if(login->_msgId != user->_recvMsgId)
//                 {//��ǰ��ϢID�ͱ�������Ϣ������ƥ��
//                     g_Log->e<MyLogic>(_LOGFMT_("OnMsgDispatch sessionId<%llu> msgID<%d> _nRecvMsgID<%d> diff<%d>")
//                                       , sessionId, login->_msgId
//                                       , user->_recvMsgId, login->_msgId - user->_recvMsgId);
//                 }
// 
//                 // ���ذ�
//                 ++user->_recvMsgId;
//                 fs::LoginRes ret;
//                 ret._msgId = user->_sendMsgId;
//                 user->SendData(session, &ret);
//                 ++user->_sendMsgId;
//                 return;
//             }//���� ��Ϣ---���� ����   ������ ���ݻ�����  ������ 
//             break;
//             case fs::ProtocolCmd::LogoutReq:
//             {
//                 fs::FS_MsgReadStream r(msgData);
//                 // ��ȡ��Ϣ����
//                 r.ReadInt16();
//                 // ��ȡ��Ϣ����
//                 r.GetNetMsgCmd();
//                 auto n1 = r.ReadInt8();
//                 auto n2 = r.ReadInt16();
//                 auto n3 = r.ReadInt32();
//                 auto n4 = r.ReadFloat();
//                 auto n5 = r.ReadDouble();
//                 uint32_t n = 0;
//                 r.ReadWithoutOffsetPos(n);
//                 char name[32] = {};
//                 auto n6 = r.ReadArray(name, 32);
//                 char pw[32] = {};
//                 auto n7 = r.ReadArray(pw, 32);
//                 int ata[10] = {};
//                 auto n8 = r.ReadArray(ata, 10);
//                 ///
//                 fs::FS_MsgWriteStream s(128);
//                 s.SetNetMsgCmd(fs::ProtocolCmd::LogoutNty);
//                 s.WriteInt8(n1);
//                 s.WriteInt16(n2);
//                 s.WriteInt32(n3);
//                 s.WriteFloat(n4);
//                 s.WriteDouble(n5);
//                 s.WriteArray(name, n6);
//                 s.WriteArray(pw, n7);
//                 s.WriteArray(ata, n8);
//                 s.Finish();
// 
//                 // TODO:��Ҫ֧��������
// //                 _dispatcher->SendData(sessionId, )
// //                 client->SendData(*s.GetDataAddr(), s.GetWrLength());
//                 //                 g_Log->i<EasyFSServer>(_LOGFMT_("socket<%d> logout")
//                 //                                        , static_cast<Int32>(client->GetSocket()));
//                 return;
//             }
//             break;
//             case fs::ProtocolCmd::CheckHeartReq:
//             {
//                 fs::CheckHeartRes ret;
//                 _dispatcher->SendData(sessionId, &ret);
//                 //g_Log->any("socket<%d> CheckHeartReq", static_cast<Int32>(client->GetSocket()));
//                 return;
//             }
//             default:
//             {
//                 g_Log->w<MyLogic>(_LOGFMT_("recv <sessionId=%llu> undefine msgType,dataLen��%hu")
//                                   , sessionId, msgData->_packetLength);
//             }
//             break;
//         }
// 
//         return;
//     }
//     virtual void OnMsgDispatch(UInt64 sessionId, NetMsg_DataHeader *msgData)
//     {
//        
//     }
//     virtual void OnSessionDisconnected(UInt64 sessionId)
//     {
//         RemoveUser(sessionId);
//          // g_Log->any<MyLogic>("sessionid[%llu] Disconnected", sessionId);
//     }
// 
// 
// };
// 
// class EasyFSServer : public fs::FS_MainIocpServer
// {
//     OBJ_POOL_CREATE_DEF(EasyFSServer);
// public:
//     EasyFSServer()
//     {
//         _isCheckMsgID = true;
//         _bSendBack = true;
//         _bSendFull = true;
//     }
// 
//     // FS_Server 4 ����̴߳��� ����ȫ
//     // ���ֻ����1��cellServer���ǰ�ȫ��
//     virtual void OnNetJoin(fs::FS_Client * client)
//     {
//         fs::FS_TcpServer::OnNetJoin(client);
//         Int32 joinedCnt = _clientJoinedCnt;
//        // g_Log->net<EasyFSServer>("OnNetJoin joinedcnt[%d]", joinedCnt);
//         // g_Log->any("client<%d> joined curJoinedCnt[%d]", (Int32)(client->GetSocket()), joinedCnt);
// //         g_Log->i<EasyFSServer>(_LOGFMT_("socket<%d> joined, client joined cnt<%d>")
// //                                , static_cast<Int32>(client->GetSocket()), joinedCnt);
//     }
//     //cellServer 4 ����̴߳��� ����ȫ
//     //���ֻ����1��cellServer���ǰ�ȫ��
//     virtual void OnNetLeave(fs::FS_Client *client)
//     {
//         fs::FS_TcpServer::OnNetLeave(client);
//         // Int32 joinedCnt = _clientJoinedCnt;
//         // g_Log->any("client<%d> leave curJoinedCnt[%d]", (Int32)(client->GetSocket()), joinedCnt);
// 
// //         g_Log->i<EasyFSServer>(_LOGFMT_("socket<%d> leave, client joined cnt<%d>")
// //                                , static_cast<Int32>(client->GetSocket()), joinedCnt);
//     }
//     //cellServer 4 ����̴߳��� ����ȫ
//     //���ֻ����1��cellServer���ǰ�ȫ��
//     virtual Int32 OnNetMsg(fs::FS_Server *server, fs::FS_Client *client, fs::NetMsg_DataHeader *header)
//     {
//         fs::FS_TcpServer::OnNetMsg(server, client, header);
//         // Int32 recvMsgCnt = _recvMsgCount;
//         // g_Log->any("curRecv[%d]", recvMsgCnt);
//         switch(header->_cmd)
//         {
//             case fs::ProtocolCmd::LoginReq:
//             {
//                 fs::LoginReq *login = static_cast<fs::LoginReq *>(header);
// 
//                 // �����ϢID
//                 if(_isCheckMsgID)
//                 {
//                     if(login->_msgId != client->_recvMsgId)
//                     {//��ǰ��ϢID�ͱ�������Ϣ������ƥ��
//                         g_Log->e<EasyFSServer>(_LOGFMT_("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d> diff<%d>")
//                                                , client->GetSocket(), login->_msgId
//                                                , client->_recvMsgId, login->_msgId - client->_recvMsgId);
//                     }
// 
//                     ++client->_recvMsgId;
//                 }
// 
// //                 g_Log->net<EasyFSServer>("<Recv>socket<%d> loginReq, userName[%s] pwd[%s] msgId[%d] "
// //                                         , static_cast<Int32>(client->GetSocket()), login->_userName, login->_pwd, login->_msgId);
//                 // ��¼�߼�
// 
//                 // ......
//                 // ��Ӧ��Ϣ
//                 if(_bSendBack)
//                 {
//                     fs::LoginRes ret;
//                     ret._msgId = client->_sendMsgId;
// //                     g_Log->net<EasyFSServer>("<Send>socket<%d> LoginRes, _result[%d] msgId[%d] "
// //                                              , static_cast<Int32>(client->GetSocket()), ret._result, ret._msgId);
//                     client->SendData(&ret);
//                     ++client->_sendMsgId;
// 
// //                     if(SOCKET_ERROR == client->SendData(&ret))
// //                     {
// //                         // ���ͻ��������ˣ���Ϣû����ȥ,Ŀǰֱ��������
// //                         // �ͻ�����Ϣ̫�࣬��Ҫ����Ӧ�Բ���
// //                         // �������ӣ�ҵ��ͻ��˲�������ô����Ϣ
// //                         // ģ�Ⲣ������ʱ�Ƿ���Ƶ�ʹ���
// //                         if(_bSendFull)
// //                             g_Log->w<EasyFSServer>(_LOGFMT_("<Socket=%d> Send Full"), client->GetSocket());
// //                     }
// //                     else {
// //                     }
// 
//                     // g_Log->net<EasyFSServer>("<Socket=%d> send login res", client->GetSocket());
//                 }
// 
//                 return StatusDefs::Success;
//                 //CELLLog_Info("recv <Socket=%d> msgType��CMD_LOGIN, dataLen��%d,userName=%s PassWord=%s", cSock, login->dataLength, login->userName, login->PassWord);
//             }//���� ��Ϣ---���� ����   ������ ���ݻ�����  ������ 
//             break;
//             case fs::ProtocolCmd::LogoutReq:
//             {
//                 fs::FS_MsgReadStream r(header);
//                 // ��ȡ��Ϣ����
//                 r.ReadInt16();
//                 // ��ȡ��Ϣ����
//                 r.GetNetMsgCmd();
//                 auto n1 = r.ReadInt8();
//                 auto n2 = r.ReadInt16();
//                 auto n3 = r.ReadInt32();
//                 auto n4 = r.ReadFloat();
//                 auto n5 = r.ReadDouble();
//                 uint32_t n = 0;
//                 r.ReadWithoutOffsetPos(n);
//                 char name[32] = {};
//                 auto n6 = r.ReadArray(name, 32);
//                 char pw[32] = {};
//                 auto n7 = r.ReadArray(pw, 32);
//                 int ata[10] = {};
//                 auto n8 = r.ReadArray(ata, 10);
//                 ///
//                 fs::FS_MsgWriteStream s(128);
//                 s.SetNetMsgCmd(fs::ProtocolCmd::LogoutNty);
//                 s.WriteInt8(n1);
//                 s.WriteInt16(n2);
//                 s.WriteInt32(n3);
//                 s.WriteFloat(n4);
//                 s.WriteDouble(n5);
//                 s.WriteArray(name, n6);
//                 s.WriteArray(pw, n7);
//                 s.WriteArray(ata, n8);
//                 s.Finish();
//                 client->SendData(*s.GetDataAddr(), s.GetWrLength());
// //                 g_Log->i<EasyFSServer>(_LOGFMT_("socket<%d> logout")
// //                                        , static_cast<Int32>(client->GetSocket()));
//                 return StatusDefs::Success;
//             }
//             break;
//             case fs::ProtocolCmd::CheckHeartReq:
//             {
//                 fs::CheckHeartRes ret;
//                 client->SendData(&ret);
//                 //g_Log->any("socket<%d> CheckHeartReq", static_cast<Int32>(client->GetSocket()));
//                 return StatusDefs::Success;
//             }
//             default:
//             {
//                 g_Log->w<EasyFSServer>(_LOGFMT_("recv <socket=%d> undefine msgType,dataLen��%u")
//                                        , static_cast<Int32>(client->GetSocket()), header->_packetLength);
//             }
//             break;
//         }
// 
//         return StatusDefs::Unknown;
//     }
// 
// private:
//     //�Զ����־ �յ���Ϣ�󽫷���Ӧ����Ϣ
//     bool _bSendBack;
//     //�Զ����־ �Ƿ���ʾ�����ͻ�������д��
//     bool _bSendFull;
//     //�Ƿ�����յ�����ϢID�Ƿ�����
//     bool _isCheckMsgID;
// };

// OBJ_POOL_CREATE_DEF_IMPL(EasyFSServer, 10)

FS_NAMESPACE_END


static union {
    char c[4];
    unsigned long l;
} endian_test = {{'l', '?', '?', 'b'}};

void TestServer::Run()
{
    endian_test;
    if((char)(endian_test.l) == 'b')
    {
        printf("big endian\n");
    }
    else
    {
        printf("little endian\n");
    }
    //fs::MyLogic *newLogic = new fs::MyLogic;
    fs::FS_ServerCore *serverCore = new fs::FS_ServerCore();
    auto st = serverCore->Start(NULL);
    if(st == StatusDefs::Success)
    {
        getchar();
    }
    else
    {
        g_Log->e<TestServer>(_LOGFMT_("Start server fail st[%d] server will close now. please check! "));
    }
    //serverCore->Wait();
    serverCore->Close();
    Fs_SafeFree(serverCore);

    std::cout << "free server core" << std::endl;
    getchar();
}
