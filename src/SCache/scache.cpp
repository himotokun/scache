#include <algorithm>
#include "scache.h"
#include "lru.h"

template<typename T>
static bool isSubset(const std::vector<T>& haystack,
		const std::vector<T>& needle)
{
	for(auto& elem: needle){
		if(std::find(haystack.begin(), haystack.end(), elem) ==
				haystack.end()){
			return false;
		}
	}
	return true;
}

size_t SmartCache::getSegment(const std::string& id, size_t size,
		TObjectAttr& attrib)
{
	size_t result = 0;
	size_t maxDepth = 0;
	size_t segSize = segments.size();

	// try find already existing object
	for(size_t i = 0; i < segSize; i++){
		if(segments[i].cache->TryGet(id)){
			return i;
		}
	}

	// if not found, calculate potential segment
	for(size_t i = 0; i < segSize; i++){
		if(isSubset(attrib, segments[i].motive) &&
				segments[i].motive.size() > maxDepth &&
				(segments[i].size >= size || segments[i].queue >= size))
		{
			maxDepth = segments[i].motive.size();
			result = i;
		}
	}

	return result;
}

size_t SmartCache::getRealSegmentSize(const TMotive& motive) const
{
	dprint("SmartCache::getRealSegmentSize < [");
	for(auto& m: motive.first){
		dprint("'%s'='%s',", m.first.c_str(), m.second.c_str());
	}
	dprint("]\n");
	double precision = ((double)motive.second.trafficSize) /
		slotizer.GetDivisor();

	//size_t result = (motive.second.objectsSize * precision) *
	//	cacheSize / totalObjectsSize;

	size_t result = precision * cacheSize;

	/*dprint("#calc#: %lu * (%lu / %lu) * %lu / %lu\n",
		motive.second.objectsSize,
		motive.second.trafficSize, slotizer.GetDivisor(),
		cacheSize, totalObjectsSize);*/

	dprint("SmartCache::getRealSegmentSize > %lu\n", result);
	return result;
}

#if 1
void SmartCache::recountSegments()
{
	size_t i = 0;
	size_t freeSpace = segments[i].size;
	std::vector<size_t> queuedSizes(segments.size() - 1, 0);

	dprint("*** COUNTED MOTIVES ***\n");
	for(auto& m: motives){
		dprint("Motive %lu: [", i);
		for(auto& p: m.first){
			dprint("'%s'='%s', ", p.first.c_str(), p.second.c_str());
		}
		dprint("]\n");
		/*printf("Precision: %lf, objects: %lu (%lu KB)\n",
			(double)m.second.trafficSize / slotizer.GetDivisor(),
			m.second.ids.size(), m.second.objectsSize);*/
		dprint("Precision: %lf%%\n",
			(double)m.second.trafficSize / slotizer.GetDivisor() * 100);
		i++;
	}
	dprint("\n");

	// count potential sizes for existing segments
	for(size_t iNew = 0; iNew < motives.size(); iNew++){
		bool doNew = true;
		size_t newSize = getRealSegmentSize(motives[iNew]);
		for(size_t iOld = 1; iOld < segments.size(); iOld++){
			if(isSubset(segments[iOld].motive, motives[iNew].first)){
				queuedSizes[iOld - 1] = newSize;
				doNew = false;
				// instanly change the motive
				segments[iOld].motive = motives[iNew].first;
				break;
			}
		}
		// create the new segment
		if(doNew){
			segments.push_back(CacheSegment());
			segments.rbegin()->motive = motives[iNew].first;
			segments.rbegin()->cache = factory->Create();
			queuedSizes.push_back(newSize);
		}
	}

	dprint("QUEUED SIZES:\n");
	for(size_t i = 0; i < queuedSizes.size(); i++){
		dprint("\t#%lu - %lu\n", i + 1, queuedSizes[i]);
	}
	dprint("\n");

	// delete unneeded segments
	for(size_t i = 1; i < segments.size();){
		dprint("* delete #%lu: ", i);
		if(queuedSizes[i - 1] == 0){
			dprint(" yes\n");
			freeSpace += segments[i].size;
			factory->Destroy(segments[i].cache);
			segments.erase(segments.begin() + i);
			queuedSizes.erase(queuedSizes.begin() +  i - 1);
		}else{
			dprint(" no\n");
			i++;
		}
	}
	// parse segments shrinking
	for(size_t i = 1; i < segments.size(); i++){
		dprint("* shrink #%lu: %lu -> %lu: ", i,
			segments[i].size, queuedSizes[i - 1]);
		if(queuedSizes[i - 1] < segments[i].size){
			dprint(" yes\n");
			freeSpace += segments[i].size - queuedSizes[i - 1];
			segments[i].SetSize(queuedSizes[i - 1]);
		}else{
			dprint(" no\n");
		}
	}
	// parse segments expanding
	for(size_t i = 1; i < segments.size(); i++){
		dprint("* expand #%lu: %lu -> %lu: ", i,
			segments[i].size, queuedSizes[i - 1]);
		if(queuedSizes[i - 1] > segments[i].size){
			dprint(" yes\n");
			if(freeSpace < queuedSizes[i - 1] - segments[i].size){
				segments[i].SetSize(segments[i].size + freeSpace);
				freeSpace = 0;
				dprint("\t[last]\n");
				break;
			}
			freeSpace -= queuedSizes[i - 1] - segments[i].size;
			segments[i].SetSize(queuedSizes[i - 1]);
		}else{
			dprint(" no\n");
		}
	}
	// fill the free space with non-motived segment
	segments[0].SetSize(freeSpace);
	// do the pre-push
	if(true){
		for(auto& seg: segments){
			auto table = slotizer.GetPrePush(seg.motive);
			for(auto& item: table){
				auto& id = item.first;
				auto& size = item.second.second;
				seg.cache->Push(id, size, lastTimestamp);
			}
		}
	}
	// delete dead segments
	for(size_t i = 0; i < segments.size(); i++){
		if(segments[i].size == 0){
			segments.erase(segments.begin() + i);
		}else{
			i++;
		}
	}
}
#else

void SmartCache::recountSegments()
{
	system("clear");

#if 0
	printf("*** COUNTED MOTIVES ***\n");
	for(auto& m: motives){
		printf("Motive %lu: [", i);
		for(auto& p: m.first){
			printf("'%s'='%s', ", p.first.c_str(), p.second.c_str());
		}
		printf("]\n");
		printf("Precision: %lf, objects: %lu (%lu KB)\n",
			(double)m.second.trafficSize / slotizer.GetDivisor(),
			m.second.ids.size(), m.second.objectsSize);
		i++;
	}
	printf("\n");
#endif

	// count sizes for existing segments
	size_t freeSpace = cacheSize;
	for(size_t iNew = 0; iNew < motives.size() && freeSpace > 0; iNew++){
		bool doNew = true;
		size_t newSize = getRealSegmentSize(motives[iNew]);
		for(size_t iOld = 1; iOld < segments.size(); iOld++){
			if(isSubset(segments[iOld].motive, motives[iNew].first)){
				if(newSize < freeSpace){
					segments[iOld].queue = newSize;
					freeSpace -= newSize;
				}else{
					segments[iOld].queue = freeSpace;
					freeSpace = 0;
					break;
				}
				doNew = false;
				segments[iOld].motive = motives[iNew].first;
				break;
			}
		}
		// create the new both queued and zero-sized real segments
		if(doNew && segments.size() <= maxSegments){
			segments.push_back(CacheSegment());
			segments.rbegin()->cache = factory->Create();
			segments.rbegin()->motive = motives[iNew].first;
			segments.rbegin()->SetSize(0);
			if(newSize < freeSpace){
				segments.rbegin()->queue = newSize;
				freeSpace -= newSize;
			}else{
				segments.rbegin()->queue = freeSpace;
				freeSpace = 0;
				break;
			}
		}
	}
	segments[0].queue = freeSpace;

	for(auto& s: segments){
		if(s.cache != segments[0].cache)
			s.queue = cacheSize;
	}

	// delete dead segments
	for(size_t i = 0; i < segments.size(); i++){
		if(segments[i].size == 0 && segments[i].queue == 0){
			factory->Destroy(segments[i].cache);
			segments.erase(segments.begin() + i);
		}
	}

rt:	printf("### SEGMENT STATE ###\n");
	size_t i = 0;
	for(auto& s: segments){
		printf("Segment %lu: [", i);
		for(auto& p: s.motive){
			printf("'%s'='%s', ", p.first.c_str(), p.second.c_str());
		}
		printf("]\n");
		printf("Size: %luM, %.3lg%% of cache, used %.3lg%% | "
				"Queued: %luM, %.3lg%% of cache | Rel: S/Q %.3lg%% | "
				"RHR: %.3lg, BHR: %.3lg\n",
			s.size / 1024, (double)s.size / cacheSize * 100,
			static_cast<LRU*>(s.cache)->GetUsedSpace() * 100,
			s.queue / 1024, (double)s.queue / cacheSize * 100,
			(double)s.size / s.queue * 100,
			s.cache->GetRHR(), s.cache->GetBHR()
			);
		i++;
	}
	/*printf("*** SECOND SEGMENT ***\n");
	printf("Size: %luM, %.3lg%% of cache, used %.3lg%% | "
			"Queued: %luM, %.3lg%% of cache | Rel: S/Q %.3lg%% | "
			"RHR: %.3lg, BHR: %.3lg\n",
		second.size / 1024, (double)second.size / cacheSize * 100,
		static_cast<LRU*>(second.cache)->GetUsedSpace() * 100,
		second.queue / 1024, (double)second.queue / cacheSize * 100,
		(double)second.size / second.queue * 100,
		second.cache->GetRHR(), second.cache->GetBHR()
		);*/
	printf("\n");
}
#endif

size_t SmartCache::getRecalculatedSegment(const std::string& id,
		size_t size, time_t timestamp, TObjectAttr& attrib)
{
	size_t idx = getSegment(id, size, attrib);

	if(segments[idx].queue > segments[idx].size){
		size_t left = size;
		// shrink other segments
		for(auto& s: segments){
			if(s.size > s.queue){
				size_t diff = s.size - s.queue;
				if(left < diff){
					s.SetSize(s.size - left);
					segments[idx].SetSize(segments[idx].size + size);
					return idx;;
				}else{
					left -= diff;
					s.SetSize(s.queue);
				}
			}
		}
		// expand current segment
		segments[idx].SetSize(segments[idx].size + size - left);
	}
	return idx;
}

bool SmartCache::Get(const std::string& id, size_t size, time_t timestamp)
{
	bool flag = false;
	totalRq++;
	totalBt += size;
	lastTimestamp = timestamp;

	if(allIds.count(id) == 0){
		allIds.insert(id);
		totalObjectsSize += size;
	}

	auto attrib = Storage()[id];
	size_t seg = getSegment(id, timestamp, attrib);

	bool isHit = segments[seg].cache->Get(id, size, timestamp);
	if(isHit){
		hitRq++;
		hitBt += size;
		flag = true;
	}

	motives.clear();
	bool doUpd = slotizer.AddRequest(id, attrib, size, timestamp, motives);
	if(doUpd){
		recountSegments();
	}

	return flag;
}

void SmartCache::SetSize(size_t size)
{
	assert(cacheSize == 0);
	segments[0].SetSize(cacheSize = size);
	// TODO
}

void SmartCache::PrintSegmentState() const
{
	printf("### SEGMENT STATE ###\n");
	size_t i = 0;
	for(auto& s: segments){
		printf("Segment %lu: [", i);
		for(auto& p: s.motive){
			printf("'%s'='%s', ", p.first.c_str(), p.second.c_str());
		}
		printf("]\n");
		printf("Size: %.3lg%% (used %.3lg%%)\n",
			(double)s.size / cacheSize * 100,
			static_cast<LRU*>(s.cache)->GetUsedSpace() * 100);
		i++;
	}
	printf("\n");

}
