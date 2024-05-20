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


#ifndef TIMEVAL_H
#define TIMEVAL_H

#ifdef _WIN32
//#include <time.h>

struct timeval_t
{ 
	/* Nominally a BSD or POSIX.1 structure, (with tv_sec declared as
  * time_t), but subverted by Microsoft with tv_sec declared as long,
  * to avoid __time32_t vs. __time64_t ambiguity; (tv_sec is ALWAYS
  * a 32-bit entity in Windows' use of this structure).  Addionally,
  * POSIX.1-2001 mandates that tv_usec should be suseconds_t, (which
  * is nominally an alias for long), but we retain long to maintain
  * consistency with Microsoft usage.
  */
	long tv_sec;		/* whole number of seconds in interval */
	long tv_usec; 	/* additional fraction as microseconds */
};

#ifndef _TIMEZONE_DEFINED /* also in sys/time.h */
#define _TIMEZONE_DEFINED
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif

int gettimeofday(struct timeval_t* tv, struct timezone* tz);

void usleep(long long  usec);

#else
#include <unistd.h>
#include "sys/time.h"
//typedef timeval timeval_t;
#define timeval_t timeval
#endif

#include <stdint.h>

#include <iostream>




/** A wrapper on usleep to sleep for milliseconds. */
inline void msleep(long v) { 
	long long inp = v *1000; 
	usleep(inp); 
	}


/** A C++ wrapper for struct timeval. */
class Timeval {

	private:

		struct timeval_t mTimeval;

	public:

	/** Set the value to gettimeofday. */
	void now() { gettimeofday(&mTimeval,NULL); }

	/** Set the value to gettimeofday plus an offset. */
	void future(unsigned ms);

	//@{
	Timeval(unsigned sec, unsigned usec)
	{
		mTimeval.tv_sec = sec;
		mTimeval.tv_usec = usec;
	}

	Timeval(const struct timeval_t& wTimeval)
		:mTimeval(wTimeval)
	{}

	/**
		Create a Timeval offset into the future.
		@param offset milliseconds
	*/
	Timeval(unsigned offset=0) { future(offset); }
	//@}

	/** Convert to a struct timespec. */
	struct timespec timespec() const;

	/** Return total seconds. */
	double seconds() const;

	time_t sec() const { return mTimeval.tv_sec; }
	uint32_t usec() const { return mTimeval.tv_usec; }

	/** Return differnce from other (other-self), in ms. */
	long delta(const Timeval& other) const;

	/** Elapsed time in ms. */
	long elapsed() const { return delta(Timeval()); }

	/** Remaining time in ms. */
	long remaining() const { return -elapsed(); }

	/** Return true if the time has passed, as per gettimeofday. */
	bool passed() const;

	/** Add a given number of minutes to the time. */
	void addMinutes(unsigned minutes) { mTimeval.tv_sec += minutes*60; }

};

std::ostream& operator<<(std::ostream& os, const Timeval&);

std::ostream& operator<<(std::ostream& os, const struct timespec&);


#endif
// vim: ts=4 sw=4
