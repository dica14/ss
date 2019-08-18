#include "Tables.h"
#include "TwoPassAssembler.h"


Tables::Tables()
{
	numSymbol = 0;
	Symbols* sym = new Symbols(0, "UND", "UND", false, true);
	addToSymbolList(sym);
	Sections *sec = new Sections("UND", "UND");
	addToSectionList(sec);
}


Tables::~Tables()
{
}

bool Tables::addToSymbolList(Symbols* sym)
{
	for (list<Symbols*>::iterator iter = symbolsList.begin(); iter != symbolsList.end(); ++iter) {
		if ((*iter)->getName().compare(sym->getName()) == 0) {
			if ((*iter)->getSection().compare("UND") == 0) {
				(*iter)->setSection(sym->getSection());
				(*iter)->setValue(sym->getValue());
				return true;
			}
			HelpFunctions::throwError("Label is already defined!", true, TwoPassAssembler::numOfLine);
			exit(3);
		}
	}
	symbolsList.push_back(sym);
	return true;
}

void Tables::addToSectionList(Sections* sec)
{
	sectionsList.push_back(sec);
}

void Tables::addToRelocationList(Relocation * rel)
{
	relocationsList.push_back(rel);
}

bool Tables::setGlobalSymbol(string name)
{
	for (list<Symbols*>::iterator iter = symbolsList.begin(); iter != symbolsList.end(); ++iter) {
		if ((*iter)->getName().compare(name) == 0) {
			(*iter)->setGlobal(true);
			return true;
		}
	}
	return false;
}

bool Tables::getGlobalSymbol(string name)
{
	for (list<Symbols*>::iterator iter = symbolsList.begin(); iter != symbolsList.end(); ++iter) {
		if ((*iter)->getName().compare(name) == 0) {
			return (*iter)->getGlobal();
		}
	}
	HelpFunctions::throwError("Label is not defined!", true, TwoPassAssembler::numOfLine);
	exit(3);
}

Sections * Tables::getSectionByName(string name)
{
	for (list<Sections*>::iterator iter = sectionsList.begin(); iter != sectionsList.end(); ++iter) {
		if ((*iter)->getName().compare(name) == 0) return *iter;
	}
	return nullptr;
}

string Tables::getSymbolSectionType(string name)
{
	for (list<Symbols*>::iterator iter = symbolsList.begin(); iter != symbolsList.end(); ++iter) {
		if ((*iter)->getName().compare(name) == 0) return (*iter)->getSection();
	}
	return "";
}

int Tables::getSymbolValue(string name)
{
	for (list<Symbols*>::iterator iter = symbolsList.begin(); iter != symbolsList.end(); ++iter) {
		if ((*iter)->getName().compare(name) == 0) return (*iter)->getValue();
	}
	HelpFunctions::throwError("Symbol coludn't be found in symbol list!", true, TwoPassAssembler::numOfLine);
	exit(2);
}

int Tables::getSymbolNum(string name)
{
	for (list<Symbols*>::iterator iter = symbolsList.begin(); iter != symbolsList.end(); ++iter) {
		if ((*iter)->getName().compare(name) == 0) return (*iter)->getNum();
	}
	HelpFunctions::throwError("Symbol coludn't be found in symbol list!", true, TwoPassAssembler::numOfLine);
	exit(2);
}

int Tables::getSectionSize(string name)
{
	for (list<Sections*>::iterator iter = sectionsList.begin(); iter != sectionsList.end(); ++iter) {
		if ((*iter)->getName().compare(name) == 0) return (*iter)->getSize();
	}
	HelpFunctions::throwError("Section coludn't be found in section list!", true, TwoPassAssembler::numOfLine);
	exit(2);
}

void Tables::printSymbolTable(ofstream & outputFile)
{
	string headerSymTable = "NUM\tNAME\tSECTION\t\tGLOBAL\tVALUE\tSIZE";
	outputFile << "TABLE\n";

	outputFile.fill(' ');
	outputFile.width(7);
	outputFile << left << "NUM";

	outputFile.fill(' ');
	outputFile.width(15);
	outputFile << "NAME";

	outputFile.fill(' ');
	outputFile.width(10);
	outputFile << "SECTION";

	outputFile.fill(' ');
	outputFile.width(10);
	outputFile << "LOCAL";

	outputFile.fill(' ');
	outputFile.width(10);
	outputFile << "VALUE";


	outputFile.fill(' ');
	outputFile.width(15);
	outputFile << "SIZE" << '\n';

	for (list<Symbols*>::iterator iter = symbolsList.begin(); iter != symbolsList.end(); ++iter) {
		outputFile.fill(' ');
		outputFile.width(7);
		outputFile << left << (*iter)->getNum();

		outputFile.fill(' ');
		outputFile.width(15);
		outputFile << (*iter)->getName();

		outputFile.fill(' ');
		outputFile.width(10);
		int num = getSymbolNum((*iter)->getSection());
		outputFile << num;

		outputFile.fill(' ');
		outputFile.width(10);
		if ((*iter)->getGlobal())
			outputFile << "true";
		else
			outputFile << "false";

		outputFile.fill(' ');
		outputFile.width(10);
		outputFile << (*iter)->getValue();


		outputFile.fill(' ');
		outputFile.width(15);
		if (!(*iter)->getIsSection())
			outputFile << 0 << '\n';
		else {
			unsigned int size = getSectionSize((*iter)->getName());
			outputFile << size << '\n';

		}
	}
	outputFile.flush();

}

void Tables::printRelocation(ofstream & outputFile)
{
	int i = 0;
	for (list<Sections*>::iterator iter = sectionsList.begin(); iter != sectionsList.end(); ++iter)
	{
		if ((*iter)->getSize() == 0) continue;
		outputFile << '\n';
		outputFile << "#rel" << (*iter)->getName() << "\n";
		outputFile << "OFFSET\tTYPE\t\tREFF\n";
		for (list<Relocation*>::iterator relocation = relocationsList.begin(); relocation != relocationsList.end(); ++relocation)
			if ((*relocation)->getSection().compare((*iter)->getName()) == 0) (*relocation)->print(outputFile);
			
	}
}

void Tables::printSections(ofstream & outputFile)
{
	int i;
	outputFile << right;
	for (list<Sections*>::iterator iter = sectionsList.begin(); iter != sectionsList.end(); ++iter)
	{
		if ((*iter)->getType() == ".bss" || (*iter)->getType() == "UND") continue;
		outputFile << '\n';
		outputFile << "#" << (*iter)->getName()<< "\n";
		i = 0;
		vector<unsigned char> content = (*iter)->getContent();
		for (vector<unsigned char>::iterator byte = content.begin(); byte != content.end(); ++byte)
		{
			unsigned char b = (*byte);
			i++;
			outputFile.width(2);
			outputFile.fill('0');
			outputFile << hex << (int)(b) << " ";
			if (i % 12 == 0)
				outputFile << '\n';
		}
		outputFile << "\n";
	}
	outputFile.flush();
}

