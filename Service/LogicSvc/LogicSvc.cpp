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
 * @file  : LogicSvc.cpp
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/1/15
 * @brief :
 */
#include "stdafx.h"
#include "Service/LogicSvc/LogicSvc.h"
#include "Service/Common/LogicCommon.h"
#include "Service/LogicSvc/Modules/Common/ModuleCommon.h"

#include "Service/LogicSvc/Modules/NodeConnection/NodeConnection.h"
#include "Service/LogicSvc/Modules/CheckStartup/CheckStartup.h"
#include "Service/LogicSvc/Modules/ProtocolStack/ProtocolStack.h"
#include "Service/LogicSvc/Modules/EventFilter/EventFilter.h"
#include "Service/LogicSvc/Modules/User/User.h"
#include "Service/LogicSvc/Modules/GlobalSys/GlobalSys.h"

#include "Service/LogicSvc/Modules/TestSys/TestSys.h"
#include "Service/LogicSvc/Modules/ProtocolProtector/ProtocolProtector.h"
#include "Service/LogicSvc/Modules/StubBehavior/StubBehavior.h"
#include "Service/LogicSvc/Modules/TestProtocols/TestProtocols.h"

#include "Service/LogicSvc/Cfgs/Cfgs.h"

OBJ_POOL_CREATE_DEF_IMPL(LogicSvc, 1)
void LogicSvc::Release()
{
    delete this;
}

void LogicSvc::OnClose()
{
    CfgLoader::Clear();
}

Int32 LogicSvc::OnLoadCfgs()
{
    Int32 st = StatusDefs::Success;
    st = CfgLoader::LoadAllCfgs();
    if (st != StatusDefs::Success)
    {
        g_Log->e<LogicSvc>(_LOGFMT_("CfgLoader::LoadAllCfgs fail st[%d]"), st);
        CfgLoader::Clear();
        return st;
    }

    return StatusDefs::Success;
}

void LogicSvc::OnRegisterFacades()
{
    // ע���������
    RegisterFacade<ProtocolMsgHandlerMgrFactory>();
    RegisterFacade<ProtocolStackMgrFactory>();
    RegisterFacade<CheckStartupMgrFactory>();
    RegisterFacade<NodeConnectionMgrFactory>();
    RegisterFacade<EventFilterFactory>();
    RegisterFacade<UserMgrFactory>();
    RegisterFacade<GlobalSysMgrFactory>();
    RegisterFacade<ProtocolBlackListMgrFactory>();

    // ��ע��globalϵͳ
    RegisterFacade<TestSysGlobalFactory>();
    RegisterFacade<StubBehaviorGlobalFactory>();
    RegisterFacade<ProtocolProtectorGlobalFactory>();
    RegisterFacade<TestProtocolsGlobalFactory>();

    // ע��userϵͳ
    RegisterFacade<TestSysFactory>();
    RegisterFacade<TestProtocolsFactory>();
}

void LogicSvc::OnLocalServerReady()
{
    // �ױ��ط����������¼� 
    auto ev = new fs::FS_Event(EventIds::Logic_LocalServerReady);
    g_GlobalEventMgr->FireEvent(ev);
}

void LogicSvc::WillStartup()
{
    g_Log->i<LogicSvc>(_LOGFMT_("server will start up."));
    auto ev = new fs::FS_Event(EventIds::Logic_WillStartup);
    g_GlobalEventMgr->FireEvent(ev);
}
// 
// void LogicSvc::OnMsgDispatch(UInt64 sessionId, fs::NetMsgDecoder *msgDecoder)
// {
//     // ����Ϣ�����¼�
// //     fs::FS_Event *ev = new fs::FS_Event(EventIds::Logic_BeforeMsgDispatch);
// //     ev->SetParam(FS_EventParam::SessionId, sessionId);
// //     ev->SetParam("MsgDecoder", msgDecoder);
// //     g_GlobalEventMgr->FireEvent(ev);
// 
//     fs::Time s, e;
//     s.FlushTime();
//     g_ProtocolStackMgr->InvokeProtocolHandler(sessionId, msgDecoder);
//     e.FlushTime();
//     Int64 useTime = (s - e).GetTotalMilliSeconds();
// 
//     // ��ǰЭ���ʱ����
//     g_Log->performance<LogicSvc>(_LOGFMT_("sessionId[%llu], cmd[%d], len[%d], use time[%lld]ms"), sessionId, msgDecoder->GetCmd(), msgDecoder->GetMsgLen(), useTime);
// // 
// //     auto st = g_ProtocolProtectorGlobal->CheckProtocol(sessionId, msgDecoder);
// //     if(st != StatusDefs::Success)
// //     {
// //         g_Log->w<LogicSvc>(_LOGFMT_("check protocol fail st[%d] sessionId[%llu]"), st, sessionId);
// //         return;
// //     }
// // 
// // //     const auto cmd = msgDecoder->GetCmd();
// // //     if(cmd >= fs::ProtocolCmd::CMD_End ||
// // //        cmd <= fs::ProtocolCmd::CMD_Begin)
// // //     {
// // //         // TODO:�����Э��ſ���������̽�Թ���
// // //         g_Log->w<LogicSvc>(_LOGFMT_("unknown protocolcmd[%u] len[%u] sessionId[%llu]")
// // //                           , cmd, msgDecoder->GetMsgLen(), sessionId);
// // //         return;
// // //     }
// // 
// //     // TODO:�費��Ҫ�Ե������ݳ��Ƚ�������,��֤�����յ����ȹ���?
// //     // if(msgData->_packetLength)
// //     InvokeProtocolHandler(sessionId, msgDecoder);
// }

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

    ev = new fs::FS_Event(EventIds::Logic_OnSessionDisconnect);
    EventPtrSetParam(ev, SessionId, sessionId);
    g_GlobalEventMgr->FireEvent(ev);

    ev = new fs::FS_Event(EventIds::Logic_OnAfterSessionDisconnected);
    EventPtrSetParam(ev, SessionId, sessionId);
    g_GlobalEventMgr->FireEvent(ev);
}

Int32 LogicSvc::OnSessionConnected(UInt64 sessionId, UInt64 stub)
{
    // ��ʼ��userϵͳ
    auto *ev = new fs::FS_Event(EventIds::Logic_OnSessionWillConnected);
    EventPtrSetParam(ev, SessionId, sessionId);
    EventPtrSetParam(ev, Stub, stub);
    g_GlobalEventMgr->FireEvent(ev);

    // ���ڴ����������������� ҵ��Ķ������������ʱ����
    ev = new fs::FS_Event(EventIds::Logic_OnSessionConnecting);
    EventPtrSetParam(ev, SessionId, sessionId);
    EventPtrSetParam(ev, Stub, stub);
    g_GlobalEventMgr->FireEvent(ev);

    // ������;session���رգ��������������ҵ������߼�,��Ϊ�Ͽ����� TODO:���������¼����ҵ���߼��Ƿ����Ӱ��,����Ӧ��������ʧ���¼�
    auto session = _dispatcher->GetSession(sessionId);
    if (!session->IsValid())
    {
#ifdef _DEBUG
        g_Log->net<LogicSvc>("sessionId[%llu], stub[%llu], become invalid after connecting addr[%s]"
            , sessionId, stub, session->GetAddr()->ToString().c_str());
#endif

        ev = new fs::FS_Event(EventIds::Logic_OnSessionConnectFail);
        EventPtrSetParam(ev, SessionId, sessionId);
        EventPtrSetParam(ev, Stub, stub);
        g_GlobalEventMgr->FireEvent(ev);

        return StatusDefs::Success;
    }

    // ���ҵ����
    ev = new fs::FS_Event(EventIds::Logic_OnSessionConnected);
    EventPtrSetParam(ev, SessionId, sessionId);
    EventPtrSetParam(ev, Stub, stub);
    g_GlobalEventMgr->FireEvent(ev);

    // ������ϵͳ֮��
    ev = new fs::FS_Event(EventIds::Logic_OnAfterSessionConnected);
    EventPtrSetParam(ev, SessionId, sessionId);
    EventPtrSetParam(ev, Stub, stub);
    g_GlobalEventMgr->FireEvent(ev);

    g_Log->net<LogicSvc>("new sessionId[%llu] stub[%llu] connected "
                  , sessionId, stub);

    return StatusDefs::Success;
}

void LogicSvc::OnConnectFailureRes(const fs::FS_ConnectInfo &connectInfo, Int32 resultCode)
{
    auto ev = new fs::FS_Event(EventIds::Logic_ConnectFailure);
    ev->SetParam(FS_EventParam::TargetIp, connectInfo._ip);
    ev->SetParam(FS_EventParam::TargetPort, connectInfo._port);
    ev->SetParam(FS_EventParam::Stub, connectInfo._stub);
    ev->SetParam(FS_EventParam::ResultCode, resultCode);
    g_GlobalEventMgr->FireEvent(ev);
}

void LogicSvc::OnSessionMsgHandle(fs::FS_Session *session)
{

}
