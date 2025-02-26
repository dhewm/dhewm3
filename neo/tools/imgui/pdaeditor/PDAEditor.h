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

#ifndef __PDAEDITOR_H__
#define __PDAEDITOR_H__

#include "idlib/Dict.h"
#include "../../edit_public.h"

class idDeclEmail;
class idDeclAudio;
class idDeclVideo;

namespace ImGuiTools
{

/////////////////////////////////////////////////////////////////////////////
// CCDialogPDAEditor dialog

class DialogPDAAdd {
public:
	DialogPDAAdd();

	void					Reset();
	bool					Draw();

	idStr					name;
};

class DialogPDAEditEmail {
public:
	DialogPDAEditEmail();   // standard constructor

	void					Reset();
	bool					Draw();

	void					SetName( idStr& name );
	idStr &					GetName();
	void					SetEmail( const idDeclEmail* email );

	idStr					GetDeclText();

private:
	idStr					to;
	idStr					from;
	idStr					date;
	idStr					subject;
	idStr					body;

	idStr					name;
};

class DialogPDAEditAudio {
public:
	DialogPDAEditAudio();

	void					Reset();
	bool					Draw();

	void					SetName( idStr &name );
	idStr &					GetName();
	void					SetAudio( const idDeclAudio *audio );

	idStr					GetDeclText();

private:
	idStr					wave;
	idStr					audioName;
	idStr					info;
	idStr					preview;

	idStr					name;
};

class DialogPDAEditVideo {
public:
	DialogPDAEditVideo();

	void					Reset();
	bool					Draw();

	void					SetName( idStr &name );
	idStr &					GetName();
	void					SetVideo( const idDeclVideo *video );

	idStr					GetDeclText();

private:
	idStr					preview;
	idStr					video;
	idStr					videoName;
	idStr					info;
	idStr					audio;

	idStr					name;
};

class PDAEditor {
public:
							PDAEditor();   // standard constructor

	static PDAEditor&	Instance();

	void					OnInitDialog();

	void					Reset();
	void					Draw();

	ID_INLINE void			ShowIt(bool show) {
		isShown = show;
	}
	ID_INLINE bool			IsShown() {
		return isShown;
	}

protected:
	void					OnMove( int x, int y );
	void					OnDestroy();

	void					OnSelChangePDA( int index );

	void					OnBtnClickedSave();
	void					OnBtnClickedRandom();

	void					OnBtnClickedPDAAdd();
	void					OnBtnClickedPDADel();

	void					OnBtnClickedEmailAdd();
	void					OnEmailAdd();
	void					OnBtnClickedEmailEdit();
	void					OnEmailEdit();
	void					OnBtnClickedEmailDel();

	void					OnBtnClickedAudioAdd();
	void					OnAudioAdd();
	void					OnBtnClickedAudioEdit();
	void					OnAudioEdit();
	void					OnBtnClickedAudioDel();

	void					OnBtnClickedVideoAdd();
	void					OnVideoAdd();
	void					OnBtnClickedVideoEdit();
	void					OnVideoEdit();
	void					OnBtnClickedVideoDel();

private:
	bool					isShown;

	idStr					windowTitle;
	bool					saveButtonEnabled;

	int						pdaListSel;
	idList<idStr>			pdaList;
	int						emailListSel;
	idList<idStr>			emailList;
	int						audioListSel;
	idList<idStr>			audioList;
	int						videoListSel;
	idList<idStr>			videoList;

	idStr					fullName;
	idStr					shortName;
	idStr					post;
	idStr					title;
	idStr					security;
	idStr					idnum;

	DialogPDAAdd			addPDADlg;
	DialogPDAEditEmail		editEmailDlg;
	DialogPDAEditAudio		editAudioDlg;
	DialogPDAEditVideo		editVideoDlg;

private:
	void PopulatePDAList();
};

}

#endif /* !__PDAEDITOR_H__ */
