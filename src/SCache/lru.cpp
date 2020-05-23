#include <iterator>
#include "lru.h"

LRU::LRU():
	size(0),
	used(0),
	hitBt(0),
	totalBt(0),
	hitRq(0),
	totalRq(0)
{}

void LRU::freeSpace()
{
	auto min = table.begin();
	for(auto it = min; it != table.end(); it++){
		if(it->second.second < min->second.second){
			min = it;
		}
	}
	used -= min->second.first;
	table.erase(min);
}

bool LRU::put(const std::string& id, size_t itemSize, time_t timestamp)
{
	if(itemSize > size){
		return false;
	}
	if(!isInfinite()){
		while(size - used < itemSize){
			freeSpace();
		}
	}
	table[id] = std::make_pair(itemSize, timestamp);
	used += itemSize;
	return true;
}

bool LRU::Push(const std::string& id, size_t itemSize, time_t timestamp)
{
	auto it = table.find(id);
	if(it != table.end()){
		return true;
	}else{
		if(size - used < itemSize){
			return false;
		}
		table[id] = std::make_pair(itemSize, timestamp);
		used += itemSize;
		return true;
	}
}

bool LRU::Get(const std::string& id, size_t itemSize, time_t timestamp)
{
	totalRq++;
	totalBt += itemSize;

	auto it = table.find(id);
	if(it != table.end()){
		it->second.second = timestamp;
		hitRq++;
		hitBt += itemSize;
		return true;
	}else{
		put(id, itemSize, timestamp);
		return false;
	}
}

bool LRU::TryGet(const std::string& id) const
{
	auto it = table.find(id);
	return it != table.end();
}
