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
 * @file  : LogicSvc.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/1/15
 * @brief :
 */
#include "stdafx.h"
#include "Service/LogicSvc/LogicSvc.h"
#include "Service/Common/LogicCommon.h"
#include "Service/LogicSvc/Modules/User/User.h"
#include "Service/LogicSvc/Modules/GlobalSys/GlobalSys.h"
#include "Service/LogicSvc/Modules/Common/ModuleCommon.h"
#include "Service/LogicSvc/Modules/TestSys/TestSys.h"
#include "Service/LogicSvc/Modules/EventFilter/EventFilter.h"
#include "Service/LogicSvc/Modules/ProtocolProtector/ProtocolProtector.h"

void LogicSvc::Release()
{
    delete this;
}

void LogicSvc::OnRegisterFacades()
{
    // ע���������
    RegisterFacade<UserMgrFactory>();
    RegisterFacade<GlobalSysMgrFactory>();
    RegisterFacade<EventFilterFactory>();

    // ע������
    // ����ϵͳ
    RegisterFacade<TestSysFactory>();
    // ����global
    RegisterFacade<TestSysGlobalFactory>();
    // Э�����
    RegisterFacade<ProtocolProtectorGlobalFactory>();
}

void LogicSvc::OnMsgDispatch(UInt64 sessionId, fs::NetMsgDecoder *msgDecoder)
{
    // ����Ϣ�����¼�
//     fs::FS_Event *ev = new fs::FS_Event(EventIds::Logic_BeforeMsgDispatch);
//     ev->SetParam(FS_EventParam::SessionId, sessionId);
//     ev->SetParam("MsgDecoder", msgDecoder);
//     g_GlobalEventMgr->FireEvent(ev);

    auto st = g_ProtocolProtectorGlobal->CheckProtocol(sessionId, msgDecoder);
    if(st != StatusDefs::Success)
    {
        g_Log->w<LogicSvc>(_LOGFMT_("check protocol fail st[%d] sessionId[%llu]"), st, sessionId);
        return;
    }

//     const auto cmd = msgDecoder->GetCmd();
//     if(cmd >= fs::ProtocolCmd::CMD_End ||
//        cmd <= fs::ProtocolCmd::CMD_Begin)
//     {
//         // TODO:�����Э��ſ���������̽�Թ���
//         g_Log->w<LogicSvc>(_LOGFMT_("unknown protocolcmd[%u] len[%u] sessionId[%llu]")
//                           , cmd, msgDecoder->GetMsgLen(), sessionId);
//         return;
//     }

    // TODO:�費��Ҫ�Ե������ݳ��Ƚ�������,��֤�����յ����ȹ���?
    // if(msgData->_packetLength)
    InvokeProtocolHandler(sessionId, msgDecoder);
}

void LogicSvc::OnSessionDisconnected(UInt64 sessionId, std::list<fs::IDelegate<void, UInt64> *> *disconnectedDelegate)
{
    if(disconnectedDelegate)
    {
        for(auto iterDelegate = disconnectedDelegate->begin(); iterDelegate != disconnectedDelegate->end(); )
        {
            auto item = *iterDelegate;
            item->Invoke(sessionId);
            FS_Release(item);
            iterDelegate = disconnectedDelegate->erase(iterDelegate);
        }
    }

    fs::FS_Event *ev = new fs::FS_Event(EventIds::Logic_OnSessionWillDisconnect);
    ev->SetParam(FS_EventParam::SessionId, sessionId);
    g_GlobalEventMgr->FireEvent(ev);

    fs::FS_Event *afterEv = new fs::FS_Event(EventIds::Logic_OnAfterSessionDisconnected);
    afterEv->SetParam(FS_EventParam::SessionId, sessionId);
    g_GlobalEventMgr->FireEvent(afterEv);
}

Int32 LogicSvc::OnSessionConnected(UInt64 sessionId)
{
    // �մ���userϵͳ
    fs::FS_Event *ev = new fs::FS_Event(EventIds::Logic_OnSessionConnected);
    ev->SetParam(FS_EventParam::SessionId, sessionId);
    g_GlobalEventMgr->FireEvent(ev);

    // ������ϵͳ֮��
    fs::FS_Event *afterSessionConnectedEv = new fs::FS_Event(EventIds::Logic_OnAfterSessionConnected);
    afterSessionConnectedEv->SetParam(FS_EventParam::SessionId, sessionId);
    g_GlobalEventMgr->FireEvent(afterSessionConnectedEv);

    g_Log->i<LogicSvc>(_LOGFMT_("new sessionId[%llu] connected ")
                  , sessionId);

    return StatusDefs::Success;
}

void LogicSvc::OnConnectorConnectOpFinish()
{
    // ���������Ӳ�����ɵ���һ������ȫ�����ӳɹ�
    fs::FS_Event *ev = new fs::FS_Event(EventIds::Logic_OnAllConnectToRemoteOpFinish);
    g_GlobalEventMgr->FireEvent(ev);
}
