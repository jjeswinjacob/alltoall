// Main Functionality - prints out debugging messages into dbg.log (format is node address [globaltime] message)
// Logs node addition and removal from network into log files

#include "Log.h"

Log::Log(Params *p) {
	par = p;
	firstTime = false;
}

// If another log object is passed, we can use it to initialize this new log
Log::Log(const Log &anotherLog) {
	this->par = anotherLog.par;
	this->firstTime = anotherLog.firstTime;
}

// Returns a reference to a log object
Log& Log::operator = (const Log& anotherLog) {
	this->par = anotherLog.par;
	this->firstTime = anotherLog.firstTime;
	return *this;
}

Log::~Log() {}

// Print out to file dbg.log, along with Address of node.
// Address is a part of Member.h and it contains the address of a single node
// These are not essentialy pointers, but values are passed by reference
void Log::LOG(Address *addr, const char * str, ...) {

	// static makes sure only one copy of variable is present over all functions
	// FILE can be used to read or weite tot files
	static FILE *fp;
	static FILE *fp2;
	va_list vararglist;
	static char buffer[30000];
	static int numwrites;

	// stdstrings contain file pointer names
	static char stdstring[30];
	static char stdstring2[40];
	static char stdstring3[40]; 
	static int dbg_opened=0;

	// Guess if it has been opened, set the variable to hold 639, else it is 0
	if(dbg_opened != 639){
		numwrites=0;
		// ??
		stdstring2[0]=0;

		// Copy String 2 to string 3. Concatenate debug log to string 2.
		// Concatenate stats log to string 3
		// This ensures whatever is in stats log and debug log already is maintained
		// Copy debug log to stats log
		strcpy(stdstring3, stdstring2);
		strcat(stdstring2, DBG_LOG);
		strcat(stdstring3, STATS_LOG);

		// These contain the filename
		// Usually you'll put up in debug log. But if you mention specifically you print to stats log
		fp = fopen(stdstring2, "w");
		fp2 = fopen(stdstring3, "w");

		// ??
		dbg_opened=639;
	}
	else 
		sprintf(stdstring, "%d.%d.%d.%d:%d ", addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], *(short *)&addr->addr[4]);
		// We're printing the id and port of address into a c-style string in printf like format
		// *(short *)&addr->addr[4] helps us get the entire short value of port from character address

	// vararglist of type va_list
	// Initializing vararglist to store all values after str
	va_start(vararglist, str);

	// vsprintf(buffer, format, args)
	vsprintf(buffer, str, vararglist);
	va_end(vararglist);

	// Probably to separate different log instances
	if (!firstTime) {
		int magicNumber = 0;
		string magic = MAGIC_NUMBER;
		int len = magic.length();
		for ( int i = 0; i < len; i++ ) {
			magicNumber += (int)magic.at(i);
		}
		// Print to file; fprintf - Print to File; fscanf - Read from file
		// %x is a hexadecimal output
		fprintf(fp, "%x\n", magicNumber);
		firstTime = true; // Because MAX_WRITES = 1
	}

	// If comparison value is true
	// buffer contains the values of Variable Arguments
	// stdstring contains the address - node id and port number
	if(memcmp(buffer, "#STATSLOG#", 10)==0) {
		fprintf(fp2, "\n %s", stdstring);
		fprintf(fp2, "[%d] ", par->getcurrtime()); // Returns Global time
		fprintf(fp2, buffer); // buffer contains the "#STATSLOG"
	}
	// If Passing #STATSLOG#, print in fp2, else print in fp
	else {
		fprintf(fp, "\n %s", stdstring);
		fprintf(fp, "[%d] ", par->getcurrtime());
		fprintf(fp, buffer);
	}

	if(++numwrites >= MAXWRITES){
		fflush(fp); // file flush - Puts values in streams into files
		fflush(fp2);
		numwrites=0;
	}

}

// To Log a node add. 
// This automatically adds added address into the the dbg.log
void Log::logNodeAdd(Address *thisNode, Address *addedAddr) {
	static char stdstring[100];
	sprintf(stdstring, "Node %d.%d.%d.%d:%d joined at time %d", addedAddr->addr[0], addedAddr->addr[1], addedAddr->addr[2], addedAddr->addr[3], *(short *)&addedAddr->addr[4], par->getcurrtime());
    LOG(thisNode, stdstring);
}

// DESCRIPTION: To log a node remove
void Log::logNodeRemove(Address *thisNode, Address *removedAddr) {
	static char stdstring[30];
	sprintf(stdstring, "Node %d.%d.%d.%d:%d removed at time %d", removedAddr->addr[0], removedAddr->addr[1], removedAddr->addr[2], removedAddr->addr[3], *(short *)&removedAddr->addr[4], par->getcurrtime());
    LOG(thisNode, stdstring);
}

// This does no adding or removing of nodes, but logs whatever is happening
// Is ir like each node contains a debug log and a stats log, becuase we're passing *thisNode?