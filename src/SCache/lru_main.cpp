#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exception.h"
#include "lru.h"
#include "sim.h"

const char* usage =
	"usage: lru [cache size] [dataset filename]";

void debug(const char* str)
{
	printf("%s", str);
	fflush(stdout);
}

void ok()
{
	printf("OK\n");
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

void clear()
{
	system("clear");
	system("clear");
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
		if(argc != 3){
			printf("%s\n", usage);
			return 1;
		}
		clear();
		printf("Welcome to SCache Simple LRU simulator v1.5!\n");
		time_t startTime = time(0);
		size_t size = ParseSize(argv[1]);
		debug("Initializing cache... ");
		LRU cache;
		cache.SetSize(size);
		ok();
		Sim2 sim(&cache);
		sim.Open(argv[2]);
		//sim.SetPid(atoi(argv[3]));
		size_t counter = 0, divisor = 10000;
		std::string logFile = std::string("log/LRU_") +
			clean_filename(argv[2]) + ".log";
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
				printf("Parsed %lu requests (%.3lf KRqs) "
					"[BHR: %.3lf] "
					"<used %.3lf%%>\n",
					counter, sim.GetSpeed()/1000.0,
					cache.GetBHR(),
					cache.GetUsedSpace()*100.0);
				fprintf(o, "(%lu;%lf) ", counter, cache.GetBHR());
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
