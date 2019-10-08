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
 * @file  : Trigger.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_Trigger_H__
#define __Base_Common_Component_Impl_Trigger_H__

#pragma once

#include <base/exportbase.h>
#include "base/common/component/Defs/TriggerDefs.h"
#include "base/common/basedefs/Macro/MacroDefs.h"
#include<functional>
#include "base/common/component/Impl/FS_Delegate.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT TriggerOccasion;

class BASE_EXPORT Trigger
{
public:
    Trigger(IDelegate<bool, Int32> *canInterrupt);
    virtual ~Trigger();

public:
    // 外部请释放delegate，内部会创建delegate的副本
    Int32 Reg(Int32 occasion
                , Int32 triggerType
                , const IDelegate<Int32, TriggerExecuteBody *> &exec // 外部请释放delegate，内部会创建delegate的副本
                , Int32 execTimes = 1
                , Int32 addType = TriggerDefs::AddIfExist); // 

    Int32 Exec(Int32 occasion);

    void EraseAllTrigger(Int32 triggerType); // 
    void Clear();
    bool IsExist(Int32 occasionType, Int32 triggerType);

private:
    void _CancelTriggerTypeOccasionsRelevance(const std::set<Int32> &triggerTypes, Int32 occasion);

private:
    friend class TriggerOccasion;
    std::map<Int32, TriggerOccasion *> _occasions;     // key:occationtype
    std::map<Int32, std::set<Int32>> _triggerTypeRefOccasions;        // key:triggerType, val:set(occations) 
    IDelegate<bool, Int32> *_canInterrupt;  // bool:return,Int32:thisOccation
};

FS_NAMESPACE_END

#endif // !__Base_Common_Component_Impl_Trigger_H__