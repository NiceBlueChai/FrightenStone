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
 * @file  : FS_IniFile.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include "FrightenStone/common/asyn/Lock/Impl/Locker.h"
#include "FrightenStone/common/component/Impl/FS_String.h"
#include "FrightenStone/common/component/Impl/File/FS_IniFile.h"
#include "FrightenStone/common/assist/utils/Impl/FS_FileUtil.h"
#include "FrightenStone/common/component/Impl/File/Defs/IniFileDefs.h"
#include "FrightenStone/common/assist/utils/utils.h"

#ifdef _WIN32
#pragma region ini file io
#include<io.h>
#pragma endregion
#endif

FS_NAMESPACE_BEGIN

FS_IniFile::FS_IniFile()
    :_isDirtied(false)
    ,_maxLine(0)
{

}

FS_IniFile::~FS_IniFile()
{
//#ifndef _WIN32
    _UpdateIni();
//#endif
}

bool FS_IniFile::Init(const char *path)
{
    // detect if file is existed
    if(UNLIKELY(!FS_FileUtil::IsFileExist(path)))
    {
        auto fp = FS_FileUtil::OpenFile(path, true);
        if(UNLIKELY(!fp))
            return false;

        FS_FileUtil::CloseFile(*fp);
    }

    _filePath = path;

    return _Init();
}

const FS_String &FS_IniFile::GetPath() const
{
    return _filePath;
}

void FS_IniFile::Lock()
{
    _lock.Lock();
}

void FS_IniFile::Unlock()
{
    _lock.Unlock();
}

bool FS_IniFile::ReadStr(const char *segmentName, const char *keyName, const char *defaultStr, FS_String &strOut)
{
    if(UNLIKELY(_filePath.empty()))
        return false;

    return _ReadStr(segmentName, keyName, defaultStr, strOut);
}

Int64 FS_IniFile::ReadInt(const char *segmentName, const char *keyName, Int64 defaultInt)
{
    FS_String cache;
    if(_ReadStr(segmentName, keyName, "", cache) && !cache.empty())
        return StringUtil::StringToInt64(cache.c_str());

    return defaultInt;
}

UInt64 FS_IniFile::ReadUInt(const char *segmentName, const char *keyName, UInt64 defaultInt)
{
    FS_String cache;
    if(_ReadStr(segmentName, keyName, "", cache) && !cache.empty())
        return StringUtil::StringToUInt64(cache.c_str());

    return defaultInt;
}

bool FS_IniFile::WriteStr(const char *segmentName, const char *keyName, const char *wrStr)
{
    if(UNLIKELY(_filePath.empty()))
        return false;

    return _WriteStr(segmentName, keyName, wrStr);
}

bool FS_IniFile::HasCfgs(const char *segmentName)
{
    return  _segmentRefKeyValues.find(segmentName) != _segmentRefKeyValues.end();
}

bool FS_IniFile::ChangeLineBetweenSegs()
{
    // �򿪲�����ļ�
    auto fp = FS_FileUtil::OpenFile(_filePath.c_str(), false, "w");
    if(!fp)
        return false;

    FS_String lineData;
    FS_String seg;
    for(auto iterLineData = _lineRefContent.begin(); iterLineData != _lineRefContent.end(); ++iterLineData)
    {
        lineData.Clear();
        lineData = iterLineData->second + "\n";
        if(IniFileMethods::IsSegment(lineData, seg))
            FS_FileUtil::WriteFile(*fp, "\n");
        FS_FileUtil::WriteFile(*fp, lineData);
    }

    FS_FileUtil::FlushFile(*fp);
    FS_FileUtil::CloseFile(*fp);

    // ����load����
    _isDirtied = false;
    return _LoadAllCfgs();
}

bool FS_IniFile::_Init()
{
    // ��ȡ������������
    if(!_LoadAllCfgs())
        return false;

    return true;
}

bool FS_IniFile::_LoadAllCfgs()
{
    // ˢ���ļ�
    _UpdateIni();

    auto fp = FS_FileUtil::OpenFile(_filePath.c_str());
    if(!fp)
        return false;

    // ����seg,key value
    // ��ȡÿһ�У�����[��ʾ�µ�segment
    // һ��һ�еĶ�ȡ��������������Ϊֹ
    _maxLine = 0;
    _lineRefContent.clear();
    _segOrKeyRefLine.clear();
    _segmentRefKeyValues.clear();
    _segmentRefMaxValidLine.clear();

    std::map<FS_String, FS_String> *curKeyValues = NULL;
    FS_String curSegment;
    while(true)
    {
        FS_String lineData;
        auto cnt = FS_FileUtil::ReadOneLine(*fp, lineData);
        if(cnt || !FS_FileUtil::IsEnd(*fp))
            _lineRefContent.insert(std::make_pair(++_maxLine, lineData));
        if(!cnt)
        {
            if(FS_FileUtil::IsEnd(*fp))
                break;
            continue;
        }

        // ����ȡ
        Int32 contentType = IniFileDefs::ContentType::Invalid;
        FS_String validContent;
        auto hasValidData = IniFileMethods::ExtractValidRawData(lineData, contentType, validContent);
        if(hasValidData)
            _OnReadValidData(validContent, contentType, _maxLine, curSegment, curKeyValues);
    }

    return true;
}

bool FS_IniFile::_ReadStr(const char *segmentName, const char *keyName, const char *defaultStr, FS_String &strOut)
{
    auto iterKeyValue = _segmentRefKeyValues.find(segmentName);
    if(iterKeyValue == _segmentRefKeyValues.end())
        return false;

    auto &keyValue = iterKeyValue->second;
    auto iterValue = keyValue.find(keyName);
    if(iterValue == keyValue.end())
        return false;

    if(iterValue->second.empty())
    {
        strOut = defaultStr;
    }
    else
    {
        strOut = iterValue->second;
    }

    return true;
}

bool FS_IniFile::_WriteStr(const char *segmentName, const char *keyName, const char *wrStr)
{
    // Ѱ�Ҷ� û�б㴴���Σ���׷���к�
    auto iterKeyValue = _segmentRefKeyValues.find(segmentName);
    if(iterKeyValue == _segmentRefKeyValues.end())
    {
        iterKeyValue = _segmentRefKeyValues.insert(std::make_pair(segmentName, std::map<FS_String, FS_String>())).first;
        _segOrKeyRefLine.insert(std::make_pair(segmentName, ++_maxLine));

        FS_String segContent = "[";
        segContent << segmentName << "]";
        _lineRefContent.insert(std::make_pair(_maxLine, segContent));
        _isDirtied = true;
    }

    // Ѱ�Ҽ� û�б㴴����ֵ�� ��׷���к� ����������
    auto &keyValue = iterKeyValue->second;
    auto iterValue = keyValue.find(keyName);
    if(iterValue == keyValue.end())
    {
        const auto &segStr = iterKeyValue->first;
        Int32 segMaxValidLine = _GetSegmentKeyValueMaxValidLine(segStr);
        if(segMaxValidLine < 0)
        {// �β�����
            return false;
        }

        // ����keyvalue
        keyValue.insert(std::make_pair(keyName, wrStr));
        FS_String keyValueContent = keyName;
        keyValueContent << "=" << wrStr;

        // Ϊkey��������Ψһ����
        FS_String segKey = segStr + "-" + keyName;

        // �����µ�������
        return _InsertNewLineData(++segMaxValidLine, segStr, keyName, wrStr);
    }
    else
    {
        if(iterValue->second != wrStr)
        {
            // ��ֵ
            iterValue->second = wrStr;

            // �µļ�ֵ��������
            FS_String segKey = iterKeyValue->first + "-" + keyName;
            auto iterLine = _segOrKeyRefLine.find(segKey);

            auto iterContent = _lineRefContent.find(iterLine->second);
            auto &content = iterContent->second;

            // ����
            const auto &splitStr = content.Split(';', 1);

            FS_String comments;
            if(splitStr.size() >= 2)
            {// ����ע��
                comments << ";";
                comments << splitStr[1];
            }

            content = keyName;
            content << "=" << iterValue->second;
            if(!comments.empty())
                content << "\t\t\t\t" << comments;

            _isDirtied = true;
        }
    }

    // �����ļ�
    _UpdateIni();

    return true;
}

bool FS_IniFile::_InsertNewLineData(Int32 line, const FS_String &segment, const FS_String &key,  const FS_String &value)
{
    // �β����ڲ��ɲ���
    auto iterKeyValue = _segmentRefKeyValues.find(segment);
    if(iterKeyValue == _segmentRefKeyValues.end())
        return false;

    auto iterContent = _lineRefContent.find(line);
    if(iterContent != _lineRefContent.end())
    {// �Ѵ�������ҪŲ��������
        FS_String swapStr, frontStr;
        frontStr = iterContent->second;
        for(++iterContent; iterContent != _lineRefContent.end(); ++iterContent)
        {
            swapStr = iterContent->second;
            iterContent->second = frontStr;
            frontStr = swapStr;
        }

        // ���һ���ڵ�Ų���µ���
        ++_maxLine;
        _lineRefContent.insert(std::make_pair(_maxLine, frontStr));
        auto iterToModify = _lineRefContent.find(line);
        IniFileMethods::MakeKeyValuePairStr(key, value, iterToModify->second);
    }
    else
    {// ����������Ųֱ�Ӳ���
        FS_String keyValue;
        IniFileMethods::MakeKeyValuePairStr(key, value, keyValue);
        _lineRefContent.insert(std::make_pair(line, keyValue));
    }

    // ��������к�
    if(_maxLine < line)
        _maxLine = line;

    _isDirtied = true;
    _UpdateIni();

    // ���¼�������
    return _LoadAllCfgs();
}

void FS_IniFile::_UpdateIni()
{
    if(!_isDirtied)
        return;

    _isDirtied = false;

    // �򿪲�����ļ�
    auto fp = FS_FileUtil::OpenFile(_filePath.c_str(), false, "w");
    FS_String lineData;
    for(auto iterLineData = _lineRefContent.begin(); iterLineData != _lineRefContent.end(); ++iterLineData)
    {
        lineData.Clear();
        lineData = iterLineData->second + "\n";
        FS_FileUtil::WriteFile(*fp, lineData);
    }

    FS_FileUtil::FlushFile(*fp);
    FS_FileUtil::CloseFile(*fp);
}

void FS_IniFile::_OnReadValidData(const FS_String &validContent
                                  , Int32 contentType
                                  , Int32 line
                                  , FS_String &curSegment
                                  , std::map<FS_String, FS_String> *&curKeyValues)
{
    if(contentType == IniFileDefs::ContentType::Segment)
    {// �Ƕ�
        curSegment = validContent;
        auto iterKeyValues = _segmentRefKeyValues.find(curSegment);
        if(iterKeyValues == _segmentRefKeyValues.end())
            iterKeyValues = _segmentRefKeyValues.insert(std::make_pair(curSegment, std::map<FS_String, FS_String>())).first;
        curKeyValues = &(iterKeyValues->second);

        // ��¼�����ڵ��к�
        auto iterLine = _segOrKeyRefLine.find(curSegment);
        if(iterLine == _segOrKeyRefLine.end())
        {
            _segOrKeyRefLine.insert(std::make_pair(curSegment, line));
        }
    }
    else if(contentType == IniFileDefs::ContentType::KeyValue)
    {// �Ǽ�ֵ��
        if(!curSegment.empty())
        {// ��Ҫ���ж����м�ֵ��
            FS_String key;
            FS_String value;
            if(IniFileMethods::SpiltKeyValue(validContent, key, value))
            {
                auto iterValue = curKeyValues->find(key);
                if(iterValue == curKeyValues->end())
                {
                    curKeyValues->insert(std::make_pair(key, value));

                    // ��¼��ֵ�����ڵ��к�
                    FS_String segKey = curSegment + "-" + key;
                    auto iterLine = _segOrKeyRefLine.find(segKey);
                    if(iterLine != _segOrKeyRefLine.end())
                        perror("segOrKey repeated");

                    _segOrKeyRefLine.insert(std::make_pair(segKey, line));
                }

                // ���¼�ֵ�������Ч�к�
                auto iterMaxValidLine = _segmentRefMaxValidLine.find(curSegment);
                if(iterMaxValidLine == _segmentRefMaxValidLine.end())
                    iterMaxValidLine = _segmentRefMaxValidLine.insert(std::make_pair(curSegment, 0)).first;

                if(iterMaxValidLine->second < line)
                    iterMaxValidLine->second = line;
            }
        }
    }
}

Int32 FS_IniFile::_GetSegmentKeyValueMaxValidLine(const FS_String &segment) const
{
    // ���ö��ǿյ�����segment��������Ϊ����к�
    auto iterLine = _segmentRefMaxValidLine.find(segment);
    if(iterLine == _segmentRefMaxValidLine.end())
    {
        auto iterSegLine = _segOrKeyRefLine.find(segment);
        if(iterSegLine == _segOrKeyRefLine.end())
        {// �öβ�����
            return -1;
        }

        return iterSegLine->second;
    }

    return iterLine->second;
}

FS_NAMESPACE_END


