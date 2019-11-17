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

FS_NAMESPACE_BEGIN

// objtype��blocksize������void *�ߴ��������
template<typename ObjType>
const size_t IObjAlloctor<ObjType>::_objBlockSize = __FS_MEMORY_ALIGN__(sizeof(ObjType)) + sizeof(ObjBlock<ObjType>);

template<typename ObjType>
inline IObjAlloctor<ObjType>::IObjAlloctor(size_t blockAmount)
    :_curNodeObjs(NULL)
    ,_alloctedInCurNode(0)
    ,_nodeCapacity(blockAmount)
    ,_header(NULL)
    ,_lastNode(NULL)
    ,_nodeCnt(0)
    ,_bytesOccupied(0)
    ,_objInUse(0)
    ,_lastDeleted(NULL)
    ,_objpoolAllowedMaxOccupiedBytes(__OBJ_POOL_MAX_ALLOW_BYTES_DEF__)
{
    // ��ʼ���ڵ�
    _header = new AlloctorNode<ObjType>(_nodeCapacity);
    _lastNode = _header;

    // lastnode�ڹ����г�ʼ��
    _curNodeObjs = _lastNode->_objs;
    _alloctedInCurNode = 0;
    ++_nodeCnt;
    _bytesOccupied += _nodeCapacity * _objBlockSize;

    _InitNode(_lastNode);
}

template<typename ObjType>
inline IObjAlloctor<ObjType>::~IObjAlloctor()
{
    if(_header)
    {
        auto *node = _header->_nextNode;
        while(node)
        {
            auto *nextNode = node->_nextNode;
            delete node;
            node = nextNode;
        }
        Fs_SafeFree(_header);
    }
}

template<typename ObjType>
inline void *IObjAlloctor<ObjType>::Alloc()
{
    _locker.Lock();
    auto ptr = MixAllocNoLocker();
    _locker.Unlock();
    return ptr;
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::Free(void *ptr)
{
    _locker.Lock();
    // free�Ķ��󹹳����������´η���
    MixFreeNoLocker(ptr);
    _locker.Unlock();
}

template<typename ObjType>
inline void *IObjAlloctor<ObjType>::MixAllocNoLocker()
{
    auto ptr = AllocNoLocker();
    if(ptr)
        return ptr;

    return _AllocFromSys();
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::MixFreeNoLocker(void *ptr)
{
    ObjBlock<ObjType> *ptrHeader = reinterpret_cast<ObjBlock<ObjType> *>(reinterpret_cast<char *>(reinterpret_cast<char *>(ptr) - sizeof(ObjBlock<ObjType>)));
//     std::cout << "ptrHeader=" << ptrHeader << std::endl;
//     std::cout << "ptr" << ptr << std::endl;
//     std::cout << "header" << header << std::endl;
//     std::cout << "sizeof(ObjBlock<ObjType>)" << sizeof(ObjBlock<ObjType>) << std::endl;
//     std::cout << "isInPool" << ptrHeader->_isInPool << std::endl;
    if(ptrHeader->_isInPool)
    {
        FreeNoLocker(ptr);
        return;
    }

    ::free(ptrHeader);
}

template<typename ObjType>
inline void *IObjAlloctor<ObjType>::AllocNoLocker()
{
    if(_lastDeleted)
    {
        ObjType *ptr = _lastDeleted;
        _lastDeleted = *((ObjType **)(_lastDeleted));
        ++_objInUse;
        return ptr;
    }

    // �����½ڵ�
    if(_alloctedInCurNode >= _nodeCapacity)
    {
        if(g_curObjPoolOccupiedBytes > _objpoolAllowedMaxOccupiedBytes)
            return NULL;

        _NewNode();
    }

    // �ڴ���з������
    auto ptr = reinterpret_cast<char *>(_curNodeObjs) + _alloctedInCurNode * _objBlockSize;
    ++_alloctedInCurNode;
    ++_objInUse;

    // ptr��ͷ����ʼ��Ϣ
    return ptr + sizeof(ObjBlock<ObjType>);
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::FreeNoLocker(void *ptr)
{
    // free�Ķ��󹹳����������´η���
    *((ObjType **)ptr) = _lastDeleted;
    _lastDeleted = reinterpret_cast<ObjType *>(ptr);
    --_objInUse;
}

template<typename ObjType>
template<typename... Args>
inline ObjType *IObjAlloctor<ObjType>::New(Args &&... args)
{
    return ::new(MixAllocNoLocker())ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
inline ObjType *IObjAlloctor<ObjType>::NewWithoutConstruct()
{
    return  reinterpret_cast<ObjType *>(MixAllocNoLocker());
}

template<typename ObjType>
template<typename... Args>
inline ObjType *IObjAlloctor<ObjType>::NewByPtr(void *ptr, Args &&... args)
{
    return ::new(ptr)ObjType(std::forward<Args>(args)...);
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::Delete(ObjType *ptr)
{
    // ���������ͷ�
    ptr->~ObjType();

    MixFreeNoLocker(ptr);
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::DeleteWithoutDestructor(ObjType *ptr)
{
    MixFreeNoLocker(ptr);
}

template<typename ObjType>
inline size_t IObjAlloctor<ObjType>::GetObjInUse()
{
#if __FS_THREAD_SAFE__
    _locker.Lock();
    auto cnt = _objInUse;
    _locker.Unlock();

    return cnt;
#else
    return _objInUse;
#endif
}

template<typename ObjType>
inline size_t IObjAlloctor<ObjType>::GetTotalObjBlocks()
{
#if __FS_THREAD_SAFE__
    _locker.Lock();
    auto cnt = _nodeCnt * _nodeCapacity;
    _locker.Unlock();

    return cnt;
#else
    return _nodeCnt * _nodeCapacity;
#endif
}

template<typename ObjType>
inline size_t IObjAlloctor<ObjType>::GetNodeCnt()
{
#if __FS_THREAD_SAFE__
    _locker.Lock();
    auto cnt = _nodeCnt;
    _locker.Unlock();

    return cnt;
#else
    return _nodeCnt;
#endif
}

template<typename ObjType>
inline size_t IObjAlloctor<ObjType>::GetBytesOccupied()
{
#if __FS_THREAD_SAFE__
    _locker.Lock();
    auto cnt = _bytesOccupied;
    _locker.Unlock();

    return cnt;
#else
    return _bytesOccupied;
#endif
}

template<typename ObjType>
inline size_t IObjAlloctor<ObjType>::GetObjBlockSize()
{
    return _objBlockSize;
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::Lock()
{
    _locker.Lock();
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::UnLock()
{
    _locker.Unlock();
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::SetAllowMaxOccupiedBytes(UInt64 maxBytes)
{
    _objpoolAllowedMaxOccupiedBytes = maxBytes;
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::_NewNode()
{
    // ��������
    auto *newNode = new AlloctorNode<ObjType>(_nodeCapacity);

    // lastnode�ڹ����г�ʼ��
    _lastNode->_nextNode = newNode;
    _lastNode = newNode;
    _curNodeObjs = newNode->_objs;
    _alloctedInCurNode = 0;
    ++_nodeCnt;
    _bytesOccupied += _nodeCapacity * _objBlockSize;

    _InitNode(newNode);
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::_InitNode(AlloctorNode<ObjType> *newNode)
{
    auto curBuff = reinterpret_cast<char *>(newNode->_objs);

    /**
    *   �����ڴ�
    */
    // memset(_curBuf, 0, newNode->_nodeSize);

    // �����ڴ������
    for(size_t i = 0; i < _nodeCapacity; ++i)
    {
        char *cache = (curBuff + _objBlockSize * i);
        ObjBlock<ObjType> *block = reinterpret_cast<ObjBlock<ObjType> *>(cache);
        block->_isInPool = true;
    }
}

template<typename ObjType>
inline void * IObjAlloctor<ObjType>::_AllocFromSys()
{
    char *ptr = reinterpret_cast<char *>(::malloc(_objBlockSize));
    reinterpret_cast<ObjBlock<ObjType> *>(ptr)->_isInPool = false;
    return ptr + sizeof(ObjBlock<ObjType>);
}

FS_NAMESPACE_END

#endif
