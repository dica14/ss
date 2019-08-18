#include "Relocation.h"

string Relocation::relocationType[] = { "R_386_32L", "R_386_32H", "R_386_32_PLUS", "R_386_32_MINUS" };

Relocation::Relocation(string section, int offset, string type, int reference)
{
	this->section = section;
	this->offset = offset;
	this->type = type;
	this->reference = reference;
}

Relocation::~Relocation()
{
}
