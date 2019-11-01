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
 * @file  : MessageQueue.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/11/1
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_MessageQueue_Impl_MessageQueue_H__
#define __Base_Common_Component_Impl_MessageQueue_Impl_MessageQueue_H__

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"

FS_NAMESPACE_BEGIN

struct BASE_EXPORT FS_MessageBlock;

// ��Ϣ������ѭFIFOԭ�򣬽��ն�ȡһ������һ��
class BASE_EXPORT MessageQueue
{
public:
    // ѹ��ĩ�ڵ�
    void Push(bool isLock = true);
    // ��ǰ�ڵ㵯��
    void Pop(bool isLock = true);

    // ��Ϣ���н��ն�
    void RecieverWait();

    // ��Ϣ���з��Ͷ�
    void WakeupReciever();
    void SenderLock();
    void SenderUnlock();

private:
    void _HandleMsg(FS_MessageBlock *msgBlock);

private:
    std::list<FS_MessageBlock *> _msgBlocks;            // ��Ϣ����
};

FS_NAMESPACE_END

#endif
