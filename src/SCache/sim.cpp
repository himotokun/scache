#include <stdlib.h>
#include "sim.h"

bool Sim1::Open(std::string filename)
{
	f = fopen(filename.c_str(), "r");
	return f != 0;
}

bool Sim1::parseLine(TRequest& result)
{
	if(feof(f)){
		return false;
	}
	if(isFirst){
		firstCall = time(0);
		while(fgetc(f) != '\n')
			;
		isFirst = false;
	}
// REQTIME,STYPE,PID,CID,CDUR,
// CSIZE,CBITRATE,REQSIZEFORTHISTIME,DUROFREQFORTHISTIME

	int res = fscanf(f,
		"%ld,%*[^,],%lu,%[^,],%*[^,],%lf,%*[^,],%*[^,],%*s",
		&(result.timestamp), &(result.pid), result.id, &(result.size));
#if 0
	printf("id=%s, pid=%lu, size=%lf, time=%ld\n", result.id, result.pid,
		result.size, result.timestamp);
	if(res != 4){
		for(int i = 0; i < 10; i++){
			int c = fgetc(f);
			printf("%d [%c]\n", c, c);
		}
	}
#endif
	return res == 4; // argument count
}

bool Sim1::ReadLine()
{
	TRequest rq;
	if(!parseLine(rq)){
		return false;
	}
	if(cache != 0 && rq.pid == pid){
		cache->Get(std::string(rq.id), (size_t)rq.size, rq.timestamp);
	}
	rqCounter++;
	return true;
}

bool Sim2::Open(std::string filename)
{
	f = fopen(filename.c_str(), "r");
	return f != 0;
}

bool Sim2::parseLine(TRequest& result)
{
	if(feof(f)){
		return false;
	}
	if(isFirst){
		firstCall = time(0);
		/*while(fgetc(f) != '\n')
			;*/
		isFirst = false;
	}
	int res = fscanf(f, "%[^,],%lu,%lu\n", result.id,
		&(result.size), &(result.timestamp));
#if 0
	printf("id=%s, size=%lu, time=%lu\n", result.id,
		result.size, result.timestamp);
	if(res != 3){
		for(int i = 0; i < 10; i++){
			int c = fgetc(f);
			printf("%d [%c]\n", c, c);
		}
	}
#endif
	return res == 3; // argument count
}

bool Sim2::ReadLine()
{
	TRequest rq;
	if(!parseLine(rq)){
		return false;
	}
	if(cache != 0){
		cache->Get(std::string(rq.id), (size_t)rq.size, rq.timestamp);
	}
	rqCounter++;
	return true;
}
