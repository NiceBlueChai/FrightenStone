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
 * @file  : FS_Server.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/3
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/FS_Server.h"
#include "base/common/log/Log.h"
#include "base/common/socket/socket.h"
#include "base/common/component/Impl/TimeSlice.h"
#include "base/common/net/Impl/FS_Client.h"
#include "base/common/assist/utils/Impl/STLUtil.h"

FS_NAMESPACE_BEGIN

FS_Server::FS_Server()
    :_threadPool(new FS_ThreadPool(0, 1))
{
}

FS_Server::~FS_Server()
{
    g_Log->net("FS_Server%d.~FS_Server exit begin", _id);
    g_Log->sys(_LOGFMT_( "FS_Server%d.~FS_Server exit begin"), _id);
    Close();
    _locker.Lock();
    STLUtil::DelMapContainer(_socketRefClients);
    STLUtil::DelVectorContainer(_clientsCache);
    _locker.Unlock();
    g_Log->net("FS_Server%d.~FS_Server exit begin", _id);
    g_Log->sys(_LOGFMT_("FS_Server%d.~FS_Server exit begin"), _id);
    Fs_SafeFree(_threadPool);
}

#pragma region misc
void FS_Server::_ClearClients()
{
    _locker.Lock();
    STLUtil::DelMapContainer(_socketRefClients);
    STLUtil::DelVectorContainer(_clientsCache);
    _locker.Unlock();
}
#pragma endregion

#pragma region recv/addclient/start/close
Int32 FS_Server::RecvData(FS_Client *client)
{
    // ���տͻ�������
    int len = client->RecvData();
    // ����<���յ���������>�¼�
    _eventHandleObj->OnPrepareNetRecv(client);
    return len;
}

void FS_Server::Start()
{
    auto clientMsgTrasfer = DelegatePlusFactory::Create(this, &FS_Server::_ClientMsgTransfer);
    _threadPool->AddTask(clientMsgTrasfer);
}

void FS_Server::BeforeClose()
{
}

// �ر�Socket
void FS_Server::Close()
{
    BeforeClose();
    g_Log->net("FS_Server%d.Close begin", _id);
    g_Log->sys(_LOGFMT_("FS_Server%d.Close begin"), _id);
    _threadPool->Clear();
    g_Log->net("FS_Server%d.Close end", _id);
    g_Log->sys(_LOGFMT_("FS_Server%d.Close end"), _id);
}
#pragma endregion

#pragma region net message handle
void FS_Server::_ClientMsgTransfer(const FS_ThreadPool *pool)
{
    while(!pool->IsClearingPool())
    {
        if(!_clientsCache.empty())
        {// �ӻ��������ȡ���ͻ�����
            _locker.Lock();
            for(auto client : _clientsCache)
            {
                _socketRefClients[client->GetSocket()] = client;
                client->_serverId = _id;
                _eventHandleObj->OnNetJoin(client);
                _OnClientJoin(client);
            }
            _clientsCache.clear();
            _clientsChange = true;
            _locker.Unlock();
        }

        // ���û����Ҫ����Ŀͻ��ˣ�������
        if(_socketRefClients.empty())
        {
            SocketUtil::Sleep(1);

            // �ɵ�ʱ���
            _lastHeartDetectTime.FlushTime();
            continue;
        }

        // TODO:��������Ż�
        _DetectClientHeartTime();
        auto st = _BeforeClientMsgTransfer();
        if(st != StatusDefs::Success)
        {
            g_Log->net("FS_Server_OnClientStatusDirtied: st[%d] ", st);
            break;
        }

        // �ͻ�����Ϣ����
        _OnClientMsgArrived();
    }

    // �˳�������ͻ���
    _ClearClients();

    g_Log->net("FS_Server%d.OnRun exit", _id);
    g_Log->sys(_LOGFMT_("FS_Server%d.OnRun exit"), _id);
}

void FS_Server::_DetectClientHeartTime()
{
    // ��ǰʱ���
    const auto &nowTime = Time::Now();
    auto dt = nowTime - _lastHeartDetectTime;
    _lastHeartDetectTime.FlushTime();

    // ���������Ż�TODO:ʹ������
    FS_Client *client = NULL;
    for(auto iter = _socketRefClients.begin(); iter != _socketRefClients.end(); )
    {
        client = iter->second;

        // �������
        if(client->CheckHeart(dt))
        {
#ifdef FS_USE_IOCP
            if(client->IsPostIoChange())
                client->Destroy();
            else
                _OnClientLeave(client);
#else
            _OnClientLeave(client);
#endif // CELL_USE_IOCP
            iter = _socketRefClients.erase(iter);
            continue;
        }

        //// ��ʱ���ͼ��
        // pClient->checkSend(dt);

        ++iter;
    }
}

void FS_Server::_OnClientLeave(FS_Client *client)
{
    _eventHandleObj->OnNetLeave(client);
    _clientsChange = true;
    delete client;
}

void FS_Server::_OnClientJoin(FS_Client *client)
{
    // ...
}

void FS_Server::_OnPrepareNetRecv(FS_Client *client)
{
     _eventHandleObj->OnPrepareNetRecv(client);
}

void FS_Server::_OnClientMsgArrived()
{
    FS_Client *client = NULL;
    for(auto itr : _socketRefClients)
    {
        client = itr.second;
        // ѭ�� �ж��Ƿ�����Ϣ��Ҫ����
        while(client->HasRecvMsg())
        {
            // ����������Ϣ
            _HandleNetMsg(client, client->FrontMsg());
            // �Ƴ���Ϣ���У�����������ǰ��һ������
            client->PopFrontMsg();
        }
    }
}
#pragma endregion

FS_NAMESPACE_END


