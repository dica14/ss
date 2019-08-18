#ifndef _HELP_FUNC_
#define _HELP_FUNC_
#define INT_MAX 2147483647
#include "Symbols.h"
#include <string>
#include <iostream>
#include <list>
#include <set>
using namespace std;
enum Kind { Directive, Label, Comment, Section, Operation, Error };
enum Addr { MemDir, RegInd, RegPreInc, RegPosInc, RegPreDec, RegPosDec};
class HelpFunctions
{
private:
	
public:
	HelpFunctions();
	~HelpFunctions();
	
	 set<char> separatorsSet;
	 set<char> longSeparatorSet;
	 set<string> directivesSet;
	 set<string> sectionsSet;
	 set<string> operationsSet;
	 set<string> conditionsSet;
	 set<string> memDirectivesSet;
	 //////////////////////////
	 static set<char> base2set, base8set, base10set, base16set;
	 static string toLower(string str);
	 static void throwError(string msg, bool assembly, int numOfLine);
	 bool isInSet(set<string> set, string str);
	 bool isSeparator(set<char> separator,char c);
	 bool isOperation(string str);
	 bool isLabel(string str);
	 bool isComment(string str);
	 list<string> lineToWords(string line, set<char> separator);
	 Kind decodeWord(string word);
	 string getSectionType(string name);
	 bool isSectionType(string type, string name);
	 static int stringToInt(string number, int bitLength, bool signedInt);
	 static bool isBase(set<char> base, string number);
	 static int baseFactToInt(int fact, string number, int bitLength, bool signedInt);
	 static int base10ToInt(string number, int bitLength, bool signedInt);
	 static bool isNumber(string number);
	 bool isLongExpresssion(string expression, list<Symbols*> symbolSection);
	 bool isSymbol(string name, list<Symbols*> symbolsList);
};



#endif // !_HELP_FUNC_



