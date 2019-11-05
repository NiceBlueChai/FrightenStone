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
 * @file  : FS_StreamImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/9
 * @brief :
 * 
 *
 * 
 */
#ifdef __Base_Common_Component_Impl_FS_Stream_H__
#pragma once

FS_NAMESPACE_BEGIN
inline FS_Stream::FS_Stream()
    :_buff(NULL)
    ,_size(0)
    ,_writePos(0)
    ,_readPos(0)
    ,_needDelete(false)
    ,_isPoolCreate(false)
{
}

inline FS_Stream::FS_Stream(Byte8 *data, Int32 size, bool needDelete, bool isPoolCreate)
    :_buff(data)
    , _size(size)
    , _needDelete(needDelete)
    ,_isPoolCreate(isPoolCreate)
{
}

#pragma region assist
inline char ** FS_Stream::GetDataAddr()
{
    return &_buff;
}

inline Int32 FS_Stream::GetWrLength() const
{
    return _writePos;
}

inline bool FS_Stream::CanRead(Int32 len) const
{
    return _size - _readPos >= len;
}

inline bool FS_Stream::CanWrite(Int32 len) const
{
     return _size - _writePos >= len;
}

inline void FS_Stream::OffsetWrLenOnWrChange(Int32 len)
{
     _writePos += len;
}

inline void FS_Stream::OffsetOnReadChange(Int32 len)
{
    _readPos += len;
}

inline void FS_Stream::SetWritePos(Int32 n)
{
    _writePos = n;
}

template<typename StreamObjType>
inline StreamObjType *FS_Stream::CastTo()
{
    return reinterpret_cast<StreamObjType *>(this);
}

template<typename StreamObjType>
inline const StreamObjType *FS_Stream::CastTo() const
{
    return reinterpret_cast<StreamObjType *>(this);
}

template<typename ObjType>
inline bool FS_Stream::SerializeFrom(const ObjType &obj)
{
    return obj.SerializeTo(this);
}

template<typename ObjType>
inline bool FS_Stream::DeserializeTo(ObjType &obj)
{
    return obj.DeserializeFrom(this);
}

inline bool FS_Stream::SerializeTo(FS_String &str) const
{
    // ��д������ռ�ÿռ�
    str.AppendBitData(reinterpret_cast<const char *>(&_size), sizeof(_size));
    str.AppendBitData(reinterpret_cast<const char *>(&_writePos), sizeof(_writePos));
    str.AppendBitData(reinterpret_cast<const char *>(&_readPos), sizeof(_readPos));
    str.AppendBitData(reinterpret_cast<const char *>(&_needDelete), sizeof(_needDelete));
    str.AppendBitData(reinterpret_cast<const char *>(&_isPoolCreate), sizeof(_isPoolCreate));

    // ����Ч���ݿ���str
    if(_buff)
    {
        auto validSize = _writePos > 0 ? _writePos : _size;
        if(validSize)
            str.AppendBitData(_buff, validSize);
    }

    return !str.empty();
}


#pragma endregion

#pragma region read
template<typename ObjType>
inline bool FS_Stream::Read(ObjType &n, bool isOffset)
{
    // ����Ҫ��ȡ���ݵ��ֽڳ���
    Int32 objSize = static_cast<Int32>(sizeof(ObjType));

    // �ж��ܲ��ܶ�
    if(CanRead(objSize))
    {
        // ��Ҫ��ȡ������ ��������
        memcpy(&n, _buff + _readPos, objSize);

        // ƫ���Ѷ�����λ��
        if(isOffset)
            OffsetOnReadChange(objSize);

        return true;
    }

    g_Log->e<FS_Stream>(_LOGFMT_("error, Cant Read. struct name[%s]."), typeid(ObjType).name());
    return false;
}

template<typename ObjType>
inline UInt32 FS_Stream::ReadArray(ObjType *arr, UInt32 len)
{
    UInt32 elementCnt = 0;

    // ��ȡ����Ԫ�ظ���,����ƫ�ƶ�ȡλ��
    Read(elementCnt, false);

    // �жϻ��������ܷ�ŵ���
    if(elementCnt <= len)
    {
        // ����������ֽڳ���
        Int32 bytes = static_cast<Int32>(elementCnt * sizeof(ObjType));

        // �ж��ܲ��ܶ���
        if( CanRead(static_cast<Int32>(bytes + sizeof(UInt32))) )
        {
            // �����Ѷ�λ��+���鳤����ռ�пռ�
            OffsetOnReadChange(static_cast<Int32>(sizeof(UInt32)));

            // ��Ҫ��ȡ������ ��������
            memcpy(arr, _buff + _readPos, static_cast<size_t>(bytes));

            // �����Ѷ�����λ��
            OffsetOnReadChange(static_cast<Int32>(bytes));
            return elementCnt;
        }
    }

    g_Log->e<FS_Stream>(_LOGFMT_("error,ReadArray failed. element name[%s]."), typeid(ObjType).name());
    return 0;
}

template<typename ObjType>
inline bool FS_Stream::ReadWithoutOffsetPos(ObjType &n)
{
    return Read(n, false);
}

inline Byte8 FS_Stream::ReadInt8()
{
    Byte8 def = 0;
    Read(def);
    return def;
}

inline Int16 FS_Stream::ReadInt16()
{
    Int16 n = 0;
    Read(n);
    return n;
}


inline Int32 FS_Stream::ReadInt32()
{
    Int32 n = 0;
    Read(n);
    return n;
}

inline Int64 FS_Stream::ReadInt64()
{
    Int64 n = 0;
    Read(n);
    return n;
}

inline U8 FS_Stream::ReadUInt8()
{
    U8 def = 0;
    Read(def);
    return def;
}

inline UInt16 FS_Stream::ReadUInt16()
{
    UInt16 n = 0;
    Read(n);
    return n;
}

inline UInt32 FS_Stream::ReadUInt32()
{
    UInt32 n = 0;
    Read(n);
    return n;
}

inline UInt64 FS_Stream::ReadUInt64()
{
    UInt64 n = 0;
    Read(n);
    return n;
}

inline Float FS_Stream::ReadFloat()
{
    Float n = 0.0f;
    Read(n);
    return n;
}

inline Double FS_Stream::ReadDouble()
{
    Double n = 0.0;
    Read(n);
    return n;
}

inline bool FS_Stream::ReadString(FS_String &str)
{
    return ReadString(str.GetRaw());
}

#pragma endregion

#pragma region Write

template<typename ObjType>
inline bool FS_Stream::Write(const ObjType &n)
{
    // ����Ҫд�����ݵ��ֽڳ���
    Int32 len = static_cast<Int32>(sizeof(ObjType));

    bool canWrite = CanWrite(len);
    if(!canWrite && !_isPoolCreate)
    {// ����д���Ҳ����ڴ�ش����Ļ�����
        g_Log->e<FS_Stream>(_LOGFMT_("Write failed. obj name[%s]"), typeid(ObjType).name());
        return false;
    }
    else if(!canWrite)
    {// ����д�������ڴ�ش����Ļ�������������Ϊ2��
        g_MemoryPool->Lock();
        _buff = reinterpret_cast<char *>(g_MemoryPool->Realloc(_buff, 2 * _size));
        g_MemoryPool->Unlock();
        _size *= 2;
    }

    // ��Ҫд������� ������������β��
    memcpy(_buff + _writePos, &n, static_cast<size_t>(len));

    // ������д������β��λ��
    OffsetWrLenOnWrChange(len);
    return true;
}

template<typename ObjType>
inline bool FS_Stream::WriteArray(ObjType *data, UInt32 len)
{
    // ����Ҫд��������ֽڳ���
    auto bytes = sizeof(ObjType)*len;

    // �ж��ܲ���д��
    bool canWrite = CanWrite(static_cast<Int32>(bytes + sizeof(UInt32)));
    if(!canWrite && !_isPoolCreate)
    {// ����д���Ҳ����ڴ�ش����Ļ�����
        g_Log->e<FS_Stream>(_LOGFMT_("WriteArray failed. obj name[%s]"), typeid(ObjType).name());
        return false;
    }
    else if(!canWrite)
    {// ����д�������ڴ�ش����Ļ�������������Ϊ2��
        g_MemoryPool->Lock();
        _buff = reinterpret_cast<char *>(g_MemoryPool->Realloc(_buff, 2 * _size));
        g_MemoryPool->Unlock();
        _size *= 2;
    }

    // ��д�������Ԫ�س���
    Write(len);

    // ��Ҫд������� ������������β��
    memcpy(_buff + _writePos, data, bytes);

    // ��������β��λ��
    OffsetWrLenOnWrChange(static_cast<Int32>(bytes));
    return true;
}

inline bool FS_Stream::Write(const FS_String &str)
{
    return Write(str.GetRaw());
}

inline bool FS_Stream::Write(const std::string &str)
{
    return WriteArray(str.c_str(), static_cast<UInt32>(str.size()));
}

inline bool FS_Stream::WriteInt8(Byte8 n)
{
    return Write(n);
}

inline bool FS_Stream::WriteInt16(Int16 n)
{
    return Write(n);
}

inline bool FS_Stream::WriteInt32(Int32 n)
{
    return Write(n);
}

inline bool FS_Stream::WriteFloat(Float n)
{
    return Write(n);
}

inline bool FS_Stream::WriteDouble(Double n)
{
    return Write(n);
}

#pragma endregion

FS_NAMESPACE_END

#endif
