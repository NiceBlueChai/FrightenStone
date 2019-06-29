#ifndef __Test_TestLogModule_H__
#define __Test_TestLogModule_H__
#pragma once
#include "stdafx.h"

class TestLogModule
{
public:
    static void Run()
    {
        fs::TimeUtil::SetTimeZone();
        auto logMgr = fs::ILog::InitModule("main");
        fs::Time nowTime, nowTime2;
        nowTime.FlushTime();
//         logMgr->w<TestLogModule>(_LOGFMT_("hello world%s"), "wocao");
//         logMgr->e<TestLogModule>(_LOGFMT_("hello world%s"), "wocao");
//         logMgr->crash<TestLogModule>(_LOGFMT_("hello world%s"), "wocao");
//         logMgr->memleak<TestLogModule>(_LOGFMT_("hello world%s"), "wocao");
//         logMgr->net<TestLogModule>(_LOGFMT_("hello world%s"), "wocao");
        for(Int32 i = 0; i < 1000000; ++i)
            logMgr->w<TestLogModule>(_LOGFMT_("hello world%s"), "wocao");
        nowTime2.FlushTime();
        std::cout << (nowTime2 - nowTime).GetTotalMilliSeconds() << std::endl;
        logMgr->FinishModule();
        Fs_SafeFree(logMgr);

//         std::cout << "-----------------------------------------" << std::endl;
//         fs::Time nowTime, nowTime2, nowTime3;
//         fs::FS_String cache;
//         // 1. log �Ż����飺����format tostring ���ƵĴ���,�������ĺܴ�
//         // 
//         fs::FS_String str = "2019-06-27 22:13:10.442468<Warn>[class TestLogModule][TestLogModule::Run][line:17]: hello world\n";
//         auto fp = fs::FS_FileUtil::OpenFile(".\\test.log", true);
//         nowTime3.FlushTime();
//         nowTime.FlushTime();
//         for(Int32 i = 0; i < 1000000; ++i)
//         {
//             fs::FS_FileUtil::WriteFile(*fp, str);
//         }
//         fs::FS_FileUtil::FlushFile(*fp);
//         nowTime2.FlushTime();
//         std::cout << (nowTime2 - nowTime).GetTotalMilliSeconds() << std::endl;
    }
};
#endif
