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
 * @file  : FS_ConnectInfo.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2019/12/08
 * @brief :
 * 
 *
 * 
 */

#ifndef __Frame_Include_FrightenStone_Common_Net_Defs_FS_ConnectInfo_H__
#define __Frame_Include_FrightenStone_Common_Net_Defs_FS_ConnectInfo_H__

#pragma once

#include "FrightenStone/exportbase.h"
#include <FrightenStone/common/basedefs/BaseDefs.h>
#include <FrightenStone/common/component/Impl/FS_String.h>
#include "FrightenStone/common/objpool/objpool.h"

FS_NAMESPACE_BEGIN

struct BASE_EXPORT FS_ConnectInfo
{
    OBJ_POOL_CREATE_ANCESTOR(FS_ConnectInfo);

    FS_ConnectInfo():_port(0), _stub(0){}

    FS_String _ip;
    UInt16 _port;
    UInt64 _stub;
};

FS_NAMESPACE_END

#endif
