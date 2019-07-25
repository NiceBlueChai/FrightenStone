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
 * @file  : ObjPoolHelperImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/7/22
 * @brief :
 * 
 *
 * 
 */
#ifdef __Base_Common_ObjPool_Impl_ObjPoolHelper_H__
#pragma once

FS_NAMESPACE_BEGIN

template<typename ObjType>
inline ObjPoolHelper<ObjType>::ObjPoolHelper(size_t objAmount)
    : _objAmount(objAmount)
    ,_objAlloctorHeader(NULL)
{
    _Init();
}

template<typename ObjType>
inline ObjPoolHelper<ObjType>::~ObjPoolHelper()
{
    if(_objAlloctorHeader)
    {
#if __FS_THREAD_SAFE__
        _Lock();
#endif

        _Finish();

#if __FS_THREAD_SAFE__
        _Unlock();
#endif
    }
}

template<typename ObjType>
inline void *ObjPoolHelper<ObjType>::Alloc()
{
#if __FS_THREAD_SAFE__
    _Lock();
#endif

    // ��ͷ�ڵ�����ڴ�
    auto ptr = _objAlloctorHeader->_curAlloctor->Alloc();

    // ͷ������ڴ����򴴽��·�����������ڵ�ͷ��
    if(_objAlloctorHeader->_curAlloctor->IsEmpty())
        _SwitchToHeader(_NewAlloctorNode());

#if __FS_THREAD_SAFE__
    _Unlock();
#endif
    return  ptr;
}

template<typename ObjType>
inline void ObjPoolHelper<ObjType>::Free(void *ptr)
{
#if __FS_THREAD_SAFE__
     _Lock();
#endif

     // ͨ����ַ�ҵ����ڽڵ�
     auto node = _PtrToAlloctorNode(ptr);

     // ͨ���ڵ�ķ������ͷ��ڴ�
     node->_curAlloctor->Free(ptr);

     // �ڴ�������ڴ��ԣ��ֱ�ӷŵ�ͷ�ڵ�
     if(node->_curAlloctor->NotBusy())
         _SwitchToHeader(node);

#if __FS_THREAD_SAFE__
    _Unlock();
#endif
}

template<typename T>
inline void ObjPoolHelper<T>::AddRef(void *ptr)
{
#if __FS_THREAD_SAFE__
    _Lock();
#endif

    // ͨ����ַ�ҵ����ڽڵ�
    _PtrToAlloctorNode(ptr)->_curAlloctor->AddRef(ptr);

#if __FS_THREAD_SAFE__
    _Unlock();
#endif
}

template<typename ObjType>
inline size_t ObjPoolHelper<ObjType>::GetMemleakObjNum() const
{
    auto header = _objAlloctorHeader;
    size_t cnt = 0;
    do 
    {
        cnt += header->_curAlloctor->GetObjInUse();
        header = header->_nextNode;
    } while (header);

    return cnt;
}

template<typename ObjType>
inline void ObjPoolHelper<ObjType>::_Init()
{
    if(_objAlloctorHeader)
        return;

    _objAlloctorHeader = _NewAlloctorNode();
}

template<typename ObjType>
inline void ObjPoolHelper<ObjType>::_Finish()
{
    while(_objAlloctorHeader)
    {
        _objAlloctorHeader->_curAlloctor->FinishMemory();
        Fs_SafeFree(_objAlloctorHeader->_curAlloctor);
        auto curNode = _objAlloctorHeader;

        if(curNode->_preNode)
            curNode->_preNode->_nextNode = curNode->_nextNode;

        if(curNode->_nextNode)
            curNode->_nextNode->_preNode = curNode->_preNode;

        _objAlloctorHeader = curNode->_nextNode;
        Fs_SafeFree(curNode);
    }
    _objAlloctorHeader = NULL;
}

template<typename ObjType>
inline void ObjPoolHelper<ObjType>::_Lock()
{
    _locker.Lock();
}

template<typename ObjType>
inline void ObjPoolHelper<ObjType>::_Unlock()
{
    _locker.Lock();
}

template<typename ObjType>
inline AlloctorNode<ObjType> *ObjPoolHelper<ObjType>::_NewAlloctorNode()
{
    // ��ʼ���µķ�����
    auto newAlloctor = new IObjAlloctor<ObjType>(new AlloctorNode<ObjType>, _objAmount);
    newAlloctor->_curNode->_curAlloctor = newAlloctor;
    newAlloctor->InitMemory();
    return newAlloctor->_curNode;
}

template<typename ObjType>
inline void ObjPoolHelper<ObjType>::_SwitchToHeader(AlloctorNode<ObjType> *node)
{
    // �Ѿ��ǵ�һ���ڵ�
    if(_objAlloctorHeader == node)
        return;

    // ��ǰ��ڵ��м����루�ж�ǰ��ڵ��Ƿ�Ϊ�գ���Ϊ�ռ����ڵ�Ϊͷ�ڵ㣩
    if(node->_preNode)
        node->_preNode->_nextNode = node->_nextNode;

    if(node->_nextNode)
        node->_nextNode->_preNode = node->_preNode;

    // ��ͷ���Խ�
    _ReplaceHeader(node);
}

template<typename ObjType>
inline void ObjPoolHelper<ObjType>::_ReplaceHeader(AlloctorNode<ObjType> *node)
{
    _objAlloctorHeader->_preNode = node;
    node->_nextNode = _objAlloctorHeader;
    _objAlloctorHeader = node;
}

template<typename ObjType>
inline AlloctorNode<ObjType> *ObjPoolHelper<ObjType>::_PtrToAlloctorNode(void *ptr)
{
    // �ڴ��ͷ
    char *ptrToFree = reinterpret_cast<char *>(ptr);
    ObjBlock<ObjType> *blockHeader = reinterpret_cast<ObjBlock<ObjType> *>(reinterpret_cast<char*>(ptrToFree - sizeof(ObjBlock<ObjType>)));
    return blockHeader->_alloctor->GetNode();
}

FS_NAMESPACE_END

#endif
