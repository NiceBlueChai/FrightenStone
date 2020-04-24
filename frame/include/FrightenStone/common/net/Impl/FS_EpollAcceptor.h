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
 * @file  : FS_EpollAcceptor.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/02/01
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_FS_EpollAcceptor_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_FS_EpollAcceptor_H__

#pragma once

#ifndef _WIN32

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/net/Impl/IFS_Acceptor.h"
#include "FrightenStone/common/net/Defs/FS_NetDefs.h"

FS_NAMESPACE_BEGIN

class FS_EpollPoller;
class BASE_EXPORT FS_EpollAcceptor : public IFS_Acceptor
{
    OBJ_POOL_CREATE_DEF(FS_EpollAcceptor);
public:
    FS_EpollAcceptor(UInt32 compId
                    , Locker &sessionLocker
                    ,Int32 &curSessionCnt
                    ,Int32 &maxSessionQuantityLimit
                    ,UInt64 &curMaxSessionId
                    , const AcceptorCfgs &cfg
                    , IFS_NetEngine *netEngine);
    virtual ~FS_EpollAcceptor();

public:    
    virtual Int32 BeforeStart(const NetEngineTotalCfgs &totalCfgs);
    virtual Int32 Start();
    virtual void AfterStart();
    virtual void WillClose();
    virtual void BeforeClose();
    virtual void Close();
    virtual void AfterClose();

    /* TCP ������� */
    #pragma region tcp normal operate
    /*
    * brief:
    *       1. - InitSocket ��ʼ��Socket��
    *       2. - Bind ��IP�Ͷ˿ں�
    *       3. - Listen �����˿ں�
    */
private:
    SOCKET _InitSocket();
    Int32 _Bind(const Byte8 *ip, UInt16 port);
    Int32 _Listen(Int32 unconnectQueueLen = SOMAXCONN);
    void _GetRealIp(const FS_String &cfgIp, const Byte8 *&realIp);
    #pragma endregion

    /* �����¼� */
private:
    virtual void OnSessionDisconnected(UInt64 sessionId);

    /* ���ݳ�Ա */
    #pragma region data member
private:
    // ����
    AcceptorCfgs *_cfgs;
    FS_EpollPoller *_poller;
    // �����˿��׽���
    SOCKET _sock;
    bool _isInit;

    // �ͻ�����������
    Locker &_locker;
    Int32 &_curSessionCnt;
    Int32 &_maxSessionQuantityLimit;
    UInt64 &_curMaxSessionId;
#pragma endregion
};

FS_NAMESPACE_END

#include "FrightenStone/common/net/Impl/FS_EpollAcceptorImpl.h"

#endif

#endif
