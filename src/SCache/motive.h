#ifndef MOTIVE_H
#define MOTIVE_H

#include <utility>
#include <string>
#include <vector>
#include "attrib.h"

typedef std::map<std::string, std::pair<size_t, size_t>> TPushTable;
typedef std::vector<std::pair<std::string, std::pair<size_t, size_t>>>
	TPushArray;

struct TMotiveData{
	size_t trafficSize;
	//size_t objectsSize;
	TPushTable ids;
	TMotiveData();
	TMotiveData(const std::string& firstId, size_t size);
};

typedef std::pair<TObjectAttr, TMotiveData> TMotive;

class MotiveTable{
	std::map<TObjectAttr, TMotiveData> table;
	size_t divisor;
	int depth;
	mutable std::vector<TObjectAttr> currentSets;

	void makeFixedSets(const TObjectAttr& attrib, size_t size) const;
	void makeAllSets(const TObjectAttr& attrib) const;
	
public:
	MotiveTable();

	void SetDepth(int _depth);
	void AddRequest(const std::string& id,
		const TObjectAttr& attrib, size_t size);
	void Recalculate(std::vector<TMotive>& result,
		size_t maxSegments, double minPrecision);
	void Compress(double quotient);
	size_t GetDivisor() const;
	TPushArray GetPrePush(const TObjectAttr& attrib);
};

#endif // MOTIVE_H
