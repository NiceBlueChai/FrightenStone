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
 * @file  : ObjAlloctor.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_ObjPool_Defs_ObjAlloctor_H__
#define __Base_Common_ObjPool_Defs_ObjAlloctor_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include <set>
#include "base/common/objpool/Defs/ObjBlock.h"
#include "base/common/objpool/Defs/ObjPoolDefs.h"

FS_NAMESPACE_BEGIN

template<typename ObjType>
class ObjBlock;

template<typename ObjType>
class AlloctorNode;

// �ڴ����������
template<typename ObjType>
class IObjAlloctor
{
    template<typename ObjType>
    friend class ObjPoolHelper;
public:
    IObjAlloctor(AlloctorNode<ObjType> *curNode, size_t blockAmount);
    virtual ~IObjAlloctor();

public:
    void *Alloc();
    void  Free(void *ptr);
    bool NotBusy();    // freeʱ���ж�
    bool IsEmpty() const;
    size_t GetObjInUse() const;
    AlloctorNode<ObjType> *GetNode();

public:
    void  InitMemory();
    void FinishMemory();

protected:
    bool                _isInit;
    char                *_buf;                  // ������������ڴ��ַ
    AlloctorNode<ObjType> *_curNode;            // ����ڵ�
    ObjBlock<ObjType>   *_usableBlockHeader;    // ��� next�����ǿ��õ�δ������ڴ�飬����ʱ���_usableBlockHeader�ڵ㣬�ͷ�ʱ��Ҫ�ͷŵĽڵ����_usableBlockHeader֮ǰ�������ýڵ�ͷ
    size_t          _blockAmount;           // �ڴ��������
    size_t          _blockSize;             // �ڴ���С
    size_t          _freeBlockLeft;         // ʣ������ڴ��
    size_t          _objInUse;              // ����ʹ�õĶ���
    // std::set<ObjBlock<ObjType> *> _inUsings;   // ����ʹ�õ��ڴ��
};

FS_NAMESPACE_END

#include "base/common/objpool/Defs/ObjAlloctorImpl.h"

#endif
