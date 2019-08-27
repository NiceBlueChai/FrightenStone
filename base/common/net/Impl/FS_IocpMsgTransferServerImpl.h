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
 * @file  : FS_IocpMsgTransferServerImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/5
 * @brief :
 * 
 *
 * 
 */
#ifdef __Base_Common_Net_Impl_FS_IocpMsgTransferServer_H__
#pragma once

FS_NAMESPACE_BEGIN

inline void FS_IocpMsgTransferServer::_DelayRmClient(IO_EVENT *ioEvent, std::set<UInt64> &delayRemoveClients)
{
    FS_Client *client = reinterpret_cast<FS_Client *>(_ioEvent->_data._ptr);
    if(!client)
    {
        g_Log->e<FS_IocpMsgTransferServer>(_LOGFMT_("_ioEvent->_data._ptr is null"));
        return;
    }

    // ���ߴ���
    delayRemoveClients.insert(client->GetId());
}

FS_NAMESPACE_END

#endif
