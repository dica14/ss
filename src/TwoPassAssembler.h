#ifndef _TWO_PASS_ASSEMBLER_
#define _TWO_PASS_ASSEMBLER_
#include "HelpFunctions.h"
#include "Sections.h"
#include "VonNeuman32bit.h"
#include "Tables.h"
#include <fstream>
#include <iostream>
#include <list>

using namespace std;

class TwoPassAssembler
{
private:
	ifstream inputFileStream;
	ofstream outputFileStream;
	int passNo;
	Sections* currSection;
	
	
	bool endAsm;
public:
	static int numOfLine;
	static int locCnt;
	HelpFunctions help;
	VonNeuman32bit vonNeuman;
	Tables tables;
	TwoPassAssembler();
	~TwoPassAssembler();

	int getNumOfLine() { return numOfLine; }

	void firstPass(string inputFile);
	void firstLabelProcess(list<string> list);
	void firstDirectiveProcess(list<string> list);
	void firstOperationProcess(list<string> list);
	void firstSectionProcess(list<string> list);
	
	void secondPass(string inputFile, string outputFile);
	void secondLabelProcess(list<string> list);
	void secondDirectiveProcess(list<string> list);
	void secondOperationProcess(list<string> list);
	void secondSectionProcess(list<string> list);

	int addLongRelocation(string word, char op);
	void addSectionRelocation(char op);
};

#endif
