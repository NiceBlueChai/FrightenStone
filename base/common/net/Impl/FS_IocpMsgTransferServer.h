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
 * @file  : FS_IocpMsgTransferServer.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/5
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_FS_IocpMsgTransferServer_H__
#define __Base_Common_Net_Impl_FS_IocpMsgTransferServer_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/net/Impl/FS_Server.h"
#include "base/common/objpool/objpool.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_Iocp;
struct BASE_EXPORT IO_EVENT;

 // ������Ϣת�����������
class BASE_EXPORT FS_IocpMsgTransferServer :public FS_Server
{
    OBJ_POOL_CREATE(FS_IocpMsgTransferServer, _objPoolHelper);
public:
    FS_IocpMsgTransferServer();
    virtual ~FS_IocpMsgTransferServer() noexcept;

public:
    // �ڹرշ���ǰ��Щ��������˳�iocp
    virtual void BeforeClose();

    /* ������Ϣ���� */
    #pragma region net msg handle
    /*
    *   brief:
    *       1. _BeforeClientMsgTransfer - �ڽ��ͻ�����Ϣת����ҵ���߼�����֮ǰ���� ���������Կͻ�����Ϣֱ����ʱ���ߴ���
    *       2. _ListenIocpNetEvents - �������Կͻ��˵�������Ϣֱ�� ÿ��ֻ����һ�������¼�
    *       3. _RmClient - �Ƴ��ͻ���
    *       4. _RmClient - �Ƴ��ͻ���
    *       5. _OnClientJoin - �ͻ������봦��
    */
protected:
    virtual Int32 _BeforeClientMsgTransfer();
    Int32 _ListenIocpNetEvents();
    void _RmClient(FS_Client *client);
    void _RmClient(IO_EVENT &ioEvent);
    virtual void _OnClientJoin(FS_Client *client);
    #pragma endregion

private:
    FS_Iocp *_iocpClientMsgTransfer;        // ֻ�շ��ͻ�����Ϣ������������
    IO_EVENT *_ioEvent;
};

FS_NAMESPACE_END

#include "base/common/net/Impl/FS_IocpMsgTransferServerImpl.h"

#endif

