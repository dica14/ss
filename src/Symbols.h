#ifndef _SYMBOLS_
#define _SYMBOLS_

#include <string>
using namespace std;
class Symbols
{
private:
	int num;
	string name;
	string section;
	bool global;
	int size;
	int value;
	bool isSection;
public:
	Symbols(int num, string name, string section, bool global, bool isSection,int value = 0, int size = 0);
	~Symbols();

	void setGlobal(bool global);
	bool getGlobal() { return global; }
	string getName() { return name; }
	string getSection() { return section; }
	void setSection(string section) { this->section = section; }
	int getValue() { return value; }
	int getNum() { return num; }
	bool getIsSection() { return isSection; }
	void setValue(int value) { this->value = value; }
};
#endif // !_SYMBOLS_



