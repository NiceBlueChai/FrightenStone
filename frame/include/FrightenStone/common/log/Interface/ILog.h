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
 * @file  : ILog.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/6/12
 * @brief :
 * 
 *
 * 
 */
#ifndef __Frame_Include_FrightenStone_Common_Log_Interface_ILog_H__
#define __Frame_Include_FrightenStone_Common_Log_Interface_ILog_H__
#pragma once

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/component/Impl/FS_Delegate.h"
#include "FrightenStone/common/log/Defs/LogData.h"
#include "FrightenStone/common/assist/utils/Impl/SystemUtil.h"

 // 函数与行号便利宏
#undef _FUNC_LINE_ARGS_
#define _FUNC_LINE_ARGS_ __FUNCTION__, __LINE__

#undef _LOGFMT_
#define _LOGFMT_(x) _FUNC_LINE_ARGS_, x

FS_NAMESPACE_BEGIN

class BASE_EXPORT ILog
{
public:
    static ILog *GetInstance();
    ILog();
    virtual ~ILog();

    virtual Int32 InitModule(const Byte8 *rootDir = NULL) = 0;
    virtual void FinishModule() = 0;
    virtual void FlushAllFile() = 0;

    // json日志

    // 
    // 详细日志details.log
    // 请使用便利宏_LOGFMT_
    template<typename ObjType, typename... Args>
    void i(const char *funcName, Int32 codeLine, const char *fmt, const Args&... args);
    // 请使用便利宏_LOGFMT_
    template<typename ObjType, typename... Args>
    void d(const char *funcName, Int32 codeLine, const char *fmt, const Args&... args);
    // 请使用便利宏_LOGFMT_
    template<typename ObjType, typename... Args>
    void w(const char *funcName, Int32 codeLine, const char *fmt, const Args&... args);
    // 请使用便利宏_LOGFMT_ 会触发断言
    template<typename ObjType, typename... Args>
    void e(const char *funcName, Int32 codeLine, const char *fmt, const Args&... args);

    // crash日志
    // 请使用便利宏_LOGFMT_ 会触发断言
    template<typename... Args>
    void crash(const char *fmt, const Args&... args);
    // 网络日志
    // 请使用便利宏_LOGFMT_ 不支持控制台输出
    template<typename ObjType, typename... Args>
    void net(const char *fmt, const Args&... args);

    // 内存泄漏日志
    // 请使用便利宏_LOGFMT_
    template<typename... Args>
    void memleak(const char *fmt, const Args&... args);
    // 请使用便利宏_LOGFMT_
    template<typename... Args>
    void mempool(const char *fmt, const Args&... args);
    // 请使用便利宏_LOGFMT_
    template<typename... Args>
    void objpool(const char *fmt, const Args&... args);

    // 系统分析日志
    // 请使用便利宏_LOGFMT_
    template<typename ObjType, typename... Args>
    void sys(const char *funcName, Int32 codeLine, const char *fmt, const Args&... args);
    template<typename ObjType, typename... Args>
    void any(const char *fmt, const Args&... args);
    template<typename... Args>
    void custom(const char *fmt, const Args&... args);

    /* 功能函数 */
    // hook与具体类型有关
    template<typename ObjType>
    const IDelegate<void, const LogData *> * InstallLogHookFunc(Int32 level, ObjType *obj, void (ObjType::*func)(const LogData *logData));
    const IDelegate<void, const LogData *> * InstallLogHookFunc(Int32 level, void (*func)(const LogData *logData));
    // hook与具体类型有关
    template<typename ObjType>
    const IDelegate<void, LogData *> * InstallBeforeLogHookFunc(Int32 level, ObjType *obj, void (ObjType::*func)(LogData *logData));
    virtual void UnInstallLogHookFunc(Int32 level, const IDelegate<void, const LogData *> *delegate) = 0;
    const IDelegate<void, LogData *> * InstallBeforeLogHookFunc(Int32 level, void(*func)(LogData *logData));
    virtual void UnInstallBeforeLogHookFunc(Int32 level, const IDelegate<void, LogData *> *delegate) = 0;

    // 创建日志文件调用 ！！！外部不可调用，由系统初始化时候调用
    virtual Int32 CreateLogFile(Int32 fileUnqueIndex, const char *logPath, const char *fileName) = 0;

protected:
    // 与具体类型无关hook
    virtual const IDelegate<void, const LogData *> *_InstallLogHookFunc(Int32 level, IDelegate<void, const LogData *> *delegate) = 0;
    virtual const IDelegate<void, LogData *> *_InstallBeforeLogHookFunc(Int32 level, IDelegate<void, LogData *> *delegate) = 0;
    virtual void _WriteLog(Int32 level, Int32 fileUniqueIndex, LogData *logData) = 0;
};

FS_NAMESPACE_END

#include "FrightenStone/common/log/Interface/ILogImpl.h"

// 需要验证多进程下是否公用一个对象
// extern BASE_EXPORT fs::ILog *g_LogObj;
#define g_Log  (fs::ILog::GetInstance())

#endif
