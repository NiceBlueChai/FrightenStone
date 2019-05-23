#ifndef __Base_Common_Component_Impl_TimeSlice__H__
#define __Base_Common_Component_Impl_TimeSlice__H__
/**
* @file    Time.h
* @author  Huiya Song<120453674@qq.com>
* @date    2019/03/01
* @brief
*/

#pragma once

#include "base/common/basedefs/DataType/DataType.h"
#include "base/common/basedefs/Macro/MacroDefs.h"

FS_NAMESPACE_BEGIN

class Time;
class FS_String;

class BASE_EXPORT TimeSlice
{
public:
    /**
     * Ĭ�Ϲ��죬��ʼ��0ʱ��Ƭ����.
     */
    TimeSlice();

    /**
     * Construct by slice, in seconds.
     * @param[in] seconds      - the slice seconds part.
     * @param[in] milliSeconds - the slice milli-seconds part.
     * @param[in] microSeconds - the slice micro-seconds part.
     */
    explicit TimeSlice(int seconds, int milliSeconds = 0, int microSeconds = 0);

    /**
     * Construct by slice string representation(fmt: 00:00:00.xxxxxx).
     * @param[in] slice - the slice value string representation, fmt: 00:00:00.xxxxx, the micro-seconds is optional.
     */
    TimeSlice(const FS_String &fmtSlice);

    /**
     * Copy constructor.
     */
    TimeSlice(const TimeSlice &slice);

    /**
     * Time slice parts constructor.
     * @param[in] days         - the days part.
     * @param[in] hours        - the hours part.
     * @param[in] minutes      - the minutes part.
     * @param[in] seconds      - the seconds part.
     * @param[in] milliSeconds - the milli-seconds part, default is 0.
     * @param[in] microSeconds - the micro-seconds part, default is 0.
     */
    TimeSlice(int days, int hours, int minutes, int seconds, int milliSeconds = 0, int microSeconds = 0);

    /**
     * Destructor.
     */
    ~TimeSlice();

    /**
     * Get days/hours/minutes/seconds/milli-seconds/micro-seconds.
     * @return int - the time slice parts value. slice ��һ����
     */
    int GetDays() const;
    int GetHours() const;
    int GetMinutes() const;
    int GetSeconds() const;
    int GetMilliSeconds() const;
    int GetMicroSeconds() const;

    int GetTotalDays() const;
    int GetTotalHours() const;
    int GetTotalMinutes() const;
    int GetTotalSeconds() const;
    Int64 GetTotalMilliSeconds() const;
    const Int64 &GetTotalMicroSeconds() const;

    TimeSlice operator +(const TimeSlice &span) const;
    TimeSlice operator -(const TimeSlice &span) const;

    TimeSlice &operator +=(const TimeSlice &span);
    TimeSlice &operator -=(const TimeSlice &span);

    bool operator ==(const TimeSlice &span) const;
    bool operator !=(const TimeSlice &span) const;
    bool operator <(const TimeSlice &span)const;
    bool operator >(const TimeSlice &span) const;
    bool operator <=(const TimeSlice &span) const;
    bool operator >=(const TimeSlice &span) const;

    TimeSlice &operator =(const TimeSlice &span);

    FS_String ToString() const;
private:
    friend class Time;
    TimeSlice(const Int64 &slice);

private:
    Int64 _slice;
};

FS_NAMESPACE_END

#include "base/common/component/Impl/TimeSliceImpl.h"

#endif
