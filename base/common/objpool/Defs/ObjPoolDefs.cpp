#include "stdafx.h"
#include "base/common/objpool/Defs/ObjPoolDefs.h"
#include "base/common/log/Log.h"
#include "base/common/memleak/memleak.h"

FS_NAMESPACE_BEGIN

const Int32 ObjPoolDefs::__g_FreeRate = 50;           // �ͷ��ڴ�ʱ�ж��ڴ�������ʵ���__g_BusyThresholdValue�ٷ���ʱתΪ�������ڴ�

void ObjPoolMethods::PrintObjPoolInfo(const char *objName, size_t nodeCnt, size_t totalObjBlocks, size_t objBlockSize, size_t bytesOccupied, size_t memObjInUsingCnt, size_t memInUsingBytes, const char *extStr)
{
    if(memInUsingBytes)
    {// �ڴ�й©��ӡ�ڴ�й©
        g_Log->objpool("obj name[%s], extStr[%s] pool node cnt[%llu] totalObjBlockCnt[%llu] objBlockSize[%llu] pool memory bytes occupiedBytes[%llu]"
                       " memObjInUsingCnt[%llu] memInUsingBytes[%llu]"
                       , objName
                       , extStr
                       , nodeCnt
                       , totalObjBlocks
                       , objBlockSize
                       , bytesOccupied
                       , memObjInUsingCnt
                       , memInUsingBytes);
    }
    
    // ��ӡ�ڴ�ռ����Ϣ
    g_Log->objpool("[objpool memory info cur total occupied bytes[%llu]]: obj name[%s], extStr[%s], pool node cnt[%llu] totalObjBlockCnt[%llu] objBlockSize[%llu] pool memory bytes occupiedBytes[%llu]"
               , (UInt64)(g_curObjPoolOccupiedBytes)
               , objName
               , extStr
               , nodeCnt
               , totalObjBlocks
               , objBlockSize
               , bytesOccupied);
}

void ObjPoolMethods::RegisterToMemleakMonitor(const char *objName, IDelegate<size_t, Int64 &, Int64 &, const char *> *callback)
{
    // TODO
    if(!g_MemleakMonitor)
        fs::MemleakMonitor::GetInstance();
    g_MemleakMonitor->RegisterObjPoolCallback(objName, callback);
}

void ObjPoolMethods::UnRegisterMemleakDelegate(const char *objName)
{
    // TODO
    if(!g_MemleakMonitor)
        fs::MemleakMonitor::GetInstance();

    g_MemleakMonitor->UnRegisterObjPool(objName);
}

void ObjPoolMethods::RegisterModifyAllowMaxBytes(IDelegate<void, UInt64> *callback)
{
    // TODO
    if(!g_MemleakMonitor)
        fs::MemleakMonitor::GetInstance();

    g_MemleakMonitor->RegisterObjPoolModifyMaxAllowBytesCallback(callback);
}

FS_NAMESPACE_END

