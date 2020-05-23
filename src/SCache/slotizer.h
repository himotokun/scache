#ifndef SLOTIZER_H
#define SLOTIZER_H

#include "attrib.h"
#include "motive.h"

class Slotizer{
	size_t slotsCount;
	time_t slotLength;
	double compressionQuot;

	size_t maxSegments;
	double minPrecision;
	
	time_t lastCall;
	size_t lastDivisor;
	MotiveTable* slots;

	size_t getSlot(time_t timestamp) const{
		return timestamp / slotLength % slotsCount;
	}
	bool isNewSlot(time_t time) const{
		return getSlot(lastCall) != getSlot(time);
	}
	void prepareNewSlot(time_t timestamp);
public:
	Slotizer();
	~Slotizer();

	// return true if motives have been recalculated
	bool AddRequest(const std::string& id,
		const TObjectAttr& attrib, size_t size, time_t timestamp,
		std::vector<TMotive>& resultMotives);

	void SetSlotLength(time_t length){ slotLength = length; }
	void SetSlotsCount(size_t count);
	void SetCompressionQuot(double quot){ compressionQuot = quot; }

	void SetMaxSegments(size_t _maxSeg){ maxSegments = _maxSeg;  }
	void SetMinPrecision(double _prec){ minPrecision = _prec; }
	void SetDepth(size_t depth);

	size_t GetDivisor() const{ return lastDivisor; }
	TPushArray GetPrePush(const TObjectAttr& attrib){
		return slots[getSlot(lastCall)].GetPrePush(attrib);
	}
};

#endif // SLOTIZER_H
