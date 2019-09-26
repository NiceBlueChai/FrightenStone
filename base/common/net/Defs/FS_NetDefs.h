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
 * @file  : FS_NetDefs.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/08/03
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Defs_FS_NetDefs_H__
#define __Base_Common_Net_Defs_FS_NetDefs_H__
#pragma once


 //SOCKET
#ifdef _WIN32
#undef FD_SETSIZE
#define FD_SETSIZE      1000000
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#else
#ifdef __APPLE__
#define _DARWIN_UNLIMITED_SELECT
#endif // !__APPLE__
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>
#include<signal.h>
#include<sys/socket.h>
#include<netinet/tcp.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 16384    // 16K
#define SEND_BUFF_SZIE 10240    // 10K
#define FS_BUFF_SIZE_DEF 16384  // 16K
#endif // !RECV_BUFF_SZIE


// 客户端心跳检测死亡计时时间(单位ms)
#define CLIENT_HREAT_DEAD_TIME 120000
// 在间隔指定时间后才允许发送(单位ms)
#define CLIENT_SEND_BUFF_TIME 200

// 心跳队列排序
// class FS_ClientComp
// {
// public:
//     bool operator(const FS_Client *l, const FS_Client *r) const;
// };

// 网络模型定义
class NetModuleType
{
public:
    enum
    {
        None = 0,
        Iocp,
        Epoll,
    };
};

#endif
