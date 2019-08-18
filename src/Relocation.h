#ifndef _RELOCATION_
#define _RELOCATION_
#include <fstream>
#include <string>

using namespace std;
class Relocation
{
private:
	string section;
	int offset;
	string type;
	int reference;
public:
	Relocation(string section, int offset, string type, int reference);
	~Relocation();
	static string relocationType[];
	void print(ofstream &outputFile) {
		outputFile << dec << offset << '\t' << type << '\t' << reference << '\n';
	}
	string getSection() { return section; }
};




#endif // !_RELOCATION_

