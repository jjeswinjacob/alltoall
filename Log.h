#ifndef _LOG_H_
#define _LOG_H_

#include "stdincludes.h"
#include "Params.h"
#include "Member.h"

// number of writes after which to flush file
#define MAXWRITES 1
// MAGIC_NUMBER ??
#define MAGIC_NUMBER "CS425"
#define DBG_LOG "dbg.log"
#define STATS_LOG "stats.log"

// Functions to log messages in a debug log
class Log{
	private:
		// Contains parameters from conf file
		Params *par;
		bool firstTime;
	public:
		// Passing a pointer to Log 
		Log(Params *p);
		Log(const Log &anotherLog);
		Log& operator = (const Log &anotherLog);
		virtual ~Log();
		void LOG(Address *, const char * str, ...);
		void logNodeAdd(Address *, Address *);
		void logNodeRemove(Address *, Address *);
};

#endif