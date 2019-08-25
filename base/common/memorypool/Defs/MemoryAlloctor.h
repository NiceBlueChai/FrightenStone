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
 * @file  : MemoryAlloctor.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/5
 * @brief : ��Ҫ�����ڴ�й©��־
 *          alloctor�ṹ��             | - block
 *                          node    -  | - block
 *                                     | - ...
 *
 *                          newNode...
 *
 *                                     | - block
 *                          freeList - | - block
 *                                     | - ...
 *
 *          alloc ���ȴ�_lastDeleted����ȡ���ڴ治�㴴��һ���ڵ�
 *          
 *          free ʱ��block��������
 * 
 */
#ifndef __Base_Common_MemoryPool_Defs_MemoryAlloctor_H__
#define __Base_Common_MemoryPool_Defs_MemoryAlloctor_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include <set>
#include "base/common/component/Impl/FS_Delegate.h"

FS_NAMESPACE_BEGIN

struct MemBlocksNode;
class MemoryBlock;
class FS_String;

// �ڴ����������
class BASE_EXPORT IMemoryAlloctor
{
    friend class MemoryPoolMgr;
public:
    IMemoryAlloctor(const bool &canCreateNewNode);
    virtual ~IMemoryAlloctor();

public:
    void *AllocMemory(size_t bytesCnt);
    void  FreeMemory(void *ptr);
    size_t GetBlockSize() const;
    size_t GetEffectiveBlockSize() const;
    size_t GetOccupiedBytes() const;
    size_t GetInUsedBytes() const;
    void PrintMemInfo() const;

public:
    void InitMemory();
    void FinishMemory();

protected:
    void _NewNode();
    void _InitNode(MemBlocksNode *newNode);

protected:
    bool            _isFinish;
    char            *_curBuf;               // ������������ڴ��ַ
    MemoryBlock     *_usableBlockHeader;    // ��� next�����ǿ��õ�δ������ڴ�飬����ʱ���_usableBlockHeader�ڵ㣬�ͷ�ʱ��Ҫ�ͷŵĽڵ����_usableBlockHeader֮ǰ�������ýڵ�ͷ
    MemoryBlock     *_lastDeleted;          // free����
    Int64           _memBlockInUse;         // ����ʹ�õ��ڴ�����

    /* �ڴ�ڵ� */
    MemBlocksNode *_header;                 // ͷ�ڵ�
    MemBlocksNode *_lastNode;               // ��ǰ���½ڵ�
    Int64 _curNodeCnt;                      // �ڵ����

    // ���µĽڵ�
    size_t          _blockAmount;           // �ڴ��������
    size_t          _blockSize;             // �ڴ���С
    size_t          _effectiveBlockSize;    // ��Ч�ڴ���С���۳�MemoryBlock���ڴ��С��

    IDelegatePlus<void, size_t> *_updateMemPoolOccupied = NULL;
    const bool &_canCreateNewNode;
};

class MemoryAlloctor : public IMemoryAlloctor
{
public:
    MemoryAlloctor(size_t blockSize, size_t blockAmount, IDelegatePlus<void, size_t> *updateMemPoolOccupied, const bool &canCreateNewNode);
    virtual ~MemoryAlloctor();
};

FS_NAMESPACE_END

#include "base/common/memorypool/Defs/MemoryAlloctorImpl.h"

#endif
