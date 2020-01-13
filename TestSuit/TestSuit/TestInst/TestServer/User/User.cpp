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
 * @file  : User.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/1/8
 * @brief :
 */
#include "stdafx.h"
#include "TestInst/TestServer/User/User.h"
#include "TestInst/TestServer/User/IUserMgr.h"

User::User(IUserMgr *userMgr, UInt64 sessionId, UInt64 userId)
    :_sessionId(sessionId)
    , _userId(userId)
    , _recvMsgId(1)
    , _sendMsgId(1)
    ,_userMgr(userMgr)
{
}

User::~User()
{
    _RemoveSyss();
}

Int32 User::Login(fs::LoginData *loginData)
{
    if(loginData->_msgId != _recvMsgId)
    {// ��ǰ��ϢID�ͱ�������Ϣ������ƥ��
        g_Log->custom("OnMsgDispatch sessionId<%llu> msgID<%d> _nRecvMsgID<%d> diff<%d>"
                      , _sessionId, loginData->_msgId
                      , _recvMsgId, loginData->_msgId - _recvMsgId);
    }

    // ��ӡ
//     g_Log->custom("user[%s]-pwd[%s]-msgid[%d] login suc", 
//                   loginData->_userName
//                   , loginData->_pwd
//                   , loginData->_msgId);

    return StatusDefs::Success;
}

Int32 User::Logout()
{
    g_Log->custom("userId[%llu] sessionId[%llu] logout", _userId, _sessionId);
    return StatusDefs::Success;
}

void User::CheckHeart()
{

}

fs::IUserSys *User::GetSys(const fs::FS_String &sysName)
{
    // ���Դӽӿ��ֵ���ȡ��
    auto iterIUser = _iuserSysNameRefSyss.find(sysName);
    if(iterIUser != _iuserSysNameRefSyss.end())
        return iterIUser->second;

    // ������ʵ���ֵ���ȡ��
    auto it = _userSysNameRefSyss.find(sysName);
    return it != _userSysNameRefSyss.end() ? it->second : NULL;
}

const fs::IUserSys * User::GetSys(const fs::FS_String &sysName) const
{
    // ���Դӽӿ��ֵ���ȡ��
    auto iterIUser = _iuserSysNameRefSyss.find(sysName);
    if(iterIUser != _iuserSysNameRefSyss.end())
        return iterIUser->second;

    // ������ʵ���ֵ���ȡ��
    auto it = _userSysNameRefSyss.find(sysName);
    return it != _userSysNameRefSyss.end() ? it->second : NULL;
}

void User::Close()
{
    // TODO:��һ���Ͽ�session����Ϣ��transfer ͨ��dispatcher
    _dispatcher->CloseSession(_sessionId);
}

// NetMsg_DataHeader�ڲ��´����������
void User::SendData(fs::NetMsg_DataHeader *msgData)
{
    _dispatcher->SendData(_sessionId, msgData);

    // Ϊ��У���Ƿ���ڶ���,У����Ϣ��������
    ++_sendMsgId;
}

void User::OnDisconnect()
{
    g_Log->net<User>(" session id[%llu] user disconnect", _sessionId);
    _RemoveSyss();
}

void User::OnConnected()
{

}

void User::_AddSys(fs::IUserSys *sys)
{
    _userSysList.push_back(sys);
    _userSysNameRefSyss.insert(std::make_pair(sys->GetSysName(), sys));
    _iuserSysNameRefSyss.insert(std::make_pair(fs::FS_String("I") + sys->GetSysName(), sys));
}

fs::IUserSys * User::_CreateSys(const fs::ILogicSysInfo *sysInfo)
{
    auto sysFactory = _userMgr->GetSysFactory(sysInfo->GetSysName());

    auto sys = static_cast<IUserSys *>(sysFactory->CreateSys());
    sys->SetSysInfo(sysInfo);

    return sys;
}

void User::_RemoveSyss()
{
    fs::STLUtil::DelVectorContainer(_userSysList);
    _userSysNameRefSyss.clear();
    _iuserSysNameRefSyss.clear();
}

Int32 User::_Create()
{
    // ������ʼʱ��
    fs::Time beginTime, endTime;
    beginTime.FlushTime();

    auto session = _dispatcher->GetSession(_sessionId);
    g_Log->i<User>(_LOGFMT_("Create user, userId:%llu, sessionId:%llu, addrInfo:%s")
                   , session->GetAddr()->ToString().c_str());

    // ��������syss��������Create�������sys������������Create
    auto &sysInfos = _userMgr->GetSysInfosList();
    for(auto infoIt = sysInfos.begin(); infoIt != sysInfos.end(); infoIt++)
    {
        const auto *sysInfo = *infoIt;
        g_Log->i<User>(_LOGFMT_("Create userid[%llu] sessionId[%llu]. system: %s...")
                       , _userId, _sessionId, sysInfo->GetSysName().c_str());

        auto *sys = _CreateSys(sysInfo);
        Int32 createRet = sys->_Create(this);
        if(createRet != StatusDefs::Success)
        {
            g_Log->e<User>(_LOGFMT_("Failed to create userid[%llu] sessionid[%llu] system: %s, status: %d")
                           , _userId, _sessionId, sysInfo->GetSysName().c_str(), createRet);

            FS_Release(sys);
            return createRet;
        }

        // TODO:load����....

        _AddSys(sys);
    }

    // ��������system��AfterCreate
    for(auto sysIt = _userSysList.begin(); sysIt != _userSysList.end(); ++sysIt)
    {
        g_Log->i<User>(_LOGFMT_("After create userid[%llu] sessionId[%llu] system: %s...")
                       , _userId, _sessionId, (*sysIt)->GetSysInfo()->GetSysName().c_str());
        (*sysIt)->_AfterCreate();
    }

    // ��������ʱ��
    endTime.FlushTime();

    auto usedTime = endTime - beginTime;
    g_Log->i<User>(_LOGFMT_("sessionId[%llu] userId[%llu] created, usedTime[%lld] microseconds")
                   , _sessionId, _userId, usedTime.GetTotalMicroSeconds());

    return StatusDefs::Success;
}
