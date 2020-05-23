#pragma once

#include <vector>
#include <string>
#include "config.h"
#include "table.h"

struct TRequest{
	std::string id;
	size_t size;
	time_t time;
};


class CGenerator{
	TGeneratorConfig& config;
	CObjectTable& table;
	time_t discretRate;
	bool debugEnabled;
	volatile bool resetFlag;

	FILE* outputFile;
	size_t currentSlot;

	class CMotiveGenerator{
		CGenerator* generator;
		const TMotiveConfig& config;

		time_t getSlotBegin();
		time_t getSlotEnd();
		size_t getSizeForCurrentSlot();
		size_t getRandomTime(time_t left, time_t right);
	public:
		CMotiveGenerator(CGenerator* generator, const TMotiveConfig& config);
		size_t GenerateRequestSet(std::vector<TRequest>& result);
	};
	std::vector<CMotiveGenerator> currentState;

	void initializeOutputFile(const char* fileName);
	void doGenerate(size_t totalCount);
	void addMotive(const TMotiveConfig& config);
	void doClean();
	void writeRequestSet(std::vector<TRequest>& requests);

public:
	CGenerator(TGeneratorConfig& _config, CObjectTable& _table):
		config(_config), table(_table), currentSlot(0), debugEnabled(false),
		resetFlag(false)
	{}
	//void SetConfig(const TGeneratorConfig& _config) { config = _config; }
	//void SetTable(const CObjectTable& _table) { table = _table; }
	void SetDiscretRate(time_t rate) { discretRate = rate; }
	void Generate(const char* outputFileName, size_t totalRequestCount);
	void SetDebugEnabled(bool isEnabled = true){ debugEnabled = isEnabled; }
	void Reset(){ resetFlag = true; }
};
