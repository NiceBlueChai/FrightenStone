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
 * @file  : ObjPool.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/9
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_ObjPool_Impl_ObjPool_H__
#define __Base_Common_ObjPool_Impl_ObjPool_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/objpool/Interface/IObjPool.h"
#include "base/common/memorypool/Defs/MemoryAlloctor.h"
#include "base/common/asyn/asyn.h"
#include "base/common/status/status.h"

FS_NAMESPACE_BEGIN

class MemoryAlloctor;

class BASE_EXPORT ObjPool : public IObjPool
{
public:
    ObjPool(size_t objSize, size_t objAmount);
    virtual ~ObjPool();

    virtual Int32 InitPool();
    virtual void FinishPool();
    virtual void *Alloc(size_t bytes, const Byte8 *objName);
    virtual void  Free(void *ptr);
    virtual void  AddRef(void *ptr);
    virtual void Lock();
    virtual void Unlock();

private:
    Locker _locker;
    size_t _objSize;
    size_t _objAmount;
    MemoryAlloctor *_objAlloctor;
};

FS_NAMESPACE_END

#include "base/common/objpool/Impl/ObjPoolImpl.h"

#endif