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

// objtype��blocksize������void *�ߴ��������
template<typename ObjType>
const size_t IObjAlloctor<ObjType>::_objBlockSize = sizeof(ObjType) / sizeof(void *) * sizeof(void *) + 
(sizeof(ObjType) % sizeof(void *) ? sizeof(void *) : 0);

template<typename ObjType>
inline IObjAlloctor<ObjType>::IObjAlloctor(size_t blockAmount)
    :_curNodeObjs(NULL)
    ,_lastDeleted(NULL)
    ,_alloctedInCurNode(0)
    ,_nodeCapacity(blockAmount)
    ,_header(NULL)
    ,_lastNode(NULL)
    ,_nodeCnt(0)
    ,_bytesOccupied(0)
    ,_objInUse(0)
{
    // ��ʼ���ڵ�
    _header = new AlloctorNode<ObjType>(_nodeCapacity);
    _lastNode = _header;

    // lastnode�ڹ����г�ʼ��
    _curNodeObjs = _lastNode->_objs;
    _alloctedInCurNode = 0;
    ++_nodeCnt;
    _bytesOccupied += _nodeCapacity * _objBlockSize;
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
    if(_lastDeleted)
    {
        ObjType *ptr = _lastDeleted;

        // ȡ�����һ�����ͷŶ���ĵ�ַ
        _lastDeleted = *(reinterpret_cast<ObjType **>(_lastDeleted));
        ++_objInUse;
        return ptr;
    }

    // �����½ڵ�
    if(_alloctedInCurNode >= _nodeCapacity)
        _NewNode();

    // �ڴ���з������
    char *ptr = reinterpret_cast<char *>(_curNodeObjs);
    ptr += _alloctedInCurNode * _objBlockSize;
    ++_alloctedInCurNode;
    ++_objInUse;
    return ptr;
}

template<typename ObjType>
inline void IObjAlloctor<ObjType>::Free(void *ptr)
{
    // free�Ķ��󹹳����������´η���
    *((ObjType **)ptr) = _lastDeleted;
    _lastDeleted = reinterpret_cast<ObjType *>(ptr);
    --_objInUse;
}

template<typename ObjType>
inline size_t IObjAlloctor<ObjType>::GetObjInUse() const
{
    return _objInUse;
}

template<typename ObjType>
inline size_t IObjAlloctor<ObjType>::GetTotalObjBlocks() const
{
    return _nodeCnt * _nodeCapacity;
}

template<typename ObjType>
inline size_t IObjAlloctor<ObjType>::GetNodeCnt() const
{
    return _nodeCnt;
}

template<typename ObjType>
inline size_t IObjAlloctor<ObjType>::GetBytesOccupied() const
{
    return _bytesOccupied;
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
}

FS_NAMESPACE_END

#endif
