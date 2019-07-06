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
 * 
 *
 * 
 */
#ifndef __Base_Common_MemoryPool_Defs_MemoryAlloctor_H__
#define __Base_Common_MemoryPool_Defs_MemoryAlloctor_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"

FS_NAMESPACE_BEGIN

class MemoryBlock;
class FS_String;

// �ڴ����������
class BASE_EXPORT IMemoryAlloctor
{
    friend class MemoryPoolMgr;
public:
    IMemoryAlloctor();
    virtual ~IMemoryAlloctor();

public:
    void *AllocMemory(size_t bytesCnt, const FS_String &objName);
    void  FreeMemory(void *ptr);

private:
    void  _InitMemory();
    void _FinishMemory();

    /**
    *   �õ���ǰ���п��õĶ�����
    */
    size_t  _GetFreeBlock();

protected:
    char            *_buf;                  // ������������ڴ��ַ
    MemoryBlock     *_usableBlockHeader;    // ��� next�����ǿ��õ�δ������ڴ�飬����ʱ���_usableBlockHeader�ڵ㣬�ͷ�ʱ��Ҫ�ͷŵĽڵ����_usableBlockHeader֮ǰ�������ýڵ�ͷ
    size_t          _blockAmount;           // �ڴ��������
    size_t          _blockSize;             // �ڴ���С
};

template<size_t blockSize, size_t blockAmount>
class MemoryAlloctor : public IMemoryAlloctor
{
public:
    MemoryAlloctor();
};

FS_NAMESPACE_END

#include "base/common/memorypool/Defs/MemoryAlloctorImpl.h"

#endif
