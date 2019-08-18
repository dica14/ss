#ifndef _VON_NEUMAN_32BIT_
#define _VON_NEUMAN_32BIT_
#include "Tables.h"
#include "HelpFunctions.h"
#include "Sections.h"
#include <string>
#include <vector>
#include <list>
#include <set> 
using namespace std;

class VonNeuman32bit
{
private:
	set<string> artihmeticSet;
	set<string> logicSet;
	set<string> inOutSet;
	set<string> shiftSet;
	set<string> ldcSet;
	set<string> ldStSet;
	set<string> registerSet;
	set<string> specRegsSet;
public:
	Sections* currSection;
	Tables* tables;
	VonNeuman32bit();
	~VonNeuman32bit();
	int getOpLength(string op);
	string getRealOpName(string opname);
	bool isInSet(set<string> set, string str);
	int decodeCondition(string cond);
	vector<unsigned char> decodeOperation(list<string> list);
	vector<unsigned char> decodeArithmetic(list<string> list);
	vector<unsigned char> decodeLogic(list<string> list);
	vector<unsigned char> decodeInOut(list<string> list);
	vector<unsigned char> decodeInt(list<string> list);
	vector<unsigned char> decodeCall(list<string> list);
	vector<unsigned char> decodeLdc(list<string> list);
	vector<unsigned char> decodeMove(list<string> list);
	vector<unsigned char> decodeShift(list<string> list);
	vector<unsigned char> decodeLdSt(list<string> list);
	bool checkNumOfParams(list<string> list);
	int countParams(list<string> list);
	int addRelocationForLDC(string name, int location, bool low);
	Addr decodeAddressMode(string reg);
	string getRegisterName(string reg);
};

#endif