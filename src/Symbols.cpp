#include "Symbols.h"


Symbols::Symbols(int num, string name, string section, bool global, bool isSection, int value, int size)
{
	this->num = num;
	this->name = name;
	this->section = section;
	this->global = global;
	this->size = size;
	this->value = value;
	this->isSection = isSection;
}

Symbols::~Symbols()
{
}

void Symbols::setGlobal(bool global)
{
	this->global = global;
}

