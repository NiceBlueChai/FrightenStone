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
 * @file  : IFS_EngineComp.h
 * @author: Eric Yonng<120453674@qq.com>
 * @date  : 2019/12/30
 * @brief :
 */
#ifndef __Frame_Include_FrightenStone_Common_Net_Impl_IFS_EngineComp_H__
#define __Frame_Include_FrightenStone_Common_Net_Impl_IFS_EngineComp_H__

#pragma once

#include "FrightenStone/exportbase.h"
#include "FrightenStone/common/basedefs/BaseDefs.h"
#include "FrightenStone/common/status/status.h"
#include "FrightenStone/common/component/Impl/FS_String.h"
#include "FrightenStone/common/net/Defs/EngineCompDefs.h"

FS_NAMESPACE_BEGIN

class IFS_NetEngine;
struct NetEngineTotalCfgs;
class MessageQueueNoThread;
class ConcurrentMessageQueueNoThread;

class BASE_EXPORT IFS_EngineComp
{
public:
    IFS_EngineComp(IFS_NetEngine *engine, UInt32 compId, Int32 compType);
    virtual ~IFS_EngineComp() {}

public:
    virtual Int32 BeforeStart(const NetEngineTotalCfgs &totalCfgs);
    virtual Int32 Start();
    virtual void AfterStart() = 0;
    virtual void WillClose() = 0;         // �Ͽ���ģ��֮�������
    virtual void BeforeClose();
    virtual void Close();
    virtual void AfterClose() = 0;

public:
    virtual void OnSessionDisconnected(UInt64 sessionId) {}

public:
    // ����ת��
    template<typename CompType>
    CompType *CastTo();
    // ����ת��
    template<typename CompType>
    const CompType *CastTo() const;

    // ��ȡ���id
    virtual UInt32 GetCompId() const;
    virtual Int32 GetCompType() const;
    // ����Ϣ���У�����ָ����Ŀ��ֻ�е�һ����Ϣ���У���������Լ�������˵ֻ��pop����push
    virtual void BindCompMq(MessageQueueNoThread *compMq);
    // ���������������Ϣ����
    virtual void AttachAllCompMq(std::vector<MessageQueueNoThread *> *allCompMq);
    MessageQueueNoThread *GetMyMailBox();

    // �������
    IFS_NetEngine *GetEngine();
    void MaskReady(bool isReady);
    bool IsReady() const;

    FS_String ToString() const;
    
protected:
    UInt32 _compId;
    Int32 _compType;
    IFS_NetEngine *_engine;
    MessageQueueNoThread *_myCompMq;  // ��������Լ�������˵����Լ���������,������ɽ���push������ֻ��pop
    std::vector<MessageQueueNoThread *> *_allCompMq;    // ��������ĵ�һ��Ϣ����,ʹ�����id��������
    std::atomic_bool _isCompReady;
};

FS_NAMESPACE_END

#include "FrightenStone/common/net/Impl/IFS_EngineCompImpl.h"

#endif
