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

#include <cstdlib>

extern "C" void *GetGameAPI(void *);

extern const char *DYNMODULE_EntryPoint_Sym;
extern const char *DYNMODULE_FindResource_Sym;
extern const char *DYNMODULE_LoadResource_Sym;
extern const char *DYNMODULE_FreeResource_Sym;

#if defined(_D3XP)
const char *DYNMODULE_Name              = "ADoom3:D3XP";
#else
const char *DYNMODULE_Name              = "ADoom3:Base";
#endif
const char *GetGameAPI_Sym              = "GetGameAPI";

dynmod_export_t DYNMODULE_Exports[] =
{
    {(void *)__dynglue_FindResource,    DYNMODULE_FindResource_Sym  },
    {(void *)__dynglue_LoadResource,    DYNMODULE_LoadResource_Sym  },
    {(void *)__dynglue_FreeResource,    DYNMODULE_FreeResource_Sym  },
    {(void *)GetGameAPI,                GetGameAPI_Sym              },
    {0,                                 0                           }
};

dynmod_import_t DYNMODULE_Imports[] =
{
    {0, 0, 0, 0}
};

int DYNMODULE_Setup(void *dmifport)
{
    // No imports ..
    dynmoduleImport();
    return 1;
}

void DYNMODULE_Cleanup(void)
{
}
