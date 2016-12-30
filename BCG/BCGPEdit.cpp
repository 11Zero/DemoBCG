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
// BCGPEdit.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "bcgglobals.h"

#ifndef _BCGSUITE_
#include "BCGPToolBarImages.h"
#include "BCGPShellManager.h"
#include "BCGPTooltipManager.h"
#include "BCGPDropDownList.h"
#endif

#include "BCGPVisualManager.h"
#include "BCGProRes.h"
#include "BCGPLocalResource.h"
#include "BCGPCalculator.h"
#include "BCGPEdit.h"
#include "BCGPDlgImpl.h"
#include "BCGPDrawManager.h"
#include "BCGPGlobalUtils.h"

#ifndef _BCGSUITE_
	#define visualManagerMFC	CBCGPVisualManager::GetInstance ()
#else
	#define visualManagerMFC	CMFCVisualManager::GetInstance ()
	#define CBCGPDropDownList	CMFCDropDownListBox
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef _BCGSUITE_
UINT BCGM_EDIT_ON_FILL_AUTOCOMPLETE_LIST = ::RegisterWindowMessage (_T("BCGM_EDIT_ON_FILL_AUTOCOMPLETE_LIST"));

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditDropDownList

const UINT idStart = (UINT) -200;

class CBCGPEditDropDownList : public CBCGPDropDownList
{
	friend class CBCGPEdit;

public:
	CBCGPEditDropDownList(CBCGPEdit* pEdit) :
		CBCGPDropDownList(pEdit)
	{
		m_pEdit = pEdit;
	}
	  
	virtual void OnChooseItem (UINT uidCmdID)
	{
		CBCGPDropDownList::OnChooseItem (uidCmdID);

		if (m_pEdit->GetSafeHwnd() != NULL)
		{
			ASSERT_VALID (m_pEdit);
  
			int nIndex = (int)idStart - uidCmdID;

			CString strItemText;
			GetText(nIndex, strItemText);

			m_pEdit->m_bInAutoComplete = TRUE;
			m_pEdit->SetWindowText(strItemText);
			m_pEdit->SendMessage(WM_KEYDOWN, VK_END);
			m_pEdit->m_bInAutoComplete = FALSE;
		}
	}

	BOOL Compare(const CStringList& lstStrings) const
	{
		if (GetCount() != lstStrings.GetCount())
		{
			return FALSE;
		}

		int i = 0;

		for (POSITION pos = lstStrings.GetHeadPosition (); pos != NULL; i++)
		{
			CString strItemText;
			GetText(i, strItemText);

			if (lstStrings.GetNext(pos) != strItemText)
			{
				return FALSE;
			}
		}

		return TRUE;
	}
	  
protected:
	CBCGPEdit* m_pEdit;
};

#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPEdit

IMPLEMENT_DYNAMIC(CBCGPEdit, CEdit)

CBCGPEdit::CBCGPEdit()
{
	m_rectBtn.SetRectEmpty ();
	m_bIsButtonPressed = FALSE;
	m_bIsButtonHighlighted = FALSE;
	m_bIsButtonCaptured = FALSE;
	m_Mode = BrowseMode_None;
	m_sizeImage = CSize (0, 0);
	m_pCalcPopup = NULL;
	m_pToolTip = NULL;
	m_bShowToolTip = FALSE;
	m_bDefaultPrintClient = FALSE;
	
	m_nBrowseButtonWidth = 20;
	if (globalData.GetRibbonImageScale() != 1.0)
	{
		m_nBrowseButtonWidth = (int) (globalData.GetRibbonImageScale () * m_nBrowseButtonWidth);
	}

	m_bDefaultImage = TRUE;
	m_bVisualManagerStyle = FALSE;
	m_bOnGlass = FALSE;
	m_bSearchMode = FALSE;
	m_bHasPrompt = FALSE;
	m_bTextIsEmpty = TRUE;
	m_clrPrompt = (COLORREF)-1;
	m_clrErrorText = (COLORREF)-1;

#ifndef _BCGSUITE_
	m_pDropDownPopup = NULL;
	m_bInAutoComplete = FALSE;
#endif
}

CBCGPEdit::~CBCGPEdit()
{
}

BEGIN_MESSAGE_MAP(CBCGPEdit, CEdit)
	//{{AFX_MSG_MAP(CBCGPEdit)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCMOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_CONTROL_REFLECT_EX(EN_CHANGE, OnChange)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_DESTROY()
	ON_WM_KILLFOCUS()
	ON_WM_NCHITTEST()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLAERO, OnBCGSetControlAero)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedTipText)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_MESSAGE(WM_PRINT, OnPrint)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPEdit message handlers

void CBCGPEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bIsButtonCaptured)
	{
		ReleaseCapture ();

		m_bIsButtonPressed = FALSE;
		m_bIsButtonCaptured = FALSE;
		m_bIsButtonHighlighted = FALSE;

		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);

		if (m_rectBtn.PtInRect (point))
		{
			OnBrowse ();
		}

		return;
	}
	
	CEdit::OnLButtonUp(nFlags, point);
}
//*************************************************************************************
void CBCGPEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bIsButtonCaptured)
	{
		BOOL bIsButtonPressed = m_rectBtn.PtInRect (point);
		if (bIsButtonPressed != m_bIsButtonPressed)
		{
			m_bIsButtonPressed = bIsButtonPressed;
			RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}

		return;
	}
	
	if (m_bIsButtonHighlighted)
	{
		if (!m_rectBtn.PtInRect (point))
		{
			m_bIsButtonHighlighted = FALSE;
			ReleaseCapture ();

			RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}
	
	CEdit::OnMouseMove(nFlags, point);
}
//*************************************************************************************
void CBCGPEdit::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CEdit::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (m_Mode != BrowseMode_None)
	{
		lpncsp->rgrc [0].right -= m_nBrowseButtonWidth;
	}
}
//*************************************************************************************
void CBCGPEdit::OnNcPaint() 
{
	const BOOL bHasScrollBars = ((GetStyle () & WS_VSCROLL) == WS_VSCROLL) || ((GetStyle () & WS_HSCROLL) == WS_HSCROLL);
	const BOOL bHasBorder = (GetExStyle () & WS_EX_CLIENTEDGE) || (GetStyle () & WS_BORDER);

	if (bHasScrollBars || (!m_bVisualManagerStyle && !m_bOnGlass))
	{
		CEdit::OnNcPaint ();
	}

	if (bHasBorder && (m_bVisualManagerStyle || m_bOnGlass))
	{
		CBCGPDrawOnGlass dog (m_bOnGlass);
		visualManagerMFC->OnDrawControlBorder (this);
	}

	if (m_Mode == BrowseMode_None)
	{
		return;
	}

	CWindowDC dc(this);
	DoNcPaint(&dc, FALSE);
}
//********************************************************************************
void CBCGPEdit::DoNcPaint(CDC* pDC, BOOL bIsPrint)
{
	CRect rectWindow;
	GetWindowRect (rectWindow);

	m_rectBtn = rectWindow;
	m_rectBtn.left = m_rectBtn.right -  m_nBrowseButtonWidth;

	CRect rectClient;
	GetClientRect (rectClient);
	ClientToScreen (&rectClient);

	m_rectBtn.OffsetRect (rectClient.right + m_nBrowseButtonWidth - rectWindow.right, 0);
	m_rectBtn.top += rectClient.top - rectWindow.top;
	m_rectBtn.bottom -= rectWindow.bottom - rectClient.bottom;

	CRect rect = m_rectBtn;
	rect.OffsetRect (-rectWindow.left, -rectWindow.top);

	CRgn rgnClip;

	if (!bIsPrint)
	{
		rgnClip.CreateRectRgnIndirect (&rect);
		pDC->SelectClipRgn (&rgnClip);
	}

	OnDrawBrowseButton(pDC, rect, m_bIsButtonPressed, m_bIsButtonHighlighted);

	if (!bIsPrint)
	{
		pDC->SelectClipRgn (NULL);
	}

	ScreenToClient (&m_rectBtn);
}
//********************************************************************************
BCGNcHitTestType CBCGPEdit::OnNcHitTest(CPoint point) 
{
	CPoint ptClient = point;
	ScreenToClient (&ptClient);

	if (m_Mode != BrowseMode_None && m_rectBtn.PtInRect (ptClient))
	{
		return HTBORDER;
	}
	
	return CEdit::OnNcHitTest(point);
}
//********************************************************************************
void CBCGPEdit::OnDrawBrowseButton (CDC* pDC, CRect rect, BOOL bIsButtonPressed, BOOL bHighlight)
{
	ASSERT (m_Mode != BrowseMode_None);
	ASSERT_VALID (pDC);

	if (m_bSearchMode && m_ImageSearch.IsValid() && m_bTextIsEmpty)
	{
		if (m_bOnGlass)
		{
			CBCGPDrawManager dm(*pDC);
			dm.DrawRect(rect, IsWindowEnabled() ? globalData.clrWindow : globalData.clrBtnFace, (COLORREF)-1);
		}
		else if (m_bVisualManagerStyle)
		{
			pDC->FillRect(rect, &CBCGPVisualManager::GetInstance ()->GetEditCtrlBackgroundBrush(this));
		}
		else
		{
			pDC->FillRect(rect, IsWindowEnabled() ? &globalData.brWindow : &globalData.brBtnFace);
		}

		if (IsWindowEnabled())
		{
			m_ImageSearch.DrawEx(pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
		}
		else
		{
			CBCGPDrawState ds;

			m_ImageSearch.PrepareDrawImage (ds);

			CSize sizeImage = m_ImageSearch.GetImageSize();

			m_ImageSearch.Draw (pDC, 
				rect.left + max(0, (rect.Width() - sizeImage.cx) / 2), 
				rect.top + max(0, (rect.Height() - sizeImage.cy) / 2), 
				0, FALSE, TRUE);
			m_ImageSearch.EndDrawImage (ds);
		}

		return;
	}

	CBCGPVisualManager::BCGBUTTON_STATE state = CBCGPVisualManager::ButtonsIsRegular;

	if (bIsButtonPressed)
	{
		state = CBCGPVisualManager::ButtonsIsPressed;
	}
	else if (bHighlight)
	{
		state = CBCGPVisualManager::ButtonsIsHighlighted;
	}

	COLORREF clrText = m_bVisualManagerStyle ? globalData.clrBarText : globalData.clrBtnText;
	if (!IsWindowEnabled() && !globalData.IsHighContastMode())
	{
		clrText = globalData.clrGrayedText;
	}

	if (!CBCGPVisualManager::GetInstance ()->OnDrawBrowseButton (pDC, rect, this, state, clrText))
	{
		return;
	}

	if (m_bSearchMode && m_ImageClear.IsValid() && !m_bTextIsEmpty)
	{
		CRect rectImage = rect;

		if (bIsButtonPressed && visualManagerMFC->IsOffsetPressedButton ())
		{
			rectImage.left++;
			rectImage.top++;
		}

		m_ImageClear.DrawEx(pDC, rectImage, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
		return;
		
	}

	int iImage = 0;

	if (m_ImageBrowse.IsValid())
	{
		if (m_bDefaultImage)
		{
			switch (m_Mode)
			{
			case BrowseMode_Folder:
				iImage = 0;
				break;

			case BrowseMode_File:
				iImage = 1;
				break;

			case BrowseMode_Calculator:
				iImage = 2;
				break;
			}
		}

		CRect rectImage = rect;

		if (bIsButtonPressed && visualManagerMFC->IsOffsetPressedButton ())
		{
			rectImage.left++;
			rectImage.top++;
		}

#ifndef _BCGSUITE_
		if (m_bOnGlass)
		{
			m_ImageBrowse.ConvertTo32Bits();
		}
#endif
		if (IsWindowEnabled())
		{
			m_ImageBrowse.DrawEx (pDC, rectImage, iImage, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
		}
		else
		{
			CBCGPDrawState ds;

			m_ImageBrowse.PrepareDrawImage (ds);

			CSize sizeImage = m_ImageBrowse.GetImageSize();

			m_ImageBrowse.Draw (pDC, 
				rectImage.left + max(0, (rectImage.Width() - sizeImage.cx) / 2), 
				rectImage.top + max(0, (rectImage.Height() - sizeImage.cy) / 2), 
				iImage, FALSE, TRUE);
			m_ImageBrowse.EndDrawImage (ds);
		}
	}
	else if (!m_strLabel.IsEmpty ())
	{
		CFont* pFont = (CFont*) pDC->SelectStockObject (DEFAULT_GUI_FONT);

		CRect rectText = rect;
		rectText.DeflateRect (1, 2);
		rectText.OffsetRect (0, -2);

		if (bIsButtonPressed)
		{
			rectText.OffsetRect (1, 1);
		}

		DWORD dwDTFlags = DT_SINGLELINE | DT_CENTER | DT_VCENTER;

		if (m_bOnGlass)
		{
			CBCGPVisualManager::GetInstance ()->DrawTextOnGlass(pDC, m_strLabel, rectText, dwDTFlags, 0, clrText);
		}
		else
		{
			COLORREF clrTextOld = pDC->SetTextColor (clrText);
			int nTextMode = pDC->SetBkMode (TRANSPARENT);

			pDC->DrawText (m_strLabel, rectText, dwDTFlags);

			pDC->SetTextColor (clrTextOld);
			pDC->SetBkMode (nTextMode);
		}

		pDC->SelectObject (pFont);
	}
}
//********************************************************************************
void CBCGPEdit::EnableBrowseButton (BOOL bEnable/* = TRUE*/,
									LPCTSTR szLabel/* = _T("...")*/)
{
	ASSERT_VALID (this);
	ASSERT (szLabel != NULL);

	m_Mode = bEnable ? BrowseMode_Default : BrowseMode_None;
	m_strLabel = szLabel;

	m_ImageBrowse.Clear ();
	m_sizeImage = CSize (0, 0);

	OnChangeLayout ();
}
//********************************************************************************
void CBCGPEdit::OnChangeLayout ()
{
	ASSERT_VALID (this);

	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	m_nBrowseButtonWidth = 20;
	int nImageMargin = 8;

	if (globalData.GetRibbonImageScale() != 1.0)
	{
		m_nBrowseButtonWidth = (int) (globalData.GetRibbonImageScale () * m_nBrowseButtonWidth);
		nImageMargin = (int) (globalData.GetRibbonImageScale () * nImageMargin);
	}

	m_nBrowseButtonWidth = max(m_nBrowseButtonWidth, m_sizeImage.cx + nImageMargin);

	SetWindowPos (NULL, 0, 0, 0, 0, 
		SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER | SWP_NOMOVE);

	if (m_Mode != BrowseMode_None)
	{
		GetWindowRect (m_rectBtn);
		m_rectBtn.left = m_rectBtn.right -  m_nBrowseButtonWidth;

		ScreenToClient (&m_rectBtn);
	}
	else
	{
		m_rectBtn.SetRectEmpty ();
	}
}
//********************************************************************************
void CBCGPEdit::OnNcLButtonDblClk(UINT /*nHitTest*/, CPoint /*point*/)
{
}
//********************************************************************************
void CBCGPEdit::OnBrowse ()
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);

	if (m_bSearchMode && !m_bTextIsEmpty)
	{
		SetWindowText(_T(""));
		m_bIsButtonHighlighted = FALSE;
		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE);
		return;
	}

	switch (m_Mode)
	{
	case BrowseMode_Folder:
#ifndef BCGP_EXCLUDE_SHELL
		{
#ifndef _BCGSUITE_
			CBCGPShellManager* pShellManager = g_pShellManager;
#else
			CShellManager* pShellManager = NULL;
			CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp ());
			if (pApp != NULL)
			{
				pShellManager = pApp->GetShellManager ();
			}
#endif
			if (pShellManager != NULL)
			{
				CString strFolder;
				GetWindowText (strFolder);

				CString strResult;
				if (pShellManager->BrowseForFolder (strResult, this, strFolder, m_strFolderBrowseTitle.IsEmpty() ? NULL : (LPCTSTR)m_strFolderBrowseTitle) &&
					(strResult != strFolder))
				{
					SetWindowText (strResult);
					SetModify (TRUE);
					OnAfterUpdate ();
				}
			}
			else
			{
				ASSERT (FALSE);
			}
		}
#endif
		break;

	case BrowseMode_File:
		{
			CString strEditorText;
			GetWindowText (strEditorText);

			if (strEditorText.FindOneOf (_T("*?<>|")) >= 0)
			{
				if (!OnIllegalFileName (strEditorText))
				{
					SetFocus ();
					return;
				}
			}

			CString strFileName;
			CString strInitialDir;

			if (!strEditorText.IsEmpty ())
			{
				DWORD dwAttrs = ::GetFileAttributes (strEditorText); // Check if strEditorText is a directory
				if ((dwAttrs != DWORD(-1)) && (0 != (dwAttrs & FILE_ATTRIBUTE_DIRECTORY)))
				{
					strInitialDir = strEditorText;
				}
				else
				{
					int iBackSlashPos = strEditorText.ReverseFind (_T('\\'));
					if (iBackSlashPos > 0)
					{
						strInitialDir = strEditorText.Left (iBackSlashPos);
						strFileName = strEditorText.Mid (iBackSlashPos + 1);
					}
					else // no backslash found
					{
						// use current directory
						strFileName = strEditorText;
					}
				}
			}

			CFileDialog dlg (TRUE, m_strDefFileExt, strFileName, 0, m_strFileFilter, GetParent ());
			
			// Setup initial directory if possible
			if (!strInitialDir.IsEmpty())
			{
				dlg.m_ofn.lpstrInitialDir = strInitialDir;
			}

			if (dlg.DoModal () == IDOK &&
				strEditorText != dlg.GetPathName ())
			{
				SetWindowText (dlg.GetPathName ());
				SetModify (TRUE);
				OnAfterUpdate ();
			}

			if (GetParent () != NULL)
			{
				GetParent ()->RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
			}
		}
		break;

	case BrowseMode_Calculator:
		{
			if (m_pCalcPopup != NULL)
			{
				m_pCalcPopup->SendMessage (WM_CLOSE);
				m_pCalcPopup = NULL;
				return;
			}

			CString strValue;
			GetWindowText (strValue);

			double dblValue = 0.;
			if (!strValue.IsEmpty ())
			{
				strValue.Replace (_T(','), _T('.'));
#if _MSC_VER < 1400
				_stscanf (strValue, _T("%lf"), &dblValue);
#else
				_stscanf_s (strValue, _T("%lf"), &dblValue);
#endif
			}

			m_pCalcPopup = new CBCGPCalculatorPopup (dblValue, 0, this);

			CBCGPCalculator* pCalc = DYNAMIC_DOWNCAST (CBCGPCalculator, m_pCalcPopup->GetMenuBar());
			if (pCalc != NULL)
			{
				ASSERT_VALID (pCalc);

				if (!m_lstCalcAdditionalCommands.IsEmpty ())
				{
					pCalc->SetAdditionalCommands (m_lstCalcAdditionalCommands);
				}

				if (!m_lstCalcExtCommands.IsEmpty ())
				{
					pCalc->SetExtendedCommands (m_lstCalcExtCommands);
				}

				if (!m_strCalcDisplayFormat.IsEmpty())
				{
					pCalc->SetDisplayFormat(m_strCalcDisplayFormat);
				}
			}

			CRect rectWindow;
			GetWindowRect (rectWindow);

			if (!m_pCalcPopup->Create (this, rectWindow.left, rectWindow.bottom, NULL, TRUE))
			{
				ASSERT (FALSE);
				m_pCalcPopup = NULL;
			}
			else
			{
				m_pCalcPopup->GetMenuBar()->SetFocus ();
				
				CRect rect;
				m_pCalcPopup->GetWindowRect (&rect);
				m_pCalcPopup->UpdateShadow (&rect);

				return;
			}
		}
		break;
	}

	SetFocus ();
}
//********************************************************************************
BOOL CBCGPEdit::OnIllegalFileName (CString& strFileName)
{
	CString strError;
	strError.LoadString (AFX_IDP_INVALID_FILENAME);

	BSTR bsError = strError.AllocSysString (); // Convert to unicode string
	BSTR bsFileName = strFileName.AllocSysString ();

	struct EDITBALLOONTIP_
	{
	    DWORD cbStruct;
		LPCWSTR pszTitle;
	    LPCWSTR pszText;
		INT ttiIcon;
	} tooltip;

	tooltip.cbStruct = sizeof (tooltip);
	tooltip.pszTitle = bsError;
	tooltip.pszText = bsFileName;
	tooltip.ttiIcon = 3; // TTI_ERROR constant

	if (!SendMessage(0x1503, 0, (LPARAM)&tooltip)) // Sending EM_SHOWBALLOONTIP message to edit control
	{
		CString strMessage;
		strMessage.Format (_T("%s\r\n%s"), strFileName, strError);
		MessageBox (strMessage, NULL, MB_OK | MB_ICONEXCLAMATION);
	}

	::SysFreeString (bsFileName);
	::SysFreeString (bsError);
	return FALSE;
}
//********************************************************************************
void CBCGPEdit::SetBrowseButtonImage (HICON hIcon, BOOL bAutoDestroy, BOOL bAlphaBlend)
{
	if (m_ImageBrowse.IsValid())
	{
		m_ImageBrowse.Clear ();
	}

	if (hIcon == NULL)
	{
		m_sizeImage = CSize (0, 0);
		return;
	}

	ICONINFO info;
	::GetIconInfo (hIcon, &info);

	BITMAP bmp;
	::GetObject (info.hbmColor, sizeof (BITMAP), (LPVOID) &bmp);

	m_sizeImage.cx = bmp.bmWidth;
	m_sizeImage.cy = bmp.bmHeight;

	::DeleteObject (info.hbmColor);
	::DeleteObject (info.hbmMask);

	m_ImageBrowse.SetImageSize(m_sizeImage);
	m_ImageBrowse.AddIcon(hIcon, bAlphaBlend);

#ifndef _BCGSUITE_
	if (globalData.GetRibbonImageScale() != 1.0 && !m_ImageBrowse.IsScaled())
	{
		m_ImageBrowse.SmoothResize(globalData.GetRibbonImageScale());

		m_sizeImage.cx = (int) (globalData.GetRibbonImageScale () * m_sizeImage.cx);
		m_sizeImage.cy = (int) (globalData.GetRibbonImageScale () * m_sizeImage.cy);
	}
#endif
	m_bDefaultImage = FALSE;

	if (bAutoDestroy)
	{
		::DestroyIcon (hIcon);
	}
}
//********************************************************************************
void CBCGPEdit::SetBrowseButtonImage (HBITMAP hBitmap, BOOL bAutoDestroy)
{
	if (m_ImageBrowse.IsValid())
	{
		m_ImageBrowse.Clear ();
	}

	if (hBitmap == NULL)
	{
		m_sizeImage = CSize (0, 0);
		return;
	}

	BITMAP bmp;
	::GetObject (hBitmap, sizeof (BITMAP), (LPVOID) &bmp);

	m_sizeImage.cx = bmp.bmWidth;
	m_sizeImage.cy = bmp.bmHeight;

	m_ImageBrowse.SetImageSize(m_sizeImage);
	m_ImageBrowse.AddImage(hBitmap, TRUE);

#ifndef _BCGSUITE_
	if (globalData.GetRibbonImageScale() != 1.0 && !m_ImageBrowse.IsScaled())
	{
		m_ImageBrowse.SmoothResize(globalData.GetRibbonImageScale());
		
		m_sizeImage.cx = (int) (globalData.GetRibbonImageScale () * m_sizeImage.cx);
		m_sizeImage.cy = (int) (globalData.GetRibbonImageScale () * m_sizeImage.cy);
	}
#endif
	m_bDefaultImage = FALSE;

	if (bAutoDestroy)
	{
		::DeleteObject (hBitmap);
	}
}
//********************************************************************************
void CBCGPEdit::SetBrowseButtonImage (UINT uiBmpResId)
{
	if (m_ImageBrowse.IsValid())
	{
		m_ImageBrowse.Clear ();
	}

	if (uiBmpResId == 0)
	{
		m_sizeImage = CSize (0, 0);
		return;
	}

	m_ImageBrowse.SetTransparentColor(globalData.clrBtnFace);

	if (!m_ImageBrowse.Load (uiBmpResId))
	{
		ASSERT (FALSE);
		return;
	}

	m_ImageBrowse.SetSingleImage();
	m_bDefaultImage = FALSE;
}
//*********************************************************************************
void CBCGPEdit::EnableFileBrowseButton (LPCTSTR lpszDefExt/* = NULL*/, LPCTSTR lpszFilter/* = NULL*/)
{
	ASSERT_VALID (this);

	m_strDefFileExt = lpszDefExt == NULL ? _T("") : lpszDefExt;
	m_strFileFilter = lpszFilter == NULL ? _T("") : lpszFilter;

	m_Mode = BrowseMode_File;
	SetIntenalImage ();

	globalUtils.EnableEditCtrlAutoComplete (GetSafeHwnd (), FALSE);

	OnChangeLayout ();
}
//*********************************************************************************
void CBCGPEdit::EnableFolderBrowseButton (LPCTSTR lpszTitle)
{
#ifdef BCGP_EXCLUDE_SHELL
	ASSERT (FALSE);
#else
	ASSERT_VALID (this);
#ifndef _BCGSUITE_
	ASSERT (g_pShellManager != NULL);	// You need to call CBCGPWorkspace::InitShellManager () first!
#endif

	m_strFolderBrowseTitle = lpszTitle == NULL ? _T("") : lpszTitle;

	m_Mode = BrowseMode_Folder;
	SetIntenalImage ();

	globalUtils.EnableEditCtrlAutoComplete (GetSafeHwnd (), TRUE);

	OnChangeLayout ();
#endif
}
//*********************************************************************************
void CBCGPEdit::EnableCalculatorButton (const CStringList* plstAdditionalCommands,
										const CList<UINT, UINT>* plstExtCommands,
										LPCTSTR lpszDisplayFormat)
{
	m_Mode = BrowseMode_Calculator;
	m_lstCalcAdditionalCommands.RemoveAll ();
	m_lstCalcExtCommands.RemoveAll();

	if (plstAdditionalCommands != NULL)
	{
		m_lstCalcAdditionalCommands.AddTail ((CStringList*) plstAdditionalCommands);
	}

	if (plstExtCommands != NULL)
	{
		m_lstCalcExtCommands.AddTail ((CList<UINT, UINT>*) plstExtCommands);
	}

	if (lpszDisplayFormat != NULL)
	{
		m_strCalcDisplayFormat = lpszDisplayFormat;
	}

#ifdef _BCGSUITE_
	m_strLabel = _T("c");
#else
	SetIntenalImage ();
#endif
	OnChangeLayout ();
}
//********************************************************************************
void CBCGPEdit::SetIntenalImage ()
{
	if (m_ImageBrowse.IsValid())
	{
		m_ImageBrowse.Clear ();
	}

	CBCGPLocalResource	lr;

	UINT uiImageListResID = globalData.Is32BitIcons () ? IDB_BCGBARRES_BROWSE32 : IDB_BCGBARRES_BROWSE;

	m_sizeImage = CSize (16, 15);
	m_ImageBrowse.SetImageSize(m_sizeImage);
	m_ImageBrowse.SetTransparentColor(RGB (255, 0, 255));
	m_ImageBrowse.Load(uiImageListResID);

	m_bDefaultImage = TRUE;
}
//********************************************************************************
void CBCGPEdit::OnAfterUpdate ()
{
	if (GetOwner () == NULL)
	{
		return;
	}

	GetOwner ()->PostMessage (EN_CHANGE, GetDlgCtrlID (), (LPARAM) GetSafeHwnd ());
	GetOwner ()->PostMessage (EN_UPDATE, GetDlgCtrlID (), (LPARAM) GetSafeHwnd ());
}
//**********************************************************************************
void CBCGPEdit::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	if (!m_bIsButtonCaptured)
	{
		CPoint ptClient = point;
		ScreenToClient (&ptClient);

		if (m_rectBtn.PtInRect (ptClient))
		{
			SetCapture ();
			m_bIsButtonHighlighted = TRUE;

			RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}
	
	CEdit::OnNcMouseMove(nHitTest, point);
}
//*********************************************************************************
void CBCGPEdit::OnCancelMode() 
{
	CEdit::OnCancelMode();
	
	CloseAutocompleteList();

	if (IsWindowEnabled ())
	{
		ReleaseCapture ();
	}

	m_bIsButtonPressed = FALSE;
	m_bIsButtonCaptured = FALSE;
	m_bIsButtonHighlighted = FALSE;

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
}
//********************************************************************************
void CBCGPEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_Mode != BrowseMode_None && m_rectBtn.PtInRect (point))
	{
		SetFocus ();

		m_bIsButtonPressed = TRUE;
		m_bIsButtonCaptured = TRUE;

		SetCapture ();

		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		return;
	}
	
	CEdit::OnLButtonDown(nFlags, point);
}
//********************************************************************************
BOOL CBCGPEdit::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_SYSKEYDOWN:
		if (m_Mode != BrowseMode_None && 
			(pMsg->wParam == VK_DOWN || pMsg->wParam == VK_RIGHT))
		{
			OnBrowse ();
			return TRUE;
		}

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
		break;

	case WM_CHAR:
		if (m_Mode == BrowseMode_Calculator && !FilterCalcKey ((int) pMsg->wParam))
		{
			return TRUE;
		}
		break;
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}
//*********************************************************************************
BOOL CBCGPEdit::FilterCalcKey (int nChar)
{
	CString str;
	GetWindowText (str);

	if (isdigit (nChar))
	{
		return TRUE;
	}

	switch (nChar)
	{
	case _T('-'):
		{
			int nStartPos, nEndPos;
			GetSel (nStartPos, nEndPos);

			return str.Find (_T('-')) == -1 && nStartPos == 0;
		}

	case _T('.'):
	case _T(','):
		return str.FindOneOf (_T(".,")) == -1;
	}

	return !isprint (nChar);
}
//***********************************************************************************
void CBCGPEdit::OnCalculatorUserCommand (CBCGPCalculator* /*pCalculator*/, 
										 UINT /*uiCmd*/)
{
	ASSERT (FALSE);	// Must be implemented in derived class
}
//************************************************************************************
LRESULT CBCGPEdit::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	return 0;
}
//*************************************************************************************
LRESULT CBCGPEdit::OnBCGSetControlAero (WPARAM wp, LPARAM)
{
	m_bOnGlass = (BOOL) wp;
	return 0;
}
//*************************************************************************************
BOOL CBCGPEdit::IsDrawPrompt()
{
	ASSERT_VALID(this);
	return (m_bSearchMode || m_bHasPrompt) && (m_bTextIsEmpty && !m_strSearchPrompt.IsEmpty() || !m_strErrorMessage.IsEmpty()) && (GetFocus() != this);
}
//*************************************************************************************
void CBCGPEdit::OnPaint() 
{
	m_bShowToolTip = FALSE;

	BOOL bDrawPrompt = IsDrawPrompt();

	if (!m_bOnGlass && !bDrawPrompt)
	{
		Default ();
		return;
	}

	CPaintDC dc(this); // device context for painting

	CBCGPMemDC memDC (dc, this, 255 /* Opaque */);
	CDC* pDC = &memDC.GetDC ();

	DoPaint(pDC, bDrawPrompt, FALSE);
}
//*************************************************************************************
void CBCGPEdit::DoPaint(CDC* pDC, BOOL bDrawPrompt, BOOL /*bIsPrint*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (bDrawPrompt)
	{
		// Fill control background:
		if (GetWindowTextLength() > 0)
		{
			CRect rectClient;
			GetClientRect(rectClient);

			if (m_bVisualManagerStyle && IsWindowEnabled())
			{
				if ((GetStyle() & ES_READONLY) == ES_READONLY)
				{
					CBrush& br = CBCGPVisualManager::GetInstance ()->GetDlgBackBrush (GetParent());
					pDC->FillRect(rectClient, &br);
				}
				else
				{
					CBrush& br = CBCGPVisualManager::GetInstance ()->GetEditCtrlBackgroundBrush(this);
					pDC->FillRect(rectClient, &br);
				}
			}
			else
			{
				if (IsWindowEnabled())
				{
					pDC->FillRect(rectClient, &globalData.brWindow);
				}
				else
				{
					pDC->FillRect(rectClient, &globalData.brBtnFace);
				}
			}
		}
		else
		{
			m_bDefaultPrintClient = TRUE;
			SendMessage (WM_PRINTCLIENT, (WPARAM) pDC->GetSafeHdc (), (LPARAM) PRF_CLIENT);
			m_bDefaultPrintClient = FALSE;
		}

		COLORREF clrText = m_strErrorMessage.IsEmpty() ? m_clrPrompt : m_clrErrorText;

		if (clrText == (COLORREF)-1)
		{
#ifndef _BCGSUITE_
			clrText = m_bVisualManagerStyle ? CBCGPVisualManager::GetInstance ()->GetToolbarEditPromptColor() : globalData.clrPrompt;
#else
			clrText = globalData.clrGrayedText;
#endif
		}

		pDC->SetTextColor(clrText);
		pDC->SetBkMode(TRANSPARENT);

		CFont* pOldFont = pDC->SelectObject (GetFont());

		CRect rectText;
		GetClientRect(rectText);

		DWORD lRes = GetMargins ();
		rectText.left += LOWORD(lRes);

		if ((GetStyle () & WS_BORDER) != 0 || (GetExStyle () & WS_EX_CLIENTEDGE) != 0)
		{
			rectText.DeflateRect (1, 1);
		}

		UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;
		const CString& str = m_strErrorMessage.IsEmpty() ? m_strSearchPrompt : m_strErrorMessage;

		if (pDC->GetTextExtent(str).cx > rectText.Width())
		{
			m_bShowToolTip = TRUE;

			if (m_pToolTip->GetSafeHwnd() == NULL)
			{
				CBCGPTooltipManager::CreateToolTip (m_pToolTip, this, BCGP_TOOLTIP_TYPE_DEFAULT);

				if (m_pToolTip->GetSafeHwnd () != NULL)
				{
					m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectText, 1);
				}
			}
		}

		if (m_bOnGlass)
		{
			CBCGPVisualManager::GetInstance ()->DrawTextOnGlass(pDC, str, rectText, nFormat, 0, clrText);
		}
		else
		{
			pDC->DrawText(str, rectText, nFormat);
		}

		pDC->SelectObject (pOldFont);
	}
	else
	{
		m_bDefaultPrintClient = TRUE;
		SendMessage (WM_PRINTCLIENT, (WPARAM) pDC->GetSafeHdc (), (LPARAM) PRF_CLIENT);
		m_bDefaultPrintClient = FALSE;
	}
}
//**********************************************************************************************************
BOOL CBCGPEdit::OnChange() 
{
	if (m_bOnGlass)
	{
		InvalidateRect (NULL, FALSE);
		UpdateWindow ();
	}

#ifndef _BCGSUITE_
	BOOL bIsAutocompleteAvailable = (m_Mode == BrowseMode_None || m_Mode == BrowseMode_Default);

	if (bIsAutocompleteAvailable && !m_bInAutoComplete)
	{
		BOOL bDestroyDropDown = TRUE;
		
		CString strText;
		GetWindowText(strText);

		if (!strText.IsEmpty() && (GetStyle() & ES_MULTILINE) == 0)
		{
			CStringList	lstAutocomplete;
			if (OnGetAutoCompleteList(strText, lstAutocomplete) && !lstAutocomplete.IsEmpty())
			{
				bDestroyDropDown = FALSE;

				if (::IsWindow(m_pDropDownPopup->GetSafeHwnd()) && m_pDropDownPopup->Compare(lstAutocomplete))
				{
					// Keep existing list
				}
				else
				{
					CreateAutocompleteList(lstAutocomplete);
				}
			}
		}

		if (bDestroyDropDown)
		{
			CloseAutocompleteList();
		}
	}
#endif

	if (m_bSearchMode || m_bHasPrompt)
	{
		BOOL bTextIsEmpty = m_bTextIsEmpty;

		CString str;
		GetWindowText (str);

		m_bTextIsEmpty = str.IsEmpty();

		if (!m_strErrorMessage.IsEmpty())
		{
			SetErrorMessage(NULL, m_clrErrorText);
		}
		else if (bTextIsEmpty != m_bTextIsEmpty)
		{
			RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		}
	}

	return FALSE;
}
//**********************************************************************************************************
LRESULT CBCGPEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	CPoint ptCaret (-1, -1);

	BOOL bCheckSel = FALSE;

	if (m_bOnGlass)
	{
		if ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) ||
			(message >= WM_KEYFIRST && message <= WM_KEYLAST))
		{
			ptCaret = GetCaretPos ();
			bCheckSel = TRUE;
		}
	}

	LRESULT lres = CEdit::WindowProc(message, wParam, lParam);

	if (bCheckSel)
	{
		if (GetSel () != 0 || ptCaret != GetCaretPos ())
		{
			InvalidateRect (NULL, FALSE);
			UpdateWindow ();
		}
	}

	if (message == EM_SETSEL && m_bOnGlass)
	{
		InvalidateRect (NULL, FALSE);
		UpdateWindow ();
	}

	return lres;
}
//**********************************************************************************************************
HBRUSH CBCGPEdit::CtlColor(CDC* pDC, UINT /*nCtlColor*/) 
{
	if (m_bVisualManagerStyle)
	{
		CBrush& br = CBCGPVisualManager::GetInstance ()->GetEditCtrlBackgroundBrush(this);

		if ((GetStyle() & ES_READONLY) == ES_READONLY || !IsWindowEnabled())
		{
			LOGBRUSH lbr;
			br.GetLogBrush(&lbr);
			
			pDC->SetBkColor(lbr.lbColor);

			if (!IsWindowEnabled())
			{
				pDC->SetTextColor (CBCGPVisualManager::GetInstance ()->GetEditCtrlTextColor(this));
			}
			else
			{
				pDC->SetTextColor (globalData.clrBarText);
			}
			
			return (HBRUSH)br.GetSafeHandle();
		}
		else
		{
			LOGBRUSH lbr;
			br.GetLogBrush(&lbr);
			
			pDC->SetBkColor(lbr.lbColor);
			pDC->SetTextColor (CBCGPVisualManager::GetInstance ()->GetEditCtrlTextColor(this));

			return (HBRUSH)br.GetSafeHandle();
		}
	}

	return NULL;
}
//**************************************************************************
void CBCGPEdit::SetPrompt(LPCTSTR lpszPrompt, COLORREF clrText, BOOL bRedraw)
{
	ASSERT_VALID (this);

	CString strOldPrompt = m_strSearchPrompt;
	BOOL bColorWasChanged = m_clrPrompt != clrText;

	m_strSearchPrompt = (lpszPrompt == NULL) ? _T("") : lpszPrompt;
	m_clrPrompt = clrText;
	m_bHasPrompt = !m_strSearchPrompt.IsEmpty() || !m_strErrorMessage.IsEmpty();
	
	if (GetSafeHwnd() != NULL)
	{
		BOOL bTextWasEmpty = m_bTextIsEmpty;

		CString str;
		GetWindowText (str);
		
		m_bTextIsEmpty = str.IsEmpty();

		if (bRedraw && (bColorWasChanged || m_strSearchPrompt != strOldPrompt || bTextWasEmpty != m_bTextIsEmpty))
		{
			RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		}
	}
}
//**************************************************************************
void CBCGPEdit::SetErrorMessage(LPCTSTR lpszMessage, COLORREF clrText, BOOL bRedraw)
{
	ASSERT_VALID (this);
	
	CString strOldMessage = m_strErrorMessage;
	BOOL bColorWasChanged = m_clrErrorText != clrText;
	
	m_strErrorMessage = (lpszMessage == NULL) ? _T("") : lpszMessage;
	m_clrErrorText = clrText;
	m_bHasPrompt = !m_strSearchPrompt.IsEmpty() || !m_strErrorMessage.IsEmpty();
	
	if (GetSafeHwnd() != NULL)
	{
		CString str;
		GetWindowText (str);
		
		m_bTextIsEmpty = str.IsEmpty();
		
		if (bRedraw && (bColorWasChanged || strOldMessage != m_strErrorMessage))
		{
			RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		}
	}
}
//**************************************************************************
void CBCGPEdit::EnableSearchMode(BOOL bEnable, LPCTSTR lpszPrompt, COLORREF clrText, BOOL bRedraw)
{
	ASSERT_VALID (this);

	m_bSearchMode = bEnable;

	if (m_bSearchMode)
	{
		ASSERT(lpszPrompt != NULL);

		m_strSearchPrompt = lpszPrompt;
		m_clrPrompt = clrText;

		if (!m_ImageSearch.IsValid())
		{
			CBCGPLocalResource locaRes;
			m_ImageSearch.Load(globalData.Is32BitIcons () ?
				IDB_BCGBARRES_SEARCH32 : IDB_BCGBARRES_SEARCH);
			m_ImageSearch.SetSingleImage();
			m_ImageSearch.SetTransparentColor(globalData.clrBtnFace);
		}

		if (!m_ImageClear.IsValid())
		{
			CBCGPLocalResource locaRes;
			m_ImageClear.Load(globalData.Is32BitIcons () ?
				IDB_BCGBARRES_CLEAR32 : IDB_BCGBARRES_CLEAR);
			m_ImageClear.SetSingleImage();
			m_ImageClear.SetTransparentColor(globalData.clrBtnFace);
		}
	}
	else
	{
		if (m_ImageClear.IsValid())
		{
			m_ImageClear.Clear();
		}

		if (m_ImageSearch.IsValid())
		{
			m_ImageSearch.Clear();
		}
	}

	EnableBrowseButton(bEnable);

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}
}
//**************************************************************************
void CBCGPEdit::OnDestroy() 
{
	CloseAutocompleteList();
	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);
	CEdit::OnDestroy();
}
//*****************************************************************************************
BOOL CBCGPEdit::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	if (m_bShowToolTip && pNMH->hwndFrom == m_pToolTip->GetSafeHwnd ())
	{
		const CString& str = m_strErrorMessage.IsEmpty() ? m_strSearchPrompt : m_strErrorMessage;
		pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) str);
		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************
LRESULT CBCGPEdit::OnPrintClient(WPARAM wp, LPARAM lp)
{
	DWORD dwFlags = (DWORD)lp;
	
	BOOL bDefaultPrint = TRUE;
	
	if ((dwFlags & PRF_CLIENT) == PRF_CLIENT)
	{
		BOOL bDrawPrompt = IsDrawPrompt();

		if ((m_bOnGlass || bDrawPrompt) && !m_bDefaultPrintClient)
		{
			CDC* pDC = CDC::FromHandle((HDC) wp);
			ASSERT_VALID(pDC);

			DoPaint(pDC, bDrawPrompt, TRUE);
			bDefaultPrint = FALSE;
		}
	}

	if (bDefaultPrint)
	{
		return Default();
	}

	return 0;
}
//*****************************************************************************
LRESULT CBCGPEdit::OnPrint(WPARAM wp, LPARAM lp)
{
	DWORD dwFlags = (DWORD)lp;
	
	if ((dwFlags & PRF_NONCLIENT) == PRF_NONCLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		const BOOL bHasBorder = (GetExStyle () & WS_EX_CLIENTEDGE) || (GetStyle () & WS_BORDER);
		
		if (bHasBorder && (m_bVisualManagerStyle || m_bOnGlass))
		{
			CRect rect;
			GetWindowRect(rect);

			rect.bottom -= rect.top;
			rect.right -= rect.left;
			rect.left = rect.top = 0;

#ifndef _BCGSUITE_
			visualManagerMFC->OnDrawControlBorder(pDC, rect, this, m_bOnGlass);
#else
			visualManagerMFC->OnDrawControlBorder(this);
#endif
		}
		
		if (m_Mode != BrowseMode_None)
		{
			DoNcPaint(pDC, TRUE);
		}
	}

	return Default();
}
//*****************************************************************************
void CBCGPEdit::OnKillFocus(CWnd* pNewWnd) 
{
#ifndef _BCGSUITE_
	if (::IsWindow(m_pDropDownPopup->GetSafeHwnd()))
	{
		int nIndex = m_pDropDownPopup->GetCurSel();
		if (nIndex >= 0)
		{
			CString strItemText;
			m_pDropDownPopup->GetText(nIndex, strItemText);
			
			m_bInAutoComplete = TRUE;
			SetWindowText(strItemText);
			m_bInAutoComplete = FALSE;
		}

		m_pDropDownPopup->DestroyWindow();
		m_pDropDownPopup = NULL;
	}
#endif

	CEdit::OnKillFocus(pNewWnd);
}

#ifndef _BCGSUITE_
BOOL CBCGPEdit::OnGetAutoCompleteList(const CString& strEditText, CStringList& lstAutocomplete)
{
	UNREFERENCED_PARAMETER(strEditText);

	CWnd* pWndOwner = GetOwner();
	if (pWndOwner->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	pWndOwner->SendMessage(BCGM_EDIT_ON_FILL_AUTOCOMPLETE_LIST, (WPARAM)GetDlgCtrlID(), (LPARAM)&lstAutocomplete);

	return !lstAutocomplete.IsEmpty();
}
#endif

void CBCGPEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
#ifndef _BCGSUITE_
	if (::IsWindow(m_pDropDownPopup->GetSafeHwnd()))
	{
		if (nChar == VK_UP || nChar == VK_DOWN)
		{
			return;
		}
	}
	else if (nChar == VK_DOWN && (GetStyle() & ES_MULTILINE) == 0)
	{
		CString strText;
		GetWindowText(strText);
		
		CStringList	lstAutocomplete;
		if (OnGetAutoCompleteList(strText, lstAutocomplete) && !lstAutocomplete.IsEmpty())
		{
			CreateAutocompleteList(lstAutocomplete);
		}
	}

#endif
	
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************
void CBCGPEdit::CreateAutocompleteList(const CStringList& lstAutocomplete)
{
#ifndef _BCGSUITE_
	CloseAutocompleteList();

	m_pDropDownPopup = new CBCGPEditDropDownList(this);
	
	for (POSITION pos = lstAutocomplete.GetHeadPosition (); pos != NULL;)
	{
		m_pDropDownPopup->AddString(lstAutocomplete.GetNext(pos));
	}
	
	BOOL bIsRTL = (GetExStyle () & WS_EX_RTLREADING);
	
	CRect rect;
	GetWindowRect(&rect);
	
	m_pDropDownPopup->SetMaxHeight(15 * globalData.GetTextHeight());
	m_pDropDownPopup->Track(CPoint(bIsRTL ? rect.right : rect.left, rect.bottom), GetOwner());

	if (m_pDropDownPopup->m_wndScrollBarVert.GetSafeHwnd() != NULL)
	{
		m_pDropDownPopup->EnableVertResize(2 * globalData.GetTextHeight());
		m_pDropDownPopup->RecalcLayout();
		m_pDropDownPopup->UpdateShadow();
	}
#else
	UNREFERENCED_PARAMETER(lstAutocomplete);
#endif
}
//*****************************************************************************
void CBCGPEdit::CloseAutocompleteList()
{
#ifndef _BCGSUITE_
	if (::IsWindow(m_pDropDownPopup->GetSafeHwnd()))
	{
		m_pDropDownPopup->DestroyWindow();
		m_pDropDownPopup = NULL;
	}
#endif
}
