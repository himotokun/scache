#include <assert.h>
#include <algorithm>
#include "attrib.h"
#include "exception.h"

static MediaStorage storage;

AttributeStorage& Storage()
{
	return storage;
}

//static std::vector<std::string> blacklist = {
//	"LANGUAGE", "PRICE", "COUNTRY", "TYPE"
//};

static std::vector<std::string> whitelist = {
	"GENRES"
};

// MediaStorage

void MediaStorage::Open(std::string filename)
{
	f = fopen(filename.c_str(), "r");
	if(!f){
		throw new Exception("cannot open file " + filename);
	}
	tryAcceptToken("{");
#if 0
	while(!tryAcceptToken("}", false) && line < 100000){
#else
	while(!tryAcceptToken("}", false)){
#endif
		parseOne();
	}
	fclose(f);
}

void MediaStorage::parseOne()
{
	auto id = getStringToken();
	table[id] = TObjectAttr();
	tryAcceptToken(":");
	tryAcceptToken("{");
	while(!tryAcceptToken("}", false)){
		auto tag = getStringToken();
		tryAcceptToken(":");
		if(tryAcceptToken("[", false)){
			while(!tryAcceptToken("]", false)){
				auto value = getStringToken();
				if(std::find(whitelist.begin(), whitelist.end(), tag) !=
						whitelist.end())
				{
					table[id].push_back(std::make_pair(tag, value));
				}
				tryAcceptToken(",", false);
			}
		}else{
			auto value = getStringToken();
			if(std::find(whitelist.begin(), whitelist.end(), tag) !=
					whitelist.end())
			{
				table[id].push_back(std::make_pair(tag, value));
			}
		}
		tryAcceptToken(",", false);
	}
	tryAcceptToken(",", false);
}

bool MediaStorage::skipComments()
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

void MediaStorage::skipLine()
{
	while(getChar() != '\n' && !isEof())
		;
}

bool MediaStorage::skipSpace()
{
	int c;
	bool flag = false;
	while((c = getChar()) == ' ' || c == '\n' || c == '\t')
		flag = true;
	ungetChar(c);
	return flag;
}

std::string MediaStorage::getStringToken()
{
	while( skipSpace() || skipComments())
		;
	int quoteMode = getChar();
	if(quoteMode != '\'' && quoteMode != '"'){
		goBack();
		throw new Exception("quote mark expected at line " +
			std::to_string(line) + ", '" + (char)quoteMode +
			"' got instead");
	}
	std::string result;
	int c;
	while((c = getChar()) != quoteMode && !isEof()){
		result += c;
	}
	return result;
}

int MediaStorage::getChar()
{
	int c = fgetc(f);
	if(c == '\n'){
		line++;
		if(!(line % 100000)){
			printf("%d ", line);
			fflush(stdout);
		}
	}
	return c;
}

bool MediaStorage::tryAcceptToken(const char* token, bool
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
		throw new Exception(std::string("expected \'") + token +
				"\' at line " +
			std::to_string(line) + ", '" + c + "' got instead at pos " +
			std::to_string(ptr));
	}
	return token[ptr] == 0;
}

void MediaStorage::ungetChar(int c)
{
	//ungetc(c, f);
	goBack(1);
	if(c == '\n'){
		line--;
	}
}

void MediaStorage::goBack(int n/* = 1 */)
{
	fseek(f, -n, SEEK_CUR);
}
