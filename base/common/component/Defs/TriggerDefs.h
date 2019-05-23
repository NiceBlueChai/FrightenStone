#ifndef __Base_Common_Component_Defs_TriggerDefs_H__
#define __Base_Common_Component_Defs_TriggerDefs_H__
/**
* @file TriggerDefs.h
* @auther Huiya Song <120453674@qq.com>
* @date 2019/04/18
* @brief
*/

#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/Macro/ForAll/ForAllMacro.h"
#include "base/common/basedefs/DataType/DataType.h"
#include <functional>
#include <set>
#include <map>

FS_NAMESPACE_BEGIN

class BASE_EXPORT Trigger;

class BASE_EXPORT TriggerDefs
{
public:
    // �������ִ������뷽��
    enum AddMultiTriggerType
    {
        NotAddIfExist = 0,              // �����������
        AddIfExist = 1,                 // ������Ҳ���Լ���
        EraseAllBeforeAdd = 2,          // �����������Ƴ�����
    };

    enum
    {
        Trig_Infinite = -1,               // ���޴�������
    };
};

struct BASE_EXPORT TriggerExecuteBody
{
public:
    TriggerExecuteBody(Int32 triggerType, Int32 execTimes = 1);

    Int32 _execTimes;
    Int32 _triggerType;
    std::function<Int32(TriggerExecuteBody *)> _exec;
};

// ��֧��ʱ��˳��ִ��ÿ���¼���ͬһtriggerTypeÿ��ʱ����ֻ��ע��һ��
class BASE_EXPORT TriggerOccasion
{
public:
    TriggerOccasion(Trigger *trigger, Int32 occasion);
    virtual ~TriggerOccasion();

public:
    Int32 Reg(Int32 triggerType, const std::function<Int32(TriggerExecuteBody *)> &exec, Int32 execTimes = 1);
    Int32 Exec(std::set<Int32> &triggerType2Erase);
    void Erase(Int32 triggerType);
    bool IsExist(Int32 triggerType) const;

private:
    std::map<Int32, TriggerExecuteBody *>::iterator _Erase(const std::map<Int32, TriggerExecuteBody *>::iterator &iter);

private:
    Trigger *_trigger;
    Int32 _occasion;                                                    // ִ�е�ʱ��
    std::map<Int32, TriggerExecuteBody *> _triggerTypeRefBodys;         // ��Ҫ�ͷ� 
};


FS_NAMESPACE_END

#endif
