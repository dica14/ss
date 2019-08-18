#ifndef _SECTION_
#define _SECTION_

#include <string>
#include <vector>
using namespace std;
class Sections
{
private:
	string name;
	//int start;
	//int end;
	unsigned int size;
	string type;
	vector<unsigned char> sectionContent;

public:
	Sections(string name, string type);
	~Sections();
	vector<unsigned char> getContent() { return sectionContent; }
	string getName() { return name; }
	void setSize(unsigned int size) { this->size = size; }
	int getSize() { return size; }
	void setType(string type) { this->type = type; }
	string getType() { return type; }
	void vectorPush(unsigned char byte) { sectionContent.push_back(byte); }
	
};

#endif