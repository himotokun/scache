#ifndef LRU_H
#define LRU_H

#include <string>
#include <utility>
#include <map>
#include "scache.h"
#include "dp.h"

const size_t INFINITE_SIZE = (size_t)-1;

class LRU: public ICache{
	size_t size;
	size_t used;

	size_t hitBt, totalBt;
	size_t hitRq, totalRq;

	typedef std::pair<size_t, time_t> TItemData;
	std::map<std::string, TItemData> table;

	void freeSpace();
	bool put(const std::string& id, size_t size, time_t timestamp);
	static double div(double a, size_t b){
		return b == 0 ? 0 : a / b;
	}

	bool isInfinite() const { return size == INFINITE_SIZE; }
public:
	LRU();
	virtual ~LRU(){}
	virtual bool Get(const std::string& id, size_t size, time_t timestamp);
	virtual bool Push(const std::string& id, size_t size,
		time_t timestamp);
	virtual bool TryGet(const std::string& id) const;
	virtual void SetSize(size_t newSize)
	{
		if(newSize == 0){
			table.clear();
			used = 0;
			return;
		}
		while(used > newSize){
			freeSpace();
		}
		size = newSize;
	}
	virtual size_t GetFreeSpace() const{ return size - used; }

	virtual double GetRHR() const { return div(hitRq, totalRq); }
	virtual double GetBHR() const { return div(hitBt, totalBt); }
	double GetUsedSpace() const
	{
		return size && !isInfinite() ? ((double)used) / size : 0;
	}
};

class LRUFactory: public ICacheFactory{
public:
	virtual ICache* Create(){
		return new LRU();
	}
	virtual void Destroy(ICache* cache){
		delete dynamic_cast<LRU*>(cache);
	}
};

#endif // LRU_H
