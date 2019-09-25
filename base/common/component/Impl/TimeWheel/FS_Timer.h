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
 * @file  : FS_Timer.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/06/08
 * @brief : 1. ������ʱ������
 *          2. ���ó�ʱ����cancelִ�к���
 *          3. ���ó�ʱ����
 * 
 */
#ifndef __Base_Common_Component_Impl_TimeWheel_FS_Timer_H__
#define __Base_Common_Component_Impl_TimeWheel_FS_Timer_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/component/Impl/FS_Delegate.h"
#include "base/common/component/Impl/TimeWheel/TimeWheel.h"
#include "base/common/component/Impl/TimeWheel/Comp/TimeData.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_Timer
{
public:
    FS_Timer(TimeWheel *timeWheel = NULL);
    virtual ~FS_Timer();

public:
    // ȡ��
    void Cancel();
    // ����ʱ������
    // void SetTimeWheel(TimeWheel *timeWheel);
    // �Ե�ǰʱ��Ϊ��׼�����ó�ʱ����
    Int32 Schedule(Int64 milliSecPeriod);
    Int32 Schedule(const Time &startTime, Int64 milliSecPeriod);

    // ���ó�ʱִ�к���
    template<typename ObjType>
    void SetTimeOutHandler(ObjType *objType, void (ObjType::*handler)(FS_Timer *, const Time &, const Time &));
    void SetTimeOutHandler(void(*handler)(FS_Timer *, const Time &, const Time &));
    // ����cancelִ�к���
    template<typename ObjType>
    void SetCancelHandler(ObjType *objType, void (ObjType::*handler)(FS_Timer *));
    void SetCancelHandler(void(*handler)(FS_Timer *));

    // ��timewheel���ó�ʱ
    void OnTimeOut(const Time &curWheelTIme);

    FS_String ToString() const;

private:
    Time _lastTimeOutTime;
    TimeWheel *_timeWheel;
    TimeData *_timeData;
    TimeSlice _periodCache;     // scheduleʱʹ�õ����ڻ���
    // ��ʱί�� param1:����ֵ, param2:FS_Timer, param3:lastWheelTime, param4:curTime
    IDelegate<void, FS_Timer *, const Time &, const Time &> *_timeOutDelegate;
    // ע��ί��
    IDelegate<void, FS_Timer *> *_cancelTimerDelegate;
};

FS_NAMESPACE_END

#include "base/common/component/Impl/TimeWheel/FS_TimerImpl.h"

#endif
