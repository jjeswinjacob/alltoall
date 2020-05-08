#ifndef QUEUE_H_
#define QUEUE_H_

#include "stdincludes.h"
#include "Member.h"

// Contains Received Messages
class Queue {
	public:
		Queue() {}
		virtual ~Queue() {}
		// Only 1 copy of queue available
		// 1st element is a queue(STL) of q_elt's
		static bool enqueue(queue<q_elt> *queue, void *buffer, int size) {
			q_elt element(buffer, size);
			queue->emplace(element); // Adds a new element to end of queue
			return true;
		}
};

#endif /* QUEUE_H_ */
