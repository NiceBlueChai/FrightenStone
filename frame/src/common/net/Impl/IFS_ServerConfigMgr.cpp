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
 * @file  : IFS_ServerConfigMgr.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/07
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "FrightenStone/common/net/Impl/IFS_ServerConfigMgr.h"
#include "FrightenStone/common/net/Defs/FS_NetDefs.h"
#include "FrightenStone/common/net/Impl/FS_ServerCore.h"

#include "FrightenStone/common/status/status.h"
#include "FrightenStone/common/component/Impl/FS_CpuInfo.h"
#include "FrightenStone/common/log/Log.h"
#include "FrightenStone/common/component/Impl/File/FS_IniFile.h"


FS_NAMESPACE_BEGIN

IFS_ServerConfigMgr::IFS_ServerConfigMgr()
    :_ini(NULL)
{
    g_SvrCfg = this;
}

IFS_ServerConfigMgr::~IFS_ServerConfigMgr()
{
    Fs_SafeFree(_ini);
}

Int32 IFS_ServerConfigMgr::Init()
{
    _ini = new FS_IniFile;
    _ini->SetPath(SVR_CONFIG_PATH_NAME);

    BUFFER256 cfgs;
    char *ptr = cfgs;
    if(!_ini->ReadAllKeyValueOfSection(SVR_CFG_LISTENER_SEG, ptr, static_cast<UInt16>(sizeof(cfgs))))
    {
        _InitDefCfgs();
    }

    _ReadCfgs();

    return StatusDefs::Success;
}

Int32 IFS_ServerConfigMgr::_InitDefCfgs()
{
    #pragma region listener
    // ip
    _ini->WriteStr(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_IP_KEY, SVR_CFG_LISTENER_IP);
    // port
    _ini->WriteStr(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_PORT_KEY, SVR_CFG_LISTENER_PORT);
    // 最大连接数
    _ini->WriteStr(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_CLN_LIMIT_KEY, SVR_CFG_LISTENER_CLN_LIMIT);
    #pragma endregion

    #pragma region transfer
    // 数据传输线程数
    _ini->WriteStr(SVR_CFG_TRANSFER_SEG, SVR_CFG_TRANSFER_SEG_CNT_KEY, SVR_CFG_TRANSFER_SEG_CNT);
    _ini->WriteStr(SVR_CFG_TRANSFER_SEG, SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL_KEY, SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL);
    _ini->WriteStr(SVR_CFG_TRANSFER_SEG, SVR_CFG_PREPARE_POOL_BUFFER_CNT_KEY, SVR_CFG_PREPARE_POOL_BUFFER_CNT);
    _ini->WriteStr(SVR_CFG_TRANSFER_SEG, SVR_CFG_MAX_MEMPOOL_MB_PER_TRANSFER_KEY, SVR_CFG_MAX_MEMPOOL_MB_PER_TRANSFER);
    #pragma endregion

    #pragma region dispatcher
    _ini->WriteStr(SVR_CFG_DISPATCHER_SEG, SVR_CFG_DISPATCHER_CNT_KEY, SVR_CFG_DISPATCHER_CNT);
    #pragma endregion

    #pragma region objpool
    _ini->WriteStr(SVR_CFG_OBJPOOL_SEG, SVR_CFG_MAX_ALLOW_OBJPOOL_MB_OCCUPIED_KEY, SVR_CFG_MAX_ALLOW_OBJPOOL_MB_OCCUPIED);
    #pragma endregion

    #pragma region memorypool
    _ini->WriteStr(SVR_CFG_MEMORY_POOL_SEG, SVR_CFG_MAX_ALLOW_MEMPOOL_MB_OCCUPIED_KEY, SVR_CFG_MAX_ALLOW_MEMPOOL_MB_OCCUPIED);
    #pragma endregion

    // 检查是否写入正确
    BUFFER256 buffer = {};
    char *ptr = buffer;
    _ini->ReadStr(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_IP_KEY, "", ptr, sizeof(buffer));
    if(strcmp(buffer, "127.0.0.1") != 0)
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail ip not match"));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 port = _ini->ReadInt(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_PORT_KEY, 0);
    if(port != atoi(SVR_CFG_LISTENER_PORT))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail port not match"));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 maxConnectQuantity = _ini->ReadInt(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_CLN_LIMIT_KEY, 0);
    if(maxConnectQuantity != atoi(SVR_CFG_LISTENER_CLN_LIMIT))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail maxConnectQuantity not match"));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 cnt = _ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_TRANSFER_SEG_CNT_KEY, 0);
    if(cnt != atoi(SVR_CFG_TRANSFER_SEG_CNT))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail transfer cnt not match"));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 heartbeatTime = _ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL_KEY, 0);
    if(heartbeatTime != atoi(SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL[%u] not match SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL[%u] default")
                                      , heartbeatTime, static_cast<UInt32>(atoi(SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL)));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 preBufferCnt = _ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_PREPARE_POOL_BUFFER_CNT_KEY, 0);
    if(preBufferCnt != atoi(SVR_CFG_PREPARE_POOL_BUFFER_CNT))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail SVR_CFG_PREPARE_POOL_BUFFER_CNT[%u] not match SVR_CFG_PREPARE_POOL_BUFFER_CNT[%u] default")
                                      , preBufferCnt, static_cast<UInt32>(atoi(SVR_CFG_PREPARE_POOL_BUFFER_CNT)));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 memPoolMBPerTransfer = _ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_MAX_MEMPOOL_MB_PER_TRANSFER_KEY, 0);
    if(memPoolMBPerTransfer != atoi(SVR_CFG_MAX_MEMPOOL_MB_PER_TRANSFER))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail SVR_CFG_MAX_MEMPOOL_MB_PER_TRANSFER[%u] not match SVR_CFG_MAX_MEMPOOL_MB_PER_TRANSFER[%u] default")
                                      , memPoolMBPerTransfer, static_cast<UInt32>(atoi(SVR_CFG_MAX_MEMPOOL_MB_PER_TRANSFER)));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 dispatcherCnt = _ini->ReadInt(SVR_CFG_DISPATCHER_SEG, SVR_CFG_DISPATCHER_CNT_KEY, 0);
    if(dispatcherCnt != atoi(SVR_CFG_DISPATCHER_CNT))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail SVR_CFG_DISPATCHER_CNT[%u] not match SVR_CFG_DISPATCHER_CNT[%u] default")
                                      , dispatcherCnt, static_cast<UInt32>(atoi(SVR_CFG_DISPATCHER_CNT)));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 maxAllowMB = _ini->ReadInt(SVR_CFG_OBJPOOL_SEG, SVR_CFG_MAX_ALLOW_OBJPOOL_MB_OCCUPIED_KEY, 0);
    if(maxAllowMB != atoi(SVR_CFG_MAX_ALLOW_OBJPOOL_MB_OCCUPIED))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail SVR_CFG_MAX_ALLOW_OBJPOOL_MB_OCCUPIED[%u] not match SVR_CFG_MAX_ALLOW_OBJPOOL_MB_OCCUPIED[%u] default")
                                      , maxAllowMB, static_cast<UInt32>(atoi(SVR_CFG_MAX_ALLOW_OBJPOOL_MB_OCCUPIED)));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    UInt32 memPoolMaxAllowMB = _ini->ReadInt(SVR_CFG_MEMORY_POOL_SEG, SVR_CFG_MAX_ALLOW_MEMPOOL_MB_OCCUPIED_KEY, 0);
    if(memPoolMaxAllowMB != atoi(SVR_CFG_MAX_ALLOW_MEMPOOL_MB_OCCUPIED))
    {
        g_Log->e<IFS_ServerConfigMgr>(_LOGFMT_("_InitDefCfgs fail SVR_CFG_MAX_ALLOW_MEMPOOL_MB_OCCUPIED[%u] not match SVR_CFG_MAX_ALLOW_MEMPOOL_MB_OCCUPIED[%u] default")
                                      , memPoolMaxAllowMB, static_cast<UInt32>(atoi(SVR_CFG_MAX_ALLOW_MEMPOOL_MB_OCCUPIED)));
        return StatusDefs::IocpConnector_InitDefIniFail;
    }

    return StatusDefs::Success;
}

void IFS_ServerConfigMgr::_ReadCfgs()
{
    BUFFER256 buffer = {};
    char *ptr = buffer;
    _ini->ReadStr(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_IP_KEY, "", ptr, sizeof(buffer));
    _ip = buffer;
    _port = static_cast<UInt16>(_ini->ReadInt(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_PORT_KEY, 0));
    _maxConnectQuantityLimit = static_cast<Int32>(_ini->ReadInt(SVR_CFG_LISTENER_SEG, SVR_CFG_LISTENER_CLN_LIMIT_KEY, 0));

    _transferCnt = static_cast<Int32>(_ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_TRANSFER_SEG_CNT_KEY, 0));
    _heartbeatDeadTimeInterval = static_cast<Int32>(_ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL_KEY, 0));
    _prepareBufferPoolCnt = static_cast<Int32>(_ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_PREPARE_POOL_BUFFER_CNT_KEY, 0));
    _maxMemPoolBytesPerTransfer = static_cast<UInt64>((_ini->ReadInt(SVR_CFG_TRANSFER_SEG, SVR_CFG_MAX_MEMPOOL_MB_PER_TRANSFER_KEY, 0))) * 1024 * 1024;

    _dispatcherCnt = static_cast<Int32>(_ini->ReadInt(SVR_CFG_DISPATCHER_SEG, SVR_CFG_DISPATCHER_CNT_KEY, 0));

    _maxAllowObjPoolBytesOccupied = static_cast<UInt64>(_ini->ReadInt(SVR_CFG_OBJPOOL_SEG, SVR_CFG_MAX_ALLOW_OBJPOOL_MB_OCCUPIED_KEY, 0)) * 1024 * 1024;
    _maxAllowMemPoolBytesOccupied = static_cast<UInt64>(_ini->ReadInt(SVR_CFG_MEMORY_POOL_SEG, SVR_CFG_MAX_ALLOW_MEMPOOL_MB_OCCUPIED_KEY, 0)) * 1024 * 1024;
}
FS_NAMESPACE_END
