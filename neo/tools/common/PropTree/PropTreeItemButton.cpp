// PropTreeItemButton.cpp : implementation file
//
//  Copyright (C) 1998-2001 Scott Ramsay
//	sramsay@gonavi.com
//	http://www.gonavi.com
//
//  This material is provided "as is", with absolutely no warranty expressed
//  or implied. Any use is at your own risk.
//
//  Permission to use or copy this software for any purpose is hereby granted
//  without fee, provided the above notices are retained on all copies.
//  Permission to modify the code and to distribute modified code is granted,
//  provided the above notices are retained, and a notice that the code was
//  modified is included with the above copyright notice.
//
//	If you use this code, drop me an email.  I'd like to know if you find the code
//	useful.

//#include "stdafx.h"
#include "tools/edit_gui_common.h"


#include "proptree.h"
#include "PropTreeItemButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BUTTON_SIZE 17

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemButton

CPropTreeItemButton::CPropTreeItemButton() {
	mouseDown = false;
}

CPropTreeItemButton::~CPropTreeItemButton() {
}


/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemButton message handlers

LONG CPropTreeItemButton::DrawItem( CDC* pDC, const RECT& rc, LONG x, LONG y )
{
	CSize	textSize;
	CRect	textRect;
	LONG	nTotal = 0;

	nTotal = CPropTreeItem::DrawItem( pDC, rc, x, y );

	textSize = pDC->GetOutputTextExtent( buttonText );
	UINT dpi = GetDpiForWindow(m_pProp->GetSafeHwnd());
	float scaling_factor = static_cast<float>(dpi) / 96.0f;
	int s2 = int(2 * scaling_factor);
	int s4 = int(4 * scaling_factor);
	int s12 = int(12 * scaling_factor);

	buttonRect.left = m_rc.right - ( textSize.cx + s12 + s4);
	buttonRect.top = m_rc.top + ((m_rc.bottom - m_rc.top)/2)- (textSize.cy + s4) /2;
	buttonRect.right = buttonRect.left + textSize.cx + s12;
	buttonRect.bottom = buttonRect.top + (textSize.cy + s4);

	UINT buttonStyle;

	if ( (m_dwState & TreeItemChecked) ) {
		buttonStyle = DFCS_BUTTONPUSH | DFCS_PUSHED;
	} else {
		buttonStyle = DFCS_BUTTONPUSH;
	}
	pDC->DrawFrameControl(&buttonRect, DFC_BUTTON, buttonStyle );

	textRect = buttonRect;
	textRect.left += s4;
	textRect.right -= s4-s4;
	pDC->DrawText( buttonText, textRect, DT_SINGLELINE|DT_VCENTER );

	//Adjust hit test rect to acount for window scrolling
	hitTestRect = buttonRect;
	hitTestRect.OffsetRect(0, m_pProp->GetOrigin().y);

	return nTotal;
}

void CPropTreeItemButton::DrawAttribute(CDC* pDC, const RECT& rc) {
}


LPARAM CPropTreeItemButton::GetItemValue() {
	return (LPARAM)0;
}


void CPropTreeItemButton::SetItemValue(LPARAM lParam) {
}


BOOL CPropTreeItemButton::HitButton( const POINT& pt ) {
	return hitTestRect.PtInRect( pt );
}

void CPropTreeItemButton::SetButtonText( LPCSTR text ) {
	buttonText = text;
}
