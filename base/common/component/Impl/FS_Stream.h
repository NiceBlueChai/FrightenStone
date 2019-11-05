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
 * @file  : FS_Stream.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/9
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_FS_Stream_H__
#define __Base_Common_Component_Impl_FS_Stream_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/objpool/objpool.h"
#include "base/common/component/Impl/FS_String.h"
#include "base/common/log/Log.h"
#include "base/common/memorypool/memorypool.h"

FS_NAMESPACE_BEGIN

// �ֽ���д�������д��uint32�����ݳ��ȣ���д������
// �ֽ�����ȡ�����ȶ�ȡuint32�����ݳ��ȣ��ٶ�ȡ����
// �������ڴ�ش����Ļ���������д��ʱ֧�ֻ�����������
class BASE_EXPORT FS_Stream
{
    OBJ_POOL_CREATE(FS_Stream, _objPoolHelper);
public:
    FS_Stream();
    FS_Stream(Byte8 *data, Int32 size, bool needDelete = false, bool isPoolCreate = false);
    FS_Stream(Int32 size);
    virtual ~FS_Stream();
    virtual void Release();

    /* ���� */
    #pragma region assist
    /*
    *   brief:
    *       1. - GetDataAddr ��ȡ���ݻ����������ĵ�ַ���������ݻ��������Զ���չ�ռ�������Ҫȡ�ÿռ����ڵ�ַ��
    *       2. - GetWrLength ��д�����ݻ������ĳ���
    *       3. - CanRead ���ܶ���len�ֽڵ�������?
    *       4. - CanWrite ����д��len�ֽڵ�������?
    *       5. - OffsetWrLenOnWrChange ��д��λ�ã�д��λ��_writePos ƫ��len�ֽڳ���
    *       6. - OffsetOnReadChange �Ѷ�ȡλ�ã���ȡλ�� _readPos ƫ��len�ֽڳ���
    *       7. - SetWritePos ����д��λ��_writePos
    */
public:
    char **GetDataAddr();
    Int32 GetWrLength() const;
    bool CanRead(Int32 len) const;
    bool CanWrite(Int32 len) const;
    void OffsetWrLenOnWrChange(Int32 len);
    void OffsetOnReadChange(Int32 len);
    void SetWritePos(Int32 n);
    template<typename StreamObjType>
    StreamObjType *CastTo();
    template<typename StreamObjType>
    const StreamObjType *CastTo() const;

    // ���л������л���ObjType ������bool SerializeTo(FS_Stream *) const, bool DeserializeFrom(FS_Stream *)�ӿڣ�
    template<typename ObjType>
    bool SerializeFrom(const ObjType &obj);
    template<typename ObjType>
    bool DeserializeTo(ObjType &obj);

    // �ֽ������л������л�
    bool SerializeTo(FS_String &str) const;
    bool DeserializeFrom(const FS_String &str);
    #pragma endregion

    /* ���ֽ��� */
    #pragma region Read
public:
    template<typename ObjType>
    bool Read(ObjType &n, bool isOffset = true);
    template<typename ObjType>
    UInt32 ReadArray(ObjType *arr, UInt32 len);
    template<typename ObjType>
    bool ReadWithoutOffsetPos(ObjType &n);

    Byte8 ReadInt8();
    Int16 ReadInt16();
    Int32 ReadInt32();
    Int64 ReadInt64();
    U8    ReadUInt8();
    UInt16 ReadUInt16();
    UInt32 ReadUInt32();
    UInt64 ReadUInt64();
    Float ReadFloat();
    Double ReadDouble();
    bool ReadString(std::string &str);
    bool ReadString(FS_String &str);
    #pragma endregion

    /* д�ֽ��� */
    #pragma region Write
public:
    template<typename ObjType>
    bool Write(const ObjType &n);
    template<typename ObjType>
    bool WriteArray(ObjType *data, UInt32 len);
    bool Write(const FS_String &str);
    bool Write(const std::string &str);
    bool WriteInt8(Byte8 n);
    bool WriteInt16(Int16 n);
    bool WriteInt32(Int32 n);
    bool WriteFloat(Float n);
    bool WriteDouble(Double n);
    #pragma endregion

private:
    void _Clear();

private:
    // �������ܵĿռ��С���ֽڳ���
    Int32 _size = 0;
    // ��д�����ݵ�β��λ�ã���д�����ݳ���
    Int32 _writePos = 0;
    // �Ѷ�ȡ���ݵ�β��λ��
    Int32 _readPos = 0;
    // _buff���ⲿ��������ݿ�ʱ�Ƿ�Ӧ�ñ��ͷ�
    bool _needDelete = true;
    bool _isPoolCreate = false;
    // ���ݻ�����
    char *_buff = NULL;
};

FS_NAMESPACE_END

#include "base/common/component/Impl/FS_StreamImpl.h"

#endif
