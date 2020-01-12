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
 * @file  : NetDataHeader.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2020/01/11
 * @brief :
 */
#include "stdafx.h"
#include "FrightenStone/common/net/ProtocolInterface/Impl/NetDataHeader.h"
#include "FrightenStone/common/net/ProtocolInterface/Defs/NetMsgHeaderFmtType.h"
#include "FrightenStone/common/net/ProtocolInterface/Impl/NetMsgCoder.h"
#include "FrightenStone/common/net/ProtocolInterface/Impl/NetMsgDecoder.h"

#include "FrightenStone/common/log/Log.h"

FS_NAMESPACE_BEGIN
OBJ_POOL_CREATE_DEF_IMPL(NetMsg_DataHeader, __DEF_OBJ_POOL_OBJ_NUM__);

NetMsg_DataHeader::~NetMsg_DataHeader()
{
    FS_Release(_coder);
    FS_Release(_decoder);
}

// ���л�������д�뻺����
Int64 NetMsg_DataHeader::SerializeTo(char *buffer)
{
    if(!_coder)
        _coder = new NetMsgCoder;

    // 1.�󶨻��沢��ʼ��
    _coder->BindBuffer(buffer);
    _coder->Start();
    _coder->SetCmd(_cmd);

    // 2.��������
    SerializeTo(_coder);

    // 3.����
    _coder->Finish();

    return _coder->GetTotalWriteBytes();
}

// �����绺���������л��õ���Ϣ
bool NetMsg_DataHeader::DeserializeFrom(const char *buffer)
{
    if(!_decoder)
        _decoder = new NetMsgDecoder;

    // 1.Ԥ����
    _decoder->Decode(buffer);

    // 2.ʵ�ʰ������ݶ�ȡ
    if(!DeserializeFrom(_decoder))
    {
        g_Log->w<NetMsg_DataHeader>(_LOGFMT_("cmd[%u] len[%u] DeserializeFrom fail")
                                    , _decoder->GetCmd(), _decoder->GetMsgLen());
    }

    return true;
}

FS_NAMESPACE_END
