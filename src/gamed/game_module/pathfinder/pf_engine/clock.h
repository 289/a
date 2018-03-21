#ifndef __PF_CLOCK_H__
#define __PF_CLOCK_H__

#include <time.h>
#include <string>
#include <stdio.h>

class Clock
{
private:
	std::string desc;
	int32_t time_start;
	int32_t time_end;

public:
	Clock(const std::string& description):
		desc(description),
		time_start(0),
		time_end(0)
	{
	}
	~Clock()
	{
		time_start = 0;
		time_end   = 0;
	}

	void Start()
	{
		time_start = clock();
	}
	void End()
	{
		time_end = clock();
	}

	void ElapseTime() const
	{
		int32_t elapse_time = time_end - time_start;
#ifdef PLATFORM_WINDOWS
		fprintf(stdout, "%s: elaspe_time=(%dsec, %dmsec)\n", desc.c_str(), elapse_time/CLOCKS_PER_SEC, elapse_time%CLOCKS_PER_SEC);
#else
		fprintf(stdout, "%s: elaspe_time=(%ldsec, %dmsec, %ldusec)\n", desc.c_str(), elapse_time/CLOCKS_PER_SEC, elapse_time/1000, elapse_time%CLOCKS_PER_SEC);
#endif
	}
};

#endif // __PF_CLOCK_H__
