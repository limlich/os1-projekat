#include "sortlist.h"
#include "lock.h"

SortedList::SortedList(CompareFunction cmpFunc)
	: head_(NULL), iter_(NULL), size_(0), compare(cmpFunc)
{
}

SortedList::~SortedList()
{
	SOFT_LOCKED(

		while (!empty())
			popFront();

	);
}

void SortedList::insert(void * val)
{
	SOFT_LOCKED(
		
		SLEntry * tmp = new SLEntry(val);

		SLEntry * prev = NULL;
		SLEntry * curr = head_;

		while (curr && !compare(val, curr->value)) {
			prev = curr;
			curr = curr->next;
		}

		tmp->next = curr;

		if (prev)
			prev->next = tmp;
		else
			head_ = tmp;

		++size_;

	);
}

void * SortedList::popFront()
{
	SOFT_LOCKED(

		void * retVal = NULL;

		if (head_) {
			retVal = head_->value;

			if (iter_ == head_)
				iter_ = iter_->next;

			SLEntry * tmp = head_;
			head_ = head_->next;
			delete tmp;

			--size_;
		}

	);

	return retVal;
}

void * SortedList::remove(void * val)
{
	SOFT_LOCKED(

		void * retVal = NULL;
		SLEntry * prev = NULL;
		SLEntry * curr = head_;

		while (curr && curr->value != val) {
			prev = curr;
			curr = curr->next;
		}

		if (curr) {
			retVal = curr->value;

			if (iter_ == curr)
				iter_ = iter_->next;

			if (prev)
				prev->next = curr->next;
			else
				head_ = curr->next;

			delete curr;

			--size_;
		}

	);

	return retVal;
}
