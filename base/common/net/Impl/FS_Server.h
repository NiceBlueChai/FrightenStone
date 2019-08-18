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
 * @file  : FS_Server.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/3
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_FS_Server_H__
#define __Base_Common_Net_Impl_FS_Server_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/asyn/asyn.h"
#include "base/common/net/Interface/INetEvent.h"
#include "base/common/component/Impl/Time.h"
#include "base/common/component/Impl/FS_ThreadPool.h"
#include "base/common/net/Defs/HeartBeatComp.h"

FS_NAMESPACE_BEGIN

// ������
class BASE_EXPORT FS_Server
{
    OBJ_POOL_CREATE(FS_Server, _objPoolHelper);
public:
    FS_Server();
    virtual ~FS_Server();

    /* ���� */
    #pragma region misc
    /*
    *   brief:
    *       1. - GetClientCount �ͻ�������
    *       2. - SetClientNum ����������Ŀͻ�������
    *       3. - SetEventHandleObj ����������Ϣ�������
    *       4. - SetId ���÷���id
    *       5. - _ClearClients ����ͻ���
    */
public:
    size_t GetClientCount() const;
    virtual void SetClientNum(Int32 socketNum);
    void SetEventHandleObj(INetEvent *handleObj);
    void SetId(Int32 id);
private:
    void _ClearClients();
    #pragma endregion

    /* ��������/��ӿͻ���/��������/ֹͣ���� */
    #pragma region recv/addclient/start/close
public:
    // �������� ����ճ�� ��ְ�
    Int32 RecvData(FS_Client *client);
    void AddClient(FS_Client *client);
    void Start();
    virtual void BeforeClose();
    void Close();
    #pragma endregion

    /* ������Ϣ���� */
    #pragma region net message handle
    /*
    *   brief:
    *       1. - _ClientMsgTransfer �շ��ͻ�����Ϣ��ת
    *       2. - _OnClientStatusDirtied �ͻ���״̬�仯������/�д���������/�д��������ݣ�
    *       3. - _DetectClientHeartTime �ͻ���������⣬TODO:��Ҫ�Ż�
    *       4. - _OnClientLeave �ͻ��˶Ͽ�
    *       5. - _OnClientJoin �ͻ������� iocp��epoll�в�ͬ����д����
    *       6. - _OnNetRecv ����������Ϣ
    *       7. - _OnClientMsgTransfer ������Ϣ�ٴ���ת
    *       8. - _HandleNetMsg ������Ϣ���� <��������Ϣ���> 
    *               NetMsg_DataHeader �ǻ������е����ݣ�
    *               ��Ҫת���������ط���Ҫ���п�������ᱻ��������
    */
protected:
    void _ClientMsgTransfer(const FS_ThreadPool *pool);
    virtual Int32 _BeforeClientMsgTransfer(std::set<SOCKET> &delayDestroyClients) = 0;
    
    // TODO:�����Ż�
    void _DetectClientHeartTime();
    void _AddToHeartBeatQueue(FS_Client *client);
    void _OnClientHeartBeatUpdate(FS_Client *client);

    void _RmClient(FS_Client *client);
    void _OnClientLeave(FS_Client *client);
    virtual void _OnClientJoin(FS_Client *client);
    void _OnPrepareNetRecv(FS_Client *client);

    void _OnClientMsgArrived();
    virtual Int32 _HandleNetMsg(FS_Client *client, NetMsg_DataHeader *header);
    #pragma endregion

    /* ���ݳ�Ա */
    #pragma region data member
protected:
    // ��ʽ�ͻ����� ���������ϸ��հ�����ʱ���������������Ⱥ������������
    std::map<SOCKET, FS_Client *> _socketRefClients;
    std::set<FS_Client *, HeartBeatComp> _clientHeartBeatQueue;

private:
    // ����ͻ�����
    std::vector<FS_Client *> _clientsCache; // �����ͣ�����,�����뻺��
    // ������е���
    Locker _locker;
    // �����¼��������
    INetEvent *_eventHandleObj = NULL;    // �ڲ����ͷ�
    // �ɵ�ʱ���
    Time _lastHeartDetectTime = Time::Now();
    // �߳�
    FS_ThreadPool *_threadPool;
    std::set<SOCKET> _delayRemoveClients;
protected:
    // ����id
    Int32 _id = -1;
    // �ͻ��б��Ƿ��б仯
    bool _clientsChange = true;
    Int32 _clientJoin = 0;
    #pragma endregion
};

FS_NAMESPACE_END
#include "base/common/net/Impl/FS_ServerImpl.h"

#endif
