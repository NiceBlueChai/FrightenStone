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
 * @file  : FS_EpollBufferImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/02/02
 * @brief :
 */
#ifdef __Frame_Include_FrightenStone_Common_Net_Defs_FS_EpollBuffer_H__
#pragma once

#ifndef _WIN32

FS_NAMESPACE_BEGIN

inline FS_EpollBuffer::FS_EpollBuffer(size_t bufferSize, IMemoryAlloctor *memAlloctor)
    :IFS_Buffer(bufferSize, memAlloctor)
    , _ioData{}
{
}

inline FS_EpollBuffer::~FS_EpollBuffer()
{
}

inline EpollIoData *FS_EpollBuffer::MakeRecvIoData()
{
    if(!_sessionId)
    {
        g_Log->e<FS_EpollBuffer>(_LOGFMT_("this buffer has no owner, please bind to sesion firstly"));
        return NULL;
    }

    const size_t rest = GetRest();
    if(rest == 0)
        return NULL;

    _ioData._data = GetData() + GetLength();
    _ioData._dateLen = rest;
    _ioData._sessionId = _sessionId;
    _ioData._sock = _socket;
    if(!_ioData._callback)
        _ioData._callback = DelegatePlusFactory::Create(this, &FS_EpollBuffer::_OnRecvSucCallback);

    return &_ioData;
}

inline EpollIoData *FS_EpollBuffer::MakeSendIoData()
{
    if(!_sessionId)
    {
        g_Log->e<FS_EpollBuffer>(_LOGFMT_("this buffer has no owner, please bind to sesion firstly"));
        return NULL;
    }

    auto len = GetLength();
    if(len == 0)
    {
        g_Log->w<FS_EpollBuffer>(_LOGFMT_("this buffer have no data to send sessionId[%llu]"), _sessionId);
        return NULL;
    }

    _ioData._data = GetData();
    _ioData._dateLen = len;
    _ioData._sessionId = _sessionId;
    _ioData._sock = _socket;
    if(!_ioData._callback)
        _ioData._callback = DelegatePlusFactory::Create(this, &FS_EpollBuffer::_OnSendSucCallback);

    return &_ioData;
}

inline bool FS_EpollBuffer::HasMsg() const
{
    // �ж���Ϣ�����������ݳ��ȴ�����ϢͷNetMsg_DataHeader����
    auto len = GetLength();
    if(len >= NetMsgHeaderFmtType::_msgHeaderSize)
    {
        // ��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
        const NetMsgHeaderFmtType::PacketLenDataType *packetLen =
            reinterpret_cast<const NetMsgHeaderFmtType::PacketLenDataType *>(_buff);

        // �ж���Ϣ�����������ݳ��ȴ�����Ϣ����
        return len >= *packetLen;
    }

    return false;
}

inline bool FS_EpollBuffer::NeedWrite() const
{
    return GetLength() > 0;
}

inline FS_String FS_EpollBuffer::ToString() const
{
    FS_String info;
    info.AppendFormat("%s\n", IFS_Buffer::ToString().c_str())
        .AppendFormat("ioData info: _sessionId[%llu], _sock[%d], _data[0x%p], _datalen[%lld]"
                      , _ioData._sessionId, _ioData._sock, _ioData._data, _ioData._dateLen);
    return info;
}

inline void FS_EpollBuffer::_OnSendSucCallback(Int64 transferBytes)
{
    // д��iocp��ɶ����ֽ���buffer��Ӧ���ٶ����ֽ�
    auto curPos = GetLength();
    if(curPos < transferBytes)
    {
        g_Log->e<FS_EpollBuffer>(_LOGFMT_("sessionId[%llu] bufferSize<%llu> curPos<%llu> transferBytes<%lld>")
                                , _ioData._sessionId, GetTotalSize(), curPos, transferBytes);
        return;
    }

    if(curPos == transferBytes)
    {// _lastPos=2000 ʵ�ʷ���sendBytes=2000
     // ����β��λ������
        Clear();
    }
    else {
        // _lastPos=2000 ʵ�ʷ���ret=1000
        PopFront(transferBytes);
    }
}

inline void FS_EpollBuffer::_OnRecvSucCallback(Int64 transferBytes)
{
    // ��iocp����buffer��buffer����������Ӧ�ֽ�
    if(transferBytes > 0 && GetRest() >= transferBytes)
    {
        _GetCurPos() += transferBytes;
        return;
    }

    g_Log->e<FS_EpollBuffer>(_LOGFMT_("_sessionId<%llu> _bufferSize<%llu> _curPos<%llu> transferBytes<%llu>")
                            , _ioData._sessionId, GetTotalSize(), GetLength(), transferBytes);
}


FS_NAMESPACE_END

#endif

#endif
