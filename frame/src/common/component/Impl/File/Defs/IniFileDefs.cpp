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
 * @file  : IniFileDefs.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/12/01
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include <FrightenStone/common/component/Impl/File/Defs/IniFileDefs.h>
#include <FrightenStone/common/component/Impl/FS_String.h>

FS_NAMESPACE_BEGIN

bool IniFileMethods::IsSegment(const FS_String &content, FS_String &segmentOut)
{
    // ����[] ��[]�м���ַ�ȥ�����ҿպ���������Ӣ�Ļ���ֵ
    auto cache = content;
    cache.lstrip();
    cache.rstrip();

    auto &cacheRaw = cache.GetRaw();
    auto leftBracketsPos = cacheRaw.find('[', 0);
    if(leftBracketsPos == std::string::npos)
        return false;

    auto rightBracketsPos = cacheRaw.find(']', leftBracketsPos);
    if(rightBracketsPos == std::string::npos)
        return false;

    // �ж���û�������ż��Ƿ����ַ�
    if(rightBracketsPos - leftBracketsPos - 1 == 0)
        return false;

    // ��ȡ�ֵ�segment
    FS_String segmentTmp = cacheRaw.substr(leftBracketsPos + 1, rightBracketsPos - leftBracketsPos - 1);
    segmentTmp.lstrip();
    segmentTmp.rstrip();

    // �Ƿ�������Ӣ�Ļ�����ֵ�������ַ�ΪӢ��
    if(!IsEnglishChar(segmentTmp[0]))
        return false;

    const auto len = segmentTmp.size();
    auto &segmentRaw = segmentTmp.GetRaw();
    for(size_t i = 0; i < len; ++i)
    {
        if(!IsEnglishChar(segmentRaw.at(i)) &&
           !IsNumChar(segmentRaw.at(i)))
            return false;
    }

    segmentOut = segmentRaw;
    return true;
}

bool IniFileMethods::IfExistKeyValue(const FS_String &content)
{
    auto &raw = content.GetRaw();
    return raw.find('=', 0) != std::string::npos;
}

bool IniFileMethods::ExtractValidRawData(const FS_String &content, Int32 &contentTypeOut, FS_String &validRawDataOut)
{
    contentTypeOut = IniFileDefs::Invalid;
    if(content.empty())
        return false;

    // ����ע��
    auto rawData = content.Split(';', 1)[0];
    if(rawData.empty())
        return false;

    // ȥ����Ч����
    rawData.lstrip();
    rawData.rstrip();

    // �ж��Ƿ���Ч���� ��ֵ�Ի���segment
    if(IsSegment(rawData, validRawDataOut))
    {
        contentTypeOut = IniFileDefs::Segment;
        return true;
    }

    if(IfExistKeyValue(rawData))
    {
        validRawDataOut = rawData;
        contentTypeOut = IniFileDefs::KeyValue;
        return true;
    }

    return false;
}

bool IniFileMethods::SpiltKeyValue(const FS_String &validContent, FS_String &key, FS_String &value)
{
    if(validContent.empty())
        return false;

    // ����=�ţ���=���keyֵ��ȥ����Ч���ź��һ��ΪӢ��������������Ӣ�Ļ�����ֵ
    auto &raw = validContent.GetRaw();
    auto eqPos = raw.find_first_of('=', 0);
    if(eqPos == std::string::npos)
        return false;

    // ���key�Ĵ�����
    FS_String keyRaw = raw.substr(0, eqPos);
    if(keyRaw.empty())
        return false;

    {// ����key����Чֵ��һ��Ӣ���ַ���ʼ
        keyRaw.lstrip();
        keyRaw.rstrip();
        if(!IniFileMethods::IsEnglishChar(keyRaw[0]))
            return false;

        // key������������Ӣ�Ļ�����ֵ
        auto &keyRawRaw = keyRaw.GetRaw();
        const auto len = keyRawRaw.length();
        for(size_t i = 0; i < len; ++i)
        {
            if(!IniFileMethods::IsEnglishChar(keyRawRaw.at(i)) &&
               !IniFileMethods::IsNumChar(keyRawRaw.at(i)))
                return false;
        }

        key = keyRawRaw.substr(0, keyRawRaw.size());
    }

    if(key.empty())
        return false;

    // ���ܻ�û��valueֵ
    if(raw.size() <= eqPos + 1)
        return !key.empty();

    // ���value�Ĵ����� ���ܻ�Ϊ��
    FS_String valueRaw = raw.substr(eqPos + 1, raw.size() - eqPos - 1);
    if(valueRaw.empty())
        return true;

    {// ȥ�����ҵ���Ч���ż�Ϊ��Ҫ��value
        valueRaw.lstrip();
        valueRaw.rstrip();
        value = valueRaw;
    }

    return true;
}

bool IniFileMethods::IsEnglishChar(char ch)
{
    return ch >= 'a'&&ch <= 'z' || ch >= 'A'&&ch <= 'Z';
}

bool IniFileMethods::IsNumChar(char ch)
{
    return ch >= '0'&&ch <= '9';
}

void IniFileMethods::MakeSegKey(const FS_String &segment, const FS_String &key, FS_String &segKeyOut)
{
    segKeyOut = segment + "-" + key;
}

void IniFileMethods::MakeKeyValuePairStr(const FS_String &key, const FS_String &value, FS_String &keyValueStrOut)
{
    keyValueStrOut = key + "=" + value;
}
FS_NAMESPACE_END

