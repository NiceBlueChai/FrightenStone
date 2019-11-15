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
 * @file  : MessageQueue.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/11/1
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_MessageQueue_Impl_MessageQueue_H__
#define __Base_Common_Component_Impl_MessageQueue_Impl_MessageQueue_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/asyn/asyn.h"
#include "base/common/memorypool/memorypool.h"
#include "base/common/status/status.h"

FS_NAMESPACE_BEGIN

struct BASE_EXPORT FS_MessageBlock;
class BASE_EXPORT FS_ThreadPool;
class BASE_EXPORT ITask;

// ��Ϣ������ѭFIFOԭ�򣬵�һ�����߶Ե�һ������
class BASE_EXPORT MessageQueue
{
    MEM_POOL_CREATE_DEF();

public:
    MessageQueue();
    virtual ~MessageQueue();

public:
    Int32 BeforeStart();
    Int32 Start();
    void BeforeClose();
    void Close();

public:
    // ѹ��ĩ�ڵ�
    void PushLock();
    // msgs�Ѵ���
    bool Push(std::list<FS_MessageBlock *> *&msgs);
    bool Push(FS_MessageBlock *msg);    // ��Ҫ�������notify
    void Notify();
    void PushUnlock();

    // �����̵߳ȴ���Ϣ��������ǰ�ڵ㵯��
    void PopLock();
    // �ɹ����س�ʱWaitEventTimeOut���߳ɹ�Success exportMsgsOut �����ǶѴ���
    Int32 WaitForPoping(std::list<FS_MessageBlock *> *&exportMsgsOut, ULong timeoutMilisec = INFINITE);
    void PopImmediately(std::list<FS_MessageBlock *> *&exportMsgsOut);
    bool IsConsumerInHandling();
    bool HasMsgToConsume();
    bool IsWorking() const;
    void PopUnlock();

private:
    void _MsgQueueWaiterThread(FS_ThreadPool *pool);

private:
    ConditionLocker _msgGeneratorGuard;
    std::atomic_bool _msgGeneratorChange;
    std::list<FS_MessageBlock *> *_msgGeneratorQueue;
    std::list<FS_MessageBlock *> *_msgSwitchQueue;

    ConditionLocker _msgConsumerGuard;
    std::atomic_bool _msgConsumerQueueChange;
    std::list<FS_MessageBlock *> *_msgConsumerQueue;

    std::atomic_bool _isWorking;
    FS_ThreadPool *_pool;
    std::atomic_bool _isStart;
};

// ����ʹ�ö������ߵ�һ������ģ��
// �������߶Զ������߲�������Ϣ���� ����������������������ʱ�����ϵ���Ա����ķ�����Ϣ �����������������
class BASE_EXPORT ConcurrentMessageQueue
{
    MEM_POOL_CREATE_DEF();
public:
    ConcurrentMessageQueue(UInt32 generatorQuantity, UInt32 consumerQuantity = 1);
    ~ConcurrentMessageQueue();

public:
    Int32 BeforeStart();
    Int32 Start();
    void BeforeClose();
    void Close();
    bool IsWorking() const;
    bool IsConsumerInHandling(UInt32 consumerQueueId) const;

public:
    // ѹ��ĩ�ڵ�
    void PushLock(UInt32 generatorQueueId);
    bool Push(UInt32 generatorQueueId, std::list<FS_MessageBlock *> *&msgs);
    bool Push(UInt32 generatorQueueId, FS_MessageBlock *messageBlock);
    void Notify(UInt32 generatorQueueId);
    void PushUnlock(UInt32 generatorQueueId);

    // �����̵߳ȴ���Ϣ��������ǰ�ڵ㵯��
    void PopLock(UInt32 consumerQueueId);
    // �ɹ����س�ʱWaitEventTimeOut���߳ɹ�Success  exportMsgsOut �����ǶѴ���
    Int32 WaitForPoping(UInt32 consumerQueueId, std::list<FS_MessageBlock *> *&exportMsgsOut, ULong timeoutMilisec = INFINITE);
    void NotifyPop(UInt32 consumerQueueId);
    void PopImmediately(UInt32 consumerQueueId, std::list<FS_MessageBlock *> *&exportMsgsOut);
    void PopUnlock(UInt32 consumerQueueId);
    bool HasMsgToConsume(UInt32 consumerQueueId) const;

private:
    void _Generator2ConsumerQueueTask(ITask *task, FS_ThreadPool *pool);

private:
    /* ������ */
    std::vector<ConditionLocker *> _genoratorGuards;
    std::vector<std::atomic_bool *> _generatorChange;
    std::vector<std::list<FS_MessageBlock *> *> _generatorMsgQueues;
    std::vector<std::list<FS_MessageBlock *> *> _msgSwitchQueues;

    /* �����߲��� */
    std::vector<ConditionLocker *>  _consumerGuards;
    std::vector<std::atomic_bool *> _msgConsumerQueueChanges;
    std::vector<std::list<FS_MessageBlock *> *> _consumerMsgQueues;

    /* ϵͳ���� */
    std::atomic<UInt32> _generatorQuantity;
    std::atomic<UInt32> _consumerQuantity;
    std::atomic_bool _isWorking;
    std::atomic_bool _isStart;
    FS_ThreadPool *_pool;
};

// ������϶�Զ�
// ÿ��ͨ���������ⷽʽ����generatorId��consumeridӳ��
// ͨ����Ϣ������������ڵ�generatorId��consumerId����
class RapidMq
{
public:
};

FS_NAMESPACE_END

#include "base/common/component/Impl/MessageQueue/Impl/MessageQueueImpl.h"

#endif
