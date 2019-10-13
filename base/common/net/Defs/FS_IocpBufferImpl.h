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
 * @file  : FS_IocpBufferImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/14
 * @brief :
 * 
 *
 * 
 */
#ifdef __Base_Common_Net_Defs_FS_IocpBuffer_H__

#pragma once

FS_NAMESPACE_BEGIN

inline FS_IocpBuffer::FS_IocpBuffer(size_t bufferSize)
    :IFS_Buffer(bufferSize)
    ,_sessionId(0)
    ,_socket(INVALID_SOCKET)
    ,_ioData{}
{
}

inline FS_IocpBuffer::~FS_IocpBuffer()
{
}

inline bool FS_IocpBuffer::BindTo(UInt64 sessionId, SOCKET sock)
{
    if(_sessionId)
    {
        g_Log->w<FS_IocpBuffer>(_LOGFMT_("this buffer has owner sessionId[%llu] socket[%llu] cant bind to new sessionId[%llu] new socket[%llu]")
                                , _sessionId, _socket, sessionId, sock);
        return false;
    }
    _sessionId = sessionId;
    _socket = sock;
    return true;
}

inline IoDataBase *FS_IocpBuffer::MakeRecvIoData()
{
    if(!_sessionId)
    {
        g_Log->e<FS_IocpBuffer>(_LOGFMT_("this buffer has no owner, please bind to sesion firstly"));
        return NULL;
    }

    const size_t rest = GetRest();
    if(rest == 0)
        return NULL;

    _ioData._wsaBuff.buf = GetData() + GetLength();
    _ioData._wsaBuff.len = rest;
    _ioData._sessionId = _sessionId;
    _ioData._sock = _socket;
    if(!_ioData._callback)
        _ioData._callback = DelegatePlusFactory::Create(this, &FS_IocpBuffer::_OnRecvSucCallback);

    return &_ioData;
}

inline IoDataBase *FS_IocpBuffer::MakeSendIoData()
{
    if(!_sessionId)
    {
        g_Log->e<FS_IocpBuffer>(_LOGFMT_("this buffer has no owner, please bind to sesion firstly"));
        return NULL;
    }

    auto len = GetLength();
    if(len == 0)
        return NULL;

    _ioData._wsaBuff.buf = GetData();
    _ioData._wsaBuff.len = len;
    _ioData._sessionId = _sessionId;
    _ioData._sock = _socket;
    if(!_ioData._callback)
        _ioData._callback = DelegatePlusFactory::Create(this, &FS_IocpBuffer::_OnSendSucCallback);

    return &_ioData;
}

inline bool FS_IocpBuffer::HasMsg() const
{
    // �ж���Ϣ�����������ݳ��ȴ�����ϢͷNetMsg_DataHeader����
    auto len = GetLength();
    if(len >= sizeof(NetMsg_DataHeader))
    {
        // ��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
        auto header = CastToData<NetMsg_DataHeader>();

        // �ж���Ϣ�����������ݳ��ȴ�����Ϣ����
        return len >= header->_packetLength;
    }

    return false;
}

inline bool FS_IocpBuffer::NeedWrite() const
{
    return GetLength() > 0;
}

inline UInt64 FS_IocpBuffer::GetSessionId() const
{
    return _sessionId;
}

inline SOCKET FS_IocpBuffer::GetSocket() const
{
    return _socket;
}

inline void FS_IocpBuffer::_OnSendSucCallback(size_t transferBytes)
{
    // д��iocp��ɶ����ֽ���buffer��Ӧ���ٶ����ֽ�
    auto curPos = GetLength();
    if(curPos < transferBytes)
    {
        g_Log->e<FS_IocpBuffer>(_LOGFMT_("sessionId[%llu] bufferSize<%llu> curPos<%llu> transferBytes<%llu>")
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

inline void FS_IocpBuffer::_OnRecvSucCallback(size_t transferBytes)
{
    // ��iocp����buffer��buffer����������Ӧ�ֽ�
    if(transferBytes > 0 && GetRest() >= transferBytes)
    {
        _GetCurPos() += transferBytes;
        return;
    }

    g_Log->e<FS_IocpBuffer>(_LOGFMT_("_sessionId<%llu> _bufferSize<%llu> _curPos<%llu> transferBytes<%llu>")
                            , _ioData._sessionId, GetTotalSize(), GetLength(), transferBytes);
}

FS_NAMESPACE_END

#endif
