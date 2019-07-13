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
 * @file  : CrashHandleUtil.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/2
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_CrashHandle_Impl_CrashHandleUtil_H__
#define __Base_Common_CrashHandle_Impl_CrashHandleUtil_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"

typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
typedef struct _EXCEPTION_RECORD EXCEPTION_RECORD, *PEXCEPTION_RECORD;
typedef struct _CONTEXT CONTEXT, *PCONTEXT;

#define GET_EXCEPTION_INFO() ((PEXCEPTION_POINTERS)(GetExceptionInformation()))

FS_NAMESPACE_BEGIN

class FS_String;

class BASE_EXPORT CrashHandleUtil
{
public:
    static int InitCrashHandleParams();
    static Int32 RecordExceptionInfo(EXCEPTION_POINTERS exceptionInfo);
    static Int32 InitSymbol();
    static FS_String CaptureStackBackTrace(size_t skipFrames = 0, size_t captureFrames = FS_INFINITE);

};

FS_NAMESPACE_END

#endif
