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
 * @file  : FS_Iocp.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/7
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Net_Impl_FS_Iocp_H__
#define __Base_Common_Net_Impl_FS_Iocp_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/memorypool/memorypool.h"
#include<MSWSock.h>

FS_NAMESPACE_BEGIN

struct BASE_EXPORT IoDataBase;
struct BASE_EXPORT IO_EVENT;

class BASE_EXPORT FS_Iocp
{
    MEM_POOL_CREATE_DEF();
public:
    FS_Iocp();
    virtual ~FS_Iocp();

    /* ��ʼ����ע�� */
    #pragma region init/reg
    /*
    *   brief:
    *       1. - Create �����˿�
    *       2. - Destroy ���ٶ˿�
    *       3. - reg �����˿� regʱ�����Զ���ĵ�ַ����ulong_ptr������waitʱ�򴫻أ�
    *                         ����ȷ��wait���ص�����һ��������ɶ˿ڵ�����
    *       4. - reg �����˿ڣ��������Զ���������Ϊulong_ptr������
    *       5. - LoadAcceptEx ����accept������
    *                         ��Ϊ��dll����������ĵ�����Ҫ�Լ���ǰ���ص��ڴ���
    */
public:
    Int32 Create();
    void Destroy();
    Int32 Reg(SOCKET sockfd);
    Int32 Reg(SOCKET sockfd, void *ptr);    // �ڶ������������ʱ�ش���completekey
    Int32 Reg(SOCKET sockfd, UInt64 sessionId); // �ڶ������������ʱ�ش���completekey
    Int32 LoadAcceptEx(SOCKET listenSocket);
    #pragma endregion

    /* iocp�Ĳ��� */
    #pragma region
    /*
    *   brief:
    *       1. - PostAccept acceptʱ�����ص��崫�룬��waitʱ�򴫻�
    *                       �����ص���ṹ�������ְ�����ɶ˿��ϵ�socket�е��Ӳ�����
    *                       ����󶨵��Ǽ���socket���ǿ��ܱ�����ͻ������ӣ�
    *                       Ϊ���������ĸ��ͻ�����Ҫ�ഫ��һ���ص���ṹ�Ա�����
    *       2. - PostRecv Ͷ�ݽ�������
    *       3. - PostSend Ͷ�ݷ�������
    *       4. - PostQuit Ͷ���˳�
    *       5. - WaitForCompletion �ȴ�����io�˿���ɣ�
    *                              getqueue���̰߳�ȫ��
    *                              ����һ��һ����Ϣ�Ĵ���ɶ�����ȡ������Ҫ�����̰߳�ȫ����
    */
public:
    // post�̰߳�ȫ
    Int32 PostAccept(SOCKET listenSocket, IoDataBase *ioData);
    Int32 PostRecv(SOCKET targetSock, IoDataBase *ioData);
    Int32 PostSend(SOCKET targetSock, IoDataBase *ioData);

    Int32 PostQuit();
    Int32 WaitForCompletion(IO_EVENT &ioEvent, ULong millisec = INFINITE);
    void GetClientAddrInfo(void *wsaBuff, sockaddr_in *&clientAddr);
    #pragma endregion

private:
    HANDLE _completionPort = NULL;
    LPFN_ACCEPTEX _fnAcceptEx = NULL;
    LPFN_GETACCEPTEXSOCKADDRS _fnGetAcceptClientAddrIn = NULL;
};

FS_NAMESPACE_END

#endif
