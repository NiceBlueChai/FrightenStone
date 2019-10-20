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
 * @file  : ConditionLocker.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */

#include "stdafx.h"
#include "base/common/asyn/Lock/Impl/ConditionLocker.h"
#include "base/common/status/status.h"
#include <process.h>
#include "base/common/basedefs/BaseDefs.h"

#undef FS_IS_EVENT_WAKE_UP
#define FS_IS_EVENT_SINAL_WAKE_UP(waitRet)   \
(static_cast<long long>(WAIT_OBJECT_0) <= waitRet) &&\
(waitRet <= static_cast<long long>(MAXIMUM_WAIT_OBJECTS + WAIT_OBJECT_0))

FS_NAMESPACE_BEGIN

ConditionLocker::ConditionLocker()
    :_event(NULL)
    ,_isSinal(false)
    ,_waitCnt(0)
{
    _InitAnonymousEvent();
}

ConditionLocker::~ConditionLocker()
{
    _DestroyEvent();
}

int ConditionLocker::Wait(unsigned long milisec /*= INFINITE*/)
{
    long long waitRet = WAIT_OBJECT_0;
    auto *event = _event.load();
    while(!_isSinal)
    {
        Unlock();
        ++_waitCnt;
        waitRet = WaitForMultipleObjects(1, &event, true, milisec);
        Lock();
        --_waitCnt;

        // �����Ƿ񱻻��Ѷ������¼���������
        ResetEvent(_event.load());

        if(waitRet == WAIT_TIMEOUT)
        {// �����Ƿ񱻻��ѣ���Ϊ���ѵ�ʱ��ǡ���ǳ�ʱ����ʱ������
            _isSinal = false;
            return StatusDefs::WaitEventTimeOut;
        }

        // ���ִ�����ֱ��return
        if(!FS_IS_EVENT_SINAL_WAKE_UP(waitRet))
        {
            _isSinal = false;
            return StatusDefs::WaitEventFailure;
        }
    }

    _isSinal = false;
    return StatusDefs::Success;
// 
//     if((static_cast<long long>(WAIT_OBJECT_0) <= waitRet) &&
//         (waitRet <= static_cast<long long>(MAXIMUM_WAIT_OBJECTS + WAIT_OBJECT_0)))
//     {// �ȴ����ںϷ��ķ���ֵ��64���ں˶���
//         return StatusDefs::Success;
//     }
//     else if(waitRet == WAIT_TIMEOUT)
//     {// ��ʱ�ȴ�
//         return StatusDefs::WaitEventTimeOut;
//     }
// 
//     return StatusDefs::WaitEventFailure;
}

bool ConditionLocker::Sinal()
{
    _isSinal = SetEvent(_event.load());
    return _isSinal.load();
}

void ConditionLocker::Broadcast()
{
    bool isSinal = false;
    while(_waitCnt > 0)
    {
        Lock();

        if(LIKELY(Sinal()))
            isSinal = true;

        Unlock();
        Sleep(0);
    }

    if(LIKELY(isSinal))
    {
        _isSinal = false;
        ResetEvent(_event.load());
    }
}

void ConditionLocker::ResetSinal()
{
    ResetEvent(_event.load());
}

bool ConditionLocker::_InitAnonymousEvent()
{
    _event = CreateEvent(NULL, true, false, NULL);
    if(UNLIKELY(!_event))
        return false;

    return true;
}

bool ConditionLocker::_DestroyEvent()
{
    if(UNLIKELY(!_event.load()))
        return true;

    Broadcast();

    if(UNLIKELY(!CloseHandle(_event.load())))
        return false;

    _event = NULL;

    return true;
}

FS_NAMESPACE_END
