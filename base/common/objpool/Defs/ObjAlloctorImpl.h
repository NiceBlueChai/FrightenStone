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
 * @file  : ObjAlloctorImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/24
 * @brief :
 * 
 *
 * 
 */
#ifdef __Base_Common_ObjPool_Defs_ObjAlloctor_H__
#pragma once

#ifndef BLOCK_AMOUNT_DEF
#define BLOCK_AMOUNT_DEF    10240    // Ĭ���ڴ������
#endif

FS_NAMESPACE_BEGIN

template<typename ObjType>
inline IObjAlloctor<ObjType>::IObjAlloctor(AlloctorNode<ObjType> *curNode, size_t blockAmount)
    :_isInit(false)
    ,_buf(NULL)
    ,_curNode(curNode)
    ,_blockAmount(blockAmount)
    ,_freeBlockLeft(blockAmount)
    ,_usableBlockHeader(NULL)
    ,_blockSize(sizeof(ObjType))
    ,_objInUse(0)
{
    _blockSize =_blockSize / sizeof(void *) * sizeof(void *) + (_blockSize % sizeof(void *) ? sizeof(void *) : 0);
    _blockSize = _blockSize + sizeof(ObjBlock<ObjType>);
}

template<typename ObjType>
inline IObjAlloctor<ObjType>::~IObjAlloctor()
{
    FinishMemory();
}

template<typename ObjType>
inline void *IObjAlloctor<ObjType>::Alloc()
{
    // �ڴ治����ʹ��ϵͳ�ڴ���䷽��
    ObjBlock<ObjType> *newBlock = NULL;
    if(_usableBlockHeader == 0)
    {
        newBlock = reinterpret_cast<ObjBlock<ObjType> *>(::malloc(_blockSize));
        newBlock->_isInPool = false;    // �����ڴ����
        newBlock->_ref = 1;             // ��1������
        newBlock->_alloctor = this;     // ������
        newBlock->_nextBlock = 0;
        newBlock->_objSize = sizeof(ObjType);
    }
    else
    {
        /**
        *   get one node from free list
        */
        newBlock = _usableBlockHeader;
        _usableBlockHeader = _usableBlockHeader->_nextBlock;
        newBlock->_alloctor =   this;
        newBlock->_isInPool = true;
        --_freeBlockLeft;

        ASSERT(newBlock->_ref == 0);
        newBlock->_ref = 1;
    }

    ++_objInUse;
//     if(newBlock)
//         _inUsings.insert(newBlock);

    return ((char*)newBlock) + sizeof(ObjBlock<ObjType>);   // ��block����һ����ַ��ʼ�������������뵽���ڴ�
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::Free(void *ptr)
{
    // �ڴ��ͷ
    char *ptrToFree = reinterpret_cast<char *>(ptr);
    ObjBlock<ObjType> *blockHeader = reinterpret_cast<ObjBlock<ObjType> *>(reinterpret_cast<char*>(ptrToFree - sizeof(ObjBlock<ObjType>)));

    // ���ü���
    if(--blockHeader->_ref != 0)
        return;

    // �����ڴ���������ü���Ϊ0
    if (!blockHeader->_isInPool)
    {
        ::free(blockHeader);
        --_objInUse;
        // _inUsings.erase(blockHeader);
        return;
    }

    // ���ͷŵĽڵ�嵽����ͷ�ڵ�֮ǰ
    blockHeader->_nextBlock = _usableBlockHeader;
    _usableBlockHeader = blockHeader;
    ++_freeBlockLeft;
    --_objInUse;

    // �ͷź������ʹ�����Ƴ�
    // _inUsings.erase(blockHeader);
}

template<typename ObjType>
inline bool IObjAlloctor<ObjType>::NotBusy()
{
    return (_freeBlockLeft * 100 / _blockAmount) >= ObjPoolDefs::__g_FreeRate;
}

template<typename ObjType>
inline bool IObjAlloctor<ObjType>::IsEmpty() const
{
    return !_usableBlockHeader;
}

template<typename ObjType>
inline size_t IObjAlloctor<ObjType>::GetObjInUse() const
{
    return _objInUse;
}

template<typename ObjType>
inline AlloctorNode<ObjType> *IObjAlloctor<ObjType>::GetNode()
{
    return _curNode;
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::InitMemory()
{
    if(_isInit)
        return;

    ASSERT(_buf == 0);
    if(_buf)
        return;

    /**
    *   ������Ҫ������ڴ��С�����а����ڴ�ͷ���ݴ�С
    */
    size_t	bufSize = _blockSize * _blockAmount;

    /**
    *   �����ڴ�
    */
    _buf = reinterpret_cast<char*>(::malloc(bufSize));
    memset(_buf, 0, bufSize);

    /**
    *   ����ͷ��β��ָ��ͬһλ��
    */
    _usableBlockHeader = reinterpret_cast<ObjBlock<ObjType> *>(_buf);
    _usableBlockHeader->_ref = 0;
    _usableBlockHeader->_alloctor = this;
    _usableBlockHeader->_nextBlock = 0;
    _usableBlockHeader->_isInPool = true;
    _usableBlockHeader->_objSize = sizeof(ObjType);
    ObjBlock<ObjType> *temp = _usableBlockHeader;

    // �����ڴ������
    for(size_t i = 1; i < _blockAmount; ++i)
    {
        char *cache = (_buf + _blockSize * i);
        ObjBlock<ObjType> *block = reinterpret_cast<ObjBlock<ObjType> *>(cache);
        block->_ref = 0;
        block->_isInPool = true;
        block->_alloctor = this;
        block->_nextBlock = 0;
        block->_objSize = sizeof(ObjType);
        temp->_nextBlock = block;
        temp = block;
    }

    _isInit = true;
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::FinishMemory()
{
    if(!_isInit)
        return;

    _isInit = false;

    // �ռ����ͷ�й©���ڴ�
    std::pair<Int64, Int64> amountRefSizePair;
//     for(auto &block : _inUsings)
//     {
//         if(block->_ref)
//         {
//             amountRefSizePair.second += 1;
//             amountRefSizePair.first += _blockSize;
//             if(!block->_isInPool)
//                 ::free(block);
//         }
//     }

    // ʹ��ϵͳ���ڴ�����ͷŵ�
    while(_usableBlockHeader != 0)
    {
        auto *header = _usableBlockHeader;
        _usableBlockHeader = _usableBlockHeader->_nextBlock;

        if(!header->_isInPool)
            ::free(header);
    }
    _usableBlockHeader = 0;

    // �ͷű�ϵͳ�ڴ�
    if(_buf)
    {
        ::free(_buf);
        _buf = NULL;
    }

    // �ڴ�й©��ӡ
//     if(amountRefSizePair.first)
//     {
//         g_Log->memleak("objName[%s], amount[%lld]*size[%lld];"
//                        , typeid(ObjType).name(), amountRefSizePair.first, amountRefSizePair.second);
//     }
}

FS_NAMESPACE_END

#endif
