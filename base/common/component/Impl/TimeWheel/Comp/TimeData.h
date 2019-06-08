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
 * @file  : TimeWheelData.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/6/6
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_TimeWheel_Comp_TimeData_H__
#define __Base_Common_Component_Impl_TimeWheel_Comp_TimeData_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/component/Impl/TimeSlice.h"
#include <list>

FS_NAMESPACE_BEGIN

class TimeSlice;
class Time;
class TimeData;
class FS_Timer;

class BASE_EXPORT TimeDataLess
{
public:
    bool operator ()(const TimeData *l, const TimeData *r) const;
};

class BASE_EXPORT TimeData
{
public:
    explicit TimeData(FS_Timer *timer);
    virtual ~TimeData();

    FS_String ToString() const;

    // ��ʱʱ��
    Time _expiredTime;
    // �Ƿ�ȡ��
    bool _isCancel;
    // �Ƿ�����ת��ʱ������
    bool _isRotatingWheel;
    // timewheel����Ψһid����timewheel����
    Int64 _timeWheelUniqueId;
    // timer
    FS_Timer *_timer;
    // ��ʱ����
    TimeSlice _period;
};

FS_NAMESPACE_END

#endif
