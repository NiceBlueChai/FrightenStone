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
 * @file  : ObjPoolHelper.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/9
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_ObjPool_Impl_ObjPoolHelper_H__
#define __Base_Common_ObjPool_Impl_ObjPoolHelper_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/asyn/asyn.h"
#include "base/common/objpool/Defs/ObjPoolDefs.h"
#include "base/common/objpool/Defs/ObjAlloctor.h"
#include "base/common/objpool/Defs/AlloctorNode.h"

FS_NAMESPACE_BEGIN

// ֧���̰߳�ȫ ���������̲߳���ȫ�����������
template<typename ObjType>
class ObjPoolHelper
{
public:
    ObjPoolHelper(size_t objAmount);
    virtual ~ObjPoolHelper();

public:
    size_t GetMemleakObjNum() const;
    const char *GetObjName() const;
    size_t GetMemleakBytes() const;
    size_t GetPoolBytesOccupied() const;
    size_t PrintMemleak(Int64 &poolOccupiedBytes);

    // ��������Ϊ�ӿ�
    IObjAlloctor<ObjType> *operator->();
    const IObjAlloctor<ObjType> *operator->() const;

public:
    Locker _locker;
    IObjAlloctor<ObjType> *_alloctor;
};

FS_NAMESPACE_END

#include "base/common/objpool/Impl/ObjPoolHelperImpl.h"

/// �ڴ�ش������������
// ��������Ҫ��� �����겻֧�ֻ����������� ��������������ֱ�Ӷ���һ��helper����
#undef  OBJ_POOL_CREATE
#define OBJ_POOL_CREATE(ObjType, _objpool_helper)                                                       \
public:                                                                                                 \
        void  *operator new(size_t bytes)       { return _objpool_helper._alloctor->Alloc();}           \
        void   operator delete(void *ptr)       { _objpool_helper._alloctor->Free(ptr);}                \
        static size_t GetMemleakNum();                                                                  \
                                                                                                        \
        template<typename... Args>                                                                      \
        static ObjType *New(Args &&... args)                                                            \
        {                                                                                               \
            return _objpool_helper->New(std::forward<Args>(args)...);                                   \
        }                                                                                               \
                                                                                                        \
        static ObjType *NewWithoutConstruct()                                                           \
        {                                                                                               \
            return _objpool_helper->NewWithoutConstruct();                                              \
        }                                                                                               \
                                                                                                        \
        template<typename... Args>                                                                      \
        static ObjType *NewByPtr(void *ptr, Args &&... args)                                            \
        {                                                                                               \
            return _objpool_helper->NewByPtr(ptr, std::forward<Args>(args)...);                         \
        }                                                                                               \
                                                                                                        \
        static void Delete(ObjType *ptr)                                                                \
        {                                                                                               \
            _objpool_helper->Delete(ptr);                                                               \
        }                                                                                               \
                                                                                                        \
        static void DeleteWithoutDestructor(ObjType *ptr)                                               \
        {                                                                                               \
            _objpool_helper->DeleteWithoutDestructor(ptr);                                              \
        }                                                                                               \
                                                                                                        \
        static fs::ObjPoolHelper<ObjType> _objpool_helper

// ��ʵ���ļ�����Ҫ���
#undef OBJ_POOL_CREATE_IMPL
#define OBJ_POOL_CREATE_IMPL(ObjType, _objpool_helper, objAmount)                                       \
fs::ObjPoolHelper<ObjType> ObjType::_objpool_helper(objAmount);                                         \
size_t ObjType::GetMemleakNum()                                                                         \
{                                                                                                       \
    return _objpool_helper.GetMemleakObjNum();                                                          \
}

#endif
