#ifndef _TABLES_
#define _TABLES_
#include "HelpFunctions.h"
#include "Sections.h"
#include "Relocation.h"
#include "Symbols.h"
#include <list>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
using namespace std;
class Tables
{
private:
	list<Sections*> sectionsList;
	list<Symbols*> symbolsList;
	list<Relocation*> relocationsList;
public:
	int numSymbol;
	Tables();
	~Tables();
	bool addToSymbolList(Symbols* sym);
	void addToSectionList(Sections* sec);
	void addToRelocationList(Relocation* rel);
	bool setGlobalSymbol(string name);
	bool getGlobalSymbol(string name);
	Sections* getSectionByName(string name);
	string getSymbolSectionType(string name);
	int getSymbolValue(string name);
	int getSymbolNum(string name);
	list<Symbols*> getSymbolsList() { return symbolsList; }
	int getSectionSize(string name);
	void printSymbolTable(ofstream& outputFile);
	void printRelocation(ofstream& outputFile);
	void printSections(ofstream& outputFile);

	
};



#endif // !_TABLES_


