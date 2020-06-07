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
 * @file  : NormalHandler.cpp
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/5/19
 * @brief : 
 *          ���:
                1.buffer��Ч�����Ƿ�����Զ���Э�������С,
 *              2.�����ڰ�����С��ȡ��������,�Ƚ���Ч�����Ƿ���ڰ�����,��������������һ���������ݰ�
 *              3.���Կ�ʼ����,��ð�Э��ŵ�
 */
#include "stdafx.h"
#include "Service/LogicSvc/Modules/ProtocolStack/Defs/ProtocolMsgHandler/NormalHandler.h"
#include "protocols/protocol.h"
#include "Service/Application.h"
#include "Service/LogicSvc/Modules/ProtocolStack/Interface/IProtocolMsgHandlerMgr.h"

NormalHandler::NormalHandler(const ProtocolTypeCfg *cfg, IProtocolMsgHandlerMgr *msgHandlerMgr)
    :_cfg(cfg)
    , _msgDecoder(new fs::NetMsgDecoder)
    , _msgHandlerMgr(msgHandlerMgr)
    , _dispatcher(NULL)
{
    _dispatcher = _msgHandlerMgr->GetDispatcher();
}

NormalHandler::~NormalHandler()
{
    FS_Release(_msgDecoder);
}

void NormalHandler::OnSessionMsgHandle(fs::FS_Session *session)
{// ����������success�����ֱ��ɱ��
    // �Ự������
    auto recvBuffer = session->GetRecvBuffer();
    const UInt64 sessionId = session->GetSessionId();
    auto user = g_UserMgr->GetUserBySessionId(sessionId);

    // ��������Ч���ݳ���
    auto &len = recvBuffer->GetDataLen();
    const Int64 bufferSize = recvBuffer->GetTotalSize();
    // ������ָ��
    const auto buffer = recvBuffer->GetData();
    // ת���ɰ�ͷ���ݸ�ʽ
    const fs::NetMsgHeaderFmtType::PacketLenDataType *packetLen =
        reinterpret_cast<const fs::NetMsgHeaderFmtType::PacketLenDataType *>(buffer);

    auto addr = session->GetAddr();
//     g_Log->netpackage<NormalHandler>(_LOGFMT_("before msg handle sessionId[%llu] addrinfo[%s] curbufferlen[%lld], packetlen[%u] recvBuffer raw:\n%s")
//         , sessionId, addr->ToString().c_str(), len, *packetLen, recvBuffer->ToString().c_str());

    // 1.������Ч���ݳ��ȴ��ڰ�ͷ����˵����ͷ���ݵ���
    // 2.������ȱȽϣ�����Ч���ݳ��ȱȰ����ȴ�˵�����ٴ���һ������
    // 3.һ��������Ȼ����������ǲ�������ֵģ���������ֱ��kill������
    // 4.packetLen����Ϊ0��������ѭ��
    //fs::Time s, e;
    for (; *packetLen &&
        (len >= fs::NetMsgHeaderFmtType::_msgHeaderSize) &&
        (len >= *packetLen);)
    {
        // 1.����
        if (!_msgDecoder->Decode(buffer))
        {
            g_Log->w<NormalHandler>(_LOGFMT_("Decode error sessionId[%llu]")
                , sessionId);

            g_Log->netpackage<NormalHandler>(_LOGFMT_("Decode error sessionId[%llu] addrinfo[%s] curbufferlen[%lld], packetlen[%u] recvBuffer raw:\n%s")
                , sessionId, addr->ToString().c_str(), len, *packetLen, recvBuffer->ToString().c_str());

            user->Close(UserCloseReasonType::DecodeFail);
            break;
        }

//         g_Log->net<NormalHandler>("cmd[%u] msg iscoming len[%u]"
//             , _msgDecoder->GetCmd(), _msgDecoder->GetMsgLen());

//         g_Log->netpackage<NormalHandler>(_LOGFMT_("sessionId[%llu] msgdecode info: %s")
//             , sessionId, _msgDecoder->ToString().c_str());

        // Э�鴦��ص�
        //s.FlushTime();
        if (!g_ProtocolStackMgr->InvokeProtocolHandler(sessionId, _msgDecoder))
        {
            g_Log->netpackage<NormalHandler>(_LOGFMT_("InvokeProtocolHandler error sessionId[%llu] msgdecode info: %s")
                , sessionId, _msgDecoder->ToString().c_str());

            g_Log->w<NormalHandler>(_LOGFMT_("InvokeProtocolHandler error sessionId[%llu]")
                , sessionId);
            user->Close(UserCloseReasonType::NoProtocolHandler);
        }

        //e.FlushTime();
        //Int64 useTime = (s - e).GetTotalMilliSeconds();

        // ������Ϣ
        recvBuffer->PopFront(_msgDecoder->GetMsgLen());

//         g_Log->netpackage<NormalHandler>(_LOGFMT_("sessionId[%llu]  after msg handled recvbuffer info: %s")
//             , sessionId, recvBuffer->ToString().c_str());

        // ͳ��һ����
        g_ThisApp->HandleCompEv_RecvMsgAmount();

        // ���������session���ر����˳�
        if (!session->IsValid())
            break;
    }

    // ���������ֱ��ɱ��
    if (len >= fs::NetMsgHeaderFmtType::_msgHeaderSize && *packetLen > bufferSize)
    {
        g_Log->netpackage<NormalHandler>(_LOGFMT_("net package len larger than recv buffer size sessionId[%llu] addrinfo[%s] curbufferlen[%lld], packetlen[%u] recvBuffer raw:\n%s")
            , sessionId, addr->ToString().c_str(), len, *packetLen, recvBuffer->ToString().c_str());

        g_Log->w<NormalHandler>(_LOGFMT_("sessionId[%llu] addr[%s] cur buffer data len[%lld] net package len larger than recv buffer size packetlen[%u], buffersize[%lld]")
            , sessionId, session->GetAddr()->ToString().c_str(), len, *packetLen, bufferSize);
        user->Close(UserCloseReasonType::NetPackageTooLarge);
    }
}
