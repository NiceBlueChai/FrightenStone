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

FS_NAMESPACE_BEGIN

OBJ_POOL_CREATE_IMPL(FS_IocpMsgTransferServer, _objPoolHelper, __DEF_OBJ_POOL_OBJ_NUM__)

FS_IocpMsgTransferServer::FS_IocpMsgTransferServer()
    :_iocpClientMsgTransfer(new FS_Iocp)
    ,_ioEvent(new IO_EVENT)
{
    _iocpClientMsgTransfer->Create();
}

FS_IocpMsgTransferServer::~FS_IocpMsgTransferServer()
{
    Close();
    Fs_SafeFree(_iocpClientMsgTransfer);
    Fs_SafeFree(_ioEvent);
}

void FS_IocpMsgTransferServer::BeforeClose()
{
    g_Log->net<FS_IocpMsgTransferServer>("FS_IocpMsgTransferServer%d.BeforeClose begin quit iocp", _id);
    // �˳�iocp
    _iocpClientMsgTransfer->PostQuit();
}

Int32 FS_IocpMsgTransferServer::_BeforeClientMsgTransfer(std::set<UInt64> &delayDestroyClients)
{
    // 1.����post �ͻ�������
    FS_Client *client = NULL;
    for(auto iter = _clientIdRefClients.begin(); iter != _clientIdRefClients.end(); ++iter)
    {
        client = iter->second;
        if(client->IsDestroy())
        {
            g_Log->w<FS_IocpMsgTransferServer>(_LOGFMT_("clientId[%llu] is destroyed"), iter->first);
            continue;
        }

        // ��Ҫд���ݵĿͻ���,��postSend
        if(client->NeedWrite())
        {
            auto ioData = client->MakeSendIoData();
            if(ioData)
            {
                if(_iocpClientMsgTransfer->PostSend(ioData) != StatusDefs::Success)
                {
                    delayDestroyClients.insert(client->GetId());
                    continue;
                }
            }

            ioData = client->MakeRecvIoData();
            if(ioData)
            {
                if(_iocpClientMsgTransfer->PostRecv(ioData) != StatusDefs::Success)
                {
                    delayDestroyClients.insert(client->GetId());
                    continue;
                }
            }
        }
        else if(!client->IsPostIoChange()){
            // TODO:��Ҫ������1ms�ڿͻ����Ƿ���Ҫ���recv��ֻ��ûͶ�ݹ�recv�Ŀͻ��˲ſ���Ͷ��recv��������Ͷ�ݣ����ͻ������ٺ��п���recv�����
            auto ioData = client->MakeRecvIoData();
            if(ioData)
            {
                if(_iocpClientMsgTransfer->PostRecv(ioData) != StatusDefs::Success)
                {
                    delayDestroyClients.insert(client->GetId());
                    continue;
                }
            }
        }
    }

    // 2.iocp�ȴ���Ϣ���ֱ��timeout��errorΪֹ
    Int32 ret = StatusDefs::Success;
    while(true)
    {// ֻ����wait��ʱ����ζ���ⳬʱ�����ͻ��������ݵ���ֻ�ܵȵ��´�Ͷ����recv��
        ret = _ListenIocpNetEvents(delayDestroyClients);
        if(ret == StatusDefs::IOCP_WaitTimeOut)
            return StatusDefs::Success;
        else if(ret != StatusDefs::Success)
            return ret;
    }

    return StatusDefs::Success;
}

Int32 FS_IocpMsgTransferServer::_ListenIocpNetEvents(std::set<UInt64> &delayDestroyClients)
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

    if(IocpDefs::IO_RECV == _ioEvent->_ioData->_ioType)
    {// �������� ��� Completion

        if(_ioEvent->_bytesTrans <= 0)
        {// �ͻ��˶Ͽ�����
            
            //CELLLog_Info("rmClient sockfd=%d, IO_TYPE::RECV bytesTrans=%d", _ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);
            // _RmClient(*_ioEvent);
            _DelayRmClient(_ioEvent, delayDestroyClients);
            FS_Client *client = reinterpret_cast<FS_Client *>(_ioEvent->_data._ptr);
            g_Log->any<FS_IocpMsgTransferServer>("client[%llu] IO_TYPE::RECV bytesTrans[%d]",client?client->GetSocket():0, _ioEvent->_bytesTrans);
            return ret;
        }

        FS_Client *client = reinterpret_cast<FS_Client*>(_ioEvent->_data._ptr);
        if(client)
        {// �ͻ��˽�������
         
            // �ж��Ƿ�Ͽ� �ѶϿ����п������ϴ�postsendֻ��1ms��ϵͳû�����send��ͬʱ�ͻ��˱��Ƴ�����
            if(client->IsDestroy() ||
               _clientIdRefClients.find(client->GetId()) == _clientIdRefClients.end())
            {
                g_Log->e<FS_IocpMsgTransferServer>(_LOGFMT_("clientId[%llu] is destroy"), client->GetId());
                return ret;
            }

            client->OnRecvFromIocp(_ioEvent->_ioData->_owner->GetNode(), _ioEvent->_bytesTrans);
            _OnPrepareNetRecv(client);
        }
    }
    else if(IocpDefs::IO_SEND == _ioEvent->_ioData->_ioType)
    {// �������� ��� Completion

        if(_ioEvent->_bytesTrans <= 0)
        {// �ͻ��˶Ͽ�����
            _DelayRmClient(_ioEvent, delayDestroyClients);

            FS_Client *client = reinterpret_cast<FS_Client *>(_ioEvent->_data._ptr);
            g_Log->any<FS_IocpMsgTransferServer>("client[%llu] IO_TYPE::IO_SEND bytesTrans[%d]", client ? client->GetSocket() : 0, _ioEvent->_bytesTrans);

            return ret;
        }

        FS_Client *client = reinterpret_cast<FS_Client *>(_ioEvent->_data._ptr);
        if(client)
        {
            // �ж��Ƿ�Ͽ� �ѶϿ����п������ϴ�postsendֻ��1ms��ϵͳû�����send��ͬʱ�ͻ��˱��Ƴ�����
            if(client->IsDestroy() || 
               _clientIdRefClients.find(client->GetId()) == _clientIdRefClients.end())
            {
                g_Log->e<FS_IocpMsgTransferServer>(_LOGFMT_("clientId[%llu] is destroy"), client->GetId());
                return ret;
            }

            client->OnSend2iocp(_ioEvent->_ioData->_owner->GetNode(), _ioEvent->_bytesTrans);
        }
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
    _iocpClientMsgTransfer->Reg(client->GetSocket(), client);
    auto ioData = client->MakeRecvIoData();
    if(ioData)
        _iocpClientMsgTransfer->PostRecv(ioData);
}

FS_NAMESPACE_END
