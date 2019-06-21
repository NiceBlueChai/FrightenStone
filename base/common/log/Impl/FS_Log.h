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
 * @file  : FS_Log.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/6/12
 * @brief :
 *          1.�ṩд��־�ӿڣ��������������
 *          2.��Ŀ¼��־ crash, system, netlayerlog��Э����Ϣ��ҵ�����־��
 *          3.�����ļ���С���ļ�
 *          4.������������ȼ���:warning,error,debug,info��
 *          5.֧��json log ���
 *                              {
                                    time:(��ȷ��΢��)fmt:1970-01-01 00:00:00.123456
                                    class name:
                                    function:
                                    line:
                                    level:
                                    content:
                                    {
                                        op:
                                        status:statusCode
                                        ecxceptioninfo:
                                        stackinfo:
                                    }
                                }
 */
#ifndef __Base_Common_Log_Impl_Log_H__
#define __Base_Common_Log_Impl_Log_H__
#pragma once

#include "base/common/log/Interface/ILog.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT FS_Log : public ILog
{
public:
    FS_Log() {}
    virtual ~FS_Log() {}

    // json��־
    template<typename T>
    void Ji();
    template<typename T>
    void Jd();
    template<typename T>
    void Jw();
    template<typename T>
    void Je();
    
    // ��ͨ��־
    template<typename T>
    void i();
    template<typename T>
    void d();
    template<typename T>
    void w();
    template<typename T>
    void e();

    // ���ûص�
    void InstallCallBack();
};

FS_NAMESPACE_END

#include "base/common/log/Impl/FS_LogImpl.h"

#endif
