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


#include <string.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <stdarg.h>

//#include "Configuration.h"
#include "Logger.h"
#include "Threads.h"	// pat added
using namespace std;


string Log_File ="umtslog.txt"; //'Path to use for textfile based logging.  By default, this feature is disabled.  To enable, specify an absolute path to the file you wish to use, eg: /tmp/my-debug.log.  To disable again, execute "unconfig Log.File".
const char* Log_Level = "NOTICE";
char gCmdName[20] = { "Open" }; // Use a char* to avoid avoid static initialization of string, and race at startup.
int Log_Alarms_Max = 20 ;

// Reference to a global config table, used all over the system.
//extern ConfigurationTable gConfig;


/**@ The global alarms table. */
//@{
Mutex           alarmsLock;
list<string>    alarmsList;
void            addAlarm(const string&);
//@}



// (pat) If Log messages are printed before the classes in this module are inited
// (which happens when static classes have constructors that do work)
// the OpenBTS just crashes.
// Prevent that by setting sLoggerInited to true when this module is inited.
static bool sLoggerInited = 0;
static struct CheckLoggerInitStatus {
	CheckLoggerInitStatus() { sLoggerInited = 1; }
} sCheckloggerInitStatus;



/** Names of the logging levels. */
const char *levelNames[] = {
	"EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG"
};
int numLevels = 8;
bool gLogToConsole = 1;
FILE *gLogToFile = NULL;
Mutex gLogToLock;


int levelStringToInt(const string& name)
{
	// Reverse search, since the numerically larger levels are more common.
	for (int i=numLevels-1; i>=0; i--) {
		if (name == levelNames[i]) return i;
	}

	// Common substitutions.
	if (name=="INFORMATION") return 6;
	if (name=="WARN") return 4;
	if (name=="ERROR") return 3;
	if (name=="CRITICAL") return 2;
	if (name=="EMERGENCY") return 0;

	// Unknown level.
	return -1;
}

/** Given a string, return the corresponding level name. */
int lookupLevel(const string& key)
{
    string val = Log_Level;
	int level = levelStringToInt(val);

	if (level == -1) {
        string defaultLevel = "NOTICE";
		level = levelStringToInt(defaultLevel);
		_LOG(CRIT) << "undefined logging level (" << key << " = \"" << val << "\") defaulting to \"" << defaultLevel << ".\" Valid levels are: EMERG, ALERT, CRIT, ERR, WARNING, NOTICE, INFO or DEBUG";
        Log_Level = defaultLevel.c_str();
	}

	return level;
}


int getLoggingLevel(const char* filename)
{
	// Default level?
	if (!filename) return lookupLevel("Log.Level");

	// This can afford to be inefficient since it is not called that often.
	const string keyName = string("Log.Level.") + string(filename);
    //if (gConfig.defines(keyName)) return lookupLevel(keyName);
	return lookupLevel("Log.Level");
}



int gGetLoggingLevel(const char* filename)
{
	// This is called a lot and needs to be efficient.
    int level = getLoggingLevel(filename);
    return level;
//	static Mutex sLogCacheLock;
//	static map<uint64_t,int>  sLogCache;
//	static unsigned sCacheCount;
//	static const unsigned sCacheRefreshCount = 1000;

//	if (filename==NULL) return gGetLoggingLevel("");

//	HashString hs(filename);
//	uint64_t key = hs.hash();

//	sLogCacheLock.lock();
//	// Time for a cache flush?
//	if (sCacheCount>sCacheRefreshCount) {
//		sLogCache.clear();
//		sCacheCount=0;
//	}
//	// Is it cached already?
//	map<uint64_t,int>::const_iterator where = sLogCache.find(key);
//	sCacheCount++;
//	if (where!=sLogCache.end()) {
//		int retVal = where->second;
//		sLogCacheLock.unlock();
//		return retVal;
//	}
//	// Look it up in the config table and cache it.
//	// FIXME: Figure out why unlock and lock below fix the config table deadlock.
//	// (pat) Probably because getLoggingLevel may call LOG recursively via lookupLevel().
//	sLogCacheLock.unlock();
//	int level = getLoggingLevel(filename);
//	sLogCacheLock.lock();
//	sLogCache.insert(pair<uint64_t,int>(key,level));
//	sLogCacheLock.unlock();
//	return level;
}





// // copies the alarm list and returns it. list supposed to be small.
// list<string> gGetLoggerAlarms()
// {
//     alarmsLock.lock();
//     list<string> ret;
//     // excuse the "complexity", but to use std::copy with a list you need
//     // an insert_iterator - copy technically overwrites, doesn't insert.
//     insert_iterator< list<string> > ii(ret, ret.begin());
//     copy(alarmsList.begin(), alarmsList.end(), ii);
//     alarmsLock.unlock();
//     return ret;
// }

/** Add an alarm to the alarm list. */
void addAlarm(const string& s)
{
    alarmsLock.lock();
    alarmsList.push_back(s);
    unsigned maxAlarms = Log_Alarms_Max;
    while (alarmsList.size() > maxAlarms) alarmsList.pop_front();
    alarmsLock.unlock();
}


Log::~Log()
{
#if _WIN32
//#else

	if (mDummyInit) return;
	// Anything at or above LOG_CRIT is an "alarm".
	// Save alarms in the local list and echo them to stderr.
	if (mPriority <= LOG_CRIT) {
		if (sLoggerInited) addAlarm(mStream.str().c_str());
		cerr << mStream.str() << endl;
	}
	// Current logging level was already checked by the macro.
	// So just log.
	//syslog(mPriority, "%s", mStream.str().c_str());
	// pat added for easy debugging.
	if (gLogToConsole||gLogToFile) {
		int mlen = mStream.str().size();
		int neednl = (mlen==0 || mStream.str()[mlen-1] != '\n');
		gLogToLock.lock();
		if (gLogToConsole) {
			// The COUT() macro prevents messages from stomping each other but adds uninteresting thread numbers,
			// so just use std::cout.
			auto ss = mStream.str();
			std::cout << mStream.str();
			if (neednl) std::cout<<"\n";
		}
		if (gLogToFile) {
			fputs(mStream.str().c_str(),gLogToFile);
			if (neednl) {fputc('\n',gLogToFile);}
			fflush(gLogToFile);
		}
		gLogToLock.unlock();
	}
#endif
}


Log::Log(const char* name, const char* level, int facility)
{
	mDummyInit = true;
	gLogInit(name, level, facility);
}


ostringstream& Log::get()
{
	assert(mPriority<numLevels);	
	mStream << levelNames[mPriority] <<  ' ';
	return mStream;
}



void gLogInit(const char* name, const char* level, int facility)
{
    #if _WIN32
//#else

	// Set the level if one has been specified.

	if (level) {
        //gConfig.set("Log.Level",level);
        Log_Level = level;
	}

	// Pat added, tired of the syslog facility.
	// Both the transceiver and OpenBTS use this same facility, but only OpenBTS/OpenBTS-UMTS may use this log file:
    string str = Log_File;
	if (gLogToFile==0 && str.length() && 0==strncmp(gCmdName,"Open",4)) {
		const char *fn = str.c_str();
		if (fn && *fn && strlen(fn)>3) {	// strlen because a garbage char is getting in sometimes.
			gLogToFile = fopen(fn,"w"); // New log file each time we start.
			if (gLogToFile) {
				time_t now;
				time(&now);
				fprintf(gLogToFile,"Starting at %s",ctime(&now));
				fflush(gLogToFile);
				std::cout << "Logging to file: " << fn << "\n";
			}
		}
	}

	// Open the log connection.
	//openlog(name,0,facility);
    #endif
}


void gLogEarly(int level, const char *fmt, ...)
{
#if _WIN32
#else

	va_list args;
 
	va_start(args, fmt);
	vsyslog(level | LOG_USER, fmt, args);
    va_end(args);
#endif
}

// vim: ts=4 sw=4
