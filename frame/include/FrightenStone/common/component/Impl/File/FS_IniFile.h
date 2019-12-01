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
 * @file  : FS_IniFile.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */
#ifndef __Frame_Include_FrightenStone_Common_Component_Impl_File_FS_IniFile_H__
#define __Frame_Include_FrightenStone_Common_Component_Impl_File_FS_IniFile_H__
#pragma once

#include <FrightenStone/exportbase.h>
#include "FrightenStone/common/basedefs/DataType/DataType.h"
#include "FrightenStone/common/basedefs/Macro/MacroDefs.h"
#include "FrightenStone/common/memorypool/memorypool.h"

FS_NAMESPACE_BEGIN

class FS_String;
class Locker;
class BASE_EXPORT FS_IniFile
{
    OBJ_POOL_CREATE_DEF(FS_IniFile);

    
public:
    FS_IniFile();
    virtual ~FS_IniFile();

public:
    bool Init(const char *path);
    const FS_String &GetPath() const;

    void Lock();
    void Unlock();

    bool ReadStr(const char *segmentName, const char *keyName, const char *defaultStr, FS_String &strOut);
    Int64 ReadInt(const char *segmentName, const char *keyName, Int64 defaultInt);
    bool WriteStr(const char *segmentName, const char *keyName, const char *wrStr);

    bool HasCfgs(const char *segmentName);

private:
    bool _Init();
    bool _LoadAllCfgs();

    bool _ReadStr(const char *segmentName, const char *keyName, const char *defaultStr, FS_String &strOut);
    bool _WriteStr(const char *segmentName, const char *keyName, const char *wrStr);

    // ������������
    bool _InsertNewLineData(Int32 line, const FS_String &segment, const FS_String &key,  const FS_String &value); // �����¼������ã�����������ȫ��ʧЧ
    // ��������
    void _UpdateIni();

    // ��ȡ����Ч������
    void _OnReadValidData(const FS_String &validContent
                          , Int32 contentType
                          , Int32 line
                          , FS_String &curSegment
                          , std::map<FS_String, FS_String> *&curKeyValues);

    // �ε��������ļ�ֵ�Ե������
    Int32 _GetSegmentKeyValueMaxValidLine(const FS_String &segment) const;  // ����-1�öβ�����

private:
    Locker _lock;
    FS_String  _filePath;

    bool _isDirtied;
    Int32 _maxLine;
    std::map<Int32, FS_String> _lineRefContent;     // ÿһ�е�Ԫ����
    std::map<FS_String, Int32> _segOrKeyRefLine;    // seg��Ӧ���кŻ���seg-key���ڵ��к� ����seg, seg-key
    std::map<FS_String, std::map<FS_String, FS_String>> _segmentRefKeyValues;
    std::map<FS_String, Int32> _segmentRefMaxValidLine;        // key���Σ� value:�ö���Ч������к�
};

FS_NAMESPACE_END

#endif
