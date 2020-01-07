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


FS_NAMESPACE_BEGIN
class User : public IUser
{
public:
    User(UInt64 sessionId, UInt64 generatorId, IFS_MsgDispatcher *dispatcher)
        :_sessionId(sessionId)
        , _recvMsgId(1)
        , _sendMsgId(1)
        , _belongTransferId(generatorId)
        , _dispatcher(dispatcher)
    {
    }
    ~User()
    {
    }

    virtual UInt64 GetSessionId() const
    {
        return _sessionId;
    }

    virtual void Close()
    {
        // TODO:��һ���Ͽ�session����Ϣ��transfer ͨ��dispatcher
        // _dispatcher->CloseSession(_sessionId, _belongTransferId);
    }

    // NetMsg_DataHeader �����Ƕ���������
    void SendData(UInt64 sessionId, UInt64 generatorId, NetMsg_DataHeader *msgData)
    {
        // _dispatcher->SendData(sessionId, generatorId, msgData);
    }

    void OnDisconnect()
    {
        g_Log->net<User>(" session id[%llu] user disconnect", _sessionId);
    }



    UInt64 _sessionId;
    // ����server�����յ�����ϢID�Ƿ����� ÿ�յ�һ���ͻ�����Ϣ������1�Ա���ͻ��˵�msgidУ�飬��ƥ���򱨴���˵�������ȣ�
    Int32 _recvMsgId = 1;
    // ���Խ��շ��߼���
    // ����client�����յ�����ϢID�Ƿ����� ÿ����һ����Ϣ������1�Ա���ͻ��˵�sendmsgidУ�飬��ƥ����ͻ��˱���˵�������ȣ�
    Int32 _sendMsgId = 1;
    IFS_MsgDispatcher *_dispatcher;
    UInt64 _belongTransferId;
    FS_Timer _time;
};

class MyLogic : public IFS_BusinessLogic
{
public:
    std::map<UInt64, User *> _users;

public:
    MyLogic() {}
    virtual ~MyLogic()
    {
        STLUtil::DelMapContainer(_users);
    }
    virtual void Release()
    {
        delete this;
    }

public:
    User * GetUser(UInt64 sessionId)
    {
        auto iterUser = _users.find(sessionId);
        if(iterUser == _users.end())
            return NULL;

        return iterUser->second;
    }

    void OnDisconnect(UInt64 sessionId)
    {
        auto iterUser = _users.find(sessionId);
        if(iterUser == _users.end())
            return;

        iterUser->second->OnDisconnect();
    }

    void RemoveUser(UInt64 sessionId)
    {
        auto iterUser = _users.find(sessionId);
        if(iterUser == _users.end())
            return;

        FS_Release(iterUser->second);
        _users.erase(iterUser);
    }

    User *NewUser(UInt64 sessionId, UInt64 generatorId)
    {
        auto user = new User(sessionId, generatorId, _dispatcher);
        _users.insert(std::make_pair(sessionId, user));
        return user;
    }

    virtual Int32 Start()
    {
        return StatusDefs::Success;
    }

    void BeforeClose()
    {
        //STLUtil::DelMapContainer(_users);
    }

    virtual void Close()
    {
    }

    virtual void OnSessionDisconnected(UInt64 sessionId, std::list<IDelegate<void, IUser *> *> *disconnectedDelegate)
    {
        auto user = GetUser(sessionId);
        if(user)
        {
            if(disconnectedDelegate)
            {
                for(auto iterDelegate = disconnectedDelegate->begin(); iterDelegate != disconnectedDelegate->end(); )
                {
                    auto item = *iterDelegate;
                    item->Invoke(user);
                    FS_Release(item);
                    iterDelegate = disconnectedDelegate->erase(iterDelegate);
                }
            }

            user->OnDisconnect();
        }

        RemoveUser(sessionId);
        // g_Log->any<MyLogic>("sessionid[%llu] Disconnected", sessionId);
    }

    virtual fs::IUser *OnSessionConnected(UInt64 sessionId, UInt64 generatorId)
    {
        return NewUser(sessionId, generatorId);
    }

    virtual void OnMsgDispatch(UInt64 sessionId, UInt64 generatorId, NetMsg_DataHeader *msgData)
    {
        auto user = GetUser(sessionId);
        if(!user)
            return;

        // g_Log->any<MyLogic>("sessionid[%llu] handle a msg", sessionId);
        switch(msgData->_cmd)
        {
            case fs::ProtocolCmd::LoginReq:
            {
                fs::LoginReq *login = static_cast<fs::LoginReq *>(msgData);

                //                 g_Log->custom("user[%s] pwd[%s] login. at dispatcher[%d]"
                //                               , login->_userName, login->_pwd, _dispatcher->GetId());
                                //     g_Log->any<MyLogic>("sessionid[%llu] login username[%s], pwd[%s] msgId[%d] user recvmsgid"
                                //                         , sessionId, login->_userName, login->_pwd, login->_msgId, user->_recvMsgId);
                                     // �����ϢID
                if(login->_msgId != user->_recvMsgId)
                {//��ǰ��ϢID�ͱ�������Ϣ������ƥ��
                    g_Log->custom("OnMsgDispatch sessionId<%llu> msgID<%d> _nRecvMsgID<%d> diff<%d>"
                                  , sessionId, login->_msgId
                                  , user->_recvMsgId, login->_msgId - user->_recvMsgId);
                }

                // ���ذ�
                ++user->_recvMsgId;

                for(Int32 i = 0; i < 1; ++i)
                {
                    fs::LoginRes ret;
                    ret._msgId = user->_sendMsgId;
                    user->SendData(sessionId, generatorId, &ret);
                    ++user->_sendMsgId;
                }

                return;
            }//���� ��Ϣ---���� ����   ������ ���ݻ�����  ������ 
            break;
            case fs::ProtocolCmd::LogoutReq:
            {
                fs::FS_MsgReadStream r(msgData);
                // ��ȡ��Ϣ����
                r.ReadInt16();
                // ��ȡ��Ϣ����
                r.GetNetMsgCmd();
                auto n1 = r.ReadInt8();
                auto n2 = r.ReadInt16();
                auto n3 = r.ReadInt32();
                auto n4 = r.ReadFloat();
                auto n5 = r.ReadDouble();
                uint32_t n = 0;
                r.ReadWithoutOffsetPos(n);
                char name[32] = {};
                auto n6 = r.ReadArray(name, 32);
                char pw[32] = {};
                auto n7 = r.ReadArray(pw, 32);
                int ata[10] = {};
                auto n8 = r.ReadArray(ata, 10);
                ///
                fs::FS_MsgWriteStream s(128);
                s.SetNetMsgCmd(fs::ProtocolCmd::LogoutNty);
                s.WriteInt8(n1);
                s.WriteInt16(n2);
                s.WriteInt32(n3);
                s.WriteFloat(n4);
                s.WriteDouble(n5);
                s.WriteArray(name, n6);
                s.WriteArray(pw, n7);
                s.WriteArray(ata, n8);
                s.Finish();

                // TODO:��Ҫ֧��������
//                 _dispatcher->SendData(sessionId, )
//                 client->SendData(*s.GetDataAddr(), s.GetWrLength());
                //                 g_Log->i<EasyFSServer>(_LOGFMT_("socket<%d> logout")
                //                                        , static_cast<Int32>(client->GetSocket()));
                return;
            }
            break;
            case fs::ProtocolCmd::CheckHeartReq:
            {
                fs::CheckHeartRes ret;
                //_dispatcher->SendData(sessionId, &ret);
                //g_Log->any("socket<%d> CheckHeartReq", static_cast<Int32>(client->GetSocket()));
                return;
            }
            default:
            {
                g_Log->custom("recv <sessionId=%llu> undefine msgType,dataLen��%hu"
                              , sessionId, msgData->_packetLength);
            }
            break;
        }

        return;
    }
};
// 
// class FS_MyClient : public FS_NetEngine
// {
// public:
//     FS_MyClient()
//     {
//         _config = new FS_ClientCfgMgr;
//     }
//     ~FS_MyClient()
//     {
//         STLUtil::DelVectorContainer(_logics);
//         Fs_SafeFree(_config);
//     }
// 
// protected:
//     // ��ȡ����λ��
//     virtual Int32 _OnReadCfgs()
//     {
//         auto ret = _config->Init();
//         if(ret != StatusDefs::Success)
//         {
//             g_Log->e<FS_MyClient>(_LOGFMT_("config Init fail ret[%d]"), ret);
//             return ret;
//         }
// 
//         // TODO:
//         _totalCfgs = new NetEngineTotalCfgs;
//         auto &commonConfig = _totalCfgs->_commonCfgs;
//         commonConfig._maxSessionQuantityLimit = _config->GetMaxSessionQuantityLimit();
//         commonConfig._acceptorQuantityLimit = _config->GetAcceptorQuantity();
//         commonConfig._dispatcherQuantity = _config->GetDispatcherCnt();
//         commonConfig._transferQuantity = _config->GetTransferCnt();
// 
//         auto &connectorCfg = _totalCfgs->_connectorCfgs;
//         connectorCfg._connectTimeOutMs = _config->GetConnectorConnectTimeOutMs();
// 
//         auto &acceptorCfg = _totalCfgs->_acceptorCfgs;
//         acceptorCfg._ip = _config->GetListenIp();
//         acceptorCfg._port = _config->GetListenPort();
// 
//         auto &transferCfg = _totalCfgs->_transferCfgs;
//         transferCfg._maxAlloctorBytesPerTransfer = _config->GetMaxAllowAlloctorBytesPerDispatcher();
//         transferCfg._prepareBufferPoolCnt = _config->GetPrepareBufferCnt();
// 
//         auto &dispatcherCfg = _totalCfgs->_dispatcherCfgs;
//         dispatcherCfg._heartbeatDeadTimeMsInterval = _config->GetHeartbeatDeadTimeIntervalMs();
//         dispatcherCfg._dispatcherResolutionInterval = _config->GetDispatcherResolutionIntervalMs()*Time::_microSecondPerMilliSecond;
// 
//         auto &objPoolCfgs = _totalCfgs->_objPoolCfgs;
//         objPoolCfgs._maxAllowObjPoolBytesOccupied = _config->GetMaxAllowObjPoolBytesOccupied();
// 
//         auto &mempoolCfgs = _totalCfgs->_mempoolCfgs;
//         mempoolCfgs._maxAllowMemPoolBytesOccupied = _config->GetMaxAllowMemPoolBytesOccupied();
//         return StatusDefs::Success;
//     }
// 
//     // ��ʼ������ʱ
//     virtual Int32 _OnInitFinish()
//     {
//         _logics.resize(_totalCfgs->_commonCfgs._dispatcherQuantity);
//         Int32 quantity = static_cast<Int32>(_logics.size());
//         for(Int32 i = 0; i < quantity; ++i)
//             _logics[i] = new fs::MyLogic;
// 
//         return StatusDefs::Success;
//     }
//     // ��ȡҵ���,�Ա�󶨵�dispatcher��
//     virtual void _GetLogics(std::vector<IFS_BusinessLogic *> &logics)
//     {
//         logics = _logics;
//     }
// 
// private:
//     FS_ClientCfgMgr * _config;
//     std::vector<fs::IFS_BusinessLogic *> _logics;
// };
// 
// FS_NAMESPACE_END
// 
// void FS_ClientRun::Run()
// {
//     // 1.ʱ��
//     fs::TimeUtil::SetTimeZone();
// 
//     // 2.���ܱ���������ʶ��
//     fs::SmartVarRtti::InitRttiTypeNames();
// 
//     // 3.��ʼ���ֲ߳̾��洢���
//     Int32 ret = fs::FS_TlsUtil::CreateUtilTlsHandle();
//     if(ret != StatusDefs::Success)
//     {
//         std::cout << "error:" << ret << std::endl;
//         return;
//     }
// 
//     // 4.log��ʼ�� NULLĬ���Գ�����Ϊ��׼����Ŀ¼
//     ret = g_Log->InitModule(NULL);
//     if(ret != StatusDefs::Success)
//     {
//         std::cout << "error:" << ret << std::endl;
//         return;
//     }
// 
//     // 5. crash dump switch start
//     ret = fs::CrashHandleUtil::InitCrashHandleParams();
//     if(ret != StatusDefs::Success)
//     {
//         g_Log->e<FS_ClientRun>(_LOGFMT_("init crash handle params fail ret[%d]"), ret);
//         return;
//     }
// 
//     // 6.��С���жϣ�������ֻ֧��x86��С���ֽ����cpu
//     if(!fs::SystemUtil::IsLittleEndian())
//     {
//         ret = StatusDefs::SystemUtil_NotLittleEndian;
//         g_Log->e<FS_ClientRun>(_LOGFMT_("not little endian ret[%d]"), ret);
//         return;
//     }
// 
//     // ��ʼ������
//     g_CfgMgr.Init();
// 
//     // �����ն������߳�
//     // ���ڽ�������ʱ�û������ָ��
//     const auto threadQuantity = g_CfgMgr.GetThreadQuantity();
//     fs::FS_ThreadPool *pool = new fs::FS_ThreadPool(0, threadQuantity + 1);
// 
//     //����ģ��ͻ����߳�
//     for(int n = 0; n < threadQuantity; n++)
//     {
//         fs::ITask *t = new ClientTask(pool, n);
//         pool->AddTask(*t, true, 1);
//     }
// 
//     // ÿ������ͳ��
//     fs::Time lastTime;
//     fs::Time nowTime;
//     nowTime.FlushTime();
//     lastTime.FlushTime();
//     fs::TimeSlice diff(1);
//     const auto clientQuantity = g_CfgMgr.GetClientQuantity();
//     while(true)
//     {
//         auto t = nowTime.FlushTime() - lastTime;
//         if(t >= diff)
//         {
//             g_Log->custom("thread<%d>,clients<%d>,connect<%d>,time<%d>,send<%d>"
//                           , threadQuantity, clientQuantity, (int)g_ConnectNum, t.GetTotalSeconds(), (int)g_SendCount);
//             g_SendCount = 0;
//             lastTime.FlushTime();
//         }
//         fs::SystemUtil::Sleep(1);
//     }
// 
//     //
//     pool->Close();
//     g_Log->custom("�������˳�����");
//     return;
// }

FS_NAMESPACE_END

void FS_ClientRun::Run()
{
}
#endif
