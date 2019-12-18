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
 * @file  : FS_MyClient.cpp
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/12/6
 * @brief :
 * 
 *
 * 
 */
#include "stdafx.h"
#include <FS_MyClient/FS_MyClient/FS_MyClient.h>

static fs::FS_ClientCfgMgr g_CfgMgr;

#ifdef _WIN32

FS_MyClient::FS_MyClient()
{

}

FS_MyClient::~FS_MyClient()
{
}

Int32 FS_MyClient::Init()
{
    _checkMsgID = g_CfgMgr.NeedCheckMsgId();

    return  fs::FS_IocpTcpClient::Init();
}

void FS_MyClient::OnNetMsg(fs::NetMsg_DataHeader *header)
{
    _isSend = false;
    switch(header->_cmd)
    {
        case fs::ProtocolCmd::LoginRes:
        {
            fs::LoginRes *loginRes = static_cast<fs::LoginRes *>(header);
            if(_checkMsgID)
            {
                if(loginRes->_msgId != _recvMsgID)
                {// ��ǰ��ϢID�ͱ�������Ϣ������ƥ��
                    g_Log->e<FS_MyClient>(_LOGFMT_("OnNetMsg sessionId<%llu> socket<%llu> msgID<%d> _nRecvMsgID<%d> diff<%d>")
                                          , _session->GetSessionId()
                                          , _session->GetSocket()
                                          , loginRes->_msgId
                                          , _recvMsgID
                                          , loginRes->_msgId - _recvMsgID);
                }
                ++_recvMsgID;
            }
            //CELLLog_Info("<socket=%d> recv msgType��CMD_LOGIN_RESULT", (int)_pClient->sockfd());
        }
        break;
        default:
        {
            g_Log->w<FS_MyClient>(_LOGFMT_("error, <sessionId<%llu>, socket=%llu, cmd<%hu> len<%hu>> recv undefine msgType")
                                  , _session->GetSessionId(), _session->GetSocket()
                                  , header->_cmd, header->_packetLength);
        }
    }
}

Int32 FS_MyClient::SendTest(fs::LoginReq *login)
{
    Int32 ret = 0;
    //���ʣ�෢�ʹ�������0
    if(_sendCount > 0 && !_isSend)
    {
        login->_msgId = _sendMsgID;
        ret = SendData(login);
        //CELLLog_Info("%d", _nSendMsgID);
        if(SOCKET_ERROR != ret)
        {
            _isSend = g_CfgMgr.IsSendAfterSvrResArrive();
            ++_sendMsgID;
            // ���ʣ�෢�ʹ�������һ��
            --_sendCount;
        }
    }

    return ret;
}

bool FS_MyClient::CheckSend(time_t dt)
{
    _restTime += dt;
    // ÿ����nSendSleep����
    auto sleepIntval = g_CfgMgr.GetSendPeriod();
    sleepIntval = sleepIntval > 0 ? sleepIntval : 1;

    if(_restTime >= sleepIntval)
    {
        // ���ü�ʱ
        _restTime -= sleepIntval;
        // ���÷��ͼ���
        _sendCount = g_CfgMgr.GetMsgNumPerPeriod();
    }

    return _sendCount > 0;
}



void OnTaskRun(fs::FS_ThreadPool *pool)
{
    
}

ClientTask::ClientTask(fs::FS_ThreadPool *pool, Int32 id)
    :_id(id)
    ,_pool(pool)
{

}

ClientTask::~ClientTask()
{
}

std::atomic_int g_SendCount(0);
std::atomic_int g_ReadyCount(0);
std::atomic_int g_ConnectNum(0);

Int32 ClientTask::Run()
{
    g_Log->i<ClientTask>(_LOGFMT_("thead<%d>,start"), _id);

    // ����
    const auto clientQuantity = g_CfgMgr.GetClientQuantity();
    const auto threadQuantity = g_CfgMgr.GetThreadQuantity();
    const auto sendSleepInterval = std::max<Int32>(g_CfgMgr.GetSendPeriod(), 1);
    const auto &ip = g_CfgMgr.GetTargetSvrIp();
    const auto port = g_CfgMgr.GetTargetSvrPort();
    const auto msgNumPerPeriod = g_CfgMgr.GetMsgNumPerPeriod();
    const auto workSleep = 1;

    // �ͻ�����
    std::vector<FS_MyClient *> clients(clientQuantity);

    // ���㱾�߳̿ͻ�����clients�ж�Ӧ��index
    int begin = 0;
    int end = clientQuantity / threadQuantity;
    if(end < 1)
        end = 1;

    int nTemp1 = sendSleepInterval > 0 ? sendSleepInterval : 1;
    for(int n = begin; n < end; n++)
    {
        if(!_pool->IsPoolWorking())
            break;

        clients[n] = new FS_MyClient();
        clients[n]->Init();
        clients[n]->_restTime = n % nTemp1;

        // ���߳�ʱ����CPU
        fs::SocketUtil::Sleep(0);
    }
    for(int n = begin; n < end; n++)
    {
        if(!_pool->IsPoolWorking())
            break;

        if(INVALID_SOCKET == clients[n]->InitSocket())
            break;

        if(StatusDefs::Success != clients[n]->Connect(ip.c_str(), port))
            break;

        ++g_ConnectNum;
        fs::SocketUtil::Sleep(0);
    }

    // �����������
    g_Log->i<ClientTask>(_LOGFMT_("thread<%d>,Connect<begin=%d, end=%d ,nConnect=%d>")
                         , _id, begin, end, (Int32)g_ConnectNum);

    ++g_ReadyCount;
    while(g_ReadyCount < threadQuantity && _pool->IsPoolWorking())
    {// �ȴ������߳�׼���ã��ٷ�������
        fs::SocketUtil::Sleep(10);
    }

    // ��Ϣ
    fs::LoginReq login;
    // �����������ֵ
    strcpy(login._userName, "lyd");
    strcpy(login._pwd, "lydmm");

    //
    // �շ����ݶ���ͨ��onRun�߳�
    // SendDataֻ�ǽ�����д�뷢�ͻ�����
    // �ȴ�select����дʱ�Żᷢ������
    // �ɵ�ʱ���
    auto t2 = fs::Time::Now();
    // �µ�ʱ���
    auto t0 = t2;
    // ������ʱ��
    fs::TimeSlice dt;
    while(_pool->IsPoolWorking())
    {
        t0.FlushTime();
        dt = t0 - t2;
        t2 = t0;

        // ����while (pThread->isRun())ѭ����Ҫ��������
        // ����work
        {
            Int32 count = 0;
            // ÿ��ÿ���ͻ��˷���nMsg������
            for(Int32 m = 0; m < msgNumPerPeriod; m++)
            {
                // ÿ���ͻ���1��1����д����Ϣ
                for(int n = begin; n < end; n++)
                {
                    if(clients[n]->IsRun())
                    {
                        if(clients[n]->SendTest(&login) > 0)
                        {
                            ++g_SendCount;
                        }
                    }
                }
            }

            // sendCount += count;
            for(int n = begin; n < end; n++)
            {
                if(clients[n]->IsRun())
                {	// ��ʱ����Ϊ0��ʾselect���״̬����������
                    if(!clients[n]->OnRun(0))
                    {	//���ӶϿ�
                        --g_ConnectNum;
                        continue;
                    }
                    //��ⷢ�ͼ����Ƿ���Ҫ����
                    clients[n]->CheckSend(dt.GetTotalMilliSeconds());
                }
            }
        }

        fs::SystemUtil::Sleep(workSleep);
    }

    // --------------------------
    // �ر���Ϣ�շ��߳�
    // tRun.Close();
    // �رտͻ���
    for(int n = begin; n < end; n++)
    {
        clients[n]->Close();
        delete clients[n];
    }

    g_Log->i<ClientTask>(_LOGFMT_("thread<%d>,exit"), _id);
    --g_ReadyCount;

    return 0;
}

void FS_ClientRun::Run()
{
    // 1.ʱ��
    fs::TimeUtil::SetTimeZone();

    // 2.���ܱ���������ʶ��
    fs::SmartVarRtti::InitRttiTypeNames();

    // 3.��ʼ���ֲ߳̾��洢���
    Int32 ret = fs::FS_TlsUtil::CreateUtilTlsHandle();
    if(ret != StatusDefs::Success)
    {
        std::cout << "error:" << ret << std::endl;
        return;
    }

    // 4.log��ʼ�� NULLĬ���Գ�����Ϊ��׼����Ŀ¼
    ret = g_Log->InitModule(NULL);
    if(ret != StatusDefs::Success)
    {
        std::cout << "error:" << ret << std::endl;
        return;
    }

    // 5. crash dump switch start
    ret = fs::CrashHandleUtil::InitCrashHandleParams();
    if(ret != StatusDefs::Success)
    {
        g_Log->e<FS_ClientRun>(_LOGFMT_("init crash handle params fail ret[%d]"), ret);
        return;
    }

    // 6.��С���жϣ�������ֻ֧��x86��С���ֽ����cpu
    if(!fs::SystemUtil::IsLittleEndian())
    {
        ret = StatusDefs::SystemUtil_NotLittleEndian;
        g_Log->e<FS_ClientRun>(_LOGFMT_("not little endian ret[%d]"), ret);
        return;
    }

    // ��ʼ������
    g_CfgMgr.Init();

    // �����ն������߳�
    // ���ڽ�������ʱ�û������ָ��
    const auto threadQuantity = g_CfgMgr.GetThreadQuantity();
    fs::FS_ThreadPool *pool = new fs::FS_ThreadPool(0, threadQuantity + 1);

    //����ģ��ͻ����߳�
    for(int n = 0; n < threadQuantity; n++)
    {
        fs::ITask *t = new ClientTask(pool, n);
        pool->AddTask(*t, true, 1);
    }

    // ÿ������ͳ��
    fs::Time lastTime;
    fs::Time nowTime;
    nowTime.FlushTime();
    lastTime.FlushTime();
    fs::TimeSlice diff(1);
    const auto clientQuantity = g_CfgMgr.GetClientQuantity();
    while(true)
    {
        auto t = nowTime.FlushTime() - lastTime;
        if(t >= diff)
        {
            g_Log->custom("thread<%d>,clients<%d>,connect<%d>,time<%d>,send<%d>"
                          , threadQuantity, clientQuantity, (int)g_ConnectNum, t.GetTotalSeconds(), (int)g_SendCount);
            g_SendCount = 0;
            lastTime.FlushTime();
        }
        fs::SystemUtil::Sleep(1);
    }

    //
    pool->Close();
    g_Log->custom("�������˳�����");
    return;
}
#endif
