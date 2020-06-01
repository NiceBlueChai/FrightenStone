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
 * @file  : UserCloseReasonType.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/05/31
 * @brief : �ر�ԭ��
 */

#ifndef __Service_LogicSvc_Modules_User_Defs_UserCloseReasonType_H__
#define __Service_LogicSvc_Modules_User_Defs_UserCloseReasonType_H__
#pragma once

class UserCloseReasonType
{
public:
    enum 
    {
        Normal = 0,             // �����ر�
        NoProtocolHandler = 1,  // û��Э�鴦����(һ����Ϊ����)
        NoProtocolPort = 2,     // û��Э��˿ڣ�һ����Ϊ�쳣�����޶��⣩
        NetPackageTooLarge = 3, // ���������,һ����ڻ�������С(һ����Ϊ����)
        DecodeFail = 4,         // ���������ʧ��(��Ϊ����)
        InBlackList = 5,        // ���ں������У�����Ҫ������ϵͳ��ע��
    };

    static const Byte8 *GetStr(Int32 reason);
};

#pragma region Inline
inline const Byte8 *UserCloseReasonType::GetStr(Int32 reason)
{
    switch (reason)
    {
        case UserCloseReasonType::Normal:
            return "Normal";
        case UserCloseReasonType::NoProtocolHandler:
            return "NoProtocolHandler";
        case UserCloseReasonType::NoProtocolPort:
            return "NoProtocolPort";
        case UserCloseReasonType::NetPackageTooLarge:
            return "NetPackagteTooLarge";
        case UserCloseReasonType::DecodeFail:
            return "DecodeFail";
        case UserCloseReasonType::InBlackList:
            return "InBlackList";
        default:
            g_Log->w<UserCloseReasonType>(_LOGFMT_("unknown reason[%d] please add type"), reason);
            break;
    }

    return "Unknown";
}

#pragma endregion
#endif
