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
 * @file  : IniFileDefs.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2019/12/01
 * @brief :
 * 
 *
 * 
 */
#ifndef __Frame_Include_FrightenStone_Common_Component_Impl_File_Defs_IniFileDefs_H__
#define __Frame_Include_FrightenStone_Common_Component_Impl_File_Defs_IniFileDefs_H__

#pragma once

#include <FrightenStone/exportbase.h>
#include <FrightenStone/common/basedefs/BaseDefs.h>

FS_NAMESPACE_BEGIN

class FS_String;

class BASE_EXPORT IniFileDefs
{
public:
    enum ContentType
    {
        Invalid = 0,            // ��Ч
        Segment = 1,            // ��
        KeyValue = 2,           // ��ֵ������
    };

public:
    static const char _annotationFlag;        // ע�ͷ�Ĭ��:';'
    static const char _leftSegmentFlag;       // ����������:'['
    static const char _rightSegmentFlag;      // ����������:']'
    static const char _keyValueJoinerFlag;    // ��ֵ�����ӷ�:'='
    static const char _changeLineFlag;        // ���з�:'\n'
    static const char _segKeyJoinerFlag;      // ��������ӷ�:'-'���ڴ���һ��segkey�ļ�
    static const char *_annotationSpaceStr;   // ע�ͼ���ַ���:"\t\t\t\t"
    static const char _groupDataSeparateFlag;     // ������ݷָ���:','
};

class BASE_EXPORT IniFileMethods
{
public:
    static bool IsSegment(const FS_String &content, FS_String &segmentOut);
    static bool IfExistKeyValue(const FS_String &content);
    static bool ExtractValidRawData(const FS_String &content, Int32 &contentTypeOut, FS_String &validRawDataOut); // true��ʾ��segment����kevalue
    static bool SpiltKeyValue(const FS_String &validContent, FS_String &key, FS_String &value); // validContent��Ҫ�޳�ע�ͺ������ true��ʾ������key

    static bool IsEnglishChar(char ch);
    static bool IsNumChar(char ch);

    static void MakeSegKey(const FS_String &segment, const FS_String &key, FS_String &segKeyOut);
    static void MakeKeyValuePairStr(const FS_String &key, const FS_String &value, FS_String &keyValueStrOut);
};

FS_NAMESPACE_END

#endif
