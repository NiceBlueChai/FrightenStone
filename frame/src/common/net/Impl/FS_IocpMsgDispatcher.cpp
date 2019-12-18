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
 * @file  : FS_IocpMsgDispatcher.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "FrightenStone/common/net/Impl/FS_IocpMsgDispatcher.h"
#include "FrightenStone/common/net/Impl/FS_IocpSession.h"
#include "FrightenStone/common/net/Defs/FS_IocpBuffer.h"
#include "FrightenStone/common/net/Impl/IFS_MsgTransfer.h"
#include "FrightenStone/common/net/Impl/IFS_BusinessLogic.h"
#include "FrightenStone/common/net/Impl/FS_NetEngine.h"
#include "FrightenStone/common/net/ProtocolInterface/protocol.h"
#include "FrightenStone/common/net/Defs/NetCfgDefs.h"

#include "FrightenStone/common/memorypool/memorypool.h"
#include "FrightenStone/common/status/status.h"
#include "FrightenStone/common/component/Impl/FS_ThreadPool.h"
#include "FrightenStone/common/component/Impl/TimeWheel/TimeWheel.h"
#include "FrightenStone/common/component/Impl/TimeSlice.h"
#include "FrightenStone/common/component/Impl/Time.h"
#include "FrightenStone/common/component/Impl/TimeWheel/FS_Timer.h"
#include "FrightenStone/common/assist/utils/utils.h"

#ifdef _WIN32

FS_NAMESPACE_BEGIN

FS_IocpMsgDispatcher::FS_IocpMsgDispatcher(UInt32 id, FS_NetEngine *netEngine)
    :_pool(NULL)
    , _isClose{false}
    ,_timeWheel(NULL)
    ,_logic(NULL)
    ,_messgeQueue(NULL)
    ,_id(id)
    ,_recvMsgBlocks(NULL)
    ,_cfgs(NULL)
    ,_netEngine(netEngine)
{
}

FS_IocpMsgDispatcher::~FS_IocpMsgDispatcher()
{
    Fs_SafeFree(_pool);
    Fs_SafeFree(_timeWheel);
    Fs_SafeFree(_cfgs);

//     _CrtMemCheckpoint(&s2);
//     if(_CrtMemDifference(&s3, &s1, &s2))
//         _CrtMemDumpStatistics(&s3);
}

Int32 FS_IocpMsgDispatcher::BeforeStart(const DispatcherCfgs &cfgs)
{
    _cfgs = new DispatcherCfgs;
    *_cfgs = cfgs;
    _timeWheel = new TimeWheel(_cfgs->_dispatcherResolutionInterval);
    // g_BusinessTimeWheel = _timeWheel;
    _pool = new FS_ThreadPool(0, 1);
    _recvMsgBlocks = new std::list<FS_MessageBlock *>;

    if(_logic)
    {
        _logic->SetDispatcher(this);
        _logic->SetTimeWheel(_timeWheel);
        auto st = _logic->BeforeStart();
        if(st != StatusDefs::Success)
        {
            g_Log->e<FS_IocpMsgDispatcher>(_LOGFMT_("_logic->BeforeStart error st[%d]"), st);
            return st;
        }
    }

    return StatusDefs::Success;
}

Int32 FS_IocpMsgDispatcher::Start()
{
    auto task = DelegatePlusFactory::Create(this, &FS_IocpMsgDispatcher::_OnBusinessProcessThread);
    if(!_pool->AddTask(task, true))
    {
        g_Log->e<FS_IocpMsgDispatcher>(_LOGFMT_("add task fail"));
        return StatusDefs::FS_IocpMsgHandler_StartFailOfBusinessProcessThreadFailure;
    }

    if(_logic)
    {
        auto st = _logic->Start();
        if(st != StatusDefs::Success)
        {
            g_Log->e<FS_IocpMsgDispatcher>(_LOGFMT_("_logic->Start error st[%d]"), st);
            return st;
        }
    }

    return StatusDefs::Success;
}

void FS_IocpMsgDispatcher::BeforeClose()
{
    if(_isClose)
        return;

    _messgeQueue->PopLock(_id);
    _messgeQueue->NotifyPop(_id);
    _messgeQueue->PopUnlock(_id);

    if(_logic)
        _logic->BeforeClose();
}

void FS_IocpMsgDispatcher::Close()
{
    if(_isClose)
        return;

    _isClose = true;

    // 线程退出
    _pool->Close();

    if(_logic)
        _logic->Close();

    if(!_recvMsgBlocks->empty())
    {
        g_Log->w<FS_IocpMsgDispatcher>(_LOGFMT_("recv msg block queue has unhandled msgs[%llu]"), _recvMsgBlocks->size());
        for(auto &recvMsgBlock : *_recvMsgBlocks)
        {
            FS_NetMsgBufferBlock *netMsg = recvMsgBlock->CastTo<FS_NetMsgBufferBlock>();
            NetMsg_DataHeader *header = netMsg->CastBufferTo<NetMsg_DataHeader>();
            g_Log->net<FS_IocpMsgDispatcher>("net msg[%hu] data size[%hu] generatorid[%d] sessionId[%llu] is unhandled"
                                             , header->_cmd
                                             , header->_packetLength
                                             , netMsg->_generatorId
                                             , netMsg->_sessionId);
        }

    }

    STLUtil::DelListContainer(*_recvMsgBlocks);
    Fs_SafeFree(_recvMsgBlocks);
    for(auto &iterDelegateInfo : _sessionIdRefUserDisconnected)
        STLUtil::DelListContainer(iterDelegateInfo.second);
    _sessionIdRefUserDisconnected.clear();
}
// 
// void FS_IocpMsgDispatcher::OnRecv(std::list<IFS_Session *> &sessions, Int64 &incPacketsCnt)
// {
//     incPacketsCnt = 0;
// 
//     _locker.Lock();
//     if(!_isClose)
//     {
//         IFS_Session *session = NULL;
//         bool hasMsg = false;
//         for(auto iterSession = sessions.begin(); iterSession != sessions.end();)
//         {
//             session = *iterSession;
//             auto iocpSession = session->CastTo<FS_IocpSession>();
//             auto recvBuffer = iocpSession->GetRecvBuffer()->CastToBuffer<FS_IocpBuffer>();
//             
//             hasMsg |= recvBuffer->HasMsg();
//             while(recvBuffer->HasMsg())
//             {
//                 auto frontMsg = recvBuffer->CastToData<NetMsg_DataHeader>();
//                 _MoveToBusinessLayer(session, frontMsg);
//                 recvBuffer->PopFront(frontMsg->_packetLength);
//                 ++incPacketsCnt;
//             }
// 
//             iterSession = sessions.erase(iterSession);
//         }
// 
//         if(hasMsg)
//         {
//             _isDataDirtied = true;
//             _locker.Sinal();
//         }
//     }    
//     _locker.Unlock();
// }

void FS_IocpMsgDispatcher::OnDestroy()
{
}

void FS_IocpMsgDispatcher::OnHeartBeatTimeOut()
{

}

void FS_IocpMsgDispatcher::SendData(UInt64 sessionId,  UInt64 consumerId,  NetMsg_DataHeader *msg)
{
    if(_isClose)
        return;

    // sender是一对一的需要找到对应的收到网络包的那个transfer 的id
    auto &senderMq = _netEngine->_GetSenderMq();

    // 新的待发送的消息
    FS_NetMsgBufferBlock *newMsgBlock = new FS_NetMsgBufferBlock;
    newMsgBlock->_mbType = MessageBlockType::MB_NetMsgSended;
    g_MemoryPool->Lock();
    newMsgBlock->_buffer = g_MemoryPool->Alloc<Byte8>(msg->_packetLength);
    g_MemoryPool->Unlock();
    ::memcpy(newMsgBlock->_buffer, msg, msg->_packetLength);
    newMsgBlock->_generatorId = 0;
    newMsgBlock->_sessionId = sessionId;

    // 送入消息队列
    senderMq[consumerId]->PushLock();
    if(!senderMq[consumerId]->Push(newMsgBlock))
        Fs_SafeFree(newMsgBlock);
    senderMq[consumerId]->Notify();
    senderMq[consumerId]->PushUnlock();
}

void FS_IocpMsgDispatcher::_OnBusinessProcessThread(FS_ThreadPool *pool)
{// 业务层可以不用很频繁唤醒，只等待网络层推送消息过来

    // _timeWheel->GetModifiedResolution(_resolutionInterval);
    const auto &resolutionIntervalSlice = _cfgs->_dispatcherResolutionInterval;
    while(pool->IsPoolWorking())
    {
        _messgeQueue->PopLock(_id);
        _messgeQueue->WaitForPoping(_id, _recvMsgBlocks, static_cast<ULong>(resolutionIntervalSlice.GetTotalMilliSeconds()));
        _messgeQueue->PopUnlock(_id);

        // Sleep(100);
        // 先执行定时器事件
        _timeWheel->RotateWheel();

        // 从异步消息队列取得异步处理完成返回事件 TODO: 需要有异步处理队列，其他线程塞入
        // 清理完成的异步事件 TODO:

        // 再执行业务事件
        _OnBusinessProcessing();

        // 投递业务产生的异步处理事件 TODO: 不合适修正，因为可能会错过跨天等重要节点
        // _timeWheel->GetModifiedResolution(_resolutionInterval);
    }

    g_Log->sys<FS_IocpMsgDispatcher>(_LOGFMT_("dispatcher process thread end"));
}

void FS_IocpMsgDispatcher::_OnBusinessProcessing()
{
    // 将网络数据转移到缓冲区
    {
        FS_NetMsgBufferBlock *netMsgBlock = NULL;
        UInt64 sessionId = 0;
        for(auto iterBlock = _recvMsgBlocks->begin(); iterBlock != _recvMsgBlocks->end();)
        {
            netMsgBlock = (*iterBlock)->CastTo<FS_NetMsgBufferBlock>();
            sessionId = netMsgBlock->_sessionId;
            if(netMsgBlock->_mbType == MessageBlockType::MB_NetSessionDisconnect)
            {// 会话断开
                _delayDisconnectedSessions.insert(sessionId);
            }
            else if(netMsgBlock->_mbType == MessageBlockType::MB_NetMsgArrived)
            {// 收到网络包
                auto netMsg = netMsgBlock->CastBufferTo<NetMsg_DataHeader>();
                _DoBusinessProcess(sessionId, netMsgBlock->_generatorId, netMsg);
            }
            else if(netMsgBlock->_mbType == MessageBlockType::MB_NetSessionConnected)
            {// 会话连入
                auto newUser = _logic->OnSessionConnected(sessionId);
                netMsgBlock->_newUserRes->Invoke(newUser);

                auto iterDisconnected = _sessionIdRefUserDisconnected.find(sessionId);
                if(iterDisconnected == _sessionIdRefUserDisconnected.end())
                    iterDisconnected = _sessionIdRefUserDisconnected.insert(std::make_pair(sessionId, std::list<IDelegate<void, IUser *>*>())).first;
                iterDisconnected->second.push_back(netMsgBlock->_userDisconnected);
                netMsgBlock->_userDisconnected = NULL;
            }

            Fs_SafeFree(netMsgBlock);
            iterBlock = _recvMsgBlocks->erase(iterBlock);
        }
    }

    // 延迟断开连接
    for(auto &sessionId : _delayDisconnectedSessions)
        _OnDelaySessionDisconnect(sessionId);
    _delayDisconnectedSessions.clear();
}

void FS_IocpMsgDispatcher::_DoBusinessProcess(UInt64 sessionId, UInt64 generatorId, NetMsg_DataHeader *msgData)
{
    // TODO:处理单一消息业务逻辑部分
    if(_logic)
        _logic->OnMsgDispatch(sessionId, generatorId, msgData);
}

void FS_IocpMsgDispatcher::_OnDelaySessionDisconnect(UInt64 sessionId)
{
    std::list<IDelegate<void, IUser *> *> *delagates = NULL;
    auto iterDisconnected = _sessionIdRefUserDisconnected.find(sessionId);
    if(iterDisconnected != _sessionIdRefUserDisconnected.end())
        delagates = &iterDisconnected->second;

    // TODO:真实的session断开
    if(_logic)
        _logic->OnSessionDisconnected(sessionId, delagates);

    if(delagates)
    {
        if(!delagates->empty())
            STLUtil::DelListContainer(*delagates);

        _sessionIdRefUserDisconnected.erase(iterDisconnected);
    }
}

FS_NAMESPACE_END

#endif
