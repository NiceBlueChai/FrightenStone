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
 * @file  : FS_ServerImpl.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/3
 * @brief :
 * 
 *
 * 
 */
#ifdef __Base_Common_Net_Impl_FS_Server_H__
#pragma once

FS_NAMESPACE_BEGIN
#pragma region misc
inline size_t FS_Server::GetClientCount() const
{
    return _socketRefClients.size() + _clientsCache.size();
}

inline void FS_Server::SetClientNum(Int32 socketNum)
{
}

inline void FS_Server::SetEventHandleObj(INetEvent *handleObj)
{
    _eventHandleObj = handleObj;
}

inline void FS_Server::SetId(Int32 id)
{
    _id = id;
}
#pragma endregion

#pragma region recv/addclient/start/close
inline void FS_Server::AddClient(FS_Client *client)
{
    _locker.Lock();
    _clientsCache.push_back(client);
    _locker.Unlock();
}
#pragma endregion

#pragma region net message handle
inline void FS_Server::_AddToHeartBeatQueue(FS_Client *client)
{
    _clientHeartBeatQueue.erase(client);
    _clientHeartBeatQueue.insert(client);
}

inline void FS_Server::_OnClientHeartBeatUpdate(FS_Client *client)
{
    _clientHeartBeatQueue.erase(client);
    _clientHeartBeatQueue.insert(client);
}

inline void FS_Server::_RmClient(FS_Client *client)
{
    _delayRemoveClients.insert(client->GetSocket());
}
#pragma endregion

FS_NAMESPACE_END

#endif
