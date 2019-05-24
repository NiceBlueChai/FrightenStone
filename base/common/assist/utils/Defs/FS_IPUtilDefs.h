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
 * @file  : FS_IPUtilDefs.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Assist_Utils_Impl_Defs_FS_IPUtilDefs_H__
#define __Base_Common_Assist_Utils_Impl_Defs_FS_IPUtilDefs_H__
#pragma once

class FS_IPUtilDefs
{
public:
    // ������
    enum FAMILY_TYPE
    {
        FAMILY_TYPE_UNSPEC = 0,     //  protocal nonrelationship
        FAMILY_TYPE_AF_INET = 2,    //  IPV4
        FAMILY_TYPE_AF_INET6 = 23,	//  IPV6
    };

    // Э������
    enum PROTOCOL_TYPE
    {
        PROTOCOL_TYPE_IPPROTO_IP = 0,       //  ip protocal
        PROTOCOL_TYPE_IPPROTO_IPV4 = 4,     //  ipv4
        PROTOCOL_TYPE_IPPROTO_TCP = 6,      //  tcp
        PROTOCOL_TYPE_IPPROTO_UDP = 17,     //  udp
        PROTOCOL_TYPE_IPPROTO_IPV6 = 41,    //  IPV6
    };

    //  ������׽�������
    enum SOCK_TYPE
    {
        SOCK_TYPE_SOCK_STREAM = 1,          // stream
        SOCK_TYPE_SOCK_DGRAM = 2,           // Datagram
    };

    // ai_flags
    enum AI_FLAGS_TYPE
    {
        AI_FLAGS_TYPE_NONE = 0,             // begin
        AI_FLAGS_TYPE_AI_PASSIVE = 1,       // passive��use to bind��usually in server socket case
        AI_FLAGS_TYPE_AI_CANONNAME = 2,     // use to get computer standard name
        AI_FLAGS_TYPE_AI_NUMERICHOST = 4,   // addr is numeric string
        AI_FLAGS_TYPE_END = 8,              // ai_flags value range[0,8)
    };

    // ai_flags��ֵ�ķ�ΧΪ0~7��ȡ���ڳ����������3����־λ��
    // ��������ai_flagsΪ��AI_PASSIVE | AI_CANONNAME����ai_flagsֵ��Ϊ3��
    // ���������ĺ���ֱ�Ϊ��
    // (1)AI_PASSIVE ���˱�־��λʱ����ʾ�����߽���bind()����������ʹ�÷��صĵ�ַ�ṹ,
    // ���˱�־����λʱ����ʾ����connect()����������ʹ�á�
    // ���ڵ���λNULL���Ҵ˱�־��λ���򷵻صĵ�ַ����ͨ���ַ��
    // ����ڵ���NULL���Ҵ˱�־����λ���򷵻صĵ�ַ���ǻػ���ַ��
    // (2)AI_CANNONAME ���˱�־��λʱ���ں��������صĵ�һ��addrinfo�ṹ�е�ai_cannoname��Ա�У�
    // Ӧ�ð���һ���Կ��ַ���β���ַ������ַ����������ǽڵ�������������
    // (3)AI_NUMERICHOST ���˱�־��λʱ���˱�־��ʾ�����еĽڵ���������һ�����ֵ�ַ�ַ�����
};
#endif
