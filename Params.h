// Used to specify parameters from configuration files.

#ifndef _PARAMS_H_
#define _PARAMS_H_

#include "stdincludes.h"
#include "Params.h"
#include "Member.h"

// Allowed values of enum
enum testTYPE { CREATE_TEST, READ_TEST, UPDATE_TEST, DELETE_TEST };

class Params{
	public:
		int MAX_NNB;                // max number of neighbors
		int SINGLE_FAILURE;			// single/multi failure
		double MSG_DROP_PROB;		// message drop probability
		double STEP_RATE;		    // dictates the rate of insertion
		int EN_GPSZ;			    // actual number of peers
		int MAX_MSG_SIZE;
		int DROP_MSG;
		int dropmsg;
		int globaltime;
		int allNodesJoined;
		short PORTNUM;
		Params();
		void setparams(char *);
		int getcurrtime();
};

#endif /* _PARAMS_H_ */
