#include "stdafx.h"
#include "base/common/log/Defs/LogTask.h"
#include "base/common/status/status.h"
#include "base/common/component/Impl/FS_ThreadPool.h"

FS_NAMESPACE_BEGIN

LogTask::LogTask(FS_ThreadPool *pool, IDelegatePlus<void> *taskDelegate, Int32 workIntervalMsTime)
    :_taskDelegate(taskDelegate)
    ,_pool(pool)
    ,_workIntervalMsTime(workIntervalMsTime)
{

}

Int32 LogTask::Run()
{
    while(true)
    {
        // ���������ж�
        if(_pool->IsClearingPool())
        {
            (*_taskDelegate)();
            break;
        }

        // д��־
        (*_taskDelegate)();

        // ��Ϣһ���
        Sleep(_workIntervalMsTime);
    }
    auto pLogMgr = (CLogMgr *)m_pArg;
    CHECKF_NL(pLogMgr);
    bool bFini = false;
    I32 nTimerDiff = 0;
    LogDataListMap *pMainMap = NULL;
    gtool::CSmartPtr<LogDataListMap> pCacheMap = LogDataListMap::CreateNew();
    CHECKF_NL(pCacheMap);
    LogDataListMap *pCachePoint = pCacheMap;

    DOTRY_B_NL
    {
        pLogMgr->StartFlushTask();
    while(!bFini)
    {
        DOTRY_B_NL
        {
            bFini = pLogMgr->IsFinishLog();
        nTimerDiff = pLogMgr->GetFlushTimerDiff();

        pLogMgr->LockRes();/////////////////////�ٽ���

        //��־Ǩ��
        ASSERT(MigrationLogToCache(*pLogMgr, pMainMap, pCachePoint));
        ASSERT(pCachePoint);

        ASSERT(pLogMgr->UnLockRes());/////////////////////�ٽ���
        }
            DOCATCH_ANY_NL("CFlushLogTask::Run() CRASH")
        {
            if(pLogMgr->IsLock())
            {
                ASSERT(pLogMgr->UnLockRes());/////////////////////�ٽ���
            }
        }
        DOCATCH_ANY_NL_END

            //д��־
            DOTRY_B_NL
        {
            ASSERT(WriteCacheLog(*pLogMgr, pCachePoint));
        ASSERT(pCachePoint);
        }
            DOCATCH_ANY_NL("CFlushLogTask::Run() д��־ crash")
        {
        }
        DOCATCH_ANY_NL_END

            ASSERT(pLogMgr->LockRes());
        ASSERT(pLogMgr->TimeWait(nTimerDiff));
        ASSERT(pLogMgr->UnLockRes());
    }

    pLogMgr->FinishFlushTask();
    }
        DOCATCH_ANY_NL("int CFlushLogTask::Run()")
    {
        pLogMgr->FinishFlushTask();
    }
    DOCATCH_ANY_NL_END

    return StatusDefs::Success;
}

FS_NAMESPACE_END

