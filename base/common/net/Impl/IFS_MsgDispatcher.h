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
 * @file  : IFS_MsgDispatcher.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_IFS_MsgDispatcher_H__
#define __Base_Common_Net_Impl_IFS_MsgDispatcher_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/status/status.h"
#include "base/common/component/Impl/FS_Delegate.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT IFS_Session;
struct BASE_EXPORT NetMsg_DataHeader;
class BASE_EXPORT IFS_MsgTransfer;
class BASE_EXPORT IFS_BusinessLogic;

class BASE_EXPORT IFS_MsgDispatcher
{
public:
    IFS_MsgDispatcher();
    virtual ~IFS_MsgDispatcher();

public:
    virtual Int32 BeforeStart() { return StatusDefs::Success; }
    virtual Int32 Start() = 0;
    virtual Int32 AfterStart() { return StatusDefs::Success; }
    virtual void WillClose() {} // �Ͽ���ģ��֮�������
    virtual void BeforeClose() {}
    virtual void Close() = 0;
    virtual void AfterClose() {}

    virtual void OnRecv(std::list<IFS_Session *> &sessions, Int64 &incPacketsCnt) = 0;
    virtual void OnDisconnected(IFS_Session *session) = 0;
    virtual void OnConnect(UInt64 sessionId, IFS_MsgTransfer *transfer) = 0;
    virtual void OnDestroy() = 0;
    virtual void OnHeartBeatTimeOut() = 0;

    virtual void SendData(UInt64 sessionId, NetMsg_DataHeader *msg) = 0;
    virtual void BindBusinessLogic(IFS_BusinessLogic *businessLogic) = 0;
};

FS_NAMESPACE_END

#endif