#include "TwoPassAssembler.h"




int main(int argc, char* argv[]) {
	if (argc < 3) {
		cout << "You must enter names of input and output file!";
		exit(2);
	}

	TwoPassAssembler twoPassAsm;
	string inputFile = argv[1];
	string outputFile = argv[2];

	twoPassAsm.firstPass(inputFile);

	twoPassAsm.secondPass(inputFile, outputFile);

	return 1;
}