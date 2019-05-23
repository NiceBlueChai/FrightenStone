#ifndef __Base_Common_Net_Protocol_Defs_ProtocolDefs__H__
#define __Base_Common_Net_Protocol_Defs_ProtocolDefs__H__
/**
* @file ProtocolDefs.h
* @auther Huiya Song <120453674@qq.com>
* @date 2019/04/18
* @brief
*/

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/BaseDefs.h"
#include "base/common/status/status.h"

#pragma region socket����

// �������Ͷ���
#ifdef _WIN32
typedef unsigned long long MYSOCKET;
#else
typedef int MYSOCKET;

#endif

// �껺����
#ifndef SOCKET_CACHE_SIZE
#define SOCKET_CACHE_SIZE 102400
#endif // !SOCKET_CACHE_SIZE

#define MYINVALID_SOCKET static_cast<MYSOCKET>(~0)
#ifndef SOCKET_ERROR
#define SOCKET_ERROR            (-1)
#endif

#pragma endregion

#pragma region �����ݽṹ����

// Э���
class BASE_EXPORT ProtocolCmd
{
public:
    enum
    {
        CMD_Begin = 0,              // Э�����ʼ
        LoginReq = 1,               // ��½
        LoginNty = 2,               // ��½�������ݰ�
        LoginRes = 3,               // ��½��Ӧ
        LogoutReq = 4,              // �ǳ�ע��
        LogoutNty = 5,              // �ǳ���������
        LogoutRes = 6,              // �ǳ���Ӧ
        CreatePlayerReq = 7,        // ������ɫ
        CreatePlayerRes = 8,        // ������ɫ��Ӧ
        CreatePlayerNty = 9,        // ������ɫ
    };
};

struct BASE_EXPORT PacketHeader
{
    PacketHeader();
    UInt16 _packetLength;           // ��ǰ������
    UInt16 _cmd;                    // ����
};

// ��½
struct BASE_EXPORT LoginReq : public PacketHeader
{
    LoginReq();
    char _userName[MAX_NAME_LEN];
    char _pwd[MAX_PWD_LEN];
};

// ��½
// ��Ӧ
struct BASE_EXPORT LoginRes : public PacketHeader
{
    LoginRes();

    char _userName[MAX_NAME_LEN];
    Int32 _status;
};

// ��½
// ��½���Ҫ����������
struct BASE_EXPORT LoginNty : public PacketHeader
{
    LoginNty();
    
    char _userName[MAX_NAME_LEN];
    char _pwd[MAX_PWD_LEN];
};

// 
struct BASE_EXPORT CreatePlayerNty : public PacketHeader
{
    CreatePlayerNty();

    Int32 _socket;
};

#pragma endregion

#endif // !__Base_Common_Net_Protocol_Defs_ProtocolDefs__H__
