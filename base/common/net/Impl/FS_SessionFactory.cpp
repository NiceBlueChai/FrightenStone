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
 * @file  : FS_SessionFactory.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/14
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Impl/FS_SessionFactory.h"
#include "base/common/net/Impl/FS_IocpSession.h"
#include "base/common/net/Impl/FS_SessionMgr.h"
#include "base/common/net/Defs/BriefSessionInfo.h"

FS_NAMESPACE_BEGIN

IFS_Session *FS_SessionFactory::Create(UInt64 sessionId, SOCKET sock, const sockaddr_in *addrInfo, IMemoryAlloctor *memAlloctor)
{
#ifdef _WIN32
    return new FS_IocpSession(sessionId, sock, addrInfo, memAlloctor);
#else
#endif
}


IFS_Session *FS_SessionFactory::Create(const BriefSessionInfo &sessionInfo, IMemoryAlloctor *memAlloctor)
{
#ifdef _WIN32
    return new FS_IocpSession(sessionInfo._sessionId, sessionInfo._sock, &(sessionInfo._addrInfo), memAlloctor);
#else
#endif
}

FS_NAMESPACE_END

