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
 * @file  : TimeWeel.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/6/6
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/component/Impl/Time.h"
#include "base/common/assist/utils/utils.h"
#include "base/common/component/Impl/TimeWheel/Comp/TimeData.h"
#include "base/common/component/Impl/TimeWheel/TimeWheel.h"
#include "base/common/status/status.h"
#include "base/common/component/Impl/TimeSlice.h"
#include <iostream>

FS_NAMESPACE_BEGIN

TimeWheel::TimeWheel(const TimeSlice &resolutionSlice)
    :_resolutionSlice(resolutionSlice)
    ,_increaseId(0)
    ,_curTime(Time::Now())
{

}

TimeWheel::~TimeWheel()
{
}

void TimeWheel::RotateWheel()
{
    // 更新时间
    _lastTime = _curTime;
    if(_curTime == 0)
        _curTime.FlushTime(Time::NowMicroTimestamp());

    _curTime.FlushTime();

    // 遍历
    std::set<TimeData *> timeDataToRefresh;
    for(auto iterUniqueRefTimeDatas = _timeDatas.begin(); iterUniqueRefTimeDatas != _timeDatas.end();)
    {
        // 判断是否过期
        auto timeData = *iterUniqueRefTimeDatas;
        if(timeData->_expiredTime > _curTime)
            break;

        // 执行超时delegate
        if(timeData->_timeOutDelegate)
        {
            timeData->_isRotatingWheel = true;
            const auto &lastTimeOutTime = timeData->_timer->GetLastTimeOutTime();
            if(lastTimeOutTime == 0)
                timeData->_timer->UpdateLastTimeOutTime(_curTime - timeData->_period);

            (*timeData->_timeOutDelegate)(timeData->_timer, timeData->_timer->GetLastTimeOutTime(), _curTime);
            timeData->_timer->UpdateLastTimeOutTime(_curTime);
            timeData->_isRotatingWheel = false;
        }

        // 重新加入
        if(!timeData->_isCancel)
            timeDataToRefresh.insert(timeData);

        // 移除
        iterUniqueRefTimeDatas = _timeDatas.erase(iterUniqueRefTimeDatas);
    }

    // 更新超时时间并重新加入
    for(auto iterToRefresh = timeDataToRefresh.begin(); iterToRefresh != timeDataToRefresh.end(); ++iterToRefresh)
    {
        auto timeData = *iterToRefresh;
        timeData->_expiredTime.FlushTime(timeData->_expiredTime.GetMicroTimestamp() + timeData->_period.GetTotalMicroSeconds());
        _timeDatas.insert(timeData);
    }
}

Int32 TimeWheel::Register(TimeData *timeData)
{
    if(timeData->_isRotatingWheel)
        return StatusDefs::TimeWheel_CantRegisterWhenRotatingWheel;

    const auto expiredTime = timeData->_expiredTime.GetMicroTimestamp();
    if(UNLIKELY(!expiredTime))
        return StatusDefs::TimeWheel_ExpiredTimeIsNull;

    if(!timeData->_timeWheelUniqueId)
        timeData->_timeWheelUniqueId = _NewIncreasId();
    
    auto iterTimeData = _timeDatas.find(timeData);
    if(UNLIKELY(iterTimeData != _timeDatas.end()))
        return StatusDefs::TimeWheel_RegisterAExistsTimeData;

    _timeDatas.insert(timeData);

    return StatusDefs::Success;
}

FS_NAMESPACE_END
