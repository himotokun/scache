#ifndef SIM_H
#define SIM_H

#include <time.h>
#include <stdio.h>
#include <string>
#include "scache.h"
#include "dp.h"

#define MAX_ID_LENGTH 16

class Sim1{
	ICache* cache;
	FILE* f;

	size_t rqCounter;
	time_t firstCall;
	size_t pid;

	bool isFirst;

	struct TRequest{
		char id[MAX_ID_LENGTH];
		size_t pid;
		double size;
		time_t timestamp;
	};
	bool parseLine(TRequest& result);
public:
	Sim1(ICache* _cache):
		cache(_cache),
		rqCounter(0),
		firstCall(0),
		f(0),
		isFirst(true)
	{
		SetPid(3);
	}
	~Sim1(){ if(f != 0){ fclose(f); } }
	bool Open(std::string filename);
	void SetPid(size_t _pid) { pid = _pid; }
	bool ReadLine();

	double GetSpeed() const{
		return ((double)rqCounter) / (time(0) - firstCall);
	}
};

class Sim2{
	ICache* cache;
	FILE* f;

	size_t rqCounter;
	time_t firstCall;

	bool isFirst;

	struct TRequest{
		char id[MAX_ID_LENGTH];
		size_t size;
		time_t timestamp;
	};
	bool parseLine(TRequest& result);
public:
	Sim2(ICache* _cache):
		cache(_cache),
		rqCounter(0),
		firstCall(0),
		f(0),
		isFirst(true)
	{}
	~Sim2(){ if(f != 0){ fclose(f); } }
	bool Open(std::string filename);
	bool ReadLine();

	double GetSpeed() const{
		return ((double)rqCounter) / (time(0) - firstCall);
	}
};
#endif // SIM_H
