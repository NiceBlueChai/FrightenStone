#include "stdafx.h"
#include "base/common/objpool/Defs/ObjPoolDefs.h"
#include "base/common/log/Log.h"
#include "base/common/memleak/memleak.h"

FS_NAMESPACE_BEGIN

const Int32 ObjPoolDefs::__g_FreeRate = 50;           // �ͷ��ڴ�ʱ�ж��ڴ�������ʵ���__g_BusyThresholdValue�ٷ���ʱתΪ�������ڴ�

void ObjPoolMethods::PrintMemleakInfo(const char *objName, size_t nodeCnt, size_t bytesOccupied, size_t memleakObjCnt, size_t memleakBytes)
{
    if(memleakBytes)
    {// �ڴ�й©��ӡ�ڴ�й©
        g_Log->memleak("obj name[%s], pool node cnt[%llu] pool memory bytes occupied[%llu]"
                       " memleak obj cnt[%llu] memleak bytes[%llu]"
                       , objName
                       , nodeCnt
                       , bytesOccupied
                       , memleakObjCnt
                       , memleakBytes);
    }
    
    // ��ӡ�ڴ�ռ����Ϣ
    g_Log->sys(_LOGFMT_("[objpool memory info]: obj name[%s], pool node cnt[%llu] pool memory bytes occupied[%llu]")
               , objName
               , nodeCnt
               , bytesOccupied);
}

void ObjPoolMethods::RegisterToMemleakMonitor(const char *objName, IDelegatePlus<size_t, Int64 &> *callback)
{
    // TODO
    if(!g_MemleakMonitor)
        fs::MemleakMonitor::GetInstance();
    g_MemleakMonitor->RegisterCallback(objName, callback);
}

void ObjPoolMethods::UnRegisterMemleakDelegate(const char *objName)
{
    // TODO
    if(!g_MemleakMonitor)
        fs::MemleakMonitor::GetInstance();

    g_MemleakMonitor->UnRegister(objName);
}

FS_NAMESPACE_END

