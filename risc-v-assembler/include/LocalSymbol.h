#ifndef LOCALSYMBOL_H
#define LOCALSYMBOL_H

#include <string>

using namespace std;

class LocalSymbol
{
    public:
        string symbol;
        int32_t address;

        LocalSymbol();
        virtual ~LocalSymbol();

    protected:

    private:
};

#endif // LOCALSYMBOL_H
