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
 * @file  : TestEpoll.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/3/16
 * @brief :
 */

#include "stdafx.h"
#include "TestInst/TestEpoll/TestEpoll.h"

#define TEST_EPOLL_PORT 4567

#ifndef _WIN32
void TestEpoll::Run()
{
    //fs::FS_Epoll epollEv;

    // 1.app��ʼ��
    auto st = fs::AppUtil::Init();
    if(st != StatusDefs::Success)
        return;

    // 2.����
    st = fs::AppUtil::Start();
    if(st != StatusDefs::Success)
    {
        fs::AppUtil::Finish();
        return;
    }

    do
    {

        // ����epoll
        fs::FS_Epoll *ev = new fs::FS_Epoll;
        auto st = ev->Create();
        if(st != StatusDefs::Success)
        {
            g_Log->e<TestEpoll>(_LOGFMT_("create ev fail st[%d]"), st);
            break;
        }

        // �����׽���
        auto listenSock = fs::SocketUtil::CreateSock();
        if(listenSock == INVALID_SOCKET)
        {
            g_Log->e<TestEpoll>(_LOGFMT_("create listen socket fail"));
            break;
        }

        // ��Ҫ��ַ
        fs::BriefAddrInfo addrInfo;
        addrInfo._port = TEST_EPOLL_PORT;

        // ��
        st = fs::SocketUtil::Bind(listenSock, addrInfo);
        if(st != StatusDefs::Success)
        {
            g_Log->e<TestEpoll>(_LOGFMT_("bind listensock[%d] fail st[%d]"), listenSock, st);
            break;
        }

        // ����
        st = fs::SocketUtil::Listen(listenSock);
        if(st != StatusDefs::Success)
        {
            g_Log->e<TestEpoll>(_LOGFMT_("listen sock[%d] fail st[%d]"), listenSock, st);
            break;
        }

        st = ev->AddEvent(listenSock, __ADD_FS_EPOLL_ACCEPT_EVENT_FLAGS_DEF__);
        if(st != StatusDefs::Success)
        {
            g_Log->e<TestEpoll>(_LOGFMT_("add event fail st[%d]"), st);
            break;
        }

        g_Log->custom("prepare success...");
        while(true)
        {
            auto ret = ev->Wait();
            if(ret <= 0)
                continue;

            g_Log->custom("new connect coming ret[%d]", ret);
            auto evs = ev->GetEvs();
            for(Int32 i = 0; i < ret; ++i)
            {
                if(evs[i].data.fd == listenSock)
                {
                    do
                    {
                        // accept ֱ��eagain
                        struct sockaddr_in inAddr;
                        socklen_t addrSize = static_cast<socklen_t>(sizeof(inAddr));
                        auto newSock = ::accept(listenSock, reinterpret_cast<sockaddr *>(&inAddr), &addrSize);
                        if(newSock > 0)
                        {
                            // ��������
                            g_Log->custom("accept suc newSock[%d]", newSock);
                            ::close(newSock);
                        }
                        else
                        {
                            if(errno == EAGAIN || errno == EWOULDBLOCK)
                            {
                                g_Log->custom("EAGAIN or EWOULDBLOCK coming ");
                                break;
                            }
                        }
                    } while(true);
                }
            }

            g_Log->custom("handled evs");
        }
    } while(0);

    // ����
    fs::AppUtil::Finish();


    // 1.��ʼ��
    // 1.�����׽���
    // 2.��
    // 3.listen����
    // 4.����epoll����
    // 5.epoll wait
    // 6.�ж��Ƿ�������
    // 7.accept
    // 8.����epoll����
    // ...
    // �˳������׽���
    // ����epoll��Դ
}
#endif

