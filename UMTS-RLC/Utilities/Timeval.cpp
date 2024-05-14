/*
 * OpenBTS provides an open source alternative to legacy telco protocols and 
 * traditionally complex, proprietary hardware systems.
 *
 * Copyright 2008 Free Software Foundation, Inc.
 * Copyright 2011-2014 Range Networks, Inc.
 *
 * This software is distributed under the terms of the GNU Affero General 
 * Public License version 3. See the COPYING and NOTICE files in the main 
 * directory for licensing information.
 *
 * This use of this software may be subject to additional restrictions.
 * See the LEGAL file in the main directory for details.
 */



#include "Timeval.h"

using namespace std;

#ifdef _WIN32

#include <Windows.h>

#define HAVE_STRUCT_TIMESPEC // for resolve timespec redefinition, msvc version should be higher than vs2013
#define _TIMESPEC_DEFINED

#include <pthread.h>


int gettimeofday(struct timeval_t* tv, struct timezone* tz)
{
	if (tv) {
		FILETIME               filetime; /* 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 00:00 UTC */
		ULARGE_INTEGER         x;
		ULONGLONG              usec;
		static const ULONGLONG epoch_offset_us = 11644473600000000ULL; /* microseconds betweeen Jan 1,1601 and Jan 1,1970 */

#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
		GetSystemTimePreciseAsFileTime(&filetime);
#else
		GetSystemTimeAsFileTime(&filetime);
#endif
		x.LowPart = filetime.dwLowDateTime;
		x.HighPart = filetime.dwHighDateTime;
		usec = x.QuadPart / 10 - epoch_offset_us;
		tv->tv_sec = (time_t)(usec / 1000000ULL);
		tv->tv_usec = (long)(usec % 1000000ULL);
	}
	if (tz) {
		TIME_ZONE_INFORMATION timezone;
		GetTimeZoneInformation(&timezone);
		tz->tz_minuteswest = timezone.Bias;
		tz->tz_dsttime = 0;
	}
	return 0;
}

void usleep(long long usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

#endif

void Timeval::future(unsigned offset)
{
	now();
	unsigned sec = offset/1000;
	unsigned msec = offset%1000;
	mTimeval.tv_usec += msec*1000;
	mTimeval.tv_sec += sec;
	if (mTimeval.tv_usec>1000000) {
		mTimeval.tv_usec -= 1000000;
		mTimeval.tv_sec += 1;
	}
}


struct timespec Timeval::timespec() const
{
	struct timespec retVal;
	retVal.tv_sec = mTimeval.tv_sec;
	retVal.tv_nsec = 1000 * (long)mTimeval.tv_usec;
	return retVal;
}


bool Timeval::passed() const
{
	Timeval nowTime;
	if (nowTime.mTimeval.tv_sec < mTimeval.tv_sec) return false;
	if (nowTime.mTimeval.tv_sec > mTimeval.tv_sec) return true;
	if (nowTime.mTimeval.tv_usec > mTimeval.tv_usec) return true;
	return false;
}

double Timeval::seconds() const
{
	return ((double)mTimeval.tv_sec) + 1e-6*((double)mTimeval.tv_usec);
}



long Timeval::delta(const Timeval& other) const
{
	// 2^31 milliseconds is just over 4 years.
	long deltaS = other.sec() - sec();
	long deltaUs = other.usec() - usec();
	return 1000*deltaS + deltaUs/1000;
}
	



ostream& operator<<(ostream& os, const Timeval& tv)
{
	os.setf( ios::fixed, ios::floatfield );
	os << tv.seconds();
	return os;
}


ostream& operator<<(ostream& os, const struct timespec& ts)
{
	os << ts.tv_sec << "," << ts.tv_nsec;
	return os;
}



// vim: ts=4 sw=4
