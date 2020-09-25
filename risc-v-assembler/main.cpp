/*
Written by Claude Garrett V.

This program was designed as a basic assembler for RV32I.

*/

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

int argToReg(string arg){
    if(arg == "zero") return 0;
    if(arg == "ra") return 1;
    if(arg == "sp") return 2;
    if(arg == "gp") return 3;
    if(arg == "tp") return 4;
    if(arg[0] == 't') {
        int t = stoi(arg.substr(1, arg.length()));
        if(t < 3) return t + 5;
        else return t + 25;
    }
    if(arg[0] == 's') {
        int s = stoi(arg.substr(1, arg.length()));
        if(s < 2) return s + 8;
        else return s + 16;
    }
    if(arg == "fp") return 8;
    if(arg[0] == 'a') return stoi(arg.substr(1, arg.length())) + 10;
    if(arg[0] == 'x') return stoi(arg.substr(1, arg.length()));
}

int argToImm(string arg) {
    return stoi(arg);
}

int RType(string* args, int f7, int f3, int op) {
    int ret = 0;

    ret += (f7 & 0x7f) << 25; // funct7
    ret += (argToReg(args[2]) & 0x1f) << 20; // rs2
    ret += (argToReg(args[1]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += (argToReg(args[0]) & 0x1f) << 7; // rd
    ret += (op & 0x3f); // opcode

    return ret;
}

int IType(string* args, int f3, int op) { // UNTESTED
    int ret = 0;

    ret += (argToImm(args[2]) & 0xfff) << 20;
    ret += (argToReg(args[1]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += (argToReg(args[0]) & 0x1f) << 7; // rd
    ret += (op & 0x3f); // opcode

    return ret;
}

int SType(string* args, int f3, int op) { // UNTESTED
    int ret = 0;
    int imm = argToImm(args[2]);

    ret += ((imm >> 5) & 0x7f) << 25; // imm[11:5]
    ret += (argToReg(args[1]) & 0x1f) << 20; // rs2
    ret += (argToReg(args[0]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += (imm & 0x1f) << 7; // imm[4:0]
    ret += (op & 0x3f); // opcode

    return ret;
}

int BType() {

}

int UType() {

}

int JType() {

}

int main()
{
    // Settings:
    string programFile = "assembly.txt";
    string outputFile = "output.txt";
    int instructionLength = 32;
    int memSizeKB = 8; // Memory size in KB, will be calculated as ((2^20) * n)


    // Open the input and output files.
    ifstream prog; prog.open(programFile);
    ofstream out; out.open(outputFile);

    int memSize = (2 << 10) * memSizeKB;
    int* memory = new int[memSize];

    for(int i = 0; i < memSize; i++) { // Initialize the array to zero because some junk found its way in.
        memory[i] = 0;
    }

    // Start reading in the input file.
    int lineNumber = 0;
    int hardwareInst = 0;

    string inst = "";
    string operation = "";
    string arg[5] = {"", "", "", "", ""};

    while(getline(prog, inst)) {
        // Cut out any comments.
        if(inst.find("#")) inst = inst.substr(0, inst.find("#"));
        if(inst.length() < 2) {lineNumber++; continue;} // If the line is just a comment, skip it.

        cout << "Instruction is \"" << inst << "\"\n";
        operation = inst.substr(0, inst.find(" ")); // The operation is always followed by a space. Look for the first and the content before it is the op.
        cout << "\tOperation is \"" << operation << "\"\n";

        inst = inst.substr(inst.find(" ") + 1, inst.length());
        cout << "\tArguments are \"" << inst << "\"" << endl; // The arguments are simply the remaining section after the op, remove the op and the space after it to find your args.
        int numArgs = 0;

        while(inst.find(",") != -1) { // When a comma is found, there are still multiple arguments left. This finds them.
            arg[numArgs] = inst.substr(0, inst.find(",")); // The arguments usually end in a comma.
            numArgs++;
            inst = inst.substr(inst.find(",") + 2, inst.length()); // Remove the argument you just isolated. This only supports a format with ARG, ARG where a space/comma is in between the args.
        }

        arg[numArgs] = inst; // Last piece has to be an argument.
        numArgs++;

        for(int i = 0; i < numArgs; i++) { // Print out arguments for debugging.
            cout << "\t\tArgument " << i << " is \"" << arg[i] << "\"" << endl;
        }

        // The main "switch" statement made of conditionals since strings aren't supported. Starts with RV32I instructions.
        for(int i = 0; i < operation.length(); i++) { // Convert the operation string to lowercase.
            operation[i] = tolower(operation[i]);
        }

               if(operation == "lui") {

        } else if(operation == "auipc") {

        } else if(operation == "jal") {

        } else if(operation == "jalr") {

        } else if(operation == "beq") {

        } else if(operation == "bne") {

        } else if(operation == "blt") {

        } else if(operation == "bge") {

        } else if(operation == "bltu") {

        } else if(operation == "bgeu") {

        } else if(operation == "lb") {

        } else if(operation == "lh") {

        } else if(operation == "lw") {

        } else if(operation == "lbu") {

        } else if(operation == "lhu") {

        } else if(operation == "sb") {

        } else if(operation == "sh") {

        } else if(operation == "sw") {

        } else if(operation == "addi") {

        } else if(operation == "slti") {

        } else if(operation == "sltiu") {

        } else if(operation == "xori") {

        } else if(operation == "ori") {

        } else if(operation == "andi") {

        } else if(operation == "slli") {

        } else if(operation == "srli") {

        } else if(operation == "srai") {

        } else if(operation == "add") {

        } else if(operation == "sub") {

        } else if(operation == "sll") {

        } else if(operation == "slt") {

        } else if(operation == "sltu") {

        } else if(operation == "xor") {

        } else if(operation == "srl") {

        } else if(operation == "sra") {

        } else if(operation == "or") {

        } else if(operation == "and") {

        } else if(operation == "fence") {

        } else if(operation == "ecall") {

        } else if(operation == "ebreak") {

        }

        // Prepare for next cycle.
        lineNumber++;
        hardwareInst = 0;

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

    // Testing
    string* test = new string[3];
    test[0] = "x0"; // rd
    test[1] = "x0"; // rs1
    test[2] = "x0"; // rs2

    cout << hex << setfill('0') << setw(8) << RType(test, 0, 0, 0x3f);
    delete [] test;

    return 0;
}
