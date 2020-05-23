#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cstdio>
#include "table.h"

// WARNING! Spaces not supported in this version

struct TTimeRange: std::pair<time_t, time_t>{
	operator time_t() const{
		return first + rand() % (second - first + 1);
	}
	TTimeRange(time_t first = 0, time_t second = 0):
		std::pair<time_t, time_t>(first, second)
	{}
};

struct TSizeRange: std::pair<size_t, size_t>{
	operator size_t() const{
		return first + rand() % (second - first + 1);
	}
	TSizeRange(size_t first = 0, size_t second = 0):
		std::pair<size_t, size_t>(first, second)
	{}
};

struct TFloatRange: std::pair<double, double>{
	operator double() const{
		int intFirst = first * 100, intSecond = second * 100;
		return (intFirst + rand() % (intSecond - intFirst + 1)) / 100.0;
	}
	TFloatRange(double first = 0, double second = 0):
		std::pair<double, double>(first, second)
	{}
};

//typedef std::pair<time_t, time_t> TTimeRange;
//typedef std::pair<size_t, size_t> TSizeRange;
//typedef std::pair<double, double> TFloatRange;

struct TMotiveConfig{
	TAttributeSet attributes;
	TTimeRange period;
	TTimeRange shift;
	TTimeRange length;
	TTimeRange attack;
	TSizeRange volume;
	TFloatRange similar;
};

typedef std::vector<TMotiveConfig> TGeneratorConfig;

class CConfigReader{
	TGeneratorConfig currentConfig;
	FILE* configFile;

	static bool splitString(const std::string& str,
		std::vector<std::string>& result, char delimiter);
	static bool splitStringWithQuotes(const std::string& str,
		std::vector<std::string>& result, char delimiter);
	static std::string trimComment(const std::string& str);
	static size_t getTimeMultiplier(char c);
	static size_t getSizeMultiplier(char c);
	static TTimeRange parseTimeRange(const std::string& str);
	static TSizeRange parseSizeRange(const std::string& str);
	static TFloatRange parseFloatRange(const std::string& str);

	bool parseString();
	bool tryParseAttributes(const std::string& str);
	bool tryParseParam(const std::string& str);
	bool tryAddParam(const std::string& tag, const std::string& value);
public:
	TGeneratorConfig ReadFromFile(const char* fileName);
};
