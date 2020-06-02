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
 * @file  : IGlobalSysMgrImpl.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2020/1/14
 * @brief :
 */

#ifdef __Service_LogicSvc_Modules_GlobalSys_Interface_IGlobalSysMgr_H__

#pragma once

template <typename GlobalSysFactoryType>
inline Int32 IGlobalSysMgr::RegisterGlobalSys()
{
    auto fsFactory = new GlobalSysFactoryType();
    int regRet = RegisterGlobalSys(fsFactory);
    Fs_SafeFree(fsFactory);
    return regRet;
}

template <typename GlobalSysType>
inline GlobalSysType *IGlobalSysMgr::GetSys()
{
    auto sysName = fs::RTTIUtil::GetByType<GlobalSysType>();
    if(UNLIKELY(sysName == ""))
        return NULL;

    return static_cast<GlobalSysType *>(GetSys(sysName));
}

#endif
