#ifndef _map_h_
#define _map_h_

#include "queue.h"
#include "types.h"

typedef int Key;

// return value: key associated with pointer argument
typedef Key (*MapKeyFunction)(const void*);

class Map
{
public:
	Map(MapKeyFunction keyFunc, int chunkSize = 50)
		: getKey(keyFunc), chunkSize_(chunkSize)
	{}
	~Map();

	void put(void * val);
	void * get(Key key);
	void * remove(Key key);

private:
	Queue chunks_;
	const int chunkSize_;
	MapKeyFunction getKey;
};

#endif
