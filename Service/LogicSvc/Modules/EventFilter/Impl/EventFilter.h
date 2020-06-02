/*!
 * MIT License
 *
 * Copyright (c) 2019 Eric Yonng<120453674@qq.com>
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
 * @file  : EventFilter.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/01/16
 * @brief : �¼�������,�¼�ת����(������ȫ��,��ʵҲ�Ǽ���ĳһ�¼�,����һ�¼������ĳ������ת������һ���¼��������н����ɷ�)
 */
#ifndef __Service_LogicSvc_Modules_EventFilter_Impl_EventFilter_H__
#define __Service_LogicSvc_Modules_EventFilter_Impl_EventFilter_H__

#pragma once

#include "Service/LogicSvc/Common.h"

class EventFilter : public IFS_EventFilter
{
public:
    EventFilter();
    ~EventFilter();

public:
    virtual void Register(fs::EventManager &evMgr);
    virtual void UnRegister(fs::EventManager &evMgr);

private:
    void _OnSessionWillConnected(fs::FS_Event *ev);
    void _OnSessionConnecting(fs::FS_Event *ev);
    void _OnSessionConnectFail(fs::FS_Event *ev);
    void _OnSessionConnected(fs::FS_Event *ev);
    void _OnAfterSessionConnected(fs::FS_Event *ev);
    void _OnSessionWillDisconnect(fs::FS_Event *ev);
    void _OnSessionDisconnect(fs::FS_Event *ev);
    void _OnAfterSessionDisconnect(fs::FS_Event *ev);
    void _OnLogicBeforeMsgDispatch(fs::FS_Event *ev);
    void _OnUserLogin(fs::FS_Event *ev);
    void _OnUserCreated(fs::FS_Event *ev);
    void _OnUserLoginRes(fs::FS_Event *ev);
    void _OnUserWillClose(fs::FS_Event *ev);
    void _OnUserClose(fs::FS_Event *ev);
    void _OnUserCloseFinish(fs::FS_Event *ev);

private:
    fs::FS_ListenerStub _onSessionWillConnected;            // ��������
    fs::FS_ListenerStub _onSessionConnecting;               // �������� ���ڴ������������������
    fs::FS_ListenerStub _onSessionConnectFail;              // �Ự����ʧ��
    fs::FS_ListenerStub _onSessionConnected;                // �Ự����
    fs::FS_ListenerStub _afterSessionConnected;             // �Ự����
    fs::FS_ListenerStub _onSessionWillDisconnect;           // �Ự�Ͽ�
    fs::FS_ListenerStub _onSessionDisconnect;               // �Ự�Ͽ�
    fs::FS_ListenerStub _onAfterSessionDisconnect;          // �Ự�Ͽ�
    fs::FS_ListenerStub _onLogicBeforeMsgDispatch;          // ����Ϣ����֮ǰ
    fs::FS_ListenerStub _onUserLogin;                       // �û���½
    fs::FS_ListenerStub _onUserCreated;                     // �û��������
    fs::FS_ListenerStub _onUserLoginRes;                    // �û���½����
    fs::FS_ListenerStub _onUserWillClose;                   // �û������رջỰ
    fs::FS_ListenerStub _onUserClose;                       // �û��Ự�ر�
    fs::FS_ListenerStub _onUserCloseFinish;                 // �û��Ự�رս���
};
#endif