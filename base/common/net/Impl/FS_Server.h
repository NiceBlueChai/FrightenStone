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

FS_NAMESPACE_BEGIN

class FS_Client;

class BASE_EXPORT FS_Server
{
public:
    virtual ~FS_Server();
    void SetId(Int32 id);

    virtual void SetClientNum(Int32 socketNum);
    void SetEventHandleObj(INetEvent *handleObj);

    //�ر�Socket
    void Close();
    {
        CELLLog_Info("CELLServer%d.Close begin", _id);
        _taskServer.Close();
        _thread.Close();
        CELLLog_Info("CELLServer%d.Close end", _id);
    }

    //����������Ϣ
    void OnRun(CELLThread* pThread)
    {
        while(pThread->isRun())
        {
            if(!_clientsBuff.empty())
            {//�ӻ��������ȡ���ͻ�����
                std::lock_guard<std::mutex> lock(_mutex);
                for(auto pClient : _clientsBuff)
                {
                    _clients[pClient->sockfd()] = pClient;
                    pClient->serverId = _id;
                    if(_pEventHandleObj)
                        _pEventHandleObj->OnNetJoin(pClient);
                    OnClientJoin(pClient);
                }
                _clientsBuff.clear();
                _clients_change = true;
            }

            //���û����Ҫ����Ŀͻ��ˣ�������
            if(_clients.empty())
            {
                CELLThread::Sleep(1);
                //�ɵ�ʱ���
                _oldTime = CELLTime::getNowInMilliSec();
                continue;
            }

            CheckTime();
            if(!DoNetEvents())
            {
                pThread->Exit();
                break;
            }
            DoMsg();
        }
        CELLLog_Info("CELLServer%d.OnRun exit", _id);
    }

    virtual bool DoNetEvents() = 0;

    void CheckTime()
    {
        //��ǰʱ���
        auto nowTime = CELLTime::getNowInMilliSec();
        auto dt = nowTime - _oldTime;
        _oldTime = nowTime;

        CELLClient* pClient = nullptr;
        for(auto iter = _clients.begin(); iter != _clients.end(); )
        {
            pClient = iter->second;
            //�������
            if(pClient->checkHeart(dt))
            {
#ifdef CELL_USE_IOCP
                if(pClient->isPostIoAction())
                    pClient->destory();
                else
                    OnClientLeave(pClient);
#else
                OnClientLeave(pClient);
#endif // CELL_USE_IOCP
                iter = _clients.erase(iter);
                continue;
            }

            ////��ʱ���ͼ��
            //pClient->checkSend(dt);

            iter++;
        }
    }

    void OnClientLeave(CELLClient* pClient)
    {
        if(_pEventHandleObj)
            _pEventHandleObj->OnNetLeave(pClient);
        _clients_change = true;
        delete pClient;
    }

    virtual void OnClientJoin(CELLClient* pClient)
    {

    }

    void OnNetRecv(CELLClient* pClient)
    {
        if(_pEventHandleObj)
            _pEventHandleObj->OnNetRecv(pClient);
    }

    void DoMsg()
    {
        CELLClient* pClient = nullptr;
        for(auto itr : _clients)
        {
            pClient = itr.second;
            //ѭ�� �ж��Ƿ�����Ϣ��Ҫ����
            while(pClient->hasMsg())
            {
                //����������Ϣ
                OnNetMsg(pClient, pClient->front_msg());
                //�Ƴ���Ϣ���У�����������ǰ��һ������
                pClient->pop_front_msg();
            }
        }
    }

    //�������� ����ճ�� ��ְ�
    int RecvData(CELLClient* pClient)
    {
        //���տͻ�������
        int nLen = pClient->RecvData();
        //����<���յ���������>�¼�
        if(_pEventHandleObj)
            _pEventHandleObj->OnNetRecv(pClient);
        return nLen;
    }

    //��Ӧ������Ϣ
    virtual void OnNetMsg(CELLClient* pClient, netmsg_DataHeader* header)
    {
        if(_pEventHandleObj)
            _pEventHandleObj->OnNetMsg(this, pClient, header);
    }

    void addClient(CELLClient* pClient)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        //_mutex.lock();
        _clientsBuff.push_back(pClient);
        //_mutex.unlock();
    }

    void Start()
    {
        _taskServer.Start();
        _thread.Start(
            //onCreate
            nullptr,
            //onRun
            [this](CELLThread* pThread) {
            OnRun(pThread);
        },
            //onDestory
            [this](CELLThread* pThread) {
            ClearClients();
        }
        );
    }

    size_t getClientCount()
    {
        return _clients.size() + _clientsBuff.size();
    }

    //void addSendTask(CELLClient* pClient, netmsg_DataHeader* header)
    //{
    //	_taskServer.addTask([pClient, header]() {
    //		pClient->SendData(header);
    //		delete header;
    //	});
    //}
private:
    void ClearClients()
    {
        for(auto iter : _clients)
        {
            delete iter.second;
        }
        _clients.clear();

        for(auto iter : _clientsBuff)
        {
            delete iter;
        }
        _clientsBuff.clear();
    }

protected:
    // ��ʽ�ͻ�����
    std::map<SOCKET, FS_Client *> _clients;
private:
    // ����ͻ�����
    std::vector<FS_Client *> _clientsBuff;
    // ������е���
    Locker _locker;
    // �����¼��������
    INetEvent* _pEventHandleObj = nullptr;
    // �ɵ�ʱ���
    Time _oldTime = Time::Now();
    //
    CELLThread _thread;
protected:
    //
    Int32 _id = -1;
    //�ͻ��б��Ƿ��б仯
    bool _clients_change = true;
};

FS_NAMESPACE_END

#endif
