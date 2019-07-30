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
    IObjAlloctor(size_t blockAmountPerNode);
    virtual ~IObjAlloctor();

public:
    void *Alloc();
    void  Free(void *ptr);
    size_t GetObjInUse() const;
    size_t GetTotalObjBlocks() const;    // ��ǰȫ���Ķ�������
    size_t GetNodeCnt() const;
    size_t GetBytesOccupied() const;

private:
    void _NewNode();

protected:
    void *_curNodeObjs;                     // ��ǰ�ڵ�Ķ���ػ���
    ObjType *_lastDeleted;                  // free���󹹳ɵ�����ָ��ָ����ڴ�洢������һ���ͷŵĶ���ĵ�ַ
    size_t _alloctedInCurNode;              // ��ǰ�ڵ��ѷ���Ķ������
    const size_t _nodeCapacity;             // ÿ���ڵ�������
    AlloctorNode<ObjType> *_header;         // ͷ�ڵ�
    AlloctorNode<ObjType> *_lastNode;       // ���µĽڵ�
    static const size_t _objBlockSize;      // ������С

    // �ڴ�й©���
    size_t          _nodeCnt;               // �ڵ����
    size_t          _bytesOccupied;         // �����ռ���ڴ��С = _objBlockSize * _nodeCnt * node._capacity
    size_t          _objInUse;              // ����ʹ�õĶ���
};

FS_NAMESPACE_END

#include "base/common/objpool/Defs/ObjAlloctorImpl.h"

#endif
