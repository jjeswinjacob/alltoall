#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "stdincludes.h"
#include "MP1Node.h"
#include "Log.h"
#include "Params.h"
#include "Member.h"
#include "EmulNet.h"
#include "Queue.h"

int nodeCount = 0;
// ARGS_COUNT ?
#define ARGS_COUNT 2 
#define TOTAL_RUNNING_TIME 700

class Application{
private:
	// Address for introduction to the group Coordinator Node
	char JOINADDR[30];

	// EMULNET ?
	EmulNet *en;
    Log *log;

	// ** ?
	MP1Node **mp1;
	Params *par; // Used in Application.cpp to create instance
public:
	Application(char *);

	// VIRTUAL ~ ?
	virtual ~Application();
	Address getjoinaddr(); // Part of Member
	int run();
	void mp1Run();
	void fail();
};

#endif /* _APPLICATION_H__ */
