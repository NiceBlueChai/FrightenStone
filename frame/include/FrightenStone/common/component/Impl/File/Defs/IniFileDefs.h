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
 * @file  : IniFileDefs.h
 * @author: ericyonng<120453674@qq.com>
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

class BASE_EXPORT FS_String;

class BASE_EXPORT IniFileDefs
{
public:
    enum ContentType
    {
        Invalid = 0,            // 无效
        Segment = 1,            // 段
        KeyValue = 2,           // 键值对数据
    };

public:

};

class BASE_EXPORT IniFileMethods
{
public:
    static bool IsSegment(const FS_String &content, FS_String &segmentOut);
    static bool IfExistKeyValue(const FS_String &content);
    static bool ExtractValidRawData(const FS_String &content, Int32 &contentTypeOut, FS_String &validRawDataOut); // true表示有segment或者kevalue
    static bool SpiltKeyValue(const FS_String &validContent, FS_String &key, FS_String &value); // validContent需要剔除注释后的数据 true表示至少有key

    static bool IsEnglishChar(char ch);
    static bool IsNumChar(char ch);
};

FS_NAMESPACE_END

#endif
