#ifndef __Base_Common_Status_Defs_StatusDefs_H__
#define __Base_Common_Status_Defs_StatusDefs_H__
/**
* @file StatusDefs.h
* @auther Huiya Song <120453674@qq.com>
* @date 2019/04/18
* @brief
*/

#pragma once

#include<base/exportbase.h>

class BASE_EXPORT StatusDefs
{
public:
    enum StatusEnum:int
    {
        #pragma region ͨ����[-1-499]
        Error = -1,                                             // ����
        Success = 0,                                            // �ɹ�
        WaitEventFailure = 1,                                   // ʱ�����ȴ�ʧ��
        WaitEventTimeOut = 2,                                   // �¼�����ȴ���ʱ
        Repeat = 3,                                             // �ظ�
        ParamError = 4,                                         // ��������
        #pragma endregion

        #pragma region SOCKET[500-599]
        Socket_CreateFailure = 500,                             // �����׽���ʧ��
        Socket_NotInit = 501,                                   // δ��ʼ���׽���
        #pragma endregion

        #pragma region ������[600-699]
        Trigger_InvalidOccasion = 600,                          // ��Ч����ʱ��
        Trigger_InvalidTriggerType = 601,                       // ��Ч������
        Trigger_UnkownAddType = 602,                            // ע�ᴥ����ʱδ֪�ļ��뷽ʽ
        Trigger_TriggerTypeRepeatInOccasion = 603,              // �ڼ����ʱ���д����������ظ�
        #pragma endregion 

        #pragma region ��½[600-699]
        #pragma endregion
    };
};

#endif //!__Base_Common_Status_Defs_StatusDefs_H__
