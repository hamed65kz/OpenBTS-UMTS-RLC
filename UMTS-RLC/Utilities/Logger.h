/*
 * OpenBTS provides an open source alternative to legacy telco protocols and 
 * traditionally complex, proprietary hardware systems.
 *
 * Copyright 2009, 2010 Free Software Foundation, Inc.
 * Copyright 2010 Kestrel Signal Processing, Inc.
 * Copyright 2011-2014 Range Networks, Inc.
 *
 * This software is distributed under the terms of the GNU Affero General 
 * Public License version 3. See the COPYING and NOTICE files in the main 
 * directory for licensing information.
 *
 * This use of this software may be subject to additional restrictions.
 * See the LEGAL file in the main directory for details.
 */

// (pat) WARNING is stupidly defined in /usr/local/include/osipparser2/osip_const.h.
// This must be outside the #ifndef LOGGER_H to fix it as long as Logger.h included after the above file.
#ifdef WARNING
#undef WARNING
#endif

#ifndef LOGGER_H
#define LOGGER_H


//
#include <stdint.h>
#include <stdio.h>
#include <sstream>
#include <list>
#include <map>
#include <string>


#if _WIN32
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */

#define	LOG_PRIMASK	0x07	/* mask to extract priority part (internal) */
                /* extract priority */
#define	LOG_PRI(p)	((p) & LOG_PRIMASK)

/* facility codes */
#define	LOG_KERN	(0<<3)	/* kernel messages */
#define	LOG_USER	(1<<3)	/* random user-level messages */
#define	LOG_MAIL	(2<<3)	/* mail system */
#define	LOG_DAEMON	(3<<3)	/* system daemons */
#define	LOG_AUTH	(4<<3)	/* security/authorization messages */
#define	LOG_SYSLOG	(5<<3)	/* messages generated internally by syslogd */
#define	LOG_LPR		(6<<3)	/* line printer subsystem */
#define	LOG_NEWS	(7<<3)	/* network news subsystem */
#define	LOG_UUCP	(8<<3)	/* UUCP subsystem */
#define	LOG_CRON	(9<<3)	/* clock daemon */
#define	LOG_AUTHPRIV	(10<<3)	/* security/authorization messages (private) */
#define	LOG_FTP		(11<<3)	/* ftp daemon */

    /* other codes through 15 reserved for system use */
#define	LOG_LOCAL0	(16<<3)	/* reserved for local use */
#define	LOG_LOCAL1	(17<<3)	/* reserved for local use */
#define	LOG_LOCAL2	(18<<3)	/* reserved for local use */
#define	LOG_LOCAL3	(19<<3)	/* reserved for local use */
#define	LOG_LOCAL4	(20<<3)	/* reserved for local use */
#define	LOG_LOCAL5	(21<<3)	/* reserved for local use */
#define	LOG_LOCAL6	(22<<3)	/* reserved for local use */
#define	LOG_LOCAL7	(23<<3)	/* reserved for local use */


#else
    #include <syslog.h>
#endif



#define _LOG(level) \
	Log(LOG_##level).get()<< timestr() << " "/* __FILE__  ":"  << __LINE__ << ":"*/ << __FUNCTION__ << ": "

//#define IS_LOG_LEVEL(wLevel) (gGetLoggingLevel(__FILE__)>=LOG_##wLevel)
#define IS_LOG_LEVEL(wLevel) true

#ifdef NDEBUG
#define LOG(wLevel) \
	if (LOG_##wLevel!=LOG_DEBUG && IS_LOG_LEVEL(wLevel)) _LOG(wLevel)
#else
#define LOG(wLevel) \
	if (IS_LOG_LEVEL(wLevel)) _LOG(wLevel)
#endif

// pat: And for your edification here are the 'levels' as defined in syslog.h:
// LOG_EMERG   0  system is unusable
// LOG_ALERT   1  action must be taken immediately
// LOG_CRIT    2  critical conditions
// LOG_ERR     3  error conditions
// LOG_WARNING 4  warning conditions
// LOG_NOTICE  5  normal, but significant, condition
// LOG_INFO    6  informational message
// LOG_DEBUG   7  debug-level message

// (pat) added - print out a var and its name.
// Use like this: int descriptive_name; LOG(INFO)<<LOGVAR(descriptive_name);
#define LOGVAR2(name,val) " " << name << "=" << (val)
#define LOGVAR(var) (" " #var "=") << var
#define LOGVARM(var) " " << &#var[1] << "=" << var		// Strip the first char ("m") off the var name when printing.
#define LOGVARP(var) (" " #var "=(") << var <<")"	// Put value in parens; used for classes.
#define LOGHEX(var) (" " #var "=0x") << hex << ((unsigned)var) << dec
#define LOGHEX2(name,val) " " << name << "=0x" << hex << ((unsigned)(val)) << dec
// These are kind of cheesy, but you can use for bitvector
#define LOGBV2(name,val) " " << name << "=(" << val<<" size:"<<val.size()<<")"
#define LOGBV(bv) LOGBV2(#bv,bv)
#define LOGVARRANGE(name,cur,lo,hi) " "<<name <<"=("<<(cur) << " range:"<<(lo) << " to "<<(hi) <<")"

#define LOGWATCH(stuff) if (gConfig.getNum("UMTS.Debug.Messages")) \
		std::cout << Utils::timestr().substr(4)<<" "<<stuff<<"\n";


#define OBJLOG(wLevel) \
	LOG(wLevel) << "obj: " << this << ' '

#define LOG_ASSERT(x) { if (!(x)) LOG(EMERG) << "assertion " #x " failed"; } assert(x);


#include "Threads.h"		// must be after defines above, if these files are to be allowed to use LOG()
#include "Utils.h"

/**
	A C++ stream-based thread-safe logger.
	Derived from Dr. Dobb's Sept. 2007 issue.
	Updated to use syslog.
	This object is NOT the global logger;
	every log record is an object of this class.
*/


//#define RLCLOG(stuff...) PATLOG(4, "RLC " << rlcid() <<":" << format(stuff) << mUep);

#ifdef _WIN32
#define RLCLOG(stuff,...) LOG(DEBUG) << "RLC " << rlcid() <<":" << format(stuff) << mUep;
#define RLCERR(stuff,...) LOG(ERR) << "RLC "<< rlcid() <<":" << format(stuff) << mUep;
#else
#define RLCLOG(stuff...) LOG(DEBUG) << "RLC " << rlcid() <<":" << format(stuff) << mUep;
#define RLCERR(stuff...) LOG(ERR) << "RLC "<< rlcid() <<":" << format(stuff) << mUep;
#endif
//int rrcDebugLevel = 0xffff;
//#define PATLOG(level,msg) if (level & rrcDebugLevel) std::cout << msg <<"\n";
#define PATLOG(level,msg) if (level) LOG(INFO) << msg;

class Log {

	public:

	protected:

	std::ostringstream mStream;		///< This is where we buffer up the log entry.
	int mPriority;					///< Priority of current report.
	bool mDummyInit;

	public:

	Log(int wPriority)
		:mPriority(wPriority), mDummyInit(false)
	{ }

	Log(const char* name, const char* level=NULL, int facility=LOG_USER);

	// Most of the work is in the destructor.
	/** The destructor actually generates the log entry. */
	~Log();

	std::ostringstream& get();
};
extern bool gLogToConsole;	// Pat added for easy debugging.



//std::list<std::string> gGetLoggerAlarms();		///< Get a copy of the recent alarm list.


/**@ Global control and initialization of the logging system. */
//@{
/** Initialize the global logging system. */
void gLogInit(const char* name, const char* level=NULL, int facility=LOG_USER);
/** Get the logging level associated with a given file. */
int gGetLoggingLevel(const char *filename=NULL);
/** Allow early logging when still in constructors */
void gLogEarly(int level, const char *fmt, ...) 
#ifdef __GNUC__
__attribute__((format(printf, 2, 3)))
#endif 
;
//@}


#endif

// vim: ts=4 sw=4
