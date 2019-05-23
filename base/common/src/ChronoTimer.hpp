#ifndef CHRONO_TIMER_HPP_
#define CHRONO_TIMER_HPP_

#pragma once

class ChronoTimer {
public:
	ChronoTimer()
	{
		FlushTime();
	}
	virtual ~ChronoTimer() {

	}
	void FlushTime()
	{
		_time = std::chrono::high_resolution_clock::now();
	}
	double GetSecInterval()	//��
	{
		return GetMicroSecInterval()*0.000001;
	}
	double GetMilliSecInterval()	//΢��
	{
		return GetMicroSecInterval()*0.001;
	}
	long long GetMicroSecInterval()	//΢��
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _time).count();
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _time;
};


#endif

