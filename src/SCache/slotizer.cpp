#include "slotizer.h"

Slotizer::Slotizer():
	slotsCount(0),
	slotLength(0),
	compressionQuot(0.9),
	maxSegments((size_t)-1),
	minPrecision(0.0),
	lastCall(0),
	slots(0)
{}

Slotizer::~Slotizer()
{
	if(slots != 0){
		delete[] slots;
	}
}

void Slotizer::prepareNewSlot(time_t timestamp)
{
	slots[getSlot(timestamp)].Compress(compressionQuot);
}

void Slotizer::SetDepth(size_t depth)
{
	for(size_t i = 0; i < slotsCount; i++){
		slots[i].SetDepth(depth);
	}
}

bool Slotizer::AddRequest(const std::string& id,
		const TObjectAttr& attrib, size_t size, time_t timestamp,
		std::vector<TMotive>& resultMotives)
{
	bool flag = false;
	if(lastCall != 0 && isNewSlot(timestamp) &&
			slots[getSlot(timestamp)].GetDivisor() > 0)
	{
		auto& slot = slots[getSlot(lastCall)];
		slot.Recalculate(resultMotives,
			maxSegments, minPrecision);
		lastDivisor = slot.GetDivisor();
		prepareNewSlot(timestamp);
		flag = true;
	}
	lastCall = timestamp;
	slots[getSlot(timestamp)].AddRequest(id, attrib, size);
	return flag;
}

void Slotizer::SetSlotsCount(size_t count)
{
	if(slots != 0){
		delete[] slots;
	}
	slots = new MotiveTable[slotsCount = count];
}
