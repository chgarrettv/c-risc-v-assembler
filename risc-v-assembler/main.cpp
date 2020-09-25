/*
Written by Claude Garrett V.

This program was designed as a basic compiler for RV32I.

*/

#include <iostream>
#include <fstream>
#include <iomanip>
//#include <string>

using namespace std;

int main()
{
    cout << "Starting.\n";
    // Settings:
    string programFile = "assembly.txt";
    string outputFile = "output.txt";
    int instructionLength = 32;
    int memSizeKB = 8; // Memory size in KB, will be calculated as ((2^20) * n)


    // Open the input and output files.
    ifstream prog; prog.open(programFile);
    ofstream out; out.open(outputFile);

    int memSize = (2 << 10) * memSizeKB;
    int* memory; memory = new int[memSize];

    for(int i = 0; i < memSize; i++) { // Initialize the array to zero because some junk found its way in.
        memory[i] = 0;
    }

    // Start reading in the input file.
    string inst = "";
    string operation = "";
    string arg[5] = {"", "", "", "", ""};

    int funct7 = 0;
    int rs2 = 0;
    int rs1 = 0;
    int funct3 = 0;
    int rd = 0;
    int opcode = 0;

    while(getline(prog, inst)) {
        // Cut out any comments.
        if(inst.find("#")) inst = inst.substr(0, inst.find("#"));
        if(inst.length() < 2) continue; // If the line is just a comment, skip it.

        cout << "Instruction is \"" << inst << "\"\n";
        operation = inst.substr(0, inst.find(" "));
        cout << "\tOperation is \"" << operation << "\"\n";

        inst = inst.substr(inst.find(" ") + 1, inst.length());
        cout << "\tArguments are \"" << inst << "\"" << endl;
        int numArgs = 0;

        while(inst.find(",") != -1) {
            arg[numArgs] = inst.substr(0, inst.find(","));
            numArgs++;
            inst = inst.substr(inst.find(",") + 2, inst.length());
        }

        arg[numArgs] = inst; // Last bit has to be an argument.
        numArgs++;

        for(int i = 0; i < numArgs; i++) {
            cout << "\t\tArgument " << i << " is \"" << arg[i] << "\"" << endl;
        }


        // Reset all variables.
        int funct7 = 0;
        int rs2 = 0;
        int rs1 = 0;
        int funct3 = 0;
        int rd = 0;
        int opcode = 0;
        cout << endl;
    }











    // Write everything to the output file.
    for(int i = 0; i < memSize; i++) {
        out << hex << setfill('0') << setw(instructionLength / 4) << memory[i] << endl;
    } cout << "Finished writing the program file.\n";

    // Close all of the files.
    prog.close();
    out.close();

    // Purge all of the arrays.
    delete [] memory;
    memory = nullptr;

    return 0;
}
