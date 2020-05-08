// Implements Membership Protocol - ALL to ALL HeartBeating
#include "MP1Node.h"

MP1Node::MP1Node(Member *member, Params *params, EmulNet *emul, Log *log, Address *address) {
	for( int i = 0; i < 6; i++ ) 
		NULLADDR[i] = 0;
	this->memberNode = member; // Address, Membership Tables
	this->emulNet = emul; // For Sending and Receiving Messages
	this->log = log; // Debug Messages
	this->par = params; // From .conf
	this->memberNode->addr = *address;
    // = operator has been overloaded for each of the above assignments
}

MP1Node::~MP1Node() {}

// This function receives message from the network(Possibly Emulnet) and pushes into the queue
// This function is called by a node to receive messages currently waiting for it

int MP1Node::recvLoop() {
    if ( memberNode->bFailed )
    {   // boolean indicating if this member has failed
    	return false;
    }
    else {
    	return emulNet->ENrecv(&(memberNode->addr), enqueueWrapper, NULL, 1, &(memberNode->mp1q));
        // mp1q - Queue for failure detection messages
        // Therefore, each node stores in it's own queue - mp1q
    }
}

// Enqueue the message from Emulnet into the queue
// buff is part of emulnet
int MP1Node::enqueueWrapper(void *env, char *buff, int size) {
	Queue q;
	return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
    // Calls Class Queue and performs Emplace to add buffer elements into queue
}

/* This function bootstraps the node
All initializations routines for a member.
Called by the application layer.*/
void MP1Node::nodeStart(char *servaddrstr, short servport) {
    Address joinaddr;
    joinaddr = getJoinAddress();

    // Self booting routines
    if( initThisNode(&joinaddr) == -1 ) {
        #ifdef DEBUGLOG
                log->LOG(&memberNode->addr, "init_thisnode failed. Exit.");
        #endif
        // in formar - node address [globaltime] message        
        exit(1);
    }

    if( !introduceSelfToGroup(&joinaddr) ) {
        finishUpThisNode();
        #ifdef DEBUGLOG
                log->LOG(&memberNode->addr, "Unable to join self to group. Exiting.");
        #endif
        exit(1);
    }
    return;
}

// Find out who I am and start up
int MP1Node::initThisNode(Address *joinaddr) {
	 // This function is partially implemented and may require changes
     // 1. Returning -1
	int id = *(int*)(&memberNode->addr.addr); // From memberNode, not coordinator
	int port = *(short*)(&memberNode->addr.addr[4]);

	memberNode->bFailed = false; // boolean indicating if this member has failed
	memberNode->inited = true; // boolean indicating if this member is up
	memberNode->inGroup = false; // boolean indicating if this member is in the group
    // node is up!
	memberNode->nnb = 0; // number of my neighbors
	memberNode->heartbeat = 0; // the node's own heartbeat
	memberNode->pingCounter = TFAIL; // counter for next ping
	memberNode->timeOutCounter = -1; // #define TFAIL 5
    initMemberListTable(memberNode); // Initialize the membership list by clearing

    return 0;
}

// Join the distributed system
int MP1Node::introduceSelfToGroup(Address *joinaddr) {
	MessageHdr *msg;
    #ifdef DEBUGLOG
        static char s[1024];
    #endif

    if (0 == memcmp((char *)&(memberNode->addr.addr), (char *)&(joinaddr->addr), sizeof(memberNode->addr.addr))) {
        // I am the group booter (first process to join the group). Boot up the group
        #ifdef DEBUGLOG
                log->LOG(&memberNode->addr, "Starting up group...");
        #endif
        memberNode->inGroup = true; // boolean indicating if this member is in the group
        UpdateEntry(memberNode -> addr, memberNode -> heartbeat);
    }
    else {
        // Header and content of a message // Values which can be taken by MsgTypes
        size_t msgsize = sizeof(MessageHdr) + sizeof(joinaddr->addr) + sizeof(long) + 1;
        msg = (MessageHdr *) malloc(msgsize * sizeof(char));

        // create JOINREQ message: format of data is {struct Address myaddr}
        // msg - JOINREQ, Address of New Member, HeartBeat
        msg->msgType = JOINREQ;
        memcpy((char *)(msg+1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy((char *)(msg+1) + 1 + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long)); // Heartbeat is in long

        #ifdef DEBUGLOG
                sprintf(s, "Trying to join...");
                log->LOG(&memberNode->addr, s);
        #endif

        // send JOINREQ message to introducer member
        emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, msgsize);

        free(msg);
    }
    return 1;
}

// Wind up this node and clean up state
int MP1Node::finishUpThisNode(){
   memberNode -> memberList.clear();
   memberNode -> inGroup = false;
}

// Executed periodically at each member
// Check your messages in queue and perform membership protocol duties
void MP1Node::nodeLoop() {
    if (memberNode->bFailed) {
    	return;
    }
    // You can check for messages even if you're not part of the group
    // Check my messages
    checkMessages();

    // Wait until you're in the group...
    if( !memberNode->inGroup ) return;

    // ...then jump in and share your responsibilites!
    nodeLoopOps();
    return;
}

// Check messages in the queue and call the respective message handler
void MP1Node::checkMessages() {
    // Pop waiting messages from memberNode's mp1q
    while ( !memberNode->mp1q.empty() ) {
        int size = memberNode->mp1q.front().size;
    	void *pointer = memberNode->mp1q.front().elt;
    	memberNode->mp1q.pop();
    	recvCallBack((void *)memberNode, (char *)pointer, size);
    }
    return;
}

// Message handler for different message types
bool MP1Node::recvCallBack(void *env, char *data, int size ) {
    MessageHdr *message = (MessageHdr *) data; // message - JOINREQ, Address of New Member, HeartBeat
    char *messageBody = (char *)(message + 1); // Contains Address and Heartbeat
    switch(message -> msgType) {
        case JOINREQ:
            // JOINREQ is sent only to Introducer from various nodes
            // Create JOINREP message
            {
            long senderHeartbeat;
            Address senderAddress;
            memcpy(&senderAddress, (char *)(message + 1), sizeof(memberNode -> addr.addr));
            memcpy(&senderHeartbeat, (char *)(message + 1) + 1 + sizeof(memberNode -> addr.addr), sizeof(long));
            UpdateEntry(senderAddress, senderHeartbeat); // Updates New Nodes Joining in Introducer
            sendMemberList(senderAddress, JOINREP);
            // Once Introducer Acknowledges, we send a JOINREP as below.
            // We Send Number of Entries + Our copy of Membership Table
            break;
            }
        case JOINREP:
        {
            memberNode -> inGroup = true; // Introducer has acknowledged you're in
            updateMemberList(messageBody);
            break;
        }
        // JOINREQ and JOINREP seem to be mostly for introducer. UPDATEMEMLIST is for peers.
        case UPDATEMEMBERLIST:
        {
            updateMemberList(messageBody);
            break;
        }
        default:
            return false;
    }
    return true;
}

// Check if any node hasn't responded within a timeout period and then delete the nodes
// and Propagate your membership list
void MP1Node::nodeLoopOps() {

    // Increasing One's own heartbeat
    ++memberNode -> heartbeat;
    UpdateEntry(memberNode->addr, memberNode -> heartbeat);

    // Deletion of Failed Nodes
    long Time = par -> getcurrtime();
    for(vector<MemberListEntry>::iterator it = memberNode -> memberList.begin(); it != memberNode -> memberList.end(); ++it) {
        if(Time - it -> timestamp >= TREMOVE) {
            Address addr = Ret_address(it -> id, it -> port);
            memberNode -> memberList.erase(it);
            log -> logNodeRemove(&memberNode -> addr, &addr);
        }
    }
    
    // Send Member List to Group
    for(vector<MemberListEntry>::iterator it = memberNode -> memberList.begin(); it != memberNode -> memberList.end(); ++it) {
        if(Time - it -> timestamp >= TFAIL) continue;
        Address addr = Ret_address(it -> id, it -> port);
        sendMemberList(addr, UPDATEMEMBERLIST);
    }
    return;
}

//Function checks if the address is NULL
int MP1Node::isNullAddress(Address *addr) {
	return (memcmp(addr->addr, NULLADDR, 6) == 0 ? 1 : 0);
}

// Returns the Address of the coordinator
Address MP1Node::getJoinAddress() {
    Address joinaddr;

    memset(&joinaddr, 0, sizeof(Address)); // Set all bytes of joinaddr to 0.
    *(int *)(&joinaddr.addr) = 1; // id = 1
    *(short *)(&joinaddr.addr[4]) = 0; // port = 0

    return joinaddr;
}

// Initialize the membership list
// Helps in clearning possible messages before failing.
void MP1Node::initMemberListTable(Member *memberNode) {
	memberNode->memberList.clear(); // Vector Cleared
}

//Print the Address
void MP1Node::printAddress(Address *addr)
{
    printf("%d.%d.%d.%d:%d \n",  addr->addr[0],addr->addr[1],addr->addr[2],
                                                       addr->addr[3], *(short*)&addr->addr[4]) ;    
}

void MP1Node::UpdateEntry(Address memberAddr, long heartbeat) {
    int id = *(int *)(&memberAddr.addr[0]);
    int port = *(short *)(&memberAddr.addr[4]);
    bool memberFound = false;

    for(vector<MemberListEntry>::iterator it = memberNode -> memberList.begin(); it != memberNode -> memberList.end(); ++it) {
        if(it -> id == id && it -> port == port) {
            memberFound = true;
            if(it -> heartbeat < heartbeat) {
                it -> heartbeat = heartbeat;
                it -> timestamp = par -> getcurrtime();
            }
            break;
        }
    }
    if(!memberFound) {
        memberNode -> memberList.push_back(MemberListEntry(id, port, heartbeat, par -> getcurrtime()));
        memberNode -> nnb += 1;
        log -> logNodeAdd(&memberNode -> addr, &memberAddr);
    }
}
// Once Introducer Acknowledges, we send a JOINREP as below.
// We Send Number of Entries + Our copy of Membership Table

void MP1Node::sendMemberList(Address to, MsgTypes Type) {
    int N = memberNode -> memberList.size();
    size_t size = sizeof(MessageHdr) + sizeof(long) + N * sizeof(MemberListEntry);
    MessageHdr *message = (MessageHdr *) malloc(size * sizeof(char));

    // create JOINREP message: format of data is {struct MemberListEntry} 
    message -> msgType = Type;
    memcpy((char *)(message + 1), &N, sizeof(int));
    char *pointer = (char *)(message + 1) + sizeof(int);
    for(vector<MemberListEntry>::iterator it = memberNode->memberList.begin(); it != memberNode -> memberList.end(); ++it){
        memcpy(pointer, &(*it), sizeof(MemberListEntry));
        pointer += sizeof(MemberListEntry);
    }
    emulNet -> ENsend(&memberNode -> addr, &to, (char *)message, size);
    free(message);
}

// Updates Membership Table Entries with newly received Table
void MP1Node::updateMemberList(char *data) {
    int N;
    memcpy(&N, (char *)data, sizeof(int));
    MemberListEntry* memberListEntries = (MemberListEntry *)(data + sizeof(int));
    for(int i = 0; i < N; i++){
        Address address = Ret_address(memberListEntries[i].id, memberListEntries[i].port);
        UpdateEntry(address, memberListEntries[i].heartbeat);
    }
}

// Returns Address
Address MP1Node::Ret_address(int ip, short port) {
    Address address;
    memset(&address, 0, sizeof(Address));
    memcpy(&address.addr[0], &ip, sizeof(int));
    memcpy(&address.addr[4], &port, sizeof(short));
    return address;
}