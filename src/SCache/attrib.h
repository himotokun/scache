#ifndef ATTRIB_H
#define ATTRIB_H

#include <map>
#include <vector>
#include <string>
#include <stdio.h>
#include "dp.h"

typedef std::pair<std::string, std::string> TValuePair;
typedef std::vector<TValuePair> TObjectAttr;
typedef std::map<std::string, TObjectAttr> TAttrTable;

class AttributeStorage{
protected:
	mutable TAttrTable table;
public:
	virtual void Open(std::string filename) = 0;
	TObjectAttr& operator[](const std::string& id){
		return table[id];
	}
	const TObjectAttr& operator[](const std::string& id) const{
		return table[id];
	}
	bool Has(const std::string& id) const{
		return table.find(id) != table.end();
	}
	
	// std::iterator support
	auto begin() { return table.begin(); }
	const auto begin() const { return table.begin(); }
	auto end() { return table.end(); }
	const auto end() const { return table.end(); }
};

// Class for JSON-prepared attribute lists

class MediaStorage: public AttributeStorage{
	FILE* f;
	int line;
	void parseOne();
	int getChar();
	void ungetChar(int c);
	bool isEof(){ return feof(f); }
	bool skipComments();
	void skipLine();
	std::string getStringToken();
	bool tryAcceptToken(const char* token, bool require = true);
	bool skipSpace();
	void goBack(int n = 1);
public:
	MediaStorage():
		line(1)
	{}
	MediaStorage(std::string filename):
		line(1)
	{
		Open(filename);
	}
	virtual void Open(std::string filename);
};

AttributeStorage& Storage();

#endif // ATTRIB_H
