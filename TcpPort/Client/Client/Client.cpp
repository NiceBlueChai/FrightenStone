// Client.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "base/exportbase.h"
#include "TcpPort/Client/Impl/InitProcess.h"
#include "TcpPort/Client/Impl/TcpClient.h"

// 初始化环境
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
            printf("退出cmdThread线程\n");
            break;
        }
        else {
            printf("不支持的命令。\n");
        }
    }
}

int main()
{
    const int count = FD_SETSIZE - 1;
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

    //启动UI线程
    std::thread t1(cmdThread);
    t1.detach();

    LoginReq login;
    strcpy(login._userName, "shy");
    strcpy(login._pwd, "shypwd");
    while(g_bRun)
    {
        for(int n = 0; n < count; n++)
        {
            client[n]->SendData(&login);
            client[n]->OnRun();
        }

        //printf("空闲时间处理其它业务..\n");
    }

    for(int n = 0; n < count; n++)
    {
        client[n]->Close();
    }

    printf("已退出。\n");
    getchar();
    return 0;
}

