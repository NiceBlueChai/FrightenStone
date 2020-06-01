/*!
 * MIT License
 *
 * Copyright (c) 2019 Eric Yonng<120453674@qq.com>
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
 * @file  : AppUtil.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/3/16
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Assist_Utils_Impl_AppUtil_H__
#define __Frame_Include_FrightenStone_Common_Assist_Utils_Impl_AppUtil_H__

#pragma once

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT AppUtil
{
public:
    static Int32 Init();
    static Int32 Start(UInt64 maxAllowObjPoolBytesOccupied = 16777216     // ���Ķ����ռ���ֽ��� Ĭ�� 16MB = 16777216bytes
                       , UInt64 maxAllowMemPoolBytesOccupied = 16777216    // �����ڴ��ռ���ֽ��� Ĭ�� 16MB = 16777216bytes
                       , bool enableMemMonitor = true                  // �Ƿ���������/�ڴ�ص��ڴ�����־
                       , UInt32 memMonitorPrintLogIntervalSeconds = 5   // �ڴ�����־ʱ������  Ĭ��5s
    );
    static void Wait();
    static void Finish();
};

FS_NAMESPACE_END

#endif
