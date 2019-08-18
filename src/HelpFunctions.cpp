#include "HelpFunctions.h"
#include "TwoPassAssembler.h"

set<char> HelpFunctions::base10set;
set<char> HelpFunctions::base2set;
set<char> HelpFunctions::base8set;
set<char> HelpFunctions::base16set;


HelpFunctions::HelpFunctions()
{
	
	char seperators[] = { ' ', '\t', ',', '\n', '\r', '\r' };
	separatorsSet.insert(seperators, seperators + 6);
	char longseperator[] = { '+', '-' };
	longSeparatorSet.insert(longseperator, longseperator + 2);
	string memDirectives[] = { ".char", ".word", ".long", ".align", ".skip", ".end" };
	memDirectivesSet.insert(memDirectives, memDirectives + 6);
	string directives[] = { ".char", ".word", ".long", ".align", ".skip", ".end", ".public", ".extern" };
	directivesSet.insert(directives, directives + 8);
	string sections[] = { ".text", ".data", ".bss" };
	sectionsSet.insert(sections, sections + 3);
	string operations[] = { "int", "add", "sub", "mul", "div", "cmp", "and", "or", "not",
		"test", "ldr", "str", "call", "in", "out", "mov", "shr", "shl", "ldch", "ldcl", "ldc" };
	operationsSet.insert(operations, operations + 21);
	string conditions[] = { "eq", "ne", "gt", "ge", "lt", "le", "al" };
	conditionsSet.insert(conditions, conditions + 7);
	////////////////////////
	
	char base2[] = { '0', '1' };
	base2set.insert(base2, base2 + 2);
	char base8[] = { '0', '1', '2', '3', '4', '5', '6', '7' };
	base8set.insert(base8, base8 + 8);
	char base10[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	base10set.insert(base10, base10 + 10);
	char base16[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	base16set.insert(base16, base16 + 16);
	
}


HelpFunctions::~HelpFunctions()
{
}

string HelpFunctions::toLower(string str)
{
	string result = "";
	for (int i = 0; i < str.length(); i++) {
		if ((str[i] >= 'A') && (str[i] <= 'Z')) result += str[i] + 'a' - 'A';
		else result += str[i];
	}
	return result;
}

void HelpFunctions::throwError(string msg, bool assembly, int numOfLine)
{
	cout << msg << endl;
	if (assembly) cout << "Number of line: " << numOfLine << endl;
	system("pause");
}

bool HelpFunctions::isInSet(set<string> set, string str)
{
	return (set.find(str) != set.end());
}

bool HelpFunctions::isSeparator(set<char> separator, char c)
{
	return (separator.find(c) != separator.end());
}

bool HelpFunctions::isOperation(string str)
{
	string copy = toLower(str);
	int size = copy.length();
	
	if (copy[size - 1] == 's') {
		copy = copy.substr(0, size - 1);
		size--;
	}
	if (isInSet(operationsSet, copy))
		return true;
	
	if (size > 3) {
		string cond = copy.substr(size - 2, 2);
		string op = copy.substr(0, size - 2);

		if (isInSet(operationsSet, op) && isInSet(conditionsSet, cond)) return true;
	}

	return false;
}



bool HelpFunctions::isLabel(string str)
{
	return (str[str.size() - 1] == ':');
}

bool HelpFunctions::isComment(string str)
{
	return (str[0] == ';');
}

list<string> HelpFunctions::lineToWords(string line, set<char> separator)
{
	list<string> words;
	string word;
	int start, end;
	
	while (line.size() > 0) {
		for (start = 0; start < line.size() && isSeparator(separator, line[start]); start++);
		for (end = start; end < line.size() && !isSeparator(separator, line[end]); end++);
		
		word = line.substr(start, end - start);
		if (word != "") words.push_back(word);
		line = line.substr(end);
	}
	return words;
}

Kind HelpFunctions::decodeWord(string word)
{
	int pos = word.find_last_of('.');
	string sect = word;
	if (pos) sect = sect.substr(0, pos);
	if (isOperation(word)) return Operation;
	if (isInSet(directivesSet, word)) return Directive;
	if (isInSet(sectionsSet, sect)) return Section;
	if (isLabel(word)) return Label;
	if (isComment(word)) return Comment;
	return Error;
}

string HelpFunctions::getSectionType(string name)
{
	name = HelpFunctions::toLower(name);

	if (isSectionType(".text", name)) return ".text";
	if (isSectionType(".data", name)) return ".data";
	if (isSectionType(".bss", name)) return ".bss";

	return "";
}

bool HelpFunctions::isSectionType(string type, string name)
{
	size_t pos = name.find(type);
	if (pos != -1) {
		name = name.substr(type.length());
		if (name[0] != '.' && name.length()) return false;
		return true;
	}
	return false;
}

int HelpFunctions::stringToInt(string number, int bitLength, bool signedInt)
{
	string number2;
	number = toLower(number);
	if (number.length() <= 2 ) {
		if (number[0] == '0' && number.size() > 1 ) {
			number = number.substr(1, number.length());
			if (isBase(base8set, number)) return baseFactToInt(8, number, bitLength, signedInt);
			throwError("Invalid number format!", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
		if (number[0] == '-') number2 = number.substr(1, number.length());
		if (isBase(base10set, number2))  return base10ToInt(number, bitLength, signedInt);
		throwError("Invalid number format!", true, TwoPassAssembler::numOfLine);
		exit(2);
	}
	else {
		if (number[0] == '0') {
			if (number[1] == 'b') {
				number = number.substr(2, number.length());
				if (isBase(base2set, number)) return baseFactToInt(2, number, bitLength, signedInt);
				throwError("Invalid number format!", true, TwoPassAssembler::numOfLine);
				exit(2);
			}
			if (number[1] == 'x') {
				number = number.substr(2, number.length());
				if (isBase(base16set, number)) return baseFactToInt(16, number, bitLength, signedInt);
				throwError("Invalid number format!", true, TwoPassAssembler::numOfLine);
				exit(2);
			}
			number = number.substr(1, number.length());
			if (isBase(base8set, number)) return baseFactToInt(8, number, bitLength, signedInt);
			throwError("Invalid number format!", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
		if (number[0] == '-') number2 = number.substr(1, number.length());
		if (isBase(base10set, number2)) return base10ToInt(number, bitLength, signedInt);
		throwError("Invalid number format!", true, TwoPassAssembler::numOfLine);
		exit(2);
	}
}

bool HelpFunctions::isBase(set<char> base, string number)
{
	for (int i = 0; i < number.length(); i++) {
		if (base.find(number[i]) != base.end()) continue;
		return false;
	}
	return true;
}

int HelpFunctions::baseFactToInt(int fact, string number, int bitLength, bool signedInt)
{
	int f = 1, digit;
	long num = 0;
	int max_unsigned = (1 << bitLength) - 1;
	int max_signed = (1 << (bitLength - 1)) - 1;
	int min_signed = -max_signed - 1;


	while (number.length() > 0) {
		char c = number[number.length() - 1];
		if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
		else digit = c - '0';
		num += digit * f;
		f *= fact;
		number = number.substr(0, number.length() - 1);
	}
	if (signedInt) {
		if (num > max_signed) {
			if (num > max_unsigned) { throwError("Invalid number value!", true, TwoPassAssembler::numOfLine); exit(2); }
			num -= max_unsigned - 1;/////////////////////////////////
		}	
	}
	else {
		if (num > max_unsigned) { throwError("Invalid number value!", true, TwoPassAssembler::numOfLine); exit(2); }
	}
	return num;
}


int HelpFunctions::base10ToInt(string number, int bitLength, bool signedInt)
{
	bool sign = false;
	int f = 1, digit;
	long num = 0;
	unsigned int  max_unsigned = (1 << bitLength) - 1;
	int max_signed = (1 << (bitLength - 1)) - 1;
	int min_signed = -max_signed - 1;
	if (bitLength == 32) max_unsigned = INT_MAX;

	if (number[0] == '-') {
		if(!signedInt) { throwError("Invalid number format!", true, TwoPassAssembler::numOfLine); exit(2); }
		number = number.substr(1, number.length());
		sign = true;
	}

	while (number.length() > 0) {
		digit = number[number.length() - 1] - '0';
		num += digit * f;
		f *= 10;
		number = number.substr(0, number.length() - 1);
	}
	if (sign) {
		num *= -1;
		if (num < min_signed) { throwError("Invalid number value!", true, TwoPassAssembler::numOfLine); exit(2); }
	}
	if (signedInt && num > max_signed) throwError("Invalid number value!", true, TwoPassAssembler::numOfLine);
	if (!signedInt && num > max_unsigned) throwError("Invalid number value!", true, TwoPassAssembler::numOfLine);

	return num;
}

bool HelpFunctions::isNumber(string number)
{
	
	number = toLower(number);
	if (number.length() <= 2) {
		if (number[0] == '0') {
			number = number.substr(1, number.length());
			if (isBase(base8set, number)) return true;
			return false;
		}
		if (number[0] == '-') number = number.substr(1, number.length());
		if (isBase(base10set, number))  return true;
		return false;
	}
	else {
		if (number[0] == '0') {
			if (number[1] == 'b') {
				number = number.substr(2, number.length());
				if (isBase(base2set, number)) return true;
					return false;
			}
			if (number[1] == 'x') {
				number = number.substr(2, number.length());
				if (isBase(base16set, number)) return true;
					return false;
			}
			number = number.substr(1, number.length());
			if (isBase(base8set, number)) return true;
			return false;
		}
		if (number[0] == '-') number = number.substr(1, number.length());
		if (isBase(base10set, number)) return true;
		return false;
	}
}

bool HelpFunctions::isLongExpresssion(string expression, list<Symbols*> symbolSection)
{
	list<string> words = lineToWords(expression, longSeparatorSet);
	string word = "";
	char op = '+';
	bool first = true;
	bool oper = false;
	int i = 0;
	while (i < expression.size()) {
		if (expression[i] == '-' || expression[i] == '+') {
			if (oper && !first) return false;
			op = expression[i++];
			if (first) {
				first = false;
			}
			oper = true;
		}
		else {
			word = words.front();
			words.pop_front();
			if (!isNumber(word) && !isSymbol(word, symbolSection) && word != "*") return false;
			if (!oper && !first) return false;
			i += word.size();
			oper = false;
			if (first) first = false;
		}
	}
	return true;
}

bool HelpFunctions::isSymbol(string name, list<Symbols*> symbolsList)
{
	for (list<Symbols*>::iterator iter = symbolsList.begin(); iter != symbolsList.end(); iter++) {
		if ((*iter)->getName().compare(name) == 0) return true;
	}
	return false;
}