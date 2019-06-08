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
 * @file  : TestTimeWheel.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/06/08
 * @brief :
 * 
 *
 * 
 */
#ifndef __Test_TestTimeWheel_H__
#define __Test_TestTimeWheel_H__
#pragma once

class TestWheel1
{
public:
    void TimeOut(fs::FS_Timer *&timer, const fs::Time &lastWheelTime, const fs::Time &curTime)
    {
        std::cout << "TestWheel1" << std::endl;

        if(lastWheelTime.GetLocalMinute() != curTime.GetLocalMinute())
        {
            std::cout << "cross minitue" << std::endl;
        }
    }
};

static void TimeOut(fs::FS_Timer *&timer, const fs::Time &lastWheelTime, const fs::Time &curTime)
{
    std::cout << "static test :" << std::endl;

    if(lastWheelTime.GetLocalMinute() != curTime.GetLocalMinute())
    {
        std::cout << "cross minitue" << std::endl;
    }

    timer->Cancel();
    static Int32 cnt = 5;
    if(cnt-->0)
        timer->Schedule(1000);
}

static void Cancel(fs::FS_Timer *&timer)
{
    std::cout << "cancel a timer" << std::endl;
}

class TestTimeWheel
{
public:
    static void Run()
    {
        // ����ʱ�����̲���
//         fs::TimeSlice resolution(0, 100);
//         fs::TimeWheel timeWheel(resolution);
        fs::FS_Timer timer;
        fs::FS_Timer timer2;

        // ���ó�ʱִ�к���
        TestWheel1 test1;
        timer.SetTimeOutHandler(&test1, &TestWheel1::TimeOut);
        timer.Schedule(1000);

        timer2.SetTimeOutHandler(&TimeOut);
        timer2.SetCancelHandler(&Cancel);

        while(true)
        {
            Sleep(static_cast<DWORD>(fs::g_TimeWheel.GetTimeWheelResolution().GetTotalMilliSeconds()));

            // ת��ʱ������
            fs::g_TimeWheel.RotateWheel();
            static bool isOnce = false;
            if(!isOnce)
            {
                isOnce = true;
                timer2.Schedule(10000);
            }
        }
    }
};

#endif