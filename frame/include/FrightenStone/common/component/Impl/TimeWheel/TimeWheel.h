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
#ifndef __Frame_Include_FrightenStone_Common_Component_Impl_TimeWheel_TimeWheel_H__
#define __Frame_Include_FrightenStone_Common_Component_Impl_TimeWheel_TimeWheel_H__

#pragma once

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/component/Impl/Time.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/component/Impl/TimeSlice.h"
#include "FrightenStone/common/component/Impl/TimeWheel/Comp/TimeData.h"
#include <set>
#include "FrightenStone/common/memorypool/memorypool.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT TimeWheel
{
    OBJ_POOL_CREATE_DEF(TimeWheel);
public:
    explicit TimeWheel(const TimeSlice &resolutionSlice);
    virtual ~TimeWheel();

    // 转动时间轮盘
    void RotateWheel();

    // 是否注册
    bool IsRegister(TimeData *timeData) const;
    // 注册超时数据
    Int32 Register(TimeData *timeData);
    // 取消注册
    void UnRegister(TimeData *timeData);
    // 当前时间轮盘精度 @return:返回微妙
     const TimeSlice &GetTimeWheelResolution() const;
     // 下一帧轮盘扫描时间修正值
     void GetModifiedResolution(TimeSlice &modifiedResolution) const;
     // 当前时间
     const Time &GetCurTime() const;

private:
    // 产生timewheel生命周期中唯一id
    Int64 _NewIncreasId();
    bool _IsRotating() const;
    void _BeforeRotateWheel();
    void _AfterRotateWheel();
    AsynTimeData *_NewAsynTimeData(Int32 opType, TimeData *timeData);

private:
    Int64 _rotating;                        // 是否正在转动
    const TimeSlice _resolutionSlice;       // 每一轮时间切片长度
    // Time _nowTimeToJudgeWheelTimeOut;   // 当前时间用于判断是否达到轮盘处理的最小扫描时间
    Time _curTime;                  // 当前时间戳
    std::set<TimeData *, TimeDataLess> _timeDatas;
    std::vector<AsynTimeData *> _asynData;          // 异步执行
    std::map<TimeData *, Int32> _asynRegisterFlag;  // 为异步建立索引,value:register+1,unrigister-1,

    Int64 _increaseId;              // 递增id
};

FS_NAMESPACE_END

#include "FrightenStone/common/component/Impl/TimeWheel/TimeWheelImpl.h"

extern BASE_EXPORT fs::TimeWheel *g_BusinessTimeWheel;

#endif
