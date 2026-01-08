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

#ifndef __PATHTREECTRL_H__
#define __PATHTREECTRL_H__

#include "idlib/Str.h"
#include "idlib/Heap.h"
#include "idlib/containers/Hierarchy.h"
#include "../util/TreeCtrl.h"

namespace ImGuiTools {

/*
===============================================================================

	Tree Control for path names.

===============================================================================
*/

class idPathTreeStack {
public:
						idPathTreeStack( void ) { size = 0; }

	void				PushRoot( TreeNode *root );
	void				Push( TreeNode *item, const char *name );
	void				Pop( void ) { size--; }
	TreeNode *			TopItem( void ) const { return stackItem[size-1]; }
	const char *		TopName( void ) const { return stackName[size-1]; }
	int					TopNameLength( void ) const { return stackName[size-1].Length(); }
	int					Num( void ) const { return size; }

private:
	int					size;
	TreeNode *			stackItem[128];
	idStr				stackName[128];
};

ID_INLINE void idPathTreeStack::PushRoot( TreeNode *root ) {
	assert( size == 0 );
	stackItem[size] = root;
	stackName[size] = "";
	size++;
}

ID_INLINE void idPathTreeStack::Push( TreeNode *item, const char *name ) {
	assert( size < 127 );
	stackItem[size] = item;
	stackName[size] = stackName[size-1] + name + "/";
	size++;
}

class PathTreeCtrl : public TreeCtrl {
public:
	PathTreeCtrl()
	{
	}
	~PathTreeCtrl() {
	}

	TreeNode *						FindItem( const idStr &pathName );
	TreeNode *						InsertPathIntoTree( const idStr &pathName, const int id );
	TreeNode *						AddPathToTree( const idStr &pathName, const int id, idPathTreeStack &stack );
	int								SearchTree( treeItemCompare_t compare, void *data, PathTreeCtrl &result );
};

}

#endif /* !__PATHTREECTRL_H__ */
