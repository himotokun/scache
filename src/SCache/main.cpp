#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exception.h"
#include "scache.h"
#include "lru.h"
#include "sim.h"

const char* usage =
	"usage: scache [cache size] [max segments] [min precision] [motive depth]"
	" [slot length (mins)] [slot count] [table filename] [dataset filename]";

void debug(const char* str)
{
	printf("%s", str);
	fflush(stdout);
}

void ok()
{
	printf("OK\n");
}

void OpenStorage(const char* file)
{
	debug("Opening attrib storage... ");
	Storage().Open(file);
	ok();
}

void clear()
{
	system("clear");
	system("clear");
}

struct TCacheParam{
	size_t size;
	size_t maxSegments;
	double minPrecision;
	size_t motiveDepth;
	size_t slotLength;
	size_t slotCount;
};

void SetCacheParams(SmartCache& cache, const TCacheParam& param)
{
	cache.SetSize(param.size);
	cache.SetSlotLength(60*param.slotLength);
	cache.SetSlotsCount(param.slotCount);
	cache.SetMaxSegments(param.maxSegments);
	cache.SetMinPrecision(param.minPrecision);
	cache.SetDepth(param.motiveDepth);
}

size_t ParseSize(const char* str)
{
	double mant = atof(str);
	size_t res;
	switch(str[strlen(str) - 1]){
		case 'M':
			res = mant * 1024;
			break;
		case 'G':
			res = mant * 1024 * 1024;
			break;
		case 'T':
			res = mant * 1024 * 1024 * 1024;
			break;
		default:
			printf("%s\n", usage);
			exit(1);
	}
	return res;
}

// Gets filename after all '/' (e.g. 'data/log/file.txt' -> 'file.txt')
std::string clean_filename(const char* file)
{
	const char* srcFile = file;
	int slashes = 0;
	while( slashes += *file == '/', *(file++) )
		;
	if(!slashes){
		return std::string(srcFile);
	}
	while(*(--file) != '/')
		;
	return std::string(file + 1);
}

int main(int argc, char **argv)
{
	try{
		if(argc != 9){
			printf("%s\n", usage);
			return 1;
		}
		clear();
		printf("Welcome to SCache simulator v2.7!\n");
		time_t startTime = time(0);
		TCacheParam param;
		param.size = ParseSize(argv[1]);
		param.maxSegments = atoi(argv[2]);
		param.minPrecision = atof(argv[3]);
		param.motiveDepth = atoi(argv[4]);
		param.slotLength = atoi(argv[5]);
		param.slotCount = atoi(argv[6]);
		OpenStorage(argv[7]);
		LRUFactory factory;
		debug("Initializing cache... ");
		SmartCache cache(&factory);
		SetCacheParams(cache, param);
		ok();
		Sim2 sim(&cache);
		sim.Open(argv[8]);
		//sim.SetPid(atoi(argv[8]));
		size_t counter = 0, divisor = 10000;
		std::string logFile = std::string("log/SCACHE_") +
			clean_filename(argv[8]) + ".log";
		FILE* o = fopen(logFile.c_str(), "ab");
		if(!o){
			printf("Cannot open file %s\n", logFile.c_str());
			exit(1);
		}
		for(int i = 0; i < argc; i++){
			fprintf(o, "%s ", argv[i]);
		}
		fprintf(o, "\n\n");
		while(sim.ReadLine()){
			if(++counter % divisor == 0){
				clear();
				printf("Parsed %lu requests (%.3lf KRqs) "
					"[BHR: %.3lf]\n",
					counter, sim.GetSpeed()/1000.0, cache.GetBHR());
				fprintf(o, "(%lu;%lf) ", counter, cache.GetBHR());
				cache.PrintSegmentState();
			}
		}
		fprintf(o, "\n\n");
		printf("All!\n");
		printf("Total BHR: %.3lf\n", cache.GetBHR());
		printf("Finished in %lu seconds\n", time(0) - startTime);
		fprintf(o, "Total BHR: %.3lf\n", cache.GetBHR());
		fprintf(o, "Finished in %lu seconds\n\n", time(0) - startTime);
		fclose(o);
	} catch(Exception* e){
		printf("%s\n", e->GetMessage().c_str());
		delete e;
	}
	return 0;
}
