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
 * @file  : SmartVar.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_SmartVar__H__
#define __Base_Common_Component_Impl_SmartVar__H__


#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/Macro/MacroDefs.h"
#include "base/common/basedefs/DataType/DataType.h"
#include <map>

FS_NAMESPACE_BEGIN

class FS_String;

// RawData
// RTTI

// ����ʱ����ʶ��
class BASE_EXPORT SmartVarRtti
{
public:
    // ����ʶ����Ϣ32bit 
    // [high eight bits][middle 23 bits][first bit]
    // [    ������Ϣ   ][ ��������ö�� ][  ����λ ]
    enum RttiType:unsigned int
    {
        // ��ʼ������
        SV_NIL = 0x0UL,

        // ���ű��
        SV_SIGHED           = 0x01UL,             // �з���
        SV_UNSIGHED         = 0x0UL,              // �޷���

        // ������Ϣ
        SV_BRIEF_DATA       = 0x01000000UL,       // ������������
        SV_STRING           = 0x02000000UL,       // �ַ�������
        SV_DICTIONARY       = 0x04000000UL,       // �ֵ�����

        // �����͵ľ�������ö��
        SV_BRIEF_SIGHED_DATA        = SV_BRIEF_DATA     | SV_SIGHED,                        // �з��ż���������
        SV_BRIEF_UNSIGHED_DATA      = SV_BRIEF_DATA     | SV_UNSIGHED,                      // �޷��ż���������
        SV_BRIEF_BOOL       = SV_BRIEF_SIGHED_DATA      | (0x1UL << 1),                     // bool����
        SV_BRIEF_INT8       = SV_BRIEF_SIGHED_DATA      | (0x1UL << 2),                     // I8
        SV_BRIEF_UINT8      = SV_BRIEF_UNSIGHED_DATA    | (0x1UL << 3),                     // U8
        SV_BRIEF_INT16      = SV_BRIEF_SIGHED_DATA      | (0x1UL << 4),                     // INT16
        SV_BRIEF_UINT16     = SV_BRIEF_UNSIGHED_DATA    | (0x1UL << 5),                     // UINT16
        SV_BRIEF_INT32      = SV_BRIEF_SIGHED_DATA      | (0x1UL << 6),                     // INT32
        SV_BRIEF_UINT32     = SV_BRIEF_UNSIGHED_DATA    | (0x1UL << 7),                     // UINT32
        SV_BRIEF_LONG       = SV_BRIEF_SIGHED_DATA      | (0x1UL << 8),                     // LONG
        SV_BRIEF_ULONG      = SV_BRIEF_UNSIGHED_DATA    | (0x1UL << 9),                     // ULONG
        SV_BRIEF_PTR        = SV_BRIEF_UNSIGHED_DATA    | (0x1UL << 10),                    // PTR
        SV_BRIEF_INT64      = SV_BRIEF_SIGHED_DATA      | (0x1UL << 11),                    // INT64
        SV_BRIEF_UINT64     = SV_BRIEF_UNSIGHED_DATA    | (0x1UL << 12),                    // UINT64
        SV_BRIEF_FLOAT      = SV_BRIEF_SIGHED_DATA      | (0x1UL << 13),                    // float
        SV_BRIEF_DOUBLE     = SV_BRIEF_SIGHED_DATA      | (0x1UL << 14),                    // double

        // �ַ�������
        SV_STRING_DEF       = SV_STRING | SV_SIGHED,        // Ĭ�ϵ��ַ�������

        // �ֵ�����
        SV_DICTIONARY_DEF   = SV_DICTIONARY | SV_SIGHED,    // Ĭ�ϵ��ֵ�����
    };

    static const FS_String &GetTypeName(unsigned int rttiType);
    static void InitRttiTypeNames();
private:
    static std::map<unsigned int, FS_String> _rttiTypeRefString;   // �����ַ���
    static const FS_String _nullString;
};
FS_NAMESPACE_END

FS_NAMESPACE_BEGIN

class BASE_EXPORT SmartVar
{
public:
    typedef std::map<SmartVar, SmartVar> Dict;
    typedef FS_String Str;

public:
    struct BASE_EXPORT Raw
    {
        Raw();
        ~Raw();
        UInt32 _type;
        union
        {
            UInt64 _uint64;
            double _double;

            Dict *_dict;
            Str *_str;
        }_data;
    };

public:


private:
    Raw _raw;
};

FS_NAMESPACE_END

#endif
