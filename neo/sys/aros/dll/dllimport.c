/*
** This file handles the implicit (loadtime) imports.
** For a DLL its called automatically but a normal executable must call it manually
** if it wants to import symbols from a DLL
*/

#define __DLL_LIB_BUILD

#include "dll.h"
#include <stdlib.h>
#include <string.h>

int dllImportSymbols()
{
    dll_tImportSymbol *symtable=DLL_ImportSymbols;  //reference caller's import symbol table

    while(symtable->SymbolPointer) //End of table ??
    {
        void *sym;
        void *h=dllInternalLoadLibrary(symtable->DLLFileName,symtable->DLLPortName,0L);

        if(!h)
                return 0L;

        sym=dllGetProcAddress(h,symtable->SymbolName);

        if(!sym)
            return 0L;

        *symtable->SymbolPointer=sym;

        symtable++;
    }

    
    return 1L; //Success
}
