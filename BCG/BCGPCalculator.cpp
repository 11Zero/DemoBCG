// BCGPCalculator.cpp : implementation file
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//

#include "stdafx.h"

#include "BCGCBPro.h"
#include "BCGPMath.h"

#include "BCGPCalculator.h"
#include "BCGGlobals.h"
#include "BCGPLocalResource.h"
#include "BCGProRes.h"
#include "BCGPDlgImpl.h"

#ifndef _BCGSUITE_
#include "BCGPPopupMenu.h"
#include "trackmouse.h"
#include "BCGPPropList.h"
#include "BCGPRibbonComboBox.h"
#endif

#include "BCGPEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int SEPARATOR_SIZE = 2;
static const int nDefaultButtonSize = 25;

/////////////////////////////////////////////////////////////////////////////
// CCalculatorButton

class CCalculatorButton : public CBCGPToolbarButton  
{
	friend class CBCGPCalculator;

	DECLARE_SERIAL(CCalculatorButton)

protected:
	CCalculatorButton(short nDigit = 0)
	{
		m_strText.Format (_T("%d"), nDigit);
		m_nImageIndex = (int) nDigit;
		m_nDigit = nDigit;
		m_uiCmd = CBCGPCalculator::idCommandNone;
		m_bText = TRUE;
		m_bImage = FALSE;
		m_nKbdAccel = _T('0') + nDigit;
		m_nKbdAccel2 = 0;
		m_pParent = NULL;
		m_bIsVitKey = FALSE;
		m_bIsCtrl = FALSE;
		m_bIsUserCommand = FALSE;
	}

	CCalculatorButton(LPCTSTR lpszName, CBCGPCalculator::CalculatorCommands uiCMD, int nKbdAccel,
		int nKbdAccel2 = 0, BOOL bIsVitKey = FALSE, BOOL bIsCtrl = FALSE,
		BOOL bIsUserCommand = FALSE)
	{
		ASSERT(lpszName != NULL);

		m_strText = lpszName;

		if (m_strText.IsEmpty())
		{
			m_strText = CBCGPCalculator::CommandToName(uiCMD);
		}

		m_nDigit = -1;
		m_uiCmd = uiCMD;
		m_bText = TRUE;
		m_bImage = FALSE;
		m_nKbdAccel = nKbdAccel;
		m_nKbdAccel2 = nKbdAccel2;
		m_pParent = NULL;
		m_bIsVitKey = bIsVitKey;
		m_bIsCtrl = bIsCtrl;
		m_bIsUserCommand = bIsUserCommand;

		m_nImageIndex = -1;

		if (CBCGPCalculator::idCommandClear <= uiCMD && uiCMD <= CBCGPCalculator::idCommandBackspace)
		{
			m_nImageIndex = 10 + (uiCMD - CBCGPCalculator::idCommandClear); 
		}
		else if (CBCGPCalculator::idCommandAdvSin <= uiCMD && uiCMD <= CBCGPCalculator::idCommandAdvPi)
		{
			m_nImageIndex = 10 + (uiCMD - CBCGPCalculator::idCommandAdvSin + CBCGPCalculator::idCommandBackspace);
		}
	}

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);

	virtual BOOL OnToolHitTest(const CWnd* /*pWnd*/, TOOLINFO* /*pTI*/)
	{
		return FALSE;
	}

	virtual void OnChangeParentWnd (CWnd* pWndParent)
	{
		CBCGPToolbarButton::OnChangeParentWnd (pWndParent);

		m_pParent = DYNAMIC_DOWNCAST (CBCGPCalculator, pWndParent);
	}

	short								m_nDigit;
	CBCGPCalculator::CalculatorCommands	m_uiCmd;
	int									m_nKbdAccel;
	int									m_nKbdAccel2;
	CBCGPCalculator*					m_pParent;
	BOOL								m_bIsVitKey;
	BOOL								m_bIsCtrl;
	BOOL								m_bIsUserCommand;
	int									m_nImageIndex;
};

void CCalculatorButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* /*pImages*/,
								BOOL /*bHorz*/, BOOL /*bCustomizeMode*/, BOOL bHighlight,
								BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	CBCGPVisualManager::BCGBUTTON_STATE state = CBCGPVisualManager::ButtonsIsRegular;

	if (!CBCGPToolBar::IsCustomizeMode ())
	{
		if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			state = CBCGPVisualManager::ButtonsIsPressed;
		}
		else if (bHighlight)
		{
			state = CBCGPVisualManager::ButtonsIsHighlighted;
		}
	}

	if (m_pParent->OnDrawButton (pDC, rect, this, state, m_uiCmd))
	{
		if (m_nImageIndex < 0 || m_pParent->m_CalcImages.GetCount() == 0 || globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
		{
			COLORREF clrText = CBCGPVisualManager::GetInstance ()->GetCalculatorButtonTextColor(m_bIsUserCommand, m_uiCmd);

			pDC->SetTextColor (clrText);

			CRect rectText = rect;
			pDC->DrawText (m_strText, &rectText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		}
		else
		{
			CBCGPToolBarImages& images = m_bIsUserCommand ? m_pParent->m_CalcImagesUser :
				(m_uiCmd != CBCGPCalculator::idCommandNone && m_uiCmd != CBCGPCalculator::idCommandDot) ? m_pParent->m_CalcImagesCommands : m_pParent->m_CalcImagesDigits;

			CRect rectImage (rect);
			if ((rectImage.Height () % 2) != 0)
			{
				rectImage.bottom--;
			}
			images.DrawEx(pDC, rectImage, m_nImageIndex, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
		}

	}
}

IMPLEMENT_SERIAL(CCalculatorButton, CBCGPToolbarButton, 1)

/////////////////////////////////////////////////////////////////////////////
// CCalculatorButton

class CCalculatorDisplay : public CBCGPToolbarButton  
{
	friend class CBCGPCalculator;

	DECLARE_SERIAL(CCalculatorDisplay)

protected:
	CCalculatorDisplay(LPCTSTR lpszValue = NULL)
	{
		m_strText = lpszValue == NULL ? _T("0") : lpszValue;
		m_bMem = FALSE;
		m_pParent = NULL;
	}

	virtual void OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* pImages,
						BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
						BOOL bHighlight = FALSE,
						BOOL bDrawBorder = TRUE,
						BOOL bGrayDisabledButtons = TRUE);

	virtual BOOL OnToolHitTest(const CWnd* /*pWnd*/, TOOLINFO* /*pTI*/)
	{
		return FALSE;
	}

	virtual void OnChangeParentWnd (CWnd* pWndParent)
	{
		CBCGPToolbarButton::OnChangeParentWnd (pWndParent);

		m_pParent = DYNAMIC_DOWNCAST (CBCGPCalculator, pWndParent);
	}

	BOOL				m_bMem;
	CBCGPCalculator*	m_pParent;
};

IMPLEMENT_SERIAL(CCalculatorDisplay, CBCGPToolbarButton, 1)

void CCalculatorDisplay::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* /*pImages*/,
								BOOL /*bHorz*/, BOOL /*bCustomizeMode*/, BOOL /*bHighlight*/,
								BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	const CString strMem = _T("M");

	if (m_pParent->OnDrawDisplay (pDC, rect, m_strText, m_bMem))
	{
#ifndef _BCGSUITE_
		COLORREF clrText = CBCGPVisualManager::GetInstance ()->GetCalculatorDisplayTextColor();
		COLORREF clrTextOld = (clrText == (COLORREF)-1) ? (COLORREF)-1 : pDC->SetTextColor(clrText);
#endif
		CRect rectText = rect;

		if (m_bMem)
		{
			rectText.left += pDC->GetTextExtent (strMem).cx;
		}

		rectText.DeflateRect (4, 2);

		pDC->DrawText (m_strText, &rectText, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

		if (m_bMem)
		{
			rectText = rect;
			rectText.DeflateRect (4, 2);

			pDC->DrawText (strMem, &rectText, DT_SINGLELINE);
		}

#ifndef _BCGSUITE_
		if (clrTextOld != (COLORREF)-1)
		{
			pDC->SetTextColor(clrTextOld);
		}
#endif
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPCalculator

IMPLEMENT_SERIAL(CBCGPCalculator, CBCGPPopupMenuBar, 1)

CMap<UINT, UINT, CString, const CString&> CBCGPCalculator::m_mapCommandNames;

CBCGPCalculator::CBCGPCalculator()
{
	m_dblValue = 0.;
	m_nCommandID = 0;
	m_pParentEdit = NULL;
	m_pParentRibbonEdit = NULL;

	CommonInit ();
}
//**************************************************************************************
CBCGPCalculator::CBCGPCalculator (CBCGPCalculator& src, UINT uiCommandID) :
		m_dblValue (src.m_dblValue),
		m_nCommandID (uiCommandID)
{
	CommonInit ();
	m_pParentEdit = NULL;
	m_pParentRibbonEdit = NULL;
}

#ifndef BCGP_EXCLUDE_RIBBON
#ifndef _BCGSUITE_

CBCGPCalculator::CBCGPCalculator(double dblValue, UINT nID, CBCGPRibbonComboBox* pParentRibbonEdit) :
		m_dblValue (dblValue),
		m_nCommandID (nID),
		m_pParentRibbonEdit (pParentRibbonEdit)
{
	CommonInit ();
	m_pParentEdit = NULL;
}
#endif
#endif

CBCGPCalculator::CBCGPCalculator(double dblValue, UINT nID, CBCGPEdit* pParentEdit) :
		m_dblValue (dblValue),
		m_nCommandID (nID),
		m_pParentEdit (pParentEdit)
{
	CommonInit ();
	m_pParentRibbonEdit = NULL;
}
//**************************************************************************************
void CBCGPCalculator::CommonInit ()
{
	m_bHasBorder = FALSE;
	m_bLocked = TRUE;
	m_bIsEnabled = TRUE;
	m_pWndPropList = NULL;
	m_bInternal = FALSE;
	m_nVertMargin = 3;
	m_nHorzMargin = 3;
	m_dblSecondValue = 0.;
	m_dblMemValue = 0.;
	m_uiLastCmd = 0;

	m_bIsClearBuffer = TRUE;
	m_bIsError = FALSE;
	m_bSeqResult = FALSE;

	m_nRows = 5;
	m_nColumns = 5;

	m_strDisplayFormat = _T("%.16lf");

	InitCommandNames();

	UpdateBuffer ();
}
//*********************************************************************************
void CBCGPCalculator::InitCommandNames()
{
	if (m_mapCommandNames.IsEmpty())
	{
		m_mapCommandNames.SetAt(idCommandClear,		_T("CE"));
		m_mapCommandNames.SetAt(idCommandReset,		_T("C"));
		m_mapCommandNames.SetAt(idCommandMemClear,	_T("MC"));
		m_mapCommandNames.SetAt(idCommandMemRead,	_T("MR"));
		m_mapCommandNames.SetAt(idCommandMemAdd,	_T("M+"));
		m_mapCommandNames.SetAt(idCommandAdd,		_T("+"));
		m_mapCommandNames.SetAt(idCommandSub,		_T("-"));
		m_mapCommandNames.SetAt(idCommandMult,		_T("*"));
		m_mapCommandNames.SetAt(idCommandDiv,		_T("/"));
		m_mapCommandNames.SetAt(idCommandSign,		_T("+/-"));
		m_mapCommandNames.SetAt(idCommandDot,		_T("."));
		m_mapCommandNames.SetAt(idCommandSqrt,		_T("Sqrt"));
		m_mapCommandNames.SetAt(idCommandPercent,	_T("%"));
		m_mapCommandNames.SetAt(idCommandResult,	_T("="));
		m_mapCommandNames.SetAt(idCommandRev,		_T("1/x"));
		m_mapCommandNames.SetAt(idCommandBackspace,	_T("<-"));
		m_mapCommandNames.SetAt(idCommandAdvSin,	_T("Sin"));
		m_mapCommandNames.SetAt(idCommandAdvCos,	_T("Cos"));
		m_mapCommandNames.SetAt(idCommandAdvTan,	_T("Tan"));
		m_mapCommandNames.SetAt(idCommandAdvX2,		_T("X^2"));
		m_mapCommandNames.SetAt(idCommandAdvX3,		_T("X^3"));
		m_mapCommandNames.SetAt(idCommandAdvExp,	_T("Exp"));
		m_mapCommandNames.SetAt(idCommandAdv10x,	_T("10^x"));
		m_mapCommandNames.SetAt(idCommandAdvLn,		_T("Ln"));
		m_mapCommandNames.SetAt(idCommandAdvLog,	_T("Log"));
		m_mapCommandNames.SetAt(idCommandAdvPi,		_T("Pi"));
	}
}
//*********************************************************************************
CBCGPCalculator::~CBCGPCalculator()
{
}
//*************************************************************************************
void CBCGPCalculator::AdjustLocations ()
{
	if (GetSafeHwnd () == NULL || !::IsWindow (m_hWnd) || m_bInUpdateShadow || m_Buttons.IsEmpty())
	{
		return;
	}

	ASSERT_VALID(this);

	CRect rectClient;	// Client area rectangle
	GetClientRect (rectClient);

	if (rectClient.Width () < 150 || rectClient.Height () < 150)
	{
		m_nVertMargin = 1;
		m_nHorzMargin = 1;
	}
	else
	{
		m_nVertMargin = 3;
		m_nHorzMargin = 3;
	}

	rectClient.DeflateRect (m_nHorzMargin, m_nVertMargin);

	int cx = max (nDefaultButtonSize, rectClient.Width () / m_nColumns);

	POSITION pos = m_Buttons.GetHeadPosition ();

	CBCGPToolbarButton* pDisplayButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
	ASSERT_VALID (pDisplayButton);

	CClientDC dc (this);
	CFont* pOldFont = SelectDefaultFont (&dc);
	ASSERT_VALID (pOldFont);

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	const int nDisplayHeight = tm.tmHeight + 2 +  4 * m_nVertMargin;

	dc.SelectObject (pOldFont);

	m_rectDisplay = CRect (rectClient.TopLeft (),  CSize (rectClient.Width (), nDisplayHeight));
	m_rectDisplay.DeflateRect (m_nHorzMargin, m_nVertMargin);

	CRect rectBackspace = m_rectDisplay;
	rectBackspace.left = rectBackspace.right - cx;
	rectBackspace.DeflateRect (m_nHorzMargin, 0);

	m_rectDisplay.right = rectBackspace.left - m_nHorzMargin;
	pDisplayButton->SetRect (m_rectDisplay);

	CBCGPToolbarButton* pBackspaceButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
	ASSERT_VALID (pBackspaceButton);

	pBackspaceButton->SetRect (rectBackspace);

	rectClient.top = m_rectDisplay.bottom + m_nVertMargin;

	int x = rectClient.left;
	int y = rectClient.top;

	CSize sizeButton (	cx, 
						max (nDefaultButtonSize, rectClient.Height () / m_nRows));

	for (int i = 0; pos != NULL; i++)
	{
		if (i == m_nColumns)
		{
			rectBackspace.right = x - m_nHorzMargin;
			rectBackspace.left = x - sizeButton.cx + m_nHorzMargin;

			pBackspaceButton->SetRect (rectBackspace);

			m_rectDisplay.right = rectBackspace.left - 2 * m_nHorzMargin;
			pDisplayButton->SetRect (m_rectDisplay);

			i = 0;

			x = rectClient.left;
			y += sizeButton.cy;
		}

		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_Buttons.GetNext (pos);
		ASSERT_VALID (pButton);

		CRect rectButton = CRect (CPoint (x, y), sizeButton);
		rectButton.DeflateRect (m_nHorzMargin, m_nVertMargin);

		pButton->SetRect (rectButton);

		x += sizeButton.cx;
	}

	UpdateTooltips ();
}
//***************************************************************************************
CSize CBCGPCalculator::CalcSize (BOOL /*bVertDock*/)
{
	CClientDC dc (this);
	CFont* pOldFont = SelectDefaultFont (&dc);
	ASSERT_VALID (pOldFont);

	CSize sizeBox (nDefaultButtonSize + 2 * m_nHorzMargin, nDefaultButtonSize + 2 * m_nVertMargin);

	POSITION pos = m_Buttons.GetHeadPosition ();
	if (pos == NULL)
	{
		return CSize (0, 0);
	}

	m_Buttons.GetNext (pos);	// Skip display

	while (pos != NULL)
	{
		CBCGPToolbarButton* pButton = DYNAMIC_DOWNCAST (CBCGPToolbarButton, m_Buttons.GetNext (pos));
		ASSERT_VALID (pButton);

		CSize sizeText = dc.GetTextExtent (pButton->m_strText);

		sizeBox.cx = max (sizeBox.cx, sizeText.cx + 2 * m_nHorzMargin + 8);
		sizeBox.cy = max (sizeBox.cy, sizeText.cy + 2 * m_nVertMargin + 8);
	}

	dc.SelectObject (pOldFont);

	return CSize(
		m_nColumns * sizeBox.cx + 2 * m_nHorzMargin, 
		globalData.GetTextHeight () + 4 * m_nVertMargin + m_nRows * sizeBox.cy + 2 * m_nVertMargin);
}

BEGIN_MESSAGE_MAP(CBCGPCalculator, CBCGPPopupMenuBar)
	//{{AFX_MSG_MAP(CBCGPCalculator)
	ON_WM_CREATE()
	ON_WM_NCPAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
#ifdef _BCGSUITE_
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnSetControlVMMode)
#endif
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPCalculator message handlers

void CBCGPCalculator::OnNcPaint() 
{
	CBCGPToolBar::OnNcPaint();
}
//**************************************************************************************
int CBCGPCalculator::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPPopupMenuBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	CBCGPLocalResource locaRes;

	m_CalcImages.SetImageSize (CSize(25, 20));
	m_CalcImages.Load(IDB_BCGBARRES_CALCBUTTONS);

#if (!defined _BCGSUITE_) || (_MSC_VER >= 1600)
	m_CalcImages.SmoothResize(globalData.GetRibbonImageScale ());
#endif

	OnChangeVisualManager(0, 0);
	
	m_bLeaveFocus = FALSE;
	Rebuild ();

	if (m_pWndPropList != NULL || m_pParentEdit != NULL || m_pParentRibbonEdit != NULL)
	{
		SetCapture ();
	}

	return 0;
}
//*************************************************************************************
CString CBCGPCalculator::CommandToName(CalculatorCommands cmd)
{
	InitCommandNames();

	CString strName;
	m_mapCommandNames.Lookup(cmd, strName);

	return strName;
}
//*************************************************************************************
CBCGPCalculator::CalculatorCommands CBCGPCalculator::NameToCommand(const CString& strName)
{
	InitCommandNames();

	for (POSITION pos = m_mapCommandNames.GetStartPosition (); pos != NULL;)
	{
		UINT uiCmd = 0;
		CString strMapName;

		m_mapCommandNames.GetNextAssoc(pos, uiCmd, strMapName);

		if (strMapName.CompareNoCase(strName) == 0)
		{
			return (CalculatorCommands) uiCmd;
		}
	}

	return idCommandNone;
}
//*************************************************************************************
void CBCGPCalculator::Rebuild ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	RemoveAllButtons ();

	InsertButton (new CCalculatorDisplay (m_strBuffer));

	InsertButton (new CCalculatorButton (_T(""),idCommandBackspace, VK_BACK, 0, TRUE));

	InsertButton (new CCalculatorButton (_T(""),idCommandMemClear, _T('L'), _T('l'), TRUE, TRUE));
	InsertButton (new CCalculatorButton (_T(""),idCommandMemRead, _T('R'), _T('r'), TRUE, TRUE));
	InsertButton (new CCalculatorButton (_T(""),idCommandMemAdd, _T('P'), _T('p'), TRUE, TRUE));
	InsertButton (new CCalculatorButton (_T(""),idCommandReset, VK_ESCAPE, 0, TRUE));
	InsertButton (new CCalculatorButton (_T(""),idCommandClear, VK_DELETE, 0, TRUE));
	
	InsertButton (new CCalculatorButton (7));
	InsertButton (new CCalculatorButton (8));
	InsertButton (new CCalculatorButton (9));

	InsertButton (new CCalculatorButton (_T(""),idCommandDiv, _T('/')));
	InsertButton (new CCalculatorButton (_T(""),idCommandMult, _T('*')));

	InsertButton (new CCalculatorButton (4));
	InsertButton (new CCalculatorButton (5));
	InsertButton (new CCalculatorButton (6));

	InsertButton (new CCalculatorButton (_T(""),idCommandSign, VK_F9, 0, TRUE));
	InsertButton (new CCalculatorButton (_T(""),idCommandSub, _T('-')));

	InsertButton (new CCalculatorButton (1));
	InsertButton (new CCalculatorButton (2));
	InsertButton (new CCalculatorButton (3));

	InsertButton (new CCalculatorButton (_T(""),idCommandSqrt, _T('@')));
	InsertButton (new CCalculatorButton (_T(""),idCommandAdd, _T('+')));

	InsertButton (new CCalculatorButton (0));
	InsertButton (new CCalculatorButton (_T(""),idCommandDot, _T('.'), _T(',')));
	InsertButton (new CCalculatorButton (_T(""),idCommandResult, VK_RETURN, 0, TRUE));
	InsertButton (new CCalculatorButton (_T(""),idCommandPercent, _T('%')));
	InsertButton (new CCalculatorButton (_T(""),idCommandRev, _T('R'), _T('r')));

	POSITION pos = NULL;

	for (pos = m_lstExtCommands.GetHeadPosition (); pos != NULL;)
	{
		CalculatorCommands cmd = (CalculatorCommands) m_lstExtCommands.GetNext (pos);
		InsertButton (new CCalculatorButton (CommandToName(cmd), cmd, 0));
	}

	UINT uiCommand = idCommandUser;

	for (pos = m_lstAdditionalCommands.GetHeadPosition (); pos != NULL; uiCommand++)
	{
		CString strCommand = m_lstAdditionalCommands.GetNext (pos);

		int nKey = 0;

		int nAmpIndex = strCommand.Find (_T("&"));
		if (nAmpIndex >= 0 && nAmpIndex != strCommand.GetLength () - 1)
		{
			CString strKey;
			strKey += strCommand.GetAt (nAmpIndex + 1);
			strKey.MakeLower ();

			nKey = strKey [0];
		}

		InsertButton (new CCalculatorButton (strCommand, (CalculatorCommands)uiCommand, nKey, 0, FALSE, FALSE, TRUE));
	}

	if (m_lstAdditionalCommands.IsEmpty() && m_lstExtCommands.IsEmpty())
	{
		m_nRows = 5;
		m_nColumns = 5;
	}
	else
	{
		const int nCount = GetCount ();

		m_nColumns = 5;
		m_nRows = (int) (.5 + (double) nCount / m_nColumns);
	}
}

class CBCGPCalculatorCmdUI : public CCmdUI
{
public:
	CBCGPCalculatorCmdUI();

public: // re-implementations only
	virtual void Enable(BOOL bOn);
	BOOL m_bEnabled;
};

CBCGPCalculatorCmdUI::CBCGPCalculatorCmdUI()
{
	m_bEnabled = TRUE;  // assume it is enabled
}
//*************************************************************************************
void CBCGPCalculatorCmdUI::Enable(BOOL bOn)
{
	m_bEnabled = bOn;
	m_bEnableChanged = TRUE;
}
//*************************************************************************************
BOOL CBCGPCalculator::OnSendCommand (const CBCGPToolbarButton* pButton)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pButton);

	CCalculatorButton* pCalcButton = DYNAMIC_DOWNCAST (CCalculatorButton, pButton);
	if (pCalcButton == NULL)
	{
		return FALSE;
	}

	if (m_bIsError && pCalcButton->m_uiCmd != idCommandClear &&
		pCalcButton->m_uiCmd != idCommandReset)
	{
		return TRUE;
	}

	if (pCalcButton->m_uiCmd >= idCommandUser)
	{
		CalcResult ();
		UpdateBuffer ();
		UpdateDisplay ();

		UINT uiCmd = (UINT) pCalcButton->m_uiCmd;

		if (m_pParentEdit != NULL)
		{
			m_pParentEdit->OnCalculatorUserCommand (this, uiCmd);
		}
#ifndef BCGP_EXCLUDE_RIBBON
#ifndef _BCGSUITE_
		else if (m_pParentRibbonEdit != NULL)
		{
			m_pParentRibbonEdit->OnCalculatorUserCommand (this, uiCmd);
		}
#endif
#endif
		else
		{
			OnUserCommand (uiCmd);
		}

		m_bIsClearBuffer = TRUE;

		UpdateBuffer ();
		UpdateDisplay ();
		return TRUE;
	}

	if (pCalcButton->m_uiCmd != idCommandResult)
	{
		m_bSeqResult = FALSE;
	}

	if (pCalcButton->m_nDigit >= 0)
	{
		PushDigit (pCalcButton->m_nDigit);
		return TRUE;
	}

	switch (pCalcButton->m_uiCmd)
	{
	case idCommandBackspace:
		if (!m_bIsError && !m_bIsClearBuffer)
		{
			if (m_strBuffer.GetLength () > 1)
			{
				m_strBuffer = m_strBuffer.Left (m_strBuffer.GetLength () - 1);
			}
			else
			{
				m_strBuffer = _T("0");
			}

#if _MSC_VER < 1400
			_stscanf (m_strBuffer, _T("%lf"), &m_dblValue);
#else
			_stscanf_s (m_strBuffer, _T("%lf"), &m_dblValue);
#endif
			UpdateDisplay ();
		}

		return TRUE;

	case idCommandReset:
		m_dblSecondValue = 0.;
		m_uiLastCmd = 0;

	case idCommandClear:
		m_dblValue = 0.;
		m_strBuffer = _T("0");
		m_bIsError = FALSE;
		m_bIsClearBuffer = TRUE;
		m_bSeqResult = FALSE;
		break;

	case idCommandMemClear:
		m_dblMemValue = 0.;
		UpdateDisplay ();
		return TRUE;

	case idCommandMemRead:
		m_dblValue = m_dblMemValue;
		break;

	case idCommandMemAdd:
		CalcResult ();
		m_dblMemValue += m_dblValue;
		m_bIsClearBuffer = TRUE;
		break;

	case idCommandAdd:
	case idCommandSub:
	case idCommandMult:
	case idCommandDiv:
		if (!m_bIsClearBuffer)
		{
			CalcResult ();
			UpdateBuffer ();
			UpdateDisplay ();
		}

		m_uiLastCmd = pCalcButton->m_uiCmd;
		m_dblSecondValue = m_dblValue;
		m_bIsClearBuffer = TRUE;
		return TRUE;

	case idCommandSign:
		m_dblValue = -m_dblValue;
		m_bIsClearBuffer = TRUE;
		break;

	case idCommandRev:
		if (m_dblValue == 0.)
		{
			m_bIsError = TRUE;
		}
		else
		{
			m_dblValue = 1. / m_dblValue;
			m_bIsClearBuffer = TRUE;
		}
		break;

	case idCommandDot:
		if (m_bIsClearBuffer)
		{
			m_dblValue = 0.;
			m_strBuffer = _T("0.");
			m_bIsClearBuffer = FALSE;
		}

		if (m_strBuffer.Find (_T('.')) == -1)
		{
			m_strBuffer += _T('.');
		}

		UpdateDisplay ();
		return TRUE;

	case idCommandSqrt:
		if (m_dblValue < 0.)
		{
			m_bIsError = TRUE;
		}
		else
		{
			m_dblValue = sqrt (m_dblValue);
			m_bIsClearBuffer = TRUE;
		}
		break;

	case idCommandPercent:
		m_dblValue = m_dblValue * m_dblSecondValue / 100.;
		UpdateBuffer ();
		UpdateDisplay ();

		m_bIsClearBuffer = TRUE;
		break;

	case idCommandAdvSin:
		m_dblValue = sin (m_dblValue);
		m_bIsClearBuffer = TRUE;
		break;

	case idCommandAdvCos:
		m_dblValue = cos (m_dblValue);
		m_bIsClearBuffer = TRUE;
		break;

	case idCommandAdvTan:
		m_dblValue = tan (m_dblValue);
		m_bIsClearBuffer = TRUE;
		break;

	case idCommandAdvX2:
		m_dblValue = pow (m_dblValue, 2);
		m_bIsClearBuffer = TRUE;
		break;

	case idCommandAdvX3:
		m_dblValue = pow (m_dblValue, 3);
		m_bIsClearBuffer = TRUE;
		break;

	case idCommandAdvExp:
		m_dblValue = exp (m_dblValue);
		m_bIsClearBuffer = TRUE;
		break;

	case idCommandAdv10x:
		m_dblValue = pow (10.0, m_dblValue);
		m_bIsClearBuffer = TRUE;
		break;

	case idCommandAdvLn:
		if (m_dblValue == 0.)
		{
			m_bIsError = TRUE;
		}
		else
		{
			m_dblValue = log (m_dblValue);
			m_bIsClearBuffer = TRUE;
		}
		break;

	case idCommandAdvLog:
		if (m_dblValue == 0.)
		{
			m_bIsError = TRUE;
		}
		else
		{
			m_dblValue = log10 (m_dblValue);
			m_bIsClearBuffer = TRUE;
		}
		break;

	case idCommandAdvPi:
		m_dblValue = 3.1415926535897932;
		m_bIsClearBuffer = TRUE;
		break;

	case idCommandResult:
		{
			double dblValueSaved = m_dblValue;

			CalcResult ();

			if (!m_bIsClearBuffer)
			{
				m_dblSecondValue = dblValueSaved;
			}

			m_bIsClearBuffer = TRUE;
		}

		m_bSeqResult = TRUE;

		if (m_pParentEdit != NULL && !m_bIsError)
		{
			CString strPrev;
			m_pParentEdit->GetWindowText (strPrev);

			UpdateBuffer ();
			if (strPrev != m_strBuffer)
			{
				m_pParentEdit->SetWindowText (m_strBuffer);
				m_pParentEdit->OnAfterUpdate ();
			}

			GetParent ()->SendMessage (WM_CLOSE);
			return TRUE;
		}

#ifndef BCGP_EXCLUDE_RIBBON
#ifndef _BCGSUITE_
		if (m_pParentRibbonEdit != NULL && !m_bIsError)
		{
			CString strPrev = m_pParentRibbonEdit->GetEditText ();

			UpdateBuffer ();
			if (strPrev != m_strBuffer)
			{
				m_pParentRibbonEdit->SetEditText (m_strBuffer);
			}

			GetParent ()->SendMessage (WM_CLOSE);
			return TRUE;
		}
#endif
#endif
		break;
	}

	UpdateBuffer ();
	UpdateDisplay ();

	return TRUE;
}
//*************************************************************************************
BOOL CBCGPCalculator::Create(
			CWnd*		pParentWnd,
			DWORD		dwStyle,
			UINT		nID)
{
	m_rectDisplay.SetRectEmpty ();

	return CBCGPPopupMenuBar::Create (pParentWnd, dwStyle, nID);
}
//************************************************************************************
BOOL CBCGPCalculator::CreateControl (
				CWnd*			pParentWnd,
				const CRect&	rect,
				UINT			nID)
{
	ASSERT_VALID (pParentWnd);

	EnableLargeIcons (FALSE);

	if (!Create (pParentWnd, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP, nID))
	{
		return FALSE;
	}

	SetBarStyle (
		GetBarStyle () & 
			~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	CRect rectWnd = rect;
	MoveWindow (rectWnd);

	SetWindowPos (&wndTop, -1, -1, -1, -1,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	SetOwner (pParentWnd);
	SetCommandID (nID);

	// All commands will be routed via this control, not via the parent frame:
	SetRouteCommandsViaFrame (FALSE);

#ifndef _BCGSUITE_
	SetControlVisualMode (pParentWnd);
#endif
	return TRUE;
}
//*************************************************************************************
void CBCGPCalculator::Serialize (CArchive& ar)
{
	CBCGPPopupMenuBar::Serialize (ar);

	if (ar.IsLoading ())
	{
		Rebuild ();
		AdjustLocations ();
	}
	else
	{
	}
}
//*************************************************************************************
void CBCGPCalculator::ShowCommandMessageString (UINT /*uiCmdId*/)
{
	GetOwner()->SendMessage (WM_SETMESSAGESTRING,
		m_nCommandID == (UINT) -1 ? AFX_IDS_IDLEMESSAGE : (WPARAM) m_nCommandID);
}
//***************************************************************************************
void CBCGPCalculator::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBCGPToolBar::OnMouseMove(nFlags, point);
}
//***************************************************************************************
void CBCGPCalculator::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus ();
	CBCGPToolBar::OnLButtonDown(nFlags, point);
}
//***************************************************************************************
void CBCGPCalculator::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CBCGPToolBar::OnLButtonUp(nFlags, point);
}
//***************************************************************************************
void CBCGPCalculator::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CBCGPToolBar::OnLButtonDblClk(nFlags, point);
}
//***************************************************************************************
BOOL CBCGPCalculator::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_CHAR && OnProcessKey ((int) pMsg->wParam, FALSE))
	{
		return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN && OnProcessKey ((int) pMsg->wParam, TRUE))
	{
		return TRUE;
	}

	if ((m_pParentEdit != NULL || m_pParentRibbonEdit != NULL || m_pWndPropList != NULL) && !m_bInCommand)
	{
		switch (pMsg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			{
				CRect rect;
				GetClientRect (rect);

				CPoint pt (BCG_GET_X_LPARAM (pMsg->lParam), BCG_GET_Y_LPARAM (pMsg->lParam));
				if (!rect.PtInRect (pt))
				{
					GetParent ()->SendMessage (WM_CLOSE);
					return TRUE;
				}
			}
			break;

		case WM_SYSKEYDOWN:
		case WM_CONTEXTMENU:
			GetParent ()->SendMessage (WM_CLOSE);
			return TRUE;
		}
	}

	return CBCGPPopupMenuBar::PreTranslateMessage(pMsg);
}
//*************************************************************************************
void CBCGPCalculator::OnDestroy() 
{
	if (m_pParentEdit != NULL)
	{
		m_pParentEdit->m_pCalcPopup = NULL;
		m_pParentEdit->SetFocus ();
	}
#ifndef BCGP_EXCLUDE_PROP_LIST
	else if (m_pWndPropList != NULL)
	{
		m_pWndPropList->CloseColorPopup ();
		m_pWndPropList->SetFocus ();
	}
#endif
#ifndef BCGP_EXCLUDE_RIBBON
#ifndef _BCGSUITE_
	else if (m_pParentRibbonEdit != NULL)
	{
		m_pParentRibbonEdit->m_pCalcPopup = NULL;
		m_pParentRibbonEdit->m_pWndEdit->SetFocus();
	}
#endif
#endif
	CBCGPPopupMenuBar::OnDestroy();
}
//****************************************************************************************
LRESULT CBCGPCalculator::OnMouseLeave(WPARAM wp,LPARAM lp)
{
#if _MSC_VER >= 1700 && defined(_BCGSUITE_)
	UNREFERENCED_PARAMETER(wp);
	UNREFERENCED_PARAMETER(lp);
#endif

	if (m_pParentEdit == NULL && m_pWndPropList == NULL && m_pParentRibbonEdit == NULL)
	{
#if _MSC_VER < 1700 || !defined(_BCGSUITE_)
		return CBCGPToolBar::OnMouseLeave (wp, lp);
#else
		CBCGPToolBar::OnMouseLeave();
		return 0;
#endif
	}

	if (m_hookMouseHelp != NULL || 
		(m_bMenuMode && !IsCustomizeMode () && GetDroppedDownMenu () != NULL))
	{
		return 0;
	}

	m_bTracked = FALSE;
	m_ptLastMouse = CPoint (-1, -1);

	if (m_iHighlighted >= 0)
	{
		int iButton = m_iHighlighted;
		m_iHighlighted = -1;

		OnChangeHot (m_iHighlighted);

		InvalidateButton (iButton);
		UpdateWindow(); // immediate feedback

		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}

	return 0;
}
//*********************************************************************************
void CBCGPCalculator::PushDigit (short nDigit)
{
	ASSERT_VALID (this);
	ASSERT (nDigit >= 0);
	ASSERT (nDigit <= 9);

	if (m_bIsError)
	{
		return;
	}

	if (m_bIsClearBuffer)
	{
		m_dblValue = 0.;
		m_strBuffer = _T("0");
		m_bIsClearBuffer = FALSE;
	}

	CString strDigit;
	strDigit.Format (_T("%d"), nDigit);

	if (m_strBuffer == _T("0"))
	{
		m_strBuffer.Empty ();
	}

	m_strBuffer += strDigit;
#if _MSC_VER < 1400
	_stscanf (m_strBuffer, _T("%lf"), &m_dblValue);
#else
	_stscanf_s (m_strBuffer, _T("%lf"), &m_dblValue);
#endif

	UpdateDisplay ();
}
//********************************************************************************
void CBCGPCalculator::UpdateDisplay ()
{
	if (m_Buttons.IsEmpty())
	{
		return;
	}

	CCalculatorDisplay* pDisplay = (CCalculatorDisplay*) m_Buttons.GetHead ();
	ASSERT_VALID (pDisplay);

	pDisplay->m_strText = m_bIsError ? _T("Error") : m_strBuffer;
	pDisplay->m_bMem = m_dblMemValue != 0.;

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow (m_rectDisplay);
	}
}
//*********************************************************************************
void CBCGPCalculator::UpdateBuffer ()
{
	if (fabs (m_dblValue) > pow (10., 22))
	{
		m_strBuffer.Format (_T("%le"), m_dblValue);
	}
	else
	{
		m_strBuffer.Format(m_strDisplayFormat, m_dblValue);

		while (m_strBuffer [m_strBuffer.GetLength () - 1] == _T('0'))
		{
			m_strBuffer = m_strBuffer.Left (m_strBuffer.GetLength () - 1);
		}

		if (m_strBuffer [m_strBuffer.GetLength () - 1] == _T('.'))
		{
			m_strBuffer = m_strBuffer.Left (m_strBuffer.GetLength () - 1);
		}
	}

	if (m_strBuffer.Find (_T("#INF")) >= 0 ||
		m_strBuffer.Find (_T("#IND")) >= 0 ||
		m_strBuffer.Find (_T("#NAN")) >= 0)
	{
		m_bIsError = TRUE;
		UpdateDisplay ();
	}
}
//********************************************************************************
void CBCGPCalculator::CalcResult ()
{
	switch (m_uiLastCmd)
	{
	case idCommandAdd:
		m_dblValue += m_dblSecondValue;
		break;

	case idCommandSub:
		m_dblValue = m_bSeqResult ? m_dblValue - m_dblSecondValue : m_dblSecondValue - m_dblValue;
		break;

	case idCommandMult:
		m_dblValue *= m_dblSecondValue;
		break;

	case idCommandDiv:
		if (m_bSeqResult)
		{
			if (m_dblSecondValue == 0.)
			{
				m_bIsError = TRUE;
			}
			else
			{
				m_dblValue = m_dblValue / m_dblSecondValue;
			}
		}
		else
		{
			if (m_dblValue == 0.)
			{
				m_bIsError = TRUE;
			}
			else
			{
				m_dblValue = m_dblSecondValue / m_dblValue;
			}
		}
	}
}
//********************************************************************************
BOOL CBCGPCalculator::OnProcessKey (int nKey, BOOL bIsVirtKey)
{
	if ((m_pParentEdit != NULL || m_pParentRibbonEdit != NULL || m_pWndPropList != NULL) && nKey == VK_ESCAPE)
	{
		GetParent ()->SendMessage (WM_CLOSE);
		return TRUE;
	}

	for (POSITION pos = m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CCalculatorButton* pCalculatorButton = DYNAMIC_DOWNCAST (CCalculatorButton, m_Buttons.GetNext (pos));
		if (pCalculatorButton != NULL &&
			pCalculatorButton->m_bIsVitKey == bIsVirtKey &&
			(pCalculatorButton->m_nKbdAccel == nKey || pCalculatorButton->m_nKbdAccel2 == nKey))
		{
			if (pCalculatorButton->m_bIsCtrl &&
				(::GetAsyncKeyState (VK_CONTROL) & 0x8000) == 0)
			{
				continue;
			}

			OnSendCommand (pCalculatorButton);
			return TRUE;
		}
	}

	return FALSE;
}
//*********************************************************************************
void CBCGPCalculator::SetValue (double dblValue)
{
	ASSERT_VALID (this);

	m_dblValue = dblValue;

	UpdateBuffer ();
	UpdateDisplay ();
}
//*********************************************************************************
void CBCGPCalculator::SetAdditionalCommands (const CStringList& lstCommands)
{
	m_lstAdditionalCommands.RemoveAll ();
	m_lstAdditionalCommands.AddTail ((CStringList*) &lstCommands);

	if (GetSafeHwnd () != NULL)
	{
		Rebuild ();
		AdjustLocations ();

		RedrawWindow ();
	}
}
//*********************************************************************************
void CBCGPCalculator::RemoveAdditionalCommands ()
{
	m_lstAdditionalCommands.RemoveAll ();

	Rebuild ();
	AdjustLocations ();

	if (GetSafeHwnd () != NULL)
	{
		Rebuild ();
		AdjustLocations ();

		RedrawWindow ();
	}
}
//*********************************************************************************
void CBCGPCalculator::SetExtendedCommands (const CList<UINT, UINT>& lstCommands)
{
	m_lstExtCommands.RemoveAll ();
	m_lstExtCommands.AddTail ((CList<UINT, UINT>*) &lstCommands);

	if (GetSafeHwnd () != NULL)
	{
		Rebuild ();
		AdjustLocations ();

		RedrawWindow ();
	}
}
//*********************************************************************************
void CBCGPCalculator::RemoveExtendedCommands ()
{
	m_lstExtCommands.RemoveAll ();

	Rebuild ();
	AdjustLocations ();

	if (GetSafeHwnd () != NULL)
	{
		Rebuild ();
		AdjustLocations ();

		RedrawWindow ();
	}
}
//*********************************************************************************
void CBCGPCalculator::OnUserCommand (UINT /*uiCmd*/)
{
	ASSERT (FALSE);	// Must be implemented in derived class
}
//*********************************************************************************
BOOL CBCGPCalculator::OnDrawButton (CDC* pDC, CRect rect,
		CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state, 
		int cmd)
{
	return CBCGPVisualManager::GetInstance ()->OnDrawCalculatorButton (pDC, rect, 
		pButton, state, cmd, this);
}
//***********************************************************************************
BOOL CBCGPCalculator::OnDrawDisplay (CDC* pDC, CRect rect, 
									 const CString& strText, BOOL bMem)
{
	return CBCGPVisualManager::GetInstance ()->OnDrawCalculatorDisplay (
		pDC, rect, strText, bMem, this);
}
//***********************************************************************************
void CBCGPCalculator::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPPopupMenuBar::OnSize(nType, cx, cy);
	InvalidateRect (NULL);
}
//***********************************************************************************
LRESULT CBCGPCalculator::OnChangeVisualManager (WPARAM, LPARAM)
{
	if (m_CalcImages.GetCount() == 0)
	{
		return 0;
	}

	if (m_CalcImagesCommands.GetCount() > 0)
	{
		m_CalcImagesCommands.Clear();
		m_CalcImagesUser.Clear();
		m_CalcImagesDigits.Clear();
	}

	m_CalcImages.CopyTo (m_CalcImagesDigits);
#ifndef _BCGSUITE_
	m_CalcImagesDigits.AddaptColors(RGB (0, 0, 0), 
		CBCGPVisualManager::GetInstance ()->GetCalculatorButtonTextColor(FALSE, idCommandNone), FALSE);
#endif

	m_CalcImages.CopyTo (m_CalcImagesCommands);
#ifndef _BCGSUITE_
	m_CalcImagesCommands.AddaptColors(RGB (0, 0, 0), 
		CBCGPVisualManager::GetInstance ()->GetCalculatorButtonTextColor(FALSE, idCommandClear), FALSE);
#endif

	m_CalcImages.CopyTo (m_CalcImagesUser);
#ifndef _BCGSUITE_
	m_CalcImagesUser.AddaptColors(RGB (0, 0, 0), 
		CBCGPVisualManager::GetInstance ()->GetCalculatorButtonTextColor(TRUE, idCommandUser), FALSE);
#endif

	RedrawWindow();
	return 0;
}
//*************************************************************************************
void CBCGPCalculator::DoPaint (CDC* pDC)
{
	CBCGPPopupMenuBar::DoPaint (pDC);

	if (m_bHasBorder)
	{
		CRect rect;
		GetClientRect(rect);

#ifndef _BCGSUITE_
		if (m_bIsDlgControl)
		{
			CBCGPVisualManager::GetInstance ()->OnDrawControlBorderNoTheme(pDC, rect, this, FALSE);
		}
		else
#endif
		{
			CBCGPVisualManager::GetInstance ()->OnDrawControlBorder(pDC, rect, this, FALSE);
		}
	}
}
//*************************************************************************************
void CBCGPCalculator::EnableBorder(BOOL bEnable/* = TRUE*/)
{
	m_bHasBorder = bEnable;

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}
//*************************************************************************************
void CBCGPCalculator::SetDisplayFormat(const CString& strDisplayFormat)
{
	m_strDisplayFormat = strDisplayFormat;

	UpdateBuffer();
	UpdateDisplay ();

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

#ifdef _BCGSUITE_
LRESULT CBCGPCalculator::OnSetControlVMMode(WPARAM wp, LPARAM)
{
	if ((BOOL)wp)
	{
		m_bIsDlgControl = FALSE;
	}
	return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPCalculatorPopup

IMPLEMENT_DYNAMIC(CBCGPCalculatorPopup, CBCGPPopupMenu)

BEGIN_MESSAGE_MAP(CBCGPCalculatorPopup, CBCGPPopupMenu)
	//{{AFX_MSG_MAP(CBCGPCalculatorPopup)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CBCGPCalculatorPopup::~CBCGPCalculatorPopup()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPCalculatorPopup message handlers

int CBCGPCalculatorPopup::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	if (pWndParent->GetSafeHwnd() != NULL && (pWndParent->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}

	DWORD toolbarStyle = dwDefaultToolbarStyle;
	if (m_AnimationType != NO_ANIMATION && !CBCGPToolBar::IsCustomizeMode ())
	{
		toolbarStyle &= ~WS_VISIBLE;
	}

	if (!m_wndCalculator.Create (this, toolbarStyle | CBRS_TOOLTIPS | CBRS_FLYBY, 1))
	{
		TRACE(_T ("Can't create popup menu bar\n"));
		return -1;
	}

	m_wndCalculator.SetOwner (pWndParent);
	m_wndCalculator.SetBarStyle(m_wndCalculator.GetBarStyle() | CBRS_TOOLTIPS);

	ActivatePopupMenu (BCGCBProGetTopLevelFrame (pWndParent), this);
	RecalcLayout ();
	return 0;
}
//**************************************************************************************************************
CBCGPControlBar* CBCGPCalculatorPopup::CreateTearOffBar (CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName)
{
	ASSERT_VALID (pWndMain);
	ASSERT (lpszName != NULL);
	ASSERT (uiID != 0);

	CBCGPCalculator* pCalcBar = new CBCGPCalculator;

	if (!pCalcBar->Create (pWndMain,
		dwDefaultToolbarStyle,
		uiID))
	{
		TRACE0 ("Failed to create a new toolbar!\n");
		delete pCalcBar;
		return NULL;
	}

	pCalcBar->SetCommandID (m_wndCalculator.m_nID);
	pCalcBar->SetValue (m_wndCalculator.m_dblValue);
	pCalcBar->SetExtendedCommands (m_wndCalculator.m_lstExtCommands);
	pCalcBar->SetAdditionalCommands (m_wndCalculator.m_lstAdditionalCommands);

	pCalcBar->m_dblSecondValue = m_wndCalculator.m_dblSecondValue;
	pCalcBar->m_dblMemValue = m_wndCalculator.m_dblMemValue;
	pCalcBar->m_strBuffer = m_wndCalculator.m_strBuffer;

	pCalcBar->SetWindowText (lpszName);
	pCalcBar->SetBarStyle (pCalcBar->GetBarStyle () |
		CBRS_TOOLTIPS | CBRS_FLYBY);
	pCalcBar->EnableDocking (0);

	return pCalcBar;
}

#if defined _BCGSUITE_

void CBCGPCalculator::OnFillBackground (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CRect rectClient;
	GetClientRect (rectClient);

	CBCGPVisualManager::GetInstance()->OnFillBarBackground(pDC, this, rectClient, rectClient);
}

#endif
