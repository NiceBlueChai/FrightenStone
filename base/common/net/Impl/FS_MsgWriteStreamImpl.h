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
 * @file  : FS_MsgWriteStreamImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/08/10
 * @brief :
 * 
 *
 * 
 */
#ifdef __Base_Common_Net_Impl_FS_MsgWriteStream_H__
#pragma once

FS_NAMESPACE_BEGIN
inline FS_MsgWriteStream::FS_MsgWriteStream(char *data, int size, bool isDelete)
    :FS_Stream(data, size, isDelete)
{
    // Ԥ��ռ����Ϣ��������ռ� ����д���ֽ���ʱ�����˳���λ��
    Write<UInt16>(0);
}

inline FS_MsgWriteStream::FS_MsgWriteStream(int size)
    :FS_Stream(size)
{
    // Ԥ��ռ����Ϣ��������ռ� ����д���ֽ���ʱ�����˳���λ��
    Write<UInt16>(0);
}

#pragma region write bytes
inline void FS_MsgWriteStream::SetNetMsgCmd(UInt16 cmd)
{
    Write<UInt16>(cmd);
}

inline bool FS_MsgWriteStream::WriteString(const char *str, Int32 len)
{
    return WriteArray(str, len);
}

inline bool FS_MsgWriteStream::WriteString(const char *str)
{
    return WriteArray(str, static_cast<UInt32>(strlen(str)));
}

inline void FS_MsgWriteStream::Finish()
{// �ֽ���������Ҫˢ���ڿ�ͷ���ֽ�������

    // ��λ���ֽ���ͷ��
    Int32 pos = GetWrLength();
    SetWritePos(0);

    // д���ֽ�������
    Write<UInt16>(pos);

    // ���»�ԭ��ǰд��λ��
    SetWritePos(pos);
}
#pragma endregion

FS_NAMESPACE_END

#endif
