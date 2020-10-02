/*
Written by Claude Garrett V.

This program was designed as a basic assembler for RV32I.

*/

#include <iostream>
#include <fstream>
#include <iomanip>

#include <map>
#include <vector>

#include <Translator.h>
#include <LocalSymbol.h>

using namespace std;

string labelType(string operand) {
    if(operand[0] == '.') return "local";
    if(operand.find_first_not_of("0123456789") == -1) return "local";

    return "global";
}

string immType(string operand) {
    if(operand[0] == '0' && operand[1] == 'x') return "hex"; // Hexidecimal.
    if(operand.find_first_not_of("0123456789") == -1) return "dec"; // Decimal number. Convert.

    if(operand[0] == '.') return "local"; // Starts with period means local.
    // If it ends with either f or b, and the rest is only numbers, it's a local variable.
    if((operand[operand.length() - 1] == 'f' || operand[operand.length() - 1] == 'b') && operand.find_first_not_of("0123456789") == (operand.length() -1)) return "local";

    return "global"; // Otherwise, it has to be a global variable.
}

int main()
{
    // How I think it will all work:
    /*  Take 3:
            First Pass:
                text:
                    First, trim the line of comments and leading white space.
                    If there is no content, then skip the line.

                    If it's a label:
                        See if it's global. If it is, add it to the symbol table with pc.
                        If it's local (only a number with colon, or starts with '.'):
                            Make a local symbol table, which will be a vector of LocalSymbol classes.
                                Classes will contain this data: Name and address.
                            When you encounter a local symbol, simply add a LocalSymbol class to the end of the vector.
                        After the above, continue as in-line labels are not to be supported yet.

                    If it's not a label, it should be an instruction.
                        Translate it into arguments properly using instToOperands().
                        If it's no op, skip it. This is so you can reassemble the abridged output and not accumulate no ops.
                        If it's a jump, branch, load, or store:
// HERE                         Figure out if the immediate is a label or a number.
                                Note that jumps, branches, loads, and stores can have their immediate in different places.
                                Label:
                                    If it's a backwards reference, convert it to an address.
                                    If it's a forward reference, keep it as a label.
                                    Add the necessary amount of no ops if it's forward.
                                Number:
                                    Translate it to hex if it's not, so that it can be skipped easier in the second pass.
                                    Assume the programmer is smart enough to properly jump if it's a number, otherwise they should use a pseudo instruction.
                        Add the instruction arguments to the text vector.

                Append an infinite loop to the end of the text section.

                data:
                    Find the .data keyword.
                    You should have the final instruction number after the text section is determined.
                    Add data as necessary to the data vector.
                        You'll probably want a way to store a string of data, using .string or whatever they do.

            Second Pass: This really just goes back over the data stored in the vectors, so you can close the streams.
                Just go over the entire text vector.
                You'll output in two places: You can start by dumping the assembly as is in the text vector to the abridged assembly file.
                    Add a comment on the end with the line number for compatibility, so it can be re-assembled.
                Next, read through the current instruction and do a few things:
                    If it's a jump, branch, load, or store, make sure to correct the far branches.
                        Jumps:
                            You have a no op after you instruction so you're going to use that now.
                            For the current instruction, write a auipc instruction for a relative jump. (sets rd to pc + 20 bit high immediate)
                            Then, write the old jump as the next instruction. You'll have to be sure the offset is still right!
                            If the last bit, bit 11, is set you might do some accidental subtraction, so add one to the upper imm. to correct for this.
                            Continue. Be sure to add 2 to the instruction counter so the no ops are skipped, though.
                        Loads and stores are pretty much the same as jumps, but be sure you're using the right argument as the immediate!
                        Branches:
                            This one is a little more involved. If it's far you'll have to utilize the next two no ops.
                            First, change your branch type to be the OPPOSITE of what it is now. I.e., ben means branch when equal, branch less than means gte.
                            Set the offset of this new branch to 2(?) so it's just after the next two instructions.
                            Then, place a relative or absolute jump after the branch. This path will be taken when false instead of true now and the jump will occur.
                            Write auipc/lui as necessary as the next op.
                            Write the next jump as necessary for the next (3rd) op.
                            Continue, but be sure to add 3 to the instruction counter!


    */

    // Initialize Classes:
    Translator translator;

    // Data Structures for Assembly:
    map<string, uint32_t> symbolMap;
    vector<LocalSymbol> localSymbols;
    vector<string*> text;
    vector<string> data;

    // Settings:
    string programFile = "assembly.txt";
    string outputFile = "output.txt";

    // Open the input and output files.
    ifstream prog; prog.open(programFile);
    ofstream out; out.open(outputFile);

    // Start reading in the input file.
    int lineNumber = 0;
    uint32_t hardwareInst = 0;

    // First Pass:
    string inst = "";

    string operands[5] = {"", "", "", "", ""};
    string nop[4] = {"addi", "x0", "x0", "0"}; // Constant

    int instructionNumber = 0; // This is the effective instruction line, as it will be in the computer memory.
    int progLine = -1; // This is the line of the program file. For debugging.


    // First Pass:
    while(getline(prog, inst)) {

        progLine++;
        inst = translator.stringTrimmer(inst); // Cut out the white space and comments.
        if(inst == "") {cout << "Line " << progLine << ": Skipping comment/blank line...\n"; continue;} // The line was just a comment, skip it.
        cout << "Line " << progLine << ": \"" << inst << "\"\n";

        // Labels:
        if(inst.find(":") != -1) {
            inst = inst.substr(0, inst.find(":")); // Remove the ':' from the end for easier reading.
            if(labelType(inst) == "local") {
                cout << "\tLocal label found: \"" << inst << "\", saving as alias for " << instructionNumber << "\n\n";

                LocalSymbol loc;
                loc.address = instructionNumber;
                loc.symbol = inst;
                localSymbols.push_back(loc);

                continue;
            } else if (labelType(inst) == "global") {
                cout << "\tGlobal label found: \"" << inst << "\", saving as alias for " << instructionNumber << "\n\n";

                symbolMap[inst] = instructionNumber;

                continue;
            }
        }

        // Instructions:
        translator.instrToOperands(inst, operands); // Turn into arguments format.

        int matches = 0;
        for(int i = 0; i < 4; i++) { // Make sure it's not a no op. If it is, skip it.
            if(operands[i] == nop[i]) matches++;
        }
        if(matches == 4) {
            cout << "\tNo op, skipping...\n";
            continue;
        }


        string op = operands[0];
        if(op == "jal") {
            // Argument 2 will be the immediate for jal.

            string type = immType(operands[2]);

            if(type == "hex" || type == "dec") {

            } else if(type == "local") {
                if(operands[2][operands[2].length() - 1] == 'f') { // Forward .f reference.
                    // Basically do nothing, pass 2 will resolve this...
                    test.push_back(operands);
                    text.push_back(nop);
                    continue;
                } else { // Means it's either a # or .f label, both will be treated the same way.
                    // You'll search your backlog of local labels to hopefully find it.
                    // If not, it's probably a forward ref that you haven't encountered yet, so fix in pass 2.
                    int index = -1;
                    for(int i = localSymbols.size(); i >= 0; i--) { // Go through the local symbols backwards, since local are overwritten.
                        if(localSymbols[i].symbol == operands[2]) {
                            operands[2] = localSymbols[i].address;
                            index = i;
                            break;
                        }
                    }

                    if(index != -1) { // Found the label in the above loop. No need for no ops.
                        text.push_back(operands);
                        continue;
                    } else { // Haven't found the label yet, will resolve in pass 2.
                        test.push_back(operands);
                        text.push_back(nop);
                        continue;
                    }

                }

            } else if(type == "global") {
                if(symbolMap.find(operands[2]) != symbolMap.end()) { // If the global label is already defined, replace it. You won't have to push no ops.
                    operands[2] = symbolMap[operands[2]];
                } else {
                    text.push_back(operands);
                    text.push_back(nop);
                    continue;
                }
            }

        } else if(op == "jalr" ||
                  op == "beq" ||
                  op == "bne" ||
                  op == "blt" ||
                  op == "bge" ||
                  op == "bltu" ||
                  op == "bgeu" ||
                  op == "lb" ||
                  op == "lh" ||
                  op == "lw" ||
                  op == "lbu" ||
                  op == "lhu" ||
                  op == "sb" ||
                  op == "sh" ||
                  op == "sw") {
            // For these, the immediate will be argument 3.
        }


    }

    // Second Pass:

    // Close all of the files.
    prog.close();
    out.close();

    //TypeTesting();

    return 0;
}

/* Old comments.

        There will have to be a linked list of strings.

        The assembler will work in one pass of the file, but it will go over it again in memory.

        Operation :
            You'll go through and do this after trimming comments and leading whitespace:
                text:
                    You'll execute directives as you encounter them.
                    Add valid instructions to an instruction vector.
                    And add labels to a symbol table/map.
                    If you find a forward reference, just add it to the symbol table and add no ops accordingly after it.

                data:
                    You'll need another vector for data.
                    Simply read it in during the first pass.

            Next, you'll go through your instruction vector and replace all of your labels with proper addresses.
                text:
                    You'll start by replacing any labels with the correct offset based on the symbol table.
                    You may need to replace single command jumps with two or more instructions, just replace the no ops you made earlier.

                data:
                    You'll offset all of the data addresses by the length of the text vector.

            Finally, you'll output the machine language vector to the output file.


            Operation Take 2: Done Quickly
            First Pass:
                text:
                    Execute any directives as you encounter them.
                    Make a symbol table as you go.
                    Backwards references can be resolved as they occur, there is no need to insert no ops.
                    Any forward references should be padded with no ops:
                        Add a no op after any unconditional jumps.
                        Add two no ops after conditional branches.
                        Add a no op after any load/store operations.
                    Resolve any backwards references as they occur, no ops are NOT needed.

                data:
                    Set up with addresses that occur directly after the text segment.

            Second Pass:
                text:
                    This will purely be a pass of the instruction vector.
                    Resolve forward references with the symbol table.
                        Far jump? Delete the no op after the jump, and add a LUI before it, loading the top 20 bits into the imm.
                            IF BIT 11 (LAST) OF IMM IS 1, ADD ONE TO THE LUI IMMEDIATE TO COMPENSATE FOR SUBTRACTION.
                        Far branch? Turn into branch to opposite branch type with offset of three, then lui and jump.
                            This will make you branch OVER the jump if you previously DIDN'T want to jump, and if you do jump still, you'll no go on to the lui jump.
                        Far load/store? Add in a lui before it (current inst. number), then place your corrected load/store.

                    Translate all instructions in place while you do the above.
                    End the program with an infinite loop of jump -1 so data is not operated as instructions.

                data:
                    After you finish the text, just slap the data on the end. */
