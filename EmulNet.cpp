// Emulated Network classes definition
// ENsend places passed data into a buffer, alongwith from and to
// ENRecv takes data, from buffer, and places into queue
#include "EmulNet.h"

// Emulator Node initialized.
EmulNet::EmulNet(Params *p)
{
	//trace.funcEntry("EmulNet::EmulNet");
	int i, j;
	par = p;
	emulnet.setNextId(1);
	emulnet.settCurrBuffSize(0);
	enInited=0;
	for ( i = 0; i < MAX_NODES; i++ ) {
		for ( j = 0; j < MAX_TIME; j++ ) {
			sent_msgs[i][j] = 0;
			recv_msgs[i][j] = 0;
		}
	}
	//trace.funcExit("EmulNet::EmulNet", SUCCESS);
}

EmulNet::EmulNet(EmulNet &anotherEmulNet) {
	int i, j;
	this->par = anotherEmulNet.par;
	this->enInited = anotherEmulNet.enInited;
	for ( i = 0; i < MAX_NODES; i++ ) {
		for ( j = 0; j < MAX_TIME; j++ ) {
			this->sent_msgs[i][j] = anotherEmulNet.sent_msgs[i][j];
			this->recv_msgs[i][j] = anotherEmulNet.recv_msgs[i][j];
		}
	}
	this->emulnet = anotherEmulNet.emulnet;
}

EmulNet& EmulNet::operator =(EmulNet &anotherEmulNet) {
	int i, j;
	this->par = anotherEmulNet.par;
	this->enInited = anotherEmulNet.enInited;
	for ( i = 0; i < MAX_NODES; i++ ) {
		for ( j = 0; j < MAX_TIME; j++ ) {
			this->sent_msgs[i][j] = anotherEmulNet.sent_msgs[i][j];
			this->recv_msgs[i][j] = anotherEmulNet.recv_msgs[i][j];
		}
	}
	this->emulnet = anotherEmulNet.emulnet;
	return *this;
}

EmulNet::~EmulNet() {}

// Init the emulnet for this node.
// id = id + 1; port = 0
// ENinit is called once by each node (peer) to initialize its own address (myaddr).
void *EmulNet::ENinit(Address *myaddr, short port) {
	// Initialize data structures for this member
	*(int *)(myaddr->addr) = emulnet.nextid++; // Settimng id no of new node to next available
    *(short *)(&myaddr->addr[4]) = 0;
	return myaddr;
}

// EmulNet send function
// Send Waiting Messages (to Buffer) from arguments passed
int EmulNet::ENsend(Address *myaddr, Address *toaddr, char *data, int size) {
	en_msg *em;
	static char temp[2048];
	int sendmsg = rand() % 100;

	// If Buffer Size Exceeded (or) Message Size Exceeded (or) Message Drop is on and less - CANT SEND
	if( (emulnet.currbuffsize >= ENBUFFSIZE) || (size + (int)sizeof(en_msg) >= par->MAX_MSG_SIZE) || (par->dropmsg && sendmsg < (int) (par->MSG_DROP_PROB * 100)) ) {
		return 0;
	}

	// We might be specifying the size of a blocks of en_msg's in size. The data contains the block
	em = (en_msg *)malloc(sizeof(en_msg) + size);
	em->size = size;

	// We're copying 'From' Address, 'To' Address, and data to em + 1
	memcpy(&(em->from.addr), &(myaddr->addr), sizeof(em->from.addr));
	memcpy(&(em->to.addr), &(toaddr->addr), sizeof(em->from.addr));
	memcpy(em + 1, data, size);

	// We're storing Emulnet Messaage into the Emulnet Network
	// Guess Messages 'from' and 'to' are but entries in the emulnet class data structure
	emulnet.buff[emulnet.currbuffsize++] = em;

	// myaddr is the From Address
	int src = *(int *)(myaddr->addr);
	int time = par->getcurrtime();

	// If condition equates to 0, error thrown and aborted
	assert(src <= MAX_NODES);
	assert(time < MAX_TIME);

	sent_msgs[src][time]++; // Part of Hash Table

	// If defined, print to temp
	// ??
	#ifdef DEBUGLOG
		sprintf(temp, "Sending 4+%d B msg type %d to %d.%d.%d.%d:%d ", size-4, *(int *)data, toaddr->addr[0], toaddr->addr[1], toaddr->addr[2], toaddr->addr[3], *(short *)&toaddr->addr[4]);
	#endif

	return size;
}

// Looks like a plugin to help with the above function
// RETURNS: size
int EmulNet::ENsend(Address *myaddr, Address *toaddr, string data) {
	char * str = (char *) malloc(data.length() * sizeof(char));
	memcpy(str, data.c_str(), data.size()); // Returns length of data in bytes
	int ret = this->ENsend(myaddr, toaddr, str, (data.length() * sizeof(char)));
	free(str);
	return ret;
}

// What function are we referencing? - enq?
// enq* and (void *, char *, int)
// https://www3.ntu.edu.sg/home/ehchua/programming/cpp/cp4_PointerReference.html
// Function Pointers
// Return (*operation)(void *, char *, int) as int
// ENrecv enqueues a received message using a function specified through a pointer enqueue()
// Takes Messages from buffer and puts into queue
int EmulNet::ENrecv(Address *myaddr, int (* enq)(void *, char *, int), struct timeval *t, int times, void *queue){
	// times is always assumed to be 1
	int i;
	char* tmp;
	int sz;
	en_msg *emsg;

	// We're retrieving messages from the buffer (sent by ENsend) and storing in queue.
	for( i = emulnet.currbuffsize - 1; i >= 0; i-- ) {
		emsg = emulnet.buff[i];

		// If from address is not equal to To Address
		if ( 0 == strcmp(emsg->to.addr, myaddr->addr) ) {
			sz = emsg->size;
			// Allocate Size Dynamically and Assign Value
			tmp = (char *) malloc(sz * sizeof(char));
			memcpy(tmp, (char *)(emsg+1), sz);

			emulnet.buff[i] = emulnet.buff[emulnet.currbuffsize-1]; // RHS is constant
			emulnet.currbuffsize--;

			// We're performing the enjuining of queue with elements from buffer
			// What is being passed is being executed
			// enqueueWrapper(void *env, char *buff, int size)
			(*enq)(queue, (char *)tmp, sz);

			free(emsg);

			int dst = *(int *)(myaddr->addr);
			int time = par->getcurrtime();

			assert(dst <= MAX_NODES);
			assert(time < MAX_TIME);

			recv_msgs[dst][time]++;
		}
	}
	return 0;
}

// Cleanup the EmulNet. Called exactly once at the end of the program.
// Clears Buffer. And prints the count of sent and received messages in msgcount.log
int EmulNet::ENcleanup() {
	emulnet.nextid=0;
	int i, j;
	int sent_total, recv_total;

	FILE* file = fopen("msgcount.log", "w+");

	while(emulnet.currbuffsize > 0) {
		free(emulnet.buff[--emulnet.currbuffsize]);
	}

	// EN_GPSZ is the actual number of peers
	for ( i = 1; i <= par->EN_GPSZ; i++ ) {
		fprintf(file, "node %3d ", i); // Set width to 3.
		sent_total = 0;
		recv_total = 0;

		for (j = 0; j < par->getcurrtime(); j++) {

			sent_total += sent_msgs[i][j];
			recv_total += recv_msgs[i][j];
			if (i != 67) {
				fprintf(file, " (%4d, %4d)", sent_msgs[i][j], recv_msgs[i][j]);
				if (j % 10 == 9) {
					fprintf(file, "\n         ");
				}
			}
			else {
				fprintf(file, "special %4d %4d %4d\n", j, sent_msgs[i][j], recv_msgs[i][j]);
			}
		}
		fprintf(file, "\n");
		fprintf(file, "node %3d sent_total %6u  recv_total %6u\n\n", i, sent_total, recv_total);
	}

	fclose(file);
	return 0;
}