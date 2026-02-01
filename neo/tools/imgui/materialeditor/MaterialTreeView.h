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
#ifndef MATERIALTREEVIEW_H_
#define MATERIALTREEVIEW_H_

#include "../util/ImGui_IdWidgets.h"

#include "idlib/Str.h"
#include "idlib/containers/HashTable.h"
#include "../util/TreeCtrl.h"
#include "MaterialEditor.h"
#include "MaterialView.h"

namespace ImGuiTools {

/**
* Structure used associate a material name with a tree item.
*/
typedef struct {
	idStr		materialName;
	TreeNode*	treeItem;
} MaterialTreeItem_t;

/**
* A tree view of all the materials that have been defined.
*/
class MaterialTreeView : /*public CTreeView,*/ public MaterialView {

public:
					MaterialTreeView();
	virtual			~MaterialTreeView();

	void			InitializeMaterialList(bool includeFile = true, const char* filename = NULL);
	void			BuildMaterialList(bool includeFile = true, const char* filename = NULL);

	bool			Draw( const ImVec2 & size );

	//Material Interface
	virtual void	MV_OnMaterialChange(MaterialDoc* pMaterial);
	virtual void	MV_OnMaterialApply(MaterialDoc* pMaterial);
	virtual void	MV_OnMaterialSaved(MaterialDoc* pMaterial);
	virtual void	MV_OnMaterialAdd(MaterialDoc* pMaterial);
	virtual void	MV_OnMaterialDelete(MaterialDoc* pMaterial);
	virtual void	MV_OnMaterialNameChanged(MaterialDoc* pMaterial, const char* oldName);
	virtual void	MV_OnFileReload(const char* filename);

	bool			CanCopy();
	void			OnCopy();
	bool			CanPaste();
	void			OnPaste();
	bool			CanCut();
	void			OnCut();
	bool			CanDelete();
	void			OnDeleteMaterial();
	bool			CanRename();
	void			OnRenameMaterial();
	bool			CanSaveFile();
	idStr			GetSaveFilename();

	bool			FindNextMaterial(MaterialSearchData_t* searchData);
	TreeNode*		FindNextMaterial(TreeNode *item, MaterialSearchData_t* searchData);
	TreeNode*		GetNextSeachItem(TreeNode *item, bool stayInFile);

	void			DeleteFolder(TreeNode *item, bool addUndo = true);
	TreeNode*		AddFolder(const char* name, TreeNode *parent);
	void			RenameFolder(TreeNode *item, const char* name);

	int				OnCreate();

private:
	//DECLARE_DYNCREATE(MaterialTreeView)

	/**
	* List of tree item types
	*/
	enum {
		TYPE_ROOT = 0,
		TYPE_FOLDER,
		TYPE_FILE,
		TYPE_MATERIAL_FOLDER,
		TYPE_MATERIAL
	};

	//Window Messages
public:
	void			OnTvnSelchanged( bool doubleClicked );
	bool			OnNMRclick( TreeNode *item );
	void			OnTvnBegindrag( TreeNode *item );
	void			OnTvnEndDrag( TreeNode *source, TreeNode *destination );
	void			OnInput( bool prepare, TreeNode *item );
private:
	bool			OnTvnBeginlabeledit( TreeNode *item );
	bool			OnTvnEndlabeledit( TreeNode *item, idStr &text );
	void			OnContextMenu( TreeNode *item );

	//Menu Commands
	void			OnApplyMaterial();
	void			OnApplyFile();
	void			OnApplyAll();
	void			OnSaveMaterial();
	void			OnSaveFile();
	void			OnSaveAll();
	void			OnAddMaterial();
	void			OnAddFolder();
	void			OnDeleteMaterialAccepted();
	void			OnReloadFile();
	void			OnReloadFileAccepted();

	//Internal Messages
	void OnRenameFolderComplete();
	void OnRenameMaterialComplete();

	//Utility methods
	void			RenameMaterial(TreeNode *item, const char* originalName);
	bool			GetFileName(TreeNode *item, idStr& out);
	idStr			GetMediaPath(TreeNode *item, int type);
	void			GetMaterialPaths(TreeNode *item, idList<MaterialTreeItem_t>* list);
	void			AddStrList(const char *root, idStrList *list, bool includeFile);
	void			PopupMenu(TreeNode *item);
	void			SetItemImage(TreeNode *item, bool mod, bool apply, bool children);

	void			OpenMessageBox(int message);

	//Methods for working with the quicktree
	void			CleanLookupTrees(TreeNode *item);
	void			BuildLookupTrees(TreeNode *item);
	idStr			GetQuicktreePath(TreeNode *item);


private:
	//CImageList				m_image;
	bool					treeWithFile;

	TreeCtrl				tree;

	//Hashtables for quick lookups
	idHashTable<TreeNode*>	quickTree;
	idHashTable<TreeNode*>	materialToTree;
	idHashTable<TreeNode*>	fileToTree;


	//Member variables for renaming folders
	TreeNode*				renamedFolder;
	idList<MaterialTreeItem_t> affectedMaterials;
	idStr					newName;

	int						dragImage;
	bool					bDragging;
	//ImVec2					dropPoint;
	TreeNode*				dragItem;

	//Hover Expand
	TreeNode*				hoverItem;
	int						hoverStartTime;

	bool					internalChange;

	enum {
		MSG_BOX_CLOSED						= 0,
		MSG_BOX_UNABLE_TO_RENAME_MATERIAL,
		MSG_BOX_DELETE_FOLDER,
		MSG_BOX_DELETE_MATERIAL,
		MSG_BOX_RELOAD_MODIFIED_FILE,
		MSG_BOX_RENAME_MATERIAL,
	};
	int						messageBox;
};

}

#endif /* !MATERIALTREEVIEW_H_ */
