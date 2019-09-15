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
 * @file  : FS_ClientImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/5
 * @brief :
 * 
 *
 * 
 */
#ifdef __Base_Common_Net_Impl_FS_Client_H__
#pragma once

FS_NAMESPACE_BEGIN

inline void FS_Client::SendData(NetMsg_DataHeader *header)
{
    _SendData((const char *)header, header->_packetLength);
}

inline void FS_Client::SendData(const char *data, Int32 len)
{
    _SendData(data, len);
}

inline void FS_Client::SetSender(IDelegatePlus<void, FS_Packet *> *sendFunc)
{
    _sendFunc = sendFunc;
}

inline void FS_Client::UpdateHeartBeatExpiredTime()
{
    _heartBeatExpiredTime.FlushAppendTime(CLIENT_HREAT_DEAD_TIME*Time::_microSecondPerMilliSecond);
}

inline bool FS_Client::IsPostIoChange() const
{
    return _isPostRecv || _isPostSend;
}

inline bool FS_Client::IsPostSend() const
{
    return _isPostSend;
}

inline bool FS_Client::IsPostRecv() const
{
    return _isPostRecv;
}

inline SOCKET FS_Client::GetSocket() const
{
    return _sockfd;
}

inline UInt64 FS_Client::GetId() const
{
    return _id;
}

inline bool FS_Client::IsDestroy() const
{
    return _sockfd == INVALID_SOCKET;
}

inline  const Time &FS_Client::GetHeartBeatExpiredTime() const
{
    return _heartBeatExpiredTime;
}

FS_NAMESPACE_END
#endif
