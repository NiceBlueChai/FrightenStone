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
 * @file  : LogDefs.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/06/26
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Log_Defs_LogDefs_H__
#define __Base_Common_Log_Defs_LogDefs_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/log/Defs/CreateLogFileMacro.h"
#include "base/common/log/Interface/ILog.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT LogDefs
{
public:
    template <int i>
    struct LogInitHelper
    {
        static bool InitLog(ILog *logMgr) { return true; }
    };

    template<int i>
    struct ExcludeCommentHelper
    {
        enum { realvalue = ExcludeCommentHelper<i - 1>::realvalue };
    };

    template<>
    struct ExcludeCommentHelper<0>
    {
        enum { realvalue = 0 };
    };

    // ������־�ļ�
    DECLEAR_GENERAL_LOG_BEGIN()
    GENERAL_ADDLOG(SYSLOG, crash)           // ����
    GENERAL_ADDLOG(SYSLOG, details)         // ������־
    GENERAL_ADDLOG(SYSLOG, memleak)         // �ڴ�й¶
    GENERAL_ADDLOG(SYSLOG, net)             // �����
    GENERAL_ADDLOG(TESTLOG, testcode)       // �������
    DECLEAR_GENERAL_LOG_END()
};


FS_NAMESPACE_END

#include "base/common/log/Defs/LogDefsImpl.h"

#endif