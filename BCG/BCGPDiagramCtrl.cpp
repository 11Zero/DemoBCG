//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPDiagramCtrl.cpp: implementation of the CBCGPDiagramVisualContainerCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPDiagramCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CBCGPDiagramVisualContainerCtrl, CBCGPVisualContainerCtrl)

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramVisualContainerCtrl

CBCGPDiagramVisualContainerCtrl::CBCGPDiagramVisualContainerCtrl()
{
}

CBCGPDiagramVisualContainerCtrl::~CBCGPDiagramVisualContainerCtrl()
{
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainerCtrl::PreTranslateMessage(MSG* pMsg)
{
	CBCGPDiagramVisualContainer* pContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, GetVisualContainer ());
	if (pContainer != NULL && pContainer->IsInplaceEdit ())
	{
		if (pMsg->message == WM_KEYDOWN)
		{
			if (pContainer->OnInplaceEditKeyDown (pMsg))
			{
				return TRUE;
			}
		}

		else if (pMsg->message >= WM_MOUSEFIRST &&
				 pMsg->message <= WM_MOUSELAST)
		{
			CPoint ptCursor;
			::GetCursorPos (&ptCursor);
			ScreenToClient (&ptCursor);

			CRect rectEdit;
			CWnd* pEditWnd = pContainer->GetInplaceEditWnd ();
			
			if (pEditWnd->GetSafeHwnd () != NULL)
			{
				pEditWnd->GetWindowRect (&rectEdit);
				ScreenToClient (&rectEdit);
			}

			BOOL bInPlaceEditMessage = (pMsg->hwnd == pEditWnd->GetSafeHwnd ());

			if (!rectEdit.PtInRect (ptCursor) &&
				(pMsg->message == WM_LBUTTONDOWN ||
				 pMsg->message == WM_RBUTTONDOWN ||
				 pMsg->message == WM_MBUTTONDOWN))
			{
				if (!pContainer->EndEditItem ())
				{
					return TRUE;
				}

				if (bInPlaceEditMessage)
				{
					return TRUE;
				}
			}
		}
	}

	return CBCGPVisualContainerCtrl::PreTranslateMessage(pMsg);
}