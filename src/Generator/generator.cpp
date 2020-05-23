#include "generator.h"
#define DEBUG(...) if(debugEnabled){ printf(__VA_ARGS__); fflush(stdout); }
#define DEBUG2(...) if(generator->debugEnabled){\
	printf(__VA_ARGS__); fflush(stdout); }

void CGenerator::Generate(const char* outputFileName,
	size_t totalRequestCount)
{
	initializeOutputFile(outputFileName);
	for(const auto& motive: config){
		addMotive(motive);
	}
	doGenerate(totalRequestCount);
	doClean();
}

// --------

void CGenerator::addMotive(const TMotiveConfig& config)
{
	CMotiveGenerator newMotive(this, config);
	currentState.push_back(newMotive);
}

void CGenerator::doGenerate(size_t totalCount)
{
	size_t motiveSize = currentState.size();
	size_t currentMotive = 0;
	std::vector<TRequest> currentSlotRequests;
	DEBUG("Requests left: %lu", totalCount);
	while(totalCount > 0){
		size_t generatedCount = currentState[currentMotive].
			GenerateRequestSet(currentSlotRequests);
		totalCount -=
			generatedCount < totalCount ? generatedCount : totalCount;
		++currentMotive %= motiveSize;
		// write down a slot and prepare for a new
		if(currentMotive == 0){
			writeRequestSet(currentSlotRequests);
			currentSlotRequests.clear();
			currentSlot++;
		}
		DEBUG(" %lu", totalCount);
	}
	DEBUG(" OK\n");
}

void CGenerator::initializeOutputFile(const char* fileName)
{
	outputFile = fopen(fileName, "w");
	if(!outputFile){
		throw "Error: cannot open file for writing\n";
	}
}

void CGenerator::doClean()
{
	fclose(outputFile);
}

void CGenerator::writeRequestSet(std::vector<TRequest>& requests)
{
	std::sort(requests.begin(), requests.end(),
		[](auto& a, auto& b){ return a.time < b.time; });
	for(const auto& r: requests){
		fprintf(outputFile, "%s,%lu,%lu\n", r.id.c_str(), r.size, r.time);
	}
}

// CGenerator::CMotiveGenerator

CGenerator::CMotiveGenerator::CMotiveGenerator(CGenerator* _generator,
		const TMotiveConfig& _config):
	generator(_generator),
	config(_config)
{
}

size_t CGenerator::CMotiveGenerator::GenerateRequestSet(
		std::vector<TRequest>& result)
{
	// Generates a full slot of requests
	size_t leftToGenerate = getSizeForCurrentSlot();
	size_t count = 0;
	//DEBUG2(" [generating set: %lu", leftToGenerate);
	while(leftToGenerate > 0){
		TRequest r;
		r.id = generator->table.GetRandomObject(config.attributes,
			leftToGenerate);
		if(r.id == ""){
			break;
		}
		r.size = generator->table.GetObjectSize(r.id);
		r.time = getRandomTime(getSlotBegin(), getSlotEnd());
		result.push_back(r);
		count++;
		leftToGenerate -= r.size < leftToGenerate ? r.size : leftToGenerate;
		//DEBUG2(" %lu", leftToGenerate);
		if(generator->resetFlag){
			fclose(generator->outputFile);
			throw "Process interrupted";
		}
	}
	//DEBUG2("]");
	return count;
}

time_t CGenerator::CMotiveGenerator::getSlotBegin()
{
	return generator->currentSlot * generator->discretRate;
}

time_t CGenerator::CMotiveGenerator::getSlotEnd()
{
	return (generator->currentSlot + 1) * generator->discretRate - 1;
}

size_t CGenerator::CMotiveGenerator::getRandomTime(time_t left, time_t right)
{
	return left + rand() % (right - left + 1);
}

// THE MOST DIFFICULT THING
// the only element that really uses the motive config
size_t CGenerator::CMotiveGenerator::getSizeForCurrentSlot()
{
	time_t begin = getSlotBegin();
	time_t end = getSlotEnd();

	time_t period = config.period;
	time_t shift = config.shift;
	time_t attack;
	time_t length;
	do{
		attack = config.attack;
		length = config.length;
	}while(attack >= length);

	time_t shiftFromIteration = begin % period;
	if(shiftFromIteration < shift){
		return 0;
	}
	shiftFromIteration -= shift;
	if(shiftFromIteration > length){
		return 0;
	}
	size_t relativeSlot = shiftFromIteration % generator->discretRate;
	size_t peakHeight = config.volume / (length - attack);
	size_t maxSlotVolume = generator->discretRate * peakHeight;
	if(shiftFromIteration >= attack &&
		shiftFromIteration <= length - attack)
	{
		return maxSlotVolume;
	}
	// mirror the shift because the image is symmetric
	if(shiftFromIteration > length - attack){
		shiftFromIteration = length - shiftFromIteration;
	}
	return generator->discretRate * peakHeight *
		(shiftFromIteration + generator->discretRate / 2) / attack;
}
