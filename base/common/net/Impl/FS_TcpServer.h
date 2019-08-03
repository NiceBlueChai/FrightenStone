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
 * @file  : FS_TcpServer.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/08/03
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_FS_TcpServer_H__
#define __Base_Common_Net_Impl_FS_TcpServer_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/net/Interface/INetEvent.h"
#include "base/common/component/Impl/Time.h"
#include "base/common/net/Defs/FS_NetDefs.h"
#include "base/common/component/Impl/FS_Delegate.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_Client;
class BASE_EXPORT FS_ThreadPool;

class BASE_EXPORT FS_TcpServer : public INetEvent
{
public:
    FS_TcpServer();
    virtual ~FS_TcpServer();
    virtual void Release();

    /* TCP 常规操作 */
    #pragma region tcp normal operate
    /*
    * brief:
    *       1. - InitSocket 初始化Socket等
    *       2. - Bind 绑定IP和端口号
    *       3. - Listen 监听端口号
    *       4. - Accept 接受客户端连接
    *       5. - Start 根据跨平台的不同会有不同的server实例 这也是设计成使用泛型的一个原因
    *       6. - Close 关闭服务器
    */
public:
    SOCKET InitSocket();
    Int32 Bind(const Byte8 *ip, UInt16 port);
    Int32 Listen(Int32 n);
    SOCKET Accept();
    template<class ServerT>
    void Start(Int32 svrQuantity);
    void Close();
    #pragma endregion

    /* 网络事件 */
    #pragma region net event
    /*
    * brief: 
    *       1. FS_Server 4 多个线程触发 不安全 如果只开启1个FS_Server就是安全的
    *       2. OnNetMonitorTask 监听网络任务 OnRun(旧版) 建议多条线程去做monitor而不是单条线程，完成端口的get是线程安全的
    */
public:
    virtual void OnNetJoin(FS_Client *client);
    virtual void OnNetLeave(FS_Client *client);
    virtual void OnNetMsg(FS_Server *server, FS_Client *client, NetMsg_DataHeader *header);
    virtual void OnNetRecv(FS_Client *client);
    virtual void OnNetMonitorTask(FS_ThreadPool *threadPool) = 0;
    #pragma endregion

    /* 杂项 */
    #pragma region misc
    /*
    * brief:
    *       1. - _GetSocket 获取socket
    *       2. - _AddClientToFSServer 将客户端加入管理
    *       3. - _StatisticsMsgPerSecond 统计并输出每秒收到的网络消息
    */
protected:
    SOCKET _GetSocket();
    void _AddClientToFSServer(FS_Client *client);
    void _StatisticsMsgPerSecond();
    #pragma endregion

    /* 数据成员 */
    #pragma region data member
private:
    // 线程 
    FS_ThreadPool *_threadPool;
    // 消息处理对象，内部会创建线程
    std::vector<FS_Server *> _fsServers;
    // 每秒消息计时
    Time _time;
    SOCKET _sock;

protected:
    // 客户端发送缓冲区大小
    Int32 _sendBuffSize;
    // 客户端接收缓冲区大小
    Int32 _recvBuffSize;
    // 客户端连接上限
    Int32 _maxClient;
    // SOCKET recv计数
    std::atomic<Int32> _recvCount;
    // 收到消息计数
    std::atomic<Int32> _recvMsgCount;
    // 客户端计数
    std::atomic<Int32> _clientAcceptCnt;
    // 已分配客户端计数
    std::atomic<Int32> _clientJoinedCnt;
    #pragma endregion
};


FS_NAMESPACE_END

#include "base/common/net/Impl/FS_TcpServerImpl.h"

#endif
