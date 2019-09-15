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
 * @file  : Sender.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/9/6
 * @brief :
 * 
 *
 * 
 */

#include "stdafx.h"
#include "base/common/net/Defs/Sender.h"
#include "base/common/component/Impl/FS_ThreadPool.h"
#include "base/common/component/Impl/FS_Delegate.h"
#include "base/common/log/Log.h"
#include "base/common/net/Defs/FS_Packet.h"
#include "base/common/net/Impl/FS_Iocp.h"

FS_NAMESPACE_BEGIN
OBJ_POOL_CREATE_DEF_IMPL(Sender, __DEF_OBJ_POOL_OBJ_NUM__)
Sender::Sender(const IDelegatePlus<bool, UInt64> *isClientDestroy)
    :_pool(NULL)
    , _isDestroy(false)
    , _ioEvent(new IO_EVENT)
    , _quitIocp(NULL)
    ,_isClientDestroy(isClientDestroy)
{
    _sender = new FS_Iocp();
}

void Sender::Start()
{
    _sender->Create();
    _pool = new FS_ThreadPool(0, 1);
    auto task = DelegatePlusFactory::Create(this, &Sender::_OnWork);
    if(!_pool->AddTask(task))
    {
        g_Log->e<Sender>(_LOGFMT_("add task error"));
    }
}

void Sender::Close()
{
    g_Log->net<Sender>("sender close.");
    _locker.Lock();
    _isDestroy = true;
    if(_quitIocp)
        (*_quitIocp)();
    Fs_SafeFree(_quitIocp);
    STLUtil::DelListContainer(_packetCacheQueue);
    _locker.Sinal();
    _locker.Unlock();

    if(_pool)
        _pool->Clear();
    Fs_SafeFree(_pool);
    Fs_SafeFree(_sender);
    Fs_SafeFree(_ioEvent);
    Fs_SafeFree(_isClientDestroy);
    for(auto &iterQueue : _sendingQueue)
        STLUtil::DelListContainer(iterQueue.second);
    _needPostClientIds.clear();
}

void Sender::SendPacket(FS_Packet *packet)
{
    // ��֤���������ݵ�
    if(!packet->NeedWrite())
    {
        Fs_SafeFree(packet);
        return;
    }

    // ��ӵ����ͻ������
    _locker.Lock();
    if(_isDestroy)
    {
        _locker.Unlock();
        return;
    }

    _packetCacheQueue.push_back(packet);
    //_locker.Sinal();
    _locker.Unlock();
}

void Sender::_OnWork(const FS_ThreadPool *pool)
{
    // ע��postquit�¼�
    auto __quitIocp = [this]()->void
    {
        _sender->PostQuit();
    };
    
    if(!_quitIocp)
        _quitIocp = DelegatePlusFactory::Create<decltype(__quitIocp), void>(__quitIocp);

    FS_Packet *forPostPacket = NULL;
    PacketQueueNode *queueNode = NULL;
    UInt64 clientId = 0;
    Int32 netSt = StatusDefs::Success;
    bool isAbnormal = false;
    while(!pool->IsClearingPool()&&!_isDestroy)
    {
        // �ȴ����з������ݵ���
        //if(_needPostClientIds.empty())
        {// ��������������ȴ����ݵ���
            _locker.Lock();
            // �����������ת�Ƶ���ʽ�Ĵ����Ͷ��� ����һ��������iocp
            for(auto &packet : _packetCacheQueue)
                _Add2SendQueue(packet);
            _packetCacheQueue.clear();
            _locker.Unlock();
        }

        if(_sendingQueue.empty())
        {
            SocketUtil::Sleep(1);
            continue;
        }
        
        // post���� 
        clientId = 0;
        for(auto iterClientId = _needPostClientIds.begin();
            iterClientId!=_needPostClientIds.end();
            iterClientId = _needPostClientIds.erase(iterClientId))
        {
            clientId = *iterClientId;
            auto iterQueue = _sendingQueue.find(clientId);
            if((*_isClientDestroy)(FS_DELG_ADAPTARG(clientId)))
            {
                g_Log->w<Sender>(_LOGFMT_("clientId[%llu] is destroyed"), clientId);
                STLUtil::DelListContainer(iterQueue->second);
                _sendingQueue.erase(iterQueue);
                continue;
            }
            
            // ����Ҫ��������������postsend
            queueNode = iterQueue->second.front();
            if(queueNode->_isPost)
            {
                g_Log->e<Sender>(_LOGFMT_("node has post clientId[%llu] socket[%llu]")
                                 , clientId, queueNode->_packet->GetSocket());
                continue;
            }


            forPostPacket = queueNode->_packet;
            auto ioData = forPostPacket->MakeSendIoData();
            ioData->_node = queueNode;
            if(_sender->PostSend(forPostPacket->GetSocket(), ioData) != StatusDefs::Success)
            {
                g_Log->net<Sender>("_OnWork postsend fail clientid[%llu] sock[%llu]"
                                   , forPostPacket->GetOwnerId(), forPostPacket->GetSocket());

                SocketUtil::DestroySocket(forPostPacket->GetSocket());
                STLUtil::DelListContainer(iterQueue->second);
                _sendingQueue.erase(iterQueue);
                continue;
            }
            queueNode->_isPost = true;
        }

        // �ȴ�������ɣ���ʱ1ms
        while(true)
        {
            netSt = _WaitForNetResponse();
            if(netSt == StatusDefs::IOCP_WaitTimeOut)
                break;
            else if(netSt != StatusDefs::Success)
            {
                g_Log->e<Sender>(_LOGFMT_("fatal error when waiting for net back."
                                          " thread quit with netSt[%d]")
                                 , netSt);
                isAbnormal = true;
                break;
            }
        }

        if(isAbnormal)
            break;
    }

    // ��������
    _locker.Lock();
    _isDestroy = true;
    _ClearData();
    _locker.Unlock();
}

void Sender::_OnSendSuc(PacketQueueNode *node, Int32 transferBytes)
{
}

Int32 Sender::_WaitForNetResponse()
{
    auto ret = _sender->WaitForCompletion(*_ioEvent, 1);
    if(ret != StatusDefs::Success)
    {
        if(ret != StatusDefs::IOCP_WaitTimeOut)
            g_Log->net<Sender>("_WaitForNetResponse.wait nothing but ret[%d]", ret);
        return ret;
    }

    // ����iocp�˳�
    if(_ioEvent->_data._code == IocpDefs::IO_QUIT)
    {
        g_Log->sys<Sender>(_LOGFMT_("iocp�˳� code=%lld"), _ioEvent->_data._code);
        return StatusDefs::IOCP_Quit;
    }

    // �ж�client�Ƿ��ѱ��Ƴ�
    UInt64 clientId = _ioEvent->_data._clientId;
    auto iterClientQueue = _sendingQueue.find(clientId);
    if(iterClientQueue == _sendingQueue.end())
    {
        g_Log->net<Sender>("clientId[%llu] is removed before", clientId);
        return ret;
    }

    auto &clientQueue = iterClientQueue->second;
    if(IocpDefs::IO_SEND == _ioEvent->_ioData->_ioType)
    {// �������� ��� Completion

        if(_ioEvent->_bytesTrans <= 0)
        {// �ͻ��˶Ͽ�����
            g_Log->any<Sender>("client sock[%llu] clientId[%llu] IO_TYPE::IO_SEND bytesTrans[%lu]"
                                                 , _ioEvent->_ioData->_sock
                                                 , clientId
                                                 , _ioEvent->_bytesTrans);
            g_Log->net<Sender>("client sock[%llu] clientId[%llu] will remove IO_TYPE::IO_SEND bytesTrans[%lu]"
                                                 , _ioEvent->_ioData->_sock
                                                 , clientId
                                                 , _ioEvent->_bytesTrans);
            _RemoveFromQueue(clientId);
            return ret;
        }

        const SOCKET clientSocket = _ioEvent->_ioData->_sock;
        // �ж��Ƿ�Ͽ� �ѶϿ����п������ϴ�postsendֻ��1ms��ϵͳû�����send��ͬʱ�ͻ��˱��Ƴ�����
        if((*_isClientDestroy)(FS_DELG_ADAPTARG(clientId)))
        {
            g_Log->e<Sender>(_LOGFMT_("IO_SEND clientId[%llu] clientSocket[%llu] is destroy")
                                               , clientId, clientSocket);
            _RemoveFromQueue(clientId);
            return ret;
        }

        // ��ɻص�
        (*_ioEvent->_ioData->_completedCallback)(FS_DELG_ADAPTARG(static_cast<Int32>(_ioEvent->_bytesTrans)));
        auto node = _ioEvent->_ioData->_node;

        node->_isPost = false;
        auto clientPacket = node->_packet;
        if(clientPacket->IsEmpty())
        {
            clientQueue.erase(node->_iterNode);
            Fs_SafeFree(node);
            if(clientQueue.empty())
                _sendingQueue.erase(iterClientQueue);
        }

        if(!clientQueue.empty())
            _needPostClientIds.insert(clientId);
    }
    else
    {
        g_Log->e<Sender>(_LOGFMT_("undefine io type[%d] clientId[%llu] socket[%llu].")
                         , _ioEvent->_ioData->_ioType, clientId, _ioEvent->_ioData->_sock);
    }

    return ret;
}

void Sender::_Add2SendQueue(FS_Packet *packet)
{
    const UInt64 ownerId = packet->GetOwnerId();
    const SOCKET sock = packet->GetSocket();
    auto iterSendingQueue = _sendingQueue.find(ownerId);
    if(iterSendingQueue == _sendingQueue.end())
    {
        // ����iocp
        const Int32 st = _sender->Reg(sock, ownerId);
        if(st != StatusDefs::Success)
        {
            g_Log->net<Sender>("_Add2SendQueue: Reg socket[%llu] with complete key clientId[%llu] fail st[%d]"
                               , sock, ownerId, st);
            return;
        }

        iterSendingQueue = _sendingQueue.insert(std::make_pair(ownerId, std::list<PacketQueueNode *>())).first;
    }

    auto &sendingQueue = iterSendingQueue->second;
    PacketQueueNode *node = new PacketQueueNode;
    node->_packet = packet;
    sendingQueue.push_back(node);
    node->_iterNode = --sendingQueue.end();

    if(!sendingQueue.front()->_isPost)
        _needPostClientIds.insert(ownerId);
}

void Sender::_RemoveFromQueue(UInt64 clientId)
{
    auto iterQueue = _sendingQueue.find(clientId);
    auto &clientQueue = iterQueue->second;
    
    // closesocket
    SocketUtil::DestroySocket(clientQueue.front()->_packet->GetSocket());
    STLUtil::DelListContainer(clientQueue);
    _sendingQueue.erase(iterQueue);
    _needPostClientIds.erase(clientId);
}

void Sender::_ClearData()
{
    STLUtil::DelListContainer(_packetCacheQueue);
    for(auto &iterQueue : _sendingQueue)
        STLUtil::DelListContainer(iterQueue.second);
    _needPostClientIds.clear();
}

FS_NAMESPACE_END
