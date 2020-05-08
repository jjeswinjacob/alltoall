// Emulated Network classes header file

#ifndef _EMULNET_H_
#define _EMULNET_H_

#define MAX_NODES 1000
#define MAX_TIME 3600
#define ENBUFFSIZE 30000

#include "stdincludes.h"
#include "Params.h"
#include "Member.h"

using namespace std;

// Struct Name: en_msg
// Emulnet Message
typedef struct en_msg {
	// Number of bytes after the class
	int size;
	// Source node
	Address from;
	// Destination node
	Address to;
} en_msg;

// Initializing Emulator Class with required variables
class EM {
	public:
		int nextid; // Possibly Next Node
		int currbuffsize;
		int firsteltindex;
		en_msg* buff[ENBUFFSIZE]; // Emulator Messages Buffer
		EM() {}

		// Equating two emulator instances
		EM& operator = (EM &anotherEM) {
			this->nextid = anotherEM.getNextId();
			this->currbuffsize = anotherEM.getCurrBuffSize();
			this->firsteltindex = anotherEM.getFirstEltIndex();
			int i = this->currbuffsize;
			while (i > 0) {
				this->buff[i] = anotherEM.buff[i];
				i--;
			}
			return *this;
		}
		int getNextId() {
			return nextid;
		}
		int getCurrBuffSize() {
			return currbuffsize;
		}
		int getFirstEltIndex() {
			return firsteltindex;
		}
		void setNextId(int nextid) {
			this->nextid = nextid;
		}
		void settCurrBuffSize(int currbuffsize) {
			this->currbuffsize = currbuffsize;
		}
		void setFirstEltIndex(int firsteltindex) {
			this->firsteltindex = firsteltindex;
		}
		virtual ~EM() {}
		// We're not populating the emulator messages buffer here
};

// This class defines an emulated network // (THROUGH TABLES OF MESSAGES)
class EmulNet
{ 	
	private:
		Params* par;
		int sent_msgs[MAX_NODES + 1][MAX_TIME];
		int recv_msgs[MAX_NODES + 1][MAX_TIME];
		int enInited;
		EM emulnet;
	public:
		EmulNet(Params *p);
		EmulNet(EmulNet &anotherEmulNet);
		EmulNet& operator = (EmulNet &anotherEmulNet);
		virtual ~EmulNet();
		void *ENinit(Address *myaddr, short port);
		int ENsend(Address *myaddr, Address *toaddr, string data);
		int ENsend(Address *myaddr, Address *toaddr, char *data, int size);
		// Function Declaration of enqueue??
		// enqueues a received message using a function specified through a pointer enqueue().
		int ENrecv(Address *myaddr, int (* enq)(void *, char *, int), struct timeval *t, int times, void *queue);
		int ENcleanup();
};

#endif /* _EMULNET_H_ */
