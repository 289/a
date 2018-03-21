#ifndef GAMED_GS_GLOBAL_DATE_TIME_H_
#define GAMED_GS_GLOBAL_DATE_TIME_H_


namespace gamed {

    inline bool IsLeapYear(int year)
	{
		if((year % 4) != 0)   return false;
		if((year % 400) == 0) return true;
		if((year % 100) == 0) return false;
		return true;
	}

	inline int GetMDay(int year, int mon)
	{
		static int mday[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
		int d = mday[mon];
		if(mon == 1 && IsLeapYear(year)) d = d+1;
		return d;
	}

    inline struct tm GetLocalTimeTM(time_t t)
    {
        struct tm tt;
        localtime_r(&t, &tt);
        return tt;
    }

} // namespace gamed

#endif // GAMED_GS_GLOBAL_DATE_TIME_H_
