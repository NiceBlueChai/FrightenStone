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

FS_NAMESPACE_BEGIN

class FS_Client;

class BASE_EXPORT FS_Server
{
public:
    FS_Server();
    virtual ~FS_Server();

#pragma region 
public:
    size_t GetClientCount() const;
    virtual void SetClientNum(Int32 socketNum);
    void SetEventHandleObj(INetEvent *handleObj);
    void SetId(Int32 id);

    // �������� ����ճ�� ��ְ�
    Int32 RecvData(FS_Client *client);
    void AddClient(FS_Client *client);
    void Start();
    void Close();

private:
    // �շ��ͻ�����Ϣ��ת
    void _ClientMsgTransfer(const FS_ThreadPool *pool);
    // �ͻ���״̬�仯������/�д���������/�д��������ݣ�
    virtual bool _OnClientStatusDirtied() = 0;
    void _DetectClientHeartTime();
    void _OnClientLeave(FS_Client *client);
    virtual void _OnClientJoin(FS_Client *client);
    void _OnNetRecv(FS_Client *client);
    void _OnClientMsgTransfer();
    virtual void _HandleNetMsg(FS_Client *client, NetMsg_DataHeader *header);

    void _ClearClients();

protected:
    // ��ʽ�ͻ����� ���������ϸ��հ�����ʱ���������������Ⱥ������������
    std::map<SOCKET, FS_Client *> _socketRefClients;
    std::set<FS_Client *> _clients;      // �ϸ�ʱ�� ʹ��set���ʱ���������Ƴ���O(Log n),��Ϣ�仯ʱ�����Ƴ�������Ա���������

private:
    // ����ͻ�����
    std::vector<FS_Client *> _clientsCache; // �����ͣ�����,�����뻺��
    // ������е���
    Locker _locker;
    // �����¼��������
    INetEvent *_eventHandleObj = nullptr;
    // �ɵ�ʱ���
    Time _oldTime = Time::Now();
    //
    FS_ThreadPool *_threadPool;
protected:
    // ����id
    Int32 _id = -1;
    // �ͻ��б��Ƿ��б仯
    bool _clientsChange = true;
};

FS_NAMESPACE_END

#endif
