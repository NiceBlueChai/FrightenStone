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
 * @file  : NodeConnectionMgr.cpp
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/04/06
 * @brief :
 */

#include "stdafx.h"
#include "Service/LogicSvc/Modules/NodeConnection/Impl/NodeConnectionMgr.h"
#include "Service/LogicSvc/Modules/NodeConnection/Defs/PendingStubInfo.h"

#include "Service/LogicSvc/Modules/CheckStartup/CheckStartup.h"
#include "Service/LogicSvc/Modules/StubBehavior/StubBehavior.h"


INodeConnectionMgr *g_NodeConnectionMgr = NULL;

NodeConnectionMgr::NodeConnectionMgr()
    :_onLocalServerReadyStub(0)
    ,_onAfterSessionConnectedStub(0)
    , _onSessionConnectFailStub(0)
    ,_onConnectFailureStub(0)
{
    g_NodeConnectionMgr = this;
}

NodeConnectionMgr::~NodeConnectionMgr()
{
    fs::STLUtil::DelMapContainer(_pendingStubRefInfos);
    _UnRegisterEvents();
}

Int32 NodeConnectionMgr::OnInitialize()
{
    _RegisterEvents();
    return StatusDefs::Success;
}

Int32 NodeConnectionMgr::AsynConnect(const fs::FS_ConnectInfo &connectInfo)
{
    if(!connectInfo._stub || connectInfo._ip.empty() || !connectInfo._port)
    {
        g_Log->e<NodeConnectionMgr>(_LOGFMT_("param error: stub[%llu], ip[%s], port[%hu]")
                                    , connectInfo._stub
                                    , connectInfo._ip.empty() ? "none" : connectInfo._ip.c_str()
                                    , connectInfo._port);
        return StatusDefs::ParamError;
    }

    return _AsynConnect(connectInfo);
}

Int32 NodeConnectionMgr::AsynConnect(const fs::FS_ConnectInfo &connectInfo, fs::IDelegate<void, UInt64, PendingStubInfo *, Int32> *callBack)
{
    if (!connectInfo._stub || connectInfo._ip.empty() || !connectInfo._port)
    {
        g_Log->e<NodeConnectionMgr>(_LOGFMT_("param error: stub[%llu], ip[%s], port[%hu]")
            , connectInfo._stub
            , connectInfo._ip.empty() ? "none" : connectInfo._ip.c_str()
            , connectInfo._port);
        return StatusDefs::ParamError;
    }

    return _AsynConnect(connectInfo, callBack);
}

void NodeConnectionMgr::_RegisterEvents()
{
    if(_onLocalServerReadyStub)
        return;

    _onLocalServerReadyStub = g_GlobalEventMgr->AddListener(EventIds::Logic_LocalServerReady, this, &NodeConnectionMgr::_OnLocalServerReady);
    _onAfterSessionConnectedStub = g_GlobalEventMgr->AddListener(EventIds::Logic_OnAfterSessionConnected, this, &NodeConnectionMgr::_OnAfterSessionConnected);
    _onSessionDisconnectStub = g_GlobalEventMgr->AddListener(EventIds::Logic_OnSessionDisconnect, this, &NodeConnectionMgr::_OnSessionDisconnect);
    _onSessionConnectFailStub = g_GlobalEventMgr->AddListener(EventIds::Logic_OnSessionConnectFail, this, &NodeConnectionMgr::_OnSessionConnectFail);
    _onConnectFailureStub = g_GlobalEventMgr->AddListener(EventIds::Logic_ConnectFailure, this, &NodeConnectionMgr::_OnConnectFailure);
}

void NodeConnectionMgr::_UnRegisterEvents()
{
    if(!_onLocalServerReadyStub)
        return;

    g_GlobalEventMgr->RemoveListenerX(_onLocalServerReadyStub);
    g_GlobalEventMgr->RemoveListenerX(_onAfterSessionConnectedStub);
    g_GlobalEventMgr->RemoveListenerX(_onSessionDisconnectStub);
    g_GlobalEventMgr->RemoveListenerX(_onSessionConnectFailStub);
    g_GlobalEventMgr->RemoveListenerX(_onConnectFailureStub);
}

void NodeConnectionMgr::_OnLocalServerReady(fs::FS_Event *ev)
{

}

void NodeConnectionMgr::_OnAfterSessionConnected(fs::FS_Event *ev)
{
    const UInt64 sessionId = ev->GetParam(FS_EventParam::SessionId).AsUInt64();
    const UInt64 stub = ev->GetParam(FS_EventParam::Stub).AsUInt64();
    if(!stub)
        return;

    // �ж��ǲ��Ǳ�ģ����Ҫ�����Ļ�ִ
    auto iterPending = _pendingStubRefInfos.find(stub);
    if(iterPending == _pendingStubRefInfos.end())
        return;

    // ӳ��
    auto user = g_UserMgr->GetUserBySessionId(sessionId);
    _sessionIdRefConnectedUsers.insert(std::make_pair(sessionId, user));

    // ��ӡ��Ϣ
    auto session = user->GetSession();
    auto addr = session->GetAddr();
    g_Log->net<NodeConnectionMgr>("suc connect target[%s] sessionId[%llu], stub[%llu]"
                                , addr->ToString().c_str(), sessionId, stub);

    // ���Ӵ���
    _OnConnectFinish(sessionId, addr->GetAddr(), addr->GetPort(), iterPending->second, StatusDefs::Success);
}

void NodeConnectionMgr::_OnSessionDisconnect(fs::FS_Event *ev)
{
    const UInt64 sessionId = EventPtrGetParam(ev, SessionId).AsUInt64();
    _sessionIdRefConnectedUsers.erase(sessionId);
}

void NodeConnectionMgr::_OnSessionConnectFail(fs::FS_Event *ev)
{
    const UInt64 stub = EventPtrGetParam(ev, Stub).AsUInt64();
    const UInt64 sessionId = EventPtrGetParam(ev, SessionId).AsUInt64();
    if (!stub)
    {// ����������Ĺ��˵�
        return;
    }

    // �ж��Ƿ�ģ����
    auto iterStub = _pendingStubRefInfos.find(stub);
    if (iterStub == _pendingStubRefInfos.end())
        return;

    auto session = _dispatcher->GetSession(sessionId);
    auto addr = session->GetAddr();
    g_Log->w<NodeConnectionMgr>(_LOGFMT_("targetaddr[%s] connect fail when after connecting sessionId[%llu] stub[%llu]")
        , addr->ToString().c_str(), sessionId, stub);

    // ���Ӵ���
    _OnConnectFinish(sessionId, addr->GetAddr(), addr->GetPort(), iterStub->second, StatusDefs::NodeConnectionMgr_ConnetingFail);
}

void NodeConnectionMgr::_OnConnectFailure(fs::FS_Event *ev)
{
    const auto stub = ev->GetParam(FS_EventParam::Stub).AsUInt64();
    if(!stub)
    {
        g_Log->e<NodeConnectionMgr>(_LOGFMT_("stub is zero"));
        return;
    }
    
    // �ж��Ƿ�ģ����
    auto iterStub = _pendingStubRefInfos.find(stub);
    if(iterStub == _pendingStubRefInfos.end())
        return;

    const auto &ip = ev->GetParam(FS_EventParam::TargetIp).AsStr();
    const auto port = ev->GetParam(FS_EventParam::TargetPort).AsUInt16();
    const auto resultCode = ev->GetParam(FS_EventParam::ResultCode).AsInt32();
    g_Log->w<NodeConnectionMgr>(_LOGFMT_("target[%s:%hu] connect failure code[%d] stub[%llu]")
                                , ip.c_str(), port, resultCode, stub);

    // ���Ӵ���
    _OnConnectFinish(0, ip, port, iterStub->second, resultCode);
}

void NodeConnectionMgr::_OnConnectFinish(UInt64 sessionId, const fs::FS_String &ip, UInt16 port, PendingStubInfo *stubInfo, Int32 resultCode)
{
    // �����ӽ���¼�
    _FireAsynConnectResult(sessionId, ip, port, stubInfo->_pendingStub, resultCode);

    /* ���ӻص�
     *@param:sessionId �Ựid, ����ʧ�������Ϊ0
     * @param:Stub ���
     * @param:resultCode ���ص�״̬��
    */
    auto callback = stubInfo->_callback;
    if (callback)
        callback->Invoke(sessionId, stubInfo, resultCode);

    // �Ƴ�
    _RemovePendingStubInfo(stubInfo);
}

Int32 NodeConnectionMgr::_AsynConnect(const fs::FS_ConnectInfo &connectInfo, fs::IDelegate<void, UInt64, PendingStubInfo *, Int32> *callBack)
{
    auto st = _dispatcher->AsynConnect(connectInfo);
    if(st != StatusDefs::Success)
    {
        g_Log->w<NodeConnectionMgr>(_LOGFMT_("asyn connect fail st[%d] stub[%llu] target address[%s:%hu]")
                                    , st, connectInfo._stub, connectInfo._ip.c_str(), connectInfo._port);
        return st;
    }

    // �������
    auto newStubInfo = new PendingStubInfo;
    newStubInfo->_pendingStub = connectInfo._stub;
    newStubInfo->_callback = callBack;
    newStubInfo->_ip = connectInfo._ip;
    newStubInfo->_port = connectInfo._port;
    _AddNewPendingStubInfo(connectInfo._stub, newStubInfo);

    return StatusDefs::Success;
}

void NodeConnectionMgr::_FireAsynConnectResult(UInt64 sessionId, const fs::FS_String &ip, UInt16 port, UInt64 stub, Int32 resultCode)
{
    auto ev = new fs::FS_Event(EventIds::Logic_AsynConnectResult);
    ev->SetParam(FS_EventParam::TargetIp, ip);
    ev->SetParam(FS_EventParam::TargetPort, port);
    ev->SetParam(FS_EventParam::Stub, stub);
    ev->SetParam(FS_EventParam::ResultCode, resultCode);
    ev->SetParam(FS_EventParam::SessionId, sessionId);
    g_GlobalEventMgr->FireEvent(ev);
}

void NodeConnectionMgr::_AddNewPendingStubInfo(UInt64 stub, PendingStubInfo *newStubInfo)
{
    _pendingStubRefInfos.insert(std::make_pair(stub, newStubInfo));
}

void NodeConnectionMgr::_RemovePendingStubInfo(PendingStubInfo *stubInfo)
{
    _pendingStubRefInfos.erase(stubInfo->_pendingStub);
    Fs_SafeFree(stubInfo);
}
