#ifndef _sortlist_h_
#define _sortlist_h_

#include "types.h"

// return value:
// true  - if first argument should come before the second in the list
// false - if first argument should come after  the second in the list
typedef Bool (*CompareFunction)(const void*,const void*);

class SortedList
{
public:
	SortedList(CompareFunction cmpFunc);
	~SortedList();

	void insert(void * val);
	void * popFront();
	void * remove(void * val);

	void iterReset() { iter_ = head_; }
	void iterNext() { if (iter_) iter_ = iter_->next; }
	void * iterGet() const { return iter_ ? iter_->value : NULL; }

	void * front() const { return head_ ? head_->value : NULL; }
	int size() const { return size_; }
	Bool empty() const { return !size_; }

private:
	struct SLEntry
	{
		SLEntry(void * val) : value(val), next(NULL) {}

		void *value;
		SLEntry *next;
	};

	SLEntry *head_, *iter_;
	int size_;

	CompareFunction compare;
};

#endif
