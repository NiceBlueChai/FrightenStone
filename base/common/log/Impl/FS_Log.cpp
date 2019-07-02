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
 * @file  : FS_Log.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/6/12
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/log/Impl/FS_Log.h"
#include "base/common/assist/utils/utils.h"
#include "base/common/component/Impl/File/LogFile.h"
#include "base/common/status/status.h"
#include "base/common/log/Defs/LogDefs.h"
#include "base/common/log/Defs/LogTask.h"
#include "base/common/assist/utils/Impl/FS_FileUtil.h"
#include "base/common/log/Defs/LogCaches.h"

#pragma region log config options
#define LOG_THREAD_INTERVAL_MS_TIME 1000        // ��־�߳�д��־���ʱ��
#define LOG_SIZE_LIMIT  10240000                // ��־�ߴ�����10MB
// #define LOG_SIZE_LIMIT  -1                   // ������
#define ENABLE_OUTPUT_CONSOLE 1                 // ��������̨��ӡ
#pragma endregion

FS_NAMESPACE_BEGIN

FS_Log::FS_Log(const Byte8 *rootDirName)
    :_rootDirName(rootDirName)
    , _threadPool(NULL)
    , _threadWorkIntervalMsTime(LOG_THREAD_INTERVAL_MS_TIME)
    , _threadWriteLogDelegate(NULL)
    , _logCaches(NULL)
    , _levelRefHook{NULL}
    , _levelRefBeforeLogHook{NULL}
    , _logFiles{NULL}
    ,_logDatas{NULL}
{

}

FS_Log::~FS_Log()
{
    FinishModule();
}

void FS_Log::UnInstallLogHookFunc(Int32 level, const IDelegatePlus<void, const LogData *> *delegate)
{
    if(!_levelRefHook[level])
        return;

    for(auto iterHook = _levelRefHook[level]->begin(); iterHook != _levelRefHook[level]->end(); )
    {
        if(*iterHook == delegate)
        {
            Fs_SafeFree(*iterHook);
            iterHook = _levelRefHook[level]->erase(iterHook);
            continue;
        }

        ++iterHook;
    }
}

void FS_Log::UnInstallBeforeLogHookFunc(Int32 level, const IDelegatePlus<void, LogData *> *delegate)
{
    if(!_levelRefBeforeLogHook[level])
        return;

    for(auto iterHook = _levelRefBeforeLogHook[level]->begin(); iterHook != _levelRefBeforeLogHook[level]->end(); )
    {
        if(*iterHook == delegate)
        {
            Fs_SafeFree(*iterHook);
            iterHook = _levelRefBeforeLogHook[level]->erase(iterHook);
            continue;
        }

        ++iterHook;
    }
}

Int32 FS_Log::InitModule()
{
    if(_isInit)
        return StatusDefs::Success;

    // ��ʼ��log config options
    _ReadConfig();

    // ��ʼ����־�ļ�
    _logCaches = new LogCaches;
    LogDefs::LogInitHelper<LogDefs::LOG_NUM_MAX>::InitLog(this);

    // ��ʼ���̳߳ز����д��־����
    _threadPool = new FS_ThreadPool(1, 1);
    _threadWriteLogDelegate = DelegatePlusFactory::Create(this, &FS_Log::_OnThreadWriteLog);
    ITask *logTask = new LogTask(_threadPool, _threadWriteLogDelegate, _threadWorkIntervalMsTime);
    ASSERT(_threadPool->AddTask(*logTask));

    _isInit = true;
    return StatusDefs::Success;
}

void FS_Log::FinishModule()
{
    if(_isFinish)
        return;

    _isFinish = true;

    // �ر��̳߳�
    _threadPool->Clear();

    // ��������
    Fs_SafeFree(_threadWriteLogDelegate);
    fs::STLUtil::DelArray(_logDatas);
    fs::STLUtil::DelArray(_logFiles);

    // log��hook
    for(Int32 i = 0; i < LogLevel::End; ++i)
    {
        if(!_levelRefHook[i])
            continue;

        for(auto iterHook = _levelRefHook[i]->begin(); iterHook != _levelRefHook[i]->end(); ++iterHook)
            Fs_SafeFree(*iterHook);

        Fs_SafeFree(_levelRefHook[i]);
    }

    // logǰhook
    for(Int32 i = 0; i < LogLevel::End; ++i)
    {
        if(!_levelRefBeforeLogHook[i])
            continue;

        for(auto iterHook = _levelRefBeforeLogHook[i]->begin(); iterHook != _levelRefBeforeLogHook[i]->end(); ++iterHook)
            Fs_SafeFree(*iterHook);

        Fs_SafeFree(_levelRefBeforeLogHook[i]);
    }
    Fs_SafeFree(_logCaches);
    Fs_SafeFree(_threadPool);
}

Int32 FS_Log::CreateLogFile(Int32 fileUnqueIndex, const char *logPath, const char *fileName)
{
    Int32 ret = StatusDefs::Success;
    _locker.Lock();
    do 
    {
        // 1.�Ƿ񴴽���
        auto logFile = _logFiles[fileUnqueIndex];
        if(logFile)
            break;

        // 2.�����ļ���
        FS_String logName = ".\\";
        logName += (_rootDirName + "\\" + logPath);
        if(!FS_DirectoryUtil::CreateDir(logName))
        {
            ret = StatusDefs::Log_CreateDirFail;
            break;
        }

        // 3.ϵͳ�Ƿ��һ�δ���
        logName += fileName;
        bool isFirstCreate = !FS_FileUtil::IsFileExist(logName.c_str());

        // 4.�����ļ�
        logFile = new LogFile;
        if(!logFile->Open(logName.c_str(), true, "ab+", false))
        {
            ret = StatusDefs::Log_CreateLogFileFail;
            ASSERT(!"log file open fail");
            break;
        }

        // ϵͳ�������ݾ���־
        logFile->PartitionFile(isFirstCreate);
        logFile->UpdateLastTimestamp();
        _logFiles[fileUnqueIndex] = logFile;

        // 4.������־����
        _logDatas[fileUnqueIndex] = new std::list<LogData *>;
    } while(0);
    
    _locker.Unlock();

    return ret;
}

void FS_Log::_WriteLog(Int32 level, Int32 fileUniqueIndex, LogData *logData)
{
    // 1.����level��ͬ���ò�ͬ��logǰhook ������Ӱ��logִ�е���������
    if(_levelRefBeforeLogHook[level])
    {
        for(auto iterHook = _levelRefBeforeLogHook[level]->begin();
            iterHook != _levelRefBeforeLogHook[level]->end();
            ++iterHook)
        {
            (*(*iterHook))(std::move(logData));
        }
    }

    // 2.����һ������
    LogData cache = *logData;

    // 3.����־���ݷ������
    _locker.Lock();
    _logDatas[fileUniqueIndex]->push_back(logData);
    _locker.Unlock();

    // 4.��ӡ������̨
#if ENABLE_OUTPUT_CONSOLE
    _OutputToConsole(level, cache._logToWrite);
#endif
    
    // 5.����level��ͬ���ò�ͬ��hook
    if(_levelRefHook[level])
    {
        for(auto iterHook = _levelRefHook[level]->begin();
            iterHook != _levelRefHook[level]->end();
            ++iterHook)
        {
            (*(*iterHook))(&cache);
        }
    }
}

void FS_Log::_OutputToConsole(Int32 level,const FS_String &logStr)
{
    if(level != LogLevel::Net)
    {
        SystemUtil::LockConsole();
        const Int32 oldColor = SystemUtil::GetConsoleColor();
        _SetConsoleColor(level);
        SystemUtil::OutputToConsole(logStr);
        SystemUtil::SetConsoleColor(oldColor);
        SystemUtil::UnlockConsole();
    }
}

void FS_Log::_SetConsoleColor(Int32 level)
{
    switch(level)
    {
        case LogLevel::Debug:
        case LogLevel::Info:
        case LogLevel::Net:
        {
            SystemUtil::SetConsoleColor(FS_ConsoleColor::Fg_White | FS_ConsoleColor::Bg_Black);
        }
        break;
        case LogLevel::Warning:
        {
            SystemUtil::SetConsoleColor(FS_ConsoleColor::Fg_LightYellow | FS_ConsoleColor::Bg_Black);
        }
        break;
        case LogLevel::Error:
        case LogLevel::Crash:
        case LogLevel::Memleak:
        {
            SystemUtil::SetConsoleColor(FS_ConsoleColor::Fg_Red | FS_ConsoleColor::Bg_Black);
        }
        break;
        default:
        {
            SystemUtil::SetConsoleColor(FS_ConsoleColor::Fg_White | FS_ConsoleColor::Bg_Black);
        }
    }
}

void FS_Log::_ReadConfig()
{

}

void FS_Log::_OnThreadWriteLog()
{
    // 1.ת�Ƶ���������ֻ����listָ�룩
    _locker.Lock();
    for(_logCaches->_increasePos = 0, _logCaches->_fileIndex = 0;
        _logCaches->_fileIndex < fs::LogDefs::LOG_QUANTITY; 
        ++_logCaches->_fileIndex)
    {
        if(_logDatas[_logCaches->_fileIndex]->empty())
            continue;

        // ֻ�������ݶ���ָ�뿽�����٣���죬���ҽ�����_logDatas�����ǿյ��൱����������� ��֤�������ǰ��������ΪNULL, ����NULL��ʾ����
        _logCaches->_swapCache = _logCaches->_logDataCache[_logCaches->_increasePos]->_cache;

        _logCaches->_logDataCache[_logCaches->_increasePos]->_cache = _logDatas[_logCaches->_fileIndex];
        _logCaches->_logDataCache[_logCaches->_increasePos]->_fileIndex = _logCaches->_fileIndex;

        _logDatas[_logCaches->_fileIndex] = _logCaches->_swapCache;

        ++_logCaches->_increasePos;
    }
    _locker.Unlock();

    if(_logCaches->_logDataCache[0]->_cache->empty())
        return;

    // 2.д��־
    for(_logCaches->_pos = 0; _logCaches->_pos < fs::LogDefs::LOG_QUANTITY; ++_logCaches->_pos)
    {
        if(_logCaches->_logDataCache[_logCaches->_pos]->_cache->empty())
            break;

        _logCaches->_logFile = _logFiles[_logCaches->_logDataCache[_logCaches->_pos]->_fileIndex];
        for(auto &iterLog : *_logCaches->_logDataCache[_logCaches->_pos]->_cache)
        {
            // �ļ�����ת���������ļ�
            if(_logCaches->_logFile->IsTooLarge(LOG_SIZE_LIMIT))
                _logCaches->_logFile->PartitionFile();

            // д���ļ�
            _logCaches->_logFile->Write(iterLog->_logToWrite.c_str(), iterLog->_logToWrite.GetLength());
            Fs_SafeFree(iterLog);
        }

        _logCaches->_logFile->Flush();
        _logCaches->_logDataCache[_logCaches->_pos]->_cache->clear();
    }
}
FS_NAMESPACE_END

