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
 * @file  : FS_Delegate.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_FS_Delegate_H__
#define __Base_Common_Component_Impl_FS_Delegate_H__


#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/Macro/MacroDefs.h"

#ifndef FS_DELG_ADAPTARG
#define FS_DELG_ADAPTARG(arg)   std::forward<decltype(arg)>(arg)
#endif

FS_NAMESPACE_BEGIN

// T:�࣬R�ص�����ֵ���ͣ�Args�ص�����������
template <typename T, typename R, typename... Args>
class FS_Delegate
{
public:
    FS_Delegate(T *t, R(T::*f)(Args...));
    R operator()(Args&&... args);
    virtual void Release();

private:
    T *_obj;
    R(T::*_f)(Args...);
};

class BASE_EXPORT DelegateFactory
{
public:
    template <typename T, typename R, typename... Args>
    static FS_Delegate<T, R, Args...> *Create(T *obj, R(T::*f)(Args...));
};

// R�ص�����ֵ���ͣ�Args�ص����������� ί�л��� ���ڽ���������ͣ����������޹ص�ί��
template <typename R, typename... Args>
class IDelegatePlus
{
public:
    IDelegatePlus();
    virtual ~IDelegatePlus();
    // ��ֵ��󶨳���ֵ���ã���ֵ��󶨳���ֵ����
    // ��ע�������۵��ʵ�ʹ��std::forward���������Ľ��������룬ԭ��ʲô���ʹ����󶨵ľ���ʲô����
    virtual R operator()(Args&&... args) = 0;
    virtual R operator()(Args&&... args) const = 0;
    virtual void Release();
};

template <typename T, typename R, typename... Args>
class DelegateClassPlus : public IDelegatePlus<R, Args...>
{
public:
    DelegateClassPlus(T *t, R(T::*f)(Args...));
    DelegateClassPlus(T *t, R(T::*f)(Args...) const);
    virtual ~DelegateClassPlus();

    virtual R operator()(Args&&... args);
    virtual R operator()(Args&&... args) const;

private:
    T *_obj;
    R(T::*_f)(Args...);
    // mutable R(T::*_fconst)(Args...) const;
};

template <typename R, typename... Args>
class DelegateFunctionPlus : public IDelegatePlus<R, Args...>
{
public:
    DelegateFunctionPlus(R(*f)(Args...));
    virtual ~DelegateFunctionPlus();

    virtual R operator()(Args&&... args);
    virtual R operator()(Args&&... args) const;

private:
    R(*_f)(Args...);
};

// ֧��lambda���ʽ,std::function�ȱհ�
template <typename CustomFuncType, typename Rtn, typename... Args>
class DelegateCustomFuncPlus : public IDelegatePlus<Rtn, Args...>
{
public:
    DelegateCustomFuncPlus(CustomFuncType &&customFunc);
    DelegateCustomFuncPlus(CustomFuncType const&customFunc);
    virtual ~DelegateCustomFuncPlus();

    virtual Rtn operator()(Args&&...);
    virtual Rtn operator()(Args&&... args) const;

private:
    CustomFuncType _customFun;
};

class BASE_EXPORT DelegatePlusFactory
{
public:
    template <typename T, typename R, typename... Args>
    static IDelegatePlus<R, Args...> *Create(T *obj, R(T::*f)(Args...));
    template <typename T, typename R, typename... Args>
    static const IDelegatePlus<R, Args...> *Create(T *obj, R(T::*f)(Args...) const);

    template <typename R, typename... Args>
    static IDelegatePlus<R, Args...> *Create(R(*f)(Args...));

    // ��lambda,std::function,�磺DelegatePlusFactory::Create<decltype(func), void, int>(func);
    template <typename CustomFuncType /* = decltype(func) */, typename Rtn, typename... Args>
    static IDelegatePlus<Rtn, Args...> *Create(CustomFuncType &&func);

    // ��lambda,std::function,�磺DelegatePlusFactory::Create<decltype(func), void, int>(func);
    template <typename CustomFuncType /* = decltype(func) */, typename Rtn, typename... Args>
    static IDelegatePlus<Rtn, Args...> *Create(CustomFuncType const&func);
};

FS_NAMESPACE_END

#include "base/common/component/Impl/FS_DelegateImpl.h"

#endif
