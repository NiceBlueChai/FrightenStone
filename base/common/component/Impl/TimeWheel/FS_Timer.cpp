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
 * @file  : FS_Timer.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/06/08
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/component/Impl/Time.h"
#include "base/common/component/Impl/TimeWheel/FS_Timer.h"

FS_NAMESPACE_BEGIN

FS_Timer::FS_Timer(TimeWheel *timeWheel)
    :_timeData(new TimeData(this))
    ,_timeWheel(timeWheel)
    , _timeOutDelegate(NULL)
    , _cancelTimerDelegate(NULL)
{
    if(!_timeWheel)
        _timeWheel = &g_TimeWheel;
}

FS_Timer::~FS_Timer()
{
    Fs_SafeFree(_timeOutDelegate);
    Fs_SafeFree(_cancelTimerDelegate);
    Fs_SafeFree(_timeData);
}

void FS_Timer::Cancel()
{
    bool isCancel = _timeData->_isCancel;
    _timeData->_isCancel = true;

    if(isCancel != _timeData->_isCancel && 
       _cancelTimerDelegate)
    {
        (*_cancelTimerDelegate)(this);
    }
}

Int32 FS_Timer::Schedule(Int64 milliSecPeriod)
{
    // ����������
    TimeSlice slice(0, milliSecPeriod, 0);

    // ����ʱ��
    auto nowTime = Time::Now();
    _lastTimeOutTime = nowTime;
    const auto &expiredTime = _timeData->_isRotatingWheel ? nowTime : (nowTime + slice);

    // ��ʱ���������Ƴ�
    if(_timeData->_timeWheelUniqueId)
        _timeWheel->UnRegister(_timeData);

    // ��������
    _timeData->_isCancel = false;
    _timeData->_period = slice;
    _timeData->_expiredTime = expiredTime;

    // ע��ʱ������
    return _timeWheel->Register(_timeData);
}

void FS_Timer::OnTimeOut(const Time &curWheelTIme)
{
    if(_lastTimeOutTime == 0)
        _lastTimeOutTime = curWheelTIme - _timeData->_period;

    if(_timeOutDelegate)
        (*_timeOutDelegate)(this, _lastTimeOutTime, curWheelTIme);
    
    // �����ϴγ�ʱʱ��
    _lastTimeOutTime = curWheelTIme;
}

FS_String FS_Timer::ToString() const
{
    FS_String info;
    info.Format("Timer:_lastTimeOutTime:%lld|_timeOutDelegate:0x%p|_cancelTimerDelegate:0x%p\n%s"
                , _lastTimeOutTime.GetMicroTimestamp(), _timeOutDelegate, _cancelTimerDelegate
                , _timeData->ToString().c_str());
    return info;
}

FS_NAMESPACE_END
