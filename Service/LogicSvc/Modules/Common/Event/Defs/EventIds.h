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
 * @file  : EventIds.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/1/15
 * @brief :
 */
#ifndef __Service_LogicSvc_Modules_Common_Event_Defs_EventIds_H__
#define __Service_LogicSvc_Modules_Common_Event_Defs_EventIds_H__

#pragma once

 // ������EventParamDefs�ж��� �¼���֧�ֿ��߳�

class EventIds
{
public:
    enum
    {
        Begin = 0,                              // ��ʼ
        #pragma region Logic[1, 999]
        Logic_OnSessionWillConnected = 1,       /* �Ự���� userϵͳ����
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param["Stub"]:���UInt64
                                                */
        Logic_OnSessionConnecting = 2,          /* �Ự���� ���ڴ����������������� ҵ��Ķ������������ʱ����
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param["Stub"]:���UInt64
                                                */
        Logic_OnSessionConnectFail = 3,         /* �Ự����ʧ�� ��������������������ж�Ϊʧ�ܵ�����ʱ��Ҫ��ǲ��������¼�
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param["Stub"]:���UInt64
                                                */
        Logic_OnSessionConnected = 4,           /* �Ự����
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param["Stub"]:���UInt64
                                                */

        Logic_OnAfterSessionConnected = 5,      /* �Ự����
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param[Stub]:���UInt64
                                                */

        Logic_OnSessionWillDisconnect = 6,      /* �Ự�Ͽ� 
                                                * @param["SessionId"]:�Ựid UInt64
                                                */

        Logic_OnSessionDisconnect = 7,          /* �Ự�Ͽ� NodeMgr��������ط��Ƴ������������ӵĽڵ�
                                                * @param["SessionId"]:�Ựid UInt64
                                                */

        Logic_OnAfterSessionDisconnected = 8,   /* �Ự�Ͽ� ��ʱ��user�����Ƴ�����������user����Ĳ��������ʱ����ҵ��
                                                * @param["SessionId"]:�Ựid UInt64
                                                */

        Logic_LocalServerReady = 9,             /* ���ط�����׼������ ��ʱӦ���ظ���ϵͳ����,���л���,������eventcareBeforeStartup��
                                                */
        Logic_WillStartup = 10,                  /* ��������������,��Ҫ�ж��Ƿ����startup��startup֮ǰ�ļ��
                                                */
        Logic_TurnStartup = 11,                  /* ��������ȫ����,���Խ���Э�鴦��
                                                */
        Logic_TurnStartupFinish = 12,            /* ��������ȫ����,���Դ��������߼���
                                                */
        Logic_ConnectFailure = 13,              /* ����ʧ��
                                                * @param[TargetIp]:Ŀ��ip string
                                                * @param[TargetPort]:Ŀ��˿� UInt16
                                                * @param[Stub]:���   UInt64
                                                * @param[ResultCode]:ʧ��״̬�� Int32
                                                */
        Logic_AsynConnectResult = 14,           /* �첽���ӽ��
                                                * @param[SessionId]:�Ựid UInt64 �п���Ϊ0,Ϊ0������ʧ�����
                                                * @param[TargetIp]:Ŀ��ip string
                                                * @param[TargetPort]:Ŀ��˿� UInt16
                                                * @param[Stub]:���   UInt64
                                                * @param[ResultCode]:״̬�� ��StatusDefs Int32
                                                */
        #pragma endregion

        #pragma region User[1000, 1099]
        User_Login = 1000,                      /* ��½
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param["UserId"]:UserId UInt64
                                                * @param["RecvMsgId"]:�յ�����Ϣid Int32
                                                */
        User_Created = 1001,                    /* �û��������
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param["UserId"]:UserId UInt64
                                                */
        User_LoginRes = 1002,                   /* �û���½���
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param["UserId"]:UserId UInt64
                                                * @param["RecvMsgId"]:�յ�����Ϣid Int32
                                                */
        User_WillClose = 1003,                  /* �û������ر�
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param["UserId"]:UserId UInt64
                                                * @param["Reason"]:ԭ�� Int32
                                                */
        User_Close = 1004,                      /* �û������ر�
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param["UserId"]:UserId UInt64
                                                * @param["Reason"]:ԭ�� Int32
                                                */
        User_CloseFinish = 1005,                /* �û��رս���
                                                * @param["SessionId"]:�Ựid UInt64
                                                * @param["UserId"]:UserId UInt64
                                                * @param["Reason"]:ԭ�� Int32
                                                */
        #pragma endregion

        #pragma region Cfgs[1100, 1199]
        // reload������
        Cfgs_Reload = 1100,                     /* �������� ֻ��������,����¼���ñ���������ɾ�ģ�
                                                * @param["CfgFileType"]:�����ļ����� Int32
                                                * @param["OldCfgFileMd5"]:�ɵ������ļ�md5 string
                                                * @param["NewCfgFileMd5"]:�µ������ļ�md5 string
                                                */
        Cfgs_WillReloadEnd = 1101,              /* �������ؼ������� �Ӽ�¼���������ñ�����,�����ݽ����������� TODOδ����
                                                */
        Cfgs_ReloadEnd = 1102,                  /* �������ؼ������� ʱ���Ⱥ�˳�� TODO:δ����
                                                */
        #pragma endregion
    };
};
#endif
