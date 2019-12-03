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
 * @file  : Locker.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __Frame_Include_FrightenStone_Common_Asyn_Lock_Impl_Locker_H__
#define __Frame_Include_FrightenStone_Common_Asyn_Lock_Impl_Locker_H__


#pragma once

#include<FrightenStone/exportbase.h>
#include "FrightenStone/common/assist/assistobjs/Defs/AssistObjsDefs.h"
#include "FrightenStone/common/basedefs/Macro/MacroDefs.h"
#include "FrightenStone/common/basedefs/Resource/Resource.h"


FS_NAMESPACE_BEGIN

class MetaLocker;

class BASE_EXPORT Locker
{
public:
    Locker();
    virtual ~Locker();

public:
    void Lock();
    void Unlock();
    bool TryLock();

#ifdef _WIN32
    bool IsOtherThreadOwnLock() const;
#endif

private:
    void _Init();
    void _Destroy();
    
protected:
    std::atomic<MetaLocker *> _metaLocker;
};
FS_NAMESPACE_END

#endif // !__Frame_Include_FrightenStone_Common_Asyn_Lock_Impl_Locker_H__
