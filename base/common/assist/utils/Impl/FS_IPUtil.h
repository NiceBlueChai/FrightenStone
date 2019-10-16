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
 * @file  : FS_IPUtil.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Assist_Utils_Impl_FS_IPUtil_H__
#define __Base_Common_Assist_Utils_Impl_FS_IPUtil_H__
#pragma once

#include<base/exportbase.h>
#include "base/common/basedefs/DataType/DataType.h"
#include "base/common/basedefs/Macro/ForAll/ForAllMacro.h"

FS_NAMESPACE_BEGIN
class FS_String;

class BASE_EXPORT FS_IPUtil
{
public:
    static Int32 GetLocalIP(FS_String &ip, Int32 netCardNo = 0, bool isToBind = true, bool isStreamSock = true, bool isIpv4 = true);
    static Int32 GetIPByDomain(
        const char *domain                                                // ������������
        , const char *service                                             /* �˿ںš�80���ȡ������� ��"ftp", "http"�� */
        , FS_String &ipAddrString                                           // ���ip
        , Int32 netCardNo = 0                                                // �����������ȡ���Ǳ��صĵ�ַ��Ϊѡ����ַ�б��ĳһ����ַ
        , Int32 eFlags = FS_IPUtilDefs::AI_FLAGS_TYPE_AI_PASSIVE                 // FS_IPUtilDefs::AI_FLAGS_TYPE ����λ����� Ĭ��AI_PASSIVE ������bind�� ������������connect
        , FS_IPUtilDefs::SOCK_TYPE eSockType = FS_IPUtilDefs::SOCK_TYPE_SOCK_STREAM	        // Ĭ��������
        , FS_IPUtilDefs::FAMILY_TYPE eFamily = FS_IPUtilDefs::FAMILY_TYPE_AF_INET           // Ĭ��ipv4
        , FS_IPUtilDefs::PROTOCOL_TYPE eProtocol = FS_IPUtilDefs::PROTOCOL_TYPE_IPPROTO_IP  // Ĭ������Э�� ��ipЭ��
    );
};

FS_NAMESPACE_END

#endif
