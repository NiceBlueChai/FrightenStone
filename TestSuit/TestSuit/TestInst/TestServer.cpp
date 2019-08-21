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

class EasyFSServer : public fs::FS_MainIocpServer
{
    OBJ_POOL_CREATE_DEF(EasyFSServer);
public:
    EasyFSServer()
    {
        _isCheckMsgID = true;
        _bSendBack = true;
        _bSendFull = true;
    }

    // FS_Server 4 ����̴߳��� ����ȫ
    // ���ֻ����1��cellServer���ǰ�ȫ��
    virtual void OnNetJoin(fs::FS_Client * client)
    {
        fs::FS_TcpServer::OnNetJoin(client);
        Int32 joinedCnt = _clientJoinedCnt;
       // g_Log->net<EasyFSServer>("OnNetJoin joinedcnt[%d]", joinedCnt);
        // g_Log->any("client<%d> joined curJoinedCnt[%d]", (Int32)(client->GetSocket()), joinedCnt);
//         g_Log->i<EasyFSServer>(_LOGFMT_("socket<%d> joined, client joined cnt<%d>")
//                                , static_cast<Int32>(client->GetSocket()), joinedCnt);
    }
    //cellServer 4 ����̴߳��� ����ȫ
    //���ֻ����1��cellServer���ǰ�ȫ��
    virtual void OnNetLeave(fs::FS_Client *client)
    {
        fs::FS_TcpServer::OnNetLeave(client);
        // Int32 joinedCnt = _clientJoinedCnt;
        // g_Log->any("client<%d> leave curJoinedCnt[%d]", (Int32)(client->GetSocket()), joinedCnt);

//         g_Log->i<EasyFSServer>(_LOGFMT_("socket<%d> leave, client joined cnt<%d>")
//                                , static_cast<Int32>(client->GetSocket()), joinedCnt);
    }
    //cellServer 4 ����̴߳��� ����ȫ
    //���ֻ����1��cellServer���ǰ�ȫ��
    virtual Int32 OnNetMsg(fs::FS_Server *server, fs::FS_Client *client, fs::NetMsg_DataHeader *header)
    {
        fs::FS_TcpServer::OnNetMsg(server, client, header);
        // Int32 recvMsgCnt = _recvMsgCount;
        // g_Log->any("curRecv[%d]", recvMsgCnt);
        switch(header->_cmd)
        {
            case fs::ProtocolCmd::LoginReq:
            {
                fs::LoginReq *login = static_cast<fs::LoginReq *>(header);

                // �����ϢID
                if(_isCheckMsgID)
                {
                    if(login->_msgId != client->_recvMsgId)
                    {//��ǰ��ϢID�ͱ�������Ϣ������ƥ��
                        g_Log->e<EasyFSServer>(_LOGFMT_("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d> diff<%d>")
                                               , client->GetSocket(), login->_msgId
                                               , client->_recvMsgId, login->_msgId - client->_recvMsgId);
                    }

                    ++client->_recvMsgId;
                }

//                 g_Log->i<EasyFSServer>(_LOGFMT_("socket<%d> loginReq, userName[%s] pwd[%s] ")
//                                        , static_cast<Int32>(client->GetSocket()), login->_userName, login->_pwd);
                // ��¼�߼�
                // ......
                // ��Ӧ��Ϣ
                if(_bSendBack)
                {
                    fs::LoginRes ret;
                    ret._msgId = client->_sendMsgId;
                    if(SOCKET_ERROR == client->SendData(&ret))
                    {
                        // ���ͻ��������ˣ���Ϣû����ȥ,Ŀǰֱ��������
                        // �ͻ�����Ϣ̫�࣬��Ҫ����Ӧ�Բ���
                        // �������ӣ�ҵ��ͻ��˲�������ô����Ϣ
                        // ģ�Ⲣ������ʱ�Ƿ���Ƶ�ʹ���
                        if(_bSendFull)
                            g_Log->w<EasyFSServer>(_LOGFMT_("<Socket=%d> Send Full"), client->GetSocket());
                    }
                    else {
                        ++client->_sendMsgId;
                    }
                    // g_Log->net<EasyFSServer>("<Socket=%d> send login res", client->GetSocket());
                }

                return StatusDefs::Success;
                //CELLLog_Info("recv <Socket=%d> msgType��CMD_LOGIN, dataLen��%d,userName=%s PassWord=%s", cSock, login->dataLength, login->userName, login->PassWord);
            }//���� ��Ϣ---���� ����   ������ ���ݻ�����  ������ 
            break;
            case fs::ProtocolCmd::LogoutReq:
            {
                fs::FS_MsgReadStream r(header);
                // ��ȡ��Ϣ����
                r.ReadInt16();
                // ��ȡ��Ϣ����
                r.GetNetMsgCmd();
                auto n1 = r.ReadInt8();
                auto n2 = r.ReadInt16();
                auto n3 = r.ReadInt32();
                auto n4 = r.ReadFloat();
                auto n5 = r.ReadDouble();
                uint32_t n = 0;
                r.ReadWithoutOffsetPos(n);
                char name[32] = {};
                auto n6 = r.ReadArray(name, 32);
                char pw[32] = {};
                auto n7 = r.ReadArray(pw, 32);
                int ata[10] = {};
                auto n8 = r.ReadArray(ata, 10);
                ///
                fs::FS_MsgWriteStream s(128);
                s.SetNetMsgCmd(fs::ProtocolCmd::LogoutNty);
                s.WriteInt8(n1);
                s.WriteInt16(n2);
                s.WriteInt32(n3);
                s.WriteFloat(n4);
                s.WriteDouble(n5);
                s.WriteArray(name, n6);
                s.WriteArray(pw, n7);
                s.WriteArray(ata, n8);
                s.Finish();
                client->SendData(*s.GetDataAddr(), s.GetWrLength());
//                 g_Log->i<EasyFSServer>(_LOGFMT_("socket<%d> logout")
//                                        , static_cast<Int32>(client->GetSocket()));
                return StatusDefs::Success;
            }
            break;
            case fs::ProtocolCmd::CheckHeartReq:
            {
                fs::CheckHeartRes ret;
                client->SendData(&ret);
                //g_Log->any("socket<%d> CheckHeartReq", static_cast<Int32>(client->GetSocket()));
                return StatusDefs::Success;
            }
            default:
            {
                g_Log->w<EasyFSServer>(_LOGFMT_("recv <socket=%d> undefine msgType,dataLen��%u")
                                       , static_cast<Int32>(client->GetSocket()), header->_packetLength);
            }
            break;
        }

        return StatusDefs::Unknown;
    }

private:
    //�Զ����־ �յ���Ϣ�󽫷���Ӧ����Ϣ
    bool _bSendBack;
    //�Զ����־ �Ƿ���ʾ�����ͻ�������д��
    bool _bSendFull;
    //�Ƿ�����յ�����ϢID�Ƿ�����
    bool _isCheckMsgID;
};

OBJ_POOL_CREATE_DEF_IMPL(EasyFSServer, 10)

FS_NAMESPACE_END

void TestServer::Run()
{
    g_Log->InitModule("TestServer");
    fs::CrashHandleUtil::InitCrashHandleParams();
    fs::EasyFSServer easyServer;
    easyServer.InitSocket();
    easyServer.Bind(NULL, 4567);
    easyServer.Listen();
    easyServer.Start(8);
    while(1)
    {
        Sleep(1000);
    }
    getchar();
}
