#include "map.h"

#include "lock.h" 

Map::~Map()
{
	SOFT_LOCKED(

		while (!chunks_.empty())
			delete[] (void**)chunks_.popFront();

	);
}

void Map::put(void * val)
{
	SOFT_LOCKED(

		int index = getKey(val) % chunkSize_;
		void ** chunk = NULL;

		chunks_.iterReset();
		while ((chunk = (void**)chunks_.iterGet()) != NULL && chunk[index])
			chunks_.iterNext();

		/* create new chunk */
		if (!chunk) {
			chunk = new void*[chunkSize_];
			for (int i = 0; i < chunkSize_; ++i)
				chunk[i] = NULL;

			chunk[index] = val;

			chunks_.pushBack(chunk);
		} else /* existing chunk */
			chunk[index] = val;

	);
}

void * Map::get(Key key)
{
	SOFT_LOCKED(

		int index = key % chunkSize_;
		void ** chunk = NULL;
		void * retVal = NULL;

		chunks_.iterReset();
		while ((chunk = (void**)chunks_.iterGet()) != NULL) {
			if (chunk[index] && (key == getKey(chunk[index]))) {
				retVal = chunk[index];
				break;
			}
			chunks_.iterNext();
		}

	);

	return retVal;
}

void * Map::remove(Key key)
{
	SOFT_LOCKED(

		int index = key % chunkSize_;
		void ** chunk = NULL;
		void * retVal = NULL;

		chunks_.iterReset();
		while ((chunk = (void**)chunks_.iterGet()) != NULL) {
			if (chunk[index] && (key == getKey(chunk[index]))) {
				retVal = chunk[index];
				chunk[index] = NULL;
				break;
			}
			chunks_.iterNext();
		}

	);

	return retVal;
}
