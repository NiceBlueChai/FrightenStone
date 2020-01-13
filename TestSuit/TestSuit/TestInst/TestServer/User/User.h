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

class IUserMgr;

class User : public fs::IUserSys
{
public:
    User(IUserMgr *userMgr, UInt64 sessionId, UInt64 userId);
    ~User();

public:
    Int32 Login(fs::LoginData *loginData);
    Int32 Logout();
    void CheckHeart();
    void SucRecvMsg();

public:
    // 取得指定用户系统(泛型方法)
    template <typename UserSys>
    UserSys *GetSys();
    template <typename UserSys>
    const UserSys *GetSys() const;

    // 取得指定用户系统(非泛型方法)
    fs::IUserSys *GetSys(const fs::FS_String &sysName);
    const fs::IUserSys *GetSys(const fs::FS_String &sysName) const;

    // 取得用户系统列表
    std::vector<fs::IUserSys *> &GetSyss();
    const std::vector<fs::IUserSys *> &GetSyss() const;

public:
    virtual UInt64 GetSessionId() const;
    virtual UInt64 GetUseId() const;
    Int32 GetRecvMsgId() const;
    Int32 GetSendMsgId() const;

    virtual void Close();
    // NetMsg_DataHeader内部会拷贝到缓冲区
    void SendData(fs::NetMsg_DataHeader *msgData);
    void OnDisconnect();
    void OnConnected();

private:
    void _AddSys(fs::IUserSys *sys);
    fs::IUserSys *_CreateSys(const fs::ILogicSysInfo *sysInfo);
    void _RemoveSyss();

    Int32 _Create();

private:
    friend class UserMgr;

    UInt64 _sessionId;
    UInt64 _userId;
    // 用于server检测接收到的消息ID是否连续 每收到一个客户端消息会自增1以便与客户端的msgid校验，不匹配则报错处理（说明丢包等）
    Int32 _recvMsgId = 1;
    // 测试接收发逻辑用
    // 用于client检测接收到的消息ID是否连续 每发送一个消息会自增1以便与客户端的sendmsgid校验，不匹配则客户端报错（说明丢包等）
    Int32 _sendMsgId = 1;

    IUserMgr *_userMgr;

    std::vector<fs::IUserSys *> _userSysList;                       // 用户系统列表
    std::map<fs::FS_String, fs::IUserSys *> _userSysNameRefSyss;    // 用户系统集;         key: 系统名, value: 系统
    std::map<fs::FS_String, fs::IUserSys *> _iuserSysNameRefSyss;   //; 用户系统接口集;    key: 系统对应的接口类名, value: 系统
};

#include "TestInst/TestServer/User/UserImpl.h"