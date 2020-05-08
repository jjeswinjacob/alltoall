// Contains Details of each member in class Member - Address, Membership Table, Failure Detection Queue
// Contains Address of Nodes

#ifndef MEMBER_H_
#define MEMBER_H_

#include "stdincludes.h"

//Entry in the queue
class q_elt {
	public:
		void *elt;
		int size;
		q_elt(void *elt, int size);
};

// Class representing the address of a single node
class Address {
public:
	char addr[6];

	Address() {}
	Address(const Address &anotherAddress);
	Address& operator =(const Address &anotherAddress);
	bool operator ==(const Address &anotherAddress);

	// Initializing address into id and port into addr string
	Address(string address) {
		size_t pos = address.find(":");
		int id = stoi(address.substr(0, pos));
		short port = (short)stoi(address.substr(pos + 1, address.size()-pos-1));

		// id is 4 bytes and port is 2 bytes
		// We're copying the id value to addr[0] to addr[3]
		memcpy(&addr[0], &id, sizeof(int));
		memcpy(&addr[4], &port, sizeof(short));
	}

	string getAddress() {
		int id = 0;
		short port;
		memcpy(&id, &addr[0], sizeof(int));
		memcpy(&port, &addr[4], sizeof(short));
		return to_string(id) + ":" + to_string(port); // defined in string header
	}
	void init() {
		// sets bytes of addr to 0's
		memset(&addr, 0, sizeof(addr));
	}
};

// Remembers node id, Heartbeat, Local time in gossip protocol tables
class MemberListEntry {
	public:
		int id;
		short port;
		long heartbeat;
		long timestamp;
		MemberListEntry(int id, short port, long heartbeat, long timestamp);
		MemberListEntry(int id, short port);
		MemberListEntry(): id(0), port(0), heartbeat(0), timestamp(0) {}
		MemberListEntry(const MemberListEntry &anotherMLE);
		MemberListEntry& operator =(const MemberListEntry &anotherMLE);
		int getid();
		short getport();
		long getheartbeat();
		long gettimestamp();
		void setid(int id);
		void setport(short port);
		void setheartbeat(long hearbeat);
		void settimestamp(long timestamp);
};

// Class representing a member in the distributed system
class Member {
	public:
		// This member's Address
		Address addr;
		// boolean indicating if this member is up
		bool inited;
		// boolean indicating if this member is in the group
		bool inGroup;
		// boolean indicating if this member has failed
		bool bFailed;
		// number of my neighbors
		int nnb;
		// the node's own heartbeat
		long heartbeat;
		// counter for next ping
		int pingCounter;
		// counter for ping timeout
		int timeOutCounter;
		// Membership table // Each member has a separate member list entry
		vector<MemberListEntry> memberList;
		// My position in the membership table // Iterator on member list entries
		vector<MemberListEntry>::iterator myPos;
		// Queue for failure detection messages
		queue<q_elt> mp1q;

		Member(): inited(false), inGroup(false), bFailed(false), nnb(0), heartbeat(0), pingCounter(0), timeOutCounter(0) {}
		Member(const Member &anotherMember);
		Member& operator =(const Member &anotherMember);
		virtual ~Member() {}
};

#endif /* MEMBER_H_ */