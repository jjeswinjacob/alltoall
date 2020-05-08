#include "Application.h"

void handler(int sig) {

	// Array of void pointers which could be typecasted into any type
	void *array[10];
	// typedef for unsigned long long
	size_t size;

	// get void*'s for all funcrions on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);

	// STDERR_FILENO defined in unistd.h to point to STDERR TO WRITE TO IT
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

int main(int argc, char *argv[]) {

	// passing information to main if program is run from the command line 
	// argc represents the number of values passed to main 
	// and argv is the string array that contains the values passed.

	//signal(SIGSEGV, handler); Part of signal.h
	// Accesing illegal location raises a signal handled by handler

	// .conf could be singlefailure, multifailure, msgdropsinglefailure
	if ( argc != ARGS_COUNT ) {
		cout<<"Configuration (i.e., *.conf) file File Required"<<endl;
		return FAILURE;
	}

	// new creates object on heap
	// Create a new application object on the heap - Dynamic Memory Allocation
	Application *app = new Application(argv[1]);
	// Call the run function
	app->run();
	// When done delete the application object
	delete(app);
	return SUCCESS;
}

// .conf filename is passed as string
Application::Application(char *infile) {
	int i;
	par = new Params();
	//makes use of the computer's internal clock to control the choice of the seed. time.h, stdlib.h
	srand (time(NULL));
	// Set Parameters from configuration file in par object
	par->setparams(infile);
	// Sending parameters of par to log and emulnet objects
	log = new Log(par); 
	en = new EmulNet(par); // Initialize Emulnet
	mp1 = (MP1Node **) malloc(par->EN_GPSZ * sizeof(MP1Node *)); 
	// actual number of peers = par -> GPSZ
	// Returns Pointer to MP1Node *'s

	// Init all nodes - Create Member, Create Address
	for( i = 0; i < par->EN_GPSZ; i++ ) {
		Member *memberNode = new Member; // Contains details of Member Node
		memberNode->inited = false;
		Address *addressOfMemberNode = new Address();
		Address joinaddr;
		joinaddr = getjoinaddr(); // Returns Address of Coordinator
		addressOfMemberNode = (Address *) en->ENinit(addressOfMemberNode, par->PORTNUM);
		mp1[i] = new MP1Node(memberNode, par, en, log, addressOfMemberNode);
		log->LOG(&(mp1[i]->getMemberNode()->addr), "APP"); // Print APP in MP1[i] alomng with Address
		delete addressOfMemberNode;
	}
}

//Destructor
Application::~Application() {
	delete log;
	delete en;
	for ( int i = 0; i < par->EN_GPSZ; i++ ) {
		delete mp1[i];
	}
	free(mp1);
	delete par;
}

// Main driver function of the Application layer
int Application::run()
{
	int i;
	int timeWhenAllNodesHaveJoined = 0;
	// boolean indicating if all nodes have joined
	bool allNodesJoined = false;
	srand(time(NULL));

	// As time runs along
	for( par->globaltime = 0; par->globaltime < TOTAL_RUNNING_TIME; ++par->globaltime ) {
		// Run the membership protocol
		mp1Run();
		// Fail some nodes
		fail();
	}

	// Clean up
	en -> ENcleanup();

	for(i=0;i<=par->EN_GPSZ-1;i++) {
		 mp1[i]->finishUpThisNode();
	}

	return SUCCESS;
}

// This function performs all the membership protocol functionalities
void Application::mp1Run() {
	int i;

	// At every future point (After Node Joins) in time, if node is not failed, we Chekc for Received MEssages amd put into quuee
	// For all the nodes in the system
	for( i = 0; i <= par->EN_GPSZ-1; i++) {
		// Receive messages from the network and queue them in the membership protocol queue
		if( par->getcurrtime() > (int)(par->STEP_RATE*i) && !(mp1[i]->getMemberNode()->bFailed) ) {
			// Receive messages from the network and queue them
			mp1[i]->recvLoop(); // Whatever is in Emulnet Buffer, Place in my own Queue
		}
		// par->getcurrtime() = return globaltime;
		// STEP_RATE dictates the rate of insertion
		// Before Insertion of Node, We don't entertain messages
	}

	// For all the nodes in the system
	// Every node is allowed to join at only one moment of time. Not Rejoin.
	for( i = par->EN_GPSZ - 1; i >= 0; i-- ) {

		// Introduce nodes into the distributed system
		if( par->getcurrtime() == (int)(par->STEP_RATE*i) ) {
			// introduce the ith node into the system at time STEPRATE*i
			mp1[i]->nodeStart(JOINADDR, par->PORTNUM); // Init this Node and Introduce Yourself to Group
			cout<<i<<"-th introduced node is assigned with the address: "<<mp1[i]->getMemberNode()->addr.getAddress() << endl;
			nodeCount += i;
		}

		/*
		 * Handle all the messages in your queue and send heartbeats
		 */
		// At all times to the future of node addition, we perform Checking Messages/ InGroup/ NodeLoopOps
		else if( par->getcurrtime() > (int)(par->STEP_RATE*i) && !(mp1[i]->getMemberNode()->bFailed) ) {
			// handle messages and send heartbeats
			mp1[i]->nodeLoop();
			#ifdef DEBUGLOG
			if( (i == 0) && (par->globaltime % 500 == 0) ) {
				log->LOG(&mp1[i]->getMemberNode()->addr, "@@time=%d", par->getcurrtime());
			}
			#endif
		}
	}
}

// This function controls the failure of nodes
 // Note: this is used only by MP1
void Application::fail() {
	int i, removed;

	// fail half the members at time t=400
	if( par->DROP_MSG && par->getcurrtime() == 50 ) {
		par->dropmsg = 1;
	}

	if( par->SINGLE_FAILURE && par->getcurrtime() == 100 ) {
		removed = (rand() % par->EN_GPSZ);
		#ifdef DEBUGLOG
		log->LOG(&mp1[removed]->getMemberNode()->addr, "Node failed at time=%d", par->getcurrtime());
		#endif
		mp1[removed]->getMemberNode()->bFailed = true;
	}
	else if( par->getcurrtime() == 100 ) {
		removed = rand() % par->EN_GPSZ/2;
		for ( i = removed; i < removed + par->EN_GPSZ/2; i++ ) {
			#ifdef DEBUGLOG
			log->LOG(&mp1[i]->getMemberNode()->addr, "Node failed at time = %d", par->getcurrtime());
			#endif
			mp1[i]->getMemberNode()->bFailed = true;
		}
	}

	if( par->DROP_MSG && par->getcurrtime() == 300) {
		par->dropmsg=0;
	}

}

/**
 * FUNCTION NAME: getjoinaddr
 *
 * DESCRIPTION: This function returns the address of the coordinator
 */
Address Application::getjoinaddr(void){
	//trace.funcEntry("Application::getjoinaddr");
    Address joinaddr;
    joinaddr.init();
    *(int *)(&(joinaddr.addr))=1;
    *(short *)(&(joinaddr.addr[4]))=0;
    //trace.funcExit("Application::getjoinaddr", SUCCESS);
    return joinaddr;
}