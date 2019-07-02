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
 * @file  : FS_Log.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/6/12
 * @brief :
 *          1.�ṩд��־�ӿڣ��������������
 *          2.��Ŀ¼��־ crash, system, netlayerlog��Э����Ϣ��ҵ�����־��
 *          3.�����ļ���С���ļ�
 *          4.������������ȼ���:warning,error,debug,info��
 *          5.֧��json log ���
 *                              {
                                    time:(��ȷ��΢��)fmt:1970-01-01 00:00:00.123456
                                    class name:
                                    function:
                                    line:
                                    level:
                                    content:
                                    {
                                        op:
                                        status:statusCode
                                        ecxceptioninfo:
                                        stackinfo:
                                    }
                                }
            6.��־�ǵ��߳���־
 */
#ifndef __Base_Common_Log_Impl_Log_H__
#define __Base_Common_Log_Impl_Log_H__
#pragma once

#include "base/common/log/Interface/ILog.h"
#include "base/common/log/Defs/LogData.h"
#include "base/common/component/Impl/FS_Delegate.h"
#include "base/common/asyn/asyn.h"
#include "base/common/log//Defs/LogDefs.h"

FS_NAMESPACE_BEGIN

class FS_ThreadPool;
class LogFile;
class LogCaches;

class BASE_EXPORT FS_Log : public ILog
{
public:
    FS_Log(const Byte8 *rootDirName);
    virtual ~FS_Log();

    /* ���ܺ��� */
    virtual void UnInstallLogHookFunc(Int32 level, const IDelegatePlus<void, const LogData *> *delegate);
    virtual void UnInstallBeforeLogHookFunc(Int32 level, const IDelegatePlus<void, LogData *> *delegate);
    Int32 InitModule();
    virtual void FinishModule();
    virtual Int32 CreateLogFile(Int32 fileUnqueIndex, const char *logPath, const char *fileName);
    
protected:
    /* ��־hook */
    virtual IDelegatePlus<void, const LogData *> * _InstallLogHookFunc(Int32 level, IDelegatePlus<void, const LogData *> *delegate);    // �����delegate
    virtual const IDelegatePlus<void, LogData *> *_InstallBeforeLogHookFunc(Int32 level, IDelegatePlus<void, LogData *> *delegate);
    // ��־���
    virtual void _WriteLog(Int32 level, Int32 fileUniqueIndex, LogData *logData);

    // ����̨���
    void _OutputToConsole(Int32 level, const FS_String &logStr);
    void _SetConsoleColor(Int32 level);

    // ����
    void _ReadConfig();

    // �̲߳���
private:
    void _OnThreadWriteLog();

private:
    std::atomic_bool _isInit{false};
    std::atomic_bool _isFinish{false};

    FS_ThreadPool *_threadPool;                                                 // �̳߳�
    FS_String _rootDirName;                                                     // ������
    std::list<IDelegatePlus<void, const LogData *> *> *_levelRefHook[LogLevel::End];         // ��־�����Ӧ��hook
    std::list<IDelegatePlus<void, LogData *> *> *_levelRefBeforeLogHook[LogLevel::End];         // д��־ǰ��־�����Ӧ��hook �����޸���־���ݵ�
    Int32 _threadWorkIntervalMsTime;                                            // ��־�̹߳������ʱ��

    /* ��־�ļ����� */
    ConditionLocker _locker;                                                // ��
    LogFile *_logFiles[LogDefs::LOG_QUANTITY];                              // ��־id��־�ļ� �������ļ�ֻ�������������ɾ��
    std::list<LogData *> *_logDatas[LogDefs::LOG_QUANTITY];                 // ��־id��־����
    IDelegatePlus<void> *_threadWriteLogDelegate;                           // ��־�߳�д��־ί��

    LogCaches *_logCaches;                                                  // ����������������
};

FS_NAMESPACE_END

#include "base/common/log/Impl/FS_LogImpl.h"

#endif
