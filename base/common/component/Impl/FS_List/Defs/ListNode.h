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
 * @file  : ListNode.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/30
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_FS_List_Defs_ListNode_H__
#define __Base_Common_Component_Impl_FS_List_Defs_ListNode_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/memorypool/memorypool.h"

FS_NAMESPACE_BEGIN

template<typename ObjType>
struct ListNode
{
    OBJ_POOL_CREATE_DEF(ListNode<ObjType>);

public:
    ListNode(ObjType obj);

public:
    ObjType _obj = ObjType();
    ListNode<ObjType> *_preNode = NULL;
    ListNode<ObjType> *_nextNode = NULL;
};

FS_NAMESPACE_END

#include "base/common/component/Impl/FS_List/Defs/ListNodeImpl.h"

#endif
