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
 * @file  : FS_ClientCfgMgr.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/12/5
 * @brief :
 * 
 *
 * 
 */

#pragma once

#include "FrightenStone/exportbase.h"
#include <FrightenStone/common/basedefs/BaseDefs.h>

class FS_IniFile;

class FS_ClientCfgMgr : public fs::IFS_ConfigMgr
{
public:
    FS_ClientCfgMgr();
    ~FS_ClientCfgMgr();

    /* ��������get�ӿ� */
public:
    fs::FS_String GetTargetSvrIp() const;
    UInt16 GetTargetSvrPort() const;
    Int32 GetClientQuantity() const;
    Int32 GetMsgNumPerPeriod() const;
    Int32 GetSendPeriodMs() const;
    bool NeedCheckMsgId() const;
    bool IsSendAfterSvrResArrive() const;

    /* ��������д�ӿ� */
protected:
    virtual Int32 _InitCustomCfgs(fs::FS_IniFile *iniOperator);
    virtual void _ReadCustomCfgs(fs::FS_IniFile *iniOperator);

private:
    /* ���û��� */
    fs::FS_String _ip;              // ���ӵ�ip
    UInt16 _port = 0;           // ���ӵ�Զ�̶˿�
    Int32 _clientQuantity = 0;  // �������ٸ��ͻ���
    Int32 _msgNumPerPeriod = 0; // ÿ���ڶ��ٸ���Ϣ
    Int32 _sendPeriodMs = 0;      // ����ʱ������
    bool _checkMsgId = 0;       // �Ƿ�����Ϣid,���ڼ���Ƿ񶪰���
    bool _isSendAfterSvrResArrive = 0;  // �Ƿ����յ����������غ��������,��Ϊfalse��ᰴ��ʱ�����ڲ�ͣ�ķ���
};

#include <FS_MyClient/FS_MyClient/ClientCfgs/FS_ClientCfgMgrImpl.h>

extern FS_ClientCfgMgr *g_ClientCfgMgr;                       // �ͻ�������

