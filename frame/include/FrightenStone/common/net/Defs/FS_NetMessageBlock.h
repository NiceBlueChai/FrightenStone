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
 * @file  : FS_NetMessageBlock.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/12/29
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Defs_FS_NetMessageBlock_H__
#define __Frame_Include_FrightenStone_Common_Net_Defs_FS_NetMessageBlock_H__

#pragma once

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/component/Impl/MessageQueue/MessageQueue.h"
#include "FrightenStone/common/net/Defs/IoEvDefs.h"
#include "FrightenStone/common/status/status.h"
#include "FrightenStone/common/component/Impl/FS_Delegate.h"

#ifdef _WIN32
// #include "ws2def.h"
// #include "winsock2.h"
#endif

FS_NAMESPACE_BEGIN

class IUser;

class BASE_EXPORT NetMessageBlockType
{
public:
    enum
    {
        Net_None = 0,                    // ��Ч
        Net_NetMsgArrived = 1,           // �յ������ Transfer to Dispatcher
        Net_NetMsgSended = 2,            // ��������� Dispatcher to Transfer
        Net_NetSessionDisconnect = 3,    // �Ự�Ͽ� Transfer to Dispatcher
        Met_NetSessionConnected = 4,     // �Ự���� Transfer to Dispatcher
        Met_NetCloseSession = 5,         // �رջỰ Dispatcher to Transfer
    };
};

struct BASE_EXPORT FS_NetMsgBufferBlock : public FS_MessageBlock
{
    FS_NetMsgBufferBlock();
    ~FS_NetMsgBufferBlock();

    UInt32 _compId;                     // ���id
    UInt32 _generatorId;                // ������id
    Int32 _netMessageBlockType;         // NetMessageBlockType


//     Byte8 *_buffer; // NetMsg_DataHeader *
//     UInt64 _sessionId;
// 
//     /* ����ʱ��Ķ���ص����� _mbType==MB_NetSessionConnected */
//     IDelegate<void, IUser *> *_newUserRes;
//     IDelegate<void, IUser *> *_userDisconnected;
};

// �Ự��Ϣ����
struct BASE_EXPORT FS_NetArrivedMsg : public FS_NetMsgBufferBlock
{
    FS_NetArrivedMsg();
    ~FS_NetArrivedMsg();

    IoEvent *_ioEv;                     // �ڴ�ش���
    Int32 _errorCode;                   // ���������
};

// �Ự��Ϣ����
struct BASE_EXPORT FS_NetSessionWillConnectMsg : public FS_NetMsgBufferBlock
{
    FS_NetSessionWillConnectMsg();
    ~FS_NetSessionWillConnectMsg();

    UInt64 _sessionId;
    SOCKET _sock;
    sockaddr_in _addrInfo;

    /* �������Ķ������ */
    IDelegate<void, IUser *> *_onNewUserRes;
    IDelegate<void, IUser *> *_onUserDisconnectedRes;
};

FS_NAMESPACE_END

#include "FrightenStone/common/net/Defs/FS_NetMessageBlockImpl.h"

#endif
