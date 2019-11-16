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
 * @file  : IFS_Buffer.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/13
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Defs_IFS_Buffer_H__
#define __Base_Common_Net_Defs_IFS_Buffer_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/memorypool/memorypool.h"
#include "base/common/log/Log.h"

FS_NAMESPACE_BEGIN

class BASE_EXPORT IFS_Buffer
{
public:
    explicit IFS_Buffer(size_t bufferSize);
    virtual ~IFS_Buffer();

public:
    bool BindTo(UInt64 sessionId, SOCKET sock);
    UInt64 GetSessionId() const;
    SOCKET GetSocket() const;

    void PopFront(size_t bytesLen);
    bool PushBack(const Byte8 *data, size_t len);
    bool CanPush(size_t len);
    void Clear();
    bool IsEmpty() const;
    bool IsFull() const;
    size_t GetLength() const;
    size_t GetTotalSize() const;
    size_t GetRest() const;
    char *GetData();
    const char *GetData() const;

    template<typename ObjType>
    ObjType *CastToData();
    template<typename ObjType>
    const ObjType *CastToData() const;
    template<typename ObjType>
    ObjType *CastToBuffer();
    template<typename ObjType>
    const ObjType *CastToBuffer() const;
    
protected:
    size_t &_GetCurPos();

private:
    void _Init();
    void _Destroy();

protected:
    UInt64 _sessionId;      // FS_Packet���������ڲ����
    SOCKET _socket;         // FS_Packet���������ڲ����

private:
    size_t _bufferSize;
    char *_buff;
    size_t _curPos;     // ��ǰ����ĩβλ��
};

FS_NAMESPACE_END

#include "base/common/net/Defs/IFS_BufferImpl.h"

#endif
