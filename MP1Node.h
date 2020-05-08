#ifndef _MP1NODE_H_
#define _MP1NODE_H_

#include "stdincludes.h"
#include "Log.h"
#include "Params.h"
#include "Member.h"
#include "EmulNet.h"
#include "Queue.h"

#define TREMOVE 20
#define TFAIL 5

// Values which can be taken by MsgTypes
enum MsgTypes{
    JOINREQ,
    JOINREP,
    UPDATEMEMBERLIST
};

// Header and content of a message
typedef struct MessageHdr {
	enum MsgTypes msgType;
}MessageHdr;

//Class implementing Membership protocol functionalities for failure detection
class MP1Node {
	private:
		EmulNet *emulNet;
		Log *log;
		Params *par;
		Member *memberNode;
		char NULLADDR[6];

		void updateMemberList(char *data);
		void UpdateEntry(Address addr, long heartbeat);
		void sendMemberList(Address toAddress, MsgTypes msgtype);
		Address Ret_address(int ip, short port);
		
	public:
		MP1Node(Member *, Params *, EmulNet *, Log *, Address *);
		Member * getMemberNode() {
			return memberNode;
		}
		int recvLoop();
		static int enqueueWrapper(void *env, char *buff, int size);
		void nodeStart(char *servaddrstr, short serverport);
		int initThisNode(Address *joinaddr);
		int introduceSelfToGroup(Address *joinAddress);
		int finishUpThisNode();
		void nodeLoop();
		void checkMessages();
		bool recvCallBack(void *env, char *data, int size);
		void nodeLoopOps();
		int isNullAddress(Address *addr);
		Address getJoinAddress();
		void initMemberListTable(Member *memberNode);
		void printAddress(Address *addr);
		virtual ~MP1Node();
};
#endif