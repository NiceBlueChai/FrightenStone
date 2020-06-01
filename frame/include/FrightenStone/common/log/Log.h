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
 * @file  : Log.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2019/6/12
 * @brief :
 * 
 *  ���log�ļ�����:
    1.��Ҫ��LogDefs.h��GENERAL_ADDLOG(Workfolder, LogFileName)
    2.LogFileType.h������ļ�ö��
    3.��virtual Int32 FS_Log::_GetLogFileIndex(Int32 logTypeEnum)��case ��Ӧ��LogFileType��Ӧ��fileindex��־�ļ�id
    4.��LogData.h�����LogLevel��Ӧ��ö��,����LogData.cpp�����ö�ٶ�Ӧ���ַ�ֵ
    5.��ȱʡ:_OutputToConsole ,_IsAllowToConsole,_SetConsoleColor,����Ӧ���Ƿ������ӡ�ڿ���̨,��ӡ��ɫ���߼�
    6.��ILog.h���������־�ļ���Ӧ�Ĵ�ӡ�ӿ�
 * 
 */
#ifndef __Frame_Include_FrightenStone_Common_Log_Log_H__
#define __Frame_Include_FrightenStone_Common_Log_Log_H__
#pragma once

// ���log����
// defs...
// interface...
#include "FrightenStone/common/log/Interface/ILog.h"

#endif

