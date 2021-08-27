#include "queue.h"
#include "lock.h"

Queue::Queue()
	: head_(NULL), tail_(NULL), iter_(NULL), size_(0)
{
}

Queue::~Queue()
{
	SOFT_LOCKED(

		while (!empty())
			popFront();

	);
}

void Queue::pushBack(void * val)
{
	SOFT_LOCKED(
		
		QEntry * tmp = new QEntry(val);

		if (!head_)
			head_ = tmp;
		else
			tail_->next = tmp;

		tail_ = tmp;

		++size_;

	);
}

void * Queue::popFront()
{
	SOFT_LOCKED(

		void * retVal = NULL;

		if (head_) {
			retVal = head_->value;

			if (iter_ == head_)
				iter_ = iter_->next;

			QEntry * tmp = head_;
			head_ = head_->next;
			delete tmp;

			if (!head_)
				tail_ = NULL;

			--size_;
		}

	);

	return retVal;
}

void * Queue::remove(void * val)
{
	SOFT_LOCKED(

		void * retVal = NULL;
		QEntry * prev = NULL;
		QEntry * curr = head_;

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

			if (curr == tail_)
				tail_ = prev;

			delete curr;

			--size_;
		}

	);

	return retVal;
}
