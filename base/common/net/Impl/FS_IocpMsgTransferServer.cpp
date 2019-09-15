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
 * @file  : FS_IocpMsgTransferServer.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/5
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/FS_IocpMsgTransferServer.h"
#include "base/common/net/Impl/FS_Iocp.h"
#include "base/common/net/Impl/FS_Client.h"
#include "base/common/net/Defs/IocpDefs.h"
#include "base/common/net/Defs/FS_NetDefs.h"
#include "base/common/net/Defs/FS_NetBuffer.h"
#include "base/common/net/Impl/FS_Iocp.h"
#include "base/common/net/Interface/INetEvent.h"
#include "base/common/log/Log.h"
#include "base/common/net/Defs/Sender.h"

FS_NAMESPACE_BEGIN

OBJ_POOL_CREATE_IMPL(FS_IocpMsgTransferServer, _objPoolHelper, __DEF_OBJ_POOL_OBJ_NUM__)

FS_IocpMsgTransferServer::FS_IocpMsgTransferServer()
    :_iocpClientMsgTransfer(new FS_Iocp)
    ,_ioEvent(new IO_EVENT)
    ,_sender(NULL)
{
    auto __isClientDestroy = DelegatePlusFactory::Create(this, &FS_IocpMsgTransferServer::_IsClientDestroy);
    _sender = new Sender(__isClientDestroy);
    _iocpClientMsgTransfer->Create();
}

FS_IocpMsgTransferServer::~FS_IocpMsgTransferServer()
{
    Close();
    Fs_SafeFree(_sender);
    Fs_SafeFree(_iocpClientMsgTransfer);
    Fs_SafeFree(_ioEvent);
}

void FS_IocpMsgTransferServer::OnStart()
{
    _sender->Start();
}

void FS_IocpMsgTransferServer::BeforeClose()
{
    g_Log->net<FS_IocpMsgTransferServer>("FS_IocpMsgTransferServer%d.BeforeClose begin quit iocp", _id);
    // �˳�iocp
    _sender->Close();
    _iocpClientMsgTransfer->PostQuit();
}

Int32 FS_IocpMsgTransferServer::_OnClientNetEventHandle()
{
    // 1.����post �ͻ�������
    FS_Client *client = NULL;
    for(auto iterClientId = _needToPostClientIds.begin(); iterClientId!=_needToPostClientIds.end(); ++iterClientId)
    {
        auto iterClient = _clientIdRefClients.find(*iterClientId);
        if(iterClient == _clientIdRefClients.end())
            continue;

        client = iterClient->second;
        if(client->IsDestroy())
        {
            g_Log->w<FS_IocpMsgTransferServer>(_LOGFMT_("clientId[%llu] is destroyed"), iterClient->first);
            _OnClientLeaveAndEraseFromQueue(iterClient);
            continue;
        }
        
        // TODO:��Ҫ������1ms�ڿͻ����Ƿ���Ҫ���recv��
        // ֻ��ûͶ�ݹ�recv�Ŀͻ��˲ſ���Ͷ��recv��
        // ������Ͷ�ݣ����ͻ������ٺ��п���recv�����
        auto ioData = client->MakeRecvIoData();
        if(ioData)
        {
            if(_iocpClientMsgTransfer->PostRecv(client->GetSocket(), ioData) != StatusDefs::Success)
            {
                g_Log->net<FS_IocpMsgTransferServer>("_BeforeClientMsgTransfer PostRecv fail clientid[%llu] sock[%llu]"
                                                        , client->GetId(), client->GetSocket());
                _OnClientLeaveAndEraseFromQueue(iterClient);
                continue;
            }
        }
    }
    _needToPostClientIds.clear();

    // 2.iocp�ȴ���Ϣ���ֱ��timeout��errorΪֹ
    Int32 ret = StatusDefs::Success;
    while(true)
    {// ֻ����wait��ʱ����ζ���ⳬʱ�����ͻ��������ݵ���ֻ�ܵȵ��´�Ͷ����recv��
        ret = _ListenIocpNetEvents();
        if(ret == StatusDefs::IOCP_WaitTimeOut)
            return StatusDefs::Success;
        else if(ret != StatusDefs::Success)
            return ret;
    }

    return StatusDefs::Success;
}

Int32 FS_IocpMsgTransferServer::_ListenIocpNetEvents()
{
    auto ret = _iocpClientMsgTransfer->WaitForCompletion(*_ioEvent, 1);
    if(ret != StatusDefs::Success)
    {
        if(ret != StatusDefs::IOCP_WaitTimeOut)
            g_Log->net<FS_IocpMsgTransferServer>("FS_IOCPServer%d.DoIocpNetEvents.wait nothing but ret[%d]", _id, ret);
        // g_Log->any<FS_IocpMsgTransferServer>("FS_IOCPServer%d.DoIocpNetEvents.wait nothing but ret[%d]", _id, ret);
        return ret;
    }

    // ����iocp�˳�
    if(_ioEvent->_data._code == IocpDefs::IO_QUIT)
    {
        g_Log->sys<FS_IocpMsgTransferServer>(_LOGFMT_("iocp�˳� code=%lld"), _ioEvent->_data._code);
        return StatusDefs::IOCP_Quit;
    }

    // �ж�client�Ƿ��ѱ��Ƴ�
    const UInt64 clientId = _ioEvent->_data._clientId;
    auto client = _GetClient(clientId);
    if(!client)
    {
        g_Log->net<FS_IocpMsgTransferServer>("clientId[%llu] is removed before", clientId);
        return ret;
    }

    if(IocpDefs::IO_RECV == _ioEvent->_ioData->_ioType)
    {// �������� ��� Completion

        if(_ioEvent->_bytesTrans <= 0)
        {// �ͻ��˶Ͽ�����
            g_Log->any<FS_IocpMsgTransferServer>("client sock[%llu] clientId[%llu] IO_TYPE::RECV bytesTrans[%lu]"
                                                 , client->GetSocket()
                                                 , clientId,
                                                 _ioEvent->_bytesTrans);
            g_Log->net<FS_IocpMsgTransferServer>("client sock[%llu] clientId[%llu] will remove IO_TYPE::IO_RECV bytesTrans[%lu]"
                                                 , client->GetSocket()
                                                 , clientId
                                                 , _ioEvent->_bytesTrans);
            //CELLLog_Info("rmClient sockfd=%d, IO_TYPE::RECV bytesTrans=%d", _ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);
            // _RmClient(*_ioEvent);
            _RmClient(client);
            return ret;
        }

        // �ж��Ƿ�Ͽ� �ѶϿ����п������ϴ�postsendֻ��1ms��ϵͳû�����send��ͬʱ�ͻ��˱��Ƴ�����
        if(client->IsDestroy())
        {
            g_Log->e<FS_IocpMsgTransferServer>(_LOGFMT_("IO_RECV clientId[%llu] is destroy")
                                                , client->GetId());
        }

        // ��ɻص�
        (*_ioEvent->_ioData->_completedCallback)(std::forward<Int32>(static_cast<Int32>(_ioEvent->_bytesTrans)));
        client->ResetPostRecv();
        _OnPrepareNetRecv(client);
        _msgArrivedClientIds.insert(clientId);
        _needToPostClientIds.insert(clientId);
    }
    else 
    {
        g_Log->e<FS_IocpMsgTransferServer>(_LOGFMT_("undefine io type[%d]."), _ioEvent->_ioData->_ioType);
    }

    return ret;
}

void FS_IocpMsgTransferServer::_OnClientJoin(FS_Client *client)
{
    // �������ʱ�����������
    client->SetSender(DelegatePlusFactory::Create(_sender, &Sender::SendPacket));
    _iocpClientMsgTransfer->Reg(client->GetSocket(), client->GetId());
    auto ioData = client->MakeRecvIoData();
    _aliveGuard.Lock();
    _aliveClientIds.insert(client->GetId());
    if(ioData)
    {
        if(_iocpClientMsgTransfer->PostRecv(client->GetSocket(), ioData) != StatusDefs::Success)
        {
            g_Log->net<FS_IocpMsgTransferServer>("_OnClientJoin PostRecv fail clientid[%llu] sock[%llu]"
                                                 , client->GetId(), client->GetSocket());
            client->Close();
            _aliveClientIds.erase(client->GetId());
        }
    }
    _aliveGuard.Unlock();
}

void FS_IocpMsgTransferServer::_OnClientDestroy(UInt64 clientId)
{
    _aliveGuard.Lock();
    _aliveClientIds.erase(clientId);
    _aliveGuard.Unlock();
}

bool FS_IocpMsgTransferServer::_IsClientDestroy(UInt64 clientId)
{
    _aliveGuard.Lock();
    auto iterClientId = _aliveClientIds.find(clientId);
    if(iterClientId == _aliveClientIds.end())
    {
        _aliveGuard.Unlock();
        return true;
    }
    _aliveGuard.Unlock();
    return false;
}
FS_NAMESPACE_END

