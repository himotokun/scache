#ifndef SCACHE_H
#define SCACHE_H

#include "attrib.h"
#include "motive.h"
#include "slotizer.h"
#include <string>
#include <vector>
#include <set>
#include <assert.h>
#include "dp.h"

class ICache{
public:
	virtual bool Get(const std::string& id,
		size_t size, time_t timestamp) = 0;
	virtual bool TryGet(const std::string& id) const = 0;
	virtual void SetSize(size_t size) = 0;
	virtual size_t GetFreeSpace() const = 0;
	virtual double GetRHR() const = 0;
	virtual double GetBHR() const = 0;
	virtual bool Push(const std::string& id, size_t size,
		time_t timestamp) = 0;
	virtual ~ICache(){};
};

class ICacheFactory{
public:
	virtual ICache* Create() = 0;
	virtual void Destroy(ICache*) = 0;
};

struct CacheSegment{
	ICache* cache;
	TObjectAttr motive;
	size_t size;
	size_t queue;

	CacheSegment():
		size(0),
		cache(0),
		queue(0)
	{}

	void SetSize(size_t _size){
		dprint("\n> Set size *%p: %lu -> %lu <\n", this, size, _size);
		assert(cache != 0);
		cache->SetSize(queue = size = _size);
	}
};

class SmartCache: public ICache{
	SmartCache(const SmartCache&);

	std::vector<CacheSegment> segments;
	size_t cacheSize;
	std::vector<TMotive> motives;
	ICacheFactory* factory;
	Slotizer slotizer;

	std::set<std::string> allIds;
	size_t totalObjectsSize;
	time_t lastTimestamp;
	size_t maxSegments;

	size_t hitBt, totalBt;
	size_t hitRq, totalRq;

	size_t getSegment(const std::string&, size_t, TObjectAttr&);
	size_t getRecalculatedSegment(const std::string&, size_t, time_t,
		TObjectAttr&);

	size_t getRealSegmentSize(const TMotive& motive) const;
	void recountSegments();
public:
	SmartCache(ICacheFactory* _factory):
		segments(0),
		cacheSize(0),
		totalObjectsSize(0),
		factory(_factory),
		hitBt(0),
		totalBt(0),
		hitRq(0),
		totalRq(0)
	{
		segments.push_back(CacheSegment());
		segments[0].cache = factory->Create();
		segments[0].SetSize(cacheSize);
	}
	virtual ~SmartCache(){}

	void SetMaxSegments(size_t segNum){
		slotizer.SetMaxSegments(maxSegments = segNum);
	}
	void SetMinPrecision(double prec){ slotizer.SetMinPrecision(prec); }
	void SetSlotLength(time_t length){ slotizer.SetSlotLength(length); }
	void SetSlotsCount(size_t count){ slotizer.SetSlotsCount(count); }
	void SetDepth(size_t depth){ slotizer.SetDepth(depth); }

	virtual bool Get(const std::string& id, size_t size, time_t timestamp);
	virtual void SetSize(size_t size);

	virtual size_t GetFreeSpace() const{ assert(false); }
	virtual bool TryGet(const std::string& id) const{ assert(false); }

	virtual bool Push(const std::string& id, size_t size,
		time_t timestamp){ assert(false); };

	virtual double GetRHR() const{ return ((double)hitRq) / totalRq; }
	virtual double GetBHR() const{ return ((double)hitBt) / totalBt; }

	void PrintSegmentState() const;
};

#endif // SCACHE_H
