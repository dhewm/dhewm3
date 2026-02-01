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

#include "ImGui_IdWidgets.h"
#include "imgui.h"

#include "TreeCtrl.h"

namespace ImGuiTools {

void TreeCtrl::DeleteAllItems() {
	DeleteAllItemsOfNode( root );
	numNodes = 1;
}

void TreeCtrl::DeleteItem( TreeNode* item ) {
	if ( !item ) {
		return;
	}
	if ( item == root ) {
		DeleteAllItems();
		return;
	}

	DeleteAllItemsOfNode( item );
	SelectItem( NULL ); // TODO: figure out what to do with it?
}

TreeNode *TreeCtrl::GetRootItem() const {
	return const_cast<TreeNode *>( root );
}

void TreeCtrl::SetItemText( TreeNode *item, const char *text ) {
	assert( item );
	item->SetLabel( text );
}

const idStr& TreeCtrl::GetItemText( TreeNode *item ) const {
	assert( item );
	return item->GetLabel();
}

TreeNode *TreeCtrl::GetChildItem( TreeNode *item ) const {
	if ( item ) {
		return item->GetNode().GetChild();
	}
	return NULL;
}

TreeNode *TreeCtrl::GetParentItem( TreeNode *item ) const {
	if ( item ) {
		TreeNode *parent = item->GetNode().GetParent();
		return ( parent != root ) ? parent : NULL;
	}
	return NULL;
}

TreeNode *TreeCtrl::GetNextSiblingItem( TreeNode *item ) const {
	if ( item ) {
		return item->GetNode().GetSibling();
	}
	return NULL;
}

TreeNode *TreeCtrl::GetSelectedItem() const {
	return selectedItem;
}

void TreeCtrl::SelectItem( TreeNode *item ) {
	selectedItem = item;
	UpdatePathToSelectedItem();
}

int SortTreeNodeByLabel( TreeNode * const *a, TreeNode * const *b ) {
	return (*a)->GetLabel().Icmp( (*b)->GetLabel() );
}

void TreeCtrl::SortChildren( TreeNode *item ) {
	TreeNode *child = GetChildItem( item );
	if ( !child ) {
		return;
	}
	idList<TreeNode *> list;

	while ( child ) {
		list.Append( child );
		child = GetNextSiblingItem( child );
	}
	list.Sort( SortTreeNodeByLabel );

	int i, n = list.Num();
	for ( i = 0; i < n; i++ ) {
		list[i]->GetNode().ParentTo( item->GetNode() );
	}
}

void TreeCtrl::EditLabel( TreeNode *item ) {
	// TODO: implement
}

TreeNode *TreeCtrl::InsertItem( const idStr &name, TreeNode *parent, TreeNode *after ) {
	if ( !parent && name.IsEmpty() ) {
		// do not insert the root node again
		return root;
	}
	TreeNode *newNode = nodeAllocator.Alloc();
	int id = numNodes++;
	newNode->Init( id );
	newNode->SetLabel( name );
	if ( after ) {
		newNode->GetNode().MakeSiblingAfter( after->GetNode() );
	} else if ( parent ) {
		newNode->GetNode().ParentTo( parent->GetNode() );
	} else {
		newNode->GetNode().ParentTo( root->GetNode() );
	}

	return newNode;
}

void TreeCtrl::SetItemData( TreeNode *item, int data ) {
	assert( item );
	item->SetItem( data );
}

int TreeCtrl::GetItemData( TreeNode *item ) const {
	assert( item );
	return item->GetItem();
}

void TreeCtrl::Draw( treeItemTooltip_t tooltip, treeItemSelected_t selected, treeItemContextMenu_t contextMenu, treeItemBeginDrag_t beginDrag, treeItemEndDrag_t endDrag, treeItemInput_t input, void *data ) {
	TreeNode *parentNode = GetRootItem();
	TreeNode *node;

	for ( node = GetChildItem( parentNode ) ; node ; node = GetNextSiblingItem( node ) ) {
		DrawNode( node, tooltip, selected, contextMenu, beginDrag, endDrag, input, data );
	}
}

void TreeCtrl::UpdatePathToSelectedItem() {
	pathToSelectedItem.Clear();
	if ( !selectedItem ) {
		return;
	}

	TreeNode *node = selectedItem;
	while ( node ) {
		pathToSelectedItem.Insert( node->GetID() );
		node = GetParentItem( node );
	}
}

void TreeCtrl::DrawNode( TreeNode *node,  treeItemTooltip_t tooltip, treeItemSelected_t selected, treeItemContextMenu_t contextMenu, treeItemBeginDrag_t beginDrag, treeItemEndDrag_t endDrag, treeItemInput_t input, void *data ) {
	TreeNode *			item;
	TreeNode *			childItem;
	ImGuiTreeNodeFlags	flags = 0;
	idStr				tooltipText;
	bool				scrollToNode = false;
	bool				openNode = false;

	childItem = GetChildItem( node );

	if ( node == selectedItem ) {
		flags |= ImGuiTreeNodeFlags_Selected;
		scrollToNode = true;
	}
	if ( !childItem ) {
		flags |= ImGuiTreeNodeFlags_Leaf;
		flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
	} else {
		if ( pathToSelectedItem.Num() && pathToSelectedItem[0] == node->GetID() ) {
			openNode = true;
			flags |= ImGuiTreeNodeFlags_DefaultOpen;
			pathToSelectedItem.RemoveIndex( 0 );
		}
	}

	input( data, true, node );

	if ( openNode ) {
		ImGui::SetNextItemOpen( openNode );
	}

	bool opened = ImGui::TreeNodeEx( static_cast<const void *>(node), flags, "%s", node->GetLabel().c_str() );
	if ( scrollToNode ) {
		ImGui::SetScrollHereY();
	}
	contextMenu( data, node );
	if ( opened ) {
		if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) && !ImGui::IsItemToggledOpen() ) {
			SelectItem( node );
			selected( data, false );
		}
		if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) ) {
			SelectItem( node );
			selected( data, true );
		}
		if ( ImGui::IsItemHovered() ) {
			tooltipText.Clear();
			if ( tooltip( data, node, tooltipText ) ) {
                ImGui::BeginTooltip();
				ImGui::TextUnformatted( tooltipText.c_str() );
                ImGui::EndTooltip();
			}
		}
		if ( ImGui::IsItemFocused() ) {
			input( data, false, node );
		}
		if ( ImGui::BeginDragDropSource( ImGuiDragDropFlags_None ) )
		{
			ImGui::SetDragDropPayload( "TreeCtrl", node, sizeof( TreeNode * ) );

			// Display preview
			ImGui::Text( "%s", node->GetLabel().c_str() );
			ImGui::EndDragDropSource();
			beginDrag( data, node );
		}
		if ( ImGui::BeginDragDropTarget() )
		{
			if ( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "TreeCtrl" ) )
			{
				IM_ASSERT( payload->DataSize == sizeof( TreeNode * ) );
				TreeNode *dropSource = ( TreeNode * )payload->Data;
				endDrag( data, dropSource, node );
			}
			ImGui::EndDragDropTarget();
		}
		
		for ( item = childItem; item; item = GetNextSiblingItem( item ) ) {
			DrawNode( item, tooltip, selected, contextMenu, beginDrag, endDrag, input, data );
		}

		if ( childItem ) {
			ImGui::TreePop();
		}
	}
}

void TreeCtrl::DeleteAllItemsOfNode( TreeNode *node ) {
	if ( !node ) {
		return;
	}

	TreeNode *child = GetChildItem( node );
	while ( child ) {
		DeleteAllItemsOfNode( child );
		child = GetChildItem( node );
	}
	if ( node != root ) {
		node->Shutdown();
		nodeAllocator.Free( node );
	}
}

}
