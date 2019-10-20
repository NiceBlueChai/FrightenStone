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
 * @file  : SocketUtil.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Socket_Impl_SocketUtil_H__
#define __Base_Common_Socket_Impl_SocketUtil_H__

#pragma once

#include <base/exportbase.h>
#include "base/common/basedefs/Macro/MacroDefs.h"
#include "base/common/basedefs/DataType/DataType.h"
#include "base/common/net/net.h"
#include "base/common/socket/Defs/SocketDefs.h"

// struct sockaddr_in;

FS_NAMESPACE_BEGIN

class BASE_EXPORT SocketUtil
{
public:
    static int InitSocketEnv();
    static int ClearSocketEnv();
    static Int32 SetNoBlock(MYSOCKET socket);
    static Int32 SetBlock(MYSOCKET socket);
    static Int32 MakeReUseAddr(MYSOCKET socket);
    static Int32 MakeNoDelay(MYSOCKET socket);
    static Int32 DestroySocket(MYSOCKET &socket);
    static Int32 DestroySocket2(MYSOCKET socket);

    // ��ȡ�����ַ��Ϣ 0��ʾ�ɹ�����ֵΪ������Ϣ ��ȡԶ�̿ͻ��˵�ַ��Ϣ ��Ҫ��accept�ɹ���ɺ�ſ��Ե���
    static Int32 GetPeerAddr(UInt64 sSocket, Int32 sizeIp, Byte8 *&ip, UInt16 &port, Int32 &lastError);
    // ת��Ϊ�����ֽ���
    static bool FillTcpAddrInfo(const char *ip, UInt16 port, UInt16 family, sockaddr_in &addrObj);
    // ת��Ϊ������Ϣ
    static bool GetAddrInfoFromNetInfo(const sockaddr_in &addrObj, UInt64 szip, char *&ip, UInt16 &port);
    // �׽��ֵȴ���ʱ
    static bool IsDetectTimeOut(
        SOCKET &socket
        , fd_set &readableSet
        , fd_set &writableSet
        , long tv_sec
        , long tv_usec
        , bool enableReadableDetect = true
        , bool enableWriteableDetect = false
        , int *errOut = NULL
        , bool setOneAtLeast = false
        , bool isInfiniteWaiting = false);

    static void Sleep(UInt64 milliSec, UInt64 microSec = 0);

    // ����socket��������С
    static Int32 SetSocketCacheSize(SOCKET &socket, SocketDefs::SOCKET_CACHE_TYPE eType, Int64 cacheSize);

    // ��ȡsocket��������С
    static Int32 GetSocketCacheSize(SOCKET &socket, SocketDefs::SOCKET_CACHE_TYPE eType, Int64 &cacheSize);

    static FS_String ToFmtSpeedPerSec(Int64 bytesps);

private:
    static bool _isInitEnv;
};

FS_NAMESPACE_END

#endif // !__Base_Common_Socket_Impl_SocketUtil_H__
