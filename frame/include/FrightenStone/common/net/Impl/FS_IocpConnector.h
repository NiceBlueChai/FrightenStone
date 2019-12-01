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
 * @file  : FS_IocpConnector.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_FS_IocpConnector_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_FS_IocpConnector_H__
#pragma once

#ifdef _WIN32

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/net/Impl/IFS_Connector.h"
#include "FrightenStone/common/component/Impl/FS_Delegate.h"
#include "FrightenStone/common/component/Impl/FS_ThreadPool.h"
#include "FrightenStone/common/asyn/asyn.h"
#include "FrightenStone/common/component/Impl/FS_String.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT IFS_Session;
class BASE_EXPORT FS_IniFile;
class BASE_EXPORT FS_Iocp;
struct BASE_EXPORT IoDataBase;

class BASE_EXPORT FS_IocpConnector : public IFS_Connector
{
public:
    FS_IocpConnector();
    virtual ~FS_IocpConnector();

public:
    virtual Int32 BeforeStart();
    virtual Int32 Start();
    virtual void BeforeClose();
    virtual void Close();
    virtual void OnDisconnected(IFS_Session *session);
    /* TCP 常规操作 */
    #pragma region tcp normal operate
    /*
    * brief:
    *       1. - InitSocket 初始化Socket等
    *       2. - Bind 绑定IP和端口号
    *       3. - Listen 监听端口号
    */
private:
    SOCKET _InitSocket();
    Int32 _Bind(const Byte8 *ip, UInt16 port);
    Int32 _Listen(Int32 unconnectQueueLen = SOMAXCONN);
    #pragma endregion

    /* 网络事件 */
    #pragma region net event
    /*
    * brief: 
    *       1. FS_Server 4 多个线程触发 不安全 如果只开启1个FS_Server就是安全的
    *       2. _OnNetMonitorTask 监听网络任务 OnRun(旧版) 建议多条线程去做monitor而不是单条线程，完成端口的get是线程安全的
    *       3. OnNetJoin 玩家加入 线程不安全
    *       4. OnNetLeave 玩家掉线 线程不安全
    *       5. OnNetMsg 玩家消息到来（消息是从FS_Server的_HandleNetMsg传入）线程不安全 NetMsg_DataHeader 转发到其他线程需要拷贝避免消息被覆盖
    *       6. OnNetRecv 接收到数据 线程不安全
    */
private:
    void _OnConnected(SOCKET sock, const sockaddr_in *addrInfo, FS_Iocp *iocpListener);
    void _OnDisconnected(IFS_Session *session);
    void _OnIocpMonitorTask(FS_ThreadPool *threadPool);
    void _PreparePostAccept(FS_Iocp *listenIocp, char **&bufArray, IoDataBase **&ioDataArray);
    void _FreePrepareAcceptBuffers(char **&bufArray, IoDataBase **&ioDataArray);
    void _AddToUsedIoDataBaseQueue(IoDataBase *used);
    void _PostAcceptFromUsedDataBaseQuue(FS_Iocp *listenIocp);

    #pragma endregion

    /* 数据成员 */
    #pragma region data member
private:
    // 线程 
    FS_ThreadPool *_threadPool;
    SOCKET _sock;

    // 网络事件回调
    IDelegate<void> *_closeIocpDelegate;

    // 客户端连接上限
    Locker _locker;
    Int32 _curSessionCnt;
    Int32 _maxSessionQuantityLimit;
    UInt64 _curMaxSessionId;
    const UInt64 _maxSessionIdLimit;

    std::list<IoDataBase *> _usedIoQueue;

//     _CrtMemState s1;
//     _CrtMemState s2;
//     _CrtMemState s3;
#pragma endregion
};

#pragma region inline
inline void FS_IocpConnector::_AddToUsedIoDataBaseQueue(IoDataBase *used)
{
    _usedIoQueue.push_back(used);
}
#pragma endregion

FS_NAMESPACE_END

#endif

#endif
