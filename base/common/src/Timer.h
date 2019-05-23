#ifndef	TIMER_H_
#define TIMER_H_

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <assert.h>

#include <stdlib.h>

#pragma warning(push)
#pragma warning(disable: 4267)

NAMESPACE_BEGIN(gtool)

	//////////////////////////////////////////////////////////////////////
	// ��ʱ�࣬�ԡ��롱Ϊ��λ
	//////////////////////////////////////////////////////////////////////
	class CTimer  
	{
	public:
		CTimer(int nIntervalSecs = 0) { m_nInterval = nIntervalSecs; m_tUpdateTime = 0; }		// =0 : ���������ʼ��
		CTimer& operator = (const CTimer& rhs)
		{
			m_nInterval		= rhs.m_nInterval;
			m_tUpdateTime	= rhs.m_tUpdateTime;
			return *this;
		}
		
	public:
		// ����ʱ���ǩ
		void	Update		(void)					{ m_tUpdateTime = ::time(NULL); }
		
		// ���ڶ�ʱ����������ʱ�ӡ�ÿ�δ���ʱΪtrue��
		bool	ToNextTime	(void)					{ if(this->IsTimeOut()) return this->Update(),true; else return false; }
		
		// ��ʼ��ʱ��Ƭ��(��������
		void	SetInterval	(int nSecs)				{ m_nInterval = nSecs; }
		
		// ��ʼ����ʱ��(ͬSetInterval)
		void	Startup		(int nSecs)				{ m_nInterval = nSecs; this->Update(); }
		
		// ʱ���Ƿ񴥷�(ֻ����һ��)����Startup���ʹ�ã���ʵ��ֻ����һ�ε�ʱ�ӡ�
		bool	TimeOver	(void)					{ if(this->IsActive() && this->IsTimeOut()) return this->Clear(),true; return false; }
		void	Clear		(void)					{ m_tUpdateTime = m_nInterval = 0; }
		bool	IsActive	(void)	const			{ return m_tUpdateTime != 0; }
		void	IncInterval	(int nSecs, int nLimit)	{ const int n = m_nInterval+nSecs; m_nInterval = __min(n, nLimit); }
		void	DecInterval	(int nSecs)				{ const int n = m_nInterval+nSecs; m_nInterval = __max(n, 0); }
		
		// ����Ƿ�ʱ
		bool	IsTimeOut	(void)		const			{ if(m_tUpdateTime==0) return false;return ::time(NULL) >= m_tUpdateTime+m_nInterval; }
		
		// ����Ƿ�ʱ��ָ��ʱ��Ƭ������ͬһʱ���ǩ�������ͬ��ʱ�¼���(��ʵ��һ��������ƶ��ʱ��)
		bool	IsTimeOut	(int nSecs)	const			{ if(m_tUpdateTime==0) return false;return ::time(NULL) >= m_tUpdateTime+nSecs; }
		
		// ���ڱ䳤�ĳ�ʱ�¼���
		bool	ToNextTime	(int nSecs)				{ if(this->IsTimeOut(nSecs)) return this->Update(),true; else return false; }
		
		// ʱ�ӻ���ۣ�������ֵ���������ֵ������ͬToNextTime(...)
		bool	ToNextTick	(int nSecs);
		//	void	AppendInterval	(int nSecs)			{ if(ToNextTime()) m_nInterval=nSecs; else m_nInterval+=nSecs; }	// ToNextTime(): ��֤������nSecs��
		
		bool  MakeTimeOut(void)
		{
			m_tUpdateTime = (clock()/1000);
			if(m_tUpdateTime > m_nInterval)
			{
	   			m_tUpdateTime -= m_nInterval;
	   		return true;
			}
			else
			{
				m_tUpdateTime = 1;		
			}
			return false;
		}	
		
	public: // get
		int		GetRemain()					const	{ if(!IsActive()) return 0; const int n = m_nInterval - (int)(::time(NULL)-m_tUpdateTime); return m_tUpdateTime ? min(max(n, 0), m_nInterval) : 0; }
		int		GetInterval()						{ return m_nInterval;}
		void	StartDayCheck()						{ Startup(GetRemainSecsToZero()); }
		bool	IsDayPassed()						{ if(IsActive() && IsTimeOut()) return Startup(GetRemainSecsToZero()), true; return false; }
		DWORD  GetRemainSecsToZero();
		time_t	GetPassedSecs()						{ if (IsActive()) return ::time(NULL)-m_tUpdateTime; return 0;}

		time_t GetUpdateTime()						{ if (IsActive()) return m_tUpdateTime; return 0; }
		void	SetUpdateTime(time_t time)			{ if (IsActive()) m_tUpdateTime = time; }
		
	protected:
		I32		m_nInterval;
		time_t	m_tUpdateTime;
	};
	
	
	//////////////////////////////////////////////////////////////////////
	// ��ʱ�࣬�ԡ����롱Ϊ��λ
	//////////////////////////////////////////////////////////////////////
	class CTimerMS
	{
	public:
		CTimerMS(int nIntervalSecs = 0) { m_nInterval = nIntervalSecs; m_tUpdateTime = 0; }		// =0 : ���������ʼ��
		// virtual ~CTimerMS() {}
		CTimerMS& operator = (const CTimerMS& rhs)
		{
			m_nInterval		= rhs.m_nInterval;
			m_tUpdateTime	= rhs.m_tUpdateTime;
			return *this;
		}
		
	public:
		void	Update		(void)					{ m_tUpdateTime = clock(); }
		bool	IsTimeOut	(void)			const		{ if(m_tUpdateTime==0) return false;return clock() >= m_tUpdateTime+m_nInterval; }
		bool	ToNextTime	(void)					{ if(IsTimeOut()) return Update(),true; else return false; }
		void	SetInterval	(int nMilliSecs)		{ m_nInterval = nMilliSecs; }
		
		void	Startup		(int nMilliSecs)		{ m_nInterval = nMilliSecs; Update(); }
		bool	TimeOver	(void)					{ if(IsActive() && IsTimeOut()) return Clear(),true; return false; }
		void	Clear		(void)					{ m_nInterval = m_tUpdateTime = 0; }
		bool	IsActive	(void)	const			{ return m_tUpdateTime != 0; }
		void	IncInterval	(int nMilliSecs, int nLimit)		{ int n = m_nInterval+nMilliSecs; m_nInterval = min(n, nLimit); }
		void	DecInterval	(int nMilliSecs)		{ const int n = m_nInterval-nMilliSecs; m_nInterval =max(n, 0); }
		
		bool	IsTimeOut	(int nMilliSecs)	const	{ return clock() >= m_tUpdateTime+nMilliSecs; }
		bool	ToNextTime	(int nMilliSecs)		{ if(IsTimeOut(nMilliSecs)) return Update(),true; else return false; }
		bool	ToNextTick	(int nMilliSecs);
		//	void	AppendInterval	(int nMilliSecs)	{ if(ToNextTime()) m_nInterval=nMilliSecs; else m_nInterval+=nMilliSecs; }	// ToNextTime(): ��֤������nSecs��
		
	public: // get
		int		GetRemain(void)				const	{ if(!IsActive()) return 0; const int n = m_nInterval - (clock()-m_tUpdateTime);return m_tUpdateTime ? min(max(n, 0), m_nInterval) : 0; }
		int		GetInterval	(void)					{ return m_nInterval; }
		DWORD	GetUpdateTime(void)					{ return m_tUpdateTime; }
		int		GetPassedMS()						{ if (IsActive()) return clock()-m_tUpdateTime; return 0;}
		
	protected:
		int		m_nInterval;
		clock_t	m_tUpdateTime;
	};

NAMESPACE_END(gtool)

#pragma warning(pop)


#endif
