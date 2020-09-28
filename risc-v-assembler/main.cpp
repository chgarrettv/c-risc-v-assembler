/*
Written by Claude Garrett V.

This program was designed as a basic assembler for RV32I.

*/

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

int argToReg(string arg){ // Should return a value between 0-31.
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

    else return -1;
}

int argToImm(string arg) {
    return stoi(arg);
}

string* addressExpander(string* args) { // For instructions like lw that have a #(reg) in the assembly instruction format. This expands them into reg and immediate arguments.
    string* ret; ret = new string[args.length() + 1];

    for(int i = 0; i < args.length(); i++) {
        if(args[i].find("(") != -1 && args.find(")") != -1) {
            // Initialize the return array.
            for(int i = 0; i < args.length() - 1; i++) {
                ret[i] = args[i];
            }

            string lastArg = args[args.length() - 1];
            ret[args.length() - 1] = lastArg.substr(lastArg.find("("), lastArg.find(")") - lastArg.find("(")); // rs1
            ret[args.length()] = lastArg.substr(0, lastArg.find("(")); // imm
            return ret;
        }
        if((args[i].find("(") == -1 && args.find(")") != -1) || (args[i].find("(") != -1 && args.find(")") == -1)) {
            cout << "Incomplete parenthesis: " << args[i];
        }
    }
    return args;
}

int RType(string* args, int f7, int f3, int op) {
    // Argument order: rd, rs1, rs2
    int ret = 0;

    ret += (f7 & 0x7f) << 25; // funct7
    ret += (argToReg(args[2]) & 0x1f) << 20; // rs2
    ret += (argToReg(args[1]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += (argToReg(args[0]) & 0x1f) << 7; // rd
    ret += (op & 0x7f); // opcode

    return ret;
}

int IType(string* args, int f3, int op) { // UNTESTED
    // Argument order: rd, rs1, imm
    int ret = 0;

    ret += (argToImm(args[2]) & 0xfff) << 20;
    ret += (argToReg(args[1]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += (argToReg(args[0]) & 0x1f) << 7; // rd
    ret += (op & 0x7f); // opcode

    return ret;
}

int SType(string* args, int f3, int op) { // UNTESTED
    // Argument order: rs1, rs2, imm
    int ret = 0;
    int imm = argToImm(args[2]);

    ret += ((imm >> 5) & 0x7f) << 25; // imm[11:5]
    ret += (argToReg(args[1]) & 0x1f) << 20; // rs2
    ret += (argToReg(args[0]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += (imm & 0x1f) << 7; // imm[4:0]
    ret += (op & 0x7f); // opcode

    return ret;
}

int BType(string* args, int f3, int op) { // UNTESTED
    // Argument order: rs1, rs2, imm
    int ret = 0;
    int imm = argToImm(args[2]);

    ret += ((imm >> 12) & 0x1) << 31; // imm 12
    ret += ((imm >> 5) & 0x3f) << 25; // imm 10:5
    ret += (argToReg(args[1]) & 0x1f) << 20; // rs2
    ret += (argToReg(args[0]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += ((imm >> 1) & 0xf) << 8; // imm 4:1
    ret += ((imm >> 11) & 0x1) << 7; // imm 11
    ret += (op & 0x7f); // opcode

    return ret;
}

int UType(string* args, int op) { // UNTESTED
    // Arugment order: rd, imm
    int ret = 0;

    ret += ((argToImm(args[1]) >> 12) & 0xfffff000) << 12; // imm 31:12
    ret += (argToReg(args[0]) & 0x1f) << 7; // rd
    ret += (op & 0x7f); // opcode

    return ret;
}

int JType(string* args, int op) { // UNTESTED
    // Arugment order: rd, imm
    int ret = 0;
    int imm = argToImm(args[1]);

    ret += ((imm >> 20) & 0x1) << 31; // imm 20
    ret += ((imm >> 1) & 0x3ff) <<  21; // imm 10:1
    ret += ((imm >> 11) & 0x1) << 20; // imm 11
    ret += ((imm >> 12) & 0xff) << 12; // imm 19:12
    ret += (argToReg(args[0]) & 0x1f) << 7; // rd
    ret += (op & 0x7f); // opcode

    return ret;

}

bool TypeTesting() {
    string* t = new string[3];


    // RType:
    t[0] = "x0"; t[1] = "x0"; t[2] = "x0";
    if(RType(t, 0, 0x7, 0) != (0x7 << 12)) cout << "\tRType funct3 error.\n";

    t[0] = "x0"; t[1] = "x0"; t[2] = "x0";
    if(RType(t, 0, 0, 0x7f) != 0x7f) cout << "\tRType opcode error.\n";


    // IType:
    t[0] = "x0"; t[1] = "x0"; t[2] = "0";
    if(IType(t, 0, 0x7f) != 0x7f) cout << "\tIType opcode error.\n";

    t[0] = "x0"; t[1] = "x0"; t[2] = "-1";
    if(IType(t, 0, 0) != (0xfff << 20)) cout << "\tIType imm error.\n";


    // SType:
    t[0] = "x0"; t[1] = "x0"; t[2] = "0";
    if(SType(t, 0, 0x7f) != 0x7f) cout << "\tSType opcode error.\n";

    t[0] = "x0"; t[1] = "x0"; t[2] = "-1";
    if(SType(t, 0, 0) != (0xfe000f80)) cout << "\tSType imm error.\n";

    //cout << hex << setfill('0') << setw(8) << RType(t, 0, 0, 0x3f);
    delete [] t;

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
        if(inst.find("#")) inst = inst.substr(0, inst.find("#")); // If there is a comment, remove it from the instruction.
        if(inst.length() < 2) {lineNumber++; continue;} // If the line is just a comment, skip it.

        cout << "Instruction is \"" << inst << "\"\n";
        operation = inst.substr(0, inst.find(" ")); // The operation is always followed by a space. Look for the first and the content before it is the op.
        cout << "\tOperation is \"" << operation << "\"\n";

        inst = inst.substr(inst.find(" ") + 1, inst.length());
        cout << "\tArguments are \"" << inst << "\"" << endl; // The arguments are simply the remaining section after the op, remove the op and the space after it to find your args.
        int numArgs = 0;

        while(inst.find(",") != -1) { // When a comma is found, there are still multiple arguments left. This finds them.
            int start = 0;
            while(start < inst.length() && inst[start] == ' ') start++; // Increment start until it is no longer a space. Used to ignore white space.

            arg[numArgs] = inst.substr(start, inst.find(",") - start); // The arguments usually end in a comma.
            numArgs++;
            inst = inst.substr(inst.find(",") + 2, inst.length()); // Remove the argument you just isolated. This only supports a format with ARG, ARG where a space/comma is in between the args.
        }

        // Last piece has to be an argument.
        if(inst.find(" ") != -1) arg[numArgs] = inst.substr(inst.find_last_of(" ") + 1, inst.length());
        else arg[numArgs] = inst;

        numArgs++;

        for(int i = 0; i < numArgs; i++) { // Print out arguments for debugging.
            cout << "\t\tArgument " << i << " is \"" << arg[i] << "\"" << endl;
        }

        // The main "switch" statement made of conditionals since strings aren't supported. Starts with RV32I instructions.
        for(int i = 0; i < operation.length(); i++) { // Convert the operation string to lowercase.
            operation[i] = tolower(operation[i]);
        }


        // RV 32I Instructions:
               if(operation == "lui") {
            hardwareInst = UType();
        } else if(operation == "auipc") {
            hardwareInst = UType();
        } else if(operation == "jal") {
            hardwareInst = JType();
        } else if(operation == "jalr") {
            hardwareInst = IType();
        } else if(operation == "beq") {
            hardwareInst = BType();
        } else if(operation == "bne") {
            hardwareInst = BType();
        } else if(operation == "blt") {
            hardwareInst = BType();
        } else if(operation == "bge") {
            hardwareInst = BType();
        } else if(operation == "bltu") {
            hardwareInst = BType();
        } else if(operation == "bgeu") {
            hardwareInst = BType();
        } else if(operation == "lb") {
            hardwareInst = IType();
        } else if(operation == "lh") {
            hardwareInst = IType();
        } else if(operation == "lw") { // Think this is for all load and store: format is lw rd, #(rs1) where # is an added offset to rs1. Offset = imm.
            hardwareInst = IType();
        } else if(operation == "lbu") {
            hardwareInst = IType();
        } else if(operation == "lhu") {
            hardwareInst = IType();
        } else if(operation == "sb") {
            hardwareInst = SType();
        } else if(operation == "sh") {
            hardwareInst = SType();
        } else if(operation == "sw") {
            hardwareInst = SType();
        } else if(operation == "addi") {
            hardwareInst = IType();
        } else if(operation == "slti") {
            hardwareInst = IType();
        } else if(operation == "sltiu") {
            hardwareInst = IType();
        } else if(operation == "xori") {
            hardwareInst = IType();
        } else if(operation == "ori") {
            hardwareInst = IType();
        } else if(operation == "andi") {
            hardwareInst = IType();
        } else if(operation == "slli") { // NEED A SPECIAL IMM FOR SHAMT
            hardwareInst = IType();
        } else if(operation == "srli") { // NEED A SPECIAL IMM FOR SHAMT
            hardwareInst = IType();
        } else if(operation == "srai") { // NEED A SPECIAL IMM FOR SHAMT
            hardwareInst = IType();
        } else if(operation == "add") {
            hardwareInst = RType(arg, 0x0, 0x0, 0x33);
        } else if(operation == "sub") {
            hardwareInst = RType(arg, 0x2, 0x0, 0x33);
        } else if(operation == "sll") {
            hardwareInst = RType(arg, 0x0, 0x1, 0x33);
        } else if(operation == "slt") {
            hardwareInst = RType(arg, 0x0, 0x2, 0x33);
        } else if(operation == "sltu") {
            hardwareInst = RType(arg, 0x0, 0x3, 0x33);
        } else if(operation == "xor") {
            hardwareInst = RType(arg, 0x0, 0x4, 0x33);
        } else if(operation == "srl") {
            hardwareInst = RType(arg, 0x0, 0x5, 0x33);
        } else if(operation == "sra") {
            hardwareInst = RType(arg, 0x2, 0x5, 0x33);
        } else if(operation == "or") {
            hardwareInst = RType(arg, 0x0, 0x6, 0x33);
        } else if(operation == "and") {
            hardwareInst = RType(arg, 0x0, 0x7, 0x33);
        } else if(operation == "fence") {
            hardwareInst = IType();
        } else if(operation == "ecall") { // Hard coded instruction.
            hardwareInst = 0x00000073;
        } else if(operation == "ebreak") { // Hard coded instruction.
            hardwareInst = 0x00100073;
        }

        cout << hex << setfill('0') << setw(8) << hardwareInst;

        // Prepare for next cycle.
        lineNumber++;
        hardwareInst = 0;

        cout << endl;
    }


    // Write everything to the output file.
    for(int i = 0; i < memSize; i++) {
        out << hex << setfill('0') << setw(instructionLength / 4) << memory[i] << endl;
    } cout << "Finished writing to the program file.\n";

    // Close all of the files.
    prog.close();
    out.close();

    // Purge all of the arrays.
    delete [] memory;
    memory = nullptr;

    TypeTesting();

    return 0;
}
