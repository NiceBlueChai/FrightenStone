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
 * @file  : FS_NetBuffer.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/5
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "base/common/net/Defs/FS_NetBuffer.h"
#include "base/common/net/Defs/FS_NetDefs.h"
#include "base/common/log/Log.h"
#include "base/common/net/protocol/protocol.h"
#include "base/common/net/Defs/IocpDefs.h"
#include "base/common/memorypool/memorypool.h"
#include "base/common/net/Defs/FS_NetBufferArray.h"
#include "base/common/crashhandle/CrashHandle.h"

FS_NAMESPACE_BEGIN

OBJ_POOL_CREATE_IMPL(FS_NetBuffer, _objPoolHelper, __DEF_OBJ_POOL_OBJ_NUM__)

FS_NetBuffer::FS_NetBuffer(FS_NetBufferArray *owner, Int32 sizeBuffer)
{
    _owner = owner;
    g_MemoryPool->Lock();
    _buff = reinterpret_cast<char *>(g_MemoryPool->Alloc(sizeBuffer));
    g_MemoryPool->Unlock();
    _buffSize = sizeBuffer;
}

FS_NetBuffer::~FS_NetBuffer()
{
    g_MemoryPool->Lock();
    // ��ӡ��ջ
    //g_Log->net<FS_NetBuffer>("FS_NetBuffer will free stack trace back:\n%s", CrashHandleUtil::FS_CaptureStackBackTrace().c_str());
    g_MemoryPool->Free(_buff);
    g_MemoryPool->Unlock();
}

bool FS_NetBuffer::Push(const char *data, Int32 len)
{
    // // д��������ݲ�һ��Ҫ�ŵ��ڴ���
    // //Ҳ���Դ洢�����ݿ���ߴ��̵ȴ洢����
    // if (_nLast + nLen > _nSize)
    // {
    //  //��Ҫд������ݴ��ڿ��ÿռ�
    //  int n = (_nLast + nLen) - _nSize;
    //  //��չBUFF
    //  if (n < 8192)
    //     n = 8192;
    //  char* buff = new char[_nSize+n];
    //  memcpy(buff, _pBuff, _nLast);
    //  delete[] _pBuff;
    //  _pBuff = buff;
    //}

    if(_lastPos + len <= _buffSize)
    {
        //��Ҫ���͵����� ���������ͻ�����β��
        memcpy(_buff + _lastPos, data, len);
        //��������β��λ��
        _lastPos += len;

        if(_lastPos == SEND_BUFF_SZIE)
            ++_fullCount;
        
        return true;
    }
    else {
        ++_fullCount;
    }

    return false;
}

void FS_NetBuffer::Pop(Int32 len)
{
    Int32 n = _lastPos - len;
    if(n > 0)
        memcpy(_buff, _buff + len, n);
    else if(n < 0)
    {
        n = 0;
        g_Log->e<FS_NetBuffer>(_LOGFMT_("n is negative lastPos[%d] len[%d]"),
                               _lastPos, len);
    }

    _lastPos = n;
    if(_fullCount > 0)
        --_fullCount;
}

Int32 FS_NetBuffer::Write2socket(SOCKET sockfd)
{
    Int32 ret = 0;
    // ������������
    if(_lastPos > 0 && INVALID_SOCKET != sockfd)
    {
        //��������
        ret = send(sockfd, _buff, _lastPos, 0);
        if(ret <= 0)
        {
            g_Log->e<FS_NetBuffer>(_LOGFMT_("write2socket1:sockfd<%d> nSize<%d> lastPos<%d> ret<%d>")
                              , sockfd, _buffSize, _lastPos, ret);

            return SOCKET_ERROR;
        }

        if(ret == _lastPos)
        {// _last=2000 ʵ�ʷ���ret=2000
            // ����β��λ������
            _lastPos = 0;
        }
        else {
            // _nLast=2000 ʵ�ʷ���ret = 1000
            // CELLLog_Info("write2socket2:sockfd<%d> nSize<%d> nLast<%d> ret<%d>", sockfd, _nSize, _nLast, ret);
            _lastPos -= ret;
            memcpy(_buff, _buff + ret, _lastPos);
        }

        _fullCount = 0;
    }
    return ret;
}

Int32 FS_NetBuffer::ReadFromSocket(SOCKET sockfd)
{
    if(_buffSize - _lastPos > 0)
    {
        // ���տͻ�������
        char *toRecv = _buff + _lastPos;
        Int32 len = (Int32)recv(sockfd, toRecv, _buffSize - _lastPos, 0);
        if(len <= 0)
        {
            g_Log->e<FS_NetBuffer>(_LOGFMT_("ReadFromSocket:sockfd<%d> buffSize<%d> lastPos<%d> nen<%d>")
                                   , sockfd, _buffSize, _lastPos, len);
            return SOCKET_ERROR;
        }

        // ��Ϣ������������β��λ�ú���
        _lastPos += len;
        return len;
    }

    return 0;
}

bool FS_NetBuffer::HasMsg() const
{
    // �ж���Ϣ�����������ݳ��ȴ�����ϢͷNetMsg_DataHeader����
    if(_lastPos >= sizeof(NetMsg_DataHeader))
    {
        // ��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
        NetMsg_DataHeader *header = (NetMsg_DataHeader*)_buff;
        // �ж���Ϣ�����������ݳ��ȴ�����Ϣ����
        return _lastPos >= header->_packetLength;
    }

    return false;
}

#pragma region iocp
#ifdef FS_USE_IOCP
IoDataBase *FS_NetBuffer::MakeRecvIoData(SOCKET sockfd)
{
    int len = _buffSize - _lastPos;
    if(len > 0)
    {
        _ioData._wsaBuff.buf = _buff + _lastPos;
        _ioData._wsaBuff.len = len;
        _ioData._sock = sockfd;
        return &_ioData;
    }

    return NULL;
}

IoDataBase *FS_NetBuffer::MakeSendIoData(SOCKET sockfd)
{
    if(_lastPos > 0)
    {
        _ioData._wsaBuff.buf = _buff;
        _ioData._wsaBuff.len = _lastPos;
        _ioData._sock = sockfd;
        return &_ioData;
    }

    return NULL;
}

bool FS_NetBuffer::OnReadFromIocp(int recvBytes)
{// ��iocp����buffer��buffer����������Ӧ�ֽ�
    if(recvBytes > 0 && _buffSize - _lastPos >= recvBytes)
    {
        _lastPos += recvBytes;
        return true;
    }
    
    g_Log->e<FS_NetBuffer>(_LOGFMT_("ReadFromBuffer:sockfd<%d> _buffSize<%d> _lastPos<%d> recvBytes<%d>")
                           , _ioData._sock, _buffSize, _lastPos, recvBytes);
    return false;
}

bool FS_NetBuffer::OnWrite2Iocp(int sendBytes)
{// д��iocp��ɶ����ֽ���buffer��Ӧ���ٶ����ֽ�
    if(_lastPos < sendBytes)
    {
        g_Log->e<FS_NetBuffer>(_LOGFMT_("Write2Buffer:sockfd<%d> _buffSize<%d> _lastPos<%d> sendBytes<%d>")
                               , _ioData._sock, _buffSize, _lastPos, sendBytes);
        return false;
    }

    if(_lastPos == sendBytes)
    {// _lastPos=2000 ʵ�ʷ���sendBytes=2000
     // ����β��λ������
        _lastPos = 0;
    }else {
        // _lastPos=2000 ʵ�ʷ���ret=1000
        _lastPos -= sendBytes;
        memcpy(_buff, _buff + sendBytes, _lastPos);
    }

    _fullCount = 0;
    return true;
}
#endif
#pragma endregion

FS_NAMESPACE_END
