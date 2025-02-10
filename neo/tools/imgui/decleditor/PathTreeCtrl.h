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

namespace ImGuiTools {

/*
===============================================================================

	Tree Control for path names.

===============================================================================
*/

class PathTreeNode {
public:
	PathTreeNode()
		: label()
		, item(0)
		, node()
		, id(-1)
	{
		
	}
	~PathTreeNode() {
		Shutdown();
	}

	void Init(int _id)
	{
		label.Clear();
		item = 0;
		node.SetOwner(this);
		id = _id;
	}

	void Shutdown()
	{
		node.RemoveFromHierarchy();
		label.Clear();
		id = 0;
	}

	ID_INLINE int					GetItem() {
		return item;
	}
	ID_INLINE void					SetItem( int _item ) {
		item = _item;
	}
	ID_INLINE void					SetLabel( const char *_label ) {
		label = _label;
	}
	ID_INLINE idStr &				GetLabel() {
		return label;
	}
	ID_INLINE idHierarchy<PathTreeNode>& GetNode() {
		return node;
	}
	ID_INLINE int				GetID() {
		return id;
	}

private:
	idStr						label;
	int							item;
	idHierarchy<PathTreeNode>	node;
	int							id;
};

class idPathTreeStack {
public:
						idPathTreeStack( void ) { size = 0; }

	void				PushRoot( PathTreeNode *root );
	void				Push( PathTreeNode *item, const char *name );
	void				Pop( void ) { size--; }
	PathTreeNode *		TopItem( void ) const { return stackItem[size-1]; }
	const char *		TopName( void ) const { return stackName[size-1]; }
	int					TopNameLength( void ) const { return stackName[size-1].Length(); }
	int					Num( void ) const { return size; }

private:
	int					size;
	PathTreeNode *		stackItem[128];
	idStr				stackName[128];
};

ID_INLINE void idPathTreeStack::PushRoot( PathTreeNode *root ) {
	assert( size == 0 );
	stackItem[size] = root;
	stackName[size] = "";
	size++;
}

ID_INLINE void idPathTreeStack::Push( PathTreeNode *item, const char *name ) {
	assert( size < 127 );
	stackItem[size] = item;
	stackName[size] = stackName[size-1] + name + "/";
	size++;
}

typedef bool (*treeItemCompare_t)( void *data, PathTreeNode *item, const char *name );
typedef bool (*treeItemTooltip_t)( void *data, PathTreeNode *item, idStr &tooltipText );
typedef void (*treeItemSelected_t)( void *data );

class PathTreeCtrl {
public:
	PathTreeCtrl()
		: nodeAllocator()
		, root()
		, selectedItem(NULL)
		, numNodes(0)
	{
		root = nodeAllocator.Alloc();
		root->Init(0);
		numNodes++;
	}
	~PathTreeCtrl() {
		DeleteAllItems();
	}

	void							DeleteAllItems();
	PathTreeNode *					GetRootItem() const;
	idStr&							GetItemText( PathTreeNode *item ) const;
	PathTreeNode *					GetParentItem( PathTreeNode *item ) const;
	PathTreeNode *					GetChildItem( PathTreeNode *item ) const;
	PathTreeNode *					GetNextSiblingItem( PathTreeNode *item ) const;
	PathTreeNode *					InsertItem( const idStr &name, PathTreeNode *parent = NULL, PathTreeNode *after = NULL );
	void							SetItemData( PathTreeNode* item, int data );
	int								GetItemData( PathTreeNode *item ) const;
	PathTreeNode *					GetSelectedItem() const;
	void							SelectItem( PathTreeNode *item );

	void							Draw( treeItemTooltip_t tooltip, treeItemSelected_t selected, void *data );

	PathTreeNode *					FindItem( const idStr &pathName );
	PathTreeNode *					InsertPathIntoTree( const idStr &pathName, const int id );
	PathTreeNode *					AddPathToTree( const idStr &pathName, const int id, idPathTreeStack &stack );
	int								SearchTree( treeItemCompare_t compare, void *data, PathTreeCtrl &result );

private:
	void							DrawNode( PathTreeNode *item,  treeItemTooltip_t tooltip, treeItemSelected_t selected, void *data );
	void							DeleteAllItemsOfNode( PathTreeNode *item );

	idBlockAlloc<PathTreeNode,256>	nodeAllocator;
	PathTreeNode *					root;
	PathTreeNode *					selectedItem;
	int								numNodes;
};

}

#endif /* !__PATHTREECTRL_H__ */
