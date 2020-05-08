#include "Member.h"

// Usually Constructor used to initialize
// Copy Constructor to also initialize
// Operator Overloaded = to make assignment easier.

// Instead of this -> elt = elt
q_elt::q_elt(void *elt, int size): elt(elt), size(size) {}

Address::Address(const Address &anotherAddress) {
	// strcpy(addr, anotherAddress.addr);
	memcpy(&addr, &anotherAddress.addr, sizeof(addr));
}

Address& Address::operator =(const Address& anotherAddress) {
	// strcpy(addr, anotherAddress.addr);
	memcpy(&addr, &anotherAddress.addr, sizeof(addr));
	return *this;
}

/**
 * Compare two Address objects
 * Return true/non-zero if they have the same ip address and port number 
 * Return false/zero if they are different 
 */
bool Address::operator ==(const Address& anotherAddress) {
	// Returns 0 if both blocks are equal
	return !memcmp(this->addr, anotherAddress.addr, sizeof(this->addr));
}

MemberListEntry::MemberListEntry(int id, short port, long heartbeat, long timestamp): id(id), port(port), heartbeat(heartbeat), timestamp(timestamp) {}
MemberListEntry::MemberListEntry(int id, short port): id(id), port(port) {}
MemberListEntry::MemberListEntry(const MemberListEntry &anotherMLE) {
	this->heartbeat = anotherMLE.heartbeat;
	this->id = anotherMLE.id;
	this->port = anotherMLE.port;
	this->timestamp = anotherMLE.timestamp;
}

MemberListEntry& MemberListEntry::operator =(const MemberListEntry &anotherMLE) {
	MemberListEntry temp(anotherMLE);
	swap(heartbeat, temp.heartbeat);
	swap(id, temp.id);
	swap(port, temp.port);
	swap(timestamp, temp.timestamp);
	return *this;
}

int MemberListEntry::getid() {
	return id;
}
short MemberListEntry::getport() {
	return port;
}
long MemberListEntry::getheartbeat() {
	return heartbeat;
}
long MemberListEntry::gettimestamp() {
	return timestamp;
}
void MemberListEntry::setid(int id) {
	this->id = id;
}
void MemberListEntry::setport(short port) {
	this->port = port;
}
void MemberListEntry::setheartbeat(long hearbeat) {
	this->heartbeat = hearbeat;
}
void MemberListEntry::settimestamp(long timestamp) {
	this->timestamp = timestamp;
}
Member::Member(const Member &anotherMember) {
	this->addr = anotherMember.addr;
	this->inited = anotherMember.inited;
	this->inGroup = anotherMember.inGroup;
	this->bFailed = anotherMember.bFailed;
	this->nnb = anotherMember.nnb;
	this->heartbeat = anotherMember.heartbeat;
	this->pingCounter = anotherMember.pingCounter;
	this->timeOutCounter = anotherMember.timeOutCounter;
	this->memberList = anotherMember.memberList;
	this->myPos = anotherMember.myPos;
	this->mp1q = anotherMember.mp1q;
}
Member& Member::operator =(const Member& anotherMember) {
	this->addr = anotherMember.addr;
	this->inited = anotherMember.inited;
	this->inGroup = anotherMember.inGroup;
	this->bFailed = anotherMember.bFailed;
	this->nnb = anotherMember.nnb;
	this->heartbeat = anotherMember.heartbeat;
	this->pingCounter = anotherMember.pingCounter;
	this->timeOutCounter = anotherMember.timeOutCounter;
	this->memberList = anotherMember.memberList;
	this->myPos = anotherMember.myPos;
	this->mp1q = anotherMember.mp1q;
	return *this;
}