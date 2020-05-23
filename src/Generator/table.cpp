#include "table.h"

static bool IsSubset(const TAttributeSet& haystack,
		const TAttributeSet& needle)
{
	for(const auto& item: needle){
		if(std::find(haystack.begin(), haystack.end(),
			item) == haystack.end())
		{
			return false;
		}
	}
	return true;
}

CObjectTable::CObjectTable(const char* fileName)
{
	CTableReader reader(*this);
	reader.ReadFromFile(fileName);
	addFilter(TAttributeSet());
}

std::string CObjectTable::GetRandomObject()
{
	return std::next(sizeTable.begin(), rand() % sizeTable.size())->first;
}

std::string CObjectTable::GetRandomObject(const TAttributeSet& attrib)
{
	if(filteredSizeTables.count(attrib) == 0){
		addFilter(attrib);
	}
	auto& filteredSizeTable = filteredSizeTables[attrib];
	return std::next(filteredSizeTable.begin(), rand() % filteredSizeTable.
		size())->first;
}

std::string CObjectTable::GetRandomObject(size_t maxSize)
{
	return GetRandomObject(TAttributeSet(), maxSize);
}

std::string CObjectTable::GetRandomObject(const TAttributeSet& attrib,
	size_t maxSize)
{
	if(filteredSizeTables.count(attrib) == 0){
		addFilter(attrib);
	}
	auto& filteredSizeTable = filteredSizeTables[attrib];
	// Exclude the limit case
	if(filteredSizeTable.begin()->second > maxSize){
		return "";
	}
	// Apply the binary search
	size_t left = 0, right = filteredSizeTable.size() - 1;
	while(left < right){
		size_t idx = (left + right) / 2;
		if(std::next(filteredSizeTable.begin(), idx)->second < maxSize){
			left = idx + 1;
		}else{
			right = idx;
		}
	}
	// Get the object
	return std::next(filteredSizeTable.begin(),
		left == 0 ? 0 : rand() % left)->first;
}

// -------

void CObjectTable::doSelectObjectsByAttribSet(const TAttributeSet& attrib)
{
	reverseCacheTable[attrib] = std::vector<std::string>();
	for(const auto& item: attributeTable){
		if(IsSubset(item.second, attrib)){
			reverseCacheTable[attrib].push_back(item.first);
		}
	}
}

void CObjectTable::addFilter(const TAttributeSet& attrib)
{
	filteredSizeTables[attrib] =
		std::vector<std::pair<std::string, size_t> >();
	auto& newTable = filteredSizeTables[attrib];
	for(const auto& item: attributeTable){
		if(IsSubset(item.second, attrib) && sizeTable[item.first] > 0){
			newTable.push_back(
				std::make_pair(item.first, sizeTable[item.first]));
		}
	}
	std::sort(newTable.begin(), newTable.end(), [](auto& a, auto& b){
		return a.second < b.second;});
}

// CTableReader

void CTableReader::ReadFromFile(const std::string& fileName)
{
	f = fopen(fileName.c_str(), "r");
	if(!f){
		throw "Error: cannot open table file";
	}
	tryAcceptToken("{");
	while(!tryAcceptToken("}", false)){
		parseOne();
	}
	fclose(f);
}

void CTableReader::parseOne()
{
	auto id = getStringToken();
	table.attributeTable[id] = TAttributeSet();
	tryAcceptToken(":");
	tryAcceptToken("{");
	while(!tryAcceptToken("}", false)){
		auto tag = getStringToken();
		tryAcceptToken(":");
		if(tryAcceptToken("[", false)){
			while(!tryAcceptToken("]", false)){
				auto value = getStringToken();
				if(tag == "size"){
					table.sizeTable[id] = std::stoi(value);
				}else{
					table.attributeTable[id].
						push_back(std::make_pair(tag, value));
				}
				tryAcceptToken(",", false);
			}
		}else{
			auto value = getStringToken();
			if(tag == "size"){
				table.sizeTable[id] = std::stoi(value);
			}else{
				table.attributeTable[id].
					push_back(std::make_pair(tag, value));
			}
		}
		tryAcceptToken(",", false);
	}
	tryAcceptToken(",", false);
}

bool CTableReader::skipComments()
{
	int c = getChar();
	if(c == '#'){
		skipLine();
		return true;
	}else{
		ungetChar(c);
		return false;
	}
}

void CTableReader::skipLine()
{
	while(getChar() != '\n' && !isEof())
		;
}

bool CTableReader::skipSpace()
{
	int c;
	bool flag = false;
	while((c = getChar()) == ' ' || c == '\n' || c == '\t')
		flag = true;
	ungetChar(c);
	return flag;
}

std::string CTableReader::getStringToken()
{
	while( skipSpace() || skipComments())
		;
	int quoteMode = getChar();
	if(quoteMode != '\'' && quoteMode != '"'){
		goBack();
		//throw new Exception("quote mark expected at line " +
		throw "quote mark expected at line " +
			std::to_string(line) + ", '" + (char)quoteMode +
			"' got instead";
	}
	std::string result;
	int c;
	while((c = getChar()) != quoteMode && !isEof()){
		result += c;
	}
	return result;
}

int CTableReader::getChar()
{
	int c = fgetc(f);
	if(c == '\n'){
		line++;
		if(!(line % 100000)){
			printf(" %d", line);
			fflush(stdout);
		}
	}
	return c;
}

bool CTableReader::tryAcceptToken(const char* token, bool
	require /*= true */)
{
	assert(token);
	while( skipSpace() || skipComments())
		;
	int ptr = 0;
	const char* p = token;
	char c;
	// Go through the symbols
	while(token[ptr] && token[ptr] == (c = getChar())){
		ptr++;
	}
	// Unget symbols if token not accepted
	if(token[ptr]){
		goBack(ptr + 1);
	}
	if(require && token[ptr]){
		//throw new Exception(std::string("expected \'") + token +
		throw std::string("expected \'") + token +
				"\' at line " +
			std::to_string(line) + ", '" + c + "' got instead at pos " +
			std::to_string(ptr);
	}
	return token[ptr] == 0;
}

void CTableReader::ungetChar(int c)
{
	//ungetc(c, f);
	goBack(1);
	if(c == '\n'){
		line--;
	}
}

void CTableReader::goBack(int n/* = 1 */)
{
	fseek(f, -n, SEEK_CUR);
}
