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

#ifndef __TREECTRL_H__
#define __TREECTRL_H__

#include "idlib/Str.h"
#include "idlib/Heap.h"
#include "idlib/containers/Hierarchy.h"
#include "idlib/containers/List.h"

namespace ImGuiTools {

/*
===============================================================================

	Tree Control for path names.

===============================================================================
*/

class TreeNode {
public:
	TreeNode()
		: label()
		, item(0)
		, node()
		, id(-1)
		, imageId(-1)
	{
		
	}
	virtual ~TreeNode() {
		Shutdown();
	}

	void Init(int _id)
	{
		label.Clear();
		item = 0;
		node.SetOwner(this);
		id = _id;
		imageId = -1;
	}

	virtual void Shutdown()
	{
		node.RemoveFromParent();
		label.Clear();
		id = 0;
		imageId = -1;
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
	ID_INLINE const idStr &				GetLabel() const {
		return label;
	}
	ID_INLINE idHierarchy<TreeNode>& GetNode() {
		return node;
	}
	ID_INLINE int				GetID() {
		return id;
	}
	ID_INLINE void				SetImageID( int _imageId ) {
		imageId = _imageId;
	}
	ID_INLINE int				GetImageID() {
		return imageId;
	}

private:
	idStr						label;
	int							item;
	idHierarchy<TreeNode>		node;
	int							id;
	int							imageId;
};

typedef bool (*treeItemCompare_t)( void *data, TreeNode *item, const char *name );
typedef bool (*treeItemTooltip_t)( void *data, TreeNode *item, idStr &tooltipText );
typedef void (*treeItemSelected_t)( void *data, bool doubleClick );
typedef void (*treeItemContextMenu_t)( void *data, TreeNode *item );
typedef void (*treeItemBeginDrag_t)( void *data, TreeNode *source );
typedef void (*treeItemEndDrag_t)( void *data, TreeNode *source, TreeNode *destination );
typedef void (*treeItemInput_t)( void *data, bool prepare, TreeNode *item );

class TreeCtrl {
public:
	TreeCtrl()
		: nodeAllocator()
		, root()
		, selectedItem(NULL)
		, pathToSelectedItem()
		, numNodes(0)
	{
		root = nodeAllocator.Alloc();
		root->Init(0);
		numNodes++;
	}
	virtual ~TreeCtrl() {
		DeleteAllItems();
		pathToSelectedItem.Clear();
	}

	void							DeleteAllItems();
	void							DeleteItem( TreeNode *item );
	TreeNode *						GetRootItem() const;
	void							SetItemText( TreeNode *item, const char *text );
	const idStr&					GetItemText( TreeNode *item ) const;
	TreeNode *						GetParentItem( TreeNode *item ) const;
	TreeNode *						GetChildItem( TreeNode *item ) const;
	TreeNode *						GetNextSiblingItem( TreeNode *item ) const;
	TreeNode *						InsertItem( const idStr &name, TreeNode *parent = NULL, TreeNode *after = NULL );
	void							SetItemData( TreeNode *item, int data );
	int								GetItemData( TreeNode *item ) const;
	TreeNode *						GetSelectedItem() const;
	void							SelectItem( TreeNode *item );
	void							SortChildren( TreeNode *item );
	void							EditLabel( TreeNode *item );

	void							Draw( treeItemTooltip_t tooltip, treeItemSelected_t selected, treeItemContextMenu_t contextMenu, treeItemBeginDrag_t beginDrag, treeItemEndDrag_t endDrag, treeItemInput_t input, void *data );

private:
	void							DrawNode( TreeNode *item,  treeItemTooltip_t tooltip, treeItemSelected_t selected, treeItemContextMenu_t contextMenu, treeItemBeginDrag_t beginDrag, treeItemEndDrag_t endDrag, treeItemInput_t input, void *data );
	void							UpdatePathToSelectedItem();
	void							DeleteAllItemsOfNode( TreeNode *item );

	idBlockAlloc<TreeNode,256>		nodeAllocator;
	TreeNode *						root;
	TreeNode *						selectedItem;
	idList<int>						pathToSelectedItem;
	int								numNodes;
};

}

#endif /* !__TREECTRL_H__ */
