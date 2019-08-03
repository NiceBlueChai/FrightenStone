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
 * @file  : TimeWeel.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/6/6
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_TimeWheel_TimeWheel_H__
#define __Base_Common_Component_Impl_TimeWheel_TimeWheel_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/component/Impl/Time.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/component/Impl/TimeSlice.h"
#include "base/common/component/Impl/TimeWheel/Comp/TimeData.h"
#include <set>
#include "base/common/objpool/objpool.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT TimeWheel
{
    OBJ_POOL_CREATE(fs::TimeWheel, _objPoolHelper);
public:
    explicit TimeWheel(const TimeSlice &resolutionSlice);
    virtual ~TimeWheel();

    // ת��ʱ������
    void RotateWheel();

    // ע�ᳬʱ����
    Int32 Register(TimeData *timeData);
    // ȡ��ע��
    void UnRegister(TimeData *timeData);
    // ��ǰʱ�����̾��� @return:����΢��
     const TimeSlice &GetTimeWheelResolution() const;
     // ��һ֡����ɨ��ʱ������ֵ
     void GetModifiedResolution(TimeSlice &modifiedResolution) const;
     // ��ǰʱ��
     const Time &GetCurTime() const;

private:
    // ����timewheel����������Ψһid
    Int64 _NewIncreasId();

private:
    const TimeSlice _resolutionSlice;       // ÿһ��ʱ����Ƭ����
    // Time _nowTimeToJudgeWheelTimeOut;   // ��ǰʱ�������ж��Ƿ�ﵽ���̴������Сɨ��ʱ��
    Time _curTime;                  // ��ǰʱ���
    std::set<TimeData *, TimeDataLess> _timeDatas;

    Int64 _increaseId;              // ����id
};

FS_NAMESPACE_END

#include "base/common/component/Impl/TimeWheel/TimeWheelImpl.h"

#endif
