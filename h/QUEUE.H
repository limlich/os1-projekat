#ifndef _queue_h_
#define _queue_h_

#include "types.h"

class Queue
{
public:
	Queue();
	~Queue();

	void pushBack(void * val);
	void * popFront();
	void * remove(void * val);

	void iterReset() { iter_ = head_; }
	void iterNext() { if (iter_) iter_ = iter_->next; }
	void * iterGet() const { return iter_ ? iter_->value : NULL; }

	void * front() const { return head_ ? head_->value : NULL; }
	int size() const { return size_; }
	Bool empty() const { return !size_; }

private:
	struct QEntry
	{
		QEntry(void * val) : value(val), next(NULL) {}

		void *value;
		QEntry *next;
	};

	QEntry *head_, *tail_, *iter_;
	int size_;
};

#endif
