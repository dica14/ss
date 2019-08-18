#include "TwoPassAssembler.h"

int TwoPassAssembler::locCnt = 0;
int TwoPassAssembler::numOfLine = 0;

TwoPassAssembler::TwoPassAssembler()
{
	vonNeuman.tables = &tables;
	//help.tables = &tables;
	currSection = nullptr;
	numOfLine = 0;
	
}

TwoPassAssembler::~TwoPassAssembler()
{
}


void TwoPassAssembler::firstPass(string inputFile)
{
	inputFileStream.open(inputFile, ios::in);
	if (!inputFileStream.is_open()) {
		help.throwError("Couldn't find input file with name of first argument in command line", false, 0);
		exit(1);
	}

	//start first pass

	endAsm = false;
	string line;
	
	while (!endAsm && !inputFileStream.eof()) {
		getline(inputFileStream, line);
		numOfLine++;
		list<string> words = help.lineToWords(line, help.separatorsSet);
		if (words.empty()) continue;
		Kind kind = help.decodeWord(words.front());
		switch (kind)
		{
		case Label: firstLabelProcess(words);
			break;
		case Directive: firstDirectiveProcess(words);
			break;
		case Operation: firstOperationProcess(words);
			break;
		case Section: firstSectionProcess(words);
			break;
		case Comment: 
			break;
		case Error: help.throwError("Unresolved line: ", true, numOfLine);
			exit(2);
		default:
			break;
		}

	}
	locCnt = 0;
	numOfLine = 0;

	inputFileStream.close();
}

void TwoPassAssembler::firstLabelProcess(list<string> lists)
{
	string secName = "UND";
	if (currSection != nullptr) secName = currSection->getName();
	string labName = lists.front();
	labName = labName.substr(0, labName.length() - 1);
	Symbols* sym =  new Symbols(++tables.numSymbol, labName, secName, false, false, locCnt);
	tables.addToSymbolList(sym);

	lists.pop_front();
	if (lists.empty()) return;

	Kind kind = help.decodeWord(lists.front());
	switch (kind) {
	case Operation: firstOperationProcess(lists);
		break;
	case Directive: firstDirectiveProcess(lists);
		break;
	case Label: help.throwError("You can't write two labels in one line!", true, numOfLine);
		exit(2);
	case Comment:
		break;
	default: help.throwError("Error!", true, numOfLine);
		exit(2);
	}
}

void TwoPassAssembler::firstDirectiveProcess(list<string> lists)
{
	string dirName = help.toLower(lists.front());
	lists.pop_front();

	int size = lists.size();

	if (!currSection && help.isInSet(help.memDirectivesSet, dirName)) {
		help.throwError("Memory directive must be inside some section!", true, numOfLine);
		exit(2);
	}

	if (dirName.compare(".end") == 0) {
		endAsm = true;
		if (currSection) currSection->setSize(locCnt);
		return;
	}

	if (dirName.compare(".public") == 0) {
		if (!size) {
			help.throwError("Directive .public must have at least one parametar!", true, numOfLine);
			exit(2);
		}
		for (int i = 0; i < size; i++) {
			string name = lists.front();
			lists.pop_front();
			if (!tables.setGlobalSymbol(name)) {
				Symbols* sym = new Symbols(++tables.numSymbol, name, "UND", true, false);
				tables.addToSymbolList(sym);
			}
		}
	}

	if (dirName.compare(".extern") == 0) {
		if (!size) {
			help.throwError("Directive .extern must have at least one parametar!", true, numOfLine);
			exit(2);
		}
		for (int i = 0; i < size; i++) {
			string name = lists.front();
			lists.pop_front();
			if (!tables.setGlobalSymbol(name)) {
				Symbols* sym = new Symbols(++tables.numSymbol, name, "UND", true, false);
				tables.addToSymbolList(sym);
			}
			else {
				help.throwError("Parametar of directive .extern can't be defined in this file!", true, numOfLine);
				exit(2);
			}
		}
	}

	if ((size >= 1 && size <= 3) && dirName.compare(".align") == 0) {
		unsigned int alignNum, cnt, max;
		alignNum = help.stringToInt(lists.front(), 32, false);
		lists.pop_front();
		switch (size){
		case 1: 
		case 2: locCnt += alignNum - locCnt % alignNum;
			break;
		case 3: {
			lists.pop_front();
			max = help.stringToInt(lists.front(), 32, false);
			cnt = alignNum - locCnt % alignNum;
			if (cnt <= max) locCnt += cnt;
		}
			break;
		}
	}
	int size2 = 0;
	if (!size) size2 = 1;
	if (dirName.compare(".char") == 0) locCnt += size * 1 + size2 * 1;
	else if (dirName.compare(".word") == 0) locCnt += size * 2 + size2 * 2;
	else if (dirName.compare(".long") == 0) locCnt += size * 4 + size2 * 4;
	else if (dirName.compare(".skip") == 0) {
		if (size) {
			int num = help.stringToInt(lists.front(), 8, false);
			locCnt += num;
		}
		else locCnt += size2 * 1;
	}
}

void TwoPassAssembler::firstOperationProcess(list<string> lists)
{
	if (!currSection || currSection->getType().compare(".text") != 0) {
		help.throwError("Operation must be in .text section!", true, numOfLine);
		exit(2);
	}
	locCnt += vonNeuman.getOpLength(lists.front());
}

void TwoPassAssembler::firstSectionProcess(list<string> lists)
{
	if (currSection != nullptr) {
		currSection->setSize(locCnt);
	}

	locCnt = 0;
	string type = help.getSectionType(lists.front());
	// section name is only thing in line
	string name = lists.front();
	currSection = new Sections(name, type);
	Symbols* sym = new Symbols(++tables.numSymbol, name, name, false, true);
	tables.addToSymbolList(sym);
	tables.addToSectionList(currSection);
	//ubaci u listu sekcija i simbola
}

void TwoPassAssembler::secondPass(string inputFile, string outputFile)
{
	inputFileStream.open(inputFile, ios::in);
	if (!inputFileStream.is_open()) {
		help.throwError("Couldn't find input file with name of first argument in command line", false, 0);
		exit(1);
	}

	//start second pass

	endAsm = false;
	string line;

	while (!endAsm && !inputFileStream.eof()) {
		getline(inputFileStream, line);
		numOfLine++;
		list<string> words = help.lineToWords(line, help.separatorsSet);
		if (words.empty()) continue;
		Kind kind = help.decodeWord(words.front());
		switch (kind)
		{
		case Label: secondLabelProcess(words);
			break;
		case Directive: secondDirectiveProcess(words);
			break;
		case Operation: secondOperationProcess(words);
			break;
		case Section: secondSectionProcess(words);
			break;
		case Comment:
			break;
		case Error: help.throwError("Unresolved line: ", true, numOfLine);
			exit(2);
		default:
			break;
		}

	}
	inputFileStream.close();

	outputFileStream.open(outputFile, ios::out);

	tables.printSymbolTable(outputFileStream);
	tables.printRelocation(outputFileStream);
	tables.printSections(outputFileStream);
	outputFileStream.close();
}

void TwoPassAssembler::secondLabelProcess(list<string> lists)
{
	lists.pop_front();
	if (lists.empty()) return;
	Kind kind = help.decodeWord(lists.front());
	switch (kind)
	{
	case Directive: secondDirectiveProcess(lists);
		break;
	case Operation: secondOperationProcess(lists);
		break;
	}
}

void TwoPassAssembler::secondDirectiveProcess(list<string> lists)
{
	if (lists.front().compare(".end") == 0) {
		endAsm = true;
		if (!currSection) currSection->setSize(locCnt);
		currSection = nullptr;
		return;
	}

	if (!currSection && currSection->getType().compare(".bss") == 0) return;

	string dirName = help.toLower(lists.front());
	lists.pop_front();
	int size = lists.size();
	
	if ((size >= 1 && size <= 3) && dirName.compare(".align") == 0) {
		unsigned int alignNum, cnt, max;
		unsigned char fill;
		alignNum = help.stringToInt(lists.front(), 32, false);
		lists.pop_front();
		switch (size) {
			case 1:
			case 2: {
				cnt = alignNum - locCnt % alignNum;
				fill = help.stringToInt(lists.front(), 8, false);
				for (int i = 0; i < cnt; i++) {
					if (size == 1) currSection->vectorPush(0);
					if (size == 2) currSection->vectorPush(fill);
				}
				locCnt += cnt;
			}

					break;
			case 3: {
				cnt = alignNum - locCnt % alignNum;
				fill = help.stringToInt(lists.front(), 8, false);
				lists.pop_front();
				max = help.stringToInt(lists.front(), 32, false);
				if (cnt <= max) {
					for (int i = 0; i < cnt; i++) currSection->vectorPush(fill);
					locCnt += cnt;
				}
			}
					break;
		}
		return;
	}

	if (dirName.compare(".extern") == 0) {
		for (int i = 0; i < size; i++) {
			string name = lists.front();
			lists.pop_front();
			string sectionType = tables.getSymbolSectionType(name);
			if (sectionType.compare("UND") != 0) { help.throwError("Parameters in .extern directive can't be defined in file!", true, numOfLine); exit(2); }
		}
		return;
	}

	if (dirName.compare(".public") == 0) {
		for (int i = 0; i < size; i++) {
			string name = lists.front();
			lists.pop_front();
			string sectionType = tables.getSymbolSectionType(name);
			if (sectionType.compare("UND") == 0) { help.throwError("Parameters in .public directive must be defined in file!", true, numOfLine); exit(2); }
		}
		return;
	}
	int fill = 0;
	if (dirName.compare(".char") == 0) {
		if (!size) {
			currSection->vectorPush(0);
			locCnt++;
		}
		else {
			for (int i = 0; i < size; i++) {
				if (lists.front().compare(";") == 0) break;
				fill = help.stringToInt(lists.front(), 8, true);
				lists.pop_front();

				signed char byte = fill & 0xff;
				currSection->vectorPush(byte);
				locCnt++;
			}
		}
		return;
	}

	if (dirName.compare(".word") == 0) {
		if (!size) {
			currSection->vectorPush(0);
			currSection->vectorPush(0);
			locCnt += 2;
		}
		else {
			for (int i = 0; i < size; i++) {
				if (lists.front().compare(";") == 0) break;
				fill = help.stringToInt(lists.front(), 16, true);
				lists.pop_front();

				signed char byte0 = fill & 0xff;
				signed char byte1 = (fill >> 8) & 0xff;
				currSection->vectorPush(byte0);
				currSection->vectorPush(byte1);
				locCnt += 2;
			}
		}
		return;
	}

	if (dirName.compare(".long") == 0) {
		if (!size) {
			currSection->vectorPush(0);
			currSection->vectorPush(0);
			currSection->vectorPush(0);
			currSection->vectorPush(0);
			locCnt += 4;
		}
		else {
			for (int i = 0; i < size; i++) {
				if (lists.front().compare(";") == 0) break;
				if (help.isNumber(lists.front())) {
					fill = help.stringToInt(lists.front(), 32, true);
					lists.pop_front();

					signed char byte0 = fill & 0xff;
					signed char byte1 = (fill >> 8) & 0xff;
					signed char byte2 = (fill >> 16) & 0xff;
					signed char byte3 = (fill >> 24) & 0xff;
					currSection->vectorPush(byte0);
					currSection->vectorPush(byte1);
					currSection->vectorPush(byte2);
					currSection->vectorPush(byte3);
					locCnt += 4;
				}
				if (help.isLongExpresssion(lists.front(), tables.getSymbolsList())) {
					fill = 0;
					string expression = lists.front();
					lists.pop_front();
					list<string> words = help.lineToWords(expression, help.longSeparatorSet);
					string word = "";
					char op = '+';
					bool first = true;
					bool oper = false;
					int i = 0;
					while (i < expression.size()) {
						if (expression[i] == '-' || expression[i] == '+') {
							op = expression[i++];
							if (first) {
								first = false;
							}
							oper = true;
						}
						else {
							int value = 0;
							word = words.front();
							words.pop_front();
							if (help.isNumber(word)) {
								 value = help.stringToInt(word, 32, true);
							}
							else if (help.isSymbol(word, tables.getSymbolsList())){
								 value = addLongRelocation(word, op);	
							}
							else addSectionRelocation(op);
							if (op = '+') fill += value;
							else fill -= value;
							i += word.size();
							oper = false;
							if (first) first = false;
						}
					}
					signed char byte0 = fill & 0xff;
					signed char byte1 = (fill >> 8) & 0xff;
					signed char byte2 = (fill >> 16) & 0xff;
					signed char byte3 = (fill >> 24) & 0xff;
					currSection->vectorPush(byte0);
					currSection->vectorPush(byte1);
					currSection->vectorPush(byte2);
					currSection->vectorPush(byte3);
					locCnt += 4;
				}
			}
		}
		return;
}

	if (dirName.compare(".skip") == 0) {
		if (size == 1) {
			int cnt = help.stringToInt(lists.front(), 32, false);
			for (int i = 0; i < cnt; i++) currSection->vectorPush(0);
			locCnt += cnt;
		}
		else {
			locCnt++;
			currSection->vectorPush(0);
		}
		return;
	}

}

void TwoPassAssembler::secondOperationProcess(list<string> lists)
{
	vector<unsigned char> opcode = vonNeuman.decodeOperation(lists);
	for (int i = 0; i < opcode.size(); i++) currSection->vectorPush(opcode[i]);
	string opName = vonNeuman.getRealOpName(lists.front());
	locCnt += vonNeuman.getOpLength(opName);
}

void TwoPassAssembler::secondSectionProcess(list<string> lists)
{
	if (currSection) currSection->setSize(locCnt);
	currSection = tables.getSectionByName(lists.front());
	vonNeuman.currSection = currSection;
	locCnt = 0;
}

int TwoPassAssembler::addLongRelocation(string word, char op)
{
	bool global = tables.getGlobalSymbol(word);
	int returnVal, reference;
	string sectionDefined, type;
	if (global) {
		reference = tables.getSymbolNum(word);
		returnVal = 0;
	}
	else {
		sectionDefined = tables.getSymbolSectionType(word);
		reference = tables.getSymbolNum(sectionDefined);
		returnVal = tables.getSymbolValue(word);
	}
	if (op == '+') type = Relocation::relocationType[2];
	else type = Relocation::relocationType[3];

	Relocation* rel = new Relocation(currSection->getName(), locCnt, type, reference);

	tables.addToRelocationList(rel);

	return returnVal;
}

void TwoPassAssembler::addSectionRelocation(char op)
{
	string type;
	if (op == '+') type = Relocation::relocationType[2];
	else type = Relocation::relocationType[3];
	int reference = tables.getSymbolNum(currSection->getName());
	Relocation* rel = new Relocation(currSection->getName(), locCnt, type, reference);
}


