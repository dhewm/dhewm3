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

#ifndef __DIALOGPDAEDITOR_H__
#define __DIALOGPDAEDITOR_H__

/////////////////////////////////////////////////////////////////////////////
// CCDialogPDAEditor dialog

class CDialogPDAEditor : public wxDialog {
public:
							CDialogPDAEditor(wxWindow* pParent = NULL);   // standard constructor

	void			InitControlsFromDialog();

private:

	void			OnInitDialogFunction( wxInitDialogEvent& event );

	void			OnActivate( wxActivateEvent &event );
	void			OnMove( wxMouseEvent &event );
	void			OnCloseDialogFunction( wxCommandEvent &event );

	void			OnSelChangePDA( wxCommandEvent &event );

	void			OnBtnClickedSave(wxCommandEvent &event );
	void			OnBtnClickedRandom(wxCommandEvent &event );

	void			OnBtnClickedPDAAdd( wxCommandEvent &event );
	void			OnBtnClickedPDADel( wxCommandEvent &event );

	void			OnBtnClickedEmailAdd( wxCommandEvent &event );
	void			OnBtnClickedEmailEdit( wxCommandEvent &event );
	void			OnBtnClickedEmailDel( wxCommandEvent &event );

	void			OnBtnClickedAudioAdd( wxCommandEvent &event );
	void			OnBtnClickedAudioEdit( wxCommandEvent &event );
	void			OnBtnClickedAudioDel( wxCommandEvent &event );

	void			OnBtnClickedVideoAdd( wxCommandEvent &event );
	void			OnBtnClickedVideoEdit( wxCommandEvent &event );
	void			OnBtnClickedVideoDel( wxCommandEvent &event );

	wxDECLARE_EVENT_TABLE();

private:
	wxListBox *				pdaList;
	wxListBox *				emailList;
	wxListBox *				audioList;
	wxListBox *				videoList;

	wxTextCtrl *			editFullname;
	wxTextCtrl *			editShortname;
	wxTextCtrl *			editPost;
	wxTextCtrl *			editTitle;
	wxTextCtrl *			editSecurity;
	wxTextCtrl *			editIdnum;

	wxButton *				saveButton;

private:

	void PopulatePDAList();
};

class CDialogPDAEditEmail : public wxDialog {
public:
							CDialogPDAEditEmail( wxWindow* pParent = NULL );   // standard constructor

	void SetName( wxString &name );
	void SetEmail( const idDeclEmail *email );

	wxString GetDeclText();

private:
	wxDECLARE_EVENT_TABLE();

	void					OnInitDialogFunction( wxInitDialogEvent& event );

	wxTextCtrl *			editTo;
	wxTextCtrl *			editFrom;
	wxTextCtrl *			editDate;
	wxTextCtrl *			editSubject;
	wxTextCtrl *			editBody;

	wxString				to;
	wxString				from;
	wxString				date;
	wxString				subject;
	wxString				body;

	wxString				name;
};

#endif /* !__DIALOGPDAEDITOR_H__ */
