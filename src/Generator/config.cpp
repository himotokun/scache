#include "config.h"

const char RangeDelimiter = '~';
const char ParamDelimiter = '=';
const char AttribBegin = '[';
const char AttribEnd = ']';
const char AttribSep = ',';
const char AttribEqu = '=';
const char CommentSym = '#';

TGeneratorConfig CConfigReader::ReadFromFile(const char* fileName)
{
	configFile = fopen(fileName, "r");
	if(!configFile){
		throw "Error: cannot open config file";
	}
	while(parseString())
		;
	fclose(configFile);
	return currentConfig;
}

// -----

size_t CConfigReader::getTimeMultiplier(char c)
{
	switch(c){
		case 's':
		case 'S':
			return 1;
		case 'm':
		case 'M':
			return 60;
		case 'h':
		case 'H':
			return 60*60;
		case 'd':
		case 'D':
			return 24*60*60;
		case 'w':
		case 'W':
			return 24*60*60*7;
		default:
			return 0;
	}
}

size_t CConfigReader::getSizeMultiplier(char c)
{
	switch(c){
		case 'k':
		case 'K':
			return 1;
		case 'm':
		case 'M':
			return 1024;
		case 'g':
		case 'G':
			return 1024*1024;
		case 't':
		case 'T':
			return 1024*1024*1024;
		default:
			return 0;
	}
}

TTimeRange CConfigReader::parseTimeRange(const std::string& str)
{
	std::vector<std::string> splitted;
	if(!splitString(str, splitted, RangeDelimiter)){
		time_t value = std::stof(str) * getTimeMultiplier(str.back());
		return TTimeRange(value, value);
	}
	TTimeRange result;
	result.first = std::stoi(splitted[0]) *
		getTimeMultiplier(splitted[0].back());
	result.second = std::stoi(splitted[1]) *
		getTimeMultiplier(splitted[1].back());
	return result;	
}

TSizeRange CConfigReader::parseSizeRange(const std::string& str)
{
	std::vector<std::string> splitted;
	if(!splitString(str, splitted, RangeDelimiter)){
		size_t value = std::stof(str) * getSizeMultiplier(str.back());
		return TSizeRange(value, value);
	}
	TSizeRange result;
	result.first = std::stof(splitted[0]) *
		getTimeMultiplier(splitted[0].back());
	result.second = std::stof(splitted[1]) *
		getTimeMultiplier(splitted[1].back());
	return result;	
}

TFloatRange CConfigReader::parseFloatRange(const std::string& str)
{
	std::vector<std::string> splitted;
	if(!splitString(str, splitted, RangeDelimiter)){
		double value = std::stof(str);
		return TFloatRange(value, value);
	}
	return TFloatRange(std::stof(splitted[0]), std::stof(splitted[1]));
}

bool CConfigReader::parseString()
{
	constexpr size_t MaxStrLength = 256;
	char strBuf[MaxStrLength];
	if(fgets(strBuf, MaxStrLength, configFile) == 0){
		return false;
	}
	strBuf[strlen(strBuf) - 1] = '\0';
	//std::string str = trimComment(strBuf);
	if(tryParseAttributes(strBuf)){
		return true;
	}else if(tryParseParam(strBuf)){
		return true;
	}else{
		return false;
	}
}

bool CConfigReader::tryParseAttributes(const std::string& str)
{
	TAttributeSet result;
	if(str.front() != AttribBegin || str.back() != AttribEnd){
		return false;
	}
	std::string trimmedStr = str.substr(1, str.length() - 2);
	if(trimmedStr.length() == 0){
		currentConfig.push_back(TMotiveConfig());
		currentConfig.back().attributes = result;
		return true;
	}
	std::vector<std::string> splitted;
	splitStringWithQuotes(trimmedStr, splitted, AttribSep);
	for(const auto& attrib: splitted){
		std::vector<std::string> currentPair;
		if(!splitString(attrib, currentPair, AttribEqu)){
			return false;
		}
		if(currentPair.size() != 2){
			return false;
		}
		result.push_back(TAttribute(currentPair[0], currentPair[1]));
	}
	currentConfig.push_back(TMotiveConfig());
	currentConfig.back().attributes = result;
	return true;
}

bool CConfigReader::tryParseParam(const std::string& str)
{
	std::vector<std::string> result;
	if(!splitString(str, result, ParamDelimiter)){
		return false;
	}
	return tryAddParam(result[0], result[1]);
}

bool CConfigReader::tryAddParam(const std::string& tag,
		const std::string& value)
{
	if(tag == "period"){
		currentConfig.back().period = parseTimeRange(value);
	}else if(tag == "shift"){
		currentConfig.back().shift = parseTimeRange(value);
	}else if(tag == "length"){
		currentConfig.back().length = parseTimeRange(value);
	}else if(tag == "attack"){
		currentConfig.back().attack = parseTimeRange(value);
	}else if(tag == "volume"){
		currentConfig.back().volume = parseSizeRange(value);
	}else if(tag == "similar"){
		return false; // not implemented yet
		currentConfig.back().similar = parseFloatRange(value);
	}else{
		return false;
	}
	return true;
}

bool CConfigReader::splitString(const std::string& str,
	std::vector<std::string>& result, char delimiter)
{
	result.clear();
	result.push_back("");
	for(size_t i = 0; i < str.length(); i++){
		if(str[i] == delimiter){
			result.push_back("");
		}else{
			result.back() += str[i];
		}
	}
	return result.size() > 1;
}

bool CConfigReader::splitStringWithQuotes(const std::string& str,
	std::vector<std::string>& result, char delimiter)
{
	result.clear();
	result.push_back("");
	char quoteMode = 0; // contains ' or " if enabled
	for(size_t i = 0; i < str.length(); i++){
		if(str[i] == '\'' || str[i] == '"'){
			if(quoteMode){
				if(str[i] == quoteMode){
					quoteMode = 0;
				}else{
					result.back() += str[i];
				}
			}else{
				quoteMode = str[i];
			}
		}else if(str[i] == delimiter && !quoteMode){
			result.push_back("");
		}else{
			result.back() += str[i];
		}
	}
	return result.size() > 1;
}

std::string CConfigReader::trimComment(const std::string& str)
{
	std::vector<std::string> result;
	splitString(str, result, CommentSym);
	return result[0];
}
