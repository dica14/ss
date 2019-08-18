#include "VonNeuman32bit.h"
#include "TwoPassAssembler.h"



VonNeuman32bit::VonNeuman32bit()
{
	currSection = nullptr;
	string arithmetic[] = { "add", "sub", "mul", "div", "cmp" };
	artihmeticSet.insert(arithmetic, arithmetic + 5);
	string logic[] = { "and", "or", "not", "test" };
	logicSet.insert(logic, logic + 4);
	string inout[] = { "in", "out" };
	inOutSet.insert(inout, inout + 2);
	string ldc[] = { "ldch", "ldcl", "ldc" };
	ldcSet.insert(ldc, ldc + 3);
	string shift[] = { "shr", "shl" };
	shiftSet.insert(shift, shift + 2);
	string ldSt[] = { "ldr", "str" };
	ldStSet.insert(ldSt, ldSt + 2);
	string registers[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "pc", "lr", "sp", "psw" };
	registerSet.insert(registers, registers + 20);
	string specRegs[] = { "pc", "lr", "sp", "psw" };
	specRegsSet.insert(specRegs, specRegs + 4);
}

VonNeuman32bit::~VonNeuman32bit()
{
}

int VonNeuman32bit::getOpLength(string op)
{
	if (op.compare("ldc") == 0) return 8;
	else return 4;
}

string VonNeuman32bit::getRealOpName(string opname)
{
	opname = HelpFunctions::toLower(opname);
	int size = opname.length();

	if (opname[size - 1] == 's') {
		opname = opname.substr(0, size - 1);
		size--;
	}
	if (!opname.compare("test") || !opname.compare("call") || isInSet(ldcSet, opname)) return opname;
	if (size > 3) {
		opname =  opname.substr(0, size - 2);
	}
	return opname;
}
bool VonNeuman32bit::isInSet(set<string> set, string str)
{
	return (set.find(str) != set.end());
}

int VonNeuman32bit::decodeCondition(string cond)
{
	if (!cond.compare("eq")) return 0;
	if (!cond.compare("ne")) return 1;
	if (!cond.compare("gt")) return 2;
	if (!cond.compare("ge")) return 3;
	if (!cond.compare("lt")) return 4;
	if (!cond.compare("le")) return 5;
	if (!cond.compare("al")) return 7;
	return -1;
}

vector<unsigned char> VonNeuman32bit::decodeOperation(list<string> lists)
{
	bool status = checkNumOfParams(lists);
	
	if (!status) {
		HelpFunctions::throwError("Invalid number of parameters!", true, TwoPassAssembler::numOfLine); exit(2);
	}
	string opName = getRealOpName(lists.front());
	if (isInSet(artihmeticSet, opName)) return decodeArithmetic(lists);
	if (isInSet(logicSet, opName)) return decodeLogic(lists);
	if (isInSet(inOutSet, opName)) return decodeInOut(lists);
	if (isInSet(ldcSet, opName)) return decodeLdc(lists);
	if (isInSet(ldStSet, opName)) return decodeLdSt(lists);
	if (isInSet(shiftSet, opName)) return decodeShift(lists);
	if (opName.compare("int") == 0) return decodeInt(lists);
	if (opName.compare("call") == 0) return decodeCall(lists);
	if (opName.compare("mov") == 0) return decodeMove(lists);

	HelpFunctions::throwError("Invalid decode Operation!", true, TwoPassAssembler::numOfLine);
	exit(2);
	
}

vector<unsigned char> VonNeuman32bit::decodeArithmetic(list<string> lists)
{
	vector<unsigned char> bytes(4);
	int cond = 7;
	bool flag = false;
	string opName = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	for (int i = 0; i < 3; i++) bytes[i] = 0;

	if (opName[opName.size() - 1] == 's') {
		flag = true;
		opName = opName.substr(0, opName.size() - 1);
	}
	if (opName.compare("add") != 0 && opName.compare("sub") != 0 && opName.compare("mul") != 0 && opName.compare("div") != 0) {
		cond = decodeCondition(opName.substr(opName.size() - 2, 2));
		opName = opName.substr(0, opName.size() - 2);
		if (cond == -1) {
			HelpFunctions::throwError("Invalid condition in call function", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
	}
	bytes[0] |= ((cond & 0xff) << 5);
	if (flag) bytes[0] |= 0x10;
	if (opName.compare("add") == 0) bytes[0] |= 0x1;
	else if (opName.compare("sub") == 0) bytes[0] |= 0x2;
	else if (opName.compare("mul") == 0) bytes[0] |= 0x3;
	else if (opName.compare("div") == 0) bytes[0] |= 0x4;
	else if (opName.compare("cmp") == 0) bytes[0] |= 0x5;

	string operand1 = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	string operand2 = HelpFunctions::toLower(lists.front());

	if (!isInSet(registerSet, operand1)) {
		HelpFunctions::throwError("Destination must be register in arithmetics operations!", true, TwoPassAssembler::numOfLine);
		exit(2);
	}
	if (!operand1.compare("psw") || !operand2.compare("psw")) {
		HelpFunctions::throwError("Register PSW is not allowed as arithmetic operation parameter!", true, TwoPassAssembler::numOfLine);
		exit(2);
	}
	if ((isInSet(specRegsSet, operand1) || isInSet(specRegsSet, operand2)) && opName.compare("add") != 0 && opName.compare("sub") != 0) {
		HelpFunctions::throwError("Registers pc, lr and sp are allowed as parameters only in add and sub operations!", true, TwoPassAssembler::numOfLine);
		exit(2);
	}

	int dst;
	if (operand1.compare("pc") == 0) dst = 0x10;
	else if (operand1.compare("lr") == 0) dst = 0x11;
	else if (operand1.compare("sp") == 0) dst = 0x12;
	else dst = HelpFunctions::base10ToInt(operand1.substr(1, operand1.size() - 1), 8, false);
	bytes[1] = ((dst & 0xff) << 3);

	if (isInSet(registerSet, operand2)) {
		int src;
		if (operand2.compare("pc") == 0) src = 0x10;
		else if (operand2.compare("lr") == 0) src = 0x11;
		else if (operand2.compare("sp") == 0) src = 0x12;
		else src = HelpFunctions::base10ToInt(operand2.substr(1, operand2.size() - 1), 8, false);
		bytes[1] |= (src >> 3) & 0x3;
		bytes[2] = (src & 0x7) << 5;
	}
	else {
		bytes[1] |= 0x4;
		int imm = HelpFunctions::stringToInt(operand2, 18, true);
		
		bytes[1] |= (imm >> 16) & 0x3;
		bytes[2] = (imm >> 8) & 0xff;
		bytes[3] = imm & 0xff;
	}
	return bytes;
}

vector<unsigned char> VonNeuman32bit::decodeLogic(list<string> lists)
{
	vector<unsigned char> bytes(4);
	int cond = 7;
	bool flag = false;
	string opName = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	for (int i = 0; i < 3; i++) bytes[i] = 0;

	if (opName[opName.size() - 1] == 's') {
		flag = true;
		opName = opName.substr(0, opName.size() - 1);
	}
	if (opName.compare("and") != 0 && opName.compare("or") != 0 && opName.compare("not") != 0 && opName.compare("test") != 0) {
		cond = decodeCondition(opName.substr(opName.size() - 2, 2));
		opName = opName.substr(0, opName.size() - 2);
		if (cond == -1) {
			HelpFunctions::throwError("Invalid condition in call function", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
	}
	bytes[0] |= ((cond & 0xff) << 5);
	if (flag) bytes[0] |= 0x10;
	if (opName.compare("and") == 0) bytes[0] |= 0x6;
	else if (opName.compare("or") == 0) bytes[0] |= 0x7;
	else if (opName.compare("not") == 0) bytes[0] |= 0x8;
	else if (opName.compare("test") == 0) bytes[0] |= 0x9;

	string operand1 = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	string operand2 = HelpFunctions::toLower(lists.front());

	if (operand1.compare("sp") && operand2.compare("sp") && (!isInSet(registerSet, operand1) || !isInSet(registerSet, operand2) || isInSet(specRegsSet, operand1) || isInSet(specRegsSet, operand2))) {
		HelpFunctions::throwError("Operands in logic opearations must be GPR or SP!", true, TwoPassAssembler::numOfLine);
		exit(2);
	}
	int reg1, reg2;
	if (operand1.compare("sp") == 0) reg1 = 18;
	else reg1 =  HelpFunctions::base10ToInt(operand1.substr(1, operand1.size() - 1), 8, false);
	if (operand2.compare("sp") == 0) reg2 = 18;
	else reg2 = HelpFunctions::base10ToInt(operand2.substr(1, operand2.size() - 1), 8, false);

	bytes[1] = ((reg1 & 0xff) << 3) || ((reg2 >> 2) & 0x7);
	bytes[2] = (reg2 & 0x3) << 6;

	return bytes;
}

vector<unsigned char> VonNeuman32bit::decodeInOut(list<string> lists)
{
	vector<unsigned char> bytes(4);
	int cond = 7;
	string opName = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	for (int i = 0; i < 3; i++) bytes[i] = 0;

	if (opName.compare("in") != 0 && opName.compare("out") != 0) {
		cond = decodeCondition(opName.substr(opName.size() - 2, 2));
		opName = opName.substr(0, opName.size() - 2);
		if (cond == -1) {
			HelpFunctions::throwError("Invalid condition in call function", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
	}
	bytes[0] |= ((cond & 0xff) << 5);
	bytes[0] |= 0xd;
	string operand1 = lists.front();
	lists.pop_front();
	string operand2 = lists.front();

	if (!isInSet(registerSet, operand1) || !isInSet(registerSet, operand2) || isInSet(specRegsSet, operand1) || isInSet(specRegsSet, operand2)) {
		HelpFunctions::throwError("Destination and source of in/out must be GPR!", true, TwoPassAssembler::numOfLine);
		exit(2);
	}
	int reg1 = HelpFunctions::base10ToInt(operand1.substr(1, operand1.size()), 8, false);
	int reg2 = HelpFunctions::base10ToInt(operand2.substr(1, operand2.size()), 8, false);

	bytes[1] = ((reg1 & 0xff) << 4) | (reg2 & 0xff);
	if (opName.compare("in") == 0) bytes[2] = 0x80;

	return bytes;
}

vector<unsigned char> VonNeuman32bit::decodeInt(list<string> lists)
{
	vector<unsigned char> bytes(4);
	int cond = 7;
	string opName = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	for (int i = 0; i < 3; i++) bytes[i] = 0;

	if (opName.compare("int") != 0) {
		 cond = decodeCondition(opName.substr(opName.size() - 2, 2));
		if (cond == -1) {
			HelpFunctions::throwError("Invalid condition in call function", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
	}
	bytes[0] |= ((cond & 0xff) << 5);
	bytes[0] |= 0x0;

	string ivtString = HelpFunctions::toLower(lists.front());

	int ivt = HelpFunctions::stringToInt(ivtString, 4, false);
	bytes[1] |= (ivt << 4) & 0xff;

	return bytes;
}

vector<unsigned char> VonNeuman32bit::decodeCall(list<string> lists)
{
	vector<unsigned char> bytes(4);
	int cond = 7;
	string opName = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	int max_immediate = (1 << 18) - 1;
	int min_immediate = -max_immediate - 1;
	for (int i = 0; i < 3; i++) bytes[i] = 0;
	//call cant have 's' at the end
	if (opName.compare("call") != 0) {
		cond = decodeCondition(opName.substr(opName.size() - 2, 2));
		if (cond == -1) {
			HelpFunctions::throwError("Invalid condition in call function", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
		
	}
	bytes[0] |= ((cond & 0xff) << 5);
	bytes[0] |= 0xc;

	string destination = HelpFunctions::toLower(lists.front());

	if (isInSet(registerSet, destination)) {
		lists.pop_front();
		if (destination.compare("pc") == 0) bytes[1] = (0x10 << 3);
		else if (destination.compare("lr") == 0) bytes[1] = (0x11 << 3);
		else if (destination.compare("sp") == 0) bytes[1] = (0x12 << 3);
		else bytes[1] = (HelpFunctions::base10ToInt(destination.substr(1), 8, false) << 3);

		int immArg = HelpFunctions::stringToInt(lists.front(), 19, true);

		if (immArg) {
			bytes[1] |= ((immArg >> 16) & 0x7);
			bytes[2] = ((immArg >> 8) & 0xff);
			bytes[3] = (immArg & 0xff);
		}
	}
	else {
		string label = lists.front();
		lists.pop_front();
		int valueLabel = tables->getSymbolValue(label);
		string sectLabel = tables->getSymbolSectionType(label);

		if (sectLabel.compare(currSection->getName()) != 0) {
			HelpFunctions::throwError("Label in function call must be in same section", true, TwoPassAssembler::numOfLine);
			exit(2);
		}

		int progCnt = TwoPassAssembler::locCnt + 8;
		int relJmp = valueLabel - progCnt;

		if (relJmp > max_immediate || relJmp < min_immediate) {
			HelpFunctions::throwError("Label can't be reached!", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
		bytes[1] = (0x10 << 3);
		bytes[1] |= (relJmp >> 16) & 0x7;
		bytes[2] = (relJmp >> 8) & 0xff;
		bytes[3] = (relJmp & 0xff);
	}

	return bytes;
}

vector<unsigned char> VonNeuman32bit::decodeLdc(list<string> lists)
{
	vector<unsigned char> bytes;
	int cond = 7;
	string opName = HelpFunctions::toLower(lists.front());
	string condString = "";
	lists.pop_front();

	if (opName.compare("ldc") != 0 && opName.compare("ldcl") != 0 && opName.compare("ldch") != 0) {
		condString = opName.substr(opName.size() - 2, 2);
		cond = decodeCondition(condString);
		opName = opName.substr(0, opName.size() - 2);
		if (cond == -1) {
			HelpFunctions::throwError("Invalid condition in mov function", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
	}

	if (opName.compare("ldc") == 0) {
		list<string> opLdcL, opLdcH;
		string ldclString = "ldcl" + condString;
		string ldchString = "ldch" + condString;
		opLdcL.push_back(ldclString);
		opLdcH.push_back(ldchString);
		
		string operand1 = lists.front();
		lists.pop_front();
		string operand2 = lists.front();

		if (!isInSet(registerSet, operand1) || isInSet(specRegsSet, operand1) || isInSet(registerSet, operand2)) {
			HelpFunctions::throwError("Destination in ldc operation must be GPR and source can't be GPR!", true, TwoPassAssembler::numOfLine);
			exit(2);
		}

		opLdcL.push_back(operand1);
		opLdcH.push_back(operand1);

		if (HelpFunctions::isNumber(operand2)){
			int c = HelpFunctions::stringToInt(operand2, 32, false);
			int cH = (c >> 16) & 0xffff;
			int cL = c & 0xffff;
			opLdcL.push_back(to_string(cL));
			opLdcH.push_back(to_string(cH));
		}
		else {
			int c = tables->getSymbolValue(operand2);
			int cH = addRelocationForLDC(operand2, TwoPassAssembler::locCnt + 2, false);
			int cL = addRelocationForLDC(operand2, TwoPassAssembler::locCnt + 6, true);
			opLdcL.push_back(to_string(cL));
			opLdcH.push_back(to_string(cH));
		}
		vector<unsigned char> ldclResult = decodeLdc(opLdcL);
		vector<unsigned char> ldchResult = decodeLdc(opLdcH);
		for (int i = 0; i < ldclResult.size(); i++) bytes.push_back(ldclResult[i]);
		for (int i = 0; i < ldchResult.size(); i++) bytes.push_back(ldchResult[i]);
		return bytes;
	}
	else {
		unsigned char byte = 0;
		byte |= (cond & 0xff) << 5;
		byte |= 0xf;
		bytes.push_back(byte);
		byte = 0;

		string operand1 = lists.front();
		lists.pop_front();
		string operand2 = lists.front();

		if (!isInSet(registerSet, operand1) || isInSet(specRegsSet, operand1) || isInSet(registerSet, operand2)) {
			HelpFunctions::throwError("Destination in ldc operation must be GPR and source can't be GPR!", true, TwoPassAssembler::numOfLine);
			exit(2);
		}

		int dstReg = HelpFunctions::base10ToInt(operand1.substr(1, operand1.size() - 1), 4, false);
		byte |= (dstReg & 0xf) << 4;
		if (opName.compare("ldch") == 0) byte |= 0x8;
		bytes.push_back(byte);
		byte = 0;

		int immediate = HelpFunctions::stringToInt(operand2, 16, false);
		byte = (immediate >> 8) & 0xff;
		bytes.push_back(byte);
		byte = (immediate & 0xff);
		bytes.push_back(byte);

		return bytes;
	}
}

vector<unsigned char> VonNeuman32bit::decodeMove(list<string> lists)
{
	vector<unsigned char> bytes(4);
	int cond = 7;
	bool flag = false;
	string opName = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	for (int i = 0; i < 3; i++) bytes[i] = 0;

	if (opName[opName.size() - 1] == 's') {
		flag = true;
		opName = opName.substr(0, opName.size() - 1);
	}
	if (opName.compare("mov") != 0) {
		cond = decodeCondition(opName.substr(opName.size() - 2, 2));
		opName = opName.substr(0, opName.size() - 2);
		if (cond == -1) {
			HelpFunctions::throwError("Invalid condition in mov function", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
	}
	bytes[0] |= ((cond & 0xff) << 5);
	bytes[0] |= 0xe;

	string operand1 = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	string operand2 = HelpFunctions::toLower(lists.front());

	if (!isInSet(registerSet, operand1) || !isInSet(registerSet, operand2)) {
		HelpFunctions::throwError("Parameters in mov function must be registers!", true, TwoPassAssembler::numOfLine);
		exit(2);
	}
	int dst;
	if (operand1.compare("pc") == 0) dst = 0x10;
	else if (operand1.compare("lr") == 0) dst = 0x11;
	else if (operand1.compare("sp") == 0) dst = 0x12;
	else if (operand1.compare("psw") == 0) dst = 0x13;
	else dst =  HelpFunctions::base10ToInt(operand1.substr(1, operand1.size()), 8, false);
	bytes[1] = ((dst & 0xff) << 3);

	int src;
	if (operand2.compare("pc") == 0) src = 0x10;
	else if (operand2.compare("lr") == 0) src = 0x11;
	else if (operand2.compare("sp") == 0) src = 0x12;
	else src = HelpFunctions::base10ToInt(operand2.substr(1, operand2.size()), 8, false);
	bytes[1] |= (src >> 2) & 0x7;
	bytes[2] = (src & 0x3) << 6;

	return bytes;
}

vector<unsigned char> VonNeuman32bit::decodeShift(list<string> lists)
{
	vector<unsigned char> bytes(4);
	int cond = 7;
	bool flag = false;
	string opName = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	for (int i = 0; i < 3; i++) bytes[i] = 0;

	if (opName[opName.size() - 1] == 's') {
		flag = true;
		opName = opName.substr(0, opName.size() - 1);
	}
	if (opName.compare("shr") != 0 && opName.compare("shl") != 0) {
		cond = decodeCondition(opName.substr(opName.size() - 2, 2));
		opName = opName.substr(0, opName.size() - 2);
		if (cond == -1) {
			HelpFunctions::throwError("Invalid condition in shl/r function", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
	}
	bytes[0] |= ((cond & 0xff) << 5);
	bytes[0] |= 0xe;

	string operand1 = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	string operand2 = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	string operand3 = HelpFunctions::toLower(lists.front());

	if (!isInSet(registerSet, operand1) || !isInSet(registerSet, operand2)) {
		HelpFunctions::throwError("Parameters in shl/r function must be registers!", true, TwoPassAssembler::numOfLine);
		exit(2);
	}
	int dst;
	if (operand1.compare("pc") == 0) dst = 0x10;
	else if (operand1.compare("lr") == 0) dst = 0x11;
	else if (operand1.compare("sp") == 0) dst = 0x12;
	else if (operand1.compare("psw") == 0) dst = 0x13;
	else dst = HelpFunctions::base10ToInt(operand1.substr(1, operand1.size() - 1), 8, false);
	bytes[1] = ((dst & 0xff) << 3);

	int src;
	if (operand2.compare("pc") == 0) src = 0x10;
	else if (operand2.compare("lr") == 0) src = 0x11;
	else if (operand2.compare("sp") == 0) src = 0x12;
	else src = HelpFunctions::base10ToInt(operand2.substr(1, operand2.size() - 1), 8, false);
	bytes[1] |= (src >> 2) & 0x7;
	bytes[2] = (src & 0x3) << 6;
	
	int value = HelpFunctions::stringToInt(operand3, 5, false);
	bytes[2] |= (value & 0x1f) << 1;
	if (opName.compare("shl") == 0) bytes[2] |= 0x1;
	return bytes;
}

vector<unsigned char> VonNeuman32bit::decodeLdSt(list<string> lists)
{
	vector<unsigned char> bytes(4);
	int cond = 7;
	bool flag = false;
	string opName = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	for (int i = 0; i < 3; i++) bytes[i] = 0;

	
	if (opName.compare("ldr") != 0 && opName.compare("str") != 0) {
		cond = decodeCondition(opName.substr(opName.size() - 2, 2));
		opName = opName.substr(0, opName.size() - 2);
		if (cond == -1) {
			HelpFunctions::throwError("Invalid condition in shl/r function", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
	}
	bytes[0] |= ((cond & 0xff) << 5);
	bytes[0] |= 0xa;

	string operand1 = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	string operand2 = HelpFunctions::toLower(lists.front());
	lists.pop_front();
	if (!isInSet(registerSet, operand1)) {
		HelpFunctions::throwError("First operand in ld/st must be register!", true, TwoPassAssembler::numOfLine);
		exit(2);
	}

	int dst;
	if (operand1.compare("pc") == 0) dst = 0x10;
	else if (operand1.compare("lr") == 0) dst = 0x11;
	else if (operand1.compare("sp") == 0) dst = 0x12;
	else if (operand1.compare("psw") == 0) dst = 0x13;
	else dst = HelpFunctions::base10ToInt(operand1.substr(1, operand1.size()), 8, false);
	bytes[1] = ((dst & 0xff) << 3);

	Addr addressMode = decodeAddressMode(operand2);
	int src, f, immediate;
	if (addressMode == MemDir) {
		src = 16;
		f = 0;
		int value = tables->getSymbolValue(operand2);
		string sectionDefined = tables->getSymbolSectionType(operand2);
		if (currSection->getName().compare(sectionDefined) != 0) {
			HelpFunctions::throwError("Label must be in same section! (ldr/str)", true, TwoPassAssembler::numOfLine);
			exit(2);
		}
		int pc = TwoPassAssembler::locCnt + 8;
		immediate = value - pc;
	}
	else {
		operand2 = getRegisterName(operand2);
		if (operand2.compare("pc") == 0) src = 0x10;
		else if (operand2.compare("lr") == 0) src = 0x11;
		else if (operand2.compare("sp") == 0) src = 0x12;
		else if (operand2.compare("psw") == 0) src = 0x13;
		else src = HelpFunctions::base10ToInt(operand2.substr(1, operand2.size()), 8, false);

		switch (addressMode)
		{
		case RegPosDec: f = 3;
			break;
		case RegPosInc: f = 2;
			break;
		case RegPreDec: f = 5;
			break;
		case RegPreInc: f = 4;
			break;
		default: f = 0;
			break;
		}

		if (operand2.compare("pc") == 0 && f != 0) {
			HelpFunctions::throwError("When address register is PC, f must be 0!", true, TwoPassAssembler::numOfLine);
			exit(2);
		}

		if (!lists.empty()) {
			string operand3 = lists.front();
			immediate = HelpFunctions::stringToInt(operand3, 10, true);
		}
		else {
			immediate = 0;
		}
	}

	bytes[1] |= (src >> 2) & 0x7;
	bytes[2] = (src & 0x3) << 6;
	bytes[2] |= (f & 0x7) << 3;
	if (opName.compare("ldr") == 0) bytes[2] |= 0x4;
	if (immediate) {
		bytes[2] |= (immediate >> 8) & 0x3;
		bytes[3] = immediate & 0xff;
	}
	return bytes;
}

bool VonNeuman32bit::checkNumOfParams(list<string> lists)
{
	string opName = getRealOpName(lists.front());
	lists.pop_front();
	if ((isInSet(artihmeticSet, opName) || isInSet(logicSet, opName) || isInSet(inOutSet, opName) || isInSet(ldcSet, opName) || !opName.compare("mov")) && countParams(lists) == 2)
		return true;
	if ((opName.compare("int") == 0) && countParams(lists) == 1) return true;
	if (isInSet(shiftSet, opName) && countParams(lists) == 3) return true;
	if (isInSet(ldStSet, opName) && (countParams(lists) == 2 || countParams(lists) == 3)) return true;
	if ((opName.compare("call") == 0) && (countParams(lists) == 2 || countParams(lists) == 1)) return true;
	return false;
}

int VonNeuman32bit::countParams(list<string> lists)
{
	int cnt = 0;

	while (lists.size() > 0) {
		string param = lists.front();
		lists.pop_front();
		if (param[0] == ';') break;
		cnt++;
	}
	return cnt;
}

int VonNeuman32bit::addRelocationForLDC(string name, int location, bool low)
{
	bool global = tables->getGlobalSymbol(name);
	int returnValue, reference;

	if (global) {
		reference = tables->getSymbolNum(name);
		returnValue = 0;
	}
	else {
		string symbolSection = tables->getSymbolSectionType(name);
		reference = tables->getSymbolNum(symbolSection);
		returnValue = tables->getSymbolValue(name);
	}

	string relocation;

	if (low) relocation = Relocation::relocationType[0];
	else relocation = Relocation::relocationType[1];

	Relocation *rel = new Relocation(currSection->getName(), location, relocation, reference);
	tables->addToRelocationList(rel);

	if (low)  returnValue &= 0xff;
	else return returnValue = (returnValue >> 16) & 0xff;

	return returnValue;
}

Addr VonNeuman32bit::decodeAddressMode(string reg)
{
	string registerName;
	int size = reg.size();
	if (reg[0] != '+' && reg[0] != '-' && reg[0] != '[' && reg[size - 1] != '-' && reg[size - 1] != '+' && reg[size - 1] != ']') {
		if (isInSet(registerSet, reg)) {
			HelpFunctions::throwError("Error in decoding address mode!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
		else return MemDir;
	}
	if (reg[0] == '[' && reg[size - 1] == ']') {
		registerName = reg.substr(1, size - 2); ///////////////7
		if (isInSet(registerSet, registerName)) return RegInd;
		else {
			HelpFunctions::throwError("Error in decoding address mode!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
	}
	if (reg[0] == '+' && reg[1] == '[' && reg[size - 1] == ']') {
		registerName = reg.substr(2, size - 3);
		if (isInSet(registerSet, registerName) && registerName.compare("pc") != 0) return RegPreInc;
		else {
			HelpFunctions::throwError("Error in decoding address mode!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
	}
	if (reg[0] == '[' && reg[size - 2] == ']' && reg[size - 1] == '+') {
		registerName = reg.substr(1, size - 3);
		if (isInSet(registerSet, registerName) && registerName.compare("pc") != 0) return RegPosInc;
		else {
			HelpFunctions::throwError("Error in decoding address mode!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
	}
	if (reg[0] == '-' && reg[1] == '[' && reg[size - 1] == ']') {
		registerName = reg.substr(2, size - 3);
		if (isInSet(registerSet, registerName) && registerName.compare("pc") != 0) return RegPreDec;
		else {
			HelpFunctions::throwError("Error in decoding address mode!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
	}
	if (reg[0] == '[' && reg[size - 2] == ']' && reg[size - 1] == '-') {
		registerName = reg.substr(1, size - 3);
		if (isInSet(registerSet, registerName) && registerName.compare("pc") != 0) return RegPosDec;
		else {
			HelpFunctions::throwError("Error in decoding address mode!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
	}
	HelpFunctions::throwError("Error in decoding address mode!", true, TwoPassAssembler::numOfLine);
	exit(4);
}

string VonNeuman32bit::getRegisterName(string reg)
{
	string registerName;
	int size = reg.size();
	if (reg[0] != '+' && reg[0] != '-' && reg[0] != '[' && reg[size - 1] != '-' && reg[size - 1] != '+' && reg[size - 1] != ']') {
		if (isInSet(registerSet, reg)) {
			HelpFunctions::throwError("Error in parsing register name!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
		else return reg;
	}
	if (reg[0] == '[' && reg[size - 1] == ']') {
		registerName = reg.substr(1, size - 2); ///////////////7
		if (isInSet(registerSet, registerName)) return registerName;
		else {
			HelpFunctions::throwError("Error in parsing register name!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
	}
	if (reg[0] == '+' && reg[1] == '[' && reg[size - 1] == ']') {
		registerName = reg.substr(2, size - 3);
		if (isInSet(registerSet, registerName) && registerName.compare("pc") != 0) return registerName;
		else {
			HelpFunctions::throwError("Error in parsing register name!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
	}
	if (reg[0] == '[' && reg[size - 2] == ']' && reg[size - 1] == '+') {
		registerName = reg.substr(1, size - 3);
		if (isInSet(registerSet, registerName) && registerName.compare("pc") != 0) return registerName;
		else {
			HelpFunctions::throwError("Error in parsing register name!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
	}
	if (reg[0] == '-' && reg[1] == '[' && reg[size - 1] == ']') {
		registerName = reg.substr(2, size - 3);
		if (isInSet(registerSet, registerName) && registerName.compare("pc") != 0) return registerName;
		else {
			HelpFunctions::throwError("Error in parsing register name!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
	}
	if (reg[0] == '[' && reg[size - 2] == ']' && reg[size - 1] == '-') {
		registerName = reg.substr(1, size - 3);
		if (isInSet(registerSet, registerName) && registerName.compare("pc") != 0) return registerName;
		else {
			HelpFunctions::throwError("Error in parsing register name!", true, TwoPassAssembler::numOfLine);
			exit(4);
		}
	}
	HelpFunctions::throwError("Error in parsing register name!", true, TwoPassAssembler::numOfLine);
	exit(4);
}
