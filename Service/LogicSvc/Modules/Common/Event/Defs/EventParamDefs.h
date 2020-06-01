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
 * @file  : EventParamDefs.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/03/19
 * @brief :
 */
#ifndef __Service_LogicSvc_Modules_Common_Event_Defs_EventParamDefs_H__
#define __Service_LogicSvc_Modules_Common_Event_Defs_EventParamDefs_H__
#pragma once

#include "FrightenStone/exportbase.h"

#pragma region Macro
#undef EventPtrGetParam
#define EventPtrGetParam(ev, Param)\
ev->GetParam(FS_EventParam::Param)

#undef EventPtrSetParam
#define EventPtrSetParam(ev, Param, Value)\
ev->SetParam(FS_EventParam::Param, Value)
#pragma endregion


class FS_EventParam
{
public:
    enum : Int32
    {
        Begin = 0,

        // ͨ�ò���[1, 10000]
        #pragma region general params [1, 10000]
        SessionId = 1,                  // �Ựid
        UserId = 2,                     // �û�id
        Stub = 3,                       // ���������Ψһ
        TargetIp =4,                    // Ŀ��ip
        TargetPort = 5,                 // Ŀ��˿�
        ResultCode = 6,                 // ���״̬��    Int32
        Reason = 7,                     // ԭ��
        #pragma endregion
        
        // �û�
        #pragma region[10001, 11000]
        RecvMsgId = 10001,              // �յ���msgid
        RecvMsgIdForCheck = 10002,      // ����У���recvmsgid
        #pragma endregion

        #pragma region cfgs[11001, 12000]
        CfgFileType = 11001,            // �����ļ�����
        OldCfgFileMd5 = 11002,          // �ɵ������ļ�md5
        NewCfgFileMd5 = 11003,          // �µ������ļ�md5
        #pragma endregion
        End,
    };
};

#endif
