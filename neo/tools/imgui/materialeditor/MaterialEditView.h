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
#ifndef MATERIALEDITVIEW_H_
#define MATERIALEDITVIEW_H_

#include "../util/ImGui_IdWidgets.h"
#include "sys/sys_imgui.h"

#include "MaterialEditor.h"
#include "MaterialView.h"
#include "MaterialPropTreeView.h"
#include "StageView.h"

#include "../util/SyntaxRichEditCtrl.h"

namespace ImGuiTools {

/**
* View that contains the material edit controls. These controls include
* the stage view, the properties view and the source view.
*/
class MaterialEditView : public MaterialView, SourceModifyOwner {

public:
	idStr						m_nameEdit;
	float						m_editSplitterPos;
	float						m_editSplitterWidth;
	float						m_editSplitterHeight;

	StageView*					m_stageView;
	MaterialPropTreeView*		m_materialPropertyView;
	SyntaxRichEditCtrl			m_textView;

public:
					MaterialEditView();
	virtual			~MaterialEditView();

	bool			Draw( const ImVec2 &size );

	//MaterialView Interface
	virtual void	MV_OnMaterialSelectionChange( MaterialDoc* pMaterial );
	virtual void	MV_OnMaterialNameChanged( MaterialDoc* pMaterial, const char* oldName );

	//SourceModifyOwner Interface
	virtual idStr GetSourceText();

	bool			CanCopy();
	void			OnCopy();
	bool			CanCut();
	void			OnCut();
	bool			CanPaste();
	void			OnPaste();
	bool			CanDelete();
	void			OnDelete();

protected:
	void			GetMaterialSource();
	void			ApplyMaterialSource();

public:
	//Message Handlers
	void			OnCreate();

protected:
	//afx_msg void	OnSize(UINT nType, int cx, int cy);
	void			OnTcnSelChange(int sel);
	void			OnEnChangeEdit();

protected:
	bool	initHack;
	bool	sourceInit;

	bool	sourceChanged;
	idStr	currentMaterialName;
	bool	propsTabSelected;
	bool	textTabSelected;
	int		tabSel;
};

}

#endif /* !MATERIALEDITVIEW_H_ */
