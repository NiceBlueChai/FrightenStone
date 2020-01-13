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
 * @file  : User.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/1/8
 * @brief :
 */

#pragma once

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "protocols/protocol.h"

class User : public fs::IUserSys
{
public:
    User(UInt64 sessionId, UInt64 userId);
    ~User();

public:
    Int32 Login(fs::LoginData *loginData);
    Int32 Logout();
    void CheckHeart();
    void SucRecvMsg();

public:
    virtual UInt64 GetSessionId() const;
    virtual UInt64 GetUseId() const;
    Int32 GetRecvMsgId() const;
    Int32 GetSendMsgId() const;

    virtual void Close();
    // NetMsg_DataHeader�ڲ��´����������
    void SendData(fs::NetMsg_DataHeader *msgData);
    void OnDisconnect();
    void OnConnected();

private:
    void _OnSendTest(fs::FS_Timer *timer, const fs::Time &lastTimeoutTime, const fs::Time &curTimeWheelTime);

private:
    UInt64 _sessionId;
    UInt64 _userId;
    // ����server�����յ�����ϢID�Ƿ����� ÿ�յ�һ���ͻ�����Ϣ������1�Ա���ͻ��˵�msgidУ�飬��ƥ���򱨴���˵�������ȣ�
    Int32 _recvMsgId = 1;
    // ���Խ��շ��߼���
    // ����client�����յ�����ϢID�Ƿ����� ÿ����һ����Ϣ������1�Ա���ͻ��˵�sendmsgidУ�飬��ƥ����ͻ��˱���˵�������ȣ�
    Int32 _sendMsgId = 1;
    fs::IFS_MsgDispatcher *_dispatcher;
    fs::IFS_BusinessLogic *_logic;

    fs::FS_Timer *_sendTest;
    fs::LoginReq _testLogin;
};

#include "FS_MyClient/FS_MyClient/User/UserImpl.h"