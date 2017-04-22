/*
** This file contains glue linked into the "shared" object
*/

#define DEBUG 1

#include <aros/debug.h>
#include <exec/types.h>

#include "sys/aros/dll/dll.h"

extern void *GetGameAPI(void *);

void* dllFindResource(int id, char *pType)
{
    return NULL;
}

void* dllLoadResource(void *pHandle)
{
    return NULL;
}

void dllFreeResource(void *pHandle)
{
    return;
}

dll_tExportSymbol DLL_ExportSymbols[] =
{
    {dllFindResource, "dllFindResource"},
    {dllLoadResource, "dllLoadResource"},
    {dllFreeResource, "dllFreeResource"},
    {(void *)GetGameAPI, "GetGameAPI"},
    {0,0}
};

dll_tImportSymbol DLL_ImportSymbols[] =
{
    {0,0,0,0}
};

int DLL_Init(void)
{
    return 1;
}

void DLL_DeInit(void)
{
}
