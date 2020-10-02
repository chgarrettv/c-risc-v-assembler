/*
Written by Claude Garrett V.
*/

#include "Translator.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <string>

Translator::Translator() {};

Translator::~Translator() {};

string Translator::stringTrimmer(string inst) { // Cut out any comments and leading white space and tabs.
    // Remove leading tabs and spaces:
        // Do this first cause a comment could have leading white space.
    if(inst == "") return inst;
    inst = inst.substr(inst.find_first_not_of(" \t"), inst.length()); // Cut off any leading white space and tabs.

    // Remove comments:
    if(inst[0] == '#') return "";
    if(inst[0] == '/' && inst[1] == '/') return "";
    if(inst.find("#") != -1) inst = inst.substr(0, inst.find("#")); // If there is a comment, remove it from the instruction.
    if(inst.find("//") != -1) inst = inst.substr(0, inst.find("/")); // Support of // comments.

    return inst;
}

string Translator::labelIsolator(string label) {
    return label.substr(0, label.find(":"));
}

string* Translator::instrToOperands(string inst, string* args) {

    cout << "Instruction is \"" << inst << "\"\n";
    args[0] = inst.substr(0, inst.find(" ")); // The operation is always followed by a space. Look for the first and the content before it is the op.
    cout << "\tOperation is \"" << args[0] << "\"\n";

    inst = inst.substr(inst.find(" ") + 1, inst.length());
    cout << "\tOperands are \"" << inst << "\"" << endl; // The arguments are simply the remaining section after the op, remove the op and the space after it to find your args.
    int numArgs = 1;

    while(inst.find(",") != -1) { // When a comma is found, there are still multiple arguments left. This finds them.
        int start = 0;
        while(start < inst.length() && inst[start] == ' ') start++; // Increment start until it is no longer a space. Used to ignore white space.

        args[numArgs] = inst.substr(start, inst.find(",") - start); // The arguments usually end in a comma.
        numArgs++;
        inst = inst.substr(inst.find(",") + 1, inst.length()); // Remove the argument you just isolated. This only supports a format with ARG, ARG where a space/comma is in between the args.
    }

    // Last piece has to be an argument.
    if(inst.find(" ") != -1) args[numArgs] = inst.substr(inst.find_first_not_of(" "), inst.find_last_not_of(" ") - inst.find_first_not_of(" ") + 1);
    else args[numArgs] = inst;

    if(args[numArgs].find("(") != -1 && args[numArgs].find(")") != -1) {
        string s = args[numArgs];

        args[numArgs] = s.substr(s.find("(") + 1, s.find(")") - s.find("(") - 1); // rs1
        args[numArgs + 1] = s.substr(0, s.find("(")); // imm

        numArgs++;
    } else if((args[numArgs].find("(") == -1 && args[numArgs].find(")") != -1) || (args[numArgs].find("(") != -1 && args[numArgs].find(")") == -1)) {
        cout << "Missing a parenthesis: " << args[numArgs];
    }

    numArgs++;

    for(int i = 0; i < numArgs; i++) { // Print out arguments for debugging.
        cout << "\t\tOperand " << i << " is \"" << args[i] << "\"" << endl;
    }

    // The main "switch" statement made of conditionals since strings aren't supported. Starts with RV32I instructions.
    for(int i = 0; i < args[0].length(); i++) { // Convert the operation string to lowercase.
        args[0][i] = tolower(args[0][i]);
    }

    return args;
}

int Translator::argToReg(string arg){ // Should return a value between 0-31.
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

    cout << "Unrecognized register: " << arg << "\n\n";
    return -1;
}

int Translator::argToImm(string arg) {
    int imm = 0;
    if(arg.find("0x") != -1) imm = (int)stoul(arg, nullptr, 16);
    else imm = (int)stoul(arg);

    cout << "\t\tImm " << arg << ": " << hex << setfill('0') << setw(8) << imm << endl;
    return imm;
}

int Translator::rType(string* args, int f7, int f3, int op) {
    // Argument order: rd, rs1, rs2
    int ret = 0;

    ret += (f7 & 0x7f) << 25; // funct7
    ret += (argToReg(args[3]) & 0x1f) << 20; // rs2
    ret += (argToReg(args[2]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += (argToReg(args[1]) & 0x1f) << 7; // rd
    ret += (op & 0x7f); // opcode

    return ret;
}

int Translator::iType(string* args, int f3, int op) { // UNTESTED
    // Argument order: rd, rs1, imm
    int ret = 0;

    ret += (argToImm(args[3]) & 0xfff) << 20;
    ret += (argToReg(args[2]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += (argToReg(args[1]) & 0x1f) << 7; // rd
    ret += (op & 0x7f); // opcode

    return ret;
}

int Translator::sType(string* args, int f3, int op) { // UNTESTED
    // Argument order: rs1, rs2, imm
    int ret = 0;
    int imm = argToImm(args[3]);

    ret += ((imm >> 5) & 0x7f) << 25; // imm[11:5]
    ret += (argToReg(args[2]) & 0x1f) << 20; // rs2
    ret += (argToReg(args[1]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += (imm & 0x1f) << 7; // imm[4:0]
    ret += (op & 0x7f); // opcode

    return ret;
}

int Translator::bType(string* args, int f3, int op) { // UNTESTED
    // Argument order: rs1, rs2, imm
    int ret = 0;
    int imm = argToImm(args[3]);

    ret += ((imm >> 12) & 0x1) << 31; // imm 12
    ret += ((imm >> 5) & 0x3f) << 25; // imm 10:5
    ret += (argToReg(args[2]) & 0x1f) << 20; // rs2
    ret += (argToReg(args[1]) & 0x1f) << 15; // rs1
    ret += (f3 & 0x7) << 12; // funct3
    ret += ((imm >> 1) & 0xf) << 8; // imm 4:1
    ret += ((imm >> 11) & 0x1) << 7; // imm 11
    ret += (op & 0x7f); // opcode

    return ret;
}

int Translator::uType(string* args, int op) { // UNTESTED
    // Arugment order: rd, imm
    int ret = 0;

    ret += ((argToImm(args[2]) >> 12) & 0xfffff000) << 12; // imm 31:12
    ret += (argToReg(args[1]) & 0x1f) << 7; // rd
    ret += (op & 0x7f); // opcode

    return ret;
}

int Translator::jType(string* args, int op) { // UNTESTED
    // Arugment order: rd, imm
    int ret = 0;
    int imm = argToImm(args[2]);

    ret += ((imm >> 20) & 0x1) << 31; // imm 20
    ret += ((imm >> 1) & 0x3ff) <<  21; // imm 10:1
    ret += ((imm >> 11) & 0x1) << 20; // imm 11
    ret += ((imm >> 12) & 0xff) << 12; // imm 19:12
    ret += (argToReg(args[1]) & 0x1f) << 7; // rd
    ret += (op & 0x7f); // opcode

    return ret;

}

bool Translator::typeTesting() {
    string* t = new string[3];


    // RType:
    t[0] = "x0"; t[1] = "x0"; t[2] = "x0";
    if(rType(t, 0, 0x7, 0) != (0x7 << 12)) cout << "\tRType funct3 error.\n";

    t[0] = "x0"; t[1] = "x0"; t[2] = "x0";
    if(rType(t, 0, 0, 0x7f) != 0x7f) cout << "\tRType opcode error.\n";


    // IType:
    t[0] = "x0"; t[1] = "x0"; t[2] = "0";
    if(iType(t, 0, 0x7f) != 0x7f) cout << "\tIType opcode error.\n";

    t[0] = "x0"; t[1] = "x0"; t[2] = "-1";
    if(iType(t, 0, 0) != (0xfff << 20)) cout << "\tIType imm error.\n";


    // SType:
    t[0] = "x0"; t[1] = "x0"; t[2] = "0";
    if(sType(t, 0, 0x7f) != 0x7f) cout << "\tSType opcode error.\n";

    t[0] = "x0"; t[1] = "x0"; t[2] = "-1";
    if(sType(t, 0, 0) != (0xfe000f80)) cout << "\tSType imm error.\n";


    // Function Tests:


    //cout << hex << setfill('0') << setw(8) << RType(t, 0, 0, 0x3f);
    delete [] t;

}

uint32_t Translator::toMachineLanguage(string* args) {
    uint32_t hardwareInst = 0;
    string operation = args[0];

    // RV32I Base Instruction Set:
           if(operation == "lui") {
        hardwareInst = uType(args, 0x37);
    } else if(operation == "auipc") {
        hardwareInst = uType(args, 0x17);
    } else if(operation == "jal") {
        hardwareInst = jType(args, 0x6f);
    } else if(operation == "jalr") {
        hardwareInst = iType(args, 0x0, 0x67);
    } else if(operation == "beq") {
        hardwareInst = bType(args, 0x0, 0x63);
    } else if(operation == "bne") {
        hardwareInst = bType(args, 0x1, 0x63);
    } else if(operation == "blt") {
        hardwareInst = bType(args, 0x4, 0x63);
    } else if(operation == "bge") {
        hardwareInst = bType(args, 0x5, 0x63);
    } else if(operation == "bltu") {
        hardwareInst = bType(args, 0x6, 0x63);
    } else if(operation == "bgeu") {
        hardwareInst = bType(args, 0x7, 0x63);
    } else if(operation == "lb") {
        hardwareInst = iType(args, 0x0, 0x03);
    } else if(operation == "lh") {
        hardwareInst = iType(args, 0x1, 0x03);
    } else if(operation == "lw") { // Think this is for all load and store: format is lw rd, 0x#(rs1) where 0x# is an added offset to rs1. Offset = imm.
        hardwareInst = iType(args, 0x2, 0x03);
    } else if(operation == "lbu") {
        hardwareInst = iType(args, 0x4, 0x03);
    } else if(operation == "lhu") {
        hardwareInst = iType(args, 0x5, 0x03);
    } else if(operation == "sb") {
        hardwareInst = sType(args, 0x0, 0x23);
    } else if(operation == "sh") {
        hardwareInst = sType(args, 0x1, 0x23);
    } else if(operation == "sw") {
        hardwareInst = sType(args, 0x2, 0x23);
    } else if(operation == "addi") {
        hardwareInst = iType(args, 0x0, 0x13);
    } else if(operation == "slti") {
        hardwareInst = iType(args, 0x2, 0x13);
    } else if(operation == "sltiu") {
        hardwareInst = iType(args, 0x3, 0x13);
    } else if(operation == "xori") {
        hardwareInst = iType(args, 0x4, 0x13);
    } else if(operation == "ori") {
        hardwareInst = iType(args, 0x6, 0x13);
    } else if(operation == "andi") {
        hardwareInst = iType(args, 0x7, 0x13);
    } else if(operation == "slli") { // NEED A SPECIAL IMM FOR SHAMT
        hardwareInst = iType(args, 0x1, 0x13);
        hardwareInst = (hardwareInst << 7) >> 7;
    } else if(operation == "srli") { // NEED A SPECIAL IMM FOR SHAMT
        hardwareInst = iType(args, 0x5, 0x13);
        hardwareInst = (hardwareInst << 7) >> 7;
    } else if(operation == "srai") { // NEED A SPECIAL IMM FOR SHAMT
        hardwareInst = iType(args, 0x5, 0x13);
        hardwareInst = (hardwareInst << 7) >> 7;
        hardwareInst += 0x40000000;
    } else if(operation == "add") {
        hardwareInst = rType(args, 0x0, 0x0, 0x33);
    } else if(operation == "sub") {
        hardwareInst = rType(args, 0x2, 0x0, 0x33);
    } else if(operation == "sll") {
        hardwareInst = rType(args, 0x0, 0x1, 0x33);
    } else if(operation == "slt") {
        hardwareInst = rType(args, 0x0, 0x2, 0x33);
    } else if(operation == "sltu") {
        hardwareInst = rType(args, 0x0, 0x3, 0x33);
    } else if(operation == "xor") {
        hardwareInst = rType(args, 0x0, 0x4, 0x33);
    } else if(operation == "srl") {
        hardwareInst = rType(args, 0x0, 0x5, 0x33);
    } else if(operation == "sra") {
        hardwareInst = rType(args, 0x2, 0x5, 0x33);
    } else if(operation == "or") {
        hardwareInst = rType(args, 0x0, 0x6, 0x33);
    } else if(operation == "and") {
        hardwareInst = rType(args, 0x0, 0x7, 0x33);
    } else if(operation == "fence") {
        //hardwareInst = IType();
    } else if(operation == "ecall") { // Hard coded instruction.
        hardwareInst = 0x00000073;
    } else if(operation == "ebreak") { // Hard coded instruction.
        hardwareInst = 0x00100073;
    }


    // RV32M Standard Extension:
      else if(operation == "mul") {
        hardwareInst = rType(args, 0x01, 0x0, 0x33);
    } else if(operation == "mulh") {
        hardwareInst = rType(args, 0x01, 0x1, 0x33);
    } else if(operation == "mulhsu") {
        hardwareInst = rType(args, 0x01, 0x2, 0x33);
    } else if(operation == "mulhu") {
        hardwareInst = rType(args, 0x01, 0x3, 0x33);
    } else if(operation == "div") {
        hardwareInst = rType(args, 0x01, 0x4, 0x33);
    } else if(operation == "divu") {
        hardwareInst = rType(args, 0x01, 0x5, 0x33);
    } else if(operation == "rem") {
        hardwareInst = rType(args, 0x01, 0x6, 0x33);
    } else if(operation == "remuw") {
        hardwareInst = rType(args, 0x01, 0x7, 0x33);
    }

    else cout << "Unrecognized instruction: " << operation << "\n\n";


    cout << "\n\tHardware Instruction: " << hex << setfill('0') << setw(8) << hardwareInst << "\n\n";
    return hardwareInst;
}
