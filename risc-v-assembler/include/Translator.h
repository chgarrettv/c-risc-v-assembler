/*
Written by Claude Garrett V.
*/

#ifndef TRANSLATOR_H
#define TRANSLATOR_H
#include <string>

using namespace std;

class Translator
{
    public:
        Translator();
        virtual ~Translator();

        string stringTrimmer(string inst);
        string labelIsolator(string label);
        string* instrToOperands(string assembly, string* args);

        int argToReg(string arg);
        int argToImm(string arg);

        int rType(string* args, int f7, int f3, int op);
        int iType(string* args, int f3, int op);
        int sType(string* args, int f3, int op);
        int bType(string* args, int f3, int op);
        int uType(string* args, int op);
        int jType(string* args, int op);
        bool typeTesting();

        uint32_t toMachineLanguage(string* arg);


    protected:

    private:
};

#endif // TRANSLATOR_H
