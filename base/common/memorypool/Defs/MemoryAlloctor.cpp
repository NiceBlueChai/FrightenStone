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
 * @file  : MemoryAlloctor.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/5
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/memorypool/Defs/MemoryAlloctor.h"
#include "base/common/memorypool/Defs/MemoryBlock.h"
#include "base/common/log/Log.h"
#include "base/common/assist/utils/Impl/SystemUtil.h"
#include "base/common/memorypool/Defs/MemoryPoolDefs.h"
#include "base/common/memorypool/Defs/MemBlocksNode.h"

FS_NAMESPACE_BEGIN

IMemoryAlloctor::IMemoryAlloctor(std::atomic<bool>  *canCreateNewNode)
    :_isFinish(false)
    ,_curBuf(NULL)
    ,_blockAmount(BLOCK_AMOUNT_DEF)
    ,_usableBlockHeader(NULL)
    ,_blockSize(0)
    ,_effectiveBlockSize(0)
    ,_lastDeleted(NULL)
    ,_memBlockInUse(0)
    ,_header(NULL)
    ,_lastNode(NULL)
    ,_curNodeCnt(0)
    ,_canCreateNewNode(canCreateNewNode)
{
    
}

IMemoryAlloctor::~IMemoryAlloctor()
{
    FinishMemory();
}

void *IMemoryAlloctor::AllocMemory(size_t bytesCnt)
{
    // 判断free链表
    MemoryBlock *newBlock = NULL;
    if(_lastDeleted)
    {
        newBlock = _lastDeleted;
        _lastDeleted = _lastDeleted->_nextBlock;
        newBlock->_alloctor = this;
        ASSERT(newBlock->_ref == 0);
        newBlock->_ref = 1;
    }
    else
    {
        // 没有可用内存开辟新节点
        if(!_usableBlockHeader)
        {
            if(_canCreateNewNode && !(*_canCreateNewNode))
                return NULL;
            _NewNode();
        }

        newBlock = _usableBlockHeader;
        _usableBlockHeader = _usableBlockHeader->_nextBlock;
        newBlock->_alloctor = this;
        newBlock->_isInPool = true;
        ASSERT(newBlock->_ref == 0);
        newBlock->_ref = 1;
    }
    
    newBlock->_objSize = static_cast<Int64>(bytesCnt);
    ++_memBlockInUse;

    return ((char*)newBlock) + sizeof(MemoryBlock);   // 从block的下一个地址开始才是真正的申请到的内存
}

void IMemoryAlloctor::FreeMemory(void *ptr)
{
    // 内存块头
    char *ptrToFree = reinterpret_cast<char*>(ptr);
    MemoryBlock *blockHeader = reinterpret_cast<MemoryBlock*>(reinterpret_cast<char*>(ptrToFree - sizeof(MemoryBlock)));

    // 引用计数
    if(--blockHeader->_ref != 0)
        return;

    // 被释放的节点插到free链表头
    blockHeader->_nextBlock = _lastDeleted;
    _lastDeleted = blockHeader;
    --_memBlockInUse;
}

void IMemoryAlloctor::_InitNode(MemBlocksNode *newNode)
{
    _curBuf = reinterpret_cast<char *>(newNode->_memBuff);
    ASSERT(_curBuf != 0);

    /**
    *   计算需要申请的内存大小，其中包含内存头数据大小
    */
    size_t  bufSize = _blockSize * _blockAmount;

    /**
    *   申请内存
    */
    memset(_curBuf, 0, bufSize);

    /**
    *   链表头和尾部指向同一位置
    */
    _usableBlockHeader = reinterpret_cast<MemoryBlock*>(_curBuf);
    _usableBlockHeader->_ref = 0;
    _usableBlockHeader->_alloctor = this;
    _usableBlockHeader->_nextBlock = 0;
    _usableBlockHeader->_isInPool = true;
    MemoryBlock *temp = _usableBlockHeader;

    // 构建内存块链表
    for(size_t i = 1; i < _blockAmount; ++i)
    {
        char *cache = (_curBuf + _blockSize * i);
        MemoryBlock *block = reinterpret_cast<MemoryBlock*>(cache);
        block->_ref = 0;
        block->_isInPool = true;
        block->_alloctor = this;
        block->_nextBlock = 0;
        temp->_nextBlock = block;
        temp = block;
    }
}

void IMemoryAlloctor::PrintMemInfo() const
{
    // 单独的内存池日志 [当前内存池占用内存情况,内存池使用情况]
    g_Log->mempool("blockSize[%llu] nodecnt[%lld],total bytes occupied[%lld],memblock in used bytes[%lld]"
                   ,_blockSize, (Int64)_curNodeCnt, _curNodeCnt*_blockAmount*_blockSize, _memBlockInUse*_blockSize);
}

void IMemoryAlloctor::InitMemory()
{
    // 初始化头节点
    _header = new MemBlocksNode(_blockSize * _blockAmount);
    _lastNode = _header;
    ++_curNodeCnt;

    _InitNode(_header);
}

void IMemoryAlloctor::FinishMemory()
{
    if(_isFinish)
        return;

    _isFinish = true;

    // free链表中非内存池分配的内存
    if(_lastDeleted)
    {
        auto node = _lastDeleted;
        while(node)
        {
            auto curNode = node;
            node = node->_nextBlock;
            if(!curNode->_isInPool)
                ::free(curNode);
        }
    }

    if(_header)
    {
        auto node = _header;
        while(node)
        {
            auto curNode = node;
            node = node->_next;
            Fs_SafeFree(curNode);
        }
        _header = NULL;
    }

    // 内存泄漏打印
//     if(_memBlockInUse)
//         g_Log->memleak("memory pool memleak info: amount[%lld] size[%lld];", (Int64)_memBlockInUse, _memBlockInUse*_blockSize);
}

void IMemoryAlloctor::_NewNode()
{
    // 构成链表
    auto *newNode = new MemBlocksNode(_blockSize * _blockAmount);
    _lastNode->_next = newNode;
    _lastNode = newNode;
    ++_curNodeCnt;

    _InitNode(newNode);
    if(_updateMemPoolOccupied)
        _updateMemPoolOccupied->Invoke(newNode->_nodeSize);
}

MemoryAlloctor::MemoryAlloctor(size_t blockSize, size_t blockAmount, IDelegate<void, size_t> *updateMemPoolOccupied, std::atomic<bool>  *canCreateNewNode)
    :IMemoryAlloctor(canCreateNewNode)
{
    _blockAmount = blockAmount;
    _blockSize = blockSize / __MEMORY_POOL_ALIGN_BYTES__ * __MEMORY_POOL_ALIGN_BYTES__ + (blockSize % __MEMORY_POOL_ALIGN_BYTES__ ? __MEMORY_POOL_ALIGN_BYTES__ : 0);
    _blockSize = _blockSize + sizeof(MemoryBlock);
    _effectiveBlockSize = _blockSize - sizeof(MemoryBlock);
    _updateMemPoolOccupied = updateMemPoolOccupied;
}

MemoryAlloctor::~MemoryAlloctor()
{
}

FS_NAMESPACE_END


