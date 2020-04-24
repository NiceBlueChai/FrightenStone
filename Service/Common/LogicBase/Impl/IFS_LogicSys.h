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
 * @file  : IFS_LogicSys.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/1/8
 * @brief :
 */
#ifndef __Service_Common_LogicBase_Impl_IFS_LogicSys_H__
#define __Service_Common_LogicBase_Impl_IFS_LogicSys_H__

#pragma once

#include "FrightenStone/exportbase.h"

class ILogicSysInfo;

// ע��:�����ٹ���������ص�ҵ��������Ϊ��û��ʼ���������OnInitialize����ʼ��
class IFS_LogicSys
{
public:
    IFS_LogicSys();
    virtual ~IFS_LogicSys();

public:
    void BindDispatcher(fs::IFS_MsgDispatcher *dispatcher);
    // �ڸ���ϵͳ����ʱ��ִ�г�ʼ��������֤һ����ҵ���߳�ִ�У�global�����̵߳���ע��globalϵͳʱ��ִ�У�usersys���ڴ�����user����ʱ��ִ�У�
    virtual void OnInitialize() {}

public:
    const fs::FS_String &GetSysName() const;
    const ILogicSysInfo *GetSysInfo() const;
    void SetSysInfo(const ILogicSysInfo *sysInfo);

protected:
    fs::IFS_MsgDispatcher *_dispatcher;
    const ILogicSysInfo *_sysInfo;
};

#include "Service/Common/LogicBase/Impl/IFS_LogicSysImpl.h"

#endif
