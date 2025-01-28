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

#include <wx/app.h>
#include <wx/frame.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/textdlg.h>
#include <wx/button.h>
#include <wx/xrc/xmlres.h>

#include "tools/edit_gui_common.h"
#include "../../sys/win32/win_local.h"

#include "DialogPDAEditor.h"

extern void InitXmlResource(); // defined in generated file (TODO: precompile ALL the xrc files)

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

/////////////////////////////////////////////////////////////////////////////
// CCDialogPDAEditor dialog
CDialogPDAEditor *g_PDAEditorDialog = NULL;


CDialogPDAEditor::CDialogPDAEditor(wxWindow* parent /*=NULL*/)
{
	SetParent(parent);
}

void CDialogPDAEditor::InitControlsFromDialog() {
	wxXmlResource::Get()->LoadDialog(this, GetParent(), "PDAEditor");

	pdaList = XRCCTRL(*this, "listPDAs", wxListBox);
	emailList = XRCCTRL(*this, "listEmail", wxListBox);
	audioList = XRCCTRL(*this, "listAudio", wxListBox);
	videoList = XRCCTRL(*this, "listVideo", wxListBox);

	editFullname = XRCCTRL(*this, "editFullname", wxTextCtrl);
	editShortname = XRCCTRL(*this, "editShortname", wxTextCtrl);
	editPost = XRCCTRL(*this, "editPost", wxTextCtrl);
	editTitle = XRCCTRL(*this, "editTitle", wxTextCtrl);
	editSecurity = XRCCTRL(*this, "editSecurity", wxTextCtrl);
	editIdnum = XRCCTRL(*this, "editIdnum", wxTextCtrl);

	saveButton = XRCCTRL(*this, "save", wxButton);
}

wxBEGIN_EVENT_TABLE( CDialogPDAEditor, wxDialog )
	EVT_INIT_DIALOG( OnInitDialogFunction )
	EVT_BUTTON( XRCID( "close" ), OnCloseDialogFunction )
	EVT_LISTBOX( XRCID( "listPDAs" ), OnSelChangePDA )
	EVT_BUTTON( XRCID( "save" ), OnBtnClickedSave)
	EVT_BUTTON( XRCID( "randomid" ), OnBtnClickedRandom )
	EVT_BUTTON( XRCID( "pdaAdd" ), OnBtnClickedPDAAdd )
	EVT_BUTTON( XRCID( "pdaDel" ), OnBtnClickedPDADel )
	EVT_BUTTON( XRCID( "emailAdd" ), OnBtnClickedEmailAdd )
	EVT_BUTTON( XRCID( "emailEdit" ), OnBtnClickedEmailEdit )
	EVT_BUTTON( XRCID( "emailDelete" ), OnBtnClickedEmailDel )
	EVT_BUTTON( XRCID( "audioAdd" ), OnBtnClickedAudioAdd )
	EVT_BUTTON( XRCID( "audioEdit" ), OnBtnClickedAudioEdit )
	EVT_BUTTON( XRCID( "audioDelete" ), OnBtnClickedAudioDel )
	EVT_BUTTON( XRCID( "videoAdd" ), OnBtnClickedVideoAdd )
	EVT_BUTTON( XRCID( "videoEdit" ), OnBtnClickedVideoEdit )
	EVT_BUTTON( XRCID( "videoDelete" ), OnBtnClickedVideoDel )
wxEND_EVENT_TABLE( )


/////////////////////////////////////////////////////////////////////////////
// CDialogPDAEditor message handlers

class MainWindow : public wxFrame {
	enum { ID_OPENIMAGE };

public:
	MainWindow(const wxString& title);
	~MainWindow();
	bool OnInit();
	void AddMenu();
	void OpenImageFile(wxCommandEvent& e);
};

MainWindow::MainWindow(const wxString& title) : wxFrame(NULL, wxID_ANY, title) {

}

MainWindow::~MainWindow() {}

class Main : public wxApp {
	bool OnInit();
};

bool Main::OnInit() {
	return true;
}

wxIMPLEMENT_APP_NO_MAIN(Main);

Main *g_mainApp;

void PDAEditorInit( const idDict *spawnArgs ) {

	if ( renderSystem->IsFullScreen() ) {
		common->Printf( "Cannot run the PDA editor in fullscreen mode.\n"
					"Set r_fullscreen to 0 and vid_restart.\n" );
		return;
	}

	if ( wxTheApp == NULL ) {
		wxApp::SetInstance(new Main());
		wxEntryStart(win32.hInstance, NULL, NULL, SW_SHOW);
		wxTheApp->CallOnInit();

		wxXmlResource::Get()->InitAllHandlers();
		InitXmlResource();
	}

	if ( g_PDAEditorDialog == NULL ) {

		g_PDAEditorDialog = new CDialogPDAEditor();
		g_PDAEditorDialog->InitControlsFromDialog();
	}

	idKeyInput::ClearStates();

	g_PDAEditorDialog->Show();
	g_PDAEditorDialog->SetFocus();

#if 0
	if ( spawnArgs ) {
		// select PDA based on spawn args
		const char *name = spawnArgs->GetString( "pda" );
		idDeclPDA *decl = static_cast<idDeclPDA *>( const_cast<idDecl *>( declManager->FindType( DECL_PDA, name ) ) );
		// FIXME: select this PDA
	}
#endif
}

void PDAEditorRun( void ) {
	while ( wxTheApp->Pending() ) {
		wxTheApp->Dispatch();
	}
}

void PDAEditorShutdown( void ) {
	delete g_PDAEditorDialog;
	g_PDAEditorDialog = NULL;
}

void CDialogPDAEditor::OnActivate( wxActivateEvent &event ) {
	wxDialog::OnActivate( event );
}

void CDialogPDAEditor::OnMove( wxMouseEvent &event ) {
	/*
	if ( GetSafeHwnd() ) {
		CRect rct;
		GetWindowRect( rct );
		// FIXME: save position
	}
	CDialog::OnMove( x, y );
	*/
}

void CDialogPDAEditor::OnCloseDialogFunction( wxCommandEvent &event ) {

	Hide();

	com_editors &= ~EDITOR_PDA;
}

void CDialogPDAEditor::OnInitDialogFunction( wxInitDialogEvent &event )
{
	wxDialog::OnInitDialog( event );

	// Indicate the PDA dialog is opened
	com_editors |= EDITOR_PDA;

	PopulatePDAList();
}

void CDialogPDAEditor::PopulatePDAList()
{
	pdaList->Clear();

	int i;
	int num = declManager->GetNumDecls(DECL_PDA);
	for ( i=0; i < num; i++ ) {
		const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, i) );
		wxString name = wxString( pda->GetName() );
		pdaList->Append( name );
	}
}

void CDialogPDAEditor::OnSelChangePDA( wxCommandEvent &event )
{
	int i, num;

	int index = pdaList->GetSelection();
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, index) );
	if ( !pda ) {
		return;
	}

	wxString windowTitle;
	windowTitle.Format( "PDA Editor - %s", pda->GetName() );

	idFile *file = fileSystem->OpenFileAppend( pda->GetFileName() );
	if ( file ) {
		fileSystem->CloseFile(file);
		saveButton->Enable( true );
	} else {
		windowTitle += " [Read Only]";
		saveButton->Enable( false );
	}

	SetTitle( windowTitle );

	emailList->Clear();
	num = pda->GetNumEmails();
	for ( i=0; i < num; i++ ) {
		emailList->Append( pda->GetEmailByIndex( i )->GetSubject() );
	}

	audioList->Clear();
	num = pda->GetNumAudios();
	for ( i=0; i < num; i++ ) {
		audioList->Append( pda->GetAudioByIndex( i )->GetAudioName() );
	}

	videoList->Clear();
	num = pda->GetNumVideos();
	for ( i=0; i < num; i++ ) {
		videoList->Append( pda->GetVideoByIndex( i )->GetVideoName() );
	}

	editFullname->SetValue( pda->GetFullName() );
	editShortname->SetValue( pda->GetPdaName() );
	editPost->SetValue( pda->GetPost() );
	editTitle->SetValue( pda->GetTitle() );
	editSecurity->SetValue( pda->GetSecurity() );
	editIdnum->SetValue( pda->GetID() );
}

void CDialogPDAEditor::OnBtnClickedSave( wxCommandEvent& event )
{
	int index = pdaList->GetSelection();
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pdaConst = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, index) );
	if ( pdaConst ) {
		idDeclPDA *pda = const_cast<idDeclPDA *>(pdaConst);

		wxString declText = "\n";
		declText += "pda ";
		declText += pda->GetName();
		declText += " {\n";

		declText += "\tname    \t\t\"" + editShortname->GetValue() + "\"\n";
		declText += "\tfullname\t\t\"" + editFullname->GetValue() + "\"\n";
		declText += "\ticon    \t\t\"\"\n";
		declText += "\tid      \t\t\"" + editIdnum->GetValue() + "\"\n";
		declText += "\tpost    \t\t\"" + editPost->GetValue() + "\"\n";
		declText += "\ttitle   \t\t\"" + editTitle->GetValue() + "\"\n";
		declText += "\tsecurity\t\t\"" + editSecurity->GetValue() + "\"\n";

		for ( int i = 0; i < pda->GetNumEmails(); i++ ) {
			declText += "\tpda_email\t\t\"";
			declText += pda->GetEmailByIndex(i)->GetName();
			declText += "\"\n";
		}

		for ( int i = 0; i < pda->GetNumAudios(); i++ ) {
			declText += "\tpda_audio\t\t\"";
			declText += pda->GetAudioByIndex(i)->GetName();
			declText += "\"\n";
		}

		for ( int i = 0; i < pda->GetNumVideos(); i++ ) {
			declText += "\tpda_video\t\t\"";
			declText += pda->GetVideoByIndex(i)->GetName();
			declText += "\"\n";
		}

		declText += "}";

		pda->SetText( declText );
		pda->ReplaceSourceFileText();
		pda->Invalidate();
	}
}

void CDialogPDAEditor::OnBtnClickedRandom( wxCommandEvent &event )
{
	editIdnum->SetValue( wxString::Format("%d-%02X", 1000+(rand()%8999), (rand()%255)) );
}

class CDialogPDAAdd : public wxDialog
{
public:
	CDialogPDAAdd(wxWindow *parent)
	{
		wxXmlResource::Get()->LoadDialog(this, parent, "PDAAdd");

		editName = XRCCTRL(*this, "editName", wxTextCtrl);
	}

	wxString name;

	void OnOK( wxCommandEvent &event )
	{
		name = editName->GetValue();
	}

private:
	wxTextCtrl* editName;

	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(CDialogPDAAdd, wxDialog)
EVT_BUTTON(wxID_OK, OnOK)
wxEND_EVENT_TABLE()

void CDialogPDAEditor::OnBtnClickedPDAAdd( wxCommandEvent &event )
{
	CDialogPDAAdd dlg(NULL);

	if ( dlg.ShowModal() == wxID_OK ) {
		wxString name = dlg.name;
		name.MakeLower();
		idDecl *decl = declManager->CreateNewDecl( DECL_PDA, name, "newpdas/" + name + ".pda" );
		decl->ReplaceSourceFileText();
		decl->Invalidate();
		PopulatePDAList();
		pdaList->SetStringSelection( name );
		OnSelChangePDA( event );
	}
}

void CDialogPDAEditor::OnBtnClickedPDADel( wxCommandEvent &event )
{
}

void CDialogPDAEditor::OnBtnClickedEmailAdd( wxCommandEvent &event )
{
	int index = pdaList->GetSelection();
	if ( index < 0 ) {
		return;
	}
	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, index) );

	if ( pda ) {
		wxString name;

		// Search for an unused name
		int newIndex = pda->GetNumEmails();
		do {
			name.Format("%s_email_%d", pda->GetName(), newIndex++);
		} while ( declManager->FindType(DECL_EMAIL, name, false) != NULL );

		CDialogPDAEditEmail addDlg;
		addDlg.SetName(name);
		if ( addDlg.ShowModal() == wxID_OK ) {
			idDeclEmail *email = static_cast<idDeclEmail *>(declManager->CreateNewDecl(DECL_EMAIL, name, pda->GetFileName()));
			email->SetText( addDlg.GetDeclText() );
			email->ReplaceSourceFileText();
			email->Invalidate();

			pda->AddEmail( name );

			// Get it again to reparse
			const idDeclEmail *emailConst = static_cast<const idDeclEmail *>( declManager->FindType( DECL_EMAIL, name) );
			emailList->Append( emailConst->GetSubject() );

			// Save the pda to include this email in the list
			// This has a side-effect of saving any other changes, but I don't really care right now
			OnBtnClickedSave( event );
		}
	}
}

void CDialogPDAEditor::OnBtnClickedEmailEdit( wxCommandEvent &event )
{
	int index = pdaList->GetSelection();
	if ( index < 0 ) {
		return;
	}
	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, index) );

	if ( pda ) {
		index = emailList->GetSelection();
		if ( index < 0 ) {
			return;
		}

		CDialogPDAEditEmail editDlg;
		editDlg.SetEmail( pda->GetEmailByIndex( index ) );
		if ( editDlg.ShowModal() == wxID_OK ) {
			idDeclEmail *email = const_cast<idDeclEmail *>( pda->GetEmailByIndex( index ) );
			email->SetText( editDlg.GetDeclText() );
			email->ReplaceSourceFileText();
			email->Invalidate();

			// Get it again to reparse
			email = const_cast<idDeclEmail *>( pda->GetEmailByIndex( index ) );

			emailList->Delete( index );
			emailList->Insert( email->GetSubject(), index );
		}
	}
}

void CDialogPDAEditor::OnBtnClickedEmailDel( wxCommandEvent &event )
{
}

void CDialogPDAEditor::OnBtnClickedAudioAdd( wxCommandEvent &event )
{
}

void CDialogPDAEditor::OnBtnClickedAudioEdit( wxCommandEvent &event )
{
}

void CDialogPDAEditor::OnBtnClickedAudioDel( wxCommandEvent &event )
{
}

void CDialogPDAEditor::OnBtnClickedVideoAdd( wxCommandEvent &event )
{
}

void CDialogPDAEditor::OnBtnClickedVideoEdit( wxCommandEvent &event )
{
}

void CDialogPDAEditor::OnBtnClickedVideoDel( wxCommandEvent &event )
{
}




CDialogPDAEditEmail::CDialogPDAEditEmail(wxWindow* parent /*=NULL*/)
{
	wxXmlResource::Get()->LoadDialog(this, parent, "PDAEditEmail");

	editTo = XRCCTRL(*this, "editTo", wxTextCtrl);
	editFrom = XRCCTRL(*this, "editFrom", wxTextCtrl);
	editDate = XRCCTRL(*this, "editDate", wxTextCtrl);
	editSubject = XRCCTRL(*this, "editSubject", wxTextCtrl);
	editBody = XRCCTRL(*this, "editBody", wxTextCtrl);
}

wxBEGIN_EVENT_TABLE(CDialogPDAEditEmail, wxDialog)
	EVT_INIT_DIALOG(CDialogPDAEditEmail::OnInitDialogFunction)
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////
// CDialogPDAEditor message handlers

void CDialogPDAEditEmail::OnInitDialogFunction( wxInitDialogEvent& event )
{
	wxDialog::OnInitDialog( event );

	SetTitle( "Editing Email: " + name );
}

void CDialogPDAEditEmail::SetName( wxString &_name )
{
	name = _name;
}

void CDialogPDAEditEmail::SetEmail( const idDeclEmail *email )
{
	editTo->SetValue( email->GetTo() );
	editFrom->SetValue( email->GetFrom() );
	editDate->SetValue( email->GetDate() );
	editSubject->SetValue( email->GetSubject() );

	wxString body = email->GetBody();
	body.Replace("\n", "\r\n");

	editBody->SetValue( body );

	name = email->GetName();
}

wxString CDialogPDAEditEmail::GetDeclText()
{
	wxString mungedBody = editBody->GetValue().c_str();
	mungedBody.Replace("\r\n\r\n", "\\n\\n\"\n\n\"");
	mungedBody.Replace("\r\n", "\\n\"\n\"");

	wxString declText;
	declText += "\n";
	declText += "email " + name + " {\n";
	declText += "\tto     \t\t\"" + editTo->GetValue() + "\"\n";
	declText += "\tfrom   \t\t\"" + editFrom->GetValue() + "\"\n";
	declText += "\tdate   \t\t\"" + editDate->GetValue() + "\"\n";
	declText += "\tsubject\t\t\"" + editSubject->GetValue() + "\"\n";
	declText += "\ttext {\n";
	declText += "\"" + mungedBody + "\"\n";
	declText += "\t}\n";
	declText += "}";

	return declText;
}
