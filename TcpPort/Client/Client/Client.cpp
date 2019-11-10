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
 * @file  : Client.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/exportbase.h"
#include "TcpPort/Client/Impl/InitProcess.h"
#include "TcpPort/Client/Impl/TcpClient.h"
#include "base/common/net/Defs/FS_NetDefs.h"

#ifndef CLIENT_QUANTITY_LIMIT
#define CLIENT_QUANTITY_LIMIT 100000
#endif

InitProcess g_InitProcess;

bool g_bRun = true;
void cmdThread()
{
    while(true)
    {
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if(0 == strcmp(cmdBuf, "exit"))
        {
            g_bRun = false;
            printf("exit thread\n");
            break;
        }
        else {
            printf("not surport。\n");
        }
    }
}

int main()
{
    const int count = CLIENT_QUANTITY_LIMIT - 1;
    TcpClient *client[count] = {NULL};

    for(int n = 0; n < count; n++)
    {
        client[n] = new TcpClient();
        client[n]->InitSocket();
    }
    for(int n = 0; n < count; n++)
    {
        client[n]->Connect("47.96.98.206", 4567);
        //client[n]->Connect("127.0.0.1", 4567);
    }

    std::thread t1(cmdThread);
    t1.detach();

    fs::LoginReq login;
    strcpy(login._userName, "shy");
    strcpy(login._pwd, "shypwd");
    while(g_bRun)
    {
        for(int n = 0; n < count; n++)
        {
            client[n]->SendData(&login);
            client[n]->OnRun();
        }

    }

    for(int n = 0; n < count; n++)
    {
        client[n]->Close();
    }

    printf("exit.\n");
    getchar();
    return 0;
}

