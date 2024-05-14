/*
 * OpenBTS provides an open source alternative to legacy telco protocols and 
 * traditionally complex, proprietary hardware systems.
 *
 * Copyright 2011-2014 Range Networks, Inc.
 *
 * This software is distributed under the terms of the GNU Affero General 
 * Public License version 3. See the COPYING and NOTICE files in the main 
 * directory for licensing information.
 *
 * This use of this software may be subject to additional restrictions.
 * See the LEGAL file in the main directory for details.
 */

//#include <unistd.h>		// For usleep
//#include <sys/time.h>	// For gettimeofday
#include <stdio.h>		// For vsnprintf
#include <ostream>		// For ostream
#include <sstream>		// For ostringstream
#include <string.h>		// For strcpy
//#include "GSMCommon.h"
#include "Utils.h"
#include "Timeval.h"
//#include <time.h>
#include <stdio.h>
#include "Logger.h"


int gMemLeakDebug = 0;

std::ostream& operator<<(std::ostream& os, std::ostringstream& ss)
{
	return os << ss.str();
}

std::ostream &osprintf(std::ostream &os, const char *fmt, ...)
{
	//return os;
	va_list ap;
	char buf[300];
	va_start(ap,fmt);
	int n = vsnprintf(buf,300,fmt,ap);
	va_end(ap);
	if (n >= (300-4)) { strcpy(&buf[(300-4)],"..."); }
	os << buf;
	return os;
}

std::string format(const char *fmt, ...)
{
	//return std::string(fmt);
	va_list ap;
	char buf[300] = {0};
	//int bb = strlen(fmt);
	//int aa = _INTSIZEOF(fmt);
	//ap = (va_list)_ADDRESSOF(fmt) + _INTSIZEOF(fmt);

	va_start(ap,fmt);
	

	//ap = (va_list)_ADDRESSOF(fmt) + _INTSIZEOF(fmt);

	int n =  vsprintf(buf,/*, 300,*/ fmt, ap);
	va_end(ap);
	if (n >= (300-4)) { strcpy(&buf[(300-4)],"..."); }
	return std::string(buf);
}

const std::string timestr()
{ 
	struct timeval_t tv;
	struct tm* tm ;
	gettimeofday(&tv, NULL);
	auto t =time(NULL);//hkz add it
	tm = localtime((time_t*)&t);
	unsigned tenths = tv.tv_usec / 100000;	// Rounding down is ok.
	return format(" %02d:%02d:%02d.%1d", tm->tm_hour, tm->tm_min, tm->tm_sec, tenths);
}

std::string Text2Str::str() const
{
	std::ostringstream ss;
	text(ss);
	return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Text2Str *val)
{
	std::ostringstream ss;
	if (val) {
		val->text(ss);
		os << ss.str(); 
	} else {
		os << "(null)";
	}
	return os;
}

// Greatest Common Denominator.
// This is by Doug Brown.
int gcd(int x, int y)
{
	if (x > y) {
		return x % y == 0 ? y : gcd(y, x % y);
	} else {
		return y % x == 0 ? x : gcd(x, y % x);
	}
}


// Split a C string into an argc,argv array in place; the input string is modified.
// Returns argc, and places results in argv, up to maxargc elements.
// The final argv receives the rest of the input string from maxargc on,
// even if it contains additional splitchars.
// The correct idiom for use is to make a copy of your string, like this:
// char *copy = strcpy((char*)alloca(the_string.length()+1),the_string.c_str());
// char *argv[2];
// int argc = cstrSplit(copy,argv,2,NULL);
// If you want to detect the error of too many arguments, add 1 to argv, like this:
// char *argv[3];
// int argc = cstrSplit(copy,argv,3,NULL);
// if (argc == 3) { error("too many arguments"; }
int cstrSplit(char *in, char **pargv,int maxargc, const char *splitchars)
{
	if (splitchars == NULL) { splitchars = " \t\r\n"; }	// Default is any space.
	int argc = 0;
	while (argc < maxargc) {
		while (*in && strchr(splitchars,*in)) {in++;}	// scan past any splitchars
		if (! *in) return argc;					// return if finished.
		pargv[argc++] = in;						// save ptr to start of arg.
		in = strpbrk(in,splitchars);			// go to end of arg.
		if (!in) return	argc;					// return if finished.
		*in++ = 0;								// zero terminate this arg.
	}
	return argc;
}

std::ostream& operator<<(std::ostream& os, const Statistic<int> &stat) { stat.text(os); return os; }
std::ostream& operator<<(std::ostream& os, const Statistic<unsigned> &stat) { stat.text(os); return os; }
std::ostream& operator<<(std::ostream& os, const Statistic<float> &stat) { stat.text(os); return os; }
std::ostream& operator<<(std::ostream& os, const Statistic<double> &stat) { stat.text(os); return os; }

std::string replaceAll(const std::string input, const std::string search, const std::string replace)
{
	std::string output = input;
 	int index = 0;

	while (true) {
		index = output.find(search, index);
		if (index == std::string::npos) {
			break;
		}

		output.replace(index, replace.length(), replace);
		index += replace.length();
	}

	return output;
}

void stringToUint(std::string strRAND, uint64_t *hRAND, uint64_t *lRAND)
{
	assert(strRAND.size() == 32);
	std::string strhRAND = strRAND.substr(0, 16);
	std::string strlRAND = strRAND.substr(16, 16);
	std::stringstream ssh;
	ssh << std::hex << strhRAND;
	ssh >> *hRAND;
	std::stringstream ssl;
	ssl << std::hex << strlRAND;
	ssl >> *lRAND;
}

std::string uintToString(uint64_t h, uint64_t l)
{
	std::ostringstream os1;
	os1.width(16);
	os1.fill('0');
	os1 << std::hex << h;
	std::ostringstream os2;
	os2.width(16);
	os2.fill('0');
	os2 << std::hex << l;
	std::ostringstream os3;
	os3 << os1.str() << os2.str();
	return os3.str();
}

std::string uintToString(uint32_t x)
{
	std::ostringstream os;
	os.width(8);
	os.fill('0');
	os << std::hex << x;
	return os.str();
}



