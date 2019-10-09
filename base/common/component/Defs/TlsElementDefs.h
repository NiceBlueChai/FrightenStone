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
 * @file  : TlsElementDefs.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/10/9
 * @brief :
 * 
 *
 * 
 */

#ifndef __Base_Common_Component_Defs_TlsElementDefs_H__
#define __Base_Common_Component_Defs_TlsElementDefs_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/component/Defs/ITlsBase.h"
#include "base/common/objpool/objpool.h"

// ����ʶ�𻺳��С
#ifndef __FS_RTTI_BUF_SIZE__
#define __FS_RTTI_BUF_SIZE__    512
#endif

FS_NAMESPACE_BEGIN

// ���оֲ��洢������������ITlsBase
// �ֲ߳̾��洢��Ҫ�洢�Ķ�������
class BASE_EXPORT TlsElemType
{
public:
    enum
    {
        Begin = 0,
        Tls_Rtti,           // ����ʶ��
        Tls_TestTls,        // ����tls
        End,
    };
};

// ����ʶ���ֲ߳̾��洢
struct BASE_EXPORT Tls_Rtti : public ITlsBase
{
    OBJ_POOL_CREATE_DEF(Tls_Rtti);

    Tls_Rtti();
    virtual ~Tls_Rtti();
    virtual void Release();

    char rtti[__FS_RTTI_BUF_SIZE__];
};

// �����ֲ߳̾��洢
struct BASE_EXPORT Tls_TestTls : public ITlsBase
{
    OBJ_POOL_CREATE_DEF(Tls_TestTls);

    Tls_TestTls();
    virtual ~Tls_TestTls();

    Int32 count;
};

FS_NAMESPACE_END

#include "base/common/component/Defs/TlsElementDefsImpl.h"

#endif
