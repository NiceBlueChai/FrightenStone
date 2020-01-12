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
 * @file  : LogFile.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/24
 * @brief :
 * 
 *
 * 
 */

#include "stdafx.h"
#include "FrightenStone/common/component/Impl/Time.h"
#include "FrightenStone/common/component/Impl/TimeSlice.h"
#include "FrightenStone/common/component/Impl/File/LogFile.h"
#include "FrightenStone/common/assist/utils/Impl/FS_FileUtil.h"
#include "FrightenStone/common/assist/utils/Impl/SystemUtil.h"

FS_NAMESPACE_BEGIN

OBJ_POOL_CREATE_DEF_IMPL(LogFile, __DEF_OBJ_POOL_OBJ_NUM__);

LogFile::LogFile()
    :_partNo(0)
{
    
}

LogFile::~LogFile()
{

}

void LogFile::PartitionFile(bool isSysFirstCreate, Time *nowTime)
{
    if(isSysFirstCreate)
        return;

    // 构建文件名
    FS_String fileNameCache;
    GetCurrentFileName(fileNameCache);

    // 查找不存在的文件名
    FS_String wholeName;
    wholeName.AppendFormat("%sOld%d", fileNameCache.c_str(), ++_partNo);
    while(FS_FileUtil::IsFileExist(wholeName.c_str()))
    {
        wholeName.Clear();
        wholeName.AppendFormat("%sOld%d", fileNameCache.c_str(), ++_partNo);
    }

    // 转储文件
    auto dest = FS_FileUtil::OpenFile(wholeName.c_str(), true);
    FS_FileUtil::ResetFileCursor(*_fp);
    FS_FileUtil::CopyFile(*_fp, *dest);
    FS_FileUtil::CloseFile(*dest);

    // 删除并重开文件
    Close();
    FS_FileUtil::DelFile(fileNameCache.c_str());
    ASSERT(Reopen(nowTime));
}
FS_NAMESPACE_END
