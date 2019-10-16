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
 * @file  : StatusDefs.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Status_Defs_StatusDefs_H__
#define __Base_Common_Status_Defs_StatusDefs_H__

#pragma once

#include<base/exportbase.h>

class BASE_EXPORT StatusDefs
{
public:
    enum StatusEnum :int
    {
#pragma region [-1, 499]
        Error = -1,                                             // ����
        Success = 0,                                            // �ɹ�
        WaitEventFailure = 1,                                   // �ȴ�����
        WaitEventTimeOut = 2,                                   // �ȴ���ʱ
        Repeat = 3,                                             // �ظ�
        ParamError = 4,                                         // ��������
        Failed = 5,                                             // ʧ��
        AppInitFail = 6,                                        // ��ʼ��ʧ��
        Unknown = 7,                                            // δ֪����
        #pragma endregion

        #pragma region SOCKET[500, 599]
        Socket_CreateFailure = 500,                             // �׽��ִ���ʧ��
        Socket_NotInit = 501,                                   // �׽���δ��ʼ��
        Socket_ParamError = 502,                                // �׽��ֲ�������
        Socket_InvalidSocket = 503,                             // ��Ч�׽���
        Socket_SetSockOptFailed = 504,                          // �����׽��ֲ���ʧ��
        Socket_GetsockoptFailed = 505,                          // ��ȡ�׽��ֲ���ʱ����
        Socket_Unknown = 506,                                   // δ֪
        Socket_SetBlockParamError = 507,                        // �����׽�����������ʧ��
        Socket_Error = 508,                                     // �׽��ֲ�������
        #pragma endregion

        #pragma region Trigger[600, 699]
        Trigger_InvalidOccasion = 600,                          // ��Ч��ʱ��
        Trigger_InvalidTriggerType = 601,                       // ��Ч�Ĵ�������
        Trigger_UnkownAddType = 602,                            // δ֪����ӷ�ʽ
        Trigger_TriggerTypeRepeatInOccasion = 603,              // ���������Ѵ��ڣ��벻Ҫ�ظ����
        #pragma endregion 

        #pragma region Aes[700-799]
        Aes_TextLengthNotEnough = 700,                          // ���Ȳ���
        Aes_Not16BytesMultiple = 701,                           // ����16�ֽڱ���
        Aes_CyphertextIsEmpty = 702,                            // ���Ĳ���Ϊ��
        Aes_PlaintextIsEmpty = 703,                             // ���Ĳ���Ϊ��
        #pragma endregion

        #pragma region TimeWheel[800-899]
        TimeWheel_ExpiredTimeIsNull = 800,                      // ����ʱ��Ϊ��
        TimeWheel_RegisterAExistsTimeData = 801,                // �ظ�ע��
        TimeWheel_CantRegisterWhenRotatingWheel = 802,          // ת������ʱ�����ڷ�ת��ת�̵������ط�ע�ᶨʱ��
        #pragma endregion

        #pragma region SystemUtil[900, 999]
        SystemUtil_GetKernel32HandleFailed = 900,               // ��ȡkernel32���ʧ��
        SystemUtil_GetGlobalMemoryStatusExFuncFailed = 901,     // ��ȡglobalmemorystatusex����ʧ��
        SystemUtil_GetGlobalMemoryStatusExFailed = 902,         // ��ȡ�ڴ���Ϣʧ��
        SystemUtil_GetModuleFileNameFailed = 903,               // ��ȡģ����ʧ��
        SystemUtil_OpenProcessQueryInfomationFailed = 904,      // ��ѯ������Ϣʧ��
        SystemUtil_LoadKernel32LibraryFailed = 905,             // ����kernel32.dllʧ��
        SystemUtil_QueryFullProcessImageNameFailed = 906,       // ��ѯȫ���̾�����ʧ��
        SystemUtil_GetProcessImageFileNameFailed = 907,         // ��ȡ���̾����ļ���ʧ��
        SystemUtil_QueryDosDeviceError = 908,                   // ��ѯ����������
        SystemUtil_GetDriveError = 909,                         // ��ȡ�̷�ʧ��
        #pragma endregion

        #pragma region FS_IPUtil[1000, 1099]
        FS_IPUtil_ParamError = 1000,                            // ��������
        FS_IPUtil_NotFound = 1001,                              // δ�ҵ�
        FS_IPUtil_GetAddrInfoFailed = 1002,                     // ��ȡ��ַ��Ϣ����
        FS_IPUtil_GetPeerNameFailed = 1003,                     // ��ȡ�Է���ַ��Ϣʧ��
        #pragma endregion

        #pragma region Socket[1100, 1199]

        #pragma endregion

        #pragma region Log[1200, 1299]
        Log_CreateDirFail = 1200,                               // ����Ŀ¼ʧ��
        Log_CreateLogFileFail = 1201,                           // ������־�ļ�ʧ��
        #pragma endregion

        #pragma region XorEncrypt[1300, 1399]
        XorEncrypt_PlainTextLenNotEnough = 1300,                // �������ݿ鳤�Ȳ���
        XorEncrypt_CypherTextSizeNotEnough = 1301,              // ���Ŀռ䲻��
        XorEncrypt_CypherTextLenNotEnough = 1302,               // �������ݿ鳤�Ȳ���
        XorEncrypt_PlainTextSizeNotEnough = 1303,               // ���Ŀռ䲻��
        #pragma endregion

        #pragma region CrashHandleUtil[1400, 1499]
        CrashHandleUtil_SymInitializeFail = 1400,               // ����pdb��ʼ��ʧ��
        #pragma endregion

        #pragma region IOCP[1500, 1599]
        IOCP_CreateCompletionPortFail = 1500,                   // ������ɶ˿�ʧ��
        IOCP_LoadAcceptExError = 1501,                          // ����AcceptEx����ʧ��
        IOCP_RegSocketToCompletionPortFail = 1502,              // �׽��ֹ�������ɶ˿�ʧ��
        IOCP_PostAcceptFail = 1503,                             // ����ɶ˿�Ͷ��io��������ʧ��
        IOCP_PostRecvFail = 1504,                               // ����ɶ˿�Ͷ��io��������ʧ��
        IOCP_PostSendFail = 1504,                               // ����ɶ˿�Ͷ��io��������ʧ��
        IOCP_LoadAcceptExFailForListenSocketIsInvalid = 1505,   // ����acceptexʧ�ܣ������˿���Ч
        IOCP_WaitTimeOut = 1506,                                // ��ʱ�ȴ�
        IOCP_IODisconnect = 1507,                               // io���ӶϿ�
        IOCP_WaitOtherError = 1508,                             // ��������
        IOCP_ClientForciblyClosed = 1509,                       // Զ�̿ͻ��˱�ǿ�йر�
        IOCP_PostQuitFail = 1510,                               // ����ɶ˿�Ͷ��quitʧ��
        IOCP_Quit = 1511,                                       // iocp�˳�
        IOCP_LoadGetAcceptExSockAddrFunFail = 1512,             // ���ػ�ȡ�ͻ��˵�ַ����ָ�����
        #pragma endregion

        #pragma region Tls[1600, 1699]
        Tls_CreateHandleFail = 1600,                            // tlshandle����ʧ��
        #pragma endregion

        #pragma region IocpConnector[1700, 1799]
        IocpConnector_StartFailOfMonitorTaskFailure = 1700,     // iocpconnector ���monitor taskʧ��
        #pragma endregion

    };
};

#endif //!__Base_Common_Status_Defs_StatusDefs_H__
