/*!
 * MIT License
 *
 * Copyright (c) 2019 Eric Yonng<120453674@qq.com>
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
 * @file  : NetMsgCoder.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/01/11
 * @brief :
 */
#ifndef __Protocols_Impl_ProtocolInterface_Impl_NetMsgCoder_H__
#define __Protocols_Impl_ProtocolInterface_Impl_NetMsgCoder_H__
#pragma once

#include "FrightenStone/exportbase.h"
#include "protocols/ProtocolInterface/Defs/NetMsgHeaderFmtType.h"

FS_NAMESPACE_BEGIN

// ����������startʱԤ������λ,����������ֻ��start֮��ʼ,finishʱ����ڳ���λд����ʵ������
class NetMsgCoder
{
    OBJ_POOL_CREATE_DEF(NetMsgCoder);
public:
    NetMsgCoder();
    virtual ~NetMsgCoder() {}
    virtual void Release();     // ����ִ�г���ĶѲ�ͬ�ͷ���ʹ��release����

    // ����������������Ӧ���ⲿ����д��д��,������ֻ����Ϊ���,������ȫ���
    // ����:start->write->finish,...(���ж����Ϣ,����ѭ��start->write->finish����)
public:
    void BindBuffer(Byte8 *msgBuffer, UInt64 bufferSize);
    void BindBuffer(FS_String *msgBuffer);
    template<typename BufferType>
    bool Start();
    template<typename BufferType>
    bool SetCmd(UInt32 cmd);
    template<typename DataType>
    bool WriteByte(DataType &data);     // �뱣֤DataTypeΪ������������д��,��֤�ڴ��ǽ��յ�,��������ڴ��������⵼��Զ�̽��ʧ��
    bool WriteByte(const Byte8 *data, UInt64 dataLen);
    template<typename DataType>
    void WriteToString(DataType &data);     // �뱣֤DataTypeΪ������������д��,��֤�ڴ��ǽ��յ�,��������ڴ��������⵼��Զ�̽��ʧ��
    void WriteToString(const Byte8 *data, UInt64 dataLen);
    // ��������
    template<typename BufferType>
    void Finish();
    bool IsBufferNotEnough() const;

    // д����ܳ���
    UInt64 GetTotalWriteBytes() const;
    // ��ȡbufferʣ��Ŀռ�

protected:
    bool _CheckSizeAndMaskNotEnough(size_t appendSize);
    bool _CheckSizeNoMaskNotEnough(size_t appendSize) const;

protected:
    void *_buffer;             // �ӹܵĻ�����,����黺������С���ⲿ���м�ⰲȫ����д�뻺��
    UInt64 _bufferSize;          // ��������byte8���͵�����Ч
    UInt64 _curHeaderPos;       // ��ʼλ��
    UInt64 _wrLen;              // ��ǰ��Ϣд�볤��
    bool _isBufferNotEnough;    // �����Ƿ��㻺����byte8������Ч
};

FS_NAMESPACE_END

#include "protocols/ProtocolInterface/Impl/NetMsgCoderImpl.h"

#endif
