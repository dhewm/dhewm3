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
#ifndef STAGEVIEW_H_
#define STAGEVIEW_H_

#include "MaterialEditor.h"
#include "MaterialView.h"
//#include "ToggleListView.h"
#include "MaterialView.h"
#include "MaterialPropTreeView.h"

namespace ImGuiTools {

class StageViewItem {
public:
	StageViewItem() : label(), data(0) {
	}
	virtual ~StageViewItem() {
		label.Clear();
	}

	ID_INLINE void SetLabel(const char *_label) {
		label = _label;
	}
	ID_INLINE idStr &GetLabel() {
		return label;
	}

	ID_INLINE int GetData() const {
		return data;
	}
	ID_INLINE void SetData(int _data) {
		data = _data;
	}

private:
	idStr		label;
	int			data;
};

/**
* View that handles managing the material stages.
*/
class StageView : public MaterialView
{

public:
	StageView();
	virtual ~StageView();

	/**
	* Defines the type of stages
	*/
	enum {
		STAGE_TYPE_MATERIAL,
		STAGE_TYPE_STAGE,
		STAGE_TYPE_SPECIAL_MAP_STAGE
	};

	/**
	* Enumeration that defines the possible states of the toggle button.
	*/
	enum {
		TOGGLE_STATE_DISABLED = 0,
		TOGGLE_STATE_ON,
		TOGGLE_STATE_OFF
	};

	//Associates a property view with this stage view
	void					SetMaterialPropertyView(MaterialPropTreeView* propView) { m_propView = propView; };

	//MaterialView Interface
	virtual void			MV_OnMaterialSelectionChange(MaterialDoc* pMaterial);
	virtual void			MV_OnMaterialStageAdd(MaterialDoc* pMaterial, int stageNum);
	virtual void			MV_OnMaterialStageDelete(MaterialDoc* pMaterial, int stageNum);
	virtual void			MV_OnMaterialStageMove(MaterialDoc* pMaterial, int from, int to);
	virtual void			MV_OnMaterialAttributeChanged(MaterialDoc* pMaterial, int stage, const char* attribName);
	virtual void			MV_OnMaterialSaved(MaterialDoc* pMaterial);

	//Edit Operation Tests
	bool					CanCopy();
	void					OnCopy();
	bool					CanPaste();
	void					OnPaste();
	bool					CanCut();
	bool					CanDelete();
	void					OnDeleteStage();
	bool					CanRename();
	void					OnRenameStage();

	//Refresh the stage list
	void					RefreshStageList();

public:
	int						OnCreate();
	bool					Draw( const ImVec2 &size );

private:
	void					DeleteAllItems();
	void					SetItemState(int index);
	int						GetItemCount() const;
	void					DeleteItem( int index );
	int						InsertItem( int index, const char *label );
	void					GetItem( int index, idStr &label );
	int						GetSelectedItemIndex();
	void					SetItemText( int index, const char *label );
	void					SetItemData( int index, int data );
	int						GetItemData( int index );

	//void					SetToggleIcons(LPCSTR disabled = NULL, LPCSTR on = NULL, LPCSTR off = NULL);
	void					SetToggleState(int index, int toggleState, bool notify = false);
	int						GetToggleState(int index);

protected:
	void					OnLvnItemchanged();
	void					OnLvnDeleteallitems();
	
	void					OnLvnBegindrag( int index );
	void					OnLvnEnddrag( int index );
	void					OnNMRclick();
	void					OnNMClick( int index );

	void					OnRenameStageAccepted();
	void					OnDeleteStageAccepted();
	void					OnDeleteAllStages();
	void					OnDeleteAllStagesAccepted();
	void					OnAddStage();
	void					OnAddBumpmapStage();
	void					OnAddDiffuseStage();
	void					OnAddSpecualarStage();

	void					OnPasteAccepted();
	
	bool					OnLvnBeginlabeledit( int index );
	void					OnLvnEndlabeledit( int index, const char *newLabel );
	
	void					OnChar( bool prepare, int index );

	//Toggle List View Interface
	void					OnStateChanged(int index, int toggleState);
	
	void					PopupMenu();
	
	void					DropItemOnList( int dropIndex );

protected:

	MaterialPropTreeView*	m_propView;
	MaterialDoc*			currentMaterial;

	enum {
		MSG_BOX_CLOSED						= 0,
		MSG_BOX_RENAME_STAGE,
		MSG_BOX_DELETE_STAGE,
		MSG_BOX_DELETE_ALL_STAGES,
		MSG_BOX_REPLACE_STAGE,
	};
	int						messageBox;

	idStr					labelEdit;
	int						labelIndex;
	idStr					replaceStageName;

	idList<StageViewItem>	items;
	int						itemCurSel;

	int						lastKeyDownTime;

	//Manual handing of the row dragging
	/*
	CImageList*				dragImage;
	*/
	bool					bDragging;

	int						dragIndex;
	int						dropIndex;

	bool					internalChange;
};

}

#endif /* !STAGEVIEW_H_ */
