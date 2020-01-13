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
 * @file  : IUserMgr.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/01/13
 * @brief :
 */

#pragma once

#include "FrightenStone/exportbase.h"

class IUserMgr : public fs::IBaseLogicSysMgr
{
public:
    // ע��Userϵͳ(ģ�巽��)
    template <typename IUserSysFactory>
    Int32 RegisterUserSys();
    // ע��Userϵͳ
    virtual Int32 RegisterUserSys(fs::IFS_LogicSysFactory *sysFactory) = 0;

    // ȡ��Userϵͳ��Ϣ
    virtual const fs::ILogicSysInfo *GetSysInfo(const fs::FS_String &sysName) const = 0;
    // ȡ��Userϵͳ����
    virtual const fs::IFS_LogicSysFactory *GetSysFactory(const fs::FS_String &sysName) const = 0;
    // ȡ��Userϵͳ�����ֵ�
    virtual const std::map<fs::FS_String, fs::IFS_LogicSysFactory *> &GetSysFactoriesDict() const = 0;
    // ȡ��Userϵͳ��Ϣ�б�
    virtual const std::vector<fs::ILogicSysInfo *> &GetSysInfosList() const = 0;

    // �����û�
    virtual Int32 CreateUser(UInt64 sessionId) = 0;

    // ��ȡ�û�
    virtual User *GetUserBySessionId(UInt64 sessionId) = 0;
    // �Ƴ��û�
    virtual void RemoveUser(UInt64 sessionId) = 0;
};

#include "TestInst/TestServer/User/IUserMgrImpl.h"