/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include <dynmod/dynmodule.h>

#include <cstring>
#include <cstdlib>

#include "sys/aros/aros_dynmodule.h"

const char *DYNMODULE_EntryPoint_Sym = EntryPoint_Str;
const char *DYNMODULE_FindResource_Sym = FindResource_Str;
const char *DYNMODULE_LoadResource_Sym = LoadResource_Str;
const char *DYNMODULE_FreeResource_Sym = FreeResource_Str;

int DYNMODULE_Init(void *slotbase, unsigned long int size, stdexitfunc_t exitfunc)
{
    static int initflag = 0;
    if (!initflag) {
        memset(slotbase, 0, size);
        if (atexit(exitfunc))
            return 0;
        else
            initflag = 1;
    }
    return 1;
}

void DYNMODULE_Exit(int exitcode)
{
    exit(exitcode);
}
