#pragma once
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>
#include <iterator>

typedef std::pair<std::string, std::string> TAttribute;
typedef std::vector<TAttribute> TAttributeSet;

class CTableReader;

class CObjectTable{
	std::map<std::string, TAttributeSet> attributeTable;
	std::map<std::string, size_t> sizeTable;
	std::map<TAttributeSet, std::vector<std::string> > reverseCacheTable;

	std::map<TAttributeSet,
		std::vector<std::pair<std::string, size_t> > > filteredSizeTables;

	void doSelectObjectsByAttribSet(const TAttributeSet& attrib);
	void addFilter(const TAttributeSet& attrib);
public:
	CObjectTable(const char* fileName);
	void SetFilter(const TAttributeSet& attrib);
	void SetFilter();

//	std::vector<std::string> GetAllObjectIds();
	std::string GetRandomObject();
	std::string GetRandomObject(const TAttributeSet& attrib);
	std::string GetRandomObject(size_t maxSize);
	std::string GetRandomObject(const TAttributeSet& attrib, size_t maxSize);
//	const TAttributeSet& GetObjectAttributes(const std::string& id);
//	const TAttributeSet& operator[](const std::string& id){
//		return GetObjectAttributes(id);
//	}
//	std::vector<std::string> GetObjectsByAttr(const TAttributeSet& attrib);
//	std::vector<std::string> operator[](const TAttributeSet& attrib){
//		return GetObjectsByAttr(attrib);
//	}
	size_t GetObjectSize(const std::string& id){ return sizeTable[id]; }

	friend class CTableReader;
};

class CTableReader{
	FILE* f;
	CObjectTable& table;

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
	CTableReader(CObjectTable& _table):
		table(_table),
		line(0)
	{}
	void ReadFromFile(const std::string& fileName);
};
