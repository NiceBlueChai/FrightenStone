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
 * @file  : FS_NetDefs.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/08/03
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Defs_FS_NetDefs_H__
#define __Base_Common_Net_Defs_FS_NetDefs_H__
#pragma once


 // SOCKET
#ifdef _WIN32
#undef FD_SETSIZE
#define FD_SETSIZE      100000
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#else
#ifdef __APPLE__
#define _DARWIN_UNLIMITED_SELECT
#endif // !__APPLE__
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>
#include<signal.h>
#include<sys/socket.h>
#include<netinet/tcp.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

//��������С��Ԫ��С
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 16384    // 16K
#define SEND_BUFF_SZIE 16384    // 16K
#define FS_BUFF_SIZE_DEF 16384  // 16K
#endif // !RECV_BUFF_SZIE


// �ͻ����������������ʱʱ��(��λms)
// �ڼ��ָ��ʱ����������(��λms)
#define CLIENT_SEND_BUFF_TIME 200
// connector�Ļ����С
#define IOCP_CONNECTOR_BUFFER 1024
// ͳ�Ʒ��������ص�ʱ����
#define IOCP_STATISTIC_INTERVAL     1000    // ʱ������λms
#define SVR_CONFIG_PATH_NAME    "./ServerCfg.ini"   // ����������

#pragma region Listener default config
#define SVR_CFG_LISTENER_SEG                "Listener"

#define SVR_CFG_LISTENER_IP_KEY             "ip"
#define SVR_CFG_LISTENER_IP                 "127.0.0.1"
#define SVR_CFG_LISTENER_PORT_KEY           "port"
#define SVR_CFG_LISTENER_PORT               "4567"
#define SVR_CFG_LISTENER_CLN_LIMIT_KEY      "MaxConnectQuantity"        // ���������
#define SVR_CFG_LISTENER_CLN_LIMIT          "100064"                    // Ĭ�ϵ����������

#pragma endregion

#pragma region Transfer default config
#define SVR_CFG_TRANSFER_SEG                            "Transfer"                      // ���ݴ�������

#define SVR_CFG_TRANSFER_SEG_CNT_KEY                    "TransferCnt"
#define SVR_CFG_TRANSFER_SEG_CNT                        "8"                             // ���ݴ���֧��8�߳�
#define SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL_KEY        "HeartbeatDeadTimeInterval"     // ��������ʱ����
#define SVR_CFG_HEARTBEAT_DEAD_TIME_INTERVAL            "30000"                         // Ĭ��30s����
#define SVR_CFG_PREPARE_POOL_BUFFER_CNT_KEY             "PreparePoolBufferCnt"          // Ԥ��׼��������buffer����ص�buffer����
#define SVR_CFG_PREPARE_POOL_BUFFER_CNT                 "1024"                          // Ԥ��׼��������buffer����ص�buffer����
#define SVR_CFG_MAX_MEMPOOL_MB_PER_TRANSFER_KEY         "MaxMemPoolMBPerTransfer"       // ÿ�����ݴ������ڴ�ش�С
#define SVR_CFG_MAX_MEMPOOL_MB_PER_TRANSFER             "1024"                          // MBΪ��λ
#pragma endregion

#pragma region Dispatcher default config
#define SVR_CFG_DISPATCHER_SEG                          "Dispatcher"                      // ��Ϣ�ַ�����

#define SVR_CFG_DISPATCHER_CNT_KEY                      "DispatcherCnt"                   // ��Ϣ�ַ�������
#define SVR_CFG_DISPATCHER_CNT                          "1"                               // ��Ϣ�ַ�������
#pragma endregion

#pragma region objpool default config
#define SVR_CFG_OBJPOOL_SEG                             "ObjPool"                         // ���������

#define SVR_CFG_MAX_ALLOW_OBJPOOL_MB_OCCUPIED_KEY       "MaxAllowMBOccupied"              // �������ռ���ڴ��С
#define SVR_CFG_MAX_ALLOW_OBJPOOL_MB_OCCUPIED           "2048"                            // Ĭ�ϴ�С2GB
#pragma endregion

#pragma region memorypool default config
#define SVR_CFG_MEMORY_POOL_SEG                         "MemoryPool"                      // �ڴ������

#define SVR_CFG_MAX_ALLOW_MEMPOOL_MB_OCCUPIED_KEY       "MaxAllowMBOccupied"              // �������ռ���ڴ��С
#define SVR_CFG_MAX_ALLOW_MEMPOOL_MB_OCCUPIED            "2048"                           // Ĭ�ϴ�С2GB
#pragma endregion

// ������������
// class FS_ClientComp
// {
// public:
//     bool operator(const FS_Client *l, const FS_Client *r) const;
// };

// ����ģ�Ͷ���
class NetModuleType
{
public:
    enum
    {
        None = 0,
        Iocp,
        Epoll,
    };
};

#endif
