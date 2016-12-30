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

// BCGPEditCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPLocalResource.h"
#include "BCGPDrawManager.h"
#include "BCGPEditCtrl.h"

#ifndef BCGP_EXCLUDE_EDIT_CTRL

#include "BCGPIntelliSenseWnd.h"
#include "BCGGlobals.h"

#ifndef _BCGPEDIT_STANDALONE
	#include "BCGPVisualManager.h"
#ifndef _BCGSUITE_
	#include "BCGPPopupMenu.h"
	#include "BCGPTooltipManager.h"
#endif
#else
	#include "resource.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const CString g_strEOL = _T ("\n");
static const TCHAR g_chEOL = _T ('\n');
static const CString g_strEOLExport = _T ("\r\n");

static CString strTipText;

static CLIPFORMAT g_cFormat = 0;

IMPLEMENT_DYNAMIC(CBCGPEditCtrl,CWnd)
IMPLEMENT_SERIAL(CBCGPEditMarker,CObject, 1)
IMPLEMENT_SERIAL(CBCGPLineColorMarker,CBCGPEditMarker, 1)
IMPLEMENT_SERIAL(CBCGPHiliteMarker,CBCGPEditMarker, 1)
IMPLEMENT_SERIAL(CBCGPOutlineBaseNode,CObject,1)
IMPLEMENT_SERIAL(CBCGPOutlineNode,CBCGPOutlineBaseNode, 1)

BOOL CBCGPOutlineBaseNode::m_bCollapsedDefault = FALSE;

#define ID_TEXT_SCROLL				1
#define ID_SCROLL_VERT				2
#define ID_SCROLL_HORZ				3
#define _MAX_COLOR_BLOCK_STR_LEN	4
#define	_MAX_RTF_LEN				120000

BOOL CBCGPEditCtrl::m_bOvrMode = FALSE;

UINT BCGM_ON_EDITCHANGE	= ::RegisterWindowMessage (_T("BCGM_ON_EDITCHANGE"));

#ifdef _UNICODE
	#define _TCF_TEXT	CF_UNICODETEXT
#else
	#define _TCF_TEXT	CF_TEXT
#endif

COLORREF BCGP_EDIT_COLOR_BLOCK::MakeLightColor(COLORREF clrIn)
{
	if (clrIn == (COLORREF)-1)
	{
		return (COLORREF)-1;
	}

	if (clrIn == 0)
	{
		int nVal = 127;
		return RGB(nVal, nVal, nVal);
	}

	double H, L, S;
	CBCGPDrawManager::RGBtoHSL(clrIn, &H, &S, &L);

	double dblRatio = 1.5;

	return CBCGPDrawManager::HLStoRGB_ONE (
		H,
		min (1., L * dblRatio),
		min (1., S * dblRatio));
}

void BCGP_EDIT_COLOR_BLOCK::SetForegroundColor(COLORREF clr)
{
	m_clrForeground = clr;
	m_clrForegroundLight = MakeLightColor(m_clrForeground);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditCtrl

CBCGPEditCtrl::CBCGPEditCtrl() :
	m_clrBack ((COLORREF) -1),
	m_clrText ((COLORREF) -1),
	m_bIsDarkBackground (FALSE),
	m_nLineHeight (0),
	m_nMaxCharWidth (0),
	m_hFont (NULL),
	m_nCurrOffset (0),
	m_bDisableSetCaret (FALSE),
	m_nLeftMarginWidth (20),
	m_nLineNumbersMarginWidth (40),
	m_nOutlineMarginWidth (20),
	m_ptCaret (CPoint (m_nLeftMarginWidth, 0)),
	m_nTabSize (4),
	m_iStartSel (-1),
	m_iEndSel (-1),
	m_bEnableWholeTextCopy (FALSE),
	m_bEnableCurrentLineCopy (FALSE),
	m_nScrollOffsetHorz (0),
	m_nScrollOffsetVert (0),
	m_nScrollHiddenVert (0),
	m_nTopVisibleOffset (0),
	m_nBottomVisibleOffset (0),
	m_szTotalScroll (0, 0),
	m_nScrollMouseWheelSpeed (1),
	m_bScrollVertEmptyPage (TRUE),
	m_bScrollBarHorz (TRUE),
	m_nCurRow (0),
	m_nCurColumn (0),
	m_nLastSelOffset (-1),
	m_nLastMaxColumn (0),
	m_posUndoBuffer (NULL),
	m_nUndoBufferSize (300),
	m_bEOL (FALSE),
	m_bBOL (FALSE),
	m_bIntelliSenseSupport(FALSE),
	m_bIsLastActionUndo (TRUE),
	m_bUndoCharMode (FALSE),
	m_bDragTextMode (FALSE),
	m_nSavedOffset (0), 
	m_nDropOffset (-1),
	m_bDefaultIndent (TRUE),
	m_pIntelliSenseWnd (NULL),
	m_pIntelliSenseImgList (NULL),
	m_pIntelliSenseLBFont (NULL),
	m_bIntelliSenseMode (FALSE),
	m_bKeepTabs (TRUE), 
	m_nTabLogicalSize (0),
	m_bBlockSelectionMode (FALSE),
	m_bAltPressedForBlockSel (FALSE),
	m_nIndentSize (4),
	m_bReadOnly (FALSE),
	m_nLineVertSpacing (1),
	m_nMaxScrollWidth (2048),
	m_nTotalLines (0),
	m_nHiddenLines (0),
	m_bEnableToolTips (FALSE),
	m_bReplaceTabsAndEOLOnCopy (TRUE),
	m_bIntelliSenseUpdate(FALSE),
	m_bIsModified (FALSE),
	m_nScrollTimeOut (100),
	m_nScrollTimer (-1),
	m_bToolTipCleared (FALSE),
	m_nMaxToolTipWidth (-1),
	m_posDocSavedUndoPos (NULL),
	m_bSavedUndo (FALSE),
	m_dwLastUndoReason (g_dwUATUndefined),
	m_bSaveFileInUnicode (FALSE),
	m_bCopyRTFToClipboard (FALSE)
{
	m_strWordDelimeters = _T(" \t\n,./?<>;:\"'{[}]~`%^&*()-+=!");
	m_strSpecialDelimiters = _T (";.{}\n");
	m_strNonSelectableChars = _T (" \n\t");
	m_strIndentChars = _T (" \t");
	m_strIntelliSenseChars = _T (".>:");
	m_strContinueStringSymbols = _T ("\\");
	m_rectView.SetRectEmpty ();
	m_rectText.SetRectEmpty ();
	m_ptStartBlockSel = m_ptEndBlockSel = CPoint (-1, -1);

	m_nScrollRightOffset = 15; // percents
	m_nScrollLeftOffset = 2;   // symbols

	m_nPrevKey = m_nLastKey = -1;

	m_hPrinterFont = NULL;
	m_hMirrorFont = NULL;

	m_nDlgCode = DLGC_WANTALLKEYS | DLGC_WANTMESSAGE;

	m_pSymbolsImgList = NULL;
	m_bEnableSymSupport = FALSE;

	m_bEnableLineNumbersMargin = FALSE;
	m_nLineNumbersMarginAutoWidth = m_nLineNumbersMarginWidth;

	m_bEnableOutlineMargin = FALSE;
	m_bEnableOutlining = FALSE;
	m_bAutoOutlining = FALSE;
	m_nMaxHintCharsNum = 512;
	m_pOutlineNodeCurr = NULL;
	m_pOutlineParser = NULL;
	if (m_bEnableLineNumbersMargin)
	{
		m_ptCaret.x += m_nLineNumbersMarginAutoWidth;
	}
	if (m_bEnableOutlineMargin)
	{
		m_ptCaret.x += m_nOutlineMarginWidth;
	}

	m_bEnableHyperlinkSupport = FALSE;
	m_clrHyperlink = (COLORREF) -1;
	m_clrHyperlinkLight = (COLORREF)-1;
	m_bColorHyperlink = TRUE;
	m_lstURLPrefixes.AddTail (_T("http://"));
	m_lstURLPrefixes.AddTail (_T("ftp://"));
	m_lstURLPrefixes.AddTail (_T("mailto:"));

	m_nCurrHyperlink = -1;
	m_nCurrHyperlinkHot = -1;

	m_nColorBlockStrLenMax = _MAX_COLOR_BLOCK_STR_LEN;

	m_bIsBorder = FALSE;

	m_bGradientMarkers = FALSE;

	m_pToolTip = NULL;

	m_clrBackSelActive = (COLORREF)-1;
	m_clrBackSelInActive = (COLORREF)-1;
	m_clrTextSelActive = (COLORREF)-1;
	m_clrTextSelInActive = (COLORREF)-1;

	m_clrBackOutline = (COLORREF)-1;
	m_clrLineOutline = (COLORREF)-1;
	m_clrBackLineNumber = (COLORREF)-1;
	m_clrTextLineNumber = (COLORREF)-1;
	m_clrBackSidebar = (COLORREF)-1;

#ifndef _BCGPEDIT_STANDALONE
	m_ScrollBarStyle = CBCGPScrollBar::BCGP_SBSTYLE_DEFAULT;
#endif
}

CBCGPEditCtrl::~CBCGPEditCtrl ()
{
	CleanUpFonts ();
	CleanUpMarkers ();
	EmptyUndoList ();
	delete m_pOutlineParser;
}

void CBCGPEditCtrl::EmptyUndoList ()
{
	while (!m_lstUndoActions.IsEmpty ())
	{
		BCGP_EDIT_UNDO_ACTION* pUndoAction = m_lstUndoActions.GetHead ();
		OnRemoveUndoAction (pUndoAction);
		delete m_lstUndoActions.RemoveHead ();
	}

	m_posUndoBuffer = NULL;
	m_bEOL = FALSE;
	m_bBOL = FALSE;
	m_bIsLastActionUndo = TRUE;
}

BEGIN_MESSAGE_MAP(CBCGPEditCtrl, CWnd)
	//{{AFX_MSG_MAP(CBCGPEditCtrl)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_RBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_GETDLGCODE()
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnTTNeedTipText)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_GETTEXT, OnGetText)
	ON_MESSAGE(WM_STYLECHANGING, OnStyleChanging)
	ON_MESSAGE(WM_GETTEXTLENGTH, OnGetTextLength)
#ifndef _BCGPEDIT_STANDALONE
	ON_REGISTERED_MESSAGE(BCGM_UPDATETOOLTIPS, OnBCGUpdateToolTips)
#endif
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPEditCtrl message handlers

int CBCGPEditCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_DropTarget.Register (this);

#ifndef _BCGPEDIT_STANDALONE
	CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
		BCGP_TOOLTIP_TYPE_EDIT);
#else
	m_pToolTip = new CToolTipCtrl;
	m_pToolTip->Create (this, TTS_ALWAYSTIP | TTS_NOPREFIX);
#endif

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		CRect rectClient;
		GetClientRect (&rectClient);

		if (m_nMaxToolTipWidth != -1)
		{
			m_pToolTip->SetMaxTipWidth (m_nMaxToolTipWidth);
		}
		
		m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectClient, GetDlgCtrlID ());
	}

	CRect rectDummy;
	rectDummy.SetRectEmpty ();

	m_wndScrollVert.Create (WS_CHILD | WS_VISIBLE | SBS_VERT, rectDummy, this, ID_SCROLL_VERT);
	m_wndScrollHorz.Create (WS_CHILD | WS_VISIBLE | SBS_HORZ, rectDummy, this, ID_SCROLL_HORZ);

#ifndef _BCGPEDIT_STANDALONE
	SetScrollBarsStyle (CBCGPScrollBar::BCGP_SBSTYLE_VISUAL_MANAGER);
#endif

	Initialize ();
	SetCaret (0);
	return 0;
}
//********************************************************************************
void CBCGPEditCtrl::PreSubclassWindow() 
{
	CWnd::PreSubclassWindow();
}
//********************************************************************************
void CBCGPEditCtrl::Initialize ()
{
	//-----------------
	// Initialize font:
	//-----------------
	OnChangeFont ();

	InitColors ();

	g_cFormat = (CLIPFORMAT) RegisterClipboardFormat (_T ("BCGP_TEXT"));

	CString strBuffer;
	ProcessTextBeforeInsert (strBuffer);
	InsertText (strBuffer, -1, FALSE, TRUE);
	m_strBuffer = strBuffer;

	BOOL bIsBorder = (GetStyle () & WS_BORDER);

	ModifyStyle (WS_BORDER, WS_CLIPCHILDREN);

	m_bIsBorder = bIsBorder;

	m_OutlineNodes.SetOwnerEditCtrl (this);

	if (!m_pOutlineParser)
	{
		m_pOutlineParser = CreateOutlineParser ();
	}

	if (m_pOutlineParser != NULL)
	{
		m_pOutlineParser->Init ();
		m_pOutlineParser->m_strDelimiters = m_strWordDelimeters;
	}

#ifndef _BCGPEDIT_STANDALONE
	CBCGPGestureConfig gestureConfig;
	gestureConfig.EnablePan(TRUE, BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA);
	
	bcgpGestureManager.SetGestureConfig(GetSafeHwnd(), gestureConfig);
#endif
}
//********************************************************************************
BOOL CBCGPEditCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}
//*******************************************************************************
void CBCGPEditCtrl::PrepareDrawText(CDC* pDC)
{
	ASSERT_VALID (pDC);

	CString strTabs (_T (' '), m_nTabSize);
	m_nTabLogicalSize = pDC->GetTextExtent (strTabs).cx;

	CString strSlashes (_T ('/'), m_nTabSize);
	int m_nSlashesLogicalSize = pDC->GetTextExtent (strSlashes).cx;

	if (m_nTabLogicalSize < m_nSlashesLogicalSize)
	{
		m_nTabLogicalSize = m_nSlashesLogicalSize;
	}
}
//*******************************************************************************
LRESULT CBCGPEditCtrl::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		OnDraw(pDC, NULL);
	}

	return 0;
}
//********************************************************************************
void CBCGPEditCtrl::OnPaint() 
{
	CPaintDC dcPaint(this); // device context for painting

	CRect rectClip;
	dcPaint.GetClipBox (rectClip);

	CBCGPMemDC memDC (dcPaint, this);
	CDC* pDC = &memDC.GetDC ();

	OnDraw(pDC, &rectClip);
}
//*****************************************************************************
void CBCGPEditCtrl::OnDraw(CDC* pDC, LPCRECT lpRectClip)
{
	ASSERT_VALID(pDC);

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectClip = lpRectClip == NULL ? rectClient : lpRectClip;

	//-------------------------
	// Fill control background:
	//-------------------------
	OnFillBackground (pDC);

	//---------------------
	// Set text attributes:
	//---------------------
	int nOldBkMode = pDC->SetBkMode (TRANSPARENT);
	COLORREF clrOldText = pDC->SetTextColor (GetDefaultTextColor ());

	CFont* pOldFont = SelectFont (pDC);

	PrepareDrawText (pDC);
	int nVertOffset = m_nScrollOffsetVert * m_nLineHeight;

	//---------------
	// Draw side bar:
	//---------------
	if (m_nLeftMarginWidth > 0 && rectClip.left < m_nLeftMarginWidth)
	{
		CRect rectLeft = rectClient;
		rectLeft.right = rectLeft.left + m_nLeftMarginWidth;
		rectLeft.top -= nVertOffset;

		OnDrawSideBar (pDC, rectLeft);
	}
	int nMargin = m_nLeftMarginWidth;

	// ---------------------------
	// Draw line numbers side bar:
	// ---------------------------
	if (m_bEnableLineNumbersMargin)
	{
		if (m_nLineNumbersMarginAutoWidth > 0 && rectClip.left < nMargin + m_nLineNumbersMarginAutoWidth)
		{
			CRect rectLeft = rectClient;
			rectLeft.left = rectLeft.left + nMargin;
			rectLeft.right = rectLeft.left + m_nLineNumbersMarginAutoWidth;
			rectLeft.top -= nVertOffset;

			OnDrawLineNumbersBar (pDC, rectLeft);
		}
		nMargin += m_nLineNumbersMarginAutoWidth;
	}

	//--------------------------
	// Draw outlining side bar :
	//--------------------------
	if (m_bEnableOutlineMargin)
	{
		if (m_nOutlineMarginWidth > 0 && rectClip.left < nMargin + m_nOutlineMarginWidth)
		{
			CRect rectLeft = rectClient;
			rectLeft.left = rectLeft.left + nMargin;
			rectLeft.right = rectLeft.left + m_nOutlineMarginWidth;
			rectLeft.top -= nVertOffset;

			OnDrawOutlineBar (pDC, rectLeft);
		}
		nMargin += m_nOutlineMarginWidth;
	}

	//-----------
	// Draw text:
	//-----------
	if (!m_strBuffer.IsEmpty ())
	{
		rectClip.IntersectRect (rectClip, m_rectText);
		CRgn rgnClip;
		rgnClip.CreateRectRgnIndirect (&rectClip);

		pDC->SelectClipRgn (&rgnClip);
		OnDrawText (pDC);
		pDC->SelectClipRgn (NULL);	
		rectClip = rectClient;
	}
	
	pDC->SelectObject (pOldFont);
	pDC->SetTextColor (clrOldText);
	pDC->SetBkMode (nOldBkMode);
}
//*****************************************************************************
afx_msg LRESULT CBCGPEditCtrl::OnSetFont (WPARAM wParam, LPARAM lParam)
{
	m_hFont = (HFONT) wParam;
	UpdateFonts();
	OnChangeFont ();

	if (OnUpdateLineNumbersMarginWidth ())
	{
		// Should update all margin-related things	
		RecalcLayout ();
		SetCaret (m_nCurrOffset, TRUE, FALSE);
		RedrawWindow ();
	}

	else if ((BOOL)lParam)
	{
		RedrawWindow ();
	}

	return 0;
}
//*****************************************************************************
afx_msg LRESULT CBCGPEditCtrl::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT) m_hFont;
}
//*****************************************************************************
CFont* CBCGPEditCtrl::SelectFont (CDC* pDC, BOOL bUnderline)
{
	CFont* pOldFont = NULL;

	if (pDC->IsPrinting ())
	{
		if (m_hPrinterFont != NULL)
		{
			pOldFont = pDC->SelectObject (CFont::FromHandle(m_hPrinterFont));
		}
	}
	else
	{
		if (bUnderline && m_fontUnderline.GetSafeHandle () != NULL)
		{
			pOldFont = pDC->SelectObject (&m_fontUnderline);
		}
		else
		{
			pOldFont = (m_hFont == NULL) ?
				(CFont*) pDC->SelectStockObject (DEFAULT_GUI_FONT) :
				pDC->SelectObject (CFont::FromHandle (m_hFont));
		}
	}

	ASSERT (pOldFont != NULL);
	return pOldFont;
}
//*****************************************************************************
LRESULT CBCGPEditCtrl::OnSetText (WPARAM, LPARAM lp)
{
	LPCTSTR lpszText = (LPCTSTR) lp;
	
	m_nCurrOffset = 0;
	m_nTotalLines = 0;
	m_nHiddenLines = 0;
	m_iEndSel = -1;
	m_iStartSel = -1;

	//workaround for vertical scroll bar
	m_strBuffer = _T(" ");
	UpdateScrollBars ();
	BOOL bNeedUpdate = OnUpdateLineNumbersMarginWidth ();

	m_strBuffer = _T("");

	m_bDisableSetCaret = TRUE;

	if (lpszText != NULL)
	{
		InsertText (lpszText, -1, FALSE, TRUE, FALSE);
	}
	
	m_posUndoBuffer = NULL;
	
	while (!m_lstUndoActions.IsEmpty ())
	{
		delete m_lstUndoActions.RemoveHead ();
	}

	if (bNeedUpdate)
	{
		RecalcLayout ();
	}

	m_bDisableSetCaret = FALSE;
	SetCaret (0);
	Invalidate ();
	UpdateWindow ();
	return TRUE;
}
//*****************************************************************************
LRESULT CBCGPEditCtrl::OnGetText (WPARAM wp, LPARAM lp)
{
	int nMaxChars = min ((int) wp, m_strBuffer.GetLength () + 1);

	LPTSTR lpszText = (LPTSTR) lp;
	ASSERT (lpszText != NULL);

	::lstrcpyn (lpszText, m_strBuffer, nMaxChars);

	return nMaxChars;
}
//*****************************************************************************
BOOL CBCGPEditCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	if (m_rectText.PtInRect (ptCursor))
	{
		if (IsPointInSelectionRect (ptCursor))
		{
			return CWnd::OnSetCursor(pWnd, nHitTest, message);
		}

		CPoint pt = ptCursor;
		int nMouseCursorOffset = -1;
		if (m_bEnableHyperlinkSupport || m_bReadOnly)
		{
			nMouseCursorOffset = HitTest (pt, TRUE, TRUE);
		}

		if (m_bEnableHyperlinkSupport && GetCapture () != this)
		{
			if (nMouseCursorOffset != -1 && 
				nMouseCursorOffset < m_strBuffer.GetLength ())
			{
				m_nCurrHyperlinkHot = LookupHyperlinkByOffset (nMouseCursorOffset);
			}
			else
			{
				m_nCurrHyperlinkHot = -1;
			}

			// if in alternate mode - highlight hovered hyperlinks
			const BOOL bCtrl = ::GetAsyncKeyState (VK_CONTROL) & 0x8000;
			if (bCtrl && m_nCurrHyperlinkHot != -1)
			{
			#ifdef _BCGPEDIT_STANDALONE
				HCURSOR hcurHand = NULL;

				if (globalData.bIsWindowsNT4 || globalData.bIsWindows9x)
				{
					CBCGPLocalResource locaRes;
					hcurHand = AfxGetApp ()->LoadCursor (IDC_BCGBARRES_HAND);
				}
				else
				{
					hcurHand = ::LoadCursor (NULL, MAKEINTRESOURCE(32649)/*IDC_HAND*/);
				}

				::SetCursor (hcurHand);

			#else
				::SetCursor (globalData.GetHandCursor ());
			#endif
				return TRUE;
			}
		}
			
		if (m_bReadOnly)
		{
			if (abs (pt.x - ptCursor.x) > m_nMaxCharWidth ||
				abs (pt.y - ptCursor.y) > m_nLineHeight)
			{
				return CWnd::OnSetCursor(pWnd, nHitTest, message);
			}
		}

		::SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_IBEAM));
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
//*****************************************************************************
void CBCGPEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	const BOOL bShift = ::GetAsyncKeyState (VK_SHIFT) & 0x8000;
	const BOOL bCtrl = ::GetAsyncKeyState (VK_CONTROL) & 0x8000;

	if (nChar > VK_ESCAPE && nChar < VK_NUMLOCK)
	{
		ToolTipPop ();
	}

	if (m_bEnableHyperlinkSupport && 
		nChar == VK_CONTROL && m_nCurrHyperlinkHot != -1)
	{
	#ifdef _BCGPEDIT_STANDALONE
		HCURSOR hcurHand = NULL;

		if (globalData.bIsWindowsNT4 || globalData.bIsWindows9x)
		{
			CBCGPLocalResource locaRes;
			hcurHand = AfxGetApp ()->LoadCursor (IDC_BCGBARRES_HAND);
		}
		else
		{
			hcurHand = ::LoadCursor (NULL, MAKEINTRESOURCE(32649)/*IDC_HAND*/);
		}

		::SetCursor (hcurHand);
	#else
		::SetCursor (globalData.GetHandCursor ());
	#endif
	}

	if (m_bReadOnly)
	{
		switch (nChar)
		{
/*		case VK_LEFT:
			ScrollUp (SB_HORZ, TRUE);
			break;

		case VK_RIGHT:
			ScrollDown (SB_HORZ, TRUE);
			break;

		case VK_HOME:
			StartOfText ();
			break;

		case VK_END:
			EndOfText ();
			break;

		case VK_UP:
			ScrollUp (SB_VERT, TRUE);
			break;

		case VK_DOWN:
			ScrollDown (SB_VERT, TRUE);
			break;
*/
		case VK_PRIOR:
			PageUp ();
			break;

		case VK_NEXT:
			PageDown ();
			break;

		case VK_RETURN:
			break;

		case VK_TAB:
			bShift ? PageUp () : PageDown ();
			break;
																				   
		case VK_BACK:
			break;
													  
		case VK_DELETE:
			break;

		case VK_ESCAPE:
			break;

		case VK_INSERT:
			break;
		}
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}							 

	m_nPrevKey = m_nLastKey;
	m_nLastKey = nChar;

	if (nChar == VK_ESCAPE && m_iStartSel != -1 ||
		nChar == VK_DELETE || nChar == VK_UP || nChar == VK_DOWN ||
		nChar == VK_PRIOR || nChar == VK_NEXT)
    {																																																																																																																																																																									
		CPoint ptCurrOffset;
		OffsetToPoint (m_nCurrOffset, ptCurrOffset);

		if (!m_rectText.PtInRect (ptCurrOffset))
		{
			SetCaret (m_nCurrOffset, TRUE);
		}
    }
		
	switch (nChar)
	{
	case VK_LEFT:
		if (bCtrl)
		{
			if (bShift)
			{
				MakeSelection (ST_PREV_WORD);
			}
			else
			{
				RemoveSelection ();
				PrevWord ();
			}
		}
		else if (bShift)
		{
			MakeSelection (ST_PREV_SYMBOL);
		}
		else
		{
			if (!RemoveSelection ())
			{
				Left ();
			}
		}
		break;

	case VK_RIGHT:
		if (bCtrl)
		{
			if (bShift)
			{
				MakeSelection (ST_NEXT_WORD);
			}
			else
			{
				RemoveSelection (FALSE);
				NextWord ();
			}
		}
		else if (bShift)
		{
			MakeSelection (ST_NEXT_SYMBOL);
		}
		else
		{
			if (!RemoveSelection (FALSE))
			{
				Right ();
			}
		}
		break;

	case VK_HOME:
		if (bCtrl)
		{
			if (bShift)
			{
				MakeSelection (ST_START_OF_TEXT);
			}
			else
			{
				RemoveSelection ();
				StartOfText ();
			}
		}
		else if (bShift)
		{
			MakeSelection (ST_HOME);
		}
		else
		{
			RemoveSelection ();
			Home ();
		}
		break;

	case VK_END:
		if (bCtrl)
		{
			if (bShift)
			{
				MakeSelection (ST_END_OF_TEXT);
			}
			else
			{
				RemoveSelection (FALSE);
				EndOfText ();
			}
		}
		else if (bShift)
		{
			MakeSelection (ST_END);
		}
		else
		{
			RemoveSelection (FALSE);
			End ();
		}
		break;

	case VK_UP:
		if (bCtrl)
		{
			if (!bShift)
			{
				ScrollText (FALSE);
			}
		}
		else if (bShift)
		{
			MakeSelection (ST_PREV_LINE);
		}
		else
		{
			BOOL bResult = RemoveSelection ();
			Up (!bResult);
		}
		break;

	case VK_DOWN:
		if (bCtrl)
		{
			if (!bShift)
			{
				ScrollText (TRUE);
			}
		}
		else if (bShift)
		{
			MakeSelection (ST_NEXT_LINE);
		}
		else
		{
			BOOL bResult = RemoveSelection (FALSE);
			Down (!bResult);

			m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);
		}
		break;

	case VK_PRIOR:
		if (!bCtrl)
		{
			if (!bShift)
			{
				RemoveSelection ();
				PageUp ();
			}
			else
			{
				MakeSelection (ST_PREV_PAGE);
			}
		}
		break;

	case VK_NEXT:
		if (!bCtrl)
		{
			if (!bShift)
			{
				RemoveSelection ();
				PageDown ();
			}
			else
			{
				MakeSelection (ST_NEXT_PAGE);
			}
		}
		break;

	case VK_RETURN:
		DeleteSelectedText (TRUE, FALSE, TRUE);		
		if (GetOverrideMode ())
		{
			Down ();
			Home ();
		}
		else
		{
			SetLastUndoReason (g_dwUATEnter);
			InsertNewLine (TRUE);
		}
		m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);
		break;

	case VK_TAB:
		if (!bCtrl)
		{
			if (!bShift && m_iStartSel == -1)
			{
				SetLastUndoReason (g_dwUATTab);
				InsertTab (TRUE);
			}	
			else if (bShift && m_iStartSel == -1)
			{
				PrevIndent ();
			}
			else if (bShift && m_iStartSel != -1)
			{
				IndentSelection (FALSE);
			}
			else
			{
				if (!IndentSelection (TRUE))
				{
					DeleteSelectedText (FALSE, FALSE, TRUE);
					InsertTab (TRUE);
				}
			}
		}
		break;

	case VK_BACK:
		if (m_nCurrOffset > 0 || m_iStartSel >= 0)
		{
			BOOL bHideCaret = FALSE;

			if (bCtrl)
			{
				HideCaret ();
				bHideCaret = TRUE;

				int nOldOffset = m_nCurrOffset;

				if (m_iStartSel == -1)
				{
					m_iStartSel = nOldOffset;
				}
				PrevWord (FALSE);
				m_iEndSel = m_nCurrOffset;

				SetCaret (nOldOffset, FALSE, FALSE);
			}

			CPoint pt;
			OffsetToPoint (m_nCurrOffset - 1, pt);

			if (pt.y >= 0)
			{
				m_nCurrOffset--;
				SetLastUndoReason (g_dwUATBackspace);
				if (!OnDelete (TRUE))
				{
					m_nCurrOffset++;
				}
				SetCaret (m_nCurrOffset, TRUE, TRUE);
			}
			else
			{
				int nOldOffset = m_nCurrOffset;
				SetCaret (m_nCurrOffset - 1, TRUE, FALSE);
				SetLastUndoReason (g_dwUATBackspace	);
				if (!OnDelete (TRUE))
				{
					SetCaret (nOldOffset, TRUE, TRUE);
				}
			}

			if (bHideCaret)
			{
				ShowCaret ();
			}
		}
		break;									  
												  
	case VK_DELETE:
		{
			BOOL bHideCaret = FALSE;
			if (bCtrl)
			{
				HideCaret ();
				bHideCaret = TRUE;

				int nOldOffset = m_nCurrOffset;

				if (m_iStartSel == -1)
				{
					m_iStartSel = nOldOffset;
				}
				NextWord (FALSE);
				m_iEndSel = m_nCurrOffset;

				SetCaret (nOldOffset, FALSE, FALSE);
			}

			SetLastUndoReason (g_dwUATDelete);
			OnDelete (TRUE);

			if (bHideCaret)
			{
				ShowCaret ();
			}
		}

	case VK_ESCAPE:
		RemoveSelection (FALSE, TRUE);
		break;

	case VK_INSERT:
		OnSetOvrMode ();
		break;
	}
		  
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************
BOOL CBCGPEditCtrl::OnSetOvrMode ()
{
	if (m_bReadOnly)
	{
		OnFailedOperation (g_dwOpSetOvrMode | g_dwOpReasonReadOnly);
		return FALSE;
	}

	SetOverrideMode (!GetOverrideMode ());
	::DestroyCaret ();
	CSize sizeCaret = GetCaretSize ();
	::CreateCaret (GetSafeHwnd (), NULL, sizeCaret.cx, sizeCaret.cy);
	ShowCaret ();
	return TRUE;
}
//*****************************************************************************
CSize CBCGPEditCtrl::GetCaretSize () const
{
	return GetOverrideMode () ? CSize (m_nMaxCharWidth, m_nLineHeight) : 
						CSize (2, m_nLineHeight);
}
//*****************************************************************************
void CBCGPEditCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
#ifdef _UNICODE
    if (nChar != VK_TAB &&
        iswprint ((wint_t) nChar) && !m_bReadOnly)
#else
	if ((isprint (nChar) || nChar > 127) && !m_bReadOnly)
#endif
	{
		DeleteSelectedText (TRUE, FALSE, TRUE);
		SetLastUndoReason (g_dwUATTyping);
		InsertChar ((TCHAR) nChar, TRUE);

		CObList lstIntelliSenseData;
		CString strIntelliSense;
		int nCurrOffset = m_nCurrOffset;

		if (IsIntelliSenseEnabled() &&
			(nCurrOffset > 0) &&
			(nCurrOffset <= m_strBuffer.GetLength()) &&
			m_strIntelliSenseChars.Find (m_strBuffer.GetAt(nCurrOffset - 1)) != -1)
		{
			if (OnFillIntelliSenseList (nCurrOffset, lstIntelliSenseData))
			{
				CPoint pt (0, 0);

				OffsetToPoint (m_nCurrOffset = nCurrOffset, pt);
				ClientToScreen (&pt);

				InvokeIntelliSense (lstIntelliSenseData, pt);
			}
		}
	}

	CWnd::OnChar(nChar, nRepCnt, nFlags);
}
//*****************************************************************************
BOOL CBCGPEditCtrl::OnFillIntelliSenseList (int& nCurrOffset, CObList& lstIntelliSenseData) const
{
	CString strIntelliSence;

	if (!OnBeforeInvokeIntelliSense(m_strBuffer, nCurrOffset, strIntelliSence))
	{
		return FALSE;
	}

	VERIFY(FillIntelliSenseList(lstIntelliSenseData, strIntelliSence.GetBuffer(0)));

	return !lstIntelliSenseData.IsEmpty();
}
//*****************************************************************************
BOOL CBCGPEditCtrl::InvokeIntelliSense()
{
	CObList lstIntelliSenseData;
	int nCurrOffset = m_nCurrOffset;

	if (!OnFillIntelliSenseList(nCurrOffset, lstIntelliSenseData))
	{
		OnFailedOperation (g_dwOpInvokeIS);
		return FALSE;
	}

	CPoint pt;

	OffsetToPoint(m_nCurrOffset, pt);
	ClientToScreen (&pt);
	
	return InvokeIntelliSense (lstIntelliSenseData, pt);
}
//*****************************************************************************
BOOL CBCGPEditCtrl::InvokeIntelliSense (CObList& lstIntelliSenseData, 
										CPoint ptTopLeft)
{
	ASSERT (m_bIntelliSenseSupport);
	
	if (lstIntelliSenseData.IsEmpty())
	{
		return FALSE;
	}

	CBCGPIntelliSenseWnd* pIntelliSenseWnd = new CBCGPIntelliSenseWnd;
	return pIntelliSenseWnd->Create (lstIntelliSenseData, 
		WS_POPUP | WS_VISIBLE | MFS_SYNCACTIVE | MFS_4THICKFRAME, 
		ptTopLeft, this, m_pIntelliSenseLBFont, m_pIntelliSenseImgList);
}
//*****************************************************************************
BOOL CBCGPEditCtrl::InvokeSymList ()
{
	ASSERT (m_bEnableSymSupport);
	CObList lstIntelliSenseData;
	int nCurrOffset = m_nCurrOffset;

	if (!OnFillSymListData (nCurrOffset, lstIntelliSenseData))
	{
		OnFailedOperation (g_dwOpInvokeIS);
		return FALSE;
	}

	CPoint pt;

	OffsetToPoint(m_nCurrOffset, pt);
	ClientToScreen (&pt);

	return InvokeSymList (lstIntelliSenseData, pt);
}
//*****************************************************************************
BOOL CBCGPEditCtrl::OnFillSymListData (int& /*nCurrOffset*/, CObList& lstIntelliSenseData) const 
{
	ASSERT (m_bEnableSymSupport);

	for (POSITION pos = m_lstSymDefs.GetHeadPosition (); pos != NULL;)
	{
		BCGP_EDIT_SYM_DEF symDef = m_lstSymDefs.GetNext (pos);
		CBCGPIntelliSenseData* pData = new CBCGPIntelliSenseData;
		pData->m_nImageListIndex = symDef.m_nIndex;
		pData->m_strItemName = symDef.m_strSymText;
		pData->m_dwData = symDef.m_chReplacement;
		lstIntelliSenseData.AddTail (pData);
	}

	return !lstIntelliSenseData.IsEmpty ();
}
//*****************************************************************************
BOOL CBCGPEditCtrl::InvokeSymList (CObList& lstIntelliSenseData, 
									CPoint ptTopLeft)
{
	ASSERT (m_bEnableSymSupport);
	if (lstIntelliSenseData.IsEmpty())
	{
		return FALSE;
	}

	CBCGPIntelliSenseWnd* pIntelliSenseWnd = new CBCGPIntelliSenseWnd;
	return pIntelliSenseWnd->Create (lstIntelliSenseData, 
		WS_POPUP | WS_VISIBLE | MFS_SYNCACTIVE | MFS_4THICKFRAME, 
		ptTopLeft, this, m_pIntelliSenseLBFont, m_pSymbolsImgList, 
		RUNTIME_CLASS (CBCGPSymImagesLB));
}
//**************************************************************************************
BOOL CBCGPEditCtrl::LookUpSymbol (LPCTSTR lpcszBuffer, int nOffset, int nCount, 
								  TCHAR chSymbol, BCGP_EDIT_SYM_DEF& symDef, 
								  int& nSymExtraLen, BOOL bForward)
{
	ASSERT (m_bEnableSymSupport);
	for (POSITION pos = m_lstSymDefs.GetHeadPosition (); pos != NULL;)
	{
		BCGP_EDIT_SYM_DEF& nextSym = m_lstSymDefs.GetNext (pos);
		if (nextSym.m_chReplacement == chSymbol)
		{
			symDef = nextSym;
			nSymExtraLen = 0;
			return TRUE;
		}

		if (!nextSym.m_strSymSequence.IsEmpty ())
		{
			int nLen = nextSym.m_strSymSequence.GetLength ();
			if (bForward && nCount - nOffset < nLen ||
				!bForward && nOffset - nLen + 1 < 0)
			{
				continue;
			}
			LPCTSTR lpcszBuffToCompare = bForward ? lpcszBuffer + nOffset : 
										 lpcszBuffer + nOffset - nLen + 1;			
			if (_tcsncmp (lpcszBuffToCompare, nextSym.m_strSymSequence, nLen) == 0)
			{
				symDef = nextSym;
				nSymExtraLen = nLen - 1;
				return TRUE;
			}
		}
	}
	
	return FALSE;
}
//*****************************************************************************
int  CBCGPEditCtrl::GetSymbolExtraLen (CString& strBuffer, int nOffset, BOOL bForward)
{
	ASSERT (m_bEnableSymSupport);
	int nSymExtraLen = 0;
	BCGP_EDIT_SYM_DEF symDef;

	if (!bForward)
	{
		nOffset--;
	}

	if (nOffset < 0)
	{
		nOffset = 0;
	}

	LookUpSymbol (strBuffer, nOffset, strBuffer.GetLength (), strBuffer.GetAt (nOffset), 
						symDef, nSymExtraLen, bForward);
	return nSymExtraLen;
}
//*****************************************************************************
BOOL CBCGPEditCtrl::EnableIntelliSense(BOOL bFl /* = TRUE */)
{
	const BOOL bIntelliSense = m_bIntelliSenseSupport;
	m_bIntelliSenseSupport = bFl;

	return bIntelliSense;
}
//*****************************************************************************
void CBCGPEditCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	RecalcLayout ();

	CRect rect (0, 0, cx, cy);
	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		m_pToolTip->SetToolRect (this, GetDlgCtrlID (), rect);
	}
}
//*****************************************************************************
void CBCGPEditCtrl::RecalcLayout ()
{
	UpdateScrollBars ();

	SetCaret (m_nCurrOffset, FALSE);

	int nHiddenLines = (m_bEnableOutlining ? m_nHiddenLines : 0);
	if (m_nTotalLines - nHiddenLines < m_rectText.Height () / m_nLineHeight &&
		m_nTopVisibleOffset > 0)
	{
		OnScroll (SB_VERT, SB_THUMBPOSITION, 0);
	}
	else
	{
		m_wndScrollVert.SetScrollPos (m_nScrollOffsetVert);
	}
	
	m_wndScrollHorz.SetScrollPos (m_nScrollOffsetHorz);
}
//*****************************************************************************
int CBCGPEditCtrl::HitTest (CPoint& pt, BOOL bNormalize, BOOL bIgnoreTextBounds)
{
	if (!m_rectText.PtInRect (pt) && !bIgnoreTextBounds)
	{
		return -1;
	}

	int x = -1;
	int y = m_rectText.top; // - m_nScrollOffsetVert * m_nLineHeight;
	int nOffset = 0;

	CClientDC dc (this);
	CFont* pOldFont = SelectFont (&dc);

	// --------------
	// Calculate row:
	// --------------
	int nRow = max (0, (pt.y + m_nScrollOffsetVert * m_nLineHeight) / m_nLineHeight);
	int nColumn = 0;
	int iRowOffset = m_nTopVisibleOffset;
	
	int nStrartFrom = m_nScrollOffsetVert;
	if (pt.y < 0)
	{
		iRowOffset = GetRowStart (nRow, TRUE);
		nStrartFrom = nRow;
	}

	for (int i = nStrartFrom; i <= nRow; i++)
	{
		int iNextRow = m_strBuffer.Find (_T('\n'), iRowOffset);
		while (iNextRow != -1 && FindCollapsedBlock (iNextRow) != NULL) // skip hidden text
		{
			iNextRow = m_strBuffer.Find (_T('\n'), iNextRow + 1);
		}

		if (i == nRow)
		{
			// -----------------
			// Calculate column:
			// -----------------
			CString strRow;
			if (iNextRow < 0)
			{
				strRow = m_strBuffer.Mid (iRowOffset);
			}
			else
			{
				strRow = m_strBuffer.Mid (iRowOffset, iNextRow - iRowOffset);
			}

			int nStringExtent = GetStringExtent (&dc, strRow, strRow.GetLength ()).cx;
			int nXRight = m_rectText.left + 1 + nStringExtent;

			if (nStringExtent > 0 && pt.x >= nXRight)
			{
				x = nXRight;
				nOffset = (iNextRow < 0) ? m_strBuffer.GetLength () : iNextRow;
				nColumn = iNextRow - iRowOffset; 
			}
			else
			{
				int cxPrev = m_rectText.left + 1;

				int nScrollOffsetPix = m_nScrollOffsetHorz * m_nMaxCharWidth; //GetHorzRowExtent (&dc, iRowOffset);
				int nOffsetInRowPix = 0;

				nOffset = -1;		

				int nStartOffsetToCheck = 0;	
				int nExtent = 0; //GetStringExtent (&dc, (LPCTSTR) strRow, nStartOffsetToCheck).cx;

				int iIdx = 0;

				for (iIdx = nStartOffsetToCheck; iIdx < strRow.GetLength (); iIdx++)
				{
					TCHAR chNext = strRow [iIdx];
					int nSymExtraLen = 0;
					int nCharWidth = 0;
					
					CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlock (iRowOffset + iIdx);
					if (pHiddenText != NULL)
					{
						OnCalcOutlineSymbol (&dc, CPoint (m_rectText.left + 1 + nExtent, y), pHiddenText);
						nCharWidth = pHiddenText->m_rectTool.Width ();
						nSymExtraLen = pHiddenText->m_nEnd - pHiddenText->m_nStart;
					}
					else if (m_bEnableSymSupport)
					{
						BCGP_EDIT_SYM_DEF symDef;
						if (LookUpSymbol (strRow, iIdx, strRow.GetLength (), chNext, symDef, nSymExtraLen))
						{
							IMAGEINFO imgInfo;
							m_pSymbolsImgList->GetImageInfo (symDef.m_nIndex, &imgInfo);
							nCharWidth = imgInfo.rcImage.right - imgInfo.rcImage.left;
						}
					}
					if (nCharWidth == 0 && !m_mapCharWidth.Lookup (chNext, nCharWidth))
					{
						nCharWidth = GetStringExtent (&dc, CString (chNext), 1).cx;
						m_mapCharWidth.SetAt (chNext, nCharWidth);
					}

					if (pHiddenText != NULL)
					{
					}
					else if (chNext == _T ('\t'))
					{
						int nRestOfTab = m_nTabSize - nColumn % m_nTabSize;
						
						nColumn += nRestOfTab; 
						
						int nRestOfTabWidth = nCharWidth - nExtent % nCharWidth;
						nCharWidth = nRestOfTabWidth;
					}
					else
					{
						nColumn++;
					}

					nExtent += nCharWidth;

					int cx = m_rectText.left + 1 + nExtent;
					int nAddition = (iIdx == strRow.GetLength () - 1) ? 0 : nCharWidth / 2;

					if (cx >= pt.x + nScrollOffsetPix + nAddition)
					{
						x = cxPrev;
						nOffset = iRowOffset + iIdx;
						nOffsetInRowPix = cx - (m_rectText.left + 1);
						break;
					}
					else if (iIdx == (strRow.GetLength () - 1)) 
                    { 
                            x = cx; 
                            nOffset = iRowOffset + strRow.GetLength(); 
                            break; 
                    } 


					cxPrev = cx;
					iIdx += nSymExtraLen;
				}

				if (nOffset == -1)
				{
					nOffset = iRowOffset + iIdx;
				}

				nColumn++;
			}

			break;
		}

		iRowOffset = iNextRow + 1;
		y += m_nLineHeight;

		if (iNextRow == -1 && x == -1)
		{
			x = m_rectText.left + 1;
			y -= m_nLineHeight;
			nOffset = m_strBuffer.GetLength ();
			break;
		}
	}

	dc.SelectObject (pOldFont);

	if (bNormalize)
	{
		pt.x = x;
		pt.y = y;
	}

	return nOffset;
}
//******************************************************************************
BOOL CBCGPEditCtrl::SetCaret (int nOffset, BOOL bScrollToCaret, BOOL bRedrawOnScroll)
{
	if (m_bDisableSetCaret)
	{
		return FALSE;
	}

	CPoint pt;
	CPoint ptRowColumn;
	int	nScrollOffset;

	if (!OffsetToPoint (nOffset, pt, &ptRowColumn, &nScrollOffset))
	{
		return FALSE;
	}

	m_nCurrOffset = nOffset;
	m_ptCaret = pt;


	BOOL bScrollRequired = FALSE;
	BOOL bCaretSet = FALSE;
	
	if (bScrollToCaret)
	{
		nScrollOffset = max (0, nScrollOffset);
		nScrollOffset = min (ptRowColumn.x, nScrollOffset);

		if (nScrollOffset != m_nScrollOffsetHorz)
		{
			bScrollRequired = OnScroll (SB_HORZ, SB_THUMBPOSITION, nScrollOffset);
			OffsetToPoint (nOffset, pt, &ptRowColumn);
			m_nCurrOffset = nOffset;
			m_ptCaret = pt;
			if (GetSafeHwnd () == ::GetFocus () ||
				(m_bIntelliSenseMode && m_pIntelliSenseWnd != NULL))
			{
				SetCaretPos (m_ptCaret);
			}
			OnSetCaret	();
			bCaretSet = TRUE;
		}

		int nDeltaY = 0;
		if (m_ptCaret.y < m_rectText.top + m_nLineHeight)
		{
			nDeltaY = ((m_ptCaret.y - m_rectText.top) / m_nLineHeight);
		}
		else if (m_ptCaret.y >= m_rectText.bottom - m_nLineHeight * 2)
		{
			nDeltaY = ((m_ptCaret.y - m_rectText.bottom + m_nLineHeight * 2) / m_nLineHeight);
		}

		if (nDeltaY != 0)
		{
			bScrollRequired = OnScroll (SB_VERT, SB_THUMBPOSITION, m_nScrollOffsetVert + nDeltaY);
			OffsetToPoint (nOffset, pt, &ptRowColumn);
			m_nCurrOffset = nOffset;
			m_ptCaret = pt;
			if (GetSafeHwnd () == ::GetFocus () ||
				(m_bIntelliSenseMode && m_pIntelliSenseWnd != NULL))
			{
				SetCaretPos (m_ptCaret);
			}
			OnSetCaret	();
			bCaretSet = TRUE;
		}
	}

	m_nCurRow	 = RowFromOffset (m_nCurrOffset);//ptRowColumn.y;
	m_nCurColumn = GetColumnFromOffset (m_nCurrOffset, FALSE);

	if (!bCaretSet && bScrollToCaret)
	{
		if (GetSafeHwnd () == ::GetFocus () ||
			(m_bIntelliSenseMode && m_pIntelliSenseWnd != NULL))
		{
			SetCaretPos (m_ptCaret);
		}
		OnSetCaret	();
	}

	if (bRedrawOnScroll && bScrollRequired)
	{
		RedrawWindow ();
	}

	return TRUE;
}
//***************************************************************************************
int CBCGPEditCtrl::GetColumnFromOffset (int nOffset, BOOL bSkipHidden) const
{
	int nRowStart = GetRowStartByOffset (nOffset, bSkipHidden);
	
	if (m_bKeepTabs)
	{
		int nColumn = 0;
		int nNumChars = nOffset - nRowStart;
		LPCTSTR lpcszRowStart = ((LPCTSTR) m_strBuffer) + nRowStart;
		for (int i = 0; i < nNumChars; i++)
		{
			CBCGPOutlineBaseNode* pHiddenText = NULL;
			pHiddenText = bSkipHidden ? FindCollapsedBlock (nRowStart + i) : NULL;
			if (pHiddenText == NULL)
			{
				TCHAR chNext = lpcszRowStart [i];
				if (chNext == _T ('\t'))
				{
					int nRestOfTab = m_nTabSize - nColumn % m_nTabSize;
					nColumn += nRestOfTab; 
				}
				else
				{
					nColumn++;
				}
			}
			else
			{
				nColumn += pHiddenText->m_strReplace.GetLength ();
				i += pHiddenText->m_nEnd - (nRowStart + i); // skip the rest of the hidden text
			}
		}
		return nColumn;
	}
	
	else if (bSkipHidden)
	{
		int nColumn = 0;
		for (int i = nRowStart; i < nOffset; )
		{
			CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlockInRange (i, nOffset, TRUE);
			if (pHiddenText != NULL)
			{
				ASSERT_VALID (pHiddenText);

				nColumn += pHiddenText->m_nStart - i;
				
				if (nOffset == pHiddenText->m_nStart)
				{
					break;
				}

				nColumn += pHiddenText->m_strReplace.GetLength ();
				i += max (pHiddenText->m_nEnd - i + 1, 1); // skip the rest of the hidden text
			}
			else
			{
				nColumn += nOffset - i;
				break;
			}
		}
		return nColumn;
	}

	return nOffset - nRowStart;
}
//******************************************************************************
void CBCGPEditCtrl::OnChangeFont (CDC* pDC)
{
	CClientDC dc (this);

	if (pDC == NULL)
	{
		pDC = &dc;
	}

	CFont* pOldFont = SelectFont (pDC);

	TEXTMETRIC tm;
	pDC->GetTextMetrics (&tm);

	if (m_pSymbolsImgList != NULL)
	{
		IMAGEINFO imgInfo;
		memset (&imgInfo, 0, sizeof (IMAGEINFO));
		if (m_pSymbolsImgList->GetImageInfo (0, &imgInfo))
		{
			int nImgHeight = imgInfo.rcImage.bottom - imgInfo.rcImage.top;
			if (nImgHeight > m_nLineHeight)
			{
				m_nLineVertSpacing += nImgHeight - m_nLineHeight;
			}
		}
	}

	m_nLineHeight = tm.tmHeight + m_nLineVertSpacing;
	m_nMaxCharWidth = tm.tmMaxCharWidth;
	m_nAveCharWidth = tm.tmAveCharWidth;

	pDC->SelectObject (pOldFont);

	m_mapCharWidth.RemoveAll ();

	//------------------
	// Initialize caret:
	//------------------
	::DestroyCaret ();

	CSize sizeCaret = GetCaretSize ();
	::CreateCaret (GetSafeHwnd (), NULL, sizeCaret.cx, sizeCaret.cy);

	UpdateScrollBars ();

	if (!m_bReadOnly)
	{
		SetCaret (m_nCurrOffset);
		ShowCaret ();
	}
}
//******************************************************************************
BOOL CBCGPEditCtrl::OffsetToPoint (int nOffset, CPoint& pt, LPPOINT ptRowColumn, LPINT pnScrollOffset)
{
	ASSERT_VALID (this);

	nOffset = min (max (nOffset, 0), m_strBuffer.GetLength ());

	CClientDC dc (this);
	CFont* pOldFont = SelectFont (&dc);

	int nRow = -1;
	int iRowOffset = 0;
	int nLowBound = 0;

	if (nOffset >= m_nTopVisibleOffset)
	{
		nLowBound = m_nTopVisibleOffset;
	}
	
	// --------------
	// Calculate row:
	// --------------
	CBCGPOutlineBaseNode* pHiddenText = NULL;
	if (nOffset - 1 >= nLowBound)
	{
		pHiddenText = FindCollapsedBlockInRange (nLowBound, nOffset - 1, FALSE);
	}

	int i = 0;

	for (i = nOffset - 1; i >= nLowBound; )
	{
		if (pHiddenText != NULL && pHiddenText->m_nEnd >= i)
		{
			// skip hidden text
			i -= max (i - pHiddenText->m_nStart + 1, 1);

			if (i >= nLowBound)
			{
				pHiddenText = FindCollapsedBlockInRange (nLowBound, i, FALSE);
			}
		}
		else
		{
			if (m_strBuffer [i] == _T('\n'))
			{
				if (nRow == -1)
				{
					nRow = 1;
				}
				else
				{
					nRow ++;
				}
			}

			i--;
		}
	}

	if (nRow == -1)
	{
		nRow = 0;
	}

	if (nOffset >= m_nTopVisibleOffset)
	{
		nRow += m_nScrollOffsetVert;
	}

	// -----------------
	// Calculate column:
	// -----------------
	iRowOffset = GetRowStartByOffset (nOffset, TRUE);
	int nScrolledExtent = m_nScrollOffsetHorz * m_nMaxCharWidth;

	int nStringExtent = 0;
	for (i = iRowOffset; i < nOffset; )
	{
		CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlockInRange (i, nOffset, TRUE);
		if (pHiddenText != NULL)
		{
			ASSERT_VALID (pHiddenText);

			nStringExtent += GetStringExtent (&dc, (LPCTSTR) m_strBuffer + i, pHiddenText->m_nStart - i).cx;

			if (nOffset == pHiddenText->m_nStart)
			{
				break;
			}

			OnCalcOutlineSymbol (&dc, pHiddenText->m_rectTool.TopLeft (), pHiddenText);
			nStringExtent += pHiddenText->m_rectTool.Width ();
			
			i += max (pHiddenText->m_nEnd - i + 1, 1);
		}
		else
		{
			nStringExtent += GetStringExtent (&dc, (LPCTSTR) m_strBuffer + i, nOffset - i).cx;
			break;
		}
	}

	pt.x = m_rectText.left + 1 + nStringExtent - nScrolledExtent - 1;

	if (pnScrollOffset != NULL)
	{
		*pnScrollOffset = m_nScrollOffsetHorz;
	
		if (pt.x < m_rectText.left + 1 + m_nMaxCharWidth)
		{
			// scroll left:
			int nDeltaX = ((pt.x - (m_rectText.left + 1)) / m_nMaxCharWidth) - m_nScrollLeftOffset;
			*pnScrollOffset = m_nScrollOffsetHorz + nDeltaX;
		}
		else if (pt.x >= m_rectText.right - m_nMaxCharWidth)
		{
			// scroll right:
			int nScrollRightOffset = m_rectText.Width () / m_nMaxCharWidth * m_nScrollRightOffset / 100;
			if (nScrollRightOffset < 1)
			{
				nScrollRightOffset = 1;
			}

			int nDeltaX = ((pt.x - m_rectText.right) / m_nMaxCharWidth) + nScrollRightOffset;
			*pnScrollOffset = m_nScrollOffsetHorz + nDeltaX;
		}
	}

	pt.x = max (pt.x, m_rectText.left - m_nScrollOffsetHorz * m_nMaxCharWidth + 1);
	pt.y = m_rectText.top + (nRow - m_nScrollOffsetVert) * m_nLineHeight;

	dc.SelectObject (pOldFont);

	if (ptRowColumn != NULL)
	{
		ptRowColumn->x = GetColumnFromOffset (nOffset, TRUE);
		ptRowColumn->y = nRow;
	}

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::Left (BOOL bRedrawOnScroll)
{
	if (m_nCurrOffset == 0)
	{
		OnFailedOperation (g_dwOpLeft);
		return FALSE;
	}

	int nCharCount = 1;
	CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlock (m_nCurrOffset - 1);
	if (pHiddenText != NULL)
	{
		nCharCount += pHiddenText->m_nEnd - pHiddenText->m_nStart;
	}
	else if (m_bEnableSymSupport && m_nCurrOffset > 0)
	{
		nCharCount += GetSymbolExtraLen (m_strBuffer, m_nCurrOffset, FALSE);
	}

	if (!SetCaret (m_nCurrOffset - nCharCount, TRUE, bRedrawOnScroll))
	{
		OnFailedOperation (g_dwOpLeft);
		return FALSE;
	}

	m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::Right (BOOL bRedrawOnScroll)
{
	if (m_nCurrOffset == m_strBuffer.GetLength ())
	{
		OnFailedOperation (g_dwOpRight);
		return FALSE;
	}

	int nCharCount = 1;
	CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlock (m_nCurrOffset);
	if (pHiddenText != NULL)
	{
		nCharCount += pHiddenText->m_nEnd - pHiddenText->m_nStart;
	}
	else if (m_bEnableSymSupport)
	{
		nCharCount += GetSymbolExtraLen (m_strBuffer, m_nCurrOffset);
	}

	if (!SetCaret (m_nCurrOffset + nCharCount, TRUE, bRedrawOnScroll))
	{
		OnFailedOperation (g_dwOpRight);
		return FALSE;
	}

	m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::Up (BOOL bSetCaretToMaxColumn, BOOL bRedrawOnScroll)
{
	if (m_nCurRow == 0)
	{
		OnFailedOperation (g_dwOpUp);
		return FALSE;
	}

	if (m_strBuffer.IsEmpty ())
	{
		OnFailedOperation (g_dwOpUp);
		return FALSE;
	}


	CPoint point (m_ptCaret.x, m_ptCaret.y);

	int nCurrOldRow = m_nCurRow;

	if (m_ptCaret.y - m_nLineHeight < m_rectText.top )
	{
		ScrollUp (SB_VERT, bRedrawOnScroll);
	}
	else
	{
		point.y -= m_nLineHeight;
	}

	int nOffset = HitTest (point, TRUE);
	if (nOffset >= 0 && nCurrOldRow > 0)
	{
		if (bSetCaretToMaxColumn && SetCaretToLastMaxColumn (nOffset, bRedrawOnScroll))
		{
			return TRUE;
		}
		else
		{
			return SetCaret (nOffset, TRUE, bRedrawOnScroll);
		}
	}

	OnFailedOperation (g_dwOpUp);
	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::Down (BOOL bSetCaretToMaxColumn, BOOL bRedrawOnScroll)
{
	CPoint point (m_ptCaret.x, m_ptCaret.y);

	if (m_ptCaret.y + m_nLineHeight >= m_rectText.bottom - m_nLineHeight && m_nCurRow < m_szTotalScroll.cy)
	{
		ScrollDown (SB_VERT, bRedrawOnScroll);
	}
	else
	{
		point.y += m_nLineHeight;
	}
	
	int nOffset = HitTest (point, TRUE);

	if (nOffset >= 0)
	{
		if (bSetCaretToMaxColumn && SetCaretToLastMaxColumn (nOffset, bRedrawOnScroll))
		{
			return TRUE;
		}
		else
		{
			return SetCaret (nOffset, TRUE, bRedrawOnScroll);
		}
	}

	OnFailedOperation (g_dwOpDown);
	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::PageUp (BOOL bRedrawOnScroll)
{
	if (m_strBuffer.IsEmpty ())
	{
		OnFailedOperation (g_dwOpPgUp);
		return FALSE;
	}

	SCROLLINFO scrollInfo;
	ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));

    scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_ALL;

	GetScrollInfo (SB_VERT, &scrollInfo);

	CPoint point (m_ptCaret.x, m_ptCaret.y);

	if ((int) scrollInfo.nPage > m_nCurRow && m_nScrollOffsetVert == 0)
	{
		point.y = 0;
	}
	else
	{
		ScrollPageUp (SB_VERT, bRedrawOnScroll);
	}

	int nOffset = HitTest (point, TRUE);

	if (nOffset >= 0)
	{
		if (SetCaretToLastMaxColumn (nOffset, bRedrawOnScroll))
		{
			return TRUE;
		}
		else
		{
			return SetCaret (nOffset, TRUE, bRedrawOnScroll);
		}
	}

	OnFailedOperation (g_dwOpPgUp);
	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::PageDown (BOOL bRedrawOnScroll)
{
	int nOffset = -1;
	if (!ScrollPageDown (SB_VERT, bRedrawOnScroll))
	{
		nOffset = max (m_strBuffer.GetLength (), 0);
	}
	else
	{
		CPoint point (m_ptCaret.x, m_ptCaret.y);
		nOffset = HitTest (point, TRUE);
	}

	nOffset = min (nOffset, m_strBuffer.GetLength ());

	if (nOffset >= 0)
	{
		if (SetCaretToLastMaxColumn (nOffset, bRedrawOnScroll))
		{
			return TRUE;
		}
		else
		{
			return SetCaret (nOffset, TRUE, bRedrawOnScroll);
		}
	}
	OnFailedOperation (g_dwOpPgDn);
	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::Home (BOOL bRedrawOnScroll)
{
	if (m_strBuffer.IsEmpty ())
	{
		OnFailedOperation (g_dwOpHome);
		return FALSE;
	}

	BOOL bResult = FALSE;
	ScrollHome (SB_HORZ, bRedrawOnScroll);
	int nRowStart = GetCurrRowStart (TRUE);
	int nRowTextStart = GetCurrRowTextStart (TRUE);

	if (m_nCurrOffset == nRowTextStart)
	{
		bResult = SetCaret (nRowStart, TRUE, bRedrawOnScroll);
	}
	else
	{
		bResult = SetCaret (nRowTextStart, TRUE, bRedrawOnScroll);
	}

	if (bResult)
	{
		m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);
	}
	return bResult;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::End (BOOL bRedrawOnScroll)
{
	int nOffset = m_nCurrOffset;
	while (nOffset < m_strBuffer.GetLength () &&
		(m_strBuffer [nOffset] != _T('\n') || FindCollapsedBlock (nOffset) != NULL))
	{
		nOffset++;
	}

	nOffset = min (nOffset, m_strBuffer.GetLength ());

	SetCaret (nOffset, TRUE, bRedrawOnScroll);
	m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);
	return TRUE;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::StartOfText (BOOL bRedraw)
{
	ScrollHome (SB_VERT, FALSE);
	ScrollHome (SB_HORZ, FALSE);

	if (bRedraw)
	{
		RedrawWindow ();
	}

	BOOL bResult = SetCaret (0, TRUE, FALSE);
	m_nLastMaxColumn = 0;

	return bResult;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::EndOfText (BOOL bRedrawOnScroll)
{
	if (m_strBuffer.IsEmpty ())
	{
		return TRUE;
	}

	return SetCaret (m_strBuffer.GetLength (), TRUE, bRedrawOnScroll);
}
//****************************************************************************************
BOOL CBCGPEditCtrl::NextWord (BOOL bRedrawOnScroll)
{
	if (m_nCurrOffset >= m_strBuffer.GetLength () - 1)
	{
		OnFailedOperation (g_dwOpNextWord);
		return FALSE;
	}

	int i = m_nCurrOffset;
	BOOL bCaretInsideWord = m_strWordDelimeters.Find (m_strBuffer [i]) < 0;
	BOOL bCaretInsideDelimiters = m_strWordDelimeters.Find (m_strBuffer [i]) >= 0 && 
								  m_strSpecialDelimiters.Find (m_strBuffer [i]) < 0 &&	
								  (m_strBuffer [i] != _T (' '));
	int nSpecialDelimiterIdx = m_strSpecialDelimiters.Find (m_strBuffer [i]);
	BOOL bCaretInsideSpecialDelimiters = (nSpecialDelimiterIdx >= 0);
	TCHAR chSpecialDelimeterToSkip = 0;

	if (bCaretInsideSpecialDelimiters)
	{
		chSpecialDelimeterToSkip = m_strSpecialDelimiters [nSpecialDelimiterIdx];
	}
	
	CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlock (i);
	if (pHiddenText != NULL)
	{
		// Move to end of hidden text:
		do
		{
			i += max (pHiddenText->m_nEnd - i + 1, 1); // skip the rest of the hidden text
			pHiddenText = (i < m_strBuffer.GetLength ()) ? FindCollapsedBlock (i) : NULL;
		}
		while (pHiddenText != NULL);
	}
	else if (bCaretInsideWord)
	{
		// Move to end of word:
		while (i < m_strBuffer.GetLength () &&
			m_strWordDelimeters.Find (m_strBuffer [i]) < 0 && 
			FindCollapsedBlock (i) == NULL)
		{
			i++;
		}
	}
	else if (bCaretInsideDelimiters)
	{
		// Move to end of delimiters string:
		while (i < m_strBuffer.GetLength () &&
			m_strWordDelimeters.Find (m_strBuffer [i]) >= 0 && 
			m_strSpecialDelimiters.Find (m_strBuffer [i]) < 0 &&
			m_strBuffer [i] != _T (' ') &&
			FindCollapsedBlock (i) == NULL)
		{
			i++;
		}
	}
	else if (bCaretInsideSpecialDelimiters)
	{
		// Move to end of special delimiters string:
		while (i < m_strBuffer.GetLength () &&
				m_strBuffer [i] == chSpecialDelimeterToSkip &&
				FindCollapsedBlock (i) == NULL)
		{
			i++;
			if (m_strBuffer [i] == g_chEOL)
			{
				break;
			}
		}
	}

	// Skip spaces:
	while (i < m_strBuffer.GetLength () &&
		m_strBuffer [i] == _T (' ') &&
		FindCollapsedBlock (i) == NULL)
	{
		i++;
	}

	if (!SetCaret (i, TRUE, bRedrawOnScroll))
	{
		OnFailedOperation (g_dwOpNextWord);
		return FALSE;
	}

	m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPEditCtrl::PrevWord (BOOL bRedrawOnScroll)
{
	if (m_nCurrOffset <= 0)
	{
		OnFailedOperation (g_dwOpPrevWord);
		return FALSE;
	}

	int i = min (m_nCurrOffset - 1, m_strBuffer.GetLength () - 1);

	BOOL bCursorAtEOL = m_strBuffer [i] == g_chEOL;
	// Skip spaces:
	while (i >= 0 && m_strBuffer [i] == _T (' '))
	{
		i--;
		bCursorAtEOL = FALSE;
	}

	if (i == 0)
	{
		SetCaret (0, TRUE, bRedrawOnScroll);
		return TRUE;
	}

	BOOL bCaretInsideWord = m_strWordDelimeters.Find (m_strBuffer [i]) < 0;
	BOOL bCaretInsideDelimiters = m_strWordDelimeters.Find (m_strBuffer [i]) >= 0 && 
								  m_strSpecialDelimiters.Find (m_strBuffer [i]) < 0 &&	
								  (m_strBuffer [i] != _T (' '));
	int nSpecialDelimiterIdx = m_strSpecialDelimiters.Find (m_strBuffer [i]);
	BOOL bCaretInsideSpecialDelimiters = (nSpecialDelimiterIdx >= 0);
	TCHAR chSpecialDelimeterToSkip = 0;

	if (bCaretInsideSpecialDelimiters)
	{
		chSpecialDelimeterToSkip = m_strSpecialDelimiters [nSpecialDelimiterIdx];
	}

	CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlock (i);
	if (pHiddenText != NULL)
	{
		// Move to end of hidden text:
		do
		{
			i -= max (i - pHiddenText->m_nStart + 1, 1); // skip the rest of the hidden text
			pHiddenText = (i >= 0) ? FindCollapsedBlock (i) : NULL;
		}
		while (pHiddenText != NULL);
	}
	else if (bCaretInsideWord)
	{
		// Move to end of word:
		while (i >= 0 &&
			m_strWordDelimeters.Find (m_strBuffer [i]) < 0 && 
			m_strBuffer [i] != g_chEOL &&
			FindCollapsedBlock (i) == NULL)
		{
			i--;
		}
	}
	else if (bCaretInsideDelimiters)
	{
		// Move to end of delimiters string:
		while (i >= 0 &&
			m_strWordDelimeters.Find (m_strBuffer [i]) >= 0 && 
			m_strSpecialDelimiters.Find (m_strBuffer [i]) < 0 &&
			m_strBuffer [i] != _T (' ') && 
			m_strBuffer [i] != g_chEOL &&
			FindCollapsedBlock (i) == NULL)
		{
			i--;
		}
	}
	else if (bCaretInsideSpecialDelimiters)
	{
		// Move to end of special delimiters string:
		while  (i >= 0 &&
				m_strBuffer [i] == chSpecialDelimeterToSkip &&
				m_strBuffer [i] != _T (' ') && m_strBuffer [i] != g_chEOL &&
				FindCollapsedBlock (i) == NULL)
		{
			i--;
		}
	}
	else
	{
		// Skip spaces:
		while  (i >= 0 && m_strBuffer [i] == _T (' ') &&
				FindCollapsedBlock (i) == NULL)
		{
			i--;
		}
	}

	if (bCursorAtEOL)
	{
		i--;
	}

	if (!SetCaret (i + 1, TRUE, bRedrawOnScroll))
	{
		OnFailedOperation (g_dwOpPrevWord);
		return FALSE;
	}

	m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPEditCtrl::PrevIndent ()
{
	int nCurColumn = GetCurColumn ();

	if (nCurColumn == 0)
	{
		OnFailedOperation (g_dwOpPrevIndent);
		return FALSE;
	}

	int nOffsetToMove = nCurColumn % m_nIndentSize;
	
	if (nOffsetToMove == 0)
	{
		nOffsetToMove = m_nIndentSize;
	}
	
	for (int i = 0; i < nOffsetToMove; i++)
	{
		Left ();
		if (m_strBuffer [m_nCurrOffset] == _T ('\t'))
		{
			i += m_nTabSize;
		}
	}

	return TRUE;
}
//****************************************************************************************
int CBCGPEditCtrl::GetCurrRowStart (BOOL bSkipHidden) const
{
	int i = 0;

	for (i = m_nCurrOffset - 1; i >= 0; i--)
	{
		if (m_strBuffer [i] == _T('\n'))
		{
			if (!bSkipHidden || FindCollapsedBlock (i) == NULL)
			{
				break;
			}
		}
	}

	return i + 1;
}
//***************************************************************************************
int CBCGPEditCtrl::GetCurrRowEnd (BOOL bSkipHidden) const
{
	int i = 0;
	for (i = m_nCurrOffset; i < m_strBuffer.GetLength (); i++)
	{
		if (m_strBuffer [i] == _T('\n'))
		{
			if (!bSkipHidden || FindCollapsedBlock (i) == NULL)
			{
				break;
			}
		}
	}

	return i;
}
//***************************************************************************************
int CBCGPEditCtrl::GetCurrRowTextStart (BOOL bSkipHidden) const
{
	int nStart = GetCurrRowStart (bSkipHidden);
	return GetRowTextStart (nStart, bSkipHidden);
}
//***************************************************************************************
int CBCGPEditCtrl::GetNumOfColumnsInRowByOffset (int nOffset, int& nRowEndOffset, BOOL bSkipHidden) const
{
	nRowEndOffset = GetRowEndByOffset (nOffset, bSkipHidden);
	return GetColumnFromOffset (nRowEndOffset, bSkipHidden);
}
//***************************************************************************************
int CBCGPEditCtrl::GetRowStart (int nRow, BOOL bSkipHidden) const
{
	if (nRow == 0)
	{
		return 0;
	}

	int nIdx = 0;
	int nRowIdx = 0;

	for (nIdx = 0;  nRowIdx != nRow; nIdx++)
	{
		nIdx = m_strBuffer.Find (_T('\n'), nIdx);
		if (nIdx == -1)
		{
			break;
		}

		if (!bSkipHidden || FindCollapsedBlock (nIdx) == NULL)
		{
			nRowIdx++;
		}
	}

	if (nIdx == -1)
	{
		return -1;
	}

	return nIdx;
}
//***************************************************************************************
int CBCGPEditCtrl::GetRowTextStart (int nRowOffset, BOOL bSkipHidden) const
{
	if (nRowOffset == -1)
	{
		return 0;
	}
	
	if (nRowOffset > m_strBuffer.GetLength ())
	{
		return m_strBuffer.GetLength () - 1;
	}

	int i = 0;
	for (i = nRowOffset; i < m_strBuffer.GetLength (); i++)
	{
		if (bSkipHidden && FindCollapsedBlock (i) != NULL)
		{
			break;
		}

		if (m_strBuffer [i] == g_chEOL)
		{
			break;
		}

		if (m_strNonSelectableChars.Find (m_strBuffer [i]) == -1)
		{
			break;
		}
	}
	
	return i;
}
//***************************************************************************************
int CBCGPEditCtrl::GetRowStartByOffset (int nOffset, BOOL bSkipHidden) const
{
	BOOL bFound = FALSE;
	if (nOffset > m_strBuffer.GetLength ())
	{
		nOffset = m_strBuffer.GetLength ();
	}

	int i = 0;
	for (i = nOffset - 1; i >= 0; i--)
	{
		if (m_strBuffer [i] == _T('\n'))
		{
			if (!bSkipHidden || FindCollapsedBlock (i) == NULL)
			{
				bFound = TRUE;
				break;
			}
		}
	}

	if (!bFound)
	{
		return 0;
	}

	return i + 1;
}
//***************************************************************************************
int CBCGPEditCtrl::GetRowEndByOffset (int nOffset, BOOL bSkipHidden) const
{
	int nIdx = m_strBuffer.Find (g_chEOL, nOffset);
	while (nIdx != -1 && bSkipHidden && FindCollapsedBlock (nIdx) != NULL) // skip hidden text
	{
		nIdx = m_strBuffer.Find (g_chEOL, nIdx + 1);
	}

	if (nIdx == -1)
	{
		nIdx = m_strBuffer.GetLength ();
	}

	return nIdx;
}
//**************************************************************************************
int CBCGPEditCtrl::GetNumOfCharsInText (const CString& strText, TCHAR ch)
{
	int nNumChars = 0;
	for (int iIdx = 0; iIdx != -1; iIdx++)
	{
		iIdx = strText.Find (ch, iIdx);
		if (iIdx != -1)
		{
			nNumChars++;
		}
		else
		{
			break;
		}
	}

	return nNumChars;
}
//***************************************************************************************
int CBCGPEditCtrl::GetNumOfCharsInText (int nStart, int nEnd, TCHAR ch, BOOL bSkipHidden) const
{
	ASSERT (nStart >= 0);
	ASSERT (nEnd < m_strBuffer.GetLength ());
	
	if (nStart > nEnd)
	{
		return 0;
	}

	int nNumChars = 0;
	for (int iIdx = nStart; iIdx != -1 && iIdx <= nEnd; iIdx++)
	{
		iIdx = m_strBuffer.Find (ch, iIdx);
		if (iIdx != -1 && iIdx <= nEnd)
		{
			if (!bSkipHidden || FindCollapsedBlock (iIdx) == NULL)
			{
				nNumChars++;
			}
		}
		else
		{
			break;
		}
	}

	return nNumChars;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::InsertChar (TCHAR nChar, BOOL bRedraw)
{
	ASSERT_VALID (this);
	ASSERT (m_nCurrOffset <= m_strBuffer.GetLength ());

	CString strChar (nChar);

	if (!OnBeforeTextInserted (strChar, m_nCurrOffset))
	{
		return FALSE;
	}

	if (nChar == _T('\n'))
	{
		if (GetOverrideMode ())
		{
			Home ();
			return Down ();
		}
		return InsertNewLine (TRUE);
	}

	int nCurrOffset = m_nCurrOffset;

	int nRowEndOffset = GetRowEndByOffset (m_nCurrOffset, FALSE);
	int nRowLen = m_bKeepTabs ? GetNumOfColumnsInRowByOffset (m_nCurrOffset, nRowEndOffset, FALSE) :
					nRowEndOffset - GetRowStartByOffset (m_nCurrOffset, FALSE);

	if (nRowLen >= m_nMaxScrollWidth - 1)
	{
		HideCaret ();
		InsertNewLine (TRUE);
		if (nRowEndOffset != nCurrOffset)
		{
			SetCaret (nCurrOffset);
		}
		ShowCaret ();
	}

	BOOL bForceInsertMode = !GetOverrideMode () || 
							((m_nCurrOffset == m_strBuffer.GetLength () ||
							  m_strBuffer.GetAt (m_nCurrOffset) == g_chEOL)) && GetOverrideMode ();

	AddUndoAction (nChar, g_dwUATInsertData, m_nCurrOffset, bForceInsertMode);

	BOOL bAtColorBlock = IsOffsetAtColorBlock (m_nCurrOffset);

	if (GetOverrideMode () && !m_strBuffer.IsEmpty () && 
		m_nCurrOffset < m_strBuffer.GetLength () &&
		m_strBuffer.GetAt (m_nCurrOffset) != g_chEOL)
	{
		m_strBuffer.SetAt (m_nCurrOffset, nChar);
	}
	else
	{
		m_strBuffer.Insert (m_nCurrOffset, nChar);
		UpdateOffsetRelatedData (m_nCurrOffset, m_nCurrOffset + 1);
	}	

	OnUpdateAutoOutlining (m_nCurrOffset, 1, FALSE);
	OnAfterTextChanged (m_nCurrOffset, CString (nChar), TRUE);
	
	UpdateScrollBars ();

	if (bRedraw)
	{
		if (IsOffsetAtColorBlock (m_nCurrOffset) || bAtColorBlock)
		{
			RedrawWindow (m_rectText);
		}
		else
		{
			RedrawRestOfLine (m_nCurrOffset);
		}
	}

	Right ();

//	IntelliSense char update ...
	CString strIntelliSense;

	if (!m_bIntelliSenseUpdate &&
		IntelliSenseCharUpdate(m_strBuffer, m_nCurrOffset, nChar, strIntelliSense))
	{
		Sleep(100);
		m_nCurrOffset--;
		OnDelete(TRUE,TRUE);

		if (strIntelliSense.IsEmpty())
		{
			ASSERT(FALSE);
		}
		else
		{
			m_bIntelliSenseUpdate = TRUE;

			try
			{
				InsertText(strIntelliSense.GetBuffer(0), m_nCurrOffset, bRedraw);
			}
			catch (...)
			{
				ASSERT(FALSE);
			}

			m_bIntelliSenseUpdate = FALSE;
		}
	}
	
	return TRUE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::InsertText (LPCTSTR lpszText, int nInsertFrom, BOOL bRedraw, 
								BOOL bSuppressUndo, BOOL bUpdateLineData, 
								BOOL bForceNextUndo, BOOL bAlwaysAtEnd)
{
	ASSERT_VALID (this);
	ASSERT (lpszText != NULL);

	if (nInsertFrom >= 0)
	{
		nInsertFrom = min (nInsertFrom, m_strBuffer.GetLength ());
	}
	else
	{
		nInsertFrom = m_nCurrOffset;
	}

	CString strText = lpszText;
	
	if (ProcessTextBeforeInsert (strText))
	{
		int nInsertedTextLength = strText.GetLength ();
		int nCurrRowStartOffset = GetRowStartByOffset (m_nCurrOffset, FALSE);

		if (m_nCurrOffset - nCurrRowStartOffset + nInsertedTextLength >= m_nMaxScrollWidth && 
			m_nCurrOffset != 0 && !m_strBuffer.IsEmpty () && 
			m_nCurrOffset != nCurrRowStartOffset &&  
			m_nCurrOffset < m_strBuffer.GetLength () - 1  && 
			(m_strBuffer [m_nCurrOffset + 1] != g_chEOL || 
			 m_strBuffer [m_nCurrOffset] != g_chEOL) &&
			 !bAlwaysAtEnd)
		{
			OnFailedOperation (0x1000000 | 0x10000000);
			strText.Insert (0, g_chEOL);
		}

		if (m_nCurrOffset >= m_strBuffer.GetLength () - 1 && 
			m_nCurrOffset != nCurrRowStartOffset && !bAlwaysAtEnd)
		{
			OnFailedOperation (0x1000000 | 0x10000000);
			strText.Insert (0, g_chEOL);
		}			

		if (!OnBeforeTextInserted (strText, nInsertFrom))
		{
			return FALSE;
		}

		OnInsertTextToBuffer (nInsertFrom, strText, bUpdateLineData);

		m_strBuffer.Insert (nInsertFrom, strText);

		UpdateOffsetRelatedData (nInsertFrom, nInsertFrom + strText.GetLength ());
		OnUpdateAutoOutlining (nInsertFrom, strText.GetLength (), FALSE);
		
		int nRowEndOffset = GetRowEndByOffset (m_nCurrOffset + nInsertedTextLength, FALSE);
		int nRowStartOffset = GetRowStartByOffset (m_nCurrOffset + nInsertedTextLength, FALSE);
		int nRowLen = nRowEndOffset - nRowStartOffset;

		if (bSuppressUndo)
		{
			EmptyUndoList ();
		}
		else
		{
			AddUndoAction (strText, g_dwUATInsertData, nInsertFrom, TRUE, 
							bForceNextUndo || nRowLen >= m_nMaxScrollWidth);
		}
		
		int nExtraCharsInserted = 0;
		if (nRowLen >= m_nMaxScrollWidth)
		{
			if (InsertNewLine (FALSE, bForceNextUndo, 
								nRowStartOffset + m_nMaxScrollWidth - 1, FALSE))
			{
				nExtraCharsInserted = GetInsertNewLineString (
					nRowStartOffset + m_nMaxScrollWidth - 1).GetLength ();
			}
		}

		UpdateScrollBars ();
		if (OnUpdateLineNumbersMarginWidth ())
		{
			RecalcLayout ();
		}

		SetCaret (nInsertFrom + strText.GetLength () + nExtraCharsInserted, TRUE, bRedraw);

		if (bRedraw)
		{
			RedrawWindow ();
		}

		OnAfterTextChanged (nInsertFrom, strText, TRUE);

		return TRUE;
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::InsertTextAsBlock (LPCTSTR lpszText, int nInsertFrom, 
									   BOOL bRedraw, BOOL bSuppressUndo, 
									   BOOL bUpdateLineData, BOOL bForceNextUndo)
{
	if (nInsertFrom >= 0)
	{
		nInsertFrom = min (nInsertFrom, m_strBuffer.GetLength ());
	}
	else
	{
		nInsertFrom = m_nCurrOffset;
	}
	
	int nRowStartOffset = GetRowStartByOffset (nInsertFrom, TRUE);
	int nRowEndOffset   = GetRowEndByOffset (nInsertFrom, TRUE);

	if (nRowEndOffset - nRowStartOffset < 2)
	{
		return InsertText (lpszText, nInsertFrom, bRedraw, bSuppressUndo, bUpdateLineData, bForceNextUndo);
	}

	CString strText = lpszText;

	if (strText.IsEmpty ())
	{
		return TRUE;
	}

	int nStartColumn = GetColumnFromOffset (nInsertFrom, TRUE);
	bool bAddedLines = false;
	for (int nTextRowOffset = 0; nTextRowOffset < strText.GetLength ();)
	{
		int nNextRowOffset = strText.Find (g_chEOL, nTextRowOffset);
		if (nNextRowOffset == -1)
		{
			nNextRowOffset = strText.GetLength ();
		}

		CString strSubRow = strText.Mid (nTextRowOffset, nNextRowOffset - nTextRowOffset);

		if (nRowEndOffset >= m_strBuffer.GetLength ())
		{
			m_strBuffer += strSubRow;
			m_nTotalLines++;
			bAddedLines = true;
			continue;
		}

		strSubRow.Remove (g_chEOL);

		int nInsertOffset = GetOffsetOfColumnInRow (nStartColumn, nRowEndOffset, TRUE, TRUE);
		nRowEndOffset += strSubRow.GetLength () + 1;

		nInsertOffset = min (nRowEndOffset - 1, nInsertOffset);
		if (OnBeforeTextInserted (strSubRow, nInsertOffset))
		{
			m_strBuffer.Insert (nInsertOffset, strSubRow);
			UpdateOffsetRelatedData (nInsertOffset, nInsertOffset + strSubRow.GetLength ());
			OnUpdateAutoOutlining (nInsertFrom, strSubRow.GetLength (), FALSE);
			OnAfterTextChanged (nInsertOffset, strSubRow, TRUE);
		}

		nRowEndOffset = GetRowEndByOffset (nRowEndOffset, TRUE);

		nTextRowOffset = nNextRowOffset + 1;
	}

	if (bAddedLines)
	{
		UpdateScrollBars ();
		if (OnUpdateLineNumbersMarginWidth ())
		{
			// Should update all margin-related things	
			RecalcLayout ();
			SetCaret (m_nCurrOffset, TRUE, FALSE);
			bRedraw = TRUE;
		}
	}
	
	if (bRedraw)
	{
		RedrawWindow ();
	}

	return TRUE;
}								
//**************************************************************************************
BOOL CBCGPEditCtrl::ProcessTextBeforeInsert (CString& strText)
{
	strText.Remove (_T('\r'));

	if (!m_bKeepTabs)
	{
		CString strTab (_T (' '), m_nTabSize);
		strText.Replace (_T("\t"), strTab);
	}

	// truncate strings that too long
	for (int i = 0, nCount = 0; i < strText.GetLength (); i++)
	{
		strText [i] == g_chEOL ? nCount = 0 : nCount++;

		if (nCount == m_nMaxScrollWidth)
		{
			strText.Insert (i, g_chEOL); 
			nCount = 0;
		}
	}

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::InsertNewLine (BOOL bRedraw, BOOL bForceNextUndo, int  nOffset, 
								   BOOL bSetCaret)
{
	if (nOffset == -1)
	{
		nOffset = m_nCurrOffset;
	}

	int nRowNum = RowFromOffset (nOffset);
	int nRowStartOffset = GetRowStartByOffset (nOffset, TRUE);
	BOOL bLineStart = m_nCurrOffset == nRowStartOffset;

	if (!OnInsertNewLine (bForceNextUndo, nOffset, bSetCaret))
	{
		return FALSE;
	}
		
	if (bLineStart)
	{
		// the new line is inserted at the beginning of the current line
		// the current line with all markesr should be moved down
		UpdateLineRelatedData (nRowNum, 1);
	}
	else
	{
		// the new line is inserted at the middle of the current line
		// the rest of the current line with all markers starting
		// from the next line should be moved down
		UpdateLineRelatedData (nRowNum + 1, 1);
	}

	UpdateScrollBars ();
	BOOL bNeedUpdate = OnUpdateLineNumbersMarginWidth ();

	m_bIsModified = TRUE;

	if (bNeedUpdate)
	{
		// Should update all margin-related things	
		RecalcLayout ();
		SetCaret (m_nCurrOffset, TRUE, FALSE);
	}
	if (bRedraw || bNeedUpdate)
	{
		RedrawWindow ();
	}

	return TRUE;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::OnInsertNewLine (BOOL bForceNextUndo, int nOffset, 
									 BOOL bSetCaret)
{
	if (nOffset == -1)
	{
		nOffset = m_nCurrOffset;
	}
	
	CString strToInsert = GetInsertNewLineString (nOffset);

	if (!OnBeforeTextInserted (strToInsert, nOffset))
	{
		return FALSE;
	}
	
	AddUndoAction (strToInsert, g_dwUATInsertData, nOffset, FALSE, bForceNextUndo);
	m_strBuffer.Insert (nOffset, strToInsert);

	UpdateOffsetRelatedData (nOffset, nOffset + strToInsert.GetLength ());
	OnUpdateAutoOutlining (nOffset, strToInsert.GetLength (), FALSE);
	OnAfterTextChanged (nOffset, strToInsert, TRUE);

	if (bSetCaret)
	{
		if (m_bDefaultIndent) 
		{
			SetCaret (nOffset + strToInsert.GetLength ());
		}
		else
		{
			Down ();
			Home ();
		}
	}
	
	m_nTotalLines++;

	return TRUE;
}
//***************************************************************************************
CString CBCGPEditCtrl::GetInsertNewLineString (int nOffset)
{
	if (nOffset == -1)
	{
		nOffset = m_nCurrOffset;
	}

	int nCurrRowStart = GetRowStartByOffset (nOffset, TRUE);
	int nCurrRowTextStart = GetRowTextStart (nCurrRowStart, TRUE);

	CString strToInsert (g_chEOL);

	if (m_bDefaultIndent && nCurrRowTextStart > nCurrRowStart)
	{
		int nCount = min (nOffset - nCurrRowStart, nCurrRowTextStart - nCurrRowStart);
		CString strRowStart = m_strBuffer.Mid (nCurrRowStart, nCount);
		strToInsert += strRowStart;  
	}

    return strToInsert;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::InsertTab (BOOL bRedraw)
{
	int nInsertOffset = m_nCurrOffset;

	const int nRowEndOffset = GetRowEndByOffset (m_nCurrOffset, TRUE);
	const int nRowStartOffset = GetRowStartByOffset (m_nCurrOffset, TRUE);
	const int nRowLen = nRowEndOffset - nRowStartOffset;

	CString strTab;
	BuildTabString (strTab, nInsertOffset - nRowStartOffset);
	const int nIndentSize = strTab.GetLength ();

	if (nRowLen + nIndentSize > m_nMaxScrollWidth - 1)
	{
		OnFailedOperation (g_dwOpInsTab | g_dwOpReasonLength);
		return FALSE;
	}

	if (GetOverrideMode ())
	{
		int nColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);
		nColumn += nIndentSize;
		int nNewOffset = GetOffsetOfColumnInRow (nColumn, m_nCurrOffset, FALSE, TRUE);
		SetCaret (nNewOffset);
		return TRUE;
	}

	if (!OnBeforeTextInserted (strTab, m_nCurrOffset))
	{
		return FALSE;
	}
	int nSaveOffset = m_nCurrOffset;
	m_strBuffer.Insert (m_nCurrOffset, strTab);
	for (int i = 0; i < strTab.GetLength (); i++)
	{
		UpdateScrollBars ();
		Right ();
	}
	UpdateOffsetRelatedData (nSaveOffset, nSaveOffset + strTab.GetLength ());
	OnUpdateAutoOutlining (nSaveOffset, strTab.GetLength (), FALSE);
	OnAfterTextChanged (nSaveOffset, strTab, TRUE);

	AddUndoAction (strTab, g_dwUATInsertData, nInsertOffset, FALSE);

	if (bRedraw)
	{
		RedrawRestOfLine (m_nCurrOffset);
	}

	return TRUE;
}
//**************************************************************************************
void CBCGPEditCtrl::RedrawText (int nStartRow, int nEndRow/* = -1*/)
{
	ASSERT_VALID (this);

	CRect rectArea;
	GetClientRect (rectArea);

	if (nEndRow != -1)
	{
		int nStartRowVirt = GetVirtualRow (nStartRow);
		rectArea.top = (nStartRowVirt - m_nScrollOffsetVert) * m_nLineHeight;

		switch (nEndRow)
		{
		case 0:	// one line
			rectArea.bottom = rectArea.top + m_nLineHeight;
			break;

		case -1: // rest of text
			break;

		default:
			ASSERT (nEndRow >= nStartRow);

			rectArea.bottom = min (rectArea.bottom,
				(rectArea.top + m_nLineHeight) * (nEndRow - nStartRow + 1));
		}
	}

	RedrawWindow (rectArea);
}
//**************************************************************************************
void CBCGPEditCtrl::RedrawTextOffsets (int nStartOffset, int nEndOffset)
{
	int nStart = min (nStartOffset, nEndOffset);
	int nEnd = max (nStartOffset, nEndOffset);

	CPoint ptStart, ptEnd;
	
	OffsetToPoint (nStart, ptStart);
	ptStart.x = 0;

	if (nEnd >= m_strBuffer.GetLength () - 1)
	{
		CRect rectArea;
		GetClientRect (rectArea);
		ptEnd = rectArea.BottomRight ();
	}
	else
	{
		OffsetToPoint (nEnd, ptEnd);
		ptEnd.y += m_nLineHeight;
		ptEnd.x = m_rectText.right;
	}

	CRect rect (ptStart, ptEnd);
	
	RedrawWindow (rect);
}
//***************************************************************************************
void CBCGPEditCtrl::OnFillBackground (CDC* pDC)
{
	ASSERT_VALID (pDC);

	CRect rectClient;
	GetClientRect (rectClient);

	//----------------
	// Fill text area:
	//----------------
	if (m_clrBack == -1)
	{
#ifndef _BCGPEDIT_STANDALONE
		::FillRect (pDC->GetSafeHdc (), &rectClient, CBCGPVisualManager::GetInstance()->GetEditBackgroundBrush(this));
#else
		::FillRect (pDC->GetSafeHdc (), &rectClient, ::GetSysColorBrush (COLOR_WINDOW));
#endif
	}
	else
	{
		CBrush br (m_clrBack);
		pDC->FillRect (&rectClient, &br);
	}

	//------------------------
	// Redraw sizing box area:
	//------------------------
	if (m_wndScrollVert.GetSafeHwnd () != NULL && m_wndScrollHorz.GetSafeHwnd () != NULL)
	{
		CRect rectBox = rectClient;
	
		rectBox.left = m_rectView.right;
		rectBox.top = m_rectView.bottom;

#if (!defined _BCGPEDIT_STANDALONE) && (!defined _BCGSUITE_)
		CBCGPVisualManager::GetInstance()->OnDrawEditCtrlResizeBox (pDC, this, rectBox);
#else
		COLORREF clrBkOld = pDC->GetBkColor ();
		pDC->FillSolidRect (rectBox, ::GetSysColor(COLOR_BTNFACE));
		pDC->SetBkColor (clrBkOld);
#endif
	}

	COLORREF clrBack = GetDefaultBackColor();

	double H, L, S;
	CBCGPDrawManager::RGBtoHSL(clrBack, &H, &S, &L);

	m_bIsDarkBackground = L < 0.7;
}
//***************************************************************************************
void CBCGPEditCtrl::OnDrawSideBar (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	CRect rectBar;
	GetClientRect (rectBar);
	rectBar.left = rect.left;
	rectBar.right = rect.right;

#ifndef _BCGPEDIT_STANDALONE
	COLORREF clrSideBar = CBCGPVisualManager::GetInstance()->GetEditBackSidebarColor(this);
	if (clrSideBar == (COLORREF)-1)
	{
		clrSideBar = m_clrBackSidebar;
	}
#else
	COLORREF clrSideBar = m_clrBackSidebar;
#endif

	pDC->FillSolidRect (&rectBar, clrSideBar);

	if (m_lstMarkers.IsEmpty ())
	{
		return;
	}

	// determine start and end offsets of the visible part of buffer
	CPoint ptTopLeft (m_rectText.left, m_rectText.top);
	CPoint ptBottomRight (m_rectText.right - 1, m_rectText.bottom - 1);

	int nStartOffset = HitTest (ptTopLeft);
	int nEndOffset = HitTest (ptBottomRight);

	if (nStartOffset == -1)
	{
		nStartOffset = 0;
	}

	if (nEndOffset == -1)
	{
		nEndOffset = m_strBuffer.GetLength () - 1;
	}

	nEndOffset = min (nEndOffset, m_strBuffer.GetLength ());

	int nRowColumnTop = RowFromOffset (nStartOffset);
	int nRowColumnBottom = RowFromOffset (nEndOffset);

	// Draw markers by groups with the same m_unPriority:
	UINT unPriority = 0;
	if (!m_lstMarkers.IsEmpty ())
	{
		CBCGPEditMarker* pMarker = DYNAMIC_DOWNCAST(CBCGPEditMarker, m_lstMarkers.GetTail ());
		ASSERT_VALID (pMarker);
		unPriority = pMarker->m_unPriority;
	}
	POSITION posFirst = NULL;
	int nCount = 1;
	
	for (POSITION pos = m_lstMarkers.GetTailPosition (); pos != NULL; )
	{
		POSITION posSave = pos;
		CBCGPEditMarker* pMarker = DYNAMIC_DOWNCAST (CBCGPEditMarker, m_lstMarkers.GetPrev (pos));
		ASSERT_VALID (pMarker);
		if (pMarker->m_unPriority == unPriority)
		{
			posFirst = posSave;
			nCount++;
		}
		else if (pMarker->m_unPriority > unPriority)
		{
			// Draw markers:
			int nCnt = 0;
			for (POSITION posInner = posFirst; posInner != NULL && nCnt < nCount; nCnt++)
			{
				CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker,
												m_lstMarkers.GetNext (posInner));
				ASSERT_VALID (pMarkerNext);
				if (pMarkerNext->m_nLine >= nRowColumnTop && 
					pMarkerNext->m_nLine <= nRowColumnBottom + 1)
				{
					CRect rectMarker (rect);
					rectMarker.top = (GetVirtualRow (pMarkerNext->m_nLine) - m_nScrollOffsetVert) * m_nLineHeight;
					rectMarker.bottom = rectMarker.top + m_nLineHeight;
					OnDrawMarker (pDC, rectMarker, pMarkerNext);
				}
			}

			unPriority = pMarker->m_unPriority;
			posFirst = posSave;
			nCount = 1;
		}

		if (pos == NULL)
		{
			// Draw markers:
			int nCnt = 0;
			for (POSITION posInner = posFirst; posInner != NULL && nCnt < nCount; nCnt++)
			{
				CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker,
												m_lstMarkers.GetNext (posInner));
				ASSERT_VALID (pMarkerNext);
				if (pMarkerNext->m_nLine >= nRowColumnTop && 
					pMarkerNext->m_nLine <= nRowColumnBottom + 1)
				{
					CRect rectMarker (rect);
					rectMarker.top = (GetVirtualRow (pMarkerNext->m_nLine) - m_nScrollOffsetVert) * m_nLineHeight;
					rectMarker.bottom = rectMarker.top + m_nLineHeight;
					OnDrawMarker (pDC, rectMarker, pMarkerNext);
				}
			}
		}
	}

}
//***************************************************************************************
void CBCGPEditCtrl::OnDrawLineNumbersBar (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

#ifndef _BCGPEDIT_STANDALONE
	COLORREF clrBack = CBCGPVisualManager::GetInstance()->GetEditLineNumbersBarBackColor(this);
	if (clrBack == (COLORREF)-1)
	{
		clrBack = (m_clrBackLineNumber == (COLORREF)-1) ? GetDefaultBackColor() : m_clrBackLineNumber;
	}

	COLORREF clrText = CBCGPVisualManager::GetInstance()->GetEditLineNumbersBarTextColor(this);
	if (clrText == (COLORREF)-1)
	{
		clrText = m_clrTextLineNumber;
	}
#else
	COLORREF clrBack = m_clrBackLineNumber;
	COLORREF clrText = m_clrTextLineNumber;
#endif

	pDC->FillSolidRect (rect, clrBack);

	// Draw dividing line:
	BOOL bDrawLine = TRUE;

#ifndef _BCGPEDIT_STANDALONE
	bDrawLine = CBCGPVisualManager::GetInstance()->IsDrawEditLineNumbersRightSideLine(this);
#endif

	if (bDrawLine)
	{
		rect.DeflateRect (0, 0, 1, 0);

		WORD wHatchBits1 [8] = { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00 };
		WORD wHatchBits2 [8] = { 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF };

		CBitmap bmpPattern;
		bmpPattern.CreateBitmap (8, 8, 1, 1, 
			(rect.top % 2) ? wHatchBits1 : wHatchBits2);

		CBrush br;
		br.CreatePatternBrush (&bmpPattern);

		CRect rectLine = rect;
		rectLine.left = rectLine.right - 1;

		pDC->FillRect (rectLine, &br);
		
		rect.DeflateRect (0, 0, 1, 0);
	}

	// Determine start and end offsets of the visible part of buffer
	CPoint ptTopLeft (m_rectText.left + 1, m_rectText.top);
	CPoint ptBottomRight (m_rectText.right - 1, m_rectText.bottom - 1);

	int nStartOffset = HitTest (ptTopLeft);
	int nEndOffset = HitTest (ptBottomRight);

	if (nStartOffset == -1)
	{
		nStartOffset = 0;
	}

	if (nEndOffset == -1)
	{
		nEndOffset = m_strBuffer.GetLength () - 1;
	}

	nEndOffset = min (nEndOffset, m_strBuffer.GetLength ());

	int nRowColumnTop = RowFromOffset (nStartOffset);
	int nRowColumnBottom = RowFromOffset (nEndOffset);

	// Draw line numbers:
	int nRow = nRowColumnTop;
	int nVirtualRow = GetVirtualRow (nRow);
	int nRowStartOffset = nStartOffset;

	int nOldMode = pDC->SetBkMode (TRANSPARENT);
	COLORREF clrTextOld = pDC->SetTextColor (clrText);

	while (nRow <= nRowColumnBottom && nRowStartOffset >= 0)
	{
		CRect rect (rect);
		rect.top = (nVirtualRow - m_nScrollOffsetVert) * m_nLineHeight;
		rect.bottom = rect.top + m_nLineHeight;
		OnDrawLineNumber (pDC, rect, nRow);

		NextRow (nRow, nVirtualRow, nRowStartOffset);
	}

	pDC->SetBkMode (nOldMode);
	pDC->SetTextColor (clrTextOld);
}
//************************************************************************************
void CBCGPEditCtrl::OnDrawLineNumber (CDC* pDC, CRect rect, const int nLineNumber)
{
	ASSERT_VALID (pDC);

	CString str;
	str.Format (_T("%d"), nLineNumber + 1);
	CSize size = GetStringExtent (pDC, str, (int) _tcslen (str));

	int nVertSpacing = m_nLineVertSpacing % 2; 
	rect.DeflateRect (0, m_nLineVertSpacing / 2 + nVertSpacing, 0, m_nLineVertSpacing / 2 + nVertSpacing);
	
	if (size.cx <= rect.Width ())
	{
		pDC->DrawText (str, rect, DT_NOCLIP | DT_SINGLELINE | DT_NOPREFIX | DT_RIGHT);
	}
}
//**************************************************************************************
void CBCGPEditCtrl::OnDrawOutlineBar (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);
	ASSERT (m_bEnableOutlineMargin);
	
	OnFillOutlineArea (pDC, rect);

	if (!m_bEnableOutlineMargin || !m_bEnableOutlining || 
		m_OutlineNodes.GetNodes ()->IsEmpty ())
	{
		return;
	}

	// Determine start and end offsets of the visible part of buffer
	CPoint ptTopLeft (m_rectText.left + 1, m_rectText.top);
	CPoint ptBottomRight (m_rectText.right - 1, m_rectText.bottom - 1);

	int nStartOffset = HitTest (ptTopLeft);
	int nEndOffset = HitTest (ptBottomRight);

	int nStartRow = RowFromOffset (nStartOffset, FALSE, TRUE);
	int nEndRow = RowFromOffset (nEndOffset, FALSE, TRUE);

	CObList lstOutlineBlocks;
	m_OutlineNodes.GetBlocksInRange (nStartOffset, nEndOffset, lstOutlineBlocks, FALSE);
	CMap <int, int, CBCGPOutlineBaseNode*, CBCGPOutlineBaseNode*> mapBlocksByRowEnd;
	
	// ------------------------------
	// Draw outline markers by lines:
	// ------------------------------
	POSITION pos = lstOutlineBlocks.GetHeadPosition ();
	int nOffset = nStartOffset;
	for (int nRow = nStartRow; nRow <= nEndRow && nOffset < m_strBuffer.GetLength (); nRow++)
	{
		int nRowStartOffset = nOffset;
		if (nRowStartOffset == -1)
		{
			break;
		}
		int nRowEndOffset = GetRowEndByOffset (nRowStartOffset, TRUE);

		// --------------------------------
		// Get outline block for this line:
		// --------------------------------
		CBCGPOutlineBaseNode* pRowOutlineBlock = NULL;

		if (m_pOutlineNodeCurr != NULL)
		{
			ASSERT_VALID (m_pOutlineNodeCurr);

			int nCurrNodeStart = m_pOutlineNodeCurr->m_nStart;
			if (nCurrNodeStart >= nRowStartOffset && 
				nCurrNodeStart <= nRowEndOffset)
			{
				pRowOutlineBlock = m_pOutlineNodeCurr;
			}
		}

		BOOL bInsideOpenBlockAtStart = FALSE;
		BOOL bInsideOpenBlockAtEnd = FALSE;
		BOOL bEndOfBlock = FALSE;

		if (!mapBlocksByRowEnd.IsEmpty ())
		{
			bInsideOpenBlockAtStart = TRUE;
		}

		for (; pos != NULL; lstOutlineBlocks.GetNext (pos))
		{
			CBCGPOutlineBaseNode* pOutlineBlock = (CBCGPOutlineBaseNode*) lstOutlineBlocks.GetAt (pos);
			ASSERT_VALID (pOutlineBlock);

			if ((pOutlineBlock->m_dwFlags & g_dwOBFComplete) == 0)
			{
				continue;
			}

			int nBlockStart = pOutlineBlock->m_nStart;
			if (nBlockStart > nRowEndOffset)
			{
				break;	// next row
			}
			
			if (pRowOutlineBlock == NULL &&
				nBlockStart >= nStartOffset)
			{
				pRowOutlineBlock = pOutlineBlock;
			}

			if (nBlockStart < nRowStartOffset)
			{
				bInsideOpenBlockAtStart = TRUE;
			}

			if (pOutlineBlock->m_nEnd >= nRowStartOffset &&
				pOutlineBlock->m_nEnd > nRowEndOffset)
			{
				int nRestOfLines = GetNumOfCharsInText(nRowEndOffset, min (nEndOffset, pOutlineBlock->m_nEnd),
													   g_chEOL, TRUE);
				mapBlocksByRowEnd.SetAt (nRow + nRestOfLines, pOutlineBlock);
				bInsideOpenBlockAtEnd = TRUE;
			}
			else
			{
				bEndOfBlock = TRUE;
			}
		}

		CBCGPOutlineBaseNode* pTmpBlock = NULL;
		if (mapBlocksByRowEnd.Lookup(nRow, pTmpBlock))
		{
			bEndOfBlock = TRUE;
		}
		
		while (mapBlocksByRowEnd.RemoveKey (nRow));
		
		if (!mapBlocksByRowEnd.IsEmpty ())
		{
			bInsideOpenBlockAtEnd = TRUE;
		}

		// ----------------------------------
		// Draw outline button for this line:
		// ----------------------------------
		CRect rectButton (rect);
		rectButton.top = (nRow - m_nScrollOffsetVert) * m_nLineHeight;
		rectButton.bottom = rectButton.top + m_nLineHeight;

		OnDrawOutlineButton (pDC, rectButton, pRowOutlineBlock, 
							 bInsideOpenBlockAtStart, bInsideOpenBlockAtEnd, bEndOfBlock);

		// Next row:
		nOffset = nRowEndOffset + 1;
	}
}
//**************************************************************************************
void CBCGPEditCtrl::OnFillOutlineArea (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);
	ASSERT (m_bEnableOutlineMargin);
	
	pDC->FillSolidRect (rect, GetDefaultBackColor ());
}
//**************************************************************************************
BOOL CBCGPEditCtrl::OnDelete (BOOL bRedraw, BOOL bForceNextUndo)
{
	if (DeleteSelectedText ())
	{
		m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);
		return TRUE;
	}

	int nBufferLen = m_strBuffer.GetLength ();
	BOOL bDelNewLine = (m_nCurrOffset >= 0 && m_nCurrOffset <= nBufferLen - 1
						&& m_strBuffer [m_nCurrOffset] == _T('\n'));

	if (bDelNewLine)
	{
		int nRowEndOffset;
		int nCurLineLen = GetNumOfColumnsInRowByOffset (m_nCurrOffset, nRowEndOffset, FALSE);
		int nNextLineLen = GetNumOfColumnsInRowByOffset (nRowEndOffset + 1, nRowEndOffset, FALSE);

		if (nCurLineLen + nNextLineLen > m_nMaxScrollWidth - 2)
		{
			OnFailedOperation (g_dwOpLineConcat | g_dwOpReasonLength);
			return FALSE;
		}
	}
	
	CString strDeletedText = m_strBuffer.Mid (m_nCurrOffset, 1);

	if (!OnBeforeTextDeleted (m_nCurrOffset, strDeletedText))
	{
		return FALSE;
	}

	if (strDeletedText.IsEmpty ())
	{
		OnFailedOperation (g_dwOpDelete | g_dwOpReasonBufEmpty);
		return FALSE;
	}
	AddUndoAction (strDeletedText, g_dwUATDeleteData, m_nCurrOffset, FALSE, bForceNextUndo);

	BOOL bWasAtColorBlock = IsOffsetAtColorBlock (m_nCurrOffset);

	if (bDelNewLine)
	{
		OnDeleteTextFromBuffer (m_nCurrOffset, m_nCurrOffset + 1, strDeletedText);
	}

	RemoveOffsetDataInRange (m_nCurrOffset, m_nCurrOffset + 1);
	m_strBuffer.Delete (m_nCurrOffset, 1);
	OnUpdateAutoOutlining (m_nCurrOffset, 0, FALSE);
	OnAfterTextChanged (m_nCurrOffset, strDeletedText, FALSE);

	if (bDelNewLine)
	{
		UpdateScrollBars ();
		if (OnUpdateLineNumbersMarginWidth ())
		{
			// Should update all margin-related things	
			RecalcLayout ();
			SetCaret (m_nCurrOffset, TRUE, FALSE);
			RedrawWindow ();
			bRedraw = FALSE;
		}
	}		

	if (bRedraw)
	{
		if (IsOffsetAtColorBlock (m_nCurrOffset) || bWasAtColorBlock)
		{
			RedrawWindow ();
		}
		else if (bDelNewLine)
		{
			RedrawRestOfText (m_nCurrOffset);
		}
		else
		{
			RedrawRestOfLine (m_nCurrOffset);
		}
	}

	m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);

	return TRUE;
}
//**************************************************************************************
void CBCGPEditCtrl::RedrawRestOfLine (int nOffset)
{
	if (nOffset >= 0)
	{
		CPoint pt;
		if (!OffsetToPoint (nOffset, pt))
		{
			ASSERT (FALSE);
		}
		else
		{
			CRect rect (m_rectText.left, pt.y, m_rectText.right, pt.y + m_nLineHeight);
			rect.InflateRect (1, 1);

			InvalidateRect (rect);
			UpdateWindow ();
		}
	}
}
//**************************************************************************************
void CBCGPEditCtrl::RedrawRestOfText (int nOffset)
{
	if (nOffset >= 0)
	{
		CPoint pt;
		if (!OffsetToPoint (nOffset, pt))
		{
			ASSERT (FALSE);
		}
		else
		{
			CRect rect = m_rectText;
			rect.left = 0;
			rect.top = pt.y - m_nScrollOffsetVert * m_nLineHeight;

			InvalidateRect (rect);
			UpdateWindow ();
		}
	}
}
//**************************************************************************************
void CBCGPEditCtrl::OnDrawText (CDC* pDC)
{
	ASSERT_VALID (pDC);

	if (m_strBuffer.IsEmpty ())
	{
		return;
	}

	BOOL bIsPrint = pDC->IsPrinting ();

	//-------------------
	// Get text rectangle
	//-------------------
	CRect rectText;

	if (bIsPrint)
	{
		rectText = m_rectText;
	}
	else
	{
		CRect rectClip; 
		GetClipBox (pDC->m_hDC, rectClip);

		rectText.IntersectRect (rectClip, m_rectText);
		rectText.left = m_rectText.left + 1;
		rectText.right = m_rectText.right;
	}

	if (rectText.IsRectEmpty ())
	{
		return;
	}

	rectText.InflateRect (1, 1);

	//------------------
	// Get row rectangle
	//------------------
	CRect rectRow = rectText;

	int nVertOffset = m_nScrollOffsetVert * m_nLineHeight;

	rectRow.OffsetRect (0, -nVertOffset);

	rectRow.DeflateRect (0, 1);

	rectRow.bottom = rectRow.top + m_nLineHeight;

//////////////////////////////////////////////////////////////////////////

	// determine start and end offsets of the visible part of buffer

	CPoint ptTopLeft (rectText.left, rectText.top);
	CPoint ptBottomRight (rectText.right - 1, rectText.bottom - 1);

	int nStartOffset = m_nTopVisibleOffset; 
	int nEndOffset = bIsPrint ? m_nBottomVisibleOffset : HitTest (ptBottomRight, FALSE, TRUE);

	if (nStartOffset == -1)
	{
		nStartOffset = 0;
	}

	if (nEndOffset == -1)
	{
		nEndOffset = m_strBuffer.GetLength () - 1;
	}

	nEndOffset = min (nEndOffset, m_strBuffer.GetLength () - 1);

	nStartOffset = GetRowStartByOffset (nStartOffset, TRUE);
	nEndOffset = GetRowEndByOffset (nEndOffset, TRUE);

	nEndOffset = min (nEndOffset, m_strBuffer.GetLength () - 1);

	rectRow.left = rectText.left - m_nScrollOffsetHorz * m_nMaxCharWidth + 1;

	rectRow.top = bIsPrint ? rectText.top : 0;
	rectRow.bottom = rectRow.top + m_nLineHeight;
	int nTabOrigin = rectRow.left;

	//-------------------
	// Set default colors
	//-------------------
	CString strWord; 

	int nOldBkMode = pDC->SetBkMode (OPAQUE);

	COLORREF clrDefaultText = GetDefaultTextColor ();
	COLORREF clrDefaultBack = GetDefaultBackColor ();

	COLORREF clrNextWordFore	= clrDefaultText;
	COLORREF clrNextWordBk		= clrDefaultBack;

	CString strOutString;
	COLORREF clrOutText = clrDefaultText;
	COLORREF clrOutBack = clrDefaultBack;

	BOOL bIsFocused = GetFocus () == this;

	COLORREF clrBkSel = bIsFocused ? m_clrBackSelActive : m_clrBackSelInActive;
	COLORREF clrTextSel = bIsFocused ? m_clrTextSelActive : m_clrTextSelInActive;

#ifndef _BCGPEDIT_STANDALONE
	if (clrBkSel == (COLORREF)-1)
	{
		clrBkSel = CBCGPVisualManager::GetInstance()->GetEditCtrlSelectionBkColor(this, bIsFocused);
	}

	if (clrTextSel == (COLORREF)-1)
	{
		clrTextSel = CBCGPVisualManager::GetInstance()->GetEditCtrlSelectionTextColor(this, bIsFocused);
	}
#endif

	CRect rectSel;

	// ----------------------------------------------
	// Build color areas for visible part of a buffer (comments, strings and so on):
	// ----------------------------------------------

	BCGP_EDIT_COLOR_BLOCK	clrBlock; 
	BOOL bIsOpenBlock = FindOpenBlock (m_nTopVisibleOffset, &clrBlock);

	int nCloseBlockOffset = -1;
	if (bIsOpenBlock)
	{
		nCloseBlockOffset = FindCloseBlock (m_nTopVisibleOffset, &clrBlock);
	}

	COLORREF clrBlockFore = (clrBlock.GetForegroundColor(m_bIsDarkBackground) == -1) ? GetDefaultTextColor () : clrBlock.GetForegroundColor(m_bIsDarkBackground);
	COLORREF clrBlockBack = (clrBlock.m_clrBackground == -1) ? GetDefaultBackColor () : clrBlock.m_clrBackground; 

	CList <BCGP_EDIT_COLOR_AREA, BCGP_EDIT_COLOR_AREA&> colorAreas;

	if (bIsOpenBlock) 
	{
		if (nCloseBlockOffset < nEndOffset)
		{
			BuildColorAreas (colorAreas, nCloseBlockOffset, nEndOffset, NULL, FALSE);
		}
	}
	else
	{
		BuildColorAreas (colorAreas, nStartOffset, nEndOffset, NULL, FALSE);
	}

	// ---------------------------------------------------------------
	// Fill background for selected block (for m_bBlockSelectionMode):
	// ---------------------------------------------------------------

	if (!bIsPrint && m_bBlockSelectionMode && m_iStartSel != -1)
	{
		CPoint ptStart;
		CPoint ptEnd;

		ptStart.x = m_ptStartBlockSel.x - m_nScrollOffsetHorz * m_nMaxCharWidth;
		ptStart.y = m_ptStartBlockSel.y - m_nScrollOffsetVert * m_nLineHeight;

		ptEnd.x = m_ptEndBlockSel.x - m_nScrollOffsetHorz * m_nMaxCharWidth;
		ptEnd.y = m_ptEndBlockSel.y - m_nScrollOffsetVert * m_nLineHeight;

		if (ptEnd.y <= ptStart.y)
		{
			int n = ptEnd.y;
			ptEnd.y = ptStart.y;
			ptStart.y = n;

			ptEnd.y += m_nLineHeight; 
			ptStart.y -= m_nLineHeight;
		}

		rectSel.SetRect (ptStart, ptEnd);
		rectSel.NormalizeRect ();

		pDC->FillSolidRect (rectSel, clrBkSel);
	}

	// -----------------------
	// Prepare temporary data:
	// -----------------------
	CPoint ptCharOffset (rectRow.TopLeft ());

	TCHAR* lpszOutBuffer = (TCHAR*) alloca (sizeof (TCHAR) * (m_nMaxScrollWidth + 1));
	memset (lpszOutBuffer, 0, sizeof (TCHAR) * (m_nMaxScrollWidth + 1));
	int iIdx = 0;

	int nNextDelimiter = -1;
	int nCurrRow = m_nScrollOffsetVert;
	int nCurrRowAbsolute = m_nScrollOffsetVert + m_nScrollHiddenVert;
	int nCurrColumn = 0;
	BOOL bColoredLine = FALSE;

	// temporary data for hyperlink support:
	BOOL bHyperlink = FALSE;
	BOOL bHyperlinkOut = FALSE;
	int nHyperlinkStart = -1;
	int nHyperlinkEnd = -1;
	int nCurrHyperlinkHotOffset = (m_bEnableHyperlinkSupport && (m_nCurrHyperlinkHot != -1)) ?
		m_arrHyperlinks [m_nCurrHyperlinkHot].m_nHyperlinkOffset : -1;
	
	ClearHyperlinks ();
	m_nCurrHyperlink = -1;
	m_nCurrHyperlinkHot = -1;
	
	// temporary data for outlining support:
	CObList lstCollapsedOutlines;
	if (m_bEnableOutlining)
	{
		m_OutlineNodes.GetBlocksByStateInRange (nStartOffset, nEndOffset, 
			lstCollapsedOutlines, TRUE, FALSE);
	}
	POSITION posCollapsedOutline = lstCollapsedOutlines.GetHeadPosition ();

	// ---------------------------------------------
	// Draw the text for the visible part of buffer:
	// ---------------------------------------------
	int i = 0;
	for (i = nStartOffset; i <= nEndOffset; i++)
	{
		COLORREF clrForeground = clrDefaultText;
		COLORREF clrBackground = clrDefaultBack;
		bColoredLine = FALSE;
		
		TCHAR chNext = m_strBuffer [i];

		BCGP_EDIT_SYM_DEF symDef;
		int nSymExtraLen = 0;
		BOOL bDrawSymbol = m_bEnableSymSupport ? 
								LookUpSymbol (m_strBuffer, i, nEndOffset + 1, chNext, symDef, nSymExtraLen) : FALSE;
		
		// ------------------
		// Search hyperlinks:
		// ------------------
		bHyperlink = FALSE;
		if (m_bEnableHyperlinkSupport)
		{
			if (i >= nHyperlinkEnd)
			{
				// Find the possible start offset - skip right till the first selectable char:
				nHyperlinkStart = i;
				while (nHyperlinkStart < nEndOffset && 
					   m_strNonSelectableChars.Find(m_strBuffer.GetAt (nHyperlinkStart)) != -1)
				{
					nHyperlinkStart++;
				}

				// Find the possible end offset:
				nHyperlinkEnd = nHyperlinkStart;
				int j = 0;

				for (j = nHyperlinkStart; 
					 j <= nEndOffset && m_strNonSelectableChars.Find(m_strBuffer.GetAt (j)) == -1;
					 j++);
				nHyperlinkEnd = j;

				if (nHyperlinkStart < nHyperlinkEnd)
				{
					CString strHyperlink;
					int nHyperlinkOffset = -1;
					if (FindHyperlinkString (nHyperlinkStart, nHyperlinkEnd, strHyperlink, nHyperlinkOffset))
					{
						ASSERT (nHyperlinkOffset >= nHyperlinkStart);
						ASSERT (nHyperlinkOffset < nHyperlinkEnd);
						ASSERT (strHyperlink.GetLength () <= nHyperlinkEnd - nHyperlinkStart);

						AddHyperlink (strHyperlink, nHyperlinkOffset);
						nHyperlinkStart = nHyperlinkOffset;
						nHyperlinkEnd = nHyperlinkOffset + strHyperlink.GetLength ();
						bHyperlink = (i >= nHyperlinkStart);
					}
					else
					{
						nHyperlinkStart = nHyperlinkEnd;
					}
				}
			}
			else if (i >= nHyperlinkStart)
			{
				bHyperlink = TRUE;
			}
		}
		if (i == nStartOffset)
		{
			bHyperlinkOut = bHyperlink;
		}

		CBCGPOutlineBaseNode* pHiddenText = NULL;
		for (POSITION posNode = posCollapsedOutline; posNode != NULL; 
			 lstCollapsedOutlines.GetNext(posNode))
		{
			CBCGPOutlineBaseNode* pNode = (CBCGPOutlineBaseNode*) lstCollapsedOutlines.GetAt (posNode);
			ASSERT_VALID (pNode);

			if (pNode->m_nStart <= i &&
				pNode->m_nEnd >= i)
			{
				pHiddenText = pNode;
				posCollapsedOutline = posNode;
				break;
			}
		}

		// ---------------------
		// Calculate char width:
		// ---------------------

		int nCharWidth = 0;
		if (pHiddenText != NULL)
		{
			ASSERT_VALID (pHiddenText);
			OnCalcOutlineSymbol (pDC, ptCharOffset, pHiddenText);
			nCharWidth = pHiddenText->m_rectTool.Width ();
		}
		else if (bDrawSymbol)
		{
			IMAGEINFO imgInfo;
			m_pSymbolsImgList->GetImageInfo (symDef.m_nIndex, &imgInfo);
			nCharWidth = imgInfo.rcImage.right - imgInfo.rcImage.left;
		} 
		else if (!m_mapCharWidth.Lookup (chNext, nCharWidth))
		{
			nCharWidth = GetStringExtent (pDC, CString (chNext), 1).cx;
			if (nCharWidth == 0 && chNext == g_chEOL)
			{
				nCharWidth = m_nMaxCharWidth;
			}
			
			m_mapCharWidth.SetAt (chNext, nCharWidth);
		}

		if (chNext == _T ('\t') && pHiddenText == NULL && !bDrawSymbol)
		{
			int nRestOfTab = m_nTabSize - nCurrColumn % m_nTabSize;
			
			nCurrColumn += nRestOfTab; 

			int nRestOfTabWidth = nCharWidth - (ptCharOffset.x - m_rectText.left - 1) % nCharWidth;
			nCharWidth = nRestOfTabWidth;
		}
		else
		{
			nCurrColumn++;
		}
		
		// --------------
		// Define colors:
		// --------------

		BOOL bColorFound = FALSE;

		// 1) selection has the highest priority, check the character for selection first:
		if (!bIsPrint && IsInsideSelection (i, ptCharOffset, rectSel, nCharWidth))
		{
			clrForeground = clrTextSel; 
			clrBackground = clrBkSel; 
			bColorFound = TRUE;
		}
		// 2) check for colored line marker:
		else if (!bIsPrint && IsColoredLine (nCurrRowAbsolute, clrForeground, clrBackground))
		{
			bColorFound = TRUE;
			bColoredLine = TRUE;
		}
		// 3) chack for color blocks:
		else if (bIsOpenBlock && i < nCloseBlockOffset)
		{
			clrForeground = clrBlockFore;
			clrBackground = clrBlockBack;
			bColorFound = TRUE;
		}
		else
		{
			// check all color areas (comments, strings and so on)
			for (POSITION pos = colorAreas.GetHeadPosition (); pos != NULL;)
			{
				BCGP_EDIT_COLOR_AREA colorArea = colorAreas.GetNext (pos);
				if (i >= colorArea.m_nStart && i <= colorArea.m_nEnd)
				{
					clrForeground = colorArea.GetForegroundColor(m_bIsDarkBackground); 
					clrBackground = colorArea.m_clrBackground;

					if (clrForeground == -1)
					{
						clrForeground = GetDefaultTextColor ();
					}

					if (clrBackground == -1)
					{
						clrBackground = GetDefaultBackColor ();
					}

					bColorFound = TRUE;
					break;
				}
			}
		}

		// 4) User can define text color by own:
		if (OnGetTextColor (i, nNextDelimiter, clrForeground, clrBackground, bColorFound))
		{
			bColorFound = TRUE;
		}

		// 5) Check all color words (keywords and words):
		if (chNext != g_chEOL && !bDrawSymbol && pHiddenText == NULL)
		{
			if (nNextDelimiter == -1) 
			{
				CString strNextWord;		
				
				for (int iIdx = i; iIdx <= nEndOffset; iIdx++)
				{
					TCHAR ch = m_strBuffer [iIdx];
					if (m_strWordDelimeters.Find (ch) != -1)
					{
						nNextDelimiter = iIdx;
						break;
					}
				}

				if (nNextDelimiter == -1)
				{
					nNextDelimiter = nEndOffset + 1;
				}

				if (nNextDelimiter != -1)
				{
					strNextWord = 
						m_strBuffer.Mid (i, nNextDelimiter - i);
				}
						
				if (!OnGetWordColor (strNextWord, clrNextWordFore, clrNextWordBk, i))
				{
					clrNextWordFore = clrDefaultText;
					clrNextWordBk = clrDefaultBack;
				}
			}

			if (i >= nNextDelimiter - 1)
			{
				nNextDelimiter = -1;
			}

			if (!bColorFound)
			{
				clrForeground = clrNextWordFore;
				clrBackground = clrNextWordBk;
			}

			ptCharOffset.x += nCharWidth;
		}

		// 6) check for hilited text:
		if (!bIsPrint)
		{
			IsHilitedText (i, clrForeground, clrBackground);
		}

		// 7) User can define color of current char:
		if (!bColorFound && !bDrawSymbol && pHiddenText == NULL)
		{
			OnGetCharColor (chNext, i, clrForeground, clrBackground);
		}

		// 8) check for hyperlink:
		if (m_bEnableHyperlinkSupport && bHyperlink)
		{
			if (m_clrHyperlink != (COLORREF)-1)
			{
				if (clrForeground != clrTextSel && m_bColorHyperlink)
				{
					if (m_bIsDarkBackground)
					{
						if (m_clrHyperlinkLight == (COLORREF)-1)
						{
							m_clrHyperlinkLight = BCGP_EDIT_COLOR_BLOCK::MakeLightColor(m_clrHyperlink);
						}

						clrForeground = m_clrHyperlinkLight;
					}
					else
					{
						clrForeground = m_clrHyperlink;
					}
				}
			}
			else
			{
				clrForeground = GetDefaultTextColor();
			}
		}


		// --------------------
		// Proceed end of line:
		// --------------------

		if (chNext == g_chEOL && pHiddenText == NULL)
		{
			pDC->SetTextColor (clrOutText);
			pDC->SetBkColor   (clrOutBack);

			CFont* pOldFont = NULL;
			if (bHyperlinkOut)
			{
				pOldFont = SelectFont (pDC, bHyperlinkOut);
			}

			if (iIdx != 0)
			{
				if (m_bKeepTabs)
				{
					rectRow.left += DrawString (pDC, lpszOutBuffer, rectRow, nTabOrigin, clrDefaultBack);
				}
				else
				{
					DrawString (pDC, lpszOutBuffer, rectRow, nTabOrigin, clrDefaultBack);
					rectRow.left += pDC->GetTextExtent (lpszOutBuffer).cx;
				}
				
				iIdx = 0;
				memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
				rectRow.TopLeft ();
			}

			if (bHyperlinkOut)
			{
				pDC->SelectObject (pOldFont);
			}

			if (i < max (m_iStartSel, m_iEndSel) && !bColoredLine)
			{
				if (clrForeground != clrOutText || clrBackground != clrOutBack)
				{
					pDC->SetTextColor (clrForeground);
					pDC->SetBkColor   (clrBackground);
				}
				
				DrawString (pDC, _T (" "), rectRow, nTabOrigin, clrDefaultBack);
			}

			clrOutText = clrForeground;
			clrOutBack = clrBackground;
			bHyperlinkOut = bHyperlink;
					
			if (!bIsPrint)
			{
				DrawColorLine (pDC, nCurrRowAbsolute, rectRow);
			}

			rectRow.OffsetRect (0, m_nLineHeight);
			rectRow.left = rectText.left - m_nScrollOffsetHorz * m_nMaxCharWidth + 1;
			ptCharOffset = rectRow.TopLeft ();

			nCurrRow++;
			nCurrRowAbsolute++;
			nCurrColumn = 0;

			continue;
		} 
		
		// -----------------------------------
		// Proceed end of same color fragment:
		// -----------------------------------
		
		if (clrForeground != clrOutText || clrBackground != clrOutBack || 
			bHyperlink != bHyperlinkOut ||
			bDrawSymbol || pHiddenText != NULL)
		{
			pDC->SetTextColor (clrOutText);
			pDC->SetBkColor   (clrOutBack);

			if (iIdx != 0)
			{
				CFont* pOldFont = NULL;
				if (bHyperlinkOut)
				{
					pOldFont = SelectFont (pDC, bHyperlinkOut);
				}

				if (m_bKeepTabs)
				{
					rectRow.left += DrawString (pDC, lpszOutBuffer, rectRow, nTabOrigin, clrDefaultBack);
				}
				else
				{
					DrawString (pDC, lpszOutBuffer, rectRow, nTabOrigin, clrDefaultBack);
					rectRow.left += pDC->GetTextExtent (lpszOutBuffer).cx;
				}

				if (bHyperlinkOut)
				{
					pDC->SelectObject (pOldFont);
				}
				
				iIdx = 0;
				memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
				rectRow.TopLeft ();
			}

			clrOutText = clrForeground;
			clrOutBack = clrBackground;
			bHyperlinkOut = bHyperlink;
		}

		// ---------------------------------------------------
		// Proceed special symbols (SymSupport and Outlining):
		// ---------------------------------------------------
		if (pHiddenText != NULL)
		{
			ASSERT_VALID (pHiddenText);

			if (clrBackground != clrDefaultBack)
			{
				CRect rect = rectRow;
				rect.right = rect.left + nCharWidth;

				pDC->FillSolidRect (rect, clrBackground);
			}
			
#ifndef _BCGPEDIT_STANDALONE
			OnDrawOutlineSymbol (pDC, pHiddenText, m_clrLineOutline == (COLORREF)-1 ? CBCGPVisualManager::GetInstance()->GetEditOutlineColor(this): m_clrLineOutline, clrBackground);
#else
			OnDrawOutlineSymbol (pDC, pHiddenText, m_clrLineOutline == (COLORREF)-1 ? RGB(128, 128, 128) : m_clrLineOutline, clrBackground);
#endif

			rectRow.left += nCharWidth;
			i += pHiddenText->m_nEnd - pHiddenText->m_nStart;

			ptCharOffset = rectRow.TopLeft ();

			nCurrRowAbsolute += GetNumOfCharsInText (pHiddenText->m_nStart, pHiddenText->m_nEnd, g_chEOL);
			
			continue;
		}
		else if (bDrawSymbol)
		{
			if (clrBackground != clrDefaultBack)
			{
				CRect rect = rectRow;
				rect.right = rect.left + nCharWidth;

				pDC->FillSolidRect (rect, clrBackground);
			}
			if (!OnDrawSymbol (pDC, rectRow, symDef))
			{
				m_pSymbolsImgList->Draw (pDC, symDef.m_nIndex, rectRow.TopLeft (), ILD_TRANSPARENT);
			}

			rectRow.left += nCharWidth;
			i += nSymExtraLen;
			
			ptCharOffset = rectRow.TopLeft ();

			continue;
		}

		if (iIdx < m_nMaxScrollWidth)
		{
			lpszOutBuffer [iIdx] = chNext;
		}

		// ---------------------------
		// Trancate very long strings:
		// ---------------------------
		if (iIdx + 1 > m_nMaxScrollWidth - 1)
		{
			if (iIdx + 1 < m_nMaxScrollWidth)
			{
				lpszOutBuffer [iIdx + 1] = _T ('\0');
			}
			continue;
		}
		
		if (iIdx + 1 < m_nMaxScrollWidth)
		{
			lpszOutBuffer [iIdx + 1] = _T ('\0');
		}
		iIdx++;
	}

	// --------------------------
	// Draw the last of the text:
	// --------------------------

	if (iIdx != 0)
	{
		pDC->SetTextColor (clrOutText);
		pDC->SetBkColor   (clrOutBack);

		CFont* pOldFont = NULL;
		if (bHyperlinkOut)
		{
			pOldFont = SelectFont (pDC, bHyperlinkOut);
		}

		if (m_bKeepTabs)
		{
			rectRow.left += DrawString (pDC, lpszOutBuffer, rectRow, nTabOrigin, clrDefaultBack);
		}
		else
		{
			DrawString (pDC, lpszOutBuffer, rectRow, nTabOrigin, clrDefaultBack);
			rectRow.left += pDC->GetTextExtent (lpszOutBuffer).cx;
		}

		if (bHyperlinkOut)
		{
			pDC->SelectObject (pOldFont);
		}

		if (i < m_strBuffer.GetLength () && m_strBuffer.GetAt (i) == g_chEOL && 
			i >= min (m_iStartSel, m_iEndSel) && i <= max (m_iStartSel, m_iEndSel))
		{
			pDC->SetTextColor (clrTextSel);
			pDC->SetBkColor   (clrBkSel);

			rectRow.left += pDC->GetTextExtent (lpszOutBuffer).cx;
			DrawString (pDC, _T (" "), rectRow, nTabOrigin, clrDefaultBack);
		}

		if (!bIsPrint)
		{
			DrawColorLine (pDC, nCurrRowAbsolute, rectRow);
		}
	}

	pDC->SetBkMode (nOldBkMode);

	// restore indexes for hyperlink support:
	if (m_bEnableHyperlinkSupport)
	{
		m_nCurrHyperlink = LookupHyperlinkByOffset(m_nCurrOffset);
		m_nCurrHyperlinkHot = LookupHyperlinkByOffset(nCurrHyperlinkHotOffset);
	}
}
//**************************************************************************************
void CBCGPEditCtrl::DrawColorLine (CDC* pDC, int nRow, CRect rectRow)
{
	CObList lstMarkers;
	GetMarkersInRange (nRow, lstMarkers, g_dwBCGPEdit_LineColorMarker);

	if (!lstMarkers.IsEmpty ())
	{
		UINT unPriority = 0;
		for (POSITION pos = lstMarkers.GetHeadPosition (); pos != NULL;)
		{
			CBCGPLineColorMarker* pLineColorMarker = 
						DYNAMIC_DOWNCAST (CBCGPLineColorMarker, lstMarkers.GetNext (pos));
			ASSERT_VALID (pLineColorMarker);

			if (pLineColorMarker != NULL)
			{
				if (unPriority > pLineColorMarker->m_unPriority)
				{
					break;
				}

				unPriority = pLineColorMarker->m_unPriority;

				if (pLineColorMarker->m_bFullLine)
				{
					COLORREF clrBack = pLineColorMarker->m_clrBackground;
					if (clrBack == -1)
					{
						clrBack = GetDefaultBackColor ();
					}
					pDC->FillSolidRect (rectRow, clrBack);
				}
			}
		}
	}
}
//**************************************************************************************
int CBCGPEditCtrl::DrawString (CDC* pDC, LPCTSTR str, CRect rect, int nOrigin, 
							   COLORREF /*clrBack*/)
{
	COLORREF crBk = pDC->GetBkColor ();
	
	if (m_nLineVertSpacing > 0)
	{
		CSize size;
		if (m_bKeepTabs)  
		{
			int nOldMode = pDC->SetBkMode (TRANSPARENT);
			size = pDC->TabbedTextOut (rect.left, rect.top + m_nLineVertSpacing / 2, str, 1, &m_nTabLogicalSize, nOrigin);
			pDC->SetBkMode (nOldMode);
		}
		else
		{
			size = GetStringExtent (pDC, str, (int) _tcslen (str));
		}

		rect.right = rect.left + size.cx;
		pDC->FillSolidRect (rect, crBk);
	}

	if (m_nLineVertSpacing > 0)
	{
		rect.top += m_nLineVertSpacing / 2;
	}

	if (m_bKeepTabs)  
	{
		int nOldMode = pDC->SetBkMode (TRANSPARENT);
		int nRes = pDC->TabbedTextOut (rect.left, rect.top, str, 1, &m_nTabLogicalSize, nOrigin).cx;
		pDC->SetBkMode (nOldMode);

		return nRes;
	}
	
	int nOldMode = pDC->SetBkMode (TRANSPARENT);
	int nRes = pDC->DrawText (str, rect, DT_NOCLIP | DT_SINGLELINE | DT_NOPREFIX);
	pDC->SetBkMode (nOldMode);

	return nRes;
	
}
//**************************************************************************************
CSize CBCGPEditCtrl::GetStringExtent (CDC* pDC, LPCTSTR lpszString, int nCount)
{
	if (m_bEnableSymSupport)
	{
		ASSERT (m_pSymbolsImgList != NULL);

		int nTotalSymLen = 0;
		TCHAR* pszString = new TCHAR [nCount];
		int j = 0;

		for (int i = 0; i < nCount; i++)
		{
			TCHAR chNext = lpszString [i];
			BCGP_EDIT_SYM_DEF symDef;
			int nSymExtraLen = 0;
			if (LookUpSymbol (lpszString, i, nCount, chNext, symDef, nSymExtraLen))
			{
				IMAGEINFO imgInfo;
				m_pSymbolsImgList->GetImageInfo (symDef.m_nIndex, &imgInfo);
				nTotalSymLen += imgInfo.rcImage.right - imgInfo.rcImage.left;
				i += nSymExtraLen;
			}
			else
			{
				pszString [j++] = chNext;
			}
		}
		
		CSize sizeResult;
		if (m_bKeepTabs)  
		{
			sizeResult = GetOutputTabbedTextExtent (pDC, pszString, j);
		}
		else
		{
			sizeResult = pDC->GetTextExtent (pszString, j);
		}

		delete [] pszString;

		sizeResult.cx += nTotalSymLen;
		return sizeResult;
	}

	if (m_bKeepTabs)  
	{
		return GetOutputTabbedTextExtent (pDC, lpszString, nCount);
	}
	
	return pDC->GetTextExtent (lpszString, nCount);
}
//**************************************************************************************
CSize CBCGPEditCtrl::GetOutputTabbedTextExtent (CDC* pDC, LPCTSTR lpszString, int nCount)
{
	// for large strings calculate text extent by 1024 characters
	CSize size (0, 0);
	const int nMaxStrLen = 1024;
	
	for (int i = 0; i < nCount; i += nMaxStrLen)
	{
		int nStrLen = min (nMaxStrLen, nCount - i);
		LPCTSTR lpszStr = lpszString + i;

		CSize sizeStr = pDC->GetOutputTabbedTextExtent (lpszStr, nStrLen, 1, &m_nTabLogicalSize);	
		size.cx += sizeStr.cx;
		size.cy = max (size.cy, sizeStr.cy);
	}

	return size;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IsInsideSelection (int nOffset, CPoint& ptCharOffset, 
									   CRect rectSel, int nCharWidth) 
{
	if (m_bBlockSelectionMode)
	{
		CPoint ptCharOffsetExt (ptCharOffset.x + nCharWidth, ptCharOffset.y + m_nLineHeight);
		CRect rectChar (ptCharOffset, ptCharOffsetExt);
		return rectSel.IntersectRect (rectSel, rectChar);
	}

	BOOL bInsideBlock = (nOffset >= m_iStartSel && nOffset < m_iEndSel ||
						 nOffset >= m_iEndSel &&  nOffset < m_iStartSel);

	return bInsideBlock;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IsColoredLine (int nRow, COLORREF& clrFore, COLORREF& clrBack)
{
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		CBCGPEditMarker* pMarker =  (CBCGPEditMarker*) m_lstMarkers.GetNext (pos);
		ASSERT_VALID (pMarker);

		if ((pMarker->m_dwMarkerType & g_dwBCGPEdit_LineColorMarker) == 0)
		{
			continue;
		}

		CBCGPLineColorMarker* pLineMarker = (CBCGPLineColorMarker*) pMarker;
		ASSERT_VALID (pLineMarker);

		if (nRow >= pLineMarker->m_nLine && 
			nRow <= pLineMarker->m_nLine + pLineMarker->m_nLineCount)
		{
			clrFore = pLineMarker->m_clrForeground;
			clrBack = pLineMarker->m_clrBackground; 
			return TRUE;
		}
	} 
	
	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IsHilitedText (int nOffset, COLORREF& clrFore, COLORREF& clrBack)
{
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		CBCGPEditMarker* pMarker =  (CBCGPEditMarker*) m_lstMarkers.GetNext (pos);
		ASSERT_VALID (pMarker);

		if ((pMarker->m_dwMarkerType & g_dwBCGPEdit_HiliteMarker) == 0)
		{
			continue;
		}

		CBCGPHiliteMarker* pHiliteMarker = (CBCGPHiliteMarker*) pMarker;
		ASSERT_VALID (pHiliteMarker);

		if (nOffset >= pHiliteMarker->m_nStart && nOffset < pHiliteMarker->m_nEnd)
		{
			clrFore = pHiliteMarker->m_clrForeground;
			clrBack = pHiliteMarker->m_clrBackground; 
			return TRUE;
		}
	} 
	
	return FALSE;
}
//**************************************************************************************
int CBCGPEditCtrl::GetHorzRowExtent (CDC* pDC, int nOffset)
{
	if (m_nScrollOffsetHorz > 0)
	{
		int nNextRowEnd = m_strBuffer.Find (g_chEOL, nOffset);
		if (nNextRowEnd == -1)
		{
			nNextRowEnd = m_strBuffer.GetLength ();
		}
		if (nNextRowEnd != -1 && nOffset < m_strBuffer.GetLength ())
		{
			CString strNextRow = m_strBuffer.Mid (nOffset, nNextRowEnd);

			CString strTabs (_T (' '), m_nTabSize);
			strNextRow.Replace (_T ("\t"), strTabs);

			int nCount = min (m_nScrollOffsetHorz, strNextRow.GetLength ());
			CString strNextRowStart = strNextRow.Mid (0, nCount);
			
			return GetStringExtent (pDC, strNextRowStart, nCount).cx;
		}
	}
	return 0;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::FindOpenBlock (int nStartOffset, BCGP_EDIT_COLOR_BLOCK* pFoundBlock)
{
	LPTSTR  lpcszBuffer = (LPTSTR) (LPCTSTR) m_strBuffer;
	LPTSTR  lpBuffer = (LPTSTR) lpcszBuffer;

	LPTSTR  lpCloseBlock = NULL;
	LPTSTR	lpOpenBlock = NULL;

	// lpBuffer points to the offsset to start the reverse search from;
	lpBuffer = lpcszBuffer + nStartOffset;

	for (POSITION pos = m_lstColorBlocks.GetHeadPosition (); pos != NULL;)
	{
		BCGP_EDIT_COLOR_BLOCK colorBlock = m_lstColorBlocks.GetNext (pos);
		if (colorBlock.m_bWholeText)
		{
			int nCloseBlockLen = colorBlock.m_strClose.GetLength ();
			int nOpenBlockLen  = colorBlock.m_strOpen.GetLength ();

			if (nOpenBlockLen == 0 || nCloseBlockLen == 0)
			{
				continue;
			}

			TCHAR chFirstCloseChar = colorBlock.m_strClose.GetAt (nCloseBlockLen - 1);
			TCHAR chFirstOpenChar = colorBlock.m_strOpen.GetAt (nOpenBlockLen - 1);

			BOOL bCloseBlockFound = FALSE;
			BOOL bOpenBlockFound = FALSE;
			do
			{
				bOpenBlockFound = FALSE;
				lpBuffer--;
				if (*lpBuffer == chFirstCloseChar && !bCloseBlockFound && 
					lpBuffer - nCloseBlockLen >= lpcszBuffer)
				{
					bCloseBlockFound = TRUE;
					for (int i = 0; i < nCloseBlockLen; i++)
					{
						if (*(lpBuffer - i) != colorBlock.m_strClose.GetAt (nCloseBlockLen - i - 1))
						{
							bCloseBlockFound = FALSE;
							break;
						}
					}

					if (bCloseBlockFound)
					{
						lpBuffer -= nCloseBlockLen;
						lpCloseBlock = lpBuffer;
					}
				}
				
				if (*lpBuffer == chFirstOpenChar)
				{
					if (nOpenBlockLen == 2 && lpBuffer > lpcszBuffer)
					{
						if (*(--lpBuffer) == colorBlock.m_strOpen.GetAt (0))		
						{
							// may be first opening symbol is the same as last closing symbol
							// like in C++ /* and */
							// To prevent this situation: */*
							if (lpBuffer > lpcszBuffer && 
								*(lpBuffer - 1) == chFirstOpenChar)
							{
								lpBuffer++;
								continue;
							}

							bOpenBlockFound = TRUE;
							lpOpenBlock = lpBuffer;
						}
						else
						{
							lpBuffer++;
						}
					}
					else if (nOpenBlockLen == 1)
					{
						bOpenBlockFound = TRUE;
						lpOpenBlock = lpBuffer;
					}
					else if (lpBuffer - nOpenBlockLen >= lpcszBuffer)
					{
						bOpenBlockFound = TRUE;
						for (int i = 0; i < nOpenBlockLen; i++)
						{
							if (*(lpBuffer - i) != colorBlock.m_strOpen.GetAt (nOpenBlockLen - i - 1))
							{
								bOpenBlockFound = FALSE;
								break;
							}
						}

						if (bOpenBlockFound)
						{
							lpBuffer -= nOpenBlockLen;
							lpOpenBlock = lpBuffer;
						}
					}

					if (bOpenBlockFound)
					{
						// check whether this open block mark is not commented out
						// at the beginning of the string by another block
		
						int nCurrOffset = (int) (lpBuffer - lpcszBuffer);
						int nRowOffset = GetRowStartByOffset (nCurrOffset, FALSE);
						CList <BCGP_EDIT_COLOR_AREA, BCGP_EDIT_COLOR_AREA&> colorAreas;

						BuildColorAreas (colorAreas, nRowOffset, nCurrOffset + 2, &colorBlock, FALSE);

						for (POSITION posTmp = colorAreas.GetHeadPosition (); posTmp != NULL;)
						{
							BCGP_EDIT_COLOR_AREA colorArea = colorAreas.GetNext (posTmp);
							if (nCurrOffset > colorArea.m_nStart && 
								nCurrOffset < colorArea.m_nEnd)
							{
								int nCount = colorArea.m_nStart - 1;
								if (nCount < 0)
								{
									nCount = 0;
								}
								lpBuffer = lpcszBuffer + nCount;
								bOpenBlockFound = FALSE;
								lpOpenBlock = NULL;
								break;
							}
						}

						if (bOpenBlockFound)
						{
							break;
						}
					}						
				}
			}
			while (lpBuffer >= lpcszBuffer);

			if (bOpenBlockFound)
			{
				*pFoundBlock = colorBlock;
				break;
			}
			else
			{
				lpBuffer = lpcszBuffer + nStartOffset;
			}
		}
	}

	return (lpOpenBlock != NULL && lpCloseBlock == NULL);
}
//**************************************************************************************
int CBCGPEditCtrl::FindCloseBlock (int nStartOffset, BCGP_EDIT_COLOR_BLOCK* pFoundBlock)
{
	int nResult = -1;
	for (POSITION pos = m_lstColorBlocks.GetHeadPosition (); pos != NULL;)
	{
		BCGP_EDIT_COLOR_BLOCK colorBlock = m_lstColorBlocks.GetNext (pos);
		if (colorBlock.m_bWholeText && colorBlock == *pFoundBlock)
		{
			nResult = m_strBuffer.Find (colorBlock.m_strClose, nStartOffset);
			if (nResult == -1)
			{
				nResult = m_strBuffer.GetLength () - 1;
			}
			else
			{
				nResult += colorBlock.m_strClose.GetLength ();
			}
		}
	}

	return nResult;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IsInBlock (int nPos, TCHAR chOpen, TCHAR chClose, 
							   int& nBlockStart, int& nBlockEnd)
{
	BOOL bBlockFound = FALSE;
	CString strBlockChars;
	strBlockChars += chOpen;
	strBlockChars += chClose;

	int nStartOffset = nPos;
	int nLeftBlock = ReverseFindOneOf (m_strBuffer, nStartOffset, strBlockChars);
	if (nLeftBlock - 1 >= 0)
	{
		TCHAR chFound = GetCharAt (nLeftBlock - 1);
		if (chFound == chOpen)
		{
			int nRightBlock = FindOneOf (m_strBuffer, nStartOffset, strBlockChars);
			if (nRightBlock != -1)
			{
				chFound = GetCharAt (nRightBlock);
				if (chFound == chClose)
				{
					bBlockFound = TRUE;
					nBlockStart = nLeftBlock - 1;
					nBlockEnd = nRightBlock;
				}
			}
		}
	}

	return bBlockFound;
}
//**************************************************************************************
void CBCGPEditCtrl::SetColorBlockStrLenMax (int nValue)
{
	m_nColorBlockStrLenMax = (nValue > 1) ? nValue : _MAX_COLOR_BLOCK_STR_LEN;
}
//**************************************************************************************
void CBCGPEditCtrl::AddEscapeSequence (LPCTSTR lpszStr)
{
	ASSERT (lpszStr != NULL);
	m_lstEscapeSequences.AddTail (lpszStr);
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IsEscapeSequence (const CString& strBuffer, int nOffset, BOOL bDirForward) const
{
	if (nOffset < 0 || nOffset >= strBuffer.GetLength ())
	{
		return FALSE;
	}

	ASSERT (nOffset >= 0);
	ASSERT (nOffset < strBuffer.GetLength ());

	for (POSITION pos = m_lstEscapeSequences.GetHeadPosition (); pos != NULL; )
	{
		const CString& str = m_lstEscapeSequences.GetNext (pos);
		if (str.GetLength () > 0 && IsIqual (strBuffer, nOffset, bDirForward, str))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IsIqual (const CString& strBuffer, int nOffset, BOOL bDirForward, const CString& str) const
{
	ASSERT (nOffset >= 0);
	ASSERT (nOffset < strBuffer.GetLength ());
	
	if (bDirForward)
	{
		const int nLeftOffset = nOffset;

		if (nLeftOffset + str.GetLength () - 1 < strBuffer.GetLength ())
		{
			ASSERT (nLeftOffset >= 0); // always
			ASSERT (nLeftOffset < strBuffer.GetLength ()); // to check
			if (DoCompare (strBuffer, nLeftOffset, str.GetLength (), str))
			{
				return TRUE;
			}
		}
	}
	else
	{
		int i = str.GetLength () - 1;
		const int nLeftOffset = nOffset - i;

		if (nLeftOffset >= 0)
		{
			ASSERT (nLeftOffset >= 0);	// to check
			ASSERT (nLeftOffset + i < strBuffer.GetLength ()); // always
			if (DoCompare (strBuffer, nLeftOffset, str.GetLength (), str))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}
//**************************************************************************************
// Compare substring of the strBuffer with the strWith
BOOL CBCGPEditCtrl::DoCompare (const CString& strBuffer, const int nLeft, const int nCount, const CString& strWith) const
{
	ASSERT (nLeft >= 0);
	ASSERT (nLeft + nCount - 1 < strBuffer.GetLength ());

	const int nOffset = nLeft;
	int i = 0;
	const int strWithLen = strWith.GetLength ();
	while (i < strWithLen && i < nCount)
	{
		if (strWith.GetAt(i) != strBuffer.GetAt(nOffset + i))
		{
			return FALSE;
		}
		i++;
	}
	return (i > 0) && (i == strWith.GetLength ());
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IsCharBlockSymbol (TCHAR ch)
{
	for (POSITION pos = m_lstColorBlocks.GetHeadPosition (); pos != NULL;)
	{
		BCGP_EDIT_COLOR_BLOCK colorBlock = m_lstColorBlocks.GetNext (pos);
		if (colorBlock.m_strOpen.Find (ch) != -1 || 
			colorBlock.m_strClose.Find (ch) != -1)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IsStringHasBlockSymbols (const CString& str)
{
	for (POSITION pos = m_lstColorBlocks.GetHeadPosition (); pos != NULL;)
	{
		BCGP_EDIT_COLOR_BLOCK colorBlock = m_lstColorBlocks.GetNext (pos);
		if (str.FindOneOf (colorBlock.m_strOpen) != -1 || 
			str.FindOneOf (colorBlock.m_strClose) != -1)
		{
			return TRUE;
		}
	}

	return FALSE;

}
//**************************************************************************************
void CBCGPEditCtrl::BuildColorAreas (CList <BCGP_EDIT_COLOR_AREA, BCGP_EDIT_COLOR_AREA&>& colorAreas, 
									 int nStartOffset, int nEndOffset,
									 BCGP_EDIT_COLOR_BLOCK* pColorBlockToExclude,
									 BOOL bRedraw)
{
	if (nStartOffset > nEndOffset)
	{
		return;
	}
	
	colorAreas.RemoveAll ();

	// get only a part of the big buffer
	CString strBuffer = m_strBuffer.Mid (nStartOffset, nEndOffset - nStartOffset + 1);
	CString strBufferUpper; // copy of the buffer for case sensitive search

	// we need to iterate the list of blocks and record the smallest start block position
	// and then look for the end of this position

	const int nBufferLen = strBuffer.GetLength ();
	int nNextStartOffset = 0;

	while (nNextStartOffset < nBufferLen)
	{
		BCGP_EDIT_COLOR_AREA colorArea;
		int nIdxMin = strBuffer.GetLength ();
		POSITION posBlockToUse = NULL;

		for (POSITION pos = m_lstColorBlocks.GetHeadPosition (); pos != NULL; 
													 m_lstColorBlocks.GetNext (pos))
		{
			BCGP_EDIT_COLOR_BLOCK colorBlock = m_lstColorBlocks.GetAt (pos);

			if (pColorBlockToExclude != NULL && 
				*pColorBlockToExclude == colorBlock)
			{
				continue;
			}

			if (colorBlock.m_strOpen.IsEmpty ())
			{
				continue;
			}
	
			if (!colorBlock.m_bCaseSensitive &&
				!colorBlock.m_strOpen.IsEmpty ())
			{
				// on demand prepare buffer for case insensitive search
				if (strBufferUpper.GetLength () != strBuffer.GetLength ())
				{
					strBufferUpper = strBuffer;
					strBufferUpper.MakeUpper ();
				}
			}

			int nIdx = nNextStartOffset;
			for (;;)
			{
				nIdx = (!colorBlock.m_bCaseSensitive) ?
					strBufferUpper.Find (colorBlock.m_strOpen, nIdx) :
					strBuffer.Find (colorBlock.m_strOpen, nIdx);

				// Nothing found
				if (nIdx < 0)
				{
					break;
				}

				// Check for a m_bWholeWord condition
				if (colorBlock.m_bWholeWord && colorBlock.m_strOpen.FindOneOf (m_strWordDelimeters) == -1)
				{
					// Need to check the delimiter
					if (// prev char is not delimiter
						(nIdx > 0 && m_strWordDelimeters.Find (strBuffer [nIdx - 1]) == -1) ||
						
						// next char is not delimiter
						(nIdx + colorBlock.m_strOpen.GetLength () < nBufferLen && 
						m_strWordDelimeters.Find (strBuffer [nIdx + colorBlock.m_strOpen.GetLength ()]) == -1))
					{
						// Skip and search on couldn't find the delimiters.
						++nIdx;
						continue;
					}
				}

				// Found a valid word
				break;
			}

			if (nIdx == -1)
			{
				continue;
			}

			if (nIdx < nIdxMin)
			{
				nIdxMin = nIdx;
				posBlockToUse = pos;
			}
		}

		if (posBlockToUse == NULL)
		{
			return;
		}

		BCGP_EDIT_COLOR_BLOCK colorBlock = m_lstColorBlocks.GetAt (posBlockToUse);
		colorArea = colorBlock;
		colorArea.m_nStart = nIdxMin; 
		colorArea.m_nEnd = -1;
		if (!colorBlock.m_strClose.IsEmpty ())
		{
			if (!colorBlock.m_bCaseSensitive)
			{
				// on demand prepare buffer for case insensitive search
				if (strBufferUpper.GetLength () != strBuffer.GetLength ())
				{
					strBufferUpper = strBuffer;
					strBufferUpper.MakeUpper ();
				}
			}

			int nNextEndOffset = nIdxMin + max (1, colorBlock.m_strOpen.GetLength ());
			do 
			{
				colorArea.m_nEnd = (!colorBlock.m_bCaseSensitive) ?
					strBufferUpper.Find (colorBlock.m_strClose, nNextEndOffset) :
					strBuffer.Find (colorBlock.m_strClose, nNextEndOffset);
				if (colorArea.m_nEnd == -1)
				{
					break;
				}
				
				// prev char is ContinueStringSymbol
				if (g_chEOL == strBuffer.GetAt (colorArea.m_nEnd) && 
					m_strContinueStringSymbols.Find (strBuffer.GetAt (colorArea.m_nEnd - 1)) >= 0 &&
					!IsEscapeSequence (strBuffer, colorArea.m_nEnd - 1, FALSE))
				{
					nNextEndOffset = colorArea.m_nEnd + 1; // skip line feed
					continue;
				}

				if (colorBlock.m_bWholeWord && colorBlock.m_strClose.FindOneOf (m_strWordDelimeters) == -1)
				{
					if (// prev char is not delimiter
						(colorArea.m_nEnd > 0 && m_strWordDelimeters.Find (strBuffer [colorArea.m_nEnd - 1]) == -1) ||

						// next char is not delimiter
						(colorArea.m_nEnd + colorBlock.m_strClose.GetLength () < nBufferLen && 
						 m_strWordDelimeters.Find (strBuffer [colorArea.m_nEnd + colorBlock.m_strClose.GetLength ()]) == -1))
					{
						nNextEndOffset = colorArea.m_nEnd + max (1, colorBlock.m_strClose.GetLength ());
						continue;
					}
				}

				break;

			}while (colorArea.m_nEnd != -1);
		}
		
		int nCloseLen = colorBlock.m_strClose.GetLength ();
		if (!colorBlock.m_bWholeText)
		{
			int nIdxEOL = strBuffer.Find (g_strEOL, nIdxMin + 1);
			if (colorArea.m_nEnd == -1 || (colorArea.m_nEnd > nIdxEOL && nIdxEOL != -1))
			{
				colorArea.m_nEnd = nIdxEOL;
				nCloseLen = 1;
			}
		}

		if (colorArea.m_nEnd == -1)
		{
			colorArea.m_nEnd = nBufferLen - 1;
		}

		nNextStartOffset = colorArea.m_nEnd + max (1, nCloseLen);

		colorArea.m_nStart += nStartOffset;
		if (nCloseLen > 0)
		{
			colorArea.m_nEnd += nStartOffset + nCloseLen - 1;
		}
		else
		{
			colorArea.m_nEnd += nStartOffset;	
		}
		
		colorAreas.AddTail (colorArea);
	}

	if (bRedraw)
	{
		RedrawWindow ();
	}
}
//**************************************************************************************
BOOL CBCGPEditCtrl::OnGetWordColor (const CString& strWord, 
									COLORREF& clrText, COLORREF& clrBk, 
									int /*nPos*/)
{
	BCGP_EDIT_SYNTAX_COLOR clrWord;

	if (strWord.IsEmpty ())
	{
		return FALSE;
	}

	if (!m_mapWordColors.Lookup (strWord, clrWord))
	{
		CString strUpper = strWord; 
		strUpper.MakeUpper ();
		if (!m_mapWordColors.Lookup (strUpper, clrWord))
		{
			return FALSE;
		}

		if (!clrWord.m_bCaseSensitive)
		{
			clrText = clrWord.GetForegroundColor(m_bIsDarkBackground);
			clrBk = clrWord.m_clrBackground;

			if (clrText == (COLORREF) -1)
			{
				clrText = GetDefaultTextColor ();
			}

			if (clrBk == (COLORREF) -1)
			{	
				clrBk = GetDefaultBackColor ();
			}

			return TRUE;
		}

		return FALSE;
	}

	if (m_mapWordColors.Lookup (strWord, clrWord))
	{
		clrText = clrWord.GetForegroundColor(m_bIsDarkBackground);
		clrBk = clrWord.m_clrBackground;

		if (clrText == (COLORREF) -1)
		{
			clrText = GetDefaultTextColor ();
		}

		if (clrBk == (COLORREF) -1)
		{	
			clrBk = GetDefaultBackColor ();
		}

		return TRUE;
	}

	return FALSE;
}
//************************************************************************************
BOOL CBCGPEditCtrl::Paste (int iPos /*= -1*/)
{
	ASSERT_VALID (this);

	if (m_bReadOnly)
	{
		OnFailedOperation (g_dwOpPaste | g_dwOpReasonReadOnly);
		return FALSE;
	}

	COleDataObject data;
	if (!data.AttachClipboard ())
	{
		TRACE0("Can't open clipboard\n");
		OnFailedOperation (g_dwOpPaste | g_dwOpReasonError);
		return FALSE;
	}
	
	if (!data.IsDataAvailable (_TCF_TEXT) && 
		!data.IsDataAvailable (g_cFormat))
	{
		TRACE0("Incorrect clipboard format\n");
		OnFailedOperation (g_dwOpPaste | g_dwOpReasonError);
		return FALSE;
	}

	BOOL bForceNextUndo = TRUE;
	
	DeleteSelectedText (FALSE, FALSE, bForceNextUndo);
	SetLastUndoReason (g_dwUATPaste);
	PasteFromDataObject (&data, iPos, TRUE, FALSE, TRUE, FALSE);

	m_bIsModified = TRUE;

	return TRUE;
}
//************************************************************************************
BOOL CBCGPEditCtrl::PasteFromDataObject (COleDataObject* pDataObject, 
										 int nInsertFrom, BOOL bRedraw, 
										 BOOL bSuppressUndo, BOOL bUpdateLineData,
										 BOOL bForceNextUndo)
{
		// check for internal data first - copy was made by edit control
	HGLOBAL hClipbuffer = NULL;
	bool bBlock = false;
	if (pDataObject->IsDataAvailable (g_cFormat))
	{
		hClipbuffer = pDataObject->GetGlobalData (g_cFormat);
		bBlock = true;
	}
	else if (pDataObject->IsDataAvailable (_TCF_TEXT))
	{
		hClipbuffer = pDataObject->GetGlobalData (_TCF_TEXT);
	}

	BOOL bResult = FALSE;
	if (hClipbuffer != NULL)
	{
		LPTSTR lpszBuffer = (LPTSTR) GlobalLock (hClipbuffer);
		ASSERT (lpszBuffer != NULL);

		bResult = bBlock ? InsertTextAsBlock (lpszBuffer, nInsertFrom, bRedraw, bSuppressUndo, bUpdateLineData, bForceNextUndo) :
						InsertText (lpszBuffer, nInsertFrom, bRedraw, bSuppressUndo, bUpdateLineData, bForceNextUndo);

		::GlobalUnlock (hClipbuffer);
	}

	return bResult;
}
//************************************************************************************
BOOL CBCGPEditCtrl::IsPasteEnabled () const
{
	return !m_bReadOnly &&
		(::IsClipboardFormatAvailable (_TCF_TEXT) ||
		 ::IsClipboardFormatAvailable (g_cFormat));
}
//************************************************************************************
BOOL CBCGPEditCtrl::Copy ()
{
	ASSERT_VALID (this);

	if (!IsCopyEnabled ())
	{
		ASSERT (FALSE);
		OnFailedOperation (g_dwOpCopy);
		return FALSE;
	}

	if (m_iStartSel >= 0)
	{
		if (m_iEndSel < 0)
		{
			ASSERT (FALSE);
			OnFailedOperation (g_dwOpCopy);
			return FALSE;
		}

		CString strBlock;
		PrepareBlock (strBlock);

		return CopyTextToClipboard (strBlock, strBlock.GetLength ());
	}

	// if no selection - copy current line
	if (m_bEnableCurrentLineCopy)
	{
		CString strLine;
		if (!GetCurLine (strLine))
		{
			ASSERT (FALSE);
			OnFailedOperation (g_dwOpCopy);
			return FALSE;
		}

		strLine += g_chEOL;

		return CopyTextToClipboard (strLine, strLine.GetLength ());	// Copy the current line
	}

	// if no selection - copy whole text
	if (!m_bEnableWholeTextCopy)
	{
		ASSERT (FALSE);
		OnFailedOperation (g_dwOpCopy);
		return FALSE;
	}

	return CopyTextToClipboard (m_strBuffer, m_strBuffer.GetLength ());	// Copy the whole text
}
//************************************************************************************
BOOL CBCGPEditCtrl::Cut ()
{
	ASSERT_VALID (this);

	if (!IsCutEnabled ())
	{
		ASSERT (FALSE);
		OnFailedOperation (g_dwOpCopy);
		return FALSE;
	}
	SetLastUndoReason (g_dwUATCut);
	return DeleteSelectedText (TRUE, TRUE);
}
//***************************************************************************************		
BOOL CBCGPEditCtrl::Clear ()
{
	ASSERT_VALID (this);
	SetLastUndoReason (g_dwUATDelete);
	return DeleteSelectedText ();
}
//***************************************************************************************		
BOOL CBCGPEditCtrl::DeleteText (int nStartOffset, int nEndOffset, 
								BOOL bRedraw)
{
	int nStart	= min (nStartOffset, nEndOffset);
	int nEnd	= max (nStartOffset, nEndOffset);

	int nDeleteCount = nEnd - nStart;

	if (nDeleteCount == 0)
	{
		return FALSE;
	}

	if (nDeleteCount > m_strBuffer.GetLength ())
	{
		int nDelta = nDeleteCount - m_strBuffer.GetLength ();
		nEnd -= nDelta;
	}


	CString strDeletedText = m_strBuffer.Mid (nStart, nEnd - nStart);

	if (!OnBeforeTextDeleted (nStart, strDeletedText))
	{
		return FALSE;
	}

	AddUndoAction (strDeletedText, g_dwUATDeleteData, nStart, TRUE);

	OnDeleteTextFromBuffer (nStart, nEnd, strDeletedText);
	RemoveOffsetDataInRange (nStart, nEnd);

	m_strBuffer.Delete (nStart, nEnd - nStart);

	OnUpdateAutoOutlining (nStart, 0, FALSE);
	OnAfterTextChanged (nStart, strDeletedText, FALSE);

	m_nCurrOffset = nStart;
	
	UpdateScrollBars ();
	BOOL bNeedUpdate = OnUpdateLineNumbersMarginWidth ();

	if (bNeedUpdate)
	{
		RecalcLayout ();
	}

	SetCaret (nStart);

	if (bRedraw || bNeedUpdate)
	{
		RedrawWindow ();
	}

	return TRUE;
}
//***************************************************************************************		
void CBCGPEditCtrl::SelectLine (int nLine, BOOL bNoScroll, BOOL bRedraw)
{
	ASSERT (nLine >= 0);

    int nStartOffset = GetRowStart (nLine);

	if (nStartOffset == -1)
	{
		return;
	}

    int nEndOffset = GetRowEndByOffset (nStartOffset);

	// Make selection:
	SetSel2 (nStartOffset, nEndOffset + 1, TRUE, FALSE);

	if (!bNoScroll)
	{
		SetCaret (nStartOffset, TRUE, FALSE);
	}
	if (bRedraw)
	{
		RedrawWindow ();
	}
}
//***************************************************************************************		
void CBCGPEditCtrl::SelectLineRange (int nFromLine, int nToLine, BOOL bNoScroll, BOOL bRedraw)
{
	ASSERT (nFromLine >= 0);
	ASSERT (nToLine >= nFromLine);

    int nStartOffset = GetRowStart (nFromLine);

	if (nStartOffset == -1)
	{
		return;
	}

	int nRow = nFromLine;
	int nEndOffset = nStartOffset;
	do
	{
		nEndOffset = GetRowEndByOffset (nEndOffset) + 1;
		nRow++;
	}
	while (nRow <= nToLine && nEndOffset < m_strBuffer.GetLength ());

	// Make selection:
	SetSel2 (nStartOffset, nEndOffset, TRUE, FALSE);

	if (!bNoScroll)
	{
		SetCaret (nStartOffset, TRUE, FALSE);
	}
	if (bRedraw)
	{
		RedrawWindow ();
	}
}
//***************************************************************************************		
BOOL CBCGPEditCtrl::DeleteSelectedText (BOOL bRedraw, BOOL bCopyToClipboard, 
										BOOL bForceNextUndo)
{
	if (m_iStartSel < 0)
	{
		OnFailedOperation (g_dwOpDelSelText);
		return FALSE;
	}

	if (m_bBlockSelectionMode)
	{
		
	}

	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel);

	CString strDeletedText = m_strBuffer.Mid (iStartSel, iEndSel - iStartSel);

	if (!OnBeforeTextDeleted (iStartSel, strDeletedText))
	{
		return FALSE;
	}

	if (bCopyToClipboard && 
		!CopyTextToClipboard (strDeletedText, strDeletedText.GetLength ()))
	{
		OnFailedOperation (g_dwOpDelSelText | g_dwOpReasonError);
		return FALSE;
	}

	int nDeleteCount = iEndSel - iStartSel;
	if (nDeleteCount > m_strBuffer.GetLength ())
	{
		int nDelta = nDeleteCount - m_strBuffer.GetLength ();
		iEndSel -= nDelta;
	}
	
	AddUndoAction (strDeletedText, g_dwUATDeleteData, iStartSel, FALSE, bForceNextUndo);

	OnDeleteTextFromBuffer (iStartSel, iEndSel, strDeletedText);
	RemoveOffsetDataInRange (iStartSel, iEndSel);
	m_strBuffer.Delete (iStartSel, iEndSel - iStartSel);

	OnUpdateAutoOutlining (iStartSel, 0, FALSE);
	OnAfterTextChanged (iStartSel, strDeletedText, FALSE);

	int nRedrawOffset = iStartSel;
	m_iStartSel = m_iEndSel = -1;
	m_ptStartBlockSel = m_ptEndBlockSel = CPoint (-1, -1);

	UpdateScrollBars ();

	m_nScrollOffsetVert = RowFromOffset (m_nTopVisibleOffset, TRUE, TRUE);
	m_nScrollHiddenVert = RowFromOffset (m_nTopVisibleOffset, TRUE, FALSE) - m_nScrollOffsetVert;
	m_wndScrollVert.SetScrollPos (m_nScrollOffsetVert);

	BOOL bNeedUpdate = OnUpdateLineNumbersMarginWidth ();
	if (bNeedUpdate)
	{
		RecalcLayout ();
	}

	if (bNeedUpdate || (bRedraw && nDeleteCount > 0))
	{
		RedrawWindow ();
	}

	m_nCurrOffset = nRedrawOffset;
	SetCaret (nRedrawOffset, TRUE, bRedraw);

	return TRUE;
}
//***************************************************************************************		
void CBCGPEditCtrl::OnInsertTextToBuffer (int nStartOffset, 
										  const CString& strInsertedText,
										  BOOL bUpdateLineData)
{
	int nNewRowsInserted = GetNumOfCharsInText (strInsertedText, g_chEOL);
	
	if (nNewRowsInserted > 0 && bUpdateLineData)
	{
		int nStartRow = RowFromOffset (nStartOffset, FALSE, TRUE);
		int nStartRowOffset = GetRowStartByOffset (nStartOffset, TRUE);
		if (nStartRowOffset == nStartOffset)
		{
			// inserted at this row from the start - move the whole row
			UpdateLineRelatedData (nStartRow, nNewRowsInserted);
		}
		else
		{
			// update markesr from the next row
			UpdateLineRelatedData (nStartRow + 1, nNewRowsInserted);
		}
	}

	m_nTotalLines += nNewRowsInserted;
}
//***************************************************************************************		
void CBCGPEditCtrl::OnDeleteTextFromBuffer (int nStartOffset, int nEndOffset, 
										  LPCTSTR lpcszDeletedText)
{
	CString strDeletedText;
	if (lpcszDeletedText == NULL)
	{
		strDeletedText = m_strBuffer.Mid (nStartOffset, nEndOffset - nStartOffset);
	}
	else
	{
		strDeletedText = lpcszDeletedText;
	}

	if (strDeletedText.Find (g_strEOL) >= 0)
	{
		int nRowCount = GetNumOfCharsInText (strDeletedText, g_chEOL);
		int nStartRow = RowFromOffset (nStartOffset, FALSE, FALSE);
		int nEndRow = RowFromOffset (nEndOffset, FALSE, FALSE);

		int nRowStartOffset = GetRowStartByOffset (nStartOffset, TRUE);
		BOOL bWholeStartRowDeleted = nRowStartOffset == nStartOffset;

		int nRowEndOffset = GetRowEndByOffset (nEndOffset, TRUE);
		BOOL bWholeEndEowDeleted = nRowEndOffset == nEndOffset;

		RemoveLineDataInRange  (bWholeStartRowDeleted ? nStartRow : nStartRow + 1, 
								bWholeEndEowDeleted ? nEndRow : nEndRow - 1);
		UpdateLineRelatedData (nStartRow + 1, -nRowCount);

		m_nTotalLines -= nRowCount;
	}

	if (nStartOffset < m_nTopVisibleOffset)
	{
		if (nEndOffset < m_nTopVisibleOffset)
		{
			m_nTopVisibleOffset -= (nEndOffset - nStartOffset);
		}
		else 
		{
			m_nTopVisibleOffset -= (m_nTopVisibleOffset - nStartOffset);
		}

		m_nTopVisibleOffset = GetRowStartByOffset (m_nTopVisibleOffset, TRUE);
	}
}
//***************************************************************************************		
BOOL CBCGPEditCtrl::CopyTextToClipboard (LPCTSTR lpszText, int nLen)
{
	ASSERT_VALID (this);
	ASSERT (lpszText != NULL);

	BOOL bCopyRTFToClipboard = m_bCopyRTFToClipboard;

	if (bCopyRTFToClipboard && _tcsclen (lpszText) > _MAX_RTF_LEN)
	{
		bCopyRTFToClipboard = FALSE;
	}

	try
	{
		if (!OpenClipboard ())
		{
			TRACE0("Can't open clipboard\n");
			return FALSE;
		}

		if (!::EmptyClipboard ())
		{
			TRACE0("Can't empty clipboard\n");
			::CloseClipboard ();
			return FALSE;
		}

		if (nLen == -1)
		{
			nLen = (int) _tcslen (lpszText);
		}
		if (m_bBlockSelectionMode && 
			!CopyTextToClipboardInternal (lpszText, g_cFormat, nLen))
		{
			::CloseClipboard ();
			return FALSE;
		}

		if (bCopyRTFToClipboard)
		{
			CString strRTF;
			ExportToRTF (strRTF);
			UINT uFormat = RegisterClipboardFormat (CF_RTF);

			if (uFormat != 0)
			{
				if (!CopyTextToClipboardInternal (strRTF, (CLIPFORMAT) uFormat, strRTF.GetLength (), TRUE))
				{
					::CloseClipboard ();
					return FALSE;
				}
			}
		}

		if (!CopyTextToClipboardInternal (lpszText, _TCF_TEXT, nLen))
		{
			::CloseClipboard ();
			return FALSE;
		}

		::CloseClipboard ();
	}
	catch (...)
	{
		TRACE0("CopyTextToClipboard: out of memory\n");
	}

	return TRUE;
}
//*****************************************************************************
HGLOBAL CBCGPEditCtrl::CopyTextToClipboardInternal (LPCTSTR lpszText, 
													CLIPFORMAT cFormat, 
													int nLen, BOOL bForceAnsi)
{
	try
	{
		HGLOBAL hClipbuffer = ExportBuffer (lpszText, nLen, 
											FALSE, m_bReplaceTabsAndEOLOnCopy, bForceAnsi);
		if (hClipbuffer != NULL)
		{
			::GlobalUnlock (hClipbuffer);
			::SetClipboardData (cFormat, hClipbuffer);
		}

		return hClipbuffer;
	}
	catch (...)
	{
		TRACE0("CopyTextToClipboardInternal: out of memory\n");
	}
	return NULL;	
}
//*****************************************************************************
HGLOBAL CBCGPEditCtrl::ExportBuffer (LPCTSTR lpszText, int nTextLen, BOOL bReplaceTabs, 
									 BOOL bReplaceEOL, BOOL bForceAnsi)
{
	SIZE_T cbFinalSize = (nTextLen + 1) * 2 * sizeof(TCHAR);
	int nFinalLen = nTextLen;

	HGLOBAL hExpBuffer = ::GlobalAlloc (GMEM_DDESHARE, cbFinalSize);

	if (hExpBuffer == NULL)
	{
		TRACE0("ExportBuffer: out of memory\n");
		return NULL;
	}

	LPTSTR lpExpBuffer = (LPTSTR) GlobalLock (hExpBuffer);
	if (lpExpBuffer == NULL)
	{
		TRACE0("ExportBuffer: out of memory\n");
		GlobalFree (hExpBuffer);
		return NULL;
	}

	
	if (!bReplaceTabs && !bReplaceEOL)
	{
		memcpy (lpExpBuffer, lpszText, nTextLen * sizeof(TCHAR));
		lpExpBuffer [nTextLen] = _T ('\0');
		cbFinalSize = (nTextLen + 1) * sizeof(TCHAR);
	}
	else
	{
		int nSpaceCount = 0;
		int nExpIdx = 0;

		for (int nTextIdx = 0; nTextIdx < nTextLen; nTextIdx++, nExpIdx++)
		{
			if (bReplaceTabs && lpszText [nTextIdx] == _T (' '))
			{
				nSpaceCount++;
				if (nSpaceCount == m_nTabSize)
				{
					nExpIdx -= (m_nTabSize - 1);
					lpExpBuffer [nExpIdx] = _T ('\t');
					nSpaceCount = 0;
					continue;
				}
			}
			else
			{
				nSpaceCount = 0;
			}

			if (bReplaceEOL && lpszText [nTextIdx] == g_chEOL)
			{
				lpExpBuffer [nExpIdx] = g_strEOLExport [0];
				lpExpBuffer [++nExpIdx] = g_strEOLExport [1];
				continue;
			}

			lpExpBuffer [nExpIdx] = lpszText [nTextIdx];
		}

		lpExpBuffer [nExpIdx] = _T ('\0');
		cbFinalSize = (nExpIdx + 1) * sizeof(TCHAR);
		nFinalLen = nExpIdx;
	}

	if (bForceAnsi)
	{
#ifdef UNICODE
		// Convert buffer to ANSI:
		int nTextLenA = ::WideCharToMultiByte(::GetACP (), 0, lpExpBuffer, nFinalLen, NULL, 0, NULL, NULL);

		char* pszTextBufferA = new char [nTextLenA + 1];
		memset (pszTextBufferA, 0, (nTextLenA + 1) * sizeof(char));

		if (::WideCharToMultiByte(::GetACP (), 0, lpExpBuffer, nFinalLen, 
			pszTextBufferA, nTextLenA, NULL, NULL) == 0)
		{
			TRACE0 ("ExportBuffer failed - system convertion API failed\n");
			delete [] pszTextBufferA;
			GlobalFree (hExpBuffer);
			return NULL;
		}
		pszTextBufferA [nTextLenA] = '\0';

		char* lpAnsiBuffer = (char*)lpExpBuffer;
		memcpy (lpAnsiBuffer, (const char*)pszTextBufferA, nTextLenA * sizeof(char));
		lpAnsiBuffer [nTextLenA] = '\0';
		cbFinalSize = (nTextLenA + 1) * sizeof(char);

		delete [] pszTextBufferA;
#endif
	}

	GlobalUnlock (hExpBuffer);

	HGLOBAL hGlobalRet = GlobalReAlloc (hExpBuffer, cbFinalSize, 0);
	if (hGlobalRet == NULL)
	{
		return hExpBuffer;
	}
	return hGlobalRet;
}
//*****************************************************************************
void CBCGPEditCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnRButtonDown(nFlags, point);
	SetFocus ();

	if (m_iStartSel < 0 || !IsPointInSelectionRect (point))
	{
		RemoveSelection (FALSE, TRUE);

		CPoint ptSave = point;
		if (point.x >= 0 && point.x < m_rectText.left)
		{
			point.x = m_rectText.left + 1;
		}

		int nOffset = HitTest (point, TRUE);
		if (nOffset >= 0)
		{
			SetCaret (nOffset);
		}

		if (GetCapture () == this)
		{
			ReleaseCapture ();
			if (m_nScrollTimer != -1)
			{
				KillTimer (m_nScrollTimer);
				m_nScrollTimer = -1;
			}
		}
	}
}
//*****************************************************************************
void CBCGPEditCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDown(nFlags, point);
	if (GetFocus () != this)
	{
		SetFocus ();
	}
	
	BOOL bPerformDragDrop = FALSE;

	int iStartSel = -1;
	int iEndSel = -1;

	iStartSel = min (m_iEndSel, m_iStartSel);
	iEndSel = max (m_iEndSel, m_iStartSel);

	if (IsPointInSelectionRect (point) && (nFlags & MK_LBUTTON))
	{
		CString strText;
		PrepareBlock (strText);

		if (OpenClipboard ())
		{
			HGLOBAL hClipbufferText = ::GlobalAlloc (GMEM_DDESHARE, (strText.GetLength () + 1) * sizeof (TCHAR));
			LPTSTR lpszBufferText = (LPTSTR) GlobalLock (hClipbufferText);

			lstrcpy (lpszBufferText, strText);

			::GlobalUnlock (hClipbufferText);
			::CloseClipboard ();

			BOOL bRemoveSelection = FALSE;

			if (hClipbufferText != NULL)
			{
				CPoint ptStart = point;

				m_bDragTextMode = TRUE;
				bPerformDragDrop = TRUE;

				COleDataSource* pSrcItem = new COleDataSource();

				pSrcItem->CacheGlobalData (_TCF_TEXT, hClipbufferText);
				
				DROPEFFECT dropEffect = pSrcItem->DoDragDrop (DROPEFFECT_COPY | DROPEFFECT_MOVE);

				CPoint ptEnd;
				GetCursorPos (&ptEnd);
				ScreenToClient (&ptEnd);

				m_bDragTextMode = FALSE;

				if (!m_bReadOnly)
				{
					ShowCaret ();
				}

				if (dropEffect != DROPEFFECT_NONE)
				{
					int nLen = iEndSel - iStartSel;

					if (dropEffect == DROPEFFECT_MOVE)
					{
						// m_nDropOffset is -1 when the text is dropped to
						// another application

						if (iStartSel >= 0 && m_nDropOffset <= iStartSel &&
							m_nDropOffset != -1)
						{
							// moving text before selecttion, text inserted, section index should
							// be incremented
							m_iStartSel += nLen;
							m_iEndSel += nLen;
						}

						if (iStartSel >= 0 && iStartSel <= m_nDropOffset && 
							m_nDropOffset != -1)
						{
							// moving text after selection, text deleted, offset should be decremented
							m_nDropOffset -= nLen;
						}
						
						if (!m_bReadOnly)
						{
							// the third TRUE parameter will cause Undo action be played twise
							DeleteSelectedText (FALSE, FALSE);
						}
					}
					
					if (m_nDropOffset != -1)
					{
						if (m_bBlockSelectionMode)
						{
							OffsetToPoint (m_iStartSel, m_ptStartBlockSel);
							OffsetToPoint (m_iEndSel, m_ptEndBlockSel);
						}
						else
						{
							m_iStartSel = m_nDropOffset;
							m_iEndSel = m_nDropOffset + nLen;
							SetCaret (m_iEndSel);
						}
					}
					else if ((ptEnd.y - ptStart.y) == 0 && (ptEnd.x - ptStart.x) == 0)
					{
						bRemoveSelection = TRUE;
					}
					
					RedrawWindow ();
					m_nDropOffset = -1;
				}

				if ((ptEnd.y - ptStart.y) == 0 && (ptEnd.x - ptStart.x) == 0)
				{
					bRemoveSelection = TRUE;
				}

				pSrcItem->InternalRelease();
			}

			if (bRemoveSelection)
			{
				m_iStartSel = m_iEndSel = -1;
				m_bBlockSelectionMode = FALSE; 
				m_ptStartBlockSel = m_ptEndBlockSel = CPoint (-1, -1);
				RedrawWindow ();
				int nOffset = HitTest (point, TRUE);
				if (nOffset >= 0)
				{
					SetCaret (nOffset);
				}
			}
		}
		return;
	}

	CPoint ptSave = point;
	if (point.x >= 0 && point.x < m_rectText.left)
	{
		point.x = m_rectText.left + 1;
	}

	int nOffset = HitTest (point, TRUE);

	if (m_bEnableOutlining && 
		ptSave.x >= m_nLeftMarginWidth + (m_bEnableLineNumbersMargin ? m_nLineNumbersMarginAutoWidth : 0) && 
		ptSave.x < m_rectText.left)
	{
		if (OnOutlineButtonClick (nOffset))
		{
			return;
		}
	}

	const BOOL bCtrl = ::GetAsyncKeyState (VK_CONTROL) & 0x8000;
	if (m_bEnableHyperlinkSupport && bCtrl &&
		m_nCurrHyperlinkHot >= 0 && m_nCurrHyperlinkHot <= m_arrHyperlinks.GetUpperBound ())
	{
		BCGP_EDIT_HYPERLINK_DEF hyperlink = m_arrHyperlinks [m_nCurrHyperlinkHot];
		OnHyperlinkClick (hyperlink.m_nHyperlinkOffset, hyperlink.m_strHyperlink);
		return;
	}

	if (nOffset >= 0)
	{
		SetCaret (nOffset);
		if ((nFlags & MK_LBUTTON) && !bPerformDragDrop)
		{
			SetCapture ();
			if (::GetAsyncKeyState (VK_MENU) & 0x8000)
			{
#ifdef __ENABLE_BLOCK_SELECTION__						
				m_bBlockSelectionMode = TRUE;
				m_bAltPressedForBlockSel = TRUE;
#endif //__ENABLE_BLOCK_SELECTION__								
			}
		}
	}

	if (m_iStartSel >= 0)
	{
		m_iEndSel = m_iStartSel = -1;
		m_bBlockSelectionMode = FALSE;
		m_ptStartBlockSel = m_ptEndBlockSel = CPoint (-1, -1);
		RedrawWindow ();
	}
}
//************************************************************************************
void CBCGPEditCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (GetCapture () == this)
	{
		ReleaseCapture ();
	}

	if (m_nScrollTimer != -1)
	{
		KillTimer (m_nScrollTimer);
		m_nScrollTimer = -1;
	}

	m_nLastMaxColumn = GetColumnFromOffset (m_nCurrOffset, TRUE);

	if (m_iEndSel == m_iStartSel && m_iStartSel != -1)
	{
		BOOL bRedraw = m_iEndSel != -1;
		m_iEndSel = m_iStartSel = -1;
		m_bBlockSelectionMode = FALSE;
		m_ptStartBlockSel = m_ptEndBlockSel = CPoint (-1, -1);
		if (bRedraw)
		{
			RedrawWindow ();
		}
	}
	else
	{
		CBCGPOutlineNode* pNode = FindCollapsedBlockByPoint(point, m_nCurrOffset);
		if (pNode != NULL)
		{
			SetSel (pNode->m_nStart, pNode->m_nEnd + 1);
		}
	}

	CWnd::OnLButtonUp(nFlags, point);
}
//************************************************************************************
void CBCGPEditCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	int nOffset = HitTest (point);
	
	if (nOffset >= 0 && nOffset < m_strBuffer.GetLength () && !m_bIntelliSenseMode)
	{
		CBCGPOutlineNode* pNode = FindCollapsedBlockByPoint(point, nOffset);
		if (pNode != NULL)
		{
			OnOutlineButtonClick (pNode);
		}

		else
		{
			TCHAR ch = m_strBuffer.GetAt (nOffset);

			bool bWordFound = false;
			int nStartSel = -1;
			int nEndSel = -1;
			
			if (ch == _T (' ') || ch == _T ('\t') || ch == _T ('\n'))
			{
				nStartSel = WhileOneOf (m_strBuffer, nOffset, false, _T (" \t"));
				nEndSel = WhileOneOf (m_strBuffer, nOffset, true, _T (" \t"));

				if (nEndSel - nStartSel > 1)
				{
					m_iStartSel = nStartSel;
					m_iEndSel = nEndSel;
					bWordFound = true;
				}
				else
				{
					m_iStartSel = nOffset;
					m_iEndSel = nOffset;
					bWordFound = true;
				}
			}

			if (!bWordFound)
			{
				if (m_strNonSelectableChars.Find (ch) != -1)
				{
					int nNextWord = WhileOneOf (m_strBuffer, nOffset, true, m_strNonSelectableChars);
					if (nNextWord == m_strBuffer.GetLength ())
					{
						nNextWord = WhileOneOf (m_strBuffer, nOffset, false, m_strNonSelectableChars);
					}
					nOffset = nNextWord;
				}
				
				FindWordStartFinish (nOffset, m_strBuffer, m_iStartSel, m_iEndSel);
			}

			SetCaret (m_iEndSel);
			RedrawWindow ();
		}
	}
	
	CWnd::OnLButtonDblClk(nFlags, point);
}
//************************************************************************************
void CBCGPEditCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	int nOldMinSel = min (m_iStartSel, m_iEndSel);
	int nOldMaxSel = max (m_iStartSel, m_iEndSel);
	
	if ((nFlags & MK_LBUTTON) && !m_bIntelliSenseMode && GetCapture () == this)
	{
		if (!m_rectText.PtInRect (point) && m_nScrollTimer == -1)
		{
			m_nScrollTimer = SetTimer (ID_TEXT_SCROLL, m_nScrollTimeOut, NULL);
		}
		else if (m_nScrollTimer != -1 && m_rectText.PtInRect (point))
		{
			KillTimer (m_nScrollTimer);
			m_nScrollTimer = -1;
		}

		if (point.x < m_rectText.left)
		{
			point.x = m_rectText.left + 1;
		}
		if (point.x > m_rectText.right)
		{
			point.x = m_rectText.right;
		}

		CPoint ptCaret = point;
		int nOffset = HitTest (ptCaret, TRUE, TRUE);

		if (m_bBlockSelectionMode)
		{
			CPoint ptOld = m_ptEndBlockSel;

			if (m_iStartSel < 0)
			{
				m_iStartSel = m_iEndSel = m_nCurrOffset;
				OffsetToPoint (m_iStartSel, m_ptStartBlockSel);
				m_ptStartBlockSel.x += m_nScrollOffsetHorz * m_nMaxCharWidth;
				m_ptStartBlockSel.y += m_nScrollOffsetVert * m_nLineHeight;

				m_ptEndBlockSel = m_ptStartBlockSel;
			}
			else
			{
				if (nOffset >= 0)
				{
					CRect rectOld (m_ptStartBlockSel, m_ptEndBlockSel);

					if (nOffset > -1)
					{
						SetCaret (nOffset, TRUE, FALSE);
					}

					m_iEndSel = nOffset;
					m_ptEndBlockSel = point;

					CPoint pt;
					OffsetToPoint (m_iEndSel, pt);
					m_ptEndBlockSel.y = pt.y + m_nLineHeight;

					if (abs (pt.x - m_ptEndBlockSel.x) > m_nMaxCharWidth)					
					{
						pt.x = m_ptEndBlockSel.x;
					}
					else
					{
						m_ptEndBlockSel.x = pt.x;
					}
					
					SetCaretPos (pt);


					m_ptEndBlockSel.x += m_nScrollOffsetHorz * m_nMaxCharWidth;
					m_ptEndBlockSel.y += m_nScrollOffsetVert * m_nLineHeight;

					if (m_ptEndBlockSel != ptOld)
					{
						RedrawWindow (m_rectText);
					}
				}
			}
		}
		else
		{
			if (m_iStartSel < 0)
			{
				m_iStartSel = m_iEndSel = m_nCurrOffset;
			}
			else
			{
				if (nOffset >= 0)
				{
					m_iEndSel = nOffset;
					SetCaret (nOffset);
					if (m_nLastSelOffset != nOffset)
					{
						int nMinNewSel = min (m_iEndSel, m_iStartSel);
						int nMaxNewSel = max (m_iEndSel, m_iStartSel);
						//RedrawWindow ();
						RedrawTextOffsets (min (nOldMinSel, nMinNewSel), 
										   max (nOldMaxSel, nMaxNewSel));
					}
				}

				m_nLastSelOffset = nOffset;
			}
		}
	}

	if (m_bEnableToolTips && GetCapture () != this)
	{
		CString strText;
		BOOL bIsHyperlink = m_bEnableHyperlinkSupport && GetHyperlinkToolTip (strText);
		BOOL bIsHiddenTextFromPoint = !bIsHyperlink && m_bEnableOutlining && GetHiddenTextFromPoint (point, strText);
		BOOL bIsWordFromPoint = !bIsHyperlink && !bIsHiddenTextFromPoint && GetWordFromPoint (point, strText);

		if (!(bIsHyperlink || bIsWordFromPoint || bIsHiddenTextFromPoint) ||
			!(strText == strTipText || 
			  (OnGetTipText (strText) &&
			   strText == strTipText)))
		{
			ToolTipPop ();
		}			

		if (!(bIsHyperlink || bIsWordFromPoint || bIsHiddenTextFromPoint))
		{
			m_strLastDisplayedToolTip.Empty ();
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}
//*************************************************************************************
void CBCGPEditCtrl::OnTimer(UINT_PTR nIDEvent) 
{
	switch (nIDEvent)
	{
	case ID_TEXT_SCROLL:
		{
			CPoint ptMouse;
			GetCursorPos (&ptMouse);
			ScreenToClient (&ptMouse);
			PostMessage (WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM (ptMouse.x, ptMouse.y));
		}
		break;
	}
	
	CWnd::OnTimer(nIDEvent);
}
//*************************************************************************************
void CBCGPEditCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);

	if (pNewWnd == GetParent () && pNewWnd != NULL &&
		pNewWnd->IsKindOf (RUNTIME_CLASS (CView)))
	{
		return;
	}

	m_nSavedOffset = m_nCurrOffset;
	m_ptSavedBlockCaretOffset = GetCaretPos ();
	m_ptSavedBlockCaretOffset.x += m_nScrollOffsetHorz * m_nMaxCharWidth;
	m_ptSavedBlockCaretOffset.y += m_nScrollOffsetVert * m_nLineHeight;

	if (/*m_bClearSelectionWhenInactive*/FALSE)
	{
		m_iStartSel = m_iEndSel = -1;
		m_bBlockSelectionMode = FALSE;
		m_ptStartBlockSel = m_ptEndBlockSel = CPoint (-1, -1);
	}

	RedrawWindow ();

	if (!m_bIntelliSenseMode && !m_bReadOnly)
	{
		::DestroyCaret ();
	}
}
//*************************************************************************************
void CBCGPEditCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	if (!m_bReadOnly)
	{
		::DestroyCaret ();
		CSize sizeCaret = GetCaretSize ();
		::CreateCaret (GetSafeHwnd (), NULL, sizeCaret.cx, sizeCaret.cy);

		if (m_bBlockSelectionMode)
		{
			m_ptSavedBlockCaretOffset.x -= m_nScrollOffsetHorz * m_nMaxCharWidth;
			m_ptSavedBlockCaretOffset.y -= m_nScrollOffsetVert * m_nLineHeight;
			SetCaretPos (m_ptSavedBlockCaretOffset);
		}
		else
		{
			CPoint pt;
			OffsetToPoint (m_nCurrOffset, pt);

			BOOL bInRect = m_rectText.PtInRect (pt);
			SetCaret (m_nCurrOffset, bInRect);
			
			CPoint ptCurrPos = GetCaretPos ();
			if (!m_rectText.PtInRect (pt))
			{
				SetCaretPos (pt);
			}
		}
		
		
		ShowCaret ();
	}

	RedrawWindow ();
}
//*************************************************************************************
void CBCGPEditCtrl::GetSel (int& nStartChar, int& nEndChar) const
{
	nStartChar = m_iStartSel;
	nEndChar = m_iEndSel;
}
//*************************************************************************************
BOOL CBCGPEditCtrl::GetSelectionRect (CRect& rect1, CRect& rect2, CRect& rect3) 
{
	ASSERT_VALID (this);

	rect1.SetRectEmpty ();
	rect2.SetRectEmpty ();
	rect3.SetRectEmpty ();

	if (m_iStartSel < 0 || m_iEndSel < 0)
	{
		return FALSE;
	}

	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel);

	CPoint ptStart;
	OffsetToPoint (iStartSel, ptStart);

	CPoint ptEnd;
	OffsetToPoint (iEndSel, ptEnd);

	if (ptStart.y == ptEnd.y)
	{
		// Single row.
		rect1 = CRect (ptStart.x, ptStart.y, ptEnd.x, ptStart.y + m_nLineHeight);
	}
	else
	{
		ASSERT (ptStart.y < ptEnd.y);

		rect1 = CRect (ptStart.x, ptStart.y, m_rectText.right, ptStart.y + m_nLineHeight);
		rect2 = CRect (m_rectText.left, ptStart.y + m_nLineHeight,m_rectText.right, ptEnd.y);
		rect3 = CRect (m_rectText.left, ptEnd.y, ptEnd.x, ptEnd.y + m_nLineHeight);
	}

	return TRUE;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::ScrollText (BOOL bDirection)
{
	bDirection ? ScrollDown (SB_VERT, TRUE) : ScrollUp (SB_VERT, TRUE);

	CRect rectText = m_rectText;
	CPoint pt;
	OffsetToPoint (m_nCurrOffset, pt);
	if (!bDirection)
	{
		// if scrolling up always keep caret one-two lines off the bottom
		rectText.bottom -= m_nLineHeight * 2;
	}
	
	if (rectText.PtInRect (pt))
	{
		SetCaret (m_nCurrOffset, FALSE, FALSE);
		OffsetToPoint (m_nCurrOffset, pt);
		SetCaretPos (pt);
	}
	else
	{
		bDirection ? pt.y += m_nLineHeight : pt.y -= m_nLineHeight;
		int nOffset = HitTest (pt);
		if (!SetCaretToLastMaxColumn (nOffset))
		{
			SetCaret (nOffset, FALSE, FALSE);
			SetCaretPos (pt);
		}
	}
	return TRUE;
}
//***************************************************************************************
void CBCGPEditCtrl::UpdateScrollBars ()
{
	ASSERT_VALID (this);

	if (m_nMaxCharWidth == 0 || m_nLineHeight == 0)
	{
		ASSERT (FALSE);
		return;
	}

//	BOOL bScrollChanged = FALSE;

	//---------------------------
	// Recalc editor client view:
	//---------------------------
	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectTextPadding;
	rectTextPadding.SetRectEmpty ();

	rectTextPadding.left += m_nLeftMarginWidth;
	if (m_bEnableLineNumbersMargin)
	{
		rectTextPadding.left += m_nLineNumbersMarginAutoWidth;
	}
	if (m_bEnableOutlineMargin)
	{
		rectTextPadding.left += m_nOutlineMarginWidth;
	}

	m_rectView = rectClient;
	m_rectText = m_rectView;
	m_rectText.DeflateRect (&rectTextPadding);

	//----------------------
	// Calculate scrollbars:
	//----------------------
	CRect rectScrollText = m_rectText;
	rectScrollText.right -= ::GetSystemMetrics (SM_CXVSCROLL);
	rectScrollText.bottom -= ::GetSystemMetrics (SM_CYHSCROLL);

	int nHiddenLines = (m_bEnableOutlining ? m_nHiddenLines : 0);
	m_szTotalScroll = CSize (m_nMaxScrollWidth, m_nTotalLines - nHiddenLines);

	int nBufLen = m_strBuffer.GetLength ();
	if (nBufLen > 0 && m_strBuffer [nBufLen - 1] != g_chEOL)
	{
		m_szTotalScroll.cy++;
	}

	//-----------------
	// Set scroll info:
	//-----------------
	SCROLLINFO siHorz;
	memset (&siHorz, 0, sizeof (SCROLLINFO));
	siHorz.cbSize = sizeof (SCROLLINFO);
	siHorz.fMask = SIF_RANGE | SIF_PAGE;
	siHorz.nMin = 0;
	siHorz.nMax = m_szTotalScroll.cx;
	siHorz.nPage = rectScrollText.Width () / m_nMaxCharWidth;

	if (!m_wndScrollHorz.SetScrollInfo (&siHorz))
	{
		m_wndScrollHorz.SetScrollRange (0, siHorz.nMax, TRUE);
	}

	SCROLLINFO siVert;
	memset (&siVert, 0, sizeof (SCROLLINFO));
	siVert.cbSize = sizeof (SCROLLINFO);
	siVert.fMask = SIF_RANGE | SIF_PAGE;
	siVert.nMin = 0;
	siVert.nMax = m_szTotalScroll.cy;
	siVert.nPage = rectScrollText.Height () / m_nLineHeight;
	siVert.nMax = m_szTotalScroll.cy + (m_bScrollVertEmptyPage ? siVert.nPage : 0);

	if (!m_wndScrollVert.SetScrollInfo (&siVert))
	{
		m_wndScrollVert.SetScrollRange (0, siVert.nMax, TRUE);
	}

	//----------------------------
	// Set position of scrollbars:
	//----------------------------
	//if (bScrollChanged)
	{
		CRect rectScrollView;
		AdjustScrollBars (&rectScrollView);

		m_rectView = rectScrollView;
		m_rectText = m_rectView;
		m_rectText.DeflateRect (&rectTextPadding);
	}
}
//***************************************************************************************
void CBCGPEditCtrl::AdjustScrollBars (CRect* pRectView)
{
	ASSERT_VALID (this);

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectView;
	rectView = rectClient;

	//----------------------------
	// Set position of scrollbars:
	//----------------------------
	const int cxScroll = ::GetSystemMetrics (SM_CXVSCROLL);
	const int cyScroll = ::GetSystemMetrics (SM_CYHSCROLL);

	SCROLLINFO si;
	memset (&si, 0, sizeof (SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_PAGE;

	m_wndScrollHorz.GetScrollInfo (&si);

	BOOL bScrollHorz = (si.nMax > 0 && (int)si.nPage < si.nMax) && m_bScrollBarHorz;

	m_wndScrollVert.GetScrollInfo (&si);

	BOOL bScrollVert = (si.nMax > 0 && (int)si.nPage < si.nMax);

	CSize szScroll (0, 0);
	if (bScrollVert)
	{
		if (globalData.m_bIsRTL)
		{
			rectView.left += cxScroll;
		}
		else
		{
			rectView.right -= cxScroll;
		}
		szScroll.cy = rectClient.Height ();
	}
	if (bScrollHorz)
	{
		rectView.bottom -= cyScroll;
		szScroll.cx = rectView.Width ();
		szScroll.cy = max (0, szScroll.cy - cyScroll);
	}

	if (m_wndScrollVert.GetSafeHwnd () != NULL)
	{
		m_wndScrollVert.EnableScrollBar (m_szTotalScroll.cy > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);

		if (bScrollVert)
		{
			m_wndScrollVert.SetWindowPos (NULL,
				globalData.m_bIsRTL ? rectClient.left : rectView.right, rectView.top,
				cxScroll, szScroll.cy, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else
		{
			m_wndScrollVert.SetWindowPos (NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
		}

		InvalidateRect(CRect (rectClient.right - cxScroll, rectClient.top, rectClient.right, rectClient.bottom));
	}

	if (m_wndScrollHorz.GetSafeHwnd () != NULL)
	{
		m_wndScrollHorz.EnableScrollBar (m_szTotalScroll.cx > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);

		if (bScrollHorz)
		{
			m_wndScrollHorz.SetWindowPos (NULL,
				rectView.left, rectClient.bottom - cyScroll,
				szScroll.cx, cyScroll, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else
		{
			m_wndScrollHorz.SetWindowPos (NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	if (pRectView != NULL)
	{
		*pRectView = rectView;
	}
}
//***************************************************************************************
CScrollBar* CBCGPEditCtrl::GetScrollBarCtrl(int nBar) const
{
	if (nBar == SB_HORZ && m_wndScrollHorz.GetSafeHwnd () != NULL)
	{
		return (CScrollBar* ) &m_wndScrollHorz;
	}
	
	if (nBar == SB_VERT && m_wndScrollVert.GetSafeHwnd () != NULL)
	{
		return (CScrollBar* ) &m_wndScrollVert;
	}

	return NULL;
}
//***************************************************************************************
void CBCGPEditCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
{
	if (m_bBlockSelectionMode)
	{
		m_ptSavedBlockCaretOffset = GetCaretPos ();
		m_ptSavedBlockCaretOffset.x += m_nScrollOffsetHorz * m_nMaxCharWidth;
		m_ptSavedBlockCaretOffset.y += m_nScrollOffsetVert * m_nLineHeight;
	}

	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_TRACKPOS;

	if (nSBCode == SB_THUMBTRACK || nSBCode == SB_THUMBPOSITION)
	{
		m_wndScrollVert.GetScrollInfo(&info);
		nPos = info.nTrackPos;
	}

	int nScrollOffsetVertOld = m_nScrollOffsetVert;

	BOOL bScroll = OnScroll (SB_VERT, nSBCode, nPos);
	CPoint pt;
	OffsetToPoint (m_nCurrOffset, pt);

	if (bScroll)
	{
		ToolTipPop ();
	
		ScrollWindow (0, (nScrollOffsetVertOld - m_nScrollOffsetVert) * m_nLineHeight, m_rectView, m_rectView);

		if (m_bEnableOutlineMargin)
		{
			int nMargin = m_nLeftMarginWidth;

			// ---------------------------
			// Draw line numbers side bar:
			// ---------------------------
			if (m_bEnableLineNumbersMargin)
			{
				nMargin += m_nLineNumbersMarginAutoWidth;
			}

			CRect rectLeft;
			GetClientRect (rectLeft);

			rectLeft.left = rectLeft.left + nMargin;
			rectLeft.right = rectLeft.left + m_nOutlineMarginWidth;
		
			RedrawWindow (rectLeft);
		}

		if (!m_bBlockSelectionMode)
		{
			if (m_rectText.PtInRect (pt))
			{
				SetCaretPos (pt);
				m_ptCaret = pt;
			}	
			else
			{
				SetCaretPos (CPoint (-10, -10));
			}
		}
		else
		{
			m_ptSavedBlockCaretOffset.x -= m_nScrollOffsetHorz * m_nMaxCharWidth;
			m_ptSavedBlockCaretOffset.y -= m_nScrollOffsetVert * m_nLineHeight;
			SetCaretPos (m_ptSavedBlockCaretOffset);
		}
	}

	m_wndScrollHorz.RedrawWindow ();
}
//***************************************************************************************
void CBCGPEditCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
{
	if (m_bBlockSelectionMode)
	{
		m_ptSavedBlockCaretOffset = GetCaretPos ();
		m_ptSavedBlockCaretOffset.x += m_nScrollOffsetHorz * m_nMaxCharWidth;
		m_ptSavedBlockCaretOffset.y += m_nScrollOffsetVert * m_nLineHeight;
	}

	BOOL bScroll = OnScroll (SB_HORZ, nSBCode, nPos);
	CPoint pt;
	OffsetToPoint (m_nCurrOffset, pt);

	if (bScroll)
	{
		ToolTipPop ();
		RedrawWindow ();
		
		if (!m_bBlockSelectionMode)
		{
			if (m_rectText.PtInRect (pt))
			{
				SetCaretPos (pt);
				m_ptCaret = pt;
			}	
			else
			{
				SetCaretPos (CPoint (-10, -10));
			}
		}
		else
		{
			m_ptSavedBlockCaretOffset.x -= m_nScrollOffsetHorz * m_nMaxCharWidth;
			m_ptSavedBlockCaretOffset.y -= m_nScrollOffsetVert * m_nLineHeight;
			SetCaretPos (m_ptSavedBlockCaretOffset);
		}
	}		
}
//***************************************************************************************
BOOL CBCGPEditCtrl::ScrollDown (int fnBar, BOOL bRedrawOnScroll)
{
	BOOL bScroll = OnScroll (fnBar, SB_LINEDOWN);
	if (bScroll && bRedrawOnScroll)
	{
		RedrawWindow ();
	}
	return bScroll;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::ScrollUp (int fnBar, BOOL bRedrawOnScroll)
{
	BOOL bScroll = OnScroll (fnBar, SB_LINEUP);
	if (bScroll && bRedrawOnScroll)
	{
		RedrawWindow ();
	}
	return bScroll;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::ScrollPageDown (int fnBar, BOOL bRedrawOnScroll)
{
	BOOL bScroll = OnScroll (fnBar, SB_PAGEDOWN);
	if (bScroll && bRedrawOnScroll)
	{
		RedrawWindow ();
	}
	return bScroll;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::ScrollPageUp (int fnBar, BOOL bRedrawOnScroll)
{
	BOOL bScroll = OnScroll (fnBar, SB_PAGEUP);
	if (bScroll && bRedrawOnScroll)
	{
		RedrawWindow ();
	}
	return bScroll;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::ScrollHome (int fnBar, BOOL bRedrawOnScroll)
{
	BOOL bScroll = OnScroll (fnBar, SB_TOP);
	if (bScroll && bRedrawOnScroll)
	{
		RedrawWindow ();
	}
	return bScroll;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::ScrollEnd (int fnBar, BOOL bRedrawOnScroll)
{
	BOOL bScroll = OnScroll (fnBar, SB_BOTTOM);
	if (bScroll && bRedrawOnScroll)
	{
		RedrawWindow ();
	}
	return bScroll;
}
//***************************************************************************************
void CBCGPEditCtrl::SetMouseWheelSpeed (int nSteps)
{
	if (nSteps > 0)
	{
		m_nScrollMouseWheelSpeed = nSteps;
	}
}
//***************************************************************************************
BOOL CBCGPEditCtrl::OnScroll (int fnBar, UINT nSBCode, int nPos)
{
	if (fnBar == SB_CTL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	int& nScrollOffset = (fnBar == SB_VERT) ? m_nScrollOffsetVert : m_nScrollOffsetHorz;
	CBCGPScrollBar& wndScroll = (fnBar == SB_VERT) ? m_wndScrollVert : m_wndScrollHorz;

	SCROLLINFO scrollInfo;
	ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));

    scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_ALL;

	wndScroll.GetScrollInfo (&scrollInfo);

	int nScrollOffsetOld = nScrollOffset;

	switch (nSBCode)
	{
	case SB_LINEUP:
		nScrollOffset = max (0, nScrollOffset - 1);
		break;

	case SB_LINEDOWN:
		nScrollOffset++;
		break;

	case SB_TOP:
		nScrollOffset = scrollInfo.nMin;
		break;

	case SB_BOTTOM:
		nScrollOffset = scrollInfo.nMax;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nScrollOffset = nPos;
		break;

	case SB_PAGEDOWN:
		nScrollOffset += scrollInfo.nPage - 1;
		break;

	case SB_PAGEUP:
		nScrollOffset -= scrollInfo.nPage - 1;
		break;

	default:
		return FALSE;
	}

	nScrollOffset = min (max (scrollInfo.nMin, nScrollOffset), scrollInfo.nMax - 1);
	nScrollOffset = max (nScrollOffset, 0);

	if (nScrollOffset == nScrollOffsetOld)
	{
		return FALSE;
	}

	if (fnBar == SB_VERT)
	{
		int nActualScroll = nScrollOffset - nScrollOffsetOld;
		bool bDirection = nActualScroll > 0; 

		// find the next visible offset
		int nNextOffset = m_nTopVisibleOffset;
		for (int iIdxRow = 0; iIdxRow < abs (nActualScroll); )
		{
			if (bDirection)
			{
				if (nNextOffset >= m_strBuffer.GetLength ())
				{
					nScrollOffset = nScrollOffsetOld;
					return FALSE;
				}

				if (m_strBuffer.GetAt (nNextOffset) == g_chEOL)
				{
					nNextOffset++;

					CBCGPOutlineNode* pCollapsedBlock = FindCollapsedBlock (nNextOffset);
					if (pCollapsedBlock == NULL ||
						pCollapsedBlock->m_nStart == nNextOffset)
					{
						iIdxRow++;
						continue;
					}
				}

				nNextOffset = m_strBuffer.Find (g_strEOL, nNextOffset + 1);
			}
			else
			{
				nNextOffset = ReverseFindOneOf (m_strBuffer, nNextOffset - 2, g_strEOL);
			}

			if (nNextOffset == -1)
			{
				nNextOffset = bDirection ? m_strBuffer.GetLength () : 0;
				break;
			}

			if (bDirection)
			{
				nNextOffset++;
			}

			CBCGPOutlineNode* pCollapsedBlock = FindCollapsedBlock (nNextOffset);
			if (pCollapsedBlock == NULL ||
				pCollapsedBlock->m_nStart == nNextOffset)
			{
				iIdxRow++;
			}
		}

		m_nTopVisibleOffset = nNextOffset;
		m_nScrollHiddenVert = RowFromOffset (m_nTopVisibleOffset, TRUE, FALSE) - nScrollOffset;
	}

	wndScroll.SetScrollPos (nScrollOffset);

	return TRUE;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/) 
{
#ifndef _BCGPEDIT_STANDALONE
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return TRUE;
	}
#endif

	SCROLLINFO scrollInfo;
	ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));

    scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_ALL;

	GetScrollInfo (SB_VERT, &scrollInfo);

	int nHiddenLines = (m_bEnableOutlining ? m_nHiddenLines : 0);
	if (scrollInfo.nPage > (UINT) (m_nTotalLines - nHiddenLines) ||
		scrollInfo.nPos >= scrollInfo.nMax)
	{
		return FALSE;
	}

	int nSteps = abs(zDelta) / WHEEL_DELTA;
	nSteps *= m_nScrollMouseWheelSpeed;

	if (nSteps > 1)
	{
		SCROLLINFO siVert;
		memset (&siVert, 0, sizeof (SCROLLINFO));
		siVert.cbSize = sizeof (SCROLLINFO);
		siVert.fMask = SIF_POS;
		siVert.nPos = (zDelta < 0) ? (m_nScrollOffsetVert + nSteps) : (m_nScrollOffsetVert - nSteps);

		SetScrollInfo (SB_VERT, &siVert);

		OnVScroll (SB_THUMBTRACK, 0, NULL);
	}
	else
	{
		OnVScroll (zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, NULL);
	}

	return TRUE;
}
//***************************************************************************************
void CBCGPEditCtrl::SetSel2 (int nStartSel, int nEndSel, BOOL bNoScroll, BOOL bRedraw)
{
	ASSERT_VALID (this);

	if (m_strBuffer.IsEmpty ())
	{
		return;
	}

	if (nStartSel == -1)
	{
		m_iStartSel = m_iEndSel = -1;
		if (bRedraw)
		{
			RedrawWindow ();
		}
		return;
	}

	if (nStartSel < 0)
	{
		nStartSel = 0;
	}

	if (nEndSel == -1)
	{
		nEndSel = m_strBuffer.GetLength ();
	}

	if (nEndSel > m_strBuffer.GetLength ())
	{
		nEndSel = m_strBuffer.GetLength ();
	}

	m_iStartSel = nStartSel;
	m_iEndSel = nEndSel;

	if (!bNoScroll)
	{
		SetCaret (nEndSel, TRUE, FALSE);
	}

	if (bRedraw)
	{
		RedrawWindow ();
	}
}
//***************************************************************************************
BOOL CBCGPEditCtrl::MakeSelection (enum CBCGPEditCtrl::BCGP_EDIT_SEL_TYPE selType)
{
	if (m_nCurrOffset == m_strBuffer.GetLength () && 
		(selType == ST_NEXT_WORD ||
		 selType == ST_NEXT_SYMBOL ||
		 selType == ST_NEXT_LINE ||
		 selType == ST_NEXT_PAGE))
	{
		OnFailedOperation (g_dwOpMakeSel);
		return FALSE;
	}


	int nOldOffset = m_nCurrOffset;

	if (m_iStartSel == -1)
	{
		m_iStartSel = nOldOffset;
	}

	BOOL bResult = FALSE;
	BOOL bRedrawRestOfText = FALSE;
	BOOL bCaretHidden = FALSE;

	int nOldVertScroll = m_nScrollOffsetVert;
	int nOldHorzScroll = m_nScrollOffsetHorz;

	switch (selType)
	{
	case ST_PREV_WORD:
		bResult = PrevWord (FALSE);
		bRedrawRestOfText = TRUE;
		break;

	case ST_NEXT_WORD:
		bResult = NextWord (FALSE);
		bRedrawRestOfText = TRUE;
		break;

	case ST_PREV_SYMBOL:
		HideCaret ();
		bResult = Left (FALSE);
		bRedrawRestOfText = TRUE;
		bCaretHidden = TRUE;
		break;

	case ST_NEXT_SYMBOL:
		HideCaret ();
		bResult = Right (FALSE);
		bRedrawRestOfText = TRUE;
		bCaretHidden = TRUE;
		break;
	
	case ST_PREV_LINE:
		bResult = Up (TRUE, FALSE);
		bRedrawRestOfText = TRUE;
		break;

	case ST_NEXT_LINE:
		bResult = Down (TRUE, FALSE);
		bRedrawRestOfText = TRUE;
		break;
	
	case ST_PREV_PAGE:
		bResult = PageUp (FALSE);
		break;

	case ST_NEXT_PAGE:
		bResult = PageDown (FALSE);
		break;

	case ST_HOME:
		bResult = Home (FALSE);
		break;

	case ST_END:
		bResult = End (FALSE);
		break;

	case ST_START_OF_TEXT:
		bResult = StartOfText ();
		break;

	case ST_END_OF_TEXT:
		bResult = EndOfText (FALSE);
		break;

	case ST_ALL_TEXT:
		SetCaret (0, TRUE);
		m_iStartSel = m_strBuffer.GetLength ();
		m_iEndSel = 0;
		RedrawWindow ();
		return TRUE;
	}

	if (bResult)
	{
		m_iEndSel = m_nCurrOffset;
		
		if (bRedrawRestOfText && 
			nOldVertScroll == m_nScrollOffsetVert &&
			nOldHorzScroll == m_nScrollOffsetHorz)
		{
			int nMinOffset = min (nOldOffset, min (m_iEndSel, min (m_iStartSel, m_nCurrOffset)));
			int nMaxOffset = max (nOldOffset, max (m_iEndSel, max (m_iStartSel, m_nCurrOffset)));
			RedrawTextOffsets (nMinOffset, nMaxOffset);

			//RedrawRestOfText (min (nOldOffset, min (m_iEndSel, min (m_iStartSel, m_nCurrOffset))));
		}
		else
		{
			RedrawWindow ();
		}
	}
	

	if (bCaretHidden)
	{
		ShowCaret ();
	}

	if (!bResult)
	{
		OnFailedOperation (g_dwOpMakeSel);
	}

	return bResult;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::RemoveSelection (BOOL bSetCaretToSelStart, BOOL bKeepCurrPos,
									 BOOL bRedraw)
{
	if (m_iStartSel != -1 || m_ptStartBlockSel.x != -1)
	{
		if (m_iStartSel > m_iEndSel)
		{
			bSetCaretToSelStart = !bSetCaretToSelStart;
		}

		int nNewOffset = m_iStartSel;
		m_iStartSel = m_iEndSel = -1;
		m_bBlockSelectionMode = FALSE;
		m_ptStartBlockSel = m_ptEndBlockSel = CPoint (-1, -1);
		if (bRedraw)
		{
			RedrawWindow ();
		}

		if (bSetCaretToSelStart && !bKeepCurrPos)
		{
			SetCaret (nNewOffset, TRUE);
		}
		
		return TRUE;
	}
	
	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IsIndentEnabled (BOOL bForward)
{
	if (m_bBlockSelectionMode)
	{
		return FALSE;
	}

	if (m_iStartSel != m_iEndSel)
	{
		int nStartOffset = min (m_iStartSel, m_iEndSel);
		int nCurOffset = max (m_iStartSel, m_iEndSel);

		if ((nStartOffset < 0) || (nStartOffset >= m_strBuffer.GetLength()))
		{
			return FALSE;
		}

		if (nCurOffset >= m_strBuffer.GetLength())
		{
			if (bForward)
			{
				return nStartOffset != nCurOffset;
			}

			nCurOffset = m_strBuffer.GetLength();
		}
	
		CString strSubNonSelectableChars;

		int ind = m_strNonSelectableChars.GetLength();
		while (ind-- > 0)
		{
			if (m_strIndentChars.Find(m_strNonSelectableChars.GetAt(ind)) == -1)
			{
				strSubNonSelectableChars += m_strNonSelectableChars.GetAt(ind);
			}
		}

		const int ciStartSel = nStartOffset;
		const int ciEndSel = nCurOffset;

		ReverseFindOneOf(strSubNonSelectableChars, ciStartSel, nCurOffset);

		if (bForward)
		{
			if (nCurOffset >= ciStartSel)
			{
				return TRUE;
			}
		}
		else
		{
			const int nNext = 1;
			const int ciEOLOffset = ((nCurOffset >= ciStartSel)? nCurOffset: -1);

			if (ciEOLOffset > ciStartSel)
			{
				if (ciEOLOffset + nNext != ciEndSel &&
					m_strIndentChars.Find(m_strBuffer.GetAt(ciEOLOffset + nNext)) != -1)
				{
					return TRUE;
				}

				do
				{
					do
					{
						ReverseFindOneOf(strSubNonSelectableChars, ciStartSel, nCurOffset);
					}
					while (nCurOffset >= ciStartSel &&
						   m_strIndentChars.Find(m_strBuffer.GetAt(nCurOffset + nNext)) == -1);

					if (nCurOffset >= ciStartSel &&
						m_strIndentChars.Find(m_strBuffer.GetAt(nCurOffset + nNext)) != -1)
					{
						return TRUE;
					}
					else if (nCurOffset < ciStartSel)
					{
						ReverseFindOneOf(strSubNonSelectableChars, 0, nCurOffset = ciStartSel);

						if (m_strIndentChars.Find(m_strBuffer.GetAt(nCurOffset + nNext)) != -1)
						{
							return TRUE;
						}
					}
				}
				while (nCurOffset >= ciStartSel);
			}
			else if (ciEOLOffset == -1)
			{
				if (ciEndSel == m_strBuffer.GetLength())
				{
					ReverseFindOneOf(strSubNonSelectableChars, 0, nStartOffset);

					if (m_strIndentChars.Find(m_strBuffer.GetAt(nStartOffset + nNext)) != -1)
					{
						return TRUE;
					}
				}
			}
			else if (ciEOLOffset + nNext != ciEndSel &&
					 m_strIndentChars.Find(m_strBuffer.GetAt(nCurOffset + nNext)) != -1)
			{
				return TRUE;
			}
			else
			{
				ReverseFindOneOf(strSubNonSelectableChars, 0, nCurOffset);

				if (m_strIndentChars.Find(m_strBuffer.GetAt(nCurOffset + nNext)) != -1)
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;	
}

//**************************************************************************************
BOOL CBCGPEditCtrl::IndentSelection (BOOL bForward)
{
	if (!IsIndentEnabled(bForward))
	{
		OnFailedOperation (g_dwOpIndent);
		return FALSE;
	}

	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel);

	int nEndRowOffset = GetRowStartByOffset (iEndSel, TRUE);

	if (nEndRowOffset == iEndSel)
	{
		iEndSel--;
	}

	BCGP_EDIT_UNDO_INDENT_DATA* pData = new BCGP_EDIT_UNDO_INDENT_DATA;

	if (IndentText (iStartSel, iEndSel, bForward, m_bBlockSelectionMode, TRUE, 
						pData->m_arInsDelStrings, FALSE))
	{
		// indent text changes the selection - recalculate endsel
		iEndSel = m_iEndSel; 
		int nEndRowOffset = GetRowStartByOffset (iEndSel, TRUE);

		if (nEndRowOffset == iEndSel)
		{
			iEndSel--;
		}

		pData->m_nStartOffset = m_iStartSel;
		pData->m_nEndOffset = iEndSel;
		pData->m_bForward = bForward;
		
		SetLastUndoReason (bForward ? g_dwUATIndent : g_dwUATUnindent);

		AddUndoAction (pData, g_dwUATIndentData);
		return TRUE;
	}
	else
	{
		delete pData;
	}

	OnFailedOperation (g_dwOpIndent);
	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IndentText (int nStartOffset, int nEndOffset, BOOL bForward, 
								BOOL bBlockSelectionMode, BOOL bSelectText, 
								CStringArray& arInsDelText, BOOL bUndoMode)
{
	if (bBlockSelectionMode)
	{
		return FALSE;
	}
	else
	{
		CString strTab;
		BuildTabString (strTab);

		int nTabStringLen = strTab.GetLength ();

		int nFirstSelRow = GetRowStartByOffset	(nStartOffset, TRUE);
		int nLastSelRow  = max (nEndOffset, GetRowEndByOffset (nEndOffset, TRUE));

		int i = 0;
		for (int iIdx = nFirstSelRow; iIdx < nLastSelRow; iIdx++, i++)
		{
			if (bForward)
			{
				int nTextStart = GetRowTextStart (iIdx, FALSE);
				if (!bUndoMode)
				{
					OnBeforeTextInserted (strTab, nTextStart);
					m_strBuffer.Insert (nTextStart, strTab);
					UpdateOffsetRelatedData (nTextStart, nTextStart + strTab.GetLength ());
					OnUpdateAutoOutlining (nTextStart, strTab.GetLength (), FALSE);
					OnAfterTextChanged (nTextStart, strTab, TRUE);
					nLastSelRow += nTabStringLen;
					arInsDelText.SetAtGrow (i, strTab);

					if (m_nTopVisibleOffset > iIdx)
					{
						m_nTopVisibleOffset += nTabStringLen;
					}
				}
				else
				{
					if (i < arInsDelText.GetSize ())
					{
						CString& str = arInsDelText [i];

						OnBeforeTextInserted (str, nTextStart);
						m_strBuffer.Insert (nTextStart, str);
						UpdateOffsetRelatedData (nTextStart, nTextStart + str.GetLength ());
						OnUpdateAutoOutlining (nTextStart, str.GetLength (), FALSE);
						OnAfterTextChanged (nTextStart, str, TRUE);

						nLastSelRow += str.GetLength ();
						if (m_nTopVisibleOffset > iIdx)
						{
							m_nTopVisibleOffset += str.GetLength ();
						}
					}
				}
			}
			else
			{
				int nTextStart = GetRowTextStart (iIdx, FALSE);
				int nRowStart = GetRowStartByOffset (nTextStart, FALSE);

				int nCharsToDelete = m_nIndentSize;

				CString strInsDelText;
				
				if (m_bKeepTabs)
				{
					while (nCharsToDelete > 0 && 
						   nTextStart - nRowStart > 0)
					{
						TCHAR ch = m_strBuffer.GetAt (nTextStart - 1);

						if (ch == _T ('\t'))  
						{
							int nColumn = GetColumnFromOffset (nTextStart - 1, FALSE);
							nCharsToDelete -= m_nTabSize - nColumn % m_nTabSize;  
						}
						else
						{
							nCharsToDelete--;
						}

						if (nCharsToDelete < 0)
						{
							break;
						}
						OnBeforeTextDeleted (nTextStart - 1, CString (ch));
						RemoveOffsetDataInRange (nTextStart - 1, nTextStart);
						m_strBuffer.Delete (nTextStart - 1, 1);
						OnUpdateAutoOutlining (nTextStart - 1, 0, FALSE);
						OnAfterTextChanged (nTextStart - 1, CString (ch), FALSE);
						strInsDelText.Insert (0, CString (ch));

						nLastSelRow--;
						nTextStart = GetRowTextStart (iIdx, FALSE);
						if (m_nTopVisibleOffset > iIdx)
						{
							m_nTopVisibleOffset--;
						}
					}
				} 
				else
				{
					if (nTextStart - nRowStart < m_nIndentSize)
					{
						nCharsToDelete = nTextStart - nRowStart;
					}
					if (nCharsToDelete > 0)
					{
						strInsDelText = m_strBuffer.Mid (nTextStart - nCharsToDelete, nCharsToDelete);
						OnBeforeTextDeleted (nTextStart - nCharsToDelete, strInsDelText);
						RemoveOffsetDataInRange (nTextStart - nCharsToDelete, nTextStart);
						m_strBuffer.Delete (nTextStart - nCharsToDelete, nCharsToDelete);
						OnUpdateAutoOutlining (nTextStart - nCharsToDelete, 0, FALSE);
						OnAfterTextChanged (nTextStart - nCharsToDelete, strInsDelText, FALSE);
						nLastSelRow -= nCharsToDelete;
						if (m_nTopVisibleOffset > iIdx)
						{
							m_nTopVisibleOffset -= nCharsToDelete;
						}
					}
				}
				
				arInsDelText.SetAtGrow (i, strInsDelText);				
			}

			iIdx = m_strBuffer.Find (g_strEOL, iIdx);
			if (iIdx == -1)
			{
				break;
			}
		}

		int nNewCaretPos = -1; 
		if (bSelectText)
		{
			if (m_iEndSel < m_iStartSel)
			{
				nNewCaretPos = nFirstSelRow;
				m_iStartSel = nLastSelRow + 1;
				m_iEndSel = nFirstSelRow;
			}
			else
			{
				nNewCaretPos = nLastSelRow + 1;
				m_iStartSel = nFirstSelRow;
				m_iEndSel = nLastSelRow + 1;
			}
		}

		UpdateScrollBars ();

		if (nFirstSelRow < m_nTopVisibleOffset)
		{
			CPoint pt, ptRowColumn;
			OffsetToPoint (nFirstSelRow, pt, &ptRowColumn);
			OnScroll (SB_VERT, SB_THUMBPOSITION, ptRowColumn.y);
			if (bSelectText)
			{
				SetCaret (nNewCaretPos);
			}
			RedrawWindow ();
		}
		else
		{
			if (bSelectText)
			{
				SetCaret (nNewCaretPos);
			}
			RedrawRestOfText (nFirstSelRow);
		}
	}
	return TRUE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::SetCaretToLastMaxColumn (int nOffset, BOOL bRedrawOnScroll)
{
	CPoint point;
	CPoint ptRowColumn (-1, -1);
	OffsetToPoint (nOffset, point, &ptRowColumn);

	if (ptRowColumn.x < m_nLastMaxColumn)
	{
		int nRowEndOffset = 0;
		int nRowLen = GetNumOfColumnsInRowByOffset (nOffset, nRowEndOffset, TRUE);
		if (nRowLen >= m_nLastMaxColumn)
		{
			int nNewOffset = GetOffsetOfColumnInRow (m_nLastMaxColumn, nRowEndOffset, TRUE, TRUE);
			if(nNewOffset >= nOffset)
			{
				SetCaret (nNewOffset, TRUE, bRedrawOnScroll);
			}
			else
			{
				SetCaret (nOffset);
			}
		}
		else
		{
			SetCaret (nRowEndOffset, TRUE, bRedrawOnScroll);
		}
		return TRUE;
	}

	return FALSE;
}
//**************************************************************************************
void CBCGPEditCtrl::AddUndoAction (BCGP_EDIT_UNDO_ACTION* pUndoAction)
{
	pUndoAction->m_dwActionType |= GetLastUndoReason ();
	pUndoAction->m_nCursorOffset = m_iEndSel != -1 ? m_iEndSel : m_nCurrOffset;

	if (!OnBeforeAddUndoAction (pUndoAction))
	{
		delete pUndoAction;
		return;
	}	

	if (!m_bEOL)
	{
		POSITION pos = m_lstUndoActions.GetTailPosition ();
		while (pos != m_posUndoBuffer)
		{
			POSITION posSave = pos;	
			BCGP_EDIT_UNDO_ACTION* pUndoAction = 
				(BCGP_EDIT_UNDO_ACTION*) m_lstUndoActions.GetPrev (pos);
			m_lstUndoActions.RemoveAt (posSave);
			OnRemoveUndoAction (pUndoAction);
			delete pUndoAction;
		}
	}

	m_lstUndoActions.AddTail (pUndoAction);
	m_bEOL = TRUE;
	m_bBOL = FALSE;

	m_posUndoBuffer = NULL;
	if (m_lstUndoActions.GetCount () > m_nUndoBufferSize)
	{
		BCGP_EDIT_UNDO_ACTION* pUndoAction = 
			(BCGP_EDIT_UNDO_ACTION*) m_lstUndoActions.RemoveHead ();
		OnRemoveUndoAction (pUndoAction);
		delete pUndoAction;
	}

	m_bIsModified = TRUE;
}
//**************************************************************************************
void CBCGPEditCtrl::AddUndoAction (CString& strText, DWORD dwAction, int nCurrOffset, 
								   BOOL bForceInsertMode, BOOL bForceNextUndo)
{
	BCGP_EDIT_UNDO_ACTION* pUndoAction = new BCGP_EDIT_UNDO_ACTION;
	pUndoAction->m_dwActionType = dwAction;
	pUndoAction->m_strActionText = strText;
	pUndoAction->m_nActionOffset = nCurrOffset;
	pUndoAction->m_bForceNextUndo = bForceNextUndo;

	pUndoAction->m_dwFlags |= (GetOverrideMode () && !bForceInsertMode) ? g_dwUAFOvr : g_dwUAFIns;
	AddUndoAction (pUndoAction);
}
//**************************************************************************************
void CBCGPEditCtrl::AddUndoAction (TCHAR ch, DWORD dwAction, int nCurrOffset, 
								   BOOL bForseInsertMode)
{
	TCHAR chCurrent = _T ('\0');

	if (!m_strBuffer.IsEmpty () &&  nCurrOffset < m_strBuffer.GetLength ())
	{
		chCurrent = m_strBuffer.GetAt (nCurrOffset);
	}

	if (ch == g_chEOL)
	{
		CString str (ch);
		AddUndoAction (str, dwAction, nCurrOffset, TRUE);
		return;
	}

	CString str (GetOverrideMode () ? chCurrent : ch);

	if (!m_bUndoCharMode && dwAction == g_dwUATInsertData && !GetOverrideMode ())
	{
		if ((m_nPrevKey == _T (' ') || m_nPrevKey > VK_HELP) && 
			(m_strWordDelimeters.Find (ch) == -1 && 
			 m_strWordDelimeters.Find ((TCHAR) m_nPrevKey) == -1 ||
			 m_strWordDelimeters.Find (ch) != -1 && 
			 m_strWordDelimeters.Find ((TCHAR) m_nPrevKey) != -1) &&
			 !m_lstUndoActions.IsEmpty () &&
			 m_bEOL && m_posUndoBuffer == NULL)
		{
			BCGP_EDIT_UNDO_ACTION* pUndoAction = m_lstUndoActions.GetTail ();
			if (!(pUndoAction->m_dwFlags & g_dwUAFSaved) &&
				(pUndoAction->m_dwActionType & g_dwUATInsertData) &&
				nCurrOffset == pUndoAction->m_nCursorOffset + pUndoAction->m_strActionText.GetLength ())
			{
				pUndoAction->m_strActionText += str;
			}
			else
			{
				AddUndoAction (str, dwAction, nCurrOffset, FALSE);
			}
		}
		else
		{
			AddUndoAction (str, dwAction, nCurrOffset, FALSE);
		}
	}
	else
	{
		AddUndoAction (str, dwAction, nCurrOffset, bForseInsertMode);
	}
}
//**************************************************************************************
void CBCGPEditCtrl::AddUndoAction (BCGP_BASE_UNDO_DATA* lpActionData, DWORD dwAction, 
								   BOOL bDestroyData)
{
	BCGP_EDIT_UNDO_ACTION* pUndoAction = new BCGP_EDIT_UNDO_ACTION;
	pUndoAction->m_lpActionData = lpActionData;
	pUndoAction->m_dwActionType = dwAction;
	pUndoAction->m_bDestroyActionData = bDestroyData;
	
	AddUndoAction (pUndoAction);
}
//**************************************************************************************
BOOL CBCGPEditCtrl::OnUndo (BOOL bSetCaret, BOOL bRedraw)
{
	if (m_lstUndoActions.IsEmpty () || m_bBOL)
	{
		OnFailedOperation (g_dwOpUndo);
		return FALSE;
	}

	if (m_bIntelliSenseMode && m_pIntelliSenseWnd != NULL)
	{
		m_pIntelliSenseWnd->PostMessage (WM_CLOSE);
	}

	if (m_posUndoBuffer == NULL)
	{
		m_posUndoBuffer = m_lstUndoActions.GetTailPosition ();
		m_bEOL = FALSE;
	}
	else if (!m_bIsLastActionUndo)
	{
		m_lstUndoActions.GetPrev (m_posUndoBuffer);
		ASSERT (m_posUndoBuffer != NULL);
	}

	POSITION posCurrent = m_posUndoBuffer;
	BCGP_EDIT_UNDO_ACTION* pUndoAction = 
		(BCGP_EDIT_UNDO_ACTION*) m_lstUndoActions.GetPrev (m_posUndoBuffer);
	
	ASSERT (pUndoAction != NULL);

	if (m_posUndoBuffer == NULL)
	{
		m_bBOL = TRUE;
	}

	m_bIsLastActionUndo = TRUE;

	BOOL bResult = ProcessUndoRedoAction (pUndoAction, TRUE, bSetCaret);

	// Check if the current state is saved or not:
	BOOL bSaved = !m_bSavedUndo && m_posUndoBuffer == NULL && m_bBOL;
	if (m_bSavedUndo && m_posUndoBuffer != NULL)
	{
		BCGP_EDIT_UNDO_ACTION* pUndoAction = 
		(BCGP_EDIT_UNDO_ACTION*) m_lstUndoActions.GetAt (m_posUndoBuffer);
		if ((pUndoAction->m_dwFlags & g_dwUAFSaved))
		{
			bSaved = TRUE;
		}
	}

	if (bSaved)
	{
		m_bIsModified = FALSE;
	}

	if (bResult)
	{
		m_lstUndoActions.SetAt (posCurrent, pUndoAction);
		
		if (m_posUndoBuffer != NULL)
		{
			BCGP_EDIT_UNDO_ACTION* pUndoAction = 
			(BCGP_EDIT_UNDO_ACTION*) m_lstUndoActions.GetAt (m_posUndoBuffer);
			if (pUndoAction->m_bForceNextUndo)
			{
				bResult = OnUndo (TRUE, FALSE);
			}
		}
	}

	if (bRedraw)
	{
		RedrawWindow ();
	}

	return bResult;
}
//**************************************************************************************
void CBCGPEditCtrl::GetUndoActions(CDWordArray& dwaUAT) const
{
	dwaUAT.RemoveAll();

	POSITION nUndoBufferPos = m_posUndoBuffer;
	BOOL bBOL = m_bBOL;
	BOOL bEOL = m_bEOL;
	BOOL bIsLastActionUndo = m_bIsLastActionUndo;
	CList <BCGP_EDIT_UNDO_ACTION*, BCGP_EDIT_UNDO_ACTION*> lstUndoActions;
	BCGP_EDIT_UNDO_ACTION* pNewUndoAction = NULL;
	
	lstUndoActions.AddTail(
		(CList<BCGP_EDIT_UNDO_ACTION*, BCGP_EDIT_UNDO_ACTION*>*) &m_lstUndoActions);

	if (!lstUndoActions.IsEmpty ())
	{
		while (!bBOL)
		{
			if (nUndoBufferPos == NULL)
			{
				nUndoBufferPos = lstUndoActions.GetTailPosition ();
				bEOL = FALSE;
			}
			else if (!bIsLastActionUndo)
			{
				lstUndoActions.GetPrev (nUndoBufferPos);
				ASSERT (nUndoBufferPos != NULL);
			}

			BCGP_EDIT_UNDO_ACTION* pUndoAction = 
				(BCGP_EDIT_UNDO_ACTION*) lstUndoActions.GetPrev (nUndoBufferPos);
			
			if (nUndoBufferPos == NULL)
			{
				bBOL = TRUE;
			}

			bIsLastActionUndo = TRUE;

			if (pUndoAction != NULL &&
				!pUndoAction->m_bForceNextUndo)
			{
				dwaUAT.Add(pUndoAction->m_dwActionType);
			}
		}
	}

	if (pNewUndoAction != NULL)
	{
		delete pNewUndoAction;
	}
}
//**************************************************************************************
BOOL CBCGPEditCtrl::OnRedo (BOOL bSetCaret, BOOL bRedraw)
{
	if (m_lstUndoActions.IsEmpty () || m_bEOL)
	{
		OnFailedOperation (g_dwOpRedo);
		return FALSE;
	}

	if (m_bIntelliSenseMode && m_pIntelliSenseWnd != NULL)
	{
		m_pIntelliSenseWnd->PostMessage (WM_CLOSE);
	}

	if (m_posUndoBuffer == NULL)
	{
		m_posUndoBuffer = m_lstUndoActions.GetHeadPosition ();
		m_bBOL = FALSE;
	}
	else if (m_bIsLastActionUndo)
	{
		m_lstUndoActions.GetNext (m_posUndoBuffer);
		ASSERT (m_posUndoBuffer != NULL);
	}

	POSITION posCurr = m_posUndoBuffer;

	BCGP_EDIT_UNDO_ACTION* pUndoAction = 
		(BCGP_EDIT_UNDO_ACTION*) m_lstUndoActions.GetNext (m_posUndoBuffer);
	ASSERT (pUndoAction != NULL);
	
	if (m_posUndoBuffer == NULL)
	{
		m_bEOL = TRUE;
	}

	BOOL bForceNextUndo = pUndoAction->m_bForceNextUndo;

	m_bIsLastActionUndo = FALSE;

	BOOL bResult = ProcessUndoRedoAction (pUndoAction, FALSE, bSetCaret);

	// Check if the current state is saved or not:
	BOOL bSaved = (!m_bSavedUndo && m_posUndoBuffer == NULL && m_bBOL) ||
				  (m_bSavedUndo && (pUndoAction->m_dwFlags & g_dwUAFSaved));
	if (bSaved)
	{
		m_bIsModified = FALSE;
	}

	if (bResult)
	{
		m_lstUndoActions.SetAt (posCurr, pUndoAction);
		
		if (bForceNextUndo)
		{
			bResult = (pUndoAction->m_dwActionType & g_dwUATDragDrop) ? OnRedo (FALSE, FALSE) :
															  OnRedo (TRUE, FALSE);
		}
	}

	if (bRedraw)
	{
		RedrawWindow ();
	}

	return bResult;
}
//**************************************************************************************
void CBCGPEditCtrl::GetRedoActions(CDWordArray& dwaUAT) const
{
	dwaUAT.RemoveAll();

	if (m_lstUndoActions.IsEmpty ())
	{
		return;
	}

	POSITION nUndoBufferPos = m_posUndoBuffer;
	BOOL bIsLastActionUndo = m_bIsLastActionUndo;
	BOOL bEOL = m_bEOL;

	while (!bEOL)
	{
		if (nUndoBufferPos == NULL)
		{
			nUndoBufferPos = m_lstUndoActions.GetHeadPosition ();
		}
		else if (bIsLastActionUndo)
		{
			m_lstUndoActions.GetNext (nUndoBufferPos);
			ASSERT (nUndoBufferPos != NULL);
		}

		BCGP_EDIT_UNDO_ACTION* pRedoAction = 
			(BCGP_EDIT_UNDO_ACTION*) m_lstUndoActions.GetNext (nUndoBufferPos);
		ASSERT (pRedoAction != NULL);
		
		if (nUndoBufferPos == NULL)
		{
			bEOL = TRUE;
		}

		bIsLastActionUndo = FALSE;

		if (pRedoAction != NULL &&
			!pRedoAction->m_bForceNextUndo)
		{
			dwaUAT.Add(pRedoAction->m_dwActionType);
		}
	}	
}
//**************************************************************************************
BOOL CBCGPEditCtrl::ProcessUndoRedoAction (BCGP_EDIT_UNDO_ACTION* pUndoAction, BOOL bUndo, 
										   BOOL bSetCaret)
{
	ASSERT (pUndoAction != NULL);
	
	int nDataLen = pUndoAction->m_strActionText.GetLength ();

	if ((pUndoAction->m_dwActionType & g_dwUATInsertData) && bUndo ||
		(pUndoAction->m_dwActionType & g_dwUATDeleteData) && !bUndo)
	{
		if ((pUndoAction->m_dwFlags & g_dwUAFOvr) && bUndo)
		{
			CString strChar;
			strChar += pUndoAction->m_strActionText [0];
			OnBeforeTextInserted (strChar, pUndoAction->m_nActionOffset);
			TCHAR chTmp = m_strBuffer.GetAt (pUndoAction->m_nActionOffset);
			m_strBuffer.SetAt (pUndoAction->m_nActionOffset, 
									pUndoAction->m_strActionText [0]);
			pUndoAction->m_strActionText = CString (chTmp);
			OnUpdateAutoOutlining (pUndoAction->m_nActionOffset, strChar.GetLength (), FALSE);
			OnAfterTextChanged (pUndoAction->m_nActionOffset, strChar, TRUE);

			m_nCurrOffset = (pUndoAction->m_nCursorOffset > m_strBuffer.GetLength ()) ? 
				m_strBuffer.GetLength () : pUndoAction->m_nCursorOffset;
		}
		else 
		{
			OnBeforeTextDeleted (pUndoAction->m_nActionOffset, pUndoAction->m_strActionText);
			OnDeleteTextFromBuffer (pUndoAction->m_nActionOffset, 
								pUndoAction->m_nActionOffset + nDataLen, 
								pUndoAction->m_strActionText);
			RemoveOffsetDataInRange (pUndoAction->m_nActionOffset,
									pUndoAction->m_nActionOffset + nDataLen);
			m_strBuffer.Delete (pUndoAction->m_nActionOffset, nDataLen);
			OnUpdateAutoOutlining (pUndoAction->m_nActionOffset, 0, FALSE);
			OnAfterTextChanged (pUndoAction->m_nActionOffset, pUndoAction->m_strActionText, FALSE);

			m_nCurrOffset = pUndoAction->m_nActionOffset; 
		}

		if (bSetCaret)
		{
			SetCaret (m_nCurrOffset);
		}
	}
	else if ((pUndoAction->m_dwActionType & g_dwUATInsertData) && !bUndo ||
			 (pUndoAction->m_dwActionType & g_dwUATDeleteData) && bUndo)
	{
		if ((pUndoAction->m_dwFlags & g_dwUAFOvr) && !bUndo)
		{
			CString strChar;
			strChar += pUndoAction->m_strActionText [0];

			OnBeforeTextInserted (strChar, pUndoAction->m_nActionOffset);

			TCHAR chTmp = m_strBuffer.GetAt (pUndoAction->m_nActionOffset);
			m_strBuffer.SetAt (pUndoAction->m_nActionOffset, 
									pUndoAction->m_strActionText [0]);
			OnUpdateAutoOutlining (pUndoAction->m_nActionOffset, strChar.GetLength (), FALSE);
			OnAfterTextChanged (pUndoAction->m_nActionOffset, strChar, TRUE);
			pUndoAction->m_strActionText = CString (chTmp);
		}
		else
		{
			OnBeforeTextInserted (pUndoAction->m_strActionText, pUndoAction->m_nActionOffset);
			OnInsertTextToBuffer (pUndoAction->m_nActionOffset, pUndoAction->m_strActionText);
			m_strBuffer.Insert (pUndoAction->m_nActionOffset, pUndoAction->m_strActionText);
			UpdateOffsetRelatedData (pUndoAction->m_nActionOffset, 
									pUndoAction->m_nActionOffset + pUndoAction->m_strActionText.GetLength ());
			OnUpdateAutoOutlining (pUndoAction->m_nActionOffset, pUndoAction->m_strActionText.GetLength (), FALSE);
			OnAfterTextChanged (pUndoAction->m_nActionOffset, pUndoAction->m_strActionText, TRUE);
		}
		m_nCurrOffset = pUndoAction->m_nActionOffset + nDataLen;
		if (bSetCaret)
		{
			SetCaret (m_nCurrOffset);
		}
	}
	else if (pUndoAction->m_dwActionType & g_dwUATIndentData)
	{
		BCGP_EDIT_UNDO_INDENT_DATA* pData = 
			(BCGP_EDIT_UNDO_INDENT_DATA*) pUndoAction->m_lpActionData;
		if (pData != NULL)
		{
			BOOL bForward = bUndo && !pData->m_bForward ||
							!bUndo && pData->m_bForward;

			int nStart = min (pData->m_nStartOffset, pData->m_nEndOffset);
			int nEnd = max (pData->m_nStartOffset, pData->m_nEndOffset);

			IndentText (nStart, nEnd, bForward, FALSE, FALSE, pData->m_arInsDelStrings, TRUE);
			if (bSetCaret)						
			{
				SetCaret (pUndoAction->m_nCursorOffset);
			}
		}
		else
		{
			ASSERT (FALSE);
		}
	}
	

	m_iStartSel = m_iEndSel = -1;
	m_bBlockSelectionMode = FALSE;
	m_ptStartBlockSel = m_ptEndBlockSel = CPoint (-1, -1);
	
	UpdateScrollBars ();
	BOOL bNeedUpdate = OnUpdateLineNumbersMarginWidth ();

	m_nScrollOffsetVert = RowFromOffset (m_nTopVisibleOffset, TRUE, TRUE);
	m_nScrollHiddenVert = RowFromOffset (m_nTopVisibleOffset, TRUE, FALSE) - m_nScrollOffsetVert;
	SetScrollPos (SB_VERT, m_nScrollOffsetVert);

	if (bNeedUpdate)
	{
		// Should update all margin-related things	
		RecalcLayout ();
		SetCaret (m_nCurrOffset, TRUE, FALSE);
		RedrawWindow ();
	}

	m_bIsModified = TRUE;

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::CanUndo () const 
{
	return m_posUndoBuffer != NULL || m_bEOL && !m_lstUndoActions.IsEmpty ();
}
//**************************************************************************************
BOOL CBCGPEditCtrl::CanRedo () const 
{
	return m_posUndoBuffer != NULL || m_bBOL && !m_lstUndoActions.IsEmpty ();;
}
//**************************************************************************************
void CBCGPEditCtrl::SetWordColor (CString strWord, COLORREF clrFrgnd, COLORREF clrBkgnd, 
								  BOOL bCaseSensitive)
{
	BCGP_EDIT_SYNTAX_COLOR syntaxColor (clrFrgnd, clrBkgnd, bCaseSensitive);
	if (!bCaseSensitive)
	{
		strWord.MakeUpper ();
	}
	m_mapWordColors.SetAt (strWord, syntaxColor);
}
//**************************************************************************************
BOOL CBCGPEditCtrl::RemoveWordFromColorTable (CString strWord)
{
	return m_mapWordColors.RemoveKey (strWord);
}
//**************************************************************************************
void CBCGPEditCtrl::RemoveAllWordsFromColorTable ()
{
	m_mapWordColors.RemoveAll ();
}
//**************************************************************************************
BOOL CBCGPEditCtrl::IsOffsetAtColorBlock (int nOffset)
{
	int nRangeStart = max (0, nOffset - m_nColorBlockStrLenMax);
	int nRangeEnd = min (m_strBuffer.GetLength (), nOffset + m_nColorBlockStrLenMax);

	CString strRange = m_strBuffer.Mid (nRangeStart, nRangeEnd - nRangeStart);
	for (POSITION pos = m_lstColorBlocks.GetHeadPosition (); pos != NULL;)
	{
		BCGP_EDIT_COLOR_BLOCK colorBlock = m_lstColorBlocks.GetNext (pos);
		if (!colorBlock.m_strOpen.IsEmpty () &&
			 strRange.Find (colorBlock.m_strOpen) != -1 ||
			!colorBlock.m_strClose.IsEmpty () && 
			strRange.Find (colorBlock.m_strClose) != -1)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//**************************************************************************************
POSITION CBCGPEditCtrl::SetBlockColor (CString strBlockOpen, CString strBlockClose, 
									 BOOL bWholeText,
									COLORREF clrFrgnd, COLORREF clrBkgnd,
									BOOL bCaseSensitive, BOOL bWholeWord)
{
	if (strBlockOpen.GetLength () > m_nColorBlockStrLenMax ||
		strBlockClose.GetLength () > m_nColorBlockStrLenMax)
	{
		ASSERT (FALSE);
		TRACE0("Open or close block length must not exceed two characters length!\n");
		return NULL;
	}

	if (bWholeText && 
		(strBlockOpen.GetLength () == 0 || strBlockClose.GetLength () == 0))
	{
		ASSERT (FALSE);
		TRACE0("Open or close block with bWholeText = TRUE must not be empty!\n");
		return NULL;
	}

	if (bWholeText)
	{
		for (POSITION pos = m_lstColorBlocks.GetHeadPosition (); pos != NULL;)
		{
			BCGP_EDIT_COLOR_BLOCK colorBlock = m_lstColorBlocks.GetNext (pos);
			if (colorBlock.m_bWholeText)
			{
				ASSERT (FALSE);
				TRACE0("Can't open the second color block with bWholeText = TRUE!\n");
				return NULL;
			}
		}
	}

	if (!bCaseSensitive)
	{
		// prepare for case sensitive search
		strBlockOpen.MakeUpper ();
		strBlockClose.MakeUpper ();
	}

	BCGP_EDIT_COLOR_BLOCK colorBlock;
	colorBlock.m_strOpen = strBlockOpen;
	colorBlock.m_strClose = strBlockClose;
	colorBlock.SetForegroundColor(clrFrgnd);
	colorBlock.m_clrBackground = clrBkgnd;
	colorBlock.m_bWholeText = bWholeText;
	colorBlock.m_bCaseSensitive = bCaseSensitive;
	colorBlock.m_bWholeWord = bWholeWord;

	POSITION posSave = NULL;
	for (POSITION pos = m_lstColorBlocks.GetHeadPosition (); pos != NULL;)
	{
		posSave = pos;
		BCGP_EDIT_COLOR_BLOCK colorBlockNext = m_lstColorBlocks.GetNext (pos);

		if (colorBlockNext.m_strOpen.GetLength () <= colorBlock.m_strOpen.GetLength ())
		{
			return m_lstColorBlocks.InsertBefore (posSave, colorBlock);
		}
	}		

	return m_lstColorBlocks.AddTail (colorBlock);
	
}
//**************************************************************************************
void CBCGPEditCtrl::OnSysColorChange() 
{
	CWnd::OnSysColorChange();

	InitColors ();
	RedrawWindow ();
}
//**************************************************************************************
BOOL CBCGPEditCtrl::RemoveBlockColor (POSITION posBlock)
{
	m_lstColorBlocks.RemoveAt (posBlock);
	return TRUE;
}
//**************************************************************************************
void CBCGPEditCtrl::RemoveAllBlocks ()
{
	m_lstColorBlocks.RemoveAll();
}
//************************************************************************************
int CBCGPEditCtrl::ReverseFindOneOf (const CString& strBuffer, int nPos, const CString& strChars) const
{
	if (nPos < 0 || nPos >= strBuffer.GetLength ())
	{
		return -1;
	}

	for (int i = nPos; i >= 0; i--)
	{
		TCHAR chNext = strBuffer.GetAt (i);
		if (strChars.Find (chNext) != -1)
		{
			return i + 1;
		}
	}

	return -1;
}
//************************************************************************************
void CBCGPEditCtrl::ReverseFindOneOf(const CString& strCharSet, const int ciStartPos, int& nCurrOffset) const
{
	do
	{
		nCurrOffset--;
	}
	while (nCurrOffset >= ciStartPos &&
		   strCharSet.Find(m_strBuffer.GetAt(nCurrOffset)) == -1);
}
//************************************************************************************
int CBCGPEditCtrl::FindOneOf (const CString& strBuffer, int nPos, const CString& strChars) const
{
	int nBufLen = strBuffer.GetLength ();
	if (nPos < 0 || nPos >= nBufLen)
	{
		return -1;
	}

	for (int i = nPos; i < nBufLen; i++)
	{
		TCHAR chNext = strBuffer.GetAt (i);
		if (strChars.Find (chNext) != -1)
		{
			return i;
		}
	}

	return -1;
}
//************************************************************************************
int CBCGPEditCtrl::WhileOneOf (const CString& strBuffer, int nOffset, bool bForward, 
							   const CString& strChars)
{
	int nResOffset = nOffset;

	while (nResOffset >= 0 && nResOffset < strBuffer.GetLength ())
	{
		TCHAR ch = strBuffer.GetAt (nResOffset);
		if (strChars.Find (ch) == -1)
		{
			break;
		}

		bForward ? nResOffset++ : nResOffset--;
	}

	if (!bForward) 
	{
		nResOffset++;
	}
		
	return nResOffset;
}
//************************************************************************************
int CBCGPEditCtrl::RowFromOffset (int nOffset, BOOL bCalcAll, BOOL bSkipHidden) const
{
	int nRow = 0; 
	int iIdx = 0;

	if (nOffset >= m_nTopVisibleOffset && !bCalcAll)
	{
		nRow = bSkipHidden ? m_nScrollOffsetVert: m_nScrollOffsetVert + m_nScrollHiddenVert;
		iIdx = m_nTopVisibleOffset;
	}
	
	for (;  ; nRow++)
	{
		iIdx = m_strBuffer.Find (g_strEOL, iIdx);
		while (iIdx >= 0 && bSkipHidden && FindCollapsedBlock (iIdx) != NULL)
		{
			iIdx = m_strBuffer.Find (g_strEOL, iIdx + 1);
		}
		if (iIdx >= nOffset || iIdx < 0)
		{
			break;
		}
		iIdx ++;
	}

	return nRow;
}
//************************************************************************************
int CBCGPEditCtrl::GetVirtualRow (int nRow, BOOL bCalcAll) const
{
	if (nRow == 0)
	{
		return 0;
	}

	int nRowIdx = 0;
	int nVirtRowIdx = 0;
	int nIdx = 0;

	if (nRow >= m_nScrollOffsetVert + m_nScrollHiddenVert && !bCalcAll)
	{
		nRowIdx = m_nScrollOffsetVert + m_nScrollHiddenVert;
		nVirtRowIdx = m_nScrollOffsetVert;
		nIdx = m_nTopVisibleOffset;
	}

	for (;  nRowIdx < nRow; nIdx++)
	{
		nIdx = m_strBuffer.Find (_T('\n'), nIdx);
		if (nIdx == -1)
		{
			break;
		}

		if (FindCollapsedBlock (nIdx) == NULL)
		{
			nVirtRowIdx++;
		}
		nRowIdx++;
	}

	return nVirtRowIdx;
}
//************************************************************************************
int CBCGPEditCtrl::GetRowFromVirtual (int nRowVirt, BOOL bCalcAll) const
{
	if (nRowVirt == 0)
	{
		return 0;
	}

	int nRowIdx = 0;
	int nVirtRowIdx = 0;
	int nIdx = 0;

	if (nRowVirt >= m_nScrollOffsetVert && !bCalcAll)
	{
		nRowIdx = m_nScrollOffsetVert + m_nScrollHiddenVert;
		nVirtRowIdx = m_nScrollOffsetVert;
		nIdx = m_nTopVisibleOffset;
	}

	for (; nVirtRowIdx < nRowVirt; nIdx++)
	{
		nIdx = m_strBuffer.Find (_T('\n'), nIdx);
		if (nIdx == -1)
		{
			break;
		}

		if (FindCollapsedBlock (nIdx) == NULL)
		{
			nVirtRowIdx++;
		}
		nRowIdx++;
	}

	return nRowIdx;
}
//************************************************************************************
int CBCGPEditCtrl::GetOffsetOfColumnInRow (int nColumn, int nRowEndOffset, BOOL bEndOffsetSpecified, BOOL bSkipHidden)
{
	int nStartRowOffset = GetRowStartByOffset (nRowEndOffset, bSkipHidden);

	if (!bEndOffsetSpecified)
	{
		nRowEndOffset = GetRowEndByOffset (nRowEndOffset, bSkipHidden);
	}
	
	if (m_bKeepTabs)
	{
		int nCurrColumn = 0;
		int iOffset = 0;

		for (iOffset = nStartRowOffset; iOffset < nRowEndOffset; iOffset++)
		{
			CBCGPOutlineBaseNode* pHiddenText = NULL;
			pHiddenText = bSkipHidden ? FindCollapsedBlock (iOffset) : NULL;
			if (pHiddenText == NULL)
			{
				TCHAR chNext = m_strBuffer.GetAt (iOffset);
				if (chNext == _T ('\t'))
				{
					int nRestOfTab = m_nTabSize - nCurrColumn % m_nTabSize;
					nCurrColumn += nRestOfTab; 
				}
				else
				{
					nCurrColumn++;
				}
			}
			else
			{
				ASSERT_VALID (pHiddenText);
				nCurrColumn += pHiddenText->m_strReplace.GetLength ();
				iOffset += pHiddenText->m_nEnd - (iOffset); // skip the rest of the hidden text
			}

			// Stop rule:
			if (nCurrColumn > nColumn)
			{
				break;
			}
		}

		return iOffset;
	}

	else if (bSkipHidden)
	{
		int nCurrColumn = 0;
		int iOffset = 0;

		for (iOffset = nStartRowOffset; iOffset < nRowEndOffset;)
		{
			CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlockInRange (iOffset, nRowEndOffset, TRUE);
			if (pHiddenText != NULL)
			{
				ASSERT_VALID (pHiddenText);

				nCurrColumn += pHiddenText->m_nStart - iOffset;

				if (nRowEndOffset == pHiddenText->m_nStart)
				{
					break;
				}

				nCurrColumn += pHiddenText->m_strReplace.GetLength ();
				iOffset += max (pHiddenText->m_nEnd - iOffset + 1, 1); // skip the rest of the hidden text

				// Stop rule:
				if (nCurrColumn > nColumn)
				{
					break;
				}
			}
			else
			{
				iOffset = nRowEndOffset;
				break;
			}
		}

		return iOffset;
	}

	return nStartRowOffset + nColumn;
}
//************************************************************************************
BOOL CBCGPEditCtrl::IsPointInSelectionRect (CPoint pt) 
{
	if (m_iStartSel >= 0 && m_iEndSel >= 0)
	{
		CRect rectSel1, rectSel2, rectSel3;
		GetSelectionRect (rectSel1, rectSel2, rectSel3);
		
		if (rectSel1.PtInRect (pt) || rectSel2.PtInRect (pt) || 
			rectSel3.PtInRect (pt))
		{
			return TRUE;
		}
	}

	return FALSE;
}

//*************************************************************************************
// Drag & Drop processing
//*************************************************************************************
//***************************************************************************************
BOOL CBCGPEditCtrlDropTarget::Register (CBCGPEditCtrl* pOwner)
{
	m_pOwner = pOwner;
	return COleDropTarget::Register (pOwner);
}
//***************************************************************************************
DROPEFFECT CBCGPEditCtrlDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	ASSERT (m_pOwner != NULL);

	if (!pDataObject->IsDataAvailable (_TCF_TEXT) && 
		!pDataObject->IsDataAvailable (g_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner->OnDragEnter(pDataObject, dwKeyState, point);
}
//***************************************************************************************
void CBCGPEditCtrlDropTarget::OnDragLeave(CWnd* /*pWnd*/) 
{
	ASSERT (m_pOwner != NULL);
	m_pOwner->OnDragLeave ();
}
//***************************************************************************************
DROPEFFECT CBCGPEditCtrlDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	ASSERT (m_pOwner != NULL);

	if (!pDataObject->IsDataAvailable (_TCF_TEXT) && 
		!pDataObject->IsDataAvailable (g_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner->OnDragOver(pDataObject, dwKeyState, point);
}
//***************************************************************************************
DROPEFFECT CBCGPEditCtrlDropTarget::OnDropEx(CWnd* /*pWnd*/, 
							COleDataObject* pDataObject, 
							DROPEFFECT dropEffect, 
							DROPEFFECT /*dropList*/, CPoint point) 
{
	ASSERT (m_pOwner != NULL);

	if (!pDataObject->IsDataAvailable (_TCF_TEXT) && 
		!pDataObject->IsDataAvailable (g_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner->OnDrop(pDataObject, dropEffect, point) ?
			dropEffect : DROPEFFECT_NONE;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::OnDrop (COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	OnDragLeave ();

	BOOL bTextFromat = pDataObject->IsDataAvailable (_TCF_TEXT);
	BOOL bBlockFormat = pDataObject->IsDataAvailable (g_cFormat);
	if (!bTextFromat && !bBlockFormat)
	{
		return FALSE;
	}

	CRect rectSel (m_ptStartBlockSel, m_ptEndBlockSel);
	rectSel.NormalizeRect ();
	VirtToClient (rectSel);

	int	nOffset = HitTest (point, FALSE, TRUE);
	if (m_iStartSel >= 0 && 
		nOffset >= min (m_iStartSel, m_iEndSel) && 
		nOffset <= max (m_iStartSel, m_iEndSel) && 
		!bBlockFormat ||
		bBlockFormat && m_bBlockSelectionMode && rectSel.PtInRect (point) ||
		nOffset == -1)
	{
		OnFailedOperation (g_dwOpDragDrop);
		return FALSE;
	}

	SetLastUndoReason (g_dwUATDragDrop);
	PasteFromDataObject (pDataObject, nOffset, !m_bDragTextMode, FALSE, TRUE, 
						 dropEffect == DROPEFFECT_MOVE && nOffset >= 0);

	// the drag drop was initiated by another application
	if (!m_bDragTextMode)
	{
		m_nSavedOffset = m_iEndSel;

		// activate the target frame
		CWnd* pMainWnd = AfxGetMainWnd ();
		if (pMainWnd != NULL)
		{
			pMainWnd->SetForegroundWindow ();
		}
	}

	m_nDropOffset = nOffset >= 0 ? nOffset : 0;

	return TRUE;
}
//***************************************************************************************
DROPEFFECT CBCGPEditCtrl::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if (m_bReadOnly)
	{
		return DROPEFFECT_NONE;
	}

	::DestroyCaret ();
	CreateGrayCaret (2, m_nLineHeight);
	ShowCaret ();
	
	return OnDragOver (pDataObject, dwKeyState, point);
}
//***************************************************************************************
void CBCGPEditCtrl::OnDragLeave()	
{
	if (!m_bReadOnly)
	{
		::DestroyCaret ();
		CSize sizeCaret = GetCaretSize ();
		::CreateCaret (GetSafeHwnd (), NULL, sizeCaret.cx, sizeCaret.cy);
	}
}
//***************************************************************************************
DROPEFFECT CBCGPEditCtrl::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if (!pDataObject->IsDataAvailable (_TCF_TEXT) && 
		!pDataObject->IsDataAvailable (g_cFormat)
		|| m_bReadOnly)
	{
		return DROPEFFECT_NONE;
	}

	CRect rect = m_rectText;
	rect.left = 0;
	int nCurrOffset = HitTest (point, FALSE, TRUE);
	
	if (!rect.PtInRect (point))
	{
		return DROPEFFECT_NONE; 
	}

	BOOL bScrollRight = FALSE;

	if (m_rectText.bottom - point.y < m_nLineHeight)
	{
		point.y += m_nLineHeight;
	}
	else if (point.y - m_rectText.top < m_nLineHeight)
	{
		point.y -= m_nLineHeight;
	}
	else if (m_rectText.right - point.x < m_nMaxCharWidth) 
	{
		point.x += m_nMaxCharWidth;
		bScrollRight = TRUE;
	}
	else if (point.x - (m_rectText.left + 1) < m_nMaxCharWidth)
	{
		point.x -= m_nMaxCharWidth;
	}

	int nOffset = HitTest (point, TRUE, TRUE);
	int nColumn = GetColumnFromOffset (nOffset, TRUE);
	int nCurrColumn = GetColumnFromOffset (nCurrOffset, TRUE);

	if (bScrollRight && nColumn <= nCurrColumn)
	{
		return (dwKeyState & MK_CONTROL) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
	}
	
	if (nOffset >= 0)
	{
		SetCaret (nOffset);
	}

	return (dwKeyState & MK_CONTROL) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
}
//************************************************************************************
void CBCGPEditCtrl::FindWordStartFinish (int nCaretOffset, const CString& strBuffer,
										 int& nStartOffset, int& nEndOffset, 
										 BOOL /*bSkipSpaces*/) const
{
	if (nCaretOffset >= strBuffer.GetLength ())
	{
		nCaretOffset = strBuffer.GetLength () - 1;
	}

	if (nCaretOffset < 0)
	{
		nCaretOffset = 0;
	}

	int i = 0;
	for (i = nCaretOffset;
	     i >= 0 && m_strNonSelectableChars.Find(strBuffer.GetAt(i)) != -1;
		 i--);
	
	if (i < 0)
	{
		nStartOffset = 0;
		nEndOffset = 0;
		return;
	}

	nStartOffset = ReverseFindOneOf (strBuffer, i, m_strWordDelimeters);
	nEndOffset = FindOneOf (strBuffer, i, m_strWordDelimeters);
	
	nStartOffset = max (nStartOffset, 0);
	nEndOffset	= nEndOffset >= 0 ? nEndOffset : strBuffer.GetLength ();
}
//************************************************************************************
BOOL CBCGPEditCtrl::GetWordFromPoint (CPoint pt, CString& strWord)
{
	CPoint ptSave  = pt;
	int nOffset = HitTest (pt, TRUE);
	if (abs (ptSave.x - pt.x) > m_nMaxCharWidth)
	{
		return FALSE;
	}
	if (nOffset == -1 || nOffset >= m_strBuffer.GetLength ())
	{
		return FALSE;
	}

	return GetWordFromOffset (nOffset, strWord);
}
//************************************************************************************
BOOL CBCGPEditCtrl::GetWordFromOffset (int nOffset, CString& strWord)
{
	int nStartOffset = -1;
	int nEndOffset = -1;

	FindWordStartFinish (nOffset, m_strBuffer, nStartOffset, nEndOffset);

	if (nEndOffset <= nStartOffset || nStartOffset == -1)
	{
		return FALSE;
	}

	strWord = m_strBuffer.Mid (nStartOffset, nEndOffset - nStartOffset);
	return TRUE;
}
//************************************************************************************
BOOL CBCGPEditCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_SYSKEYUP && pMsg->wParam == VK_MENU && 
		m_bAltPressedForBlockSel)
	{
		m_bAltPressedForBlockSel = FALSE;
		return TRUE;
	}

	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
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
	}
	
	return CWnd::PreTranslateMessage(pMsg);
}
//****************************************************************************************
BOOL CBCGPEditCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext /* = NULL */) 
{
	return CWnd::CreateEx (	0, globalData.RegisterWindowClass (_T("BCGPEditCtrl")), 
							_T(""), dwStyle, rect, pParentWnd, nID, pContext);
}
//************************************************************************************
void CBCGPEditCtrl::PrepareBlock (CString& strOut)
{
	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel);

	if (!m_bBlockSelectionMode)
	{
		strOut = m_strBuffer.Mid (iStartSel, iEndSel - iStartSel);
	}
	else
	{
		CRect rect (m_ptStartBlockSel, m_ptEndBlockSel);
		rect.NormalizeRect ();
		CPoint ptRowTopLeft = rect.TopLeft ();
		CPoint ptBottomRight = rect.BottomRight ();

		ptRowTopLeft.x	-= m_nScrollOffsetHorz * m_nMaxCharWidth;
		ptBottomRight.x -= m_nScrollOffsetHorz * m_nMaxCharWidth;

		ptRowTopLeft.y	-= m_nScrollOffsetVert * m_nLineHeight;
		ptBottomRight.y -= m_nScrollOffsetVert * m_nLineHeight;


		if (m_ptEndBlockSel.y <= m_ptStartBlockSel.y)
		{
			ptRowTopLeft.y -= m_nLineHeight;
			ptBottomRight.y += m_nLineHeight;
		}

		
		ptBottomRight.x -= m_nAveCharWidth;
		

		CPoint ptRowEnd = ptBottomRight;
		ptRowEnd.y = ptRowTopLeft.y;

		do
		{
			int nStartColumn = HitTest (ptRowTopLeft);
			int nEndColumn = HitTest (ptRowEnd);

			ptRowTopLeft.y += m_nLineHeight;
			ptRowEnd.y += m_nLineHeight;

			CString strSubRow = m_strBuffer.Mid (nStartColumn, nEndColumn - nStartColumn + 1);
			strOut += strSubRow; 
			if (strSubRow.Find (g_chEOL) == -1)
			{
				strOut += g_strEOL;
			}
		}while (ptRowTopLeft.y < ptBottomRight.y);
	}
}
//************************************************************************************
void CBCGPEditCtrl::RetrieveSelectedBlock (CStringArray& arStrings)
{
	arStrings.RemoveAll ();

	if (m_iStartSel == -1 && m_iEndSel == -1)
	{
		return;
	}

	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel);


	if (!m_bBlockSelectionMode)
	{
		arStrings.SetAtGrow (0, m_strBuffer.Mid (iStartSel, iEndSel - iStartSel));
	}
	else
	{
		CRect rect (m_ptStartBlockSel, m_ptEndBlockSel);
		rect.NormalizeRect ();
		CPoint ptRowTopLeft = rect.TopLeft ();
		CPoint ptBottomRight = rect.BottomRight ();

		ptRowTopLeft.x	-= m_nScrollOffsetHorz * m_nMaxCharWidth;
		ptBottomRight.x -= m_nScrollOffsetHorz * m_nMaxCharWidth;

		ptRowTopLeft.y	-= m_nScrollOffsetVert * m_nLineHeight;
		ptBottomRight.y -= m_nScrollOffsetVert * m_nLineHeight;


		if (m_ptEndBlockSel.y <= m_ptStartBlockSel.y)
		{
			ptRowTopLeft.y -= m_nLineHeight;
			ptBottomRight.y += m_nLineHeight;
		}
		
		ptBottomRight.x -= m_nAveCharWidth;

		CPoint ptRowEnd = ptBottomRight;
		ptRowEnd.y = ptRowTopLeft.y;

		int nRow = 0;
		do
		{
			int nStartColumn = HitTest (ptRowTopLeft);
			int nEndColumn = HitTest (ptRowEnd);

			ptRowTopLeft.y += m_nLineHeight;
			ptRowEnd.y += m_nLineHeight;

			arStrings.SetAtGrow (nRow, m_strBuffer.Mid (nStartColumn, nEndColumn - nStartColumn + 1));
		}while (ptRowTopLeft.y < ptBottomRight.y);
	}
}
//************************************************************************************
void CBCGPEditCtrl::BuildTabString (CString& str, int nOffset)
{
	ASSERT(nOffset >= 0);

	if (m_bKeepTabs)
	{
		int nTabs = m_nIndentSize / m_nTabSize;
		int nSpaces = m_nIndentSize % m_nTabSize;

		CString strTabs (_T('\t'), nTabs);
		CString strSpaces (_T(' '), nSpaces);

		str += strTabs;
		str += strSpaces;
	}
	else
	{
		int nIndentSize = m_nIndentSize - (nOffset % m_nIndentSize);
		for (int i = 0; i < nIndentSize; i++)
		{
			str += _T(' ');
		}
	}
}
//************************************************************************************
void CBCGPEditCtrl::TrimEmptyLinesFromLeft (CString& str) const
{
	// Trim empty lines:
	int iEOL = -1;
	for (int i = 0; i < str.GetLength (); i++)
	{
		if (str [i] == g_chEOL)
		{
			iEOL = i;
		}
		
		if (m_strNonSelectableChars.Find (str [i]) == -1)
		{
			break;
		}
	}
	
	if (iEOL >= 0 && iEOL + 1 < str.GetLength ())
	{
		str = str.Mid (iEOL + 1);
	}
}

//----------------------
// Marker Support
//----------------------
//************************************************************************************
void CBCGPEditCtrl::OnDrawMarker (CDC* pDC, CRect rectMarker, const CBCGPEditMarker* pMarker)
{
	int nVertSpacing = m_nLineVertSpacing % 2; 
	rectMarker.DeflateRect (0, m_nLineVertSpacing / 2 + nVertSpacing, 0, m_nLineVertSpacing / 2 + nVertSpacing);

	if (pMarker->m_dwMarkerType & g_dwBCGPEdit_BookMark)
	{
	#ifdef _BCGPEDIT_STANDALONE
		BOOL bGradientMarkers = m_bGradientMarkers && pDC->GetDeviceCaps (BITSPIXEL) > 8;
	#else
		BOOL bGradientMarkers = m_bGradientMarkers && globalData.m_nBitsPerPixel > 8;
	#endif

		if (bGradientMarkers)
		{
			CBCGPDrawManager dm (*pDC);

			rectMarker.DeflateRect (1, 0, 1, 1);

			pDC->ExcludeClipRect (CRect (rectMarker.left, rectMarker.top, rectMarker.left + 1, rectMarker.top + 1));
			pDC->ExcludeClipRect (CRect (rectMarker.right - 1, rectMarker.top, rectMarker.right, rectMarker.top + 1));
			pDC->ExcludeClipRect (CRect (rectMarker.left, rectMarker.bottom - 1, rectMarker.left + 1, rectMarker.bottom));
			pDC->ExcludeClipRect (CRect (rectMarker.right - 1, rectMarker.bottom - 1, rectMarker.right, rectMarker.bottom));

			dm.FillGradient2 (rectMarker, RGB (133, 168, 236), RGB (53, 85, 144), 45);

			pDC->SelectClipRgn (NULL);

			rectMarker.DeflateRect (1, 1);
			dm.FillGradient2 (rectMarker, RGB (255, 255, 255), RGB (157, 187, 243), 45);
		}
		else
		{
			CBrush br (RGB (0, 255, 255));
			CBrush* pBrOld = pDC->SelectObject (&br);

			pDC->RoundRect (rectMarker, CPoint (5, 5));

			pDC->SelectObject (pBrOld);
		}
	}		
}
//************************************************************************************
BOOL CBCGPEditCtrl::ToggleMarker (int nRow, DWORD dwMarkerType, LPVOID lpData, BOOL bRedraw, 
								  UINT unPriority)
{
	ASSERT_VALID (this);
	CBCGPEditMarker* pMarker = NULL;

	if (GetMarker (nRow, &pMarker, dwMarkerType))
	{
		DeleteMarker (nRow, dwMarkerType, bRedraw);
		return FALSE;
	}

	pMarker = new CBCGPEditMarker ();

	pMarker->m_dwMarkerType = dwMarkerType;
	pMarker->m_nLine = nRow;
	pMarker->m_unPriority = unPriority;
	pMarker->m_pData = lpData;
	SetMarker (pMarker, nRow, bRedraw);	

	return TRUE;
	
}
//************************************************************************************
POSITION CBCGPEditCtrl::SetMarker (CBCGPEditMarker* pMarker, int nRow, BOOL bRedraw)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pMarker);

	if (nRow == -1)
	{
		pMarker->m_nLine = GetCurRow ();
	}

	return InsertMarker (pMarker, bRedraw);
}
//************************************************************************************
POSITION CBCGPEditCtrl::InsertMarker (CBCGPEditMarker* pMarker, BOOL bRedraw)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pMarker);

	if (pMarker->m_dwMarkerType & g_dwBCGPEdit_MarkerReserved)
	{
		ASSERT (FALSE);
		TRACE0 ("The type of this marker is reserved by the library. Use values greatr than 0x0000FFF0\n");
		return NULL;
	}

	POSITION posInserted = NULL;
	
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
															m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);

		if (pMarker->m_unPriority > pMarkerNext->m_unPriority ||
			pMarker->m_unPriority == pMarkerNext->m_unPriority &&
			(pMarker->m_nLine < pMarkerNext->m_nLine ||
			pMarker->m_nLine == pMarkerNext->m_nLine &&
			pMarker->m_dwMarkerType <= pMarkerNext->m_dwMarkerType))
		{
			posInserted = m_lstMarkers.InsertBefore (posSave, pMarker);
			break;
		}
	}

	if (posInserted == NULL)
	{
		posInserted = m_lstMarkers.AddTail (pMarker);
	}

	if (bRedraw)
	{
		BOOL bFullLine = pMarker->m_dwMarkerType & g_dwBCGPEdit_LineColorMarker;
		RedrawMarkerArea (pMarker->m_nLine, bFullLine);
	}

	return posInserted;
}
//************************************************************************************
POSITION CBCGPEditCtrl::GetMarker (int nRow, CBCGPEditMarker** ppMarker, 
									DWORD dwMarkerType)
{
	ASSERT_VALID (this);
	ASSERT (ppMarker != NULL);

	POSITION posSave = NULL;
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		posSave = pos;
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);
		
		if ((pMarkerNext->m_dwMarkerType & dwMarkerType) &&
			pMarkerNext->IsInRange (nRow, nRow))
		{
			*ppMarker = pMarkerNext;
			return posSave;
		}
	}
	return NULL;
}
//************************************************************************************
void CBCGPEditCtrl::GetMarkersInRange (int nRowStart, CObList& lstMarkers, 
									   DWORD dwMarkerType, int nRowEnd) const
{
	ASSERT_VALID (this);

	if (nRowEnd == -1)
	{
		nRowEnd = nRowStart;
	}

	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);
		if ((pMarkerNext->m_dwMarkerType & dwMarkerType) &&
			pMarkerNext->IsInRange (nRowStart, nRowEnd))
		{
			lstMarkers.AddTail (pMarkerNext);
		}
	}		
}
//************************************************************************************
BOOL CBCGPEditCtrl::SetMarkerData (int nRow, LPVOID lpData, DWORD dwMarkerType, 
								   BOOL bRedraw)
{
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);
		
		if (nRow == pMarkerNext->m_nLine && 
			(pMarkerNext->m_dwMarkerType & dwMarkerType))
		{
			pMarkerNext->m_pData = lpData;	
			if (bRedraw)
			{
				RedrawMarkerArea (nRow);
			}
			return TRUE;
		}
	}

	return FALSE;
}
//************************************************************************************
BOOL CBCGPEditCtrl::DeleteMarker (int nRow, DWORD dwMarkerType, BOOL bRedraw)
{
	BOOL bDeleted = FALSE;
	POSITION posSave = NULL;
	int nLine = -1;
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		posSave = pos;
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);

		nLine = pMarkerNext->m_nLine;
		
		if (nLine == nRow && (pMarkerNext->m_dwMarkerType & dwMarkerType))
		{
			DWORD dwMarkerType = pMarkerNext->m_dwMarkerType;

			if (OnRemoveMarker (pMarkerNext, posSave))
			{
				m_lstMarkers.RemoveAt (posSave);
				delete pMarkerNext;

				bDeleted = TRUE;

				if (bRedraw)
				{
					RedrawMarkerArea (nRow);

					if (dwMarkerType & g_dwBCGPEdit_LineColorMarker)
					{
						RedrawText (nRow, 0);
					}
				}
			}
		}
	}

	return bDeleted;
}
//************************************************************************************
void CBCGPEditCtrl::DeleteMarker (POSITION posMarker)
{
	ASSERT_VALID (this);
	ASSERT (posMarker != NULL);

	CBCGPEditMarker* pMarker = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
												 m_lstMarkers.GetAt (posMarker));
	if (pMarker != NULL)
	{
		OnRemoveMarker (pMarker, posMarker);
		m_lstMarkers.RemoveAt (posMarker);
		delete pMarker;
	}
}
//************************************************************************************
void CBCGPEditCtrl::DeleteAllMarkers (DWORD dwMarkerType, BOOL bRedraw)
{
	if (dwMarkerType == (DWORD) -1)
	{
		CleanUpMarkers ();
	}
	else
	{
		for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
		{
			POSITION posSave = pos;
			CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														m_lstMarkers.GetNext (pos));
			ASSERT_VALID (pMarkerNext);
			if (pMarkerNext->m_dwMarkerType & dwMarkerType)
			{
				DeleteMarker (posSave); 
			}
		}
	}

	if (bRedraw) 
	{
		RedrawMarkerArea (-1);
		RedrawWindow ();
	}
	return;
}
//************************************************************************************
CBCGPEditMarker* CBCGPEditCtrl::FindMarkerByData (DWORD dwData) const
{
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);
		if (pMarkerNext->CompareData (dwData))
		{
			return pMarkerNext;
		}
	}

	return NULL;
}
//************************************************************************************
void CBCGPEditCtrl::GetMarkerListByData (CObList& lstMarkers, DWORD dwData) const
{
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);
		if (pMarkerNext->CompareData (dwData))
		{
			lstMarkers.AddTail (pMarkerNext);
		}
	}
}
//************************************************************************************
void CBCGPEditCtrl::GetMarkerListByType (CObList& lstMarkers, DWORD dwMarkerType) const
{
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);
		if (pMarkerNext->m_dwMarkerType & dwMarkerType)
		{
			lstMarkers.AddTail (pMarkerNext);
		}
	}
}											
//************************************************************************************
BOOL CBCGPEditCtrl::GoToNextMarker (DWORD dwMarkerType, BOOL bDirection)
{
	if (m_lstMarkers.IsEmpty ())
	{
		return FALSE;
	}

	CObList lstMarkers;
	GetMarkerListByType (lstMarkers, dwMarkerType);

	if (lstMarkers.IsEmpty ())
	{
		return FALSE;
	}

	int nRow = GetCurRow ();

	for (POSITION pos = bDirection ? lstMarkers.GetHeadPosition () : 
									 lstMarkers.GetTailPosition () ; pos != NULL;)
	{
		CBCGPEditMarker* pMarkerNext = (CBCGPEditMarker*) 
											(bDirection ? lstMarkers.GetNext (pos) : 
											lstMarkers.GetPrev (pos));
		ASSERT_VALID (pMarkerNext);
		if (pMarkerNext->m_nLine > nRow && bDirection ||
			pMarkerNext->m_nLine < nRow && !bDirection)
		{
			return GoToMarker (pMarkerNext);
		}
	}

	CBCGPEditMarker* pMarkerNext = (CBCGPEditMarker*) 
						(bDirection ? lstMarkers.GetHead () : lstMarkers.GetTail ());

	return GoToMarker (pMarkerNext);
}
//************************************************************************************
BOOL CBCGPEditCtrl::HasMarkers (DWORD dwMarkerType)
{
	if (m_lstMarkers.IsEmpty ())
	{
		return FALSE;
	}

	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
													m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);
		if (pMarkerNext->m_dwMarkerType & dwMarkerType)
		{
			return TRUE;
		}
	}
	return FALSE;
}
//************************************************************************************
void CBCGPEditCtrl::RedrawMarkerArea (int nRow, BOOL bFullLine)
{
	CRect rectArea;
	GetClientRect (rectArea);

	rectArea.right = bFullLine ? m_rectText.right : rectArea.left + m_nLeftMarginWidth;
	
	if (nRow != -1)
	{
		int nRowVirt = GetVirtualRow (nRow);
		rectArea.top = (nRowVirt - m_nScrollOffsetVert) * m_nLineHeight;
		rectArea.bottom = rectArea.top + m_nLineHeight;
	}


	RedrawWindow (rectArea);		
}
//************************************************************************************
BOOL CBCGPEditCtrl::GoToMarker (const CBCGPEditMarker* pMarker)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pMarker);

	int nOffset = GetRowStart (pMarker->m_nLine, FALSE);
	if (nOffset != -1)
	{
		ExpandLine (pMarker->m_nLine);
		return SetCaret (nOffset);
	}
	return FALSE;
}
//************************************************************************************
void CBCGPEditCtrl::CleanUpMarkers ()
{
	while (!m_lstMarkers.IsEmpty ())
	{
		POSITION posMarker = m_lstMarkers.GetHeadPosition ();
		CBCGPEditMarker* pMarker =
			DYNAMIC_DOWNCAST (CBCGPEditMarker, m_lstMarkers.RemoveHead ());
		if (pMarker != NULL)
		{
			OnRemoveMarker (pMarker, posMarker);
			delete pMarker;
		}
	}
}
//************************************************************************************
void CBCGPEditCtrl::SetMarkerData (POSITION pos, DWORD_PTR dwData)
{
	ASSERT_VALID (this);
	CBCGPEditMarker* pMarker =
			DYNAMIC_DOWNCAST (CBCGPEditMarker, m_lstMarkers.GetAt (pos));
	if (pMarker != NULL)
	{
		pMarker->m_pData = (LPVOID) dwData;
	}
}
//************************************************************************************
POSITION CBCGPEditCtrl::SetLineColorMarker (int nRow, COLORREF clrFore, 
							 COLORREF clrBk, BOOL bFullLine, int nLineCount, 
							 DWORD dwMarkerType, UINT unPriority)
{
	if (nRow == -1)
	{
		nRow = GetCurRow ();
	}

	if (clrFore == (DWORD) -1)
	{
		clrFore = GetDefaultTextColor ();
	}

	if (clrBk == (DWORD) -1)
	{
		clrBk = GetDefaultBackColor ();
	}

	CBCGPLineColorMarker* pLineColorMarker = new CBCGPLineColorMarker;
	pLineColorMarker->m_nLine = nRow;
	pLineColorMarker->m_nLineCount = nLineCount;
	pLineColorMarker->m_clrForeground = clrFore;
	pLineColorMarker->m_clrBackground = clrBk;
	pLineColorMarker->m_bFullLine = bFullLine;
	pLineColorMarker->m_dwMarkerType = dwMarkerType | g_dwBCGPEdit_LineColorMarker;
	pLineColorMarker->m_unPriority = unPriority;

	return InsertMarker (pLineColorMarker);
}
//************************************************************************************
void CBCGPEditCtrl::UpdateLineRelatedData (int nStartLine, int nNumLines)
{
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														 m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);

		if (pMarkerNext->m_nLine >= nStartLine)
		{
			int nNewLine = pMarkerNext->m_nLine + nNumLines;
			if (CanUpdateMarker (pMarkerNext))
			{
				pMarkerNext->m_nLine = nNewLine;
			}
		}
	}
}
//************************************************************************************
void CBCGPEditCtrl::RemoveLineDataInRange (int nStartRow, int nEndRow)
{
	POSITION posSave = NULL;
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		posSave = pos;
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														 m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);

		CBCGPEditMarkerRange range = pMarkerNext->IsInRange (nStartRow, nEndRow);

		if (range == FULL_IN_RANGE &&
			CanRemoveMarker (pMarkerNext))
		{
			DeleteMarker (posSave);
		}
		else if (range == PARTIAL_IN_RANGE && 
				 CanUpdateMarker (pMarkerNext))
		{
			pMarkerNext->UpdateMarkerForDeletedRange (nStartRow, nEndRow);
		}
	}
}
//************************************************************************************
BOOL CBCGPEditCtrl::LoadXMLSettings (const CString& strFileName)
{
	ASSERT_VALID (this);

	CString strBuffer;
	CString strPath = strFileName;

	if (strFileName.Find (_T("\\")) == -1 &&
		strFileName.Find (_T("/")) == -1 &&
		strFileName.Find (_T(":")) == -1)
	{
		TCHAR lpszFilePath [_MAX_PATH];
		if (::GetModuleFileName (NULL, lpszFilePath, _MAX_PATH) > 0)
		{
			TCHAR path_buffer[_MAX_PATH];   
			TCHAR drive[_MAX_DRIVE];   
			TCHAR dir[_MAX_DIR];
			TCHAR fname[_MAX_FNAME];   
			TCHAR ext[_MAX_EXT];

#if _MSC_VER < 1400
			_tsplitpath (lpszFilePath, drive, dir, NULL, NULL);
			_tsplitpath (strFileName, NULL, NULL, fname, ext);
			_tmakepath (path_buffer, drive, dir, fname, ext);
#else
			_tsplitpath_s (lpszFilePath, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
			_tsplitpath_s (strFileName, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);
			_tmakepath_s (path_buffer, drive, dir, fname, ext);
#endif

			strPath = path_buffer;
		}
	}

	try
	{
		CStdioFile file;
		if (!file.Open (strPath, CFile::modeRead))
		{
			TRACE(_T("File not found: %s"), strFileName);
			return FALSE;
		}

		CString str;

		while (file.ReadString (str))
		{
			strBuffer += str;
		}
	}
	catch (CFileException* pEx)
	{
		pEx->ReportError ();
		pEx->Delete ();

		return FALSE;
	}

	return LoadXMLSettingsFromBuffer (strBuffer);
}
//***************************************************************************************
COLORREF CBCGPEditCtrl::GetColorFromBuffer (const CString strBuffer)
{
	if (strBuffer.GetLength () < 1)
	{
		ASSERT (FALSE);
		return 0;
	}

	CString strColor = strBuffer;

	if (strColor [0] == _T('\"'))
	{
		strColor.MakeLower ();
		strColor = strColor.Mid (1, strColor.GetLength () - 2);

		if (strColor == _T("black"))
			return RGB (0, 0, 0);
		else if (strColor == _T("darkred"))
			return RGB (128, 0, 0);
		else if (strColor == _T("darkgreen"))
			return RGB (0, 128, 0);
		else if (strColor == _T("brown"))
			return RGB (128, 128, 0);
		else if (strColor == _T("darkblue"))
			return RGB (0, 0, 128);
		else if (strColor == _T("darkmagenta"))
			return RGB (128, 0, 128);
		else if (strColor == _T("darkcyan"))
			return RGB (0, 128, 128);
		else if (strColor == _T("gray"))
			return RGB (192, 192, 192);
		else if (strColor == _T("darkgray"))
			return RGB (128, 128, 128);
		else if (strColor == _T("red"))
			return RGB (255, 0, 0);
		else if (strColor == _T("green"))
			return RGB (0, 255, 0);
		else if (strColor == _T("yellow"))
			return RGB (255, 255, 0);
		else if (strColor == _T("blue"))
			return RGB (0, 0, 255);
		else if (strColor == _T("magenta"))
			return RGB (255, 0, 255);
		else if (strColor == _T("cyan"))
			return RGB (0, 255, 255);
		else if (strColor == _T("white"))
			return RGB (255, 255, 255);

		return 0;
	}

	if (strColor.GetLength () != 6)
	{
		// Should be 'rrggbb'!
		ASSERT (FALSE);
		return 0;
	}

	// Swap Red and Blue values:
	CString str;
	str.Format (_T("0x%s%s%s"), strColor.Right (2), strColor.Mid (2, 2), strColor.Left (2));

	COLORREF color;

#if _MSC_VER < 1400
	_stscanf (str, _T("%x"), &color);
#else
	_stscanf_s (str, _T("%x"), &color);
#endif

	return color;
}
//****************************************************************************************
void CBCGPEditCtrl::RemoveXMLSettings ()
{
	m_mapWordColors.RemoveAll ();
	m_lstColorBlocks.RemoveAll ();
}
//****************************************************************************************
BOOL CBCGPEditCtrl::LoadXMLSettingsFromBuffer (const CString& strInBuffer)
{
	RemoveXMLSettings ();

	CString strBuffer = strInBuffer;

	//--------------
	// Read options:
	//--------------
	CString strOptions;
	if (globalData.ExcludeTag (strBuffer, _T("OPTIONS"), strOptions))
	{
		globalData.ExcludeTag (strOptions, _T("WordDelimeters"), m_strWordDelimeters, TRUE);
		globalData.ExcludeTag (strOptions, _T("SpecialDelimiters"), m_strSpecialDelimiters, TRUE);
		globalData.ExcludeTag (strOptions, _T("IntelliSenseChars"), m_strIntelliSenseChars, TRUE);

		//-----------------------
		// Read escape sequences:
		//-----------------------
		CString strEscapeSequencesList;
		if (globalData.ExcludeTag (strOptions, _T("EscapeSequences"), strEscapeSequencesList))
		{
			while (TRUE)
			{
				CString strEscapeSequence;
				if (!globalData.ExcludeTag (strEscapeSequencesList, _T("EscapeSequence"), strEscapeSequence, TRUE))
				{
					break;
				}

				AddEscapeSequence (strEscapeSequence);
			}
		}
	}

	//-----------------
	// Read color data:
	//-----------------
	CString strColorData;
	if (globalData.ExcludeTag (strBuffer, _T("COLOR_DATA"), strColorData))
	{
		//---------------------------------
		// Read text and background colors:
		//---------------------------------
		CString strColor;
		if (globalData.ExcludeTag (strColorData, _T("EditTextColor"), strColor))
		{
			m_clrText = GetColorFromBuffer (strColor);
		}

		if (globalData.ExcludeTag (strColorData, _T("EditBackColor"), strColor))
		{
			m_clrBack = GetColorFromBuffer (strColor);
		}

		if (globalData.ExcludeTag (strColorData, _T("SelTextColor"), strColor))
		{
			m_clrTextSelActive = GetColorFromBuffer (strColor);
			m_clrTextSelInActive = m_clrTextSelActive;
		}

		if (globalData.ExcludeTag (strColorData, _T("SelBackColor"), strColor))
		{
			m_clrBackSelActive = GetColorFromBuffer (strColor);
			m_clrBackSelInActive = m_clrBackSelActive;
		}

		//----------------
		// Read keywwords:
		//----------------
		CString strKeywords;
		while (globalData.ExcludeTag (strColorData, _T("KEYWORDS"), strKeywords))
		{
			BOOL bCaseSensitive = TRUE;

			CString strCaseSensitive;
			if (globalData.ExcludeTag (strKeywords, _T("CaseSensitive"), strCaseSensitive))
			{
				strCaseSensitive.MakeUpper ();
				bCaseSensitive = (strCaseSensitive == _T("TRUE"));
			}

			CString strColor;
			if (!globalData.ExcludeTag (strKeywords, _T("Color"), strColor))
			{
				ASSERT (FALSE);
			}
			else
			{
				COLORREF color = GetColorFromBuffer (strColor);
				COLORREF colorBack = (COLORREF) -1;

				CString strBackColor;
				if (globalData.ExcludeTag (strKeywords, _T("BackColor"), strBackColor))
				{
					colorBack = GetColorFromBuffer (strBackColor);
				}

				while (TRUE)
				{
					CString strKeyword;
					if (!globalData.ExcludeTag (strKeywords, _T("Keyword"), strKeyword))
					{
						break;
					}

					SetWordColor (strKeyword, color, colorBack, bCaseSensitive);
				}
			}
		}

		//------------
		// Read words:
		//------------
		CString strWords;
		if (globalData.ExcludeTag (strColorData, _T("WORDS"), strWords))
		{
			while (TRUE)
			{
				CString strWordData;
				if (!globalData.ExcludeTag (strWords, _T("WORD"), strWordData))
				{
					break;
				}

				CString strVal;
				if (!globalData.ExcludeTag (strWordData, _T("Val"), strVal))
				{
					ASSERT (FALSE);
					break;
				}

				CString strColor;
				if (!globalData.ExcludeTag (strWordData, _T("Color"), strColor))
				{
					ASSERT (FALSE);
					break;
				}

				COLORREF color = GetColorFromBuffer (strColor);
				COLORREF colorBack = (COLORREF) -1;

				CString strBackColor;
				if (globalData.ExcludeTag (strWordData, _T("BackColor"), strBackColor))
				{
					colorBack = GetColorFromBuffer (strBackColor);
				}

				BOOL bCaseSensitive = TRUE;
				CString strCaseSensitive;

				if (globalData.ExcludeTag (strWordData, _T("CaseSensitive"), strCaseSensitive))
				{
					strCaseSensitive.MakeUpper ();
					bCaseSensitive = (strCaseSensitive == _T("TRUE"));
				}

				SetWordColor (strVal, color, colorBack, bCaseSensitive);
			}
		}

		//-------------
		// Read blocks:
		//-------------
		CString strBlocks;
		if (globalData.ExcludeTag (strColorData, _T("BLOCKS"), strBlocks))
		{
			while (TRUE)
			{
				CString strBlockData;
				if (!globalData.ExcludeTag (strBlocks, _T("BLOCK"), strBlockData))
				{
					break;
				}

				CString strStart;
				if (!globalData.ExcludeTag (strBlockData, _T("Start"), strStart, TRUE))
				{
					ASSERT (FALSE);
					break;
				}

				CString strEnd;
				globalData.ExcludeTag (strBlockData, _T("End"), strEnd, TRUE);

				COLORREF color = (COLORREF)-1;

				CString strColor;
				if (globalData.ExcludeTag (strBlockData, _T("Color"), strColor))
				{
					color = GetColorFromBuffer (strColor);
				}

				COLORREF colorBack = (COLORREF) -1;

				CString strBackColor;
				if (globalData.ExcludeTag (strBlockData, _T("BackColor"), strBackColor))
				{
					colorBack = GetColorFromBuffer (strBackColor);
				}

				CString strIsWholeText;
				BOOL bIsWholeText = FALSE;

				if (globalData.ExcludeTag (strBlockData, _T("WholeText"), strIsWholeText))
				{
					strIsWholeText.MakeLower ();
					bIsWholeText = (strIsWholeText == _T("true"));
				}

				CString strIsCaseSensitive;
				BOOL bCaseSensitive = TRUE;

				if (globalData.ExcludeTag (strBlockData, _T("CaseSensitive"), strIsCaseSensitive))
				{
					strIsCaseSensitive.MakeLower ();
					bCaseSensitive = (strIsCaseSensitive == _T("true"));
				}

				CString strIsWholeWord;
				BOOL bIsWholeWord = FALSE;
				
				if (globalData.ExcludeTag (strBlockData, _T("WholeWord"), strIsWholeWord))
				{
					strIsWholeWord.MakeLower ();
					bIsWholeWord = (strIsWholeWord == _T("true"));
				}

				SetBlockColor (strStart, strEnd, bIsWholeText, color, colorBack, bCaseSensitive, bIsWholeWord);
			}
		}
	}

	//--------------------------
	// Read outline parser data:
	//--------------------------
	CString strOutlineData;
	if (globalData.ExcludeTag (strBuffer, _T("OUTLINE_DATA"), strOutlineData))
	{
		if (!LoadOutlineParserXMLSettings (strOutlineData))
		{
			ASSERT (FALSE);
			return FALSE;
		}
	}

	//-------------------------
	// Read hyperlink settings:
	//-------------------------
	CString strHyperlinkSettings;
	if (globalData.ExcludeTag (strBuffer, _T("HYPERLINKS"), strHyperlinkSettings))
	{
		if (!LoadHyperlinkXMLSettings (strHyperlinkSettings))
		{
			ASSERT (FALSE);
			return FALSE;
		}
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPEditCtrl::ShowContextTip()
{
	if (m_bEnableToolTips)
	{
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			ToolTipPop ();
		}
	}
}
//****************************************************************************************
BOOL CBCGPEditCtrl::OnTTNeedTipText (UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	if (!m_bEnableToolTips)
	{
		return FALSE;
	}

	CPoint point;
	
	::GetCursorPos (&point);
	ScreenToClient (&point);

	BOOL bIsHyperlink = m_bEnableHyperlinkSupport && GetHyperlinkToolTip (strTipText);
	BOOL bIsHiddenTextFromPoint = !bIsHyperlink && m_bEnableOutlining && GetHiddenTextFromPoint (point, strTipText);
	BOOL bIsWordFromPoint = !bIsHyperlink && !bIsHiddenTextFromPoint && GetWordFromPoint (point, strTipText);

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		if (bIsHiddenTextFromPoint || bIsHyperlink)
		{
			m_pToolTip->SetMaxTipWidth(SHRT_MAX);
		}
		else if (m_nMaxToolTipWidth != -1)
		{
			m_pToolTip->SetMaxTipWidth(m_nMaxToolTipWidth);
		}
	}

	if (!(bIsHyperlink ||bIsWordFromPoint || bIsHiddenTextFromPoint))
	{
		ToolTipPop ();
		return FALSE;
	}
	
	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	if (!OnGetTipText (strTipText))
	{
		return FALSE;
	}

	if (m_bToolTipCleared && strTipText == m_strLastDisplayedToolTip)
	{
		return FALSE;
	}

	pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
	m_bToolTipCleared = FALSE;
	m_strLastDisplayedToolTip = strTipText;
	
	return TRUE;
}
//****************************************************************************************
void CBCGPEditCtrl::EnableToolTips (BOOL bEnable)
{
	if (m_bEnableToolTips == bEnable)
	{
		return;
	}

	m_bEnableToolTips = bEnable;

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		if (bEnable)
		{
			m_pToolTip->Activate (TRUE);
		}
		else
		{
			ToolTipPop ();
			m_pToolTip->Activate (FALSE);
		}
	}
}
//****************************************************************************************
void CBCGPEditCtrl::ToolTipPop ()
{
	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		m_pToolTip->Pop ();
	}

	strTipText.Empty ();
	m_bToolTipCleared = TRUE;
}
//****************************************************************************************
BOOL CBCGPEditCtrl::OpenFile (const CString& strFileName)
{
	CFile file;
	if (!file.Open (strFileName, CFile::modeRead))
	{
		TRACE(_T("File not found: %s"), strFileName);
		return FALSE;
	}

	return OpenFile (file);
}
//***************************************************************************************
BOOL CBCGPEditCtrl::OpenFile (CFile& file)
{
	try
	{
		// Prepare buffer:
		const DWORD dwFileSize = (const DWORD) file.GetLength();
		char* pszFileBuffer = new char [dwFileSize + 2];
		memset (pszFileBuffer, 0, dwFileSize + 2);

		// Read from a file:
		#if _MSC_VER >= 1300
		file.Read ((void*)pszFileBuffer, dwFileSize);
		#else
		file.ReadHuge ((void*)pszFileBuffer, dwFileSize);
		#endif

#ifdef _UNICODE

		if (dwFileSize == 0)
		{
			SetWindowText (_T(""));
			delete [] pszFileBuffer;
			return TRUE;
		}
		
		if (!IsTextUnicode((void*)pszFileBuffer, dwFileSize, NULL))
		{
			// Convert buffer to Unicode:
			WCHAR* pszTextBuffer = new WCHAR [dwFileSize + 1];
			memset (pszTextBuffer, 0, (dwFileSize + 1) * sizeof (WCHAR));

			const int nTextLen = MultiByteToWideChar(::GetACP (), 0, 
				pszFileBuffer, dwFileSize, pszTextBuffer, dwFileSize);
			ASSERT ((DWORD)nTextLen <= dwFileSize);
			if (nTextLen <= 0)
			{
				TRACE0 ("SaveFile failed - system convertion API failed\n");
				delete [] pszTextBuffer;
				return FALSE;
			}

			pszTextBuffer[nTextLen] = _T('\0');
			SetWindowText (pszTextBuffer);

			delete [] pszTextBuffer;
		}
		else
		{
			TCHAR* pszText = (TCHAR*) pszFileBuffer;

			// Skip byte-order mark:
			int nFlags = IS_TEXT_UNICODE_SIGNATURE;
			IsTextUnicode((void*)pszFileBuffer, dwFileSize, &nFlags);
			if (nFlags != 0) // has the Unicode byte-order mark (BOM)
			{
				pszText = pszText + 1;
			}

			SetWindowText (pszText);
		}

#else
		SetWindowText ((TCHAR*)pszFileBuffer);
#endif

		// Free memory:
		delete [] pszFileBuffer;
	}
	catch (CFileException* pEx)
	{
		pEx->ReportError ();
		pEx->Delete ();

		return FALSE;
	}

	return TRUE;
}
//***************************************************************************************
BOOL CBCGPEditCtrl::SaveFile (const CString& strFileName)
{
	CFile file;
	if (!file.Open (strFileName, CFile::modeWrite | CFile::modeCreate))
	{
		TRACE(_T("Cannot open file: %s"), strFileName);
		return FALSE;
	}

	return SaveFile (file);
}
//****************************************************************************************
BOOL CBCGPEditCtrl::SaveFile (CFile& file)
{
	try
	{
		DWORD dwTextLen = m_strBuffer.GetLength();
		if (dwTextLen > 0)
		{
#ifdef _UNICODE

			if (m_bSaveFileInUnicode) // Unicode encoding
			{
				// Add byte order mark to indicate the Unicode encoding:
				TCHAR* pszTextBuffer = new TCHAR [dwTextLen + 2];
				memset (pszTextBuffer, 0, (dwTextLen+2) * sizeof(TCHAR));
				pszTextBuffer [0] = 0xFEFF;
				memcpy (pszTextBuffer + 1, (LPCTSTR)m_strBuffer, dwTextLen * sizeof(TCHAR));
				pszTextBuffer [dwTextLen+1] = _T ('\0');
				
				// Prepare buffer:
				HGLOBAL hGlobal = ExportBuffer (pszTextBuffer, dwTextLen+1, FALSE, TRUE);
				if (hGlobal == NULL)
				{
					TRACE0 ("SaveFile failed - out of memory\n");
					delete [] pszTextBuffer;
					return FALSE;
				}
				LPTSTR lpszBuffer = (LPTSTR) GlobalLock (hGlobal);
				if (lpszBuffer == NULL)
				{
					TRACE0 ("SaveFile failed - out of memory\n");
					GlobalFree (hGlobal);
					delete [] pszTextBuffer;
					return FALSE;
				}

				// Write to file:
				#if _MSC_VER >= 1300
				file.Write(lpszBuffer, (UINT)_tcslen (lpszBuffer) * sizeof (TCHAR));
				#else
				file.WriteHuge(lpszBuffer, _tcslen (lpszBuffer) * sizeof (TCHAR));
				#endif

				// Free memory:
				GlobalFree (hGlobal);
				delete [] pszTextBuffer;
			}

			else // ANSI encoding
			{
				// Prepare buffer:
				HGLOBAL hGlobal = ExportBuffer (m_strBuffer, dwTextLen, FALSE, TRUE);
				if (hGlobal == NULL)
				{
					TRACE0 ("SaveFile failed - out of memory\n");
					return FALSE;
				}
				LPTSTR lpszBuffer = (LPTSTR) GlobalLock (hGlobal);
				if (lpszBuffer == NULL)
				{
					TRACE0 ("SaveFile failed - out of memory\n");
					GlobalFree (hGlobal);
					return FALSE;
				}

				dwTextLen = (DWORD)_tcslen (lpszBuffer);

				// Convert buffer to Ansi:
				int nTextLenA = WideCharToMultiByte(::GetACP (), 0, lpszBuffer, dwTextLen, 
					NULL, 0, NULL, NULL);

				char* pszTextBuffer = new char [nTextLenA + 1];
				memset (pszTextBuffer, 0, (nTextLenA + 1) * sizeof(char));

				if (WideCharToMultiByte(::GetACP (), 0, lpszBuffer, dwTextLen, 
					pszTextBuffer, nTextLenA, NULL, NULL) == 0)
				{
					TRACE0 ("SaveFile failed - system convertion API failed\n");
					delete [] pszTextBuffer;
					GlobalFree (hGlobal);
					return FALSE;
				}
				pszTextBuffer [nTextLenA] = '\0';

				// Write to file:
				#if _MSC_VER >= 1300
				file.Write(pszTextBuffer, nTextLenA * sizeof (char));
				#else
				file.WriteHuge(pszTextBuffer, nTextLenA * sizeof (char));
				#endif

				// Free memory:
				delete [] pszTextBuffer;
				GlobalFree (hGlobal);
			}

#else	// #ifdef _UNICODE

			// Prepare buffer:
			HGLOBAL hGlobal = ExportBuffer (m_strBuffer, dwTextLen, FALSE, TRUE);
			if (hGlobal == NULL)
			{
				TRACE0 ("SaveFile failed - out of memory\n");
				return FALSE;
			}
			LPTSTR lpszBuffer = (LPTSTR) GlobalLock (hGlobal);
			if (lpszBuffer == NULL)
			{
				TRACE0 ("SaveFile failed - out of memory\n");
				GlobalFree (hGlobal);
				return FALSE;
			}

			// Write to file:
			#if _MSC_VER >= 1300
			file.Write(lpszBuffer, (int) _tcslen (lpszBuffer) * sizeof (TCHAR));
			#else
			file.WriteHuge(lpszBuffer, (int) _tcslen (lpszBuffer) * sizeof (TCHAR));
			#endif

			// Free memory:
			GlobalFree (hGlobal);

#endif	// #ifdef _UNICODE
			
		}
		else
		{
			file.SetLength(0);
		}
	}
	catch (CFileException* pEx)
	{
		pEx->ReportError ();
		pEx->Delete ();

		return FALSE;
	}
	
	m_bIsModified = FALSE;
	m_posDocSavedUndoPos = m_posUndoBuffer;
	m_bSavedUndo = !m_bBOL;

	// Update saved flag:
	for (POSITION pos = m_lstUndoActions.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;
		BCGP_EDIT_UNDO_ACTION* pUndoAction = m_lstUndoActions.GetNext (pos);
		ASSERT (pUndoAction != NULL);

		if (m_bEOL && posSave == m_lstUndoActions.GetTailPosition () ||
			!m_bEOL && posSave == m_posUndoBuffer)
		{
			// mark as saved
			pUndoAction->m_dwFlags |= g_dwUAFSaved;
		}
		else
		{
			// clear saved flag
			pUndoAction->m_dwFlags &= ~g_dwUAFSaved;
		}
	}

	return TRUE;
}
//***********************************************************************************
BOOL CBCGPEditCtrl::DoFindText(int& nPos, int& nFindLength, 
							   LPCTSTR lpszFind, BOOL bNext /* = TRUE */, BOOL bCase /* = TRUE */, BOOL bWholeWord /* = FALSE */)
{
	if (lpszFind == NULL || lpszFind [0] == 0)
	{
		return FALSE;
	}

	int nCurrOffset;

	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel);
	if (iStartSel < 0 || iEndSel <= iStartSel)
	{
		// no selection - use caret offset
		nCurrOffset = m_nCurrOffset;
	}
	else
	{
		if (bNext)
		{
			// selection - use the end of the selection as an offset
			nCurrOffset = iEndSel;
		}
		else
		{
			// selection - use the start of the selection as an offset
			nCurrOffset = iStartSel;
		}
	}

	if (nCurrOffset >= m_strBuffer.GetLength())
	{
		nCurrOffset = m_strBuffer.GetLength() - 1;
	}
	else if (nCurrOffset < 0)
	{
		nCurrOffset = 0;
	}

	CString strTmpBuffer;

	CString strFind(lpszFind);
	CString *pStrBuffer = NULL;
	
	if (!bCase)
	{
		strTmpBuffer = m_strBuffer;
		strTmpBuffer.MakeLower();
		strFind.MakeLower();

		pStrBuffer = &strTmpBuffer;
	}

	if (!bNext)
	{
		if (!pStrBuffer)
		{
			strTmpBuffer = m_strBuffer;
			pStrBuffer = &strTmpBuffer;
		}

		pStrBuffer->MakeReverse();
		strFind.MakeReverse();

		nCurrOffset = m_strBuffer.GetLength() - nCurrOffset;
	}

	const int nFindLen = strFind.GetLength();
	lpszFind = strFind.GetBuffer(0);

	if (pStrBuffer == NULL)
	{
		pStrBuffer = &m_strBuffer;
	}

	nPos = 0;
	int nLastOffset = nCurrOffset;
	
	// Find the string from nCurrOffset towards end of the buffer:
	do
	{
		nPos = pStrBuffer->Find(lpszFind, nCurrOffset);

		// for backword search:
		if (!bNext)
		{
			if (nPos == nLastOffset)
			{
				if (m_iStartSel < m_iEndSel &&
					nPos == m_strBuffer.GetLength() - m_iEndSel)
				{
					nPos = pStrBuffer->Find(lpszFind, ++nCurrOffset);

					if (nPos == -1)
					{
						nCurrOffset += (nFindLen - 1);
					}

					nLastOffset = nCurrOffset;
				}
			}
		}

		if (!bWholeWord || nPos == -1)
		{
			break;
		}
		
		// skip all substrings which are not embaced with delimiters
		int nPos1 = (nPos > 0)? m_strWordDelimeters.Find(pStrBuffer->GetAt(nPos - 1)): 0;
		int nPos2 = (nPos + nFindLen < m_strBuffer.GetLength() - 1)? m_strWordDelimeters.Find(pStrBuffer->GetAt(nPos + nFindLen)): 0;

		if (nPos1 == -1 ||
			nPos2 == -1)
		{
			nCurrOffset = nPos + nFindLen;
			nPos = -1;

			if (nCurrOffset == m_strBuffer.GetLength())
				break;
		}
	}
	while (nPos == -1);

	// Find the string from the begin of the buffer towards nCurrOffset:
	if (nPos == -1)
	{
		nLastOffset += nFindLen - 1;
		if (nLastOffset > pStrBuffer->GetLength())
		{
			nLastOffset = pStrBuffer->GetLength();
		}

		if (nLastOffset > 0)
		{
			LPTSTR pszBuffer = pStrBuffer->GetBuffer(nLastOffset + 1);
			TCHAR tCh = pszBuffer[nLastOffset];

			pszBuffer[nLastOffset] = 0;

			if (!bWholeWord)
			{
				nPos = pStrBuffer->Find(lpszFind);
			}
			else
			{
				nCurrOffset = 0;

				do
				{
					nPos = pStrBuffer->Find(lpszFind, nCurrOffset);

					// skip all substrings which are not embaced with delimiters
					if (nPos == -1)
					{
						break;
					}
					else
					{
						int nPos1 = (nPos > 0)? m_strWordDelimeters.Find(pszBuffer[nPos - 1]): 0;
						int nPos2 = (nPos + nFindLen < nLastOffset - 1)? m_strWordDelimeters.Find(pszBuffer[nPos + nFindLen]): 0;

						if (nPos1 == -1 ||
							nPos2 == -1)
						{
							nCurrOffset = nPos + nFindLen;
							nPos = -1;

							if (nCurrOffset == nLastOffset)
							{
								break;
							}
						}
					}
				}
				while (nPos == -1);
			}
			
			pszBuffer[nLastOffset] = tCh;
			pStrBuffer->ReleaseBuffer();
		}
	}

	if (nPos == -1)
	{
		return FALSE;
	}

	if (!bNext)
	{
		nPos = m_strBuffer.GetLength() - (nPos + nFindLen);
	}

	nFindLength = nFindLen;
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPEditCtrl::FindText(LPCTSTR lpszFind, BOOL bNext /* = TRUE */, BOOL bCase /* = TRUE */, BOOL bWholeWord /* = FALSE */)
{
	int nPos = -1;
	int nFindLen = 0;

	if (!DoFindText(nPos, nFindLen, lpszFind, bNext, bCase, bWholeWord))
	{
		return FALSE;
	}
	
	ASSERT (nPos >= 0);
	SetSel (nPos, nPos + nFindLen);

	return TRUE;
}
//***********************************************************************************
BOOL CBCGPEditCtrl::ReplaceText(LPCTSTR lpszFind, LPCTSTR lpszReplace, 
								BOOL bNext, BOOL bCase, BOOL bWholeWord)
{
	if (lpszReplace == NULL)
	{
		return FALSE;
	}

	if (m_bReadOnly)
	{
		return FALSE;
	}

	BOOL bModified = FALSE;

	// -----------------------------------
	// Compare the selection with lpszFind
	// -----------------------------------
	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel);

	if (iStartSel >= 0 && iEndSel > iStartSel)
	{
		CString strSelectedText = m_strBuffer.Mid (iStartSel, iEndSel - iStartSel);
		BOOL bIdentical = (bCase) ? 
			(strSelectedText.Compare (lpszFind) == 0) : 
			(strSelectedText.CompareNoCase (lpszFind) == 0);

		if (bWholeWord)
		{
			// skip all substrings which are not embraced with delimiters
			int nPos1 = (iStartSel > 0)? m_strWordDelimeters.Find(m_strBuffer.GetAt(iStartSel - 1)): 0;
			int nPos2 = (iEndSel < m_strBuffer.GetLength() - 1)? m_strWordDelimeters.Find(m_strBuffer.GetAt(iEndSel)): 0;
			if (nPos1 == -1 ||
				nPos2 == -1)
			{
				// skip
				bIdentical = FALSE;
			}
		}

		// --------------------------------------
		// If selected text is same as lpszFind,
		// replace selected text with lpszReplace
		// --------------------------------------
		if (bIdentical)
		{
			BOOL bForceNextUndo = TRUE;

			DeleteSelectedText (FALSE, FALSE, bForceNextUndo);
			SetLastUndoReason (g_dwUATReplace);
			int nSaveOffset = m_nCurrOffset;
			InsertText (lpszReplace, -1, FALSE, FALSE, TRUE, FALSE);

			SetSel2 (nSaveOffset, m_nCurrOffset, FALSE, FALSE);

			m_bIsModified = TRUE;
			bModified = TRUE;
			
			RedrawWindow ();
		}
	}

	// ----------------------------------------
	// Search for the next match of a substring
	// ----------------------------------------
	return FindText (lpszFind, bNext, bCase, bWholeWord) || bModified;
}
//***********************************************************************************
int CBCGPEditCtrl::ReplaceAll(LPCTSTR lpszFind, LPCTSTR lpszReplace, 
							  BOOL bNext, BOOL bCase, BOOL bWholeWord)
{
	if (lpszReplace == NULL)
	{
		return 0;
	}

	if (m_bReadOnly)
	{
		return 0;
	}

	CWaitCursor wait;
	SetRedraw (FALSE);

	int nFirstOffset = -1; // save offset to prevent loop
	int nPos = -1;
	int nFindLen = 0;
	int nReplaceLen = (int) _tcslen (lpszReplace);
	
	int nCount = 0;
	while (DoFindText(nPos, nFindLen, lpszFind, bNext, bCase, bWholeWord))
	{
		if (nPos < 0)
		{
			ASSERT (FALSE);
			break;
		}

		if (nFirstOffset == -1)
		{
			nFirstOffset = nPos; 
		}
		else if (nFirstOffset == nPos)
		{
			// passed to the end of the file
			break;
		}
		else if ((nFirstOffset < nPos + nFindLen) && 
				 (nFirstOffset + nReplaceLen > nPos))
		{
			// found entry is a part of the replace string
			break;
		}

		if (nFirstOffset > nPos)
		{
			nFirstOffset += nReplaceLen - nFindLen;
		}

		SetSel2 (nPos, nPos + nFindLen, FALSE, FALSE);

		// replace selected text with lpszReplace:
		BOOL bForceNextUndo = TRUE;

		DeleteSelectedText (FALSE, FALSE, bForceNextUndo);
		SetLastUndoReason (g_dwUATReplace);
		int nSaveOffset = m_nCurrOffset;
		InsertText (lpszReplace, -1, FALSE, FALSE, TRUE, FALSE);

		SetSel2 (nSaveOffset, m_nCurrOffset, FALSE, FALSE);

		m_bIsModified = TRUE;

		nCount++;
	}

	SetRedraw ();
	RedrawWindow ();

	return nCount;
}
//***********************************************************************************
void CBCGPEditCtrl::InitColors ()
{
	m_clrBackLineNumber = (COLORREF)-1;

#ifdef _BCGPEDIT_STANDALONE
	m_clrBackSelActive = ::GetSysColor (COLOR_HIGHLIGHT);
	m_clrTextSelActive = ::GetSysColor (COLOR_HIGHLIGHTTEXT);

	double H;
	double S;
	double L;

	CBCGPDrawManager::RGBtoHSL (m_clrBackSelActive, &H, &S, &L);
	m_clrBackSelInActive = CBCGPDrawManager::HLStoRGB_ONE (
		H,
		min (1., L * 1.1),
		min (1., S * 1.1));

	m_clrTextSelInActive	= CBCGPDrawManager::PixelAlpha (m_clrTextSelActive, 110);
	m_clrBackLineNumber = GetDefaultBackColor();
#endif

	m_clrTextLineNumber = RGB (0, 130, 132);

#ifdef _BCGPEDIT_STANDALONE
	m_clrBackSidebar = ::GetSysColor(COLOR_3DLIGHT);
#else
	m_clrBackSidebar = globalData.clrBtnLight;
#endif
}
//**************************************************************************************
void CBCGPEditCtrl::UpdateFonts()
{
	m_fontUnderline.DeleteObject ();

	//------------------
	// Initialize fonts:
	//------------------
	if (m_hFont != NULL)
	{
		LOGFONT lf;
		::GetObject(m_hFont, sizeof(LOGFONT), &lf);
		lf.lfUnderline = TRUE;

		m_fontUnderline.CreateFontIndirect (&lf);
	}
}
//**************************************************************************************
void CBCGPEditCtrl::CleanUpFonts()
{
	m_fontUnderline.DeleteObject ();
}
//**************************************************************************************
COLORREF CBCGPEditCtrl::GetDefaultTextColor () const
{
	if (m_clrText != -1)
	{
		return m_clrText;
	}

#ifndef _BCGPEDIT_STANDALONE
	return CBCGPVisualManager::GetInstance()->GetEditTextColor((CBCGPEditCtrl*)this);
#else
	return ::GetSysColor (COLOR_WINDOWTEXT);
#endif
}
//**************************************************************************************
COLORREF CBCGPEditCtrl::GetDefaultBackColor () const
{
	if (m_clrBack != -1)
	{
		return m_clrBack;
	}

#ifndef _BCGPEDIT_STANDALONE
	CBrush* pBrush = CBrush::FromHandle(CBCGPVisualManager::GetInstance()->GetEditBackgroundBrush((CBCGPEditCtrl*)this));
	ASSERT_VALID(pBrush);

	LOGBRUSH lbr;
	pBrush->GetLogBrush(&lbr);

	return lbr.lbColor;
#else
	return ::GetSysColor (COLOR_WINDOW);
#endif
}
//**************************************************************************************
void CBCGPEditCtrl::VirtToClient (CPoint& pt)
{
	pt.x -= m_nScrollOffsetHorz * m_nMaxCharWidth;
	pt.y -= m_nScrollOffsetVert * m_nLineHeight;
}
//**************************************************************************************
void CBCGPEditCtrl::ClientToVirt (CPoint& pt)
{
	pt.x += m_nScrollOffsetHorz * m_nMaxCharWidth;
	pt.y += m_nScrollOffsetVert * m_nLineHeight;
}
//**************************************************************************************
void CBCGPEditCtrl::VirtToClient (CRect& rect)
{
	CPoint ptTop = rect.TopLeft ();
	VirtToClient (ptTop);
	CPoint ptBottom = rect.BottomRight ();
	VirtToClient (ptBottom);
	rect.SetRect (ptTop, ptBottom);
}
//**************************************************************************************
void CBCGPEditCtrl::ClientToVirt (CRect& rect)
{
	CPoint ptTop = rect.TopLeft ();
	ClientToVirt (ptTop);
	CPoint ptBottom = rect.BottomRight ();
	ClientToVirt (ptBottom);
	rect.SetRect (ptTop, ptBottom);
}
//**************************************************************************************
void CBCGPEditCtrl::OnBeginPrinting(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_hPrinterFont == NULL)
	{
		// get current screen font object metrics
		CFont* pFont = GetFont();
		LOGFONT lf;
		LOGFONT lfSys;

		if (pFont == NULL)
		{
			pFont = CFont::FromHandle ((HFONT) ::GetStockObject (DEFAULT_GUI_FONT));
			if (pFont == NULL)
			{
				return;
			}
		}

		VERIFY(pFont->GetObject(sizeof(LOGFONT), &lf));
		VERIFY(::GetObject(::GetStockObject(SYSTEM_FONT), sizeof(LOGFONT),
			&lfSys));
		if (lstrcmpi((LPCTSTR)lf.lfFaceName, (LPCTSTR)lfSys.lfFaceName) == 0)
			return;

		// map to printer font metrics
		HDC hDCFrom = ::GetDC(NULL);
		lf.lfHeight = ::MulDiv(lf.lfHeight, pDC->GetDeviceCaps(LOGPIXELSY),
			::GetDeviceCaps(hDCFrom, LOGPIXELSY));
		lf.lfWidth = ::MulDiv(lf.lfWidth, pDC->GetDeviceCaps(LOGPIXELSX),
			::GetDeviceCaps(hDCFrom, LOGPIXELSX));
		::ReleaseDC(NULL, hDCFrom);

		// create it, if it fails we just use the printer's default.
		m_hMirrorFont = ::CreateFontIndirect(&lf);
		m_hPrinterFont = m_hMirrorFont;
	}

	ASSERT_VALID(this);
}
//***************************************************************************************
void CBCGPEditCtrl::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	ASSERT_VALID(this);

	if (m_hMirrorFont != NULL && m_hPrinterFont == m_hMirrorFont)
	{
		AfxDeleteObject((HGDIOBJ*)&m_hMirrorFont);
		m_hPrinterFont = NULL;
	}

	UpdateScrollBars ();
}
//***************************************************************************************
void CBCGPEditCtrl::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT(pInfo != NULL);
	ASSERT(pInfo->m_bContinuePrinting);

	CFont* pOldFont = SelectFont (pDC);
	pDC->SetBkMode(TRANSPARENT);

	UINT nPage = pInfo->m_nCurPage;

	OnChangeFont (pDC);

	OnUpdateLineNumbersMarginWidth (pDC);

	// print page header and footer
	pDC->SetTextColor(GetDefaultTextColor());
	pDC->SetBkColor(GetDefaultBackColor());

	OnPrintPageHeader(pDC, pInfo);
	OnPrintPageFooter(pDC, pInfo);

	// print content
	CRect rectText = m_rectText;
	m_rectText = pInfo->m_rectDraw;

	m_rectText.DeflateRect (2 * m_nLineHeight, 2 * m_nLineHeight);

	int nRowsInPage = m_nLineHeight == 0 ? 0 : m_rectText.Height () / m_nLineHeight;

	int nScrollOffsetVert = m_nScrollOffsetVert;
	m_nScrollOffsetVert = (nPage - 1) * nRowsInPage;

	int nTopVisibleOffset = m_nTopVisibleOffset;
	m_nTopVisibleOffset = GetRowStart (m_nScrollOffsetVert, TRUE);

	int nScrollHiddenVert = m_nScrollHiddenVert;
	m_nScrollHiddenVert = RowFromOffset (m_nTopVisibleOffset, TRUE, FALSE) - m_nScrollOffsetVert;

	int nScrollOffsetHorz = m_nScrollOffsetHorz;
	m_nScrollOffsetHorz = 0;

	m_nBottomVisibleOffset = GetRowStart (m_nScrollOffsetVert + nRowsInPage, TRUE);
	if (m_nBottomVisibleOffset < 0)
	{
		pInfo->m_bContinuePrinting = FALSE;
		pInfo->SetMaxPage (nPage);
	}
	else
	{
		--m_nBottomVisibleOffset;
	}

	OnDrawText (pDC);

	m_nScrollOffsetVert = nScrollOffsetVert;
	m_nScrollHiddenVert = nScrollHiddenVert;
	m_nScrollOffsetHorz = nScrollOffsetHorz;
	m_nTopVisibleOffset = nTopVisibleOffset;

	m_rectText = rectText;

	if (pOldFont != NULL)
	{
		pDC->SelectObject(pOldFont);
	}

	OnChangeFont ();

	OnUpdateLineNumbersMarginWidth ();
}
//**************************************************************************************
void CBCGPEditCtrl::SetPrinterFont(CFont* pFont)
{
	ASSERT_VALID(this);
	m_hPrinterFont = (HFONT)pFont->GetSafeHandle();
	ASSERT_VALID(this);
}
//**************************************************************************************
void CBCGPEditCtrl::Print ()
{
	// Printing without the Document/View framework
	ASSERT_VALID(this);

    CDC dc;
    CPrintDialog printDlg(FALSE);
	
    if (printDlg.DoModal() == IDCANCEL)
        return;
	
    dc.Attach(printDlg.GetPrinterDC());
    dc.m_bPrinting = TRUE;

    CString strTitle;
    strTitle.LoadString(AFX_IDS_APP_TITLE);

    DOCINFO di;
    ::ZeroMemory (&di, sizeof (DOCINFO));
    di.cbSize = sizeof (DOCINFO);
    di.lpszDocName = strTitle;

    BOOL bPrintingOK = dc.StartDoc(&di);

    CPrintInfo printInfo;
    printInfo.m_rectDraw.SetRect(0,0,
							dc.GetDeviceCaps(HORZRES), 
							dc.GetDeviceCaps(VERTRES));

    OnBeginPrinting(&dc, &printInfo);
    for (UINT page = printInfo.GetMinPage(); 
         page <= printInfo.GetMaxPage() && bPrintingOK; 
         page++)
    {
        dc.StartPage();
        printInfo.m_nCurPage = page;

        OnPrint(&dc, &printInfo);
        bPrintingOK = (dc.EndPage() > 0);
    }
    OnEndPrinting(&dc, &printInfo);

    if (bPrintingOK)
        dc.EndDoc();
    else
        dc.AbortDoc();

    dc.DeleteDC();
}
//*************************************************************************************
void CBCGPEditCtrl::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{
		SaveFile (*ar.GetFile());
	}
	else
	{
		OpenFile (*ar.GetFile());
	}
}
//**************************************************************************************
void CBCGPEditCtrl::OnAfterTextChanged (int /*nOffsetFrom*/, const CString& /*strText*/, BOOL /*bInserted*/)
{
	GetParent ()->SendMessage (BCGM_ON_EDITCHANGE);
}
//**************************************************************************************
UINT CBCGPEditCtrl::OnGetDlgCode() 
{
	if (m_nDlgCode == (UINT) -1)
	{
		return CWnd::OnGetDlgCode();
	}
	return m_nDlgCode;
}
//**************************************************************************************
void CBCGPEditCtrl::OnCalcOutlineSymbol (CDC* pDC, CPoint ptCharOffset, CBCGPOutlineBaseNode* pHiddenText)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pHiddenText);

	CSize size = GetStringExtent (pDC, pHiddenText->m_strReplace, pHiddenText->m_strReplace.GetLength ());
	pHiddenText->m_rectTool = CRect (ptCharOffset, size);
	pHiddenText->m_rectTool.InflateRect (1, 0);
}
//**************************************************************************************
void CBCGPEditCtrl::OnDrawOutlineSymbol (CDC* pDC, CBCGPOutlineBaseNode* pHiddenText,
										  COLORREF clrFore, COLORREF clrBack)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pHiddenText);

	CPen pen;
	pen.CreatePen (PS_SOLID, 1, clrFore);
	CBrush brush;
	brush.CreateSolidBrush (clrBack);

	CPen* pPenOld = pDC->SelectObject (&pen);
	CBrush* pBrushOld = pDC->SelectObject (&brush);

	CRect rectFrame = pHiddenText->m_rectTool;
	rectFrame.bottom ++;
	if (!pDC->IsPrinting ())
	{
		pDC->Rectangle (rectFrame);
	}

	pDC->SetTextColor (clrFore);
	pDC->SetBkColor   (clrBack);
	int nOldMode = pDC->SetBkMode (TRANSPARENT);

	CRect rectText = rectFrame;
	rectText.DeflateRect (1, 0);
	pDC->DrawText (pHiddenText->m_strReplace, rectText, DT_NOCLIP | DT_SINGLELINE | DT_NOPREFIX);

	pDC->SelectObject (pPenOld);
	pDC->SelectObject (pBrushOld);
	pDC->SetBkMode (nOldMode);
}
//**************************************************************************************
void CBCGPEditCtrl::RedrawOutlineArea (int nRow)
{
	CRect rectArea;
	GetClientRect (rectArea);

	rectArea.left = rectArea.left + m_nLeftMarginWidth;
	rectArea.right = m_rectText.right;
	
	if (nRow != -1)
	{
		int nRowVirt = GetVirtualRow (nRow);
		rectArea.top = (nRowVirt - m_nScrollOffsetVert) * m_nLineHeight;
		rectArea.bottom = rectArea.top + m_nLineHeight;
	}

	RedrawWindow (rectArea);		
}
//************************************************************************************

COutlineCasheManager::COutlineCasheManager ()
{
	m_bUpdateEvent = TRUE;
	m_nCacheRange = 1000;

	m_nCachedRangeStart = 0;
	m_nCachedRangeSize = 0;
}

COutlineCasheManager::~COutlineCasheManager ()
{
}

CBCGPOutlineNode* COutlineCasheManager::Lookup (int nOffset, BOOL& bIsActual)
{
	bIsActual = FALSE;

	if (m_bUpdateEvent)
	{
		return NULL;
	}

	int iBase = m_nCachedRangeStart;
	int iOffset = nOffset - iBase;

	ASSERT (m_nCachedRangeSize <= m_array.GetSize ());

	if (iOffset >= 0 && iOffset < m_nCachedRangeSize)
	{
		bIsActual = TRUE;
		return m_array [iOffset];
	}

	UpdateEvent ();

	return NULL;
}

void COutlineCasheManager::Rebuild (const CBCGPOutlineNode* pNodes, int nOffset)
{
	if (!pNodes)
	{
		ASSERT (FALSE);
		return;
	}

	int nStart = nOffset - m_nCacheRange;
	int nEnd = nOffset + m_nCacheRange;

	//------------------------------
	// allocate array on first start
	//------------------------------
	if (m_array.GetSize () < nEnd - nStart + 1)
	{
		m_array.SetSize (nEnd - nStart + 1);
		UpdateEvent ();
	}

	//-------------------------
	// constrain min max values
	//-------------------------
	if (nStart < 0)
	{
		nStart = 0;
	}
	////
	// the following can be deleted
	int nMax = pNodes->GetOwnerEditCtrl ()->GetText ().GetLength () - 1;
	if (nMax < 0) // text is empty
	{
		Clear ();
		m_bUpdateEvent = FALSE;
		return;
	}
	if (nEnd > nMax)
	{
		nEnd = nMax;
	}
	////

	//--------------------------------
	// save all outline nodes in range
	//--------------------------------
	CObList lstBlocks; // collect collapsed blocks
	pNodes->GetBlocksByStateInRange (nStart, nEnd, lstBlocks, TRUE, FALSE, TRUE);

	SetAt (nStart, nEnd, lstBlocks);

	m_bUpdateEvent = FALSE;
}

void COutlineCasheManager::SetAt (int nRangeStart, int nRangeEnd,
								  CObList& lstBlocks)
{
	if (nRangeEnd < nRangeStart || nRangeEnd < 0)
	{
		ASSERT (FALSE);
		Clear ();
		return;
	}

	//-----------------
	// set range bounds
	//-----------------
	m_nCachedRangeStart = max (0, nRangeStart);
	m_nCachedRangeSize = (int) m_array.GetSize ();
	m_nCachedRangeSize = min (m_nCachedRangeSize, nRangeEnd - m_nCachedRangeStart + 1);

	int nLeft = 0;
	int nCur = nLeft;

	//--------------------------------------
	// For each collapsed block 
	// save reference to the collapsed block
	// or save NULL between collapsed blocks
	//--------------------------------------
	for (POSITION pos = lstBlocks.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) lstBlocks.GetNext (pos);
		ASSERT_VALID (pOutlineNode);

		// before the block
		nCur = pOutlineNode->m_nStart - 1 - m_nCachedRangeStart;
		if (nLeft <= nCur)
		{
			// save null reference for the range (nLeft, nCur)
			DoSetAt (nLeft, nCur, NULL);
		}

		// collapsed block
		nLeft = max (nLeft, pOutlineNode->m_nStart - m_nCachedRangeStart);
		nCur = pOutlineNode->m_nEnd - m_nCachedRangeStart;
		if (nLeft <= nCur)
		{
			// save reference to this node for the range (nLeft, nCur)
			DoSetAt (nLeft, nCur, pOutlineNode);
		}

		nLeft = max (nLeft, pOutlineNode->m_nEnd + 1 - m_nCachedRangeStart);
		nCur = nLeft;
	}

	// after the last block
	nCur = m_nCachedRangeSize;
	if (nLeft <= nCur)
	{
		// save null reference for the range (nLeft, nCur)
		DoSetAt (nLeft, nCur, NULL);
	}
}

void COutlineCasheManager::DoSetAt (int nLeft, int nCur,
								  CBCGPOutlineNode* pOutlineNode)
{
	// for each offset in array inside the range (nLeft, nCur)
	for (int iOffset = max (0, nLeft); iOffset <= nCur && iOffset < m_array.GetSize (); iOffset++)
	{
		m_array [iOffset] = pOutlineNode;
	}
}

void COutlineCasheManager::Clear ()
{
	for (int iOffset = 0; iOffset < m_array.GetSize (); iOffset++)
	{
		m_array [iOffset] = NULL;
	}

}


//////////////////////////
// CBCGPOutlineNode class:

CBCGPOutlineNode* CBCGPOutlineNode::FindCollapsedBlock (int nOffset) const
{
	if (nOffset < 0)
	{
		return NULL;
	}

	if (m_lstNodes.GetCount () == 0)
	{
		return 0;
	}

	ASSERT_VALID (m_pEditCtrl);
	COutlineCasheManager& outlineManager = m_pEditCtrl->GetOutlineManager ();

	BOOL bIsActual = FALSE;
	CBCGPOutlineNode* pNode = outlineManager.Lookup (nOffset, bIsActual);

	if (!pNode && !bIsActual)
	{
		//---------------
		// Search itself:
		//---------------
		outlineManager.Rebuild (this, nOffset);
		pNode = outlineManager.Lookup (nOffset, bIsActual);
	}

	return pNode;
}

CBCGPOutlineNode* CBCGPOutlineNode::FindCollapsedBlockInRange (int nStart, int nEnd, 
															 BOOL bForward) const
{
	ASSERT (nStart >= 0);
	if (nEnd < nStart)
	{
		return NULL;
	}

	if (bForward)
	{
		// ------------------------------
		// Find the leftmost hidden area:
		// ------------------------------
		for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
		{
			CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
			ASSERT_VALID (pOutlineNode);

			if (pOutlineNode->IsInRangeByOffset (nStart, nEnd) != NOT_IN_RANGE)
			{
				if (pOutlineNode->m_bCollapsed)
				{
					return pOutlineNode;
				}
				else
				{
					// Recursive call:
					CBCGPOutlineNode* pInnerOutlineNode = 
						pOutlineNode->FindCollapsedBlockInRange (nStart, nEnd, bForward);
					
					if (pInnerOutlineNode != NULL)
					{
						ASSERT_VALID (pInnerOutlineNode);
						return pInnerOutlineNode;
					}
				}
			}

		}
	}
	else
	{
		// -------------------------------
		// Find the rightmost hidden area:
		// -------------------------------
		for (POSITION pos = m_lstNodes.GetTailPosition (); pos != NULL;)
		{
			CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) m_lstNodes.GetPrev (pos);
			ASSERT_VALID (pOutlineNode);

			if (pOutlineNode->IsInRangeByOffset (nStart, nEnd) != NOT_IN_RANGE)
			{
				if (pOutlineNode->m_bCollapsed)
				{
					return pOutlineNode;
				}
				else
				{
					// Recursive call:
					CBCGPOutlineNode* pInnerOutlineNode = 
						pOutlineNode->FindCollapsedBlockInRange (nStart, nEnd, bForward);
					
					if (pInnerOutlineNode != NULL)
					{
						ASSERT_VALID (pInnerOutlineNode);
						return pInnerOutlineNode;
					}
				}
			}

		}
	}

	return NULL;
}

CBCGPOutlineNode* CBCGPOutlineNode::FindCollapsedBlockByPoint (
										CPoint point, int nStart, int nEnd) const
{
	ASSERT (nStart >= 0);
	if (nEnd < nStart)
	{
		return NULL;
	}

	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pOutlineNode);

		if (pOutlineNode->m_nStart > nEnd)
		{
			return NULL;
		}

		if (pOutlineNode->m_nEnd >= nStart)
		{
			if (pOutlineNode->m_bCollapsed &&
				pOutlineNode->m_rectTool.PtInRect (point))
			{
				return pOutlineNode;
			}
			else
			{
				// Recursive call:
				CBCGPOutlineNode* pInnerOutlineNode = 
					pOutlineNode->FindCollapsedBlockByPoint (point, nStart, nEnd);
				
				if (pInnerOutlineNode != NULL)
				{
					ASSERT_VALID (pInnerOutlineNode);
					return pInnerOutlineNode;
				}
			}
		}
	}

	return NULL;
}

CBCGPOutlineNode* CBCGPOutlineNode::GetInnermostBlock (int nOffset) const
{
	if (nOffset < 0)
	{
		return NULL;
	}

	for (POSITION pos = m_lstNodes.GetTailPosition (); pos != NULL;)
	{
		CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) m_lstNodes.GetPrev (pos);
		ASSERT_VALID (pOutlineNode);

		// Recursive call:
		if (pOutlineNode->m_nEnd >= nOffset)
		{
			CBCGPOutlineNode* pInnerOutlineNode = pOutlineNode->GetInnermostBlock (nOffset);
			if (pInnerOutlineNode != NULL)
			{
				ASSERT_VALID (pInnerOutlineNode);
				return pInnerOutlineNode;
			}
		}

		if (pOutlineNode->m_nStart <= nOffset &&
			pOutlineNode->m_nEnd >= nOffset - 1 &&
			(pOutlineNode->m_dwFlags & g_dwOBFComplete) != 0)
		{
			return pOutlineNode;
		}
	}
	
	return NULL;
}

void CBCGPOutlineNode::GetAllBlocks (CObList& lstBlocks, BOOL bRecursive) const
{
	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pOutlineNode);

		lstBlocks.AddTail (pOutlineNode);

		if (bRecursive)
		{
			CObList lstSubBlocks;
			pOutlineNode->GetAllBlocks (lstSubBlocks, bRecursive);
			lstBlocks.AddTail (&lstSubBlocks);
		}
	}
}

void CBCGPOutlineNode::GetBlocksByState (CObList& lstBlocks, BOOL bCollapsed, BOOL bRecursive) const
{
	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pOutlineNode);

		if (pOutlineNode->m_bCollapsed == bCollapsed)
		{
			lstBlocks.AddTail (pOutlineNode);
		}
		else if (bRecursive && !pOutlineNode->m_bCollapsed)
		{
			CObList lstSubBlocks;
			pOutlineNode->GetBlocksByState (lstSubBlocks, bCollapsed, bRecursive);
			lstBlocks.AddTail (&lstSubBlocks);
		}
	}
}

void CBCGPOutlineNode::GetBlocksByStateInRange (int nStart, int nEnd, CObList& lstBlocks,
												BOOL bCollapsed, BOOL bFullInRangeOnly, 
												BOOL bRecursive) const
{
	ASSERT_VALID (this);

	if (nStart > nEnd)
	{
		return;
	}

	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pOutlineNode);

		CBCGPEditOutlineRange rangeResult = pOutlineNode->IsInRangeByOffset (nStart, nEnd);
		if (rangeResult == FULL_IN_RANGE || 
			!bFullInRangeOnly && rangeResult == PARTIAL_IN_RANGE)
		{
			if (pOutlineNode->m_bCollapsed == bCollapsed)
			{
				lstBlocks.AddTail (pOutlineNode);
			}
			else if (bRecursive && !pOutlineNode->m_bCollapsed)
			{
				CObList lstSubBlocks;
				pOutlineNode->GetBlocksByStateInRange (nStart, nEnd, lstSubBlocks, bCollapsed,
														bFullInRangeOnly, bRecursive);
				lstBlocks.AddTail (&lstSubBlocks);
			}
		}
	}		
}

void CBCGPOutlineNode::GetBlocksInRange (int nStart, int nEnd, CObList& lstOutlineBlocks, 
										 BOOL bFullInRangeOnly, BOOL bRecursive) const
{
	ASSERT_VALID (this);

	if (nStart > nEnd)
	{
		return;
	}

	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pOutlineNode);

		CBCGPEditOutlineRange rangeResult = pOutlineNode->IsInRangeByOffset (nStart, nEnd);
		if (rangeResult == FULL_IN_RANGE || 
			!bFullInRangeOnly && rangeResult == PARTIAL_IN_RANGE)
		{
			lstOutlineBlocks.AddTail (pOutlineNode);

			if (bRecursive)
			{
				CObList lstSubBlocks;
				pOutlineNode->GetBlocksInRange (nStart, nEnd, lstSubBlocks, bFullInRangeOnly, bRecursive);
				lstOutlineBlocks.AddTail (&lstSubBlocks);
			}
		}
	}		
}

BOOL CBCGPOutlineNode::DeleteInnermostBlock (int nOffset, BCGP_EDIT_OUTLINE_CHANGES& changes)
{
	if (nOffset < 0)
	{
		return FALSE;
	}

	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
	{
		POSITION posToDel = pos;

		CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pOutlineNode);

		if (pOutlineNode->m_nEnd >= nOffset)
		{
			// Recursive call:
			if (pOutlineNode->DeleteInnermostBlock (nOffset, changes))
			{
				return TRUE;
			}
		}

		if (pOutlineNode->m_nStart <= nOffset &&
			pOutlineNode->m_nEnd >= nOffset)
		{
			// Delete item:
			CBCGPOutlineNode* pNodeToDel = RemoveNode (posToDel, changes);
			if (pNodeToDel != NULL)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

int CBCGPOutlineNode::DeleteBlocksInRange (int nStart, int nEnd, BCGP_EDIT_OUTLINE_CHANGES& changes)
{
	ASSERT_VALID (this);

	if (nStart > nEnd)
	{
		return 0;
	}

	int nCounter = 0;
	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
	{
		POSITION posToDel = pos;

		CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pNode);
		
		CBCGPEditOutlineRange rangeResult = pNode->IsInRangeByOffset (nStart, nEnd);
		if (rangeResult == FULL_IN_RANGE)
		{
			// Delete item:
			CBCGPOutlineNode* pNodeToDel = RemoveNode (posToDel, changes, TRUE);
			if (pNodeToDel != NULL)
			{
				ASSERT_VALID (pNodeToDel);
				nCounter += pNodeToDel->GetCount (TRUE) + 1;
			}
		}
		else if (rangeResult == PARTIAL_IN_RANGE)
		{
			// recursive call:
			nCounter += pNode->DeleteBlocksInRange (nStart, nEnd, changes);
		}
	}		

	return nCounter;
}

int CBCGPOutlineNode::ToggleOutliningInRange (int nStart, int nEnd)
{
	ASSERT_VALID (this);

	// ---------------------------------------------
	// Toggle outlinig regions inside the selection:
	// ---------------------------------------------
	CObList lstBlocks;
	GetTopBlocksInRange (nStart, nEnd, lstBlocks);

	ToggleOutliningInternal (lstBlocks);

	return (int) lstBlocks.GetCount ();
}

int CBCGPOutlineNode::ToggleAllOutlining ()
{
	ASSERT_VALID (this);

	CObList lstBlocks;
	GetAllBlocks (lstBlocks);

	ToggleOutliningInternal (lstBlocks);

	return (int) lstBlocks.GetCount ();
}

void CBCGPOutlineNode::GetTopBlocksInRange (int nStart, int nEnd, CObList& lstBlocks) const
{
	if (nStart > nEnd)
	{
		return;
	}

	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pOutlineNode);

		CBCGPEditOutlineRange rangeResult = pOutlineNode->IsInRangeByOffset (nStart, nEnd);
		if (rangeResult == FULL_IN_RANGE)
		{
			lstBlocks.AddTail (pOutlineNode);
		}
		else if (rangeResult == PARTIAL_IN_RANGE)
		{
			// search sub blocks:
			CObList lstSubBlocks;
			pOutlineNode->GetTopBlocksInRange (nStart, nEnd, lstSubBlocks);
			lstBlocks.AddTail (&lstSubBlocks);
		}
	}		
}

void CBCGPOutlineNode::UpdateBlocksForDeletedRange (int nStart, int nEnd)
{
	CBCGPOutlineBaseNode::UpdateBlocksForDeletedRange (nStart, nEnd);

	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL; )
	{
		CBCGPOutlineNode* pSubNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pSubNode);

		pSubNode->UpdateBlocksForDeletedRange (nStart, nEnd);
	}
}

void CBCGPOutlineNode::UpdateBlocksForInsertedText (int nOffset, int nCount)
{
	CBCGPOutlineBaseNode::UpdateBlocksForInsertedText (nOffset, nCount);

	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL; )
	{
		CBCGPOutlineNode* pSubNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pSubNode);

		pSubNode->UpdateBlocksForInsertedText (nOffset, nCount);
	}
}

void CBCGPOutlineNode::ToggleOutliningInternal (CObList &lstBlocks)
{
	BOOL bHiding = TRUE;
	POSITION pos = NULL;

	// ---------------------------------------------
	// If some regions are expanded and some hidden, 
	// then all hidden regions are expanded.
	// ---------------------------------------------
	for (pos = lstBlocks.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineBaseNode* pOutlineBlock = DYNAMIC_DOWNCAST (CBCGPOutlineBaseNode, 
														lstBlocks.GetNext (pos));
		ASSERT_VALID (pOutlineBlock);
		if (pOutlineBlock->m_bCollapsed)
		{
			bHiding = FALSE;
			break;
		}
	}		

	// -----------------
	// Toggle outlining:
	// -----------------
	for (pos = lstBlocks.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineBaseNode* pOutlineBlock = DYNAMIC_DOWNCAST (CBCGPOutlineBaseNode, 
														lstBlocks.GetNext (pos));
		ASSERT_VALID (pOutlineBlock);
		pOutlineBlock->Collapse (bHiding);
	}		
}

CBCGPOutlineNode* CBCGPOutlineNode::AddNode (CBCGPOutlineNode* pNewNode, BCGP_EDIT_OUTLINE_CHANGES& changes)
{
	ASSERT_VALID (pNewNode);
	ASSERT (pNewNode->m_pParentNode == NULL);

	ASSERT (m_pEditCtrl != NULL);
	pNewNode->SetOwnerEditCtrl (m_pEditCtrl);

	// ----------------------------------------------------------------------
	// Insert new node in the list of subnodes to make the result list sorted
	// ----------------------------------------------------------------------
	POSITION pos = m_lstNodes.GetHeadPosition ();
	while (pos != NULL)
	{
		POSITION posSave = pos;

		CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pNode);

		// ------------------------------------------------
		// 1 new block's bounds is less then the next's one
		// ------------------------------------------------
		if (pNewNode->m_nEnd < pNode->m_nStart)
		{
			m_lstNodes.InsertBefore (posSave, pNewNode);
			pNewNode->m_pParentNode = this;
			changes.m_lstInserted.AddTail (pNewNode);
			break;
		}
		// -------------------------------
		// 2 new block covers the next one
		// -------------------------------
		else if (pNode->IsInRangeByOffset (
					pNewNode->m_nStart, 
					pNewNode->m_nEnd) == FULL_IN_RANGE)
		{
			// All blocks covered by new one must be inserted as subnodes:
			m_lstNodes.RemoveAt (posSave);
			pNode->m_pParentNode = NULL;
			POSITION posIns = changes.m_lstInserted.Find (pNode);
			if (posIns != NULL)
			{
				changes.m_lstInserted.RemoveAt (posIns);
			}

			BCGP_EDIT_OUTLINE_CHANGES changesTmp;
			pNewNode->AddNode (pNode, changesTmp);
			changes.m_lstRemoved.AddTail (&changesTmp.m_lstRemoved);
		}
		else
		{
			CBCGPOutlineBaseNode::CBCGPEditOutlineRange rangeResult = 
				pNewNode->IsInRangeByOffset (pNode->m_nStart, pNode->m_nEnd);
			// -----------------------------------------------------
			// 3 new block is FULL_IN_RANGE of the next one's bounds
			// -----------------------------------------------------
			if (rangeResult == FULL_IN_RANGE)
			{
				// Add as subblock
				pNode->AddNode (pNewNode, changes);
				break;
			}
			// --------------------------------------------------------
			// 4 new block is PARTIAL_IN_RANGE of the next one's bounds
			// --------------------------------------------------------
			else if (rangeResult == PARTIAL_IN_RANGE)
			{
				// Blocks intersection is not allowed, so delete block in list
				RemoveNode (posSave, changes);
				pos = m_lstNodes.GetHeadPosition ();
			}
		}
	}

	if (pNewNode->m_pParentNode == NULL)
	{
		m_lstNodes.AddTail (pNewNode);
		pNewNode->m_pParentNode = this;
		changes.m_lstInserted.AddTail (pNewNode);
	}

	if (m_pEditCtrl != NULL)
	{
		m_pEditCtrl->GetOutlineManager ().UpdateEvent ();
	}

	return pNewNode;
}

CBCGPOutlineNode* CBCGPOutlineNode::RemoveNode (POSITION pos, BCGP_EDIT_OUTLINE_CHANGES& changes, BOOL bRemoveSubNodes)
{
	if (pos != NULL)
	{
		CBCGPOutlineNode* pNodeSave = (CBCGPOutlineNode*) m_lstNodes.GetAt (pos);
		ASSERT_VALID (pNodeSave);

		m_lstNodes.RemoveAt (pos);
		changes.m_lstRemoved.AddTail (pNodeSave);

		if (!bRemoveSubNodes)
		{
			// -------------------------------------
			// Move all subnodes to the parent node:
			// -------------------------------------
			while (!pNodeSave->m_lstNodes.IsEmpty ())
			{
				CBCGPOutlineNode* pSubNode = (CBCGPOutlineNode*) pNodeSave->m_lstNodes.RemoveHead ();
				ASSERT_VALID (pSubNode);

				pSubNode->m_pParentNode = NULL;

				if (pNodeSave->m_pParentNode != NULL)
				{
					ASSERT_VALID (pNodeSave->m_pParentNode);
					pNodeSave->m_pParentNode->AddNode (pSubNode, changes);
				}
				else
				{
					AddNode (pSubNode, changes);
				}
			}
		}

		pNodeSave->m_pParentNode = NULL;

		if (m_pEditCtrl != NULL)
		{
			m_pEditCtrl->GetOutlineManager ().UpdateEvent ();
		}

		return pNodeSave;
	}

	return NULL;
}

void CBCGPOutlineNode::Collapse (BOOL bCollapsed)
{
	if ((m_dwFlags & g_dwOBFComplete) != 0)
	{
		m_bCollapsed = bCollapsed;
		if (m_pEditCtrl != NULL)
		{
			m_pEditCtrl->GetOutlineManager ().UpdateEvent ();
		}
	}
}

#pragma warning (disable : 4100) 

int CBCGPOutlineNode::Print (CString &str, int nLevel, int nCount) const
{
#ifdef _DEBUG
	CString strLine;

	CString strIndent (_T('\t'), nLevel);
	strLine.Format (_T("%d#%sOutlineBlock: (%d,%d), %d subnodes\n"), 
		nCount++, strIndent,
		m_nStart, m_nEnd,
		m_lstNodes.GetCount ());

	str += strLine;

	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pNode);

		CString strLine;
		nCount = pNode->Print (strLine, nLevel + 1, nCount);

		str += strLine;
	}
#endif // _DEBUG

	return nCount;
}

#pragma warning (default : 4100)

void CBCGPOutlineNode::Verify () const
{
#ifdef _DEBUG
	ASSERT_VALID (this);
	
	if (m_pParentNode != NULL)
	{
		ASSERT_VALID (m_pParentNode);
		ASSERT (m_nStart >= 0);
		ASSERT (m_nEnd >= 0);
		ASSERT (m_nStart <= m_nEnd);
	}
	else
	{
		ASSERT (m_nStart == -1);
		ASSERT (m_nEnd == -1);
	}

	int nOffset = 0;
	for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) m_lstNodes.GetNext (pos);
		ASSERT_VALID (pNode);
		ASSERT (pNode->m_nStart >= 0);
		ASSERT (pNode->m_nEnd >= 0);
		ASSERT (pNode->m_nStart <= pNode->m_nEnd);
		ASSERT (pNode->m_nNameOffset >= 0);
		ASSERT (pNode->m_nStart - pNode->m_nNameOffset >= 0);

		ASSERT (nOffset <= (pNode->m_nStart/* - pNode->m_nNameOffset*/));
		nOffset = pNode->m_nEnd;

		ASSERT_VALID (pNode->m_pParentNode);
		ASSERT (pNode->m_pParentNode == this);

		if (m_pParentNode != NULL)
		{
			ASSERT ((pNode->m_nStart - pNode->m_nNameOffset) >= m_nStart);
			ASSERT (pNode->m_nEnd <= m_nEnd);
		}

		pNode->Verify ();
	}
#endif // _DEBUG
}

//**************************************************************************************
CBCGPOutlineNode* CBCGPEditCtrl::FindCollapsedBlock (int nOffset) const
{
	if (!m_bEnableOutlining)
	{
		return NULL;
	}

	return m_OutlineNodes.FindCollapsedBlock (nOffset);
}
//**************************************************************************************
CBCGPOutlineNode* CBCGPEditCtrl::FindCollapsedBlockInRange (int nStart, int nEnd, BOOL bForward) const
{
	if (!m_bEnableOutlining)
	{
		return NULL;
	}

	return m_OutlineNodes.FindCollapsedBlockInRange (nStart, nEnd, bForward);
}
//***************************************************************************************
CBCGPOutlineNode* CBCGPEditCtrl::FindCollapsedBlockByPoint(CPoint point, int nOffset) const
{
	if (!m_bEnableOutlining)
	{
		return NULL;
	}

	int nRowStart = GetRowStartByOffset (nOffset, TRUE);
	int nRowEnd = GetRowEndByOffset (nOffset, TRUE);

	CPoint pt = point;
	pt.x += m_nScrollOffsetHorz * m_nMaxCharWidth;
	return m_OutlineNodes.FindCollapsedBlockByPoint (pt, nRowStart, nRowEnd);
}
//***************************************************************************************
void CBCGPEditCtrl::NextRow (int& nRow, int& nRowVirtual, int& nRowStartOffset) const
{
	ASSERT (nRow >= 0);
	ASSERT (nRowVirtual >= 0);
	ASSERT (nRowStartOffset >= 0);
	
	if (nRowStartOffset >= m_strBuffer.GetLength ())
	{
		nRowStartOffset = -1;
		return;
	}

	int iNextRow = m_strBuffer.Find (g_chEOL, nRowStartOffset);
	while (iNextRow != -1 && FindCollapsedBlock (iNextRow) != NULL) // skip hidden text
	{
		iNextRow = m_strBuffer.Find (g_chEOL, iNextRow + 1);
		nRow++;
	}

	nRow++;
	nRowVirtual++;
	nRowStartOffset = (iNextRow != -1) ? iNextRow + 1 : -1;
}
//**************************************************************************************
void CBCGPEditCtrl::GetMarkersInRangeByOffset (int nStart, int nEnd, CObList& lstMarkers, 
								DWORD dwMarkerType) const
{
	ASSERT_VALID (this);

	if (nStart > nEnd)
	{
		return;
	}

	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		CBCGPEditMarker* pMarkerNext = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarkerNext);
		if ((pMarkerNext->m_dwMarkerType & dwMarkerType) &&
			pMarkerNext->IsInRangeByOffset (nStart, nEnd))
		{
			lstMarkers.AddTail (pMarkerNext);
		}
	}		
}
//**************************************************************************************
int CBCGPEditCtrl::DeleteMarkersInRangeByOffset (int nStart, int nEnd, 
												  DWORD dwMarkerType, BOOL bRedraw)
{
	ASSERT_VALID (this);

	if (nStart > nEnd)
	{
		return 0;
	}

	int nCounter = 0;
	for (POSITION pos = m_lstMarkers.GetHeadPosition (); pos != NULL;)
	{
		POSITION posToDel = pos;

		CBCGPEditMarker* pMarker = DYNAMIC_DOWNCAST (CBCGPEditMarker, 
														m_lstMarkers.GetNext (pos));
		ASSERT_VALID (pMarker);
		if ((pMarker->m_dwMarkerType & dwMarkerType) &&
			pMarker->IsInRangeByOffset (nStart, nEnd))
		{
			m_lstMarkers.RemoveAt (posToDel);
			delete pMarker;
			nCounter++;
		}
	}		

	if (bRedraw)
	{
		RedrawTextOffsets (nStart, nEnd);
	}
	
	return nCounter;
}
//**************************************************************************************
POSITION CBCGPEditCtrl::SetHiliteMarker (int nStart, int nEnd, 
										  COLORREF clrFore, COLORREF clrBk,
										  DWORD dwMarkerType, UINT unPriority)
{
	ASSERT (nStart >= 0);
	ASSERT (nEnd <= m_strBuffer.GetLength());
	ASSERT (nStart <= nEnd);

	if (clrFore == (DWORD) -1)
	{
		clrFore = GetDefaultTextColor ();
	}

	if (clrBk == (DWORD) -1)
	{
		clrBk = ::GetSysColor (COLOR_HIGHLIGHTTEXT);
	}

	CBCGPHiliteMarker* pHiliteMarker = new CBCGPHiliteMarker;
	pHiliteMarker->m_nStart = nStart;
	pHiliteMarker->m_nEnd	= nEnd;
	pHiliteMarker->m_clrForeground = clrFore;
	pHiliteMarker->m_clrBackground = clrBk;
	pHiliteMarker->m_dwMarkerType = dwMarkerType | g_dwBCGPEdit_HiliteMarker;
	pHiliteMarker->m_unPriority = unPriority;

	return InsertMarker (pHiliteMarker);
}
//**************************************************************************************
CBCGPOutlineBaseNode* CBCGPEditCtrl::AddOutlining (int nStart, int nEnd, int nNameOffset,
													   BOOL bCollapsed, LPCTSTR lpszReplace)
{
	ASSERT (nStart >= 0);
	ASSERT (nEnd < m_strBuffer.GetLength());
	ASSERT (nStart <= nEnd);

	ASSERT (nNameOffset >= 0);
	ASSERT (nNameOffset <= nStart);

	if (m_strBuffer [nEnd] == _T('\n') && nEnd > nStart + 1)
	{
		nEnd--;
	}
	
	CBCGPOutlineBaseNode outlineBlock;
	outlineBlock.m_nStart = nStart;
	outlineBlock.m_nEnd = nEnd;
	outlineBlock.m_nNameOffset = nNameOffset;
	outlineBlock.m_bCollapsed = bCollapsed;
	outlineBlock.m_strReplace = (lpszReplace != NULL) ? lpszReplace : _T("...");
	outlineBlock.m_nBlockType = g_nOBTUserBlock;


	BCGP_EDIT_OUTLINE_CHANGES changes (nStart, nEnd + 1);
	
	int nVisibleLinesBefore = GetNumOfCharsInText (nStart, nEnd, g_chEOL, TRUE);

	CBCGPOutlineBaseNode* pBlock = m_OutlineNodes.AddBlock (outlineBlock, changes);

	int nVisibleLinesAfter = GetNumOfCharsInText (nStart, nEnd, g_chEOL, TRUE);
	m_nHiddenLines += nVisibleLinesBefore - nVisibleLinesAfter;
	UpdateScrollBars ();

	OnOutlineChanges (changes);

	return pBlock;
}
//**************************************************************************************
void CBCGPEditCtrl::HideSelection ()
{
	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel) - 1;

	if (iStartSel >= 0 && iEndSel < m_strBuffer.GetLength () && iStartSel <= iEndSel)
	{
		AddOutlining (iStartSel, iEndSel);
		
		RedrawTextOffsets (iStartSel, m_strBuffer.GetLength () - 1);
		SetCaret (iEndSel + 1);
	}
}
//**************************************************************************************
void CBCGPEditCtrl::ToggleOutlining ()
{
	m_OutlineNodes.Verify ();
	
	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel) - 1;

	if (iStartSel >= 0 && iEndSel < m_strBuffer.GetLength () && iStartSel <= iEndSel)
	{
		int nVisibleLinesBefore = GetNumOfCharsInText (iStartSel, iEndSel, g_chEOL, TRUE);

		// ----------------------------------------------
		// Toggle outlining regions inside the selection:
		// ----------------------------------------------
		if (m_OutlineNodes.ToggleOutliningInRange (iStartSel, iEndSel) > 0)
		{
			int nVisibleLinesAfter = GetNumOfCharsInText (iStartSel, iEndSel, g_chEOL, TRUE);
			m_nHiddenLines += nVisibleLinesBefore - nVisibleLinesAfter;
			UpdateScrollBars ();

			RedrawTextOffsets (iStartSel, m_strBuffer.GetLength () - 1);
			SetCaret (m_nCurrOffset);

			m_OutlineNodes.Verify ();
			return;
		}
	}

	if (m_nCurrOffset < 0 || m_nCurrOffset >= m_strBuffer.GetLength ())
	{
		m_OutlineNodes.Verify ();
		return;
	}

	// ---------------------------------------------------------------
	// Toggle the innermost outlining region in which the cursor lies:
	// ---------------------------------------------------------------
	CBCGPOutlineBaseNode* pOutlineNode = m_OutlineNodes.GetInnermostBlock (m_nCurrOffset);
	if (pOutlineNode != NULL)
	{
		ASSERT_VALID (pOutlineNode);

		int nVisibleLinesBefore = GetNumOfCharsInText (pOutlineNode->m_nStart, pOutlineNode->m_nEnd, g_chEOL, TRUE);

		pOutlineNode->Collapse (!pOutlineNode->IsCollapsed ());

		int nVisibleLinesAfter = GetNumOfCharsInText (pOutlineNode->m_nStart, pOutlineNode->m_nEnd, g_chEOL, TRUE);
		m_nHiddenLines += nVisibleLinesBefore - nVisibleLinesAfter;
		UpdateScrollBars ();
		
		RedrawTextOffsets (pOutlineNode->m_nStart, m_strBuffer.GetLength () - 1);
		if (m_nCurrOffset == pOutlineNode->m_nEnd + 1)
		{
			SetCaret (m_nCurrOffset);
		}
		else
		{
			SetCaret (pOutlineNode->m_nStart);
		}
	}

	m_OutlineNodes.Verify ();
}
//**************************************************************************************
void CBCGPEditCtrl::ToggleAllOutlining ()
{
	m_OutlineNodes.Verify ();

	int nVisibleLinesBefore = GetNumOfCharsInText (0, m_strBuffer.GetLength () - 1, g_chEOL, TRUE);

	m_OutlineNodes.ToggleAllOutlining ();

	int nVisibleLinesAfter = GetNumOfCharsInText (0, m_strBuffer.GetLength () - 1, g_chEOL, TRUE);
	m_nHiddenLines += nVisibleLinesBefore - nVisibleLinesAfter;
	UpdateScrollBars ();

	CPoint ptTopLeft (m_rectText.left, m_rectText.top);
	int nStartOffset = HitTest (ptTopLeft, FALSE, TRUE);
	OnScroll (SB_VERT, SB_THUMBTRACK, nStartOffset);

	m_nScrollOffsetVert = RowFromOffset (m_nTopVisibleOffset, TRUE, TRUE);
	m_nScrollHiddenVert = RowFromOffset (m_nTopVisibleOffset, TRUE, FALSE) - m_nScrollOffsetVert;
	SetScrollPos (SB_VERT, m_nScrollOffsetVert);

	CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlock (m_nCurrOffset);
	if (pHiddenText != NULL)
	{
		ASSERT_VALID (pHiddenText);
		SetCaret (pHiddenText->m_nStart, TRUE, FALSE);
	}
	else
	{
		SetCaret (m_nCurrOffset, TRUE, FALSE);
	}

	CRect rectArea;
	GetClientRect (rectArea);
	RedrawWindow (rectArea);

	m_OutlineNodes.Verify ();
}
//**************************************************************************************
void CBCGPEditCtrl::StopHidingCurrent ()
{
	m_OutlineNodes.Verify ();

	// This command is available only when auto-outlining is turned off 
	if (!m_bAutoOutlining)
	{
		int iStartSel = min (m_iStartSel, m_iEndSel);
		int iEndSel = max (m_iStartSel, m_iEndSel) - 1;

		BCGP_EDIT_OUTLINE_CHANGES changes (iStartSel, iEndSel + 1);

		if (iStartSel >= 0 && iEndSel < m_strBuffer.GetLength () && iStartSel <= iEndSel)
		{
			changes.m_nStartOffset = iStartSel;
			changes.m_nEndOffset = m_strBuffer.GetLength () - 1;

			int nVisibleLinesBefore = GetNumOfCharsInText (iStartSel, iEndSel, g_chEOL, TRUE);

			// ---------------------------------------------------
			// Stop hiding outlining regions inside the selection:
			// ---------------------------------------------------
			if (m_OutlineNodes.DeleteBlocksInRange (iStartSel, iEndSel, changes) > 0)
			{
				int nVisibleLinesAfter = GetNumOfCharsInText (iStartSel, iEndSel, g_chEOL, TRUE);
				m_nHiddenLines += nVisibleLinesBefore - nVisibleLinesAfter;
				UpdateScrollBars ();

				OnOutlineChanges (changes);
				SetCaret (m_nCurrOffset);

				m_OutlineNodes.Verify ();
				return;
			}
		}

		if (m_nCurrOffset < 0 || m_nCurrOffset >= m_strBuffer.GetLength ())
		{
			m_OutlineNodes.Verify ();
			return;
		}
		
		// --------------------------------------------------------------------
		// Stop hiding the innermost outlining region in which the cursor lies:
		// --------------------------------------------------------------------
		CBCGPOutlineBaseNode* pOutlineNode = m_OutlineNodes.GetInnermostBlock (m_nCurrOffset);
		if (pOutlineNode != NULL)
		{
			ASSERT_VALID (pOutlineNode);

			int nStart = pOutlineNode->m_nStart;
			int nEnd = pOutlineNode->m_nStart;

			changes.m_nStartOffset = nStart;
			changes.m_nEndOffset = m_strBuffer.GetLength () - 1;

			int nVisibleLinesBefore = GetNumOfCharsInText (nStart, nEnd, g_chEOL, TRUE);

			m_OutlineNodes.DeleteInnermostBlock (m_nCurrOffset, changes);

			int nVisibleLinesAfter = GetNumOfCharsInText (nStart, nEnd, g_chEOL, TRUE);
			m_nHiddenLines += nVisibleLinesBefore - nVisibleLinesAfter;
			UpdateScrollBars ();

			OnOutlineChanges (changes);
			SetCaret (m_nCurrOffset);
		}
	}

	m_OutlineNodes.Verify ();
}
//**************************************************************************************
void CBCGPEditCtrl::CollapseToDefinitions ()
{
	m_OutlineNodes.Verify ();
	EnableAutoOutlining (TRUE);
	UpdateAutoOutlining (0, m_strBuffer.GetLength () - 1);
	ToggleAllOutlining ();
	UpdateScrollBars ();
}
//**************************************************************************************
void CBCGPEditCtrl::StopOutlining ()
{
	EnableAutoOutlining (FALSE);
	m_OutlineNodes.DeleteAllBlocks ();

	m_nHiddenLines = 0;
	UpdateScrollBars ();

	CRect rectArea;
	GetClientRect (rectArea);
	RedrawWindow (rectArea);
	SetCaret (m_nCurrOffset, FALSE);
}
//**************************************************************************************
void CBCGPEditCtrl::ExpandLine (int nLineNum)
{
	ASSERT_VALID (this);

	if (!m_bEnableOutlining)
	{
		return;
	}

	int nOffset = GetRowStart (nLineNum - 1, FALSE);
	if (nOffset < 0)
	{
		return;
	}

	int nRowEndOffset = GetRowEndByOffset (nOffset, FALSE);

	// ---------------------------------------------------
	// Expand all collapsed regions at the specified line:
	// ---------------------------------------------------
	CObList lstOutlineNodes;
	m_OutlineNodes.GetBlocksInRange (nOffset, nRowEndOffset, lstOutlineNodes, FALSE, TRUE);

	// calc affected range
	int nMinStart = nOffset;
	int nMaxEnd = nRowEndOffset;
	POSITION pos = NULL;

	for (pos = lstOutlineNodes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineBaseNode* pOutlineBlock = DYNAMIC_DOWNCAST (CBCGPOutlineBaseNode, 
														lstOutlineNodes.GetNext (pos));
		ASSERT_VALID (pOutlineBlock);
		if (pOutlineBlock->IsCollapsed ())
		{
			if (pOutlineBlock->m_nStart < nMinStart)
			{
				nMinStart = pOutlineBlock->m_nStart;
			}
			if (pOutlineBlock->m_nEnd > nMaxEnd)
			{
				nMaxEnd = pOutlineBlock->m_nEnd;
			}
		}
	}		
	
	if (nMaxEnd >= m_strBuffer.GetLength ())
	{
		nMaxEnd = m_strBuffer.GetLength () - 1;
	}

	int nVisibleLinesBefore = GetNumOfCharsInText (nMinStart, nMaxEnd, g_chEOL, TRUE);

	// expand
	for (pos = lstOutlineNodes.GetHeadPosition (); pos != NULL;)
	{
		CBCGPOutlineBaseNode* pOutlineBlock = DYNAMIC_DOWNCAST (CBCGPOutlineBaseNode, 
														lstOutlineNodes.GetNext (pos));
		ASSERT_VALID (pOutlineBlock);
		if (pOutlineBlock->IsCollapsed ())
		{
			pOutlineBlock->Collapse (FALSE);
		}
	}		

	// update
	int nVisibleLinesAfter = GetNumOfCharsInText (nMinStart, nMaxEnd, g_chEOL, TRUE);
	m_nHiddenLines += nVisibleLinesBefore - nVisibleLinesAfter;
	UpdateScrollBars ();
	
	RedrawTextOffsets (nMinStart, m_strBuffer.GetLength () - 1);
}
//**************************************************************************************
BOOL CBCGPEditCtrl::GetHiddenTextFromPoint (CPoint pt, CString& strWord, BOOL bTrimEmptyLines)
{
	int nOffset = HitTest (pt);
	if (nOffset == -1 || nOffset >= m_strBuffer.GetLength ())
	{
		return FALSE;
	}

	CBCGPOutlineNode* pNode = FindCollapsedBlockByPoint(pt, nOffset);
	if (pNode != NULL)
	{
		int nCount = min (m_nMaxHintCharsNum, pNode->m_nEnd - pNode->m_nStart + 1);
		strWord = m_strBuffer.Mid (pNode->m_nStart, nCount);

		if (bTrimEmptyLines)
		{
			TrimEmptyLinesFromLeft (strWord);
		}
		
		return TRUE;
	}

	return FALSE;
}
//**************************************************************************************
BOOL CBCGPEditCtrl::GetHiddenTextFromOffset (int nOffset, CString& strWord)
{
	CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlock (nOffset);
	if (pHiddenText == NULL && nOffset > 0)
	{
		pHiddenText = FindCollapsedBlock (nOffset - 1);
	}

	if (pHiddenText != NULL)
	{
		ASSERT_VALID (pHiddenText);

		int nCount = min (m_nMaxHintCharsNum, pHiddenText->m_nEnd - pHiddenText->m_nStart + 1);
		strWord = m_strBuffer.Mid (pHiddenText->m_nStart, nCount);
		
		return TRUE;
	}

	return FALSE;
}
//************************************************************************************
CString CBCGPEditCtrl::GetVisibleText (int nStartOffset, int nEndOffset) const
{
	ASSERT (nStartOffset >= 0);
	ASSERT (nEndOffset < m_strBuffer.GetLength ());
	ASSERT (nStartOffset <= nEndOffset);

	CString str;

	for (int i = nStartOffset; i < nEndOffset; )
	{
		CBCGPOutlineBaseNode* pHiddenText = FindCollapsedBlockInRange (i, nEndOffset);
		if (pHiddenText != NULL)
		{
			ASSERT_VALID (pHiddenText);

			str += m_strBuffer.Mid (i, pHiddenText->m_nStart - i);

			if (nEndOffset == pHiddenText->m_nStart)
			{
				break;
			}

			i += max (pHiddenText->m_nEnd - i + 1, 1);
		}
		else
		{
			str += m_strBuffer.Mid (i, nEndOffset - i);
			break;
		}
	}

	return str;
}
//************************************************************************************
void CBCGPEditCtrl::OnDrawOutlineButton (CDC* pDC, CRect rectButton, CBCGPOutlineBaseNode* pRowOutlineBlock,
										 BOOL bInsideOpenBlockAtStart, BOOL bInsideOpenBlockAtEnd, BOOL bEndOfBlock)
{
	ASSERT_VALID (pDC);

#ifndef _BCGPEDIT_STANDALONE
	CPen pen (PS_SOLID, 1, m_clrLineOutline == (COLORREF)-1 ? CBCGPVisualManager::GetInstance()->GetEditOutlineColor(this): m_clrLineOutline);
#else
	CPen pen (PS_SOLID, 1, m_clrLineOutline == (COLORREF)-1 ? RGB(128, 128, 128): m_clrLineOutline);
#endif

	CPen* pPenOld = pDC->SelectObject (&pen);

	CBrush br (m_clrBackOutline == (COLORREF)-1 ? GetDefaultBackColor() : m_clrBackOutline);

	CBrush* pBrOld = pDC->SelectObject (&br);
	int nOldMode = pDC->SetBkMode (TRANSPARENT);

	CPoint ptCenter = rectButton.CenterPoint ();
	CPoint ptLeftTop = ptCenter;
	ptLeftTop.Offset (-4, -4);

	if (pRowOutlineBlock != NULL)
	{
		ASSERT_VALID (pRowOutlineBlock);
		
		// Draw button:
		CRect rect (ptLeftTop, CSize (9, 9));
		pDC->Rectangle (rect);
		pDC->MoveTo (ptCenter.x - 2, ptCenter.y);
		pDC->LineTo (ptCenter.x + 3, ptCenter.y);
		if (pRowOutlineBlock->m_bCollapsed)
		{
			pDC->MoveTo (ptCenter.x, ptCenter.y - 2);
			pDC->LineTo (ptCenter.x, ptCenter.y + 3);
		}
		
		// Draw lines:
		if (bInsideOpenBlockAtStart)
		{
			pDC->MoveTo (ptCenter.x, rectButton.top);
			pDC->LineTo (ptCenter.x, rect.top + 1);
		}
		if (bInsideOpenBlockAtEnd)
		{
			pDC->MoveTo (ptCenter.x, rectButton.bottom);
			pDC->LineTo (ptCenter.x, rect.bottom - 1);
		}
	}
	else
	{
		// Draw lines:
		if (bInsideOpenBlockAtStart)
		{
			pDC->MoveTo (ptCenter.x, rectButton.top);
			pDC->LineTo (ptCenter.x, ptCenter.y);
			if (bEndOfBlock)
			{
				pDC->LineTo (ptCenter.x + 5, ptCenter.y);
			}
		}
		if (bInsideOpenBlockAtEnd)
		{
			pDC->MoveTo (ptCenter.x, rectButton.bottom);
			pDC->LineTo (ptCenter.x, ptCenter.y - 1);
		}
	}

	pDC->SelectObject (pPenOld);
	pDC->SelectObject (pBrOld);
	pDC->SetBkMode (nOldMode);
}
//************************************************************************************
BOOL CBCGPEditCtrl::OnOutlineButtonClick (int nOffset)
{
	if (nOffset < 0)
	{
		return FALSE;
	}

	int nRowStartOffset = GetRowStartByOffset (nOffset, TRUE);
	int nRowEndOffset = GetRowEndByOffset (nRowStartOffset, TRUE);

	CObList lstOutlineNodes;
	m_OutlineNodes.GetBlocksInRange (nRowStartOffset, nRowEndOffset, lstOutlineNodes, FALSE);

	// --------------------------------
	// Get outline block for this line:
	// --------------------------------
	CBCGPOutlineNode* pRowOutlineNode = NULL;

	if (m_pOutlineNodeCurr != NULL)
	{
		POSITION posCurrNode = lstOutlineNodes.Find(m_pOutlineNodeCurr);
		if (posCurrNode != NULL)
		{
			CBCGPOutlineNode* pOutlineNodeCurr = (CBCGPOutlineNode*) lstOutlineNodes.GetAt (posCurrNode);
			ASSERT_VALID (pOutlineNodeCurr);

			if (pOutlineNodeCurr->m_nStart >= nRowStartOffset)
			{
				pRowOutlineNode = pOutlineNodeCurr;
			}
		}
	}
	
	for (POSITION pos = lstOutlineNodes.GetHeadPosition (); pos != NULL; )
	{
		CBCGPOutlineNode* pOutlineNode = (CBCGPOutlineNode*) lstOutlineNodes.GetNext (pos);
		ASSERT_VALID (pOutlineNode);

		if ((pOutlineNode->m_dwFlags & g_dwOBFComplete) == 0)
		{
			continue;
		}

		if (pRowOutlineNode == NULL && pOutlineNode->m_nStart >= nRowStartOffset)
		{
			pRowOutlineNode = pOutlineNode;
		}
	}

	// ----------------------------
	// Handle outline button click:
	// ----------------------------
	if (pRowOutlineNode != NULL)
	{
		return OnOutlineButtonClick (pRowOutlineNode);
	}

	return FALSE;
}
//************************************************************************************
BOOL CBCGPEditCtrl::OnOutlineButtonClick (CBCGPOutlineNode *pOutlineNode)
{
	ASSERT_VALID (pOutlineNode);

	RemoveSelection (FALSE, TRUE, FALSE);

	int nVisibleLinesBefore = GetNumOfCharsInText (pOutlineNode->m_nStart, pOutlineNode->m_nEnd, g_chEOL, TRUE);

	pOutlineNode->Collapse (!pOutlineNode->IsCollapsed ());

	int nVisibleLinesAfter = GetNumOfCharsInText (pOutlineNode->m_nStart, pOutlineNode->m_nEnd, g_chEOL, TRUE);
	m_nHiddenLines += nVisibleLinesBefore - nVisibleLinesAfter;
	UpdateScrollBars ();

	RedrawTextOffsets (pOutlineNode->m_nStart, m_strBuffer.GetLength () - 1);

	int nRowStart = GetRowStartByOffset (pOutlineNode->m_nStart, TRUE);
	SetCaret (nRowStart);

	return TRUE;
}
//************************************************************************************
void CBCGPEditCtrl::UpdateOffsetRelatedData (int nStartOffset, int nEndOffset)
{
	ASSERT (nStartOffset >= 0);
	ASSERT (nEndOffset <= m_strBuffer.GetLength ());
	ASSERT (nStartOffset <= nEndOffset);

	if (m_bEnableOutlining)
	{
		m_OutlineNodes.UpdateBlocksForInsertedText (nStartOffset, nEndOffset - nStartOffset);
		GetOutlineManager ().UpdateEvent ();
	}
}
//************************************************************************************
void CBCGPEditCtrl::RemoveOffsetDataInRange (int nStartOffset, int nEndOffset)
{
	ASSERT (nStartOffset >= 0);
	ASSERT (nEndOffset <= m_strBuffer.GetLength ());
	ASSERT (nStartOffset <= nEndOffset);

	if (m_bEnableOutlining)
	{
		BCGP_EDIT_OUTLINE_CHANGES changes (nStartOffset, nEndOffset + 1);

		m_OutlineNodes.DeleteBlocksInRange (nStartOffset, nEndOffset, changes);
		m_OutlineNodes.UpdateBlocksForDeletedRange (nStartOffset, nEndOffset);
		GetOutlineManager ().UpdateEvent ();

		UpdateHiddenLines (changes);

		OnOutlineChanges (changes, FALSE);
	}
}
//****************************************************************************************
void CBCGPEditCtrl::EnableOutlining (BOOL bEnable)
{
    if (m_bEnableOutlining != bEnable)
    {
        m_bEnableOutlining = bEnable;

    	UpdateScrollBars ();
	    m_nScrollOffsetVert = RowFromOffset (m_nTopVisibleOffset, TRUE, TRUE);
	    m_nScrollHiddenVert = RowFromOffset (m_nTopVisibleOffset, TRUE, FALSE) - m_nScrollOffsetVert;
        RedrawWindow ();
        SetCaret (m_nCurrOffset, FALSE);
    }
}
//****************************************************************************************
void CBCGPEditCtrl::SetLineNumbersMargin (BOOL bShow, int nMarginWidth)
{
	ASSERT (nMarginWidth >= 0);
	BOOL bShowChanged = (bShow != m_bEnableLineNumbersMargin);
	m_bEnableLineNumbersMargin = bShow;
	m_nLineNumbersMarginWidth = nMarginWidth;

	if (OnUpdateLineNumbersMarginWidth () || bShowChanged)
	{
		RecalcLayout ();
		SetCaret (m_nCurrOffset, TRUE, FALSE);
		RedrawWindow ();
	}
}
//****************************************************************************************
void CBCGPEditCtrl::UpdateAutoOutlining (int nStartOffset, int nEndOffset)
{
	if (nStartOffset == -1)
	{
		nStartOffset = 0;
	}
	if (nEndOffset == -1)
	{
		nEndOffset = m_strBuffer.GetLength () - 1;
	}

	OnUpdateAutoOutlining (nStartOffset, nEndOffset - nStartOffset + 1);
}
//************************************************************************************
void CBCGPEditCtrl::OnUpdateAutoOutlining (int nOffsetFrom, int nCharsCount, BOOL bRedraw)
{
	if (!m_bEnableOutlining || !m_bAutoOutlining)
	{
		return;
	}

	if (nOffsetFrom < 0 ||
		nCharsCount < 0 ||
		nOffsetFrom + nCharsCount > m_strBuffer.GetLength ())
	{
		return;
	}

	if (m_pOutlineParser != NULL && m_bOutlineParserEnabled)
	{
		ASSERT_VALID (m_pOutlineParser);

		if (nCharsCount == 1 && 
			!m_pOutlineParser->IsValuedChar (m_strBuffer [nOffsetFrom]))
		{
			// no reparse needed
		}
		else
		{
			BCGP_EDIT_OUTLINE_CHANGES changes (nOffsetFrom, nOffsetFrom + nCharsCount + 1);

			// --------
			// Reparse:
			// --------
			m_pOutlineParser->UpdateOutlining (m_strBuffer, nOffsetFrom, nCharsCount, &m_OutlineNodes, changes);

			UpdateHiddenLines (changes);

			OnOutlineChanges (changes, bRedraw);
		}
	}
}
//************************************************************************************
void CBCGPEditCtrl::OnOutlineChanges (BCGP_EDIT_OUTLINE_CHANGES& changes, BOOL bRedraw)
{
	POSITION pos = NULL;

	// -------------------
	// Notify all changes:
	// -------------------
	for (pos = changes.m_lstInserted.GetHeadPosition (); pos != NULL; )
	{
		CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) changes.m_lstInserted.GetNext (pos);
		ASSERT_VALID (pNode);

		changes.m_nStartOffset = min (pNode->m_nStart - pNode->m_nNameOffset, changes.m_nStartOffset);
		changes.m_nEndOffset = max (pNode->m_nEnd, changes.m_nEndOffset);

		OnInsertOutlineBlock (pNode);
	}
	for (pos = changes.m_lstRemoved.GetHeadPosition (); pos != NULL; )
	{
		CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) changes.m_lstRemoved.GetNext (pos);
		ASSERT_VALID (pNode);

		changes.m_nStartOffset = min (pNode->m_nStart - pNode->m_nNameOffset, changes.m_nStartOffset);
		changes.m_nEndOffset = max (pNode->m_nEnd, changes.m_nEndOffset);

		OnRemoveOutlineBlock (pNode);
	}

	// -------------------------------
	// Store all changes in undo list:
	// -------------------------------
	while (!changes.m_lstRemoved.IsEmpty ())
	{
		delete changes.m_lstRemoved.RemoveHead ();
	}
	
	// ------------
	// Update view:
	// ------------
	if (bRedraw)
	{
		RedrawTextOffsets (changes.m_nStartOffset, m_strBuffer.GetLength () - 1);
	}
	else
	{
		int nStart = changes.m_nStartOffset;

		CPoint ptStart, ptEnd;
		
		OffsetToPoint (nStart, ptStart);
		ptStart.x = 0;

		CRect rectArea;
		GetClientRect (rectArea);
		ptEnd = rectArea.BottomRight ();

		CRect rect (ptStart, ptEnd);

		InvalidateRect (rect);
	}
}
//************************************************************************************
BOOL CBCGPEditCtrl::LoadOutlineParserXMLSettings (CString& strInBuffer)
{
	if (m_pOutlineParser != NULL)
	{
		m_OutlineNodes.DeleteAllBlocks ();
		m_pOutlineParser->RemoveAllBlockTypes ();

		CString strBlocks;
		//-----------------------
		// Read escape sequences:
		//-----------------------
		if (globalData.ExcludeTag (strInBuffer, _T("EscapeSequences"), strBlocks))
		{
			while (TRUE)
			{
				CString strEscapeSequence;
				if (!globalData.ExcludeTag (strBlocks, _T("EscapeSequence"), strEscapeSequence, TRUE))
				{
					break;
				}

				m_pOutlineParser->AddEscapeSequence (strEscapeSequence);
			}
		}

		//-------------
		// Read blocks:
		//-------------
		if (globalData.ExcludeTag (strInBuffer, _T("BLOCKS"), strBlocks))
		{
			while (TRUE)
			{
				CString strBlock;
				if (!globalData.ExcludeTag (strBlocks, _T("BLOCK"), strBlock))
				{
					break;
				}

				CString strOpen;
				if (!globalData.ExcludeTag (strBlock, _T("Start"), strOpen, TRUE))
				{
					ASSERT (FALSE);
					break;
				}

				CString strClose;
				if (!globalData.ExcludeTag (strBlock, _T("End"), strClose, TRUE))
				{
					ASSERT (FALSE);
					break;
				}

				CString	strReplace;
				if (!globalData.ExcludeTag (strBlock, _T("ReplaceString"), strReplace, TRUE))
				{
					strReplace = _T("...");
				}

				BOOL bAllowNestedBlocks = TRUE;
				CString strNested;
				if (globalData.ExcludeTag (strBlock, _T("AllowNestedBlocks"), strNested))
				{
					strNested.MakeUpper ();
					bAllowNestedBlocks = (strNested == _T("TRUE"));
				}

				BOOL bIgnore = FALSE;
				CString strIgnore;
				if (globalData.ExcludeTag (strBlock, _T("Ignore"), strIgnore))
				{
					strIgnore.MakeUpper ();
					bIgnore = (strIgnore == _T("TRUE"));
				}
				
				CStringList lstKeywords; 
				CString strKeywords;
				if (globalData.ExcludeTag (strBlock, _T("KEYWORDS"), strKeywords))
				{
					while (TRUE)
					{
						CString strKeyword;
						if (!globalData.ExcludeTag (strKeywords, _T("Keyword"), strKeyword))
						{
							break;
						}

						lstKeywords.AddTail (strKeyword);;
					}
				}

				m_pOutlineParser->AddBlockType (strOpen, strClose, strReplace, 
												bAllowNestedBlocks, bIgnore, &lstKeywords);
			}
		}
		
		// --------------------
		// Additional settings:
		// --------------------
		m_pOutlineParser->m_strDelimiters = m_strWordDelimeters;

		BOOL bCaseSensitive = TRUE;
		CString strCaseSensitive;
		if (globalData.ExcludeTag (strInBuffer, _T("CaseSensitive"), strCaseSensitive))
		{
			strCaseSensitive.MakeUpper ();
			bCaseSensitive = (strCaseSensitive == _T("TRUE"));
		}
		m_pOutlineParser->m_bCaseSensitive = bCaseSensitive;

		BOOL bWholeWords = FALSE;
		CString strWholeWords;
		if (globalData.ExcludeTag (strInBuffer, _T("WholeWords"), strWholeWords))
		{
			strWholeWords.MakeUpper ();
			bWholeWords = (strWholeWords == _T("TRUE"));
		}
		m_pOutlineParser->m_bWholeWords = bWholeWords;
	}

	return TRUE;
}
//************************************************************************************
void CBCGPEditCtrl::UpdateHiddenLines (BCGP_EDIT_OUTLINE_CHANGES& changes)
{
	if (changes.m_lstRemoved.GetCount () == 0 &&
		changes.m_lstInserted.GetCount () == 0)
	{
		return;	// improves performance
	}

	int nHiddenLinesBefore = 0;
	POSITION pos = NULL;

	for (pos = changes.m_lstRemoved.GetHeadPosition (); pos != NULL; )
	{
		CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) changes.m_lstRemoved.GetNext (pos);
		ASSERT_VALID (pNode);

		CObList lst;
		if (pNode->m_bCollapsed)
		{
			lst.AddTail (pNode);
		}
		else
		{
			pNode->GetBlocksByState (lst, TRUE);
		}

		for (POSITION posInner = lst.GetHeadPosition (); posInner != NULL; )
		{
			CBCGPOutlineBaseNode* pNode = (CBCGPOutlineBaseNode*) lst.GetNext (posInner);
			ASSERT_VALID (pNode);

			int nHiddenLines = GetNumOfCharsInText (pNode->m_nStart, pNode->m_nEnd, g_chEOL, FALSE);
			nHiddenLinesBefore += nHiddenLines;
		}
	}
	int nHiddenLinesAfter = 0;
	for (pos = changes.m_lstInserted.GetHeadPosition (); pos != NULL; )
	{
		CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) changes.m_lstInserted.GetNext (pos);
		ASSERT_VALID (pNode);

		CObList lst;
		if (pNode->m_bCollapsed)
		{
			lst.AddTail (pNode);
		}
		else
		{
			pNode->GetBlocksByState (lst, TRUE);
		}

		for (POSITION posInner = lst.GetHeadPosition (); posInner != NULL; )
		{
			CBCGPOutlineBaseNode* pNode = (CBCGPOutlineBaseNode*) lst.GetNext (posInner);
			ASSERT_VALID (pNode);


			int nHiddenLines = GetNumOfCharsInText (pNode->m_nStart, pNode->m_nEnd, g_chEOL, FALSE);
			nHiddenLinesAfter += nHiddenLines;
		}
	}
	m_nHiddenLines += nHiddenLinesAfter - nHiddenLinesBefore;
	UpdateScrollBars ();

	m_nScrollOffsetVert = RowFromOffset (m_nTopVisibleOffset, TRUE, TRUE);
	m_nScrollHiddenVert = RowFromOffset (m_nTopVisibleOffset, TRUE, FALSE) - m_nScrollOffsetVert;
}
//************************************************************************************
static CString ColorTag (COLORREF clr)
//COLORREF to tag
{
	BYTE bR = GetRValue (clr);
	BYTE bG = GetGValue (clr);
	BYTE bB = GetBValue (clr);

    CString strColorTag;
    strColorTag.Format (_T("<font color=\"#%0.2x%0.2x%0.2x\">"), bR, bG, bB);

	return strColorTag;
}
//************************************************************************************
void CBCGPEditCtrl::ExportToHTML (CString& strHTML)
{
	int nStartOffset = 0; 
	int nEndOffset = m_strBuffer.GetLength () - 1;

	COLORREF clrDefaultText = GetDefaultTextColor ();
	COLORREF clrDefaultBack = GetDefaultBackColor ();

	COLORREF clrNextWordFore	= clrDefaultText;
	COLORREF clrNextWordBk		= clrDefaultBack;

	COLORREF clrOutText = clrDefaultText;
	COLORREF clrOutBack = clrDefaultBack;

	// ----------------------------------------------
	// Build color areas for visible part of a buffer (comments, strings and so on):
	// ----------------------------------------------
	BCGP_EDIT_COLOR_BLOCK	clrBlock; 
	BOOL bIsOpenBlock = FindOpenBlock (nStartOffset, &clrBlock);

	int nCloseBlockOffset = -1;
	if (bIsOpenBlock)
	{
		nCloseBlockOffset = FindCloseBlock (nStartOffset, &clrBlock);
	}

	COLORREF clrBlockFore = (clrBlock.GetForegroundColor(m_bIsDarkBackground) == -1) ? GetDefaultTextColor () : clrBlock.GetForegroundColor(m_bIsDarkBackground);
	COLORREF clrBlockBack = (clrBlock.m_clrBackground == -1) ? GetDefaultBackColor () : clrBlock.m_clrBackground; 

	CList <BCGP_EDIT_COLOR_AREA, BCGP_EDIT_COLOR_AREA&> colorAreas;

	if (bIsOpenBlock) 
	{
		if (nCloseBlockOffset < nEndOffset)
		{
			BuildColorAreas (colorAreas, nCloseBlockOffset, nEndOffset, NULL, FALSE);
		}
	}
	else
	{
		BuildColorAreas (colorAreas, nStartOffset, nEndOffset, NULL, FALSE);
	}
	
	// ---------------------------------------------
	// Draw the text for the visible part of buffer:
	// ---------------------------------------------
	TCHAR* lpszOutBuffer = (TCHAR*) alloca (sizeof (TCHAR) * m_nMaxScrollWidth);
	memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
	int iIdx = 0;

	int nNextDelimiter = -1;
	int nCurrRow = 0;
	int nCurrColumn = 0;
	
	for (int i = nStartOffset; i <= nEndOffset; i++)
	{
		COLORREF clrForeground = clrDefaultText;
		COLORREF clrBackground = clrDefaultBack;

		TCHAR chNext = m_strBuffer [i];

		// --------------
		// Define colors:
		// --------------
		BOOL bColorFound = FALSE;

		// 1) ignore selection
		// 2) ignore colored line markers

		// 3) check for color blocks:
		if (bIsOpenBlock && i < nCloseBlockOffset)
		{
			clrForeground = clrBlockFore;
			clrBackground = clrBlockBack;
			bColorFound = TRUE;
		}
		else
		{
			// check all color areas (comments, strings and so on)
			for (POSITION pos = colorAreas.GetHeadPosition (); pos != NULL;)
			{
				BCGP_EDIT_COLOR_AREA colorArea = colorAreas.GetNext (pos);
				if (i >= colorArea.m_nStart && i <= colorArea.m_nEnd)
				{
					clrForeground = colorArea.GetForegroundColor(m_bIsDarkBackground); 
					clrBackground = colorArea.m_clrBackground;

					if (clrForeground == -1)
					{
						clrForeground = clrDefaultText;
					}

					if (clrBackground == -1)
					{
						clrBackground = clrDefaultBack;
					}

					bColorFound = TRUE;
					break;
				}
			}
		}

		// 4) User can define text color by own:
		if (OnGetTextColor (i, nNextDelimiter, clrForeground, clrBackground, bColorFound))
		{
			bColorFound = TRUE;
		}

		// 5) Check all color words (keywords and words):
		if (chNext != g_chEOL)
		{
			if (nNextDelimiter == -1) 
			{
				CString strNextWord;		
				
				for (int iIdx = i; iIdx <= nEndOffset; iIdx++)
				{
					TCHAR ch = m_strBuffer [iIdx];
					if (m_strWordDelimeters.Find (ch) != -1)
					{
						nNextDelimiter = iIdx;
						break;
					}
				}

				if (nNextDelimiter == -1)
				{
					nNextDelimiter = nEndOffset + 1;
				}

				if (nNextDelimiter != -1)
				{
					strNextWord = 
						m_strBuffer.Mid (i, nNextDelimiter - i);
				}
						
				if (!OnGetWordColor (strNextWord, clrNextWordFore, clrNextWordBk, i))
				{
					clrNextWordFore = clrDefaultText;
					clrNextWordBk = clrDefaultBack;
				}
			}

			if (i >= nNextDelimiter - 1)
			{
				nNextDelimiter = -1;
			}

			if (!bColorFound)
			{
				clrForeground = clrNextWordFore;
				clrBackground = clrNextWordBk;
			}
		}

		// 6) ignore hilited text

		// 7) User can define color of current char:
		if (!bColorFound)
		{
			OnGetCharColor (chNext, i, clrForeground, clrBackground);
		}

		// -----------------------------------
		// Proceed end of same color fragment:
		// -----------------------------------
		
		if (clrForeground != clrOutText || clrBackground != clrOutBack)
		{
			clrOutText = clrForeground;
			clrOutBack = clrBackground;

			if (iIdx != 0)
			{
				CString strLine (lpszOutBuffer);
				OnPrepareHTMLString (strLine);
				strHTML += strLine;

				iIdx = 0;
				memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
			}

			strHTML += ColorTag (clrOutText);
		}
		
		// --------------------
		// Proceed end of line:
		// --------------------
		if (chNext == g_chEOL)
		{
			{
				CString strLine (lpszOutBuffer);
				OnPrepareHTMLString (strLine);
				strLine += _T("<BR>");
				strLine += g_chEOL;
				strHTML += strLine;

				iIdx = 0;
				memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
			}

			nCurrRow++;
			nCurrColumn = 0;
			continue;
		}

		// -------------
		// Replace Tabs:
		// -------------
		if (chNext == _T('\t'))
		{
			int nRestOfTab = m_nTabSize - nCurrColumn % m_nTabSize;
			nCurrColumn += nRestOfTab;

			for (int k = 0; k < nRestOfTab; k++)
			{
				if (iIdx + k + 1 > m_nMaxScrollWidth - 1)
				{
					CString strLine (lpszOutBuffer);
					OnPrepareHTMLString (strLine);
					strHTML += strLine;

					iIdx = 0;
					memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
				}

				lpszOutBuffer [iIdx] = _T(' ');
				lpszOutBuffer [iIdx + 1] = _T ('\0');
				iIdx ++;
			}
		}

		else
		{
			nCurrColumn++;

			if (iIdx + 1 > m_nMaxScrollWidth - 1)
			{
				CString strLine (lpszOutBuffer);
				OnPrepareHTMLString (strLine);
				strHTML += strLine;

				iIdx = 0;
				memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
			}

			lpszOutBuffer [iIdx] = chNext;
			lpszOutBuffer [iIdx + 1] = _T ('\0');
			iIdx++;
		}
	}

	// --------------------------
	// Draw the last of the text:
	// --------------------------
	if (iIdx != 0)
	{
		CString strLine (lpszOutBuffer);
		OnPrepareHTMLString (strLine);
		strHTML += strLine;

		iIdx = 0;
		memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
	}
}
//************************************************************************************
void CBCGPEditCtrl::OnPrepareHTMLString (CString& str) const
{
	str.Replace (_T(" "), _T("&nbsp;"));
	str.Replace (_T("<"), _T("&lt;"));
	str.Replace (_T(">"), _T("&gt;"));
}
//************************************************************************************
BOOL CBCGPEditCtrl::FindHyperlinkString (int nStartOffset, int nEndOffset, CString& strURL, int &nOffset)
{
	strURL = m_strBuffer.Mid (nStartOffset, nEndOffset - nStartOffset);
	
	int nFoundIdx = -1;
	int nPrefixLen = 0;
	for (POSITION pos = m_lstURLPrefixes.GetHeadPosition (); pos != NULL; )
	{
		CString& strURLPrefix = m_lstURLPrefixes.GetNext (pos);

		int nIdx = strURL.Find (strURLPrefix);
		if (nIdx != -1 && (nFoundIdx == -1 || nIdx < nFoundIdx) &&
			(nIdx == 0 || m_strWordDelimeters.Find (strURL.GetAt (nIdx - 1)) != -1))
		{
			nFoundIdx = nIdx;
			nPrefixLen = strURLPrefix.GetLength ();
		}
	}

	if (nFoundIdx >= 0 && nStartOffset + nFoundIdx < nEndOffset)
	{
		int nEndIdx = nFoundIdx + nPrefixLen;
		for (int i = nFoundIdx + nPrefixLen;
			 i < nEndOffset - nStartOffset;)
		{
			TCHAR chNext = strURL.GetAt (i);
			i++;
			if (m_strWordDelimeters.Find (chNext) == -1)
			{
				nEndIdx = i;
			}
		}

		nOffset = nStartOffset + nFoundIdx;
		strURL = m_strBuffer.Mid (nStartOffset + nFoundIdx, nEndIdx - nFoundIdx);
		return TRUE;
	}

	return FALSE;
}
//************************************************************************************
void CBCGPEditCtrl::AddHyperlink (CString& str, int nOffset)
{
	BCGP_EDIT_HYPERLINK_DEF hyperlink (str, nOffset);
	m_arrHyperlinks.Add (hyperlink);
}
//************************************************************************************
void CBCGPEditCtrl::ClearHyperlinks ()
{
	m_arrHyperlinks.RemoveAll ();
}
//************************************************************************************
int CBCGPEditCtrl::LookupHyperlinkByOffset (int nOffset)
{
	for (int i = 0; i <= m_arrHyperlinks.GetUpperBound (); i++)
	{
		const int nHyperlinkOffset = m_arrHyperlinks [i].m_nHyperlinkOffset;
		if (nHyperlinkOffset <= nOffset &&
			nHyperlinkOffset + m_arrHyperlinks [i].m_strHyperlink.GetLength () >= nOffset)
		{
			return i;
		}
	}

	return -1;
}
//************************************************************************************
void CBCGPEditCtrl::OnHyperlinkClick (int nOffset, LPCTSTR lpszURL)
{
	ASSERT (lpszURL != NULL);

	ReleaseCapture ();

	CWaitCursor wait;

	if (::ShellExecute (NULL, NULL, lpszURL, NULL, NULL, NULL) < (HINSTANCE) 32)
	{
		TRACE(_T("Can't open URL: %s\n"), lpszURL);
	}

	m_nCurrHyperlinkHot = -1;
	RedrawRestOfLine (GetRowStartByOffset (nOffset));
}
//************************************************************************************
void CBCGPEditCtrl::HyperlinkClick ()
{
	m_nCurrHyperlink = LookupHyperlinkByOffset(m_nCurrOffset);
	if (m_bEnableHyperlinkSupport && 
		m_nCurrHyperlink >= 0 && m_nCurrHyperlink <= m_arrHyperlinks.GetUpperBound ())
	{
		BCGP_EDIT_HYPERLINK_DEF hyperlink = m_arrHyperlinks [m_nCurrHyperlink];
		OnHyperlinkClick (hyperlink.m_nHyperlinkOffset, hyperlink.m_strHyperlink);
	}
}
//************************************************************************************
void CBCGPEditCtrl::SetHyperlinkColor (BOOL bUseColor, COLORREF clr)
{
	m_bColorHyperlink = bUseColor;
	m_clrHyperlink = clr;
	m_clrHyperlinkLight = BCGP_EDIT_COLOR_BLOCK::MakeLightColor(m_clrHyperlink);
}
//************************************************************************************
BOOL CBCGPEditCtrl::GetHyperlinkString (CString& strURL)
{
	if (m_bEnableHyperlinkSupport)
	{
		m_nCurrHyperlink = LookupHyperlinkByOffset(m_nCurrOffset);
		int nHyperlink = (m_nCurrHyperlinkHot != -1) ? m_nCurrHyperlinkHot : m_nCurrHyperlink;

		if (nHyperlink >= 0 && nHyperlink <= m_arrHyperlinks.GetUpperBound ())
		{
			BCGP_EDIT_HYPERLINK_DEF hyperlink = m_arrHyperlinks [nHyperlink];
			strURL = hyperlink.m_strHyperlink;
			return OnGetHyperlinkString (hyperlink.m_nHyperlinkOffset, strURL);
		}
	}

	strURL.Empty ();
	return FALSE;
}
//************************************************************************************
BOOL CBCGPEditCtrl::GetHyperlinkToolTip (CString& strToolTip)
{
	if (m_bEnableHyperlinkSupport)
	{
		if (m_nCurrHyperlinkHot >= 0 && m_nCurrHyperlinkHot <= m_arrHyperlinks.GetUpperBound ())
		{
			BCGP_EDIT_HYPERLINK_DEF hyperlink = m_arrHyperlinks [m_nCurrHyperlinkHot];
			strToolTip.Format (_T("%s\nCTRL + click to follow link"), hyperlink.m_strHyperlink);
			return OnGetHyperlinkToolTip (hyperlink.m_nHyperlinkOffset, strToolTip);
		}
	}

	strToolTip.Empty ();
	return FALSE;
}
//************************************************************************************
BOOL CBCGPEditCtrl::LoadHyperlinkXMLSettings (CString& strInBuffer)
{
	//--------------------------------
	// Read common hyperlink settings:
	//--------------------------------
	m_bEnableHyperlinkSupport = FALSE;
	CString strEnable;
	if (globalData.ExcludeTag (strInBuffer, _T("EnableHyperlinkSupport"), strEnable))
	{
		strEnable.MakeUpper ();
		m_bEnableHyperlinkSupport = (strEnable == _T("TRUE"));
	}

	m_bColorHyperlink = TRUE;
	CString strColorHyperlink;
	if (globalData.ExcludeTag (strInBuffer, _T("HighlightWithColor"), strColorHyperlink))
	{
		strColorHyperlink.MakeUpper ();
		m_bColorHyperlink = (strColorHyperlink == _T("TRUE"));
	}

	CString strColor;
	if (globalData.ExcludeTag (strInBuffer, _T("Color"), strColor))
	{
		m_clrHyperlink = GetColorFromBuffer (strColor);
		m_clrHyperlinkLight = BCGP_EDIT_COLOR_BLOCK::MakeLightColor(m_clrHyperlink);
	}

	//-------------------
	// Read URL prefixes:
	//-------------------
	CString strURLPrefixes;
	if (globalData.ExcludeTag (strInBuffer, _T("URLPrefixes"), strURLPrefixes))
	{
		m_lstURLPrefixes.RemoveAll ();
		m_arrHyperlinks.RemoveAll ();
		m_nCurrHyperlink = -1;
		m_nCurrHyperlinkHot = -1;

		while (TRUE)
		{
			CString strURLPrefix;
			if (!globalData.ExcludeTag (strURLPrefixes, _T("URLPrefix"), strURLPrefix))
			{
				break;
			}

			m_lstURLPrefixes.AddTail (strURLPrefix);
		}
	}

	return TRUE;
}
//************************************************************************************
int CBCGPEditCtrl::GetLineInternal (int nLine, LPTSTR lpszBuffer, int nMaxLength) const
{
	ASSERT (lpszBuffer != NULL);

	int nStartOffset = GetRowStart (nLine);

	if (nStartOffset == -1)
	{
		return 0;
	}

	int nEndOffset = GetRowEndByOffset (nStartOffset);

	if (nEndOffset > nStartOffset && nEndOffset <= m_strBuffer.GetLength ())
	{
		CString strLine = m_strBuffer.Mid (nStartOffset, nEndOffset - nStartOffset);
		const int nLineLen = strLine.GetLength ();

		lstrcpyn (lpszBuffer, (LPCTSTR) strLine, min (nLineLen + 1, nMaxLength));

		return min (nLineLen, nMaxLength - 1);
	}

	return 0;
}
//************************************************************************************
int CBCGPEditCtrl::GetLine (int nLine, LPTSTR lpszBuffer) const
{
	ASSERT (lpszBuffer != NULL);
	const int nMaxLength = *((WORD*) lpszBuffer);

	return GetLineInternal (nLine, lpszBuffer, nMaxLength);
}
//************************************************************************************
int CBCGPEditCtrl::GetLine (int nLine, LPTSTR lpszBuffer, int nMaxLength) const
{
	return GetLineInternal (nLine, lpszBuffer, nMaxLength);
}
//************************************************************************************
BOOL CBCGPEditCtrl::GetCurLine (CString& strLine) const
{
	if (m_nCurrOffset < 0)
	{
		return FALSE;
	}

	int nLineBeginOffset = GetRowStartByOffset (m_nCurrOffset);
	int nLineEndOffset = GetRowEndByOffset (m_nCurrOffset);

	if (nLineBeginOffset >= 0 && nLineEndOffset > nLineBeginOffset && 
		nLineEndOffset <= m_strBuffer.GetLength ())
	{
		strLine = m_strBuffer.Mid (nLineBeginOffset, nLineEndOffset - nLineBeginOffset);
		return TRUE;
	}

	return FALSE;
}
//************************************************************************************
int CBCGPEditCtrl::GetFirstVisibleLine () const
{
	return m_nScrollOffsetVert + m_nScrollHiddenVert;
}
//************************************************************************************
int CBCGPEditCtrl::LineIndex (int nLine) const
{
	// if -1, use the current line, that is, the line that contains the caret
	return (nLine == -1) ? GetCurrRowStart () : GetRowStart (nLine);
}
//************************************************************************************
int CBCGPEditCtrl::LineFromChar (int nIndex) const
{
	return RowFromOffset (nIndex);
}
//************************************************************************************
int CBCGPEditCtrl::LineLength (int nLine) const
{
	// if -1, use the current line, that is, the line that contains the caret
	int nRowStart = (nLine < 0) ? GetCurrRowStart () : GetRowStart (nLine);

	int nRowEnd;
	return GetNumOfColumnsInRowByOffset (nRowStart, nRowEnd);
}
//************************************************************************************
void CBCGPEditCtrl::LineScroll (int nLines, int nChars/* = 0*/)
{
    BOOL bScrollRequired = FALSE;

    // -------------------
    // Scroll horizontally
    // -------------------
	int nDeltaX = nChars;
    bScrollRequired = OnScroll (SB_HORZ, SB_THUMBPOSITION, m_nScrollOffsetHorz + nDeltaX);

    // -----------------
    // Scroll vertically
    // -----------------
	int nDeltaY = nLines;
    bScrollRequired == OnScroll (SB_VERT, SB_THUMBPOSITION, m_nScrollOffsetVert + nDeltaY)
                    || bScrollRequired;

	if (bScrollRequired)
	{
		RedrawWindow ();
	}
}
//************************************************************************************
CString CBCGPEditCtrl::GetSelText () const
{
	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel);

	if (iStartSel < 0)
	{
		iStartSel = 0;
	}

	if (iEndSel > m_strBuffer.GetLength ())
	{
		iEndSel = m_strBuffer.GetLength ();
	}

	if (iStartSel >= 0 && iEndSel > iStartSel)
	{
		return m_strBuffer.Mid (iStartSel, iEndSel - iStartSel);
	}

	return (LPTSTR)NULL;
}
//************************************************************************************
void CBCGPEditCtrl::ReplaceSel (LPCTSTR lpszNewText, BOOL /*bCanUndo*/)
{
	DeleteSelectedText (FALSE, FALSE, TRUE);
	InsertText (lpszNewText, m_nCurrOffset, TRUE);
}
//************************************************************************************
void CBCGPEditCtrl::OnNcPaint() 
{
	Default ();

	CWindowDC dc (this);

	CRect rect;
	GetWindowRect (rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;
		
	if (m_bIsBorder)
	{
		OnDrawBorder (&dc, rect);
	}
}
//************************************************************************************
void CBCGPEditCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (m_bIsBorder) 
	{
		lpncsp->rgrc[0].left++; 
		lpncsp->rgrc[0].top++ ;
		lpncsp->rgrc[0].right--;
		lpncsp->rgrc[0].bottom--;
	}
}
//******************************************************************************************
LRESULT CBCGPEditCtrl::OnStyleChanging (WPARAM wp, LPARAM lp)
{
	int nStyleType = (int) wp; 
	LPSTYLESTRUCT lpStyleStruct = (LPSTYLESTRUCT) lp;

	CWnd::OnStyleChanging (nStyleType, lpStyleStruct);

	m_bIsBorder = (lpStyleStruct->styleNew & WS_BORDER);
	lpStyleStruct->styleNew &= ~WS_BORDER;

	RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);

	return 0;
}
//***************************************************************************************
void CBCGPEditCtrl::OnDrawBorder (CDC* pDC, CRect rect)
{
#ifndef _BCGPEDIT_STANDALONE
	UNUSED_ALWAYS(pDC);
	UNUSED_ALWAYS(rect);
	CBCGPVisualManager::GetInstance ()->OnDrawControlBorder (this);
#else
	pDC->Draw3dRect (rect, ::GetSysColor(COLOR_3DDKSHADOW), ::GetSysColor(COLOR_BTNHIGHLIGHT));

#endif
}
//***************************************************************************************
LRESULT CBCGPEditCtrl::OnGetTextLength (WPARAM, LPARAM)
{
	return (LRESULT) m_strBuffer.GetLength ();
}
//***************************************************************************************
BOOL CBCGPEditCtrl::OnUpdateLineNumbersMarginWidth (CDC* pDC)
{
	if (!m_bEnableLineNumbersMargin)
	{
		return FALSE;
	}

	CClientDC dc (this);

	if (pDC == NULL)
	{
		pDC = &dc;
	}

    int nRowsNum = GetLineCount ();

    // -------------------------------------
    // Calc actual line numbers margin width
    // -------------------------------------
	int nDigits = 0;
    do
	{
		++nDigits;
		nRowsNum /= 10;
	}
	while (nRowsNum);

    CFont* pOldFont = SelectFont (pDC);

	CSize size (0, 0);
	for (TCHAR c = _T('0'); c <= _T('9'); ++c)
	{
		CSize thisSize = GetStringExtent (pDC, CString(c, nDigits), nDigits);
		size.cx = max(size.cx, thisSize.cx);
		size.cy = thisSize.cy;
	}

    if (pOldFont != NULL)
    {
        pDC->SelectObject(pOldFont);
    }

    int nOldMarginWidth = m_nLineNumbersMarginAutoWidth;
    m_nLineNumbersMarginAutoWidth = max (m_nLineNumbersMarginWidth, size.cx + 3);

	// Returns TRUE, if all margin-related things should be updated
	return (nOldMarginWidth != m_nLineNumbersMarginAutoWidth);
}
//*******************************************************************************
BOOL CBCGPEditCtrl::GoToLine (int nLineNum, BOOL bExpandBlock/* = TRUE*/, BOOL bRedrawOnScroll/* = TRUE*/)
{
	ASSERT_VALID (this);

	int nOffset = GetRowStart (nLineNum - 1, FALSE);
	if (nOffset < 0)
	{
		return FALSE;
	}

	if (bExpandBlock)
	{
		ExpandLine (nLineNum);
	}

	return SetCaret (nOffset, TRUE, bRedrawOnScroll);
}
//************************************************************************************
void CBCGPEditCtrl::OnPrepareRTFString (CString& str) const
{
	// Characters, Escapes, and Character Commands
	str.Replace (_T("\\"), _T("\\\\"));
	str.Replace (_T("{"), _T("\\{"));
	str.Replace (_T("}"), _T("\\}"));

	// the line tags
	str.Replace (_T("\n"), _T("\\line\n")); // _T("\\par\n")
}
//************************************************************************************
int CBCGPEditCtrl::AddColor (COLORREF clr, CArray <COLORREF, COLORREF&> & arrClrTbl) const
{
	for (int i = 0; i < arrClrTbl.GetSize (); i++)
	{
		if (arrClrTbl [i] == clr)
		{
			return i;
		}
	}

	return (int) arrClrTbl.Add (clr);
}
//************************************************************************************
void CBCGPEditCtrl::ExportToRTF (CString& strRTF)
{
	ASSERT_VALID (this);
	
	CArray <COLORREF, COLORREF&> arrClrTbl;

	// get the body text
	CString strText;
	ExportToRTF (strText, arrClrTbl);

	// the header
	CString strHeader = _T("{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0 Courier New;}\\fs20}");

	// the color table
	CString strClrTbl = _T("{\n\\colortbl;");
	for (int i = 0; i < arrClrTbl.GetSize (); i++)
	{
		BYTE bR = GetRValue (arrClrTbl [i]);
		BYTE bG = GetGValue (arrClrTbl [i]);
		BYTE bB = GetBValue (arrClrTbl [i]);

		CString strColorTag;
		strColorTag.Format (_T("\\red%d\\green%d\\blue%d;"), bR, bG, bB);
		strClrTbl += strColorTag;
	}
	strClrTbl += _T("}\n");

	// the footer
	CString strFooter = _T("\n}");

	strRTF = strHeader + strClrTbl + 
		_T("\\pard ") + strText + 
		strFooter;
}
//************************************************************************************
void CBCGPEditCtrl::ExportToRTF (CString& strRTF, CArray <COLORREF, COLORREF&> & arrClrTbl)
{
	ASSERT_VALID (this);
	
	int iStartSel = min (m_iStartSel, m_iEndSel);
	int iEndSel = max (m_iStartSel, m_iEndSel);

	int nStartOffset = iStartSel; 
	int nEndOffset = iEndSel - 1;

	if (nStartOffset < 0)
	{
		nStartOffset = 0;
	}

	if (nEndOffset >= m_strBuffer.GetLength ())
	{
		nEndOffset = m_strBuffer.GetLength () - 1;
	}

	COLORREF clrDefaultText = GetDefaultTextColor ();
	COLORREF clrDefaultBack = GetDefaultBackColor ();

	COLORREF clrNextWordFore	= clrDefaultText;
	COLORREF clrNextWordBk		= clrDefaultBack;

	COLORREF clrOutText = clrDefaultText;
	COLORREF clrOutBack = clrDefaultBack;

	// ----------------------------------------------
	// Build color areas for visible part of a buffer (comments, strings and so on):
	// ----------------------------------------------
	BCGP_EDIT_COLOR_BLOCK	clrBlock; 
	BOOL bIsOpenBlock = FindOpenBlock (nStartOffset, &clrBlock);

	int nCloseBlockOffset = -1;
	if (bIsOpenBlock)
	{
		nCloseBlockOffset = FindCloseBlock (nStartOffset, &clrBlock);
	}

	COLORREF clrBlockFore = (clrBlock.GetForegroundColor(m_bIsDarkBackground) == -1) ? GetDefaultTextColor () : clrBlock.GetForegroundColor(m_bIsDarkBackground);
	COLORREF clrBlockBack = (clrBlock.m_clrBackground == -1) ? GetDefaultBackColor () : clrBlock.m_clrBackground; 

	CList <BCGP_EDIT_COLOR_AREA, BCGP_EDIT_COLOR_AREA&> colorAreas;

	if (bIsOpenBlock) 
	{
		if (nCloseBlockOffset < nEndOffset)
		{
			BuildColorAreas (colorAreas, nCloseBlockOffset, nEndOffset, NULL, FALSE);
		}
	}
	else
	{
		BuildColorAreas (colorAreas, nStartOffset, nEndOffset, NULL, FALSE);
	}
	
	// ---------------------------------------------
	// Draw the text for the visible part of buffer:
	// ---------------------------------------------
	TCHAR* lpszOutBuffer = (TCHAR*) alloca (sizeof (TCHAR) * m_nMaxScrollWidth);
	memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
	int iIdx = 0;

	int nNextDelimiter = -1;
	int nCurrRow = 0;
	int nCurrColumn = 0;
	
	for (int i = nStartOffset; i <= nEndOffset; i++)
	{
		COLORREF clrForeground = clrDefaultText;
		COLORREF clrBackground = clrDefaultBack;

		TCHAR chNext = m_strBuffer [i];

		// --------------
		// Define colors:
		// --------------
		BOOL bColorFound = FALSE;

		// 1) ignore selection
		// 2) ignore colored line markers

		// 3) check for color blocks:
		if (bIsOpenBlock && i < nCloseBlockOffset)
		{
			clrForeground = clrBlockFore;
			clrBackground = clrBlockBack;
			bColorFound = TRUE;
		}
		else
		{
			// check all color areas (comments, strings and so on)
			for (POSITION pos = colorAreas.GetHeadPosition (); pos != NULL;)
			{
				BCGP_EDIT_COLOR_AREA colorArea = colorAreas.GetNext (pos);
				if (i >= colorArea.m_nStart && i <= colorArea.m_nEnd)
				{
					clrForeground = colorArea.GetForegroundColor(m_bIsDarkBackground); 
					clrBackground = colorArea.m_clrBackground;

					if (clrForeground == -1)
					{
						clrForeground = clrDefaultText;
					}

					if (clrBackground == -1)
					{
						clrBackground = clrDefaultBack;
					}

					bColorFound = TRUE;
					break;
				}
			}
		}

		// 4) User can define text color by own:
		if (OnGetTextColor (i, nNextDelimiter, clrForeground, clrBackground, bColorFound))
		{
			bColorFound = TRUE;
		}

		// 5) Check all color words (keywords and words):
		if (chNext != g_chEOL)
		{
			if (nNextDelimiter == -1) 
			{
				CString strNextWord;		
				
				for (int iIdx = i; iIdx <= nEndOffset; iIdx++)
				{
					TCHAR ch = m_strBuffer [iIdx];
					if (m_strWordDelimeters.Find (ch) != -1)
					{
						nNextDelimiter = iIdx;
						break;
					}
				}

				if (nNextDelimiter == -1)
				{
					nNextDelimiter = nEndOffset + 1;
				}

				if (nNextDelimiter != -1)
				{
					strNextWord = 
						m_strBuffer.Mid (i, nNextDelimiter - i);
				}
						
				if (!OnGetWordColor (strNextWord, clrNextWordFore, clrNextWordBk, i))
				{
					clrNextWordFore = clrDefaultText;
					clrNextWordBk = clrDefaultBack;
				}
			}

			if (i >= nNextDelimiter - 1)
			{
				nNextDelimiter = -1;
			}

			if (!bColorFound)
			{
				clrForeground = clrNextWordFore;
				clrBackground = clrNextWordBk;
			}
		}

		// 6) ignore hilited text

		// 7) User can define color of current char:
		if (!bColorFound)
		{
			OnGetCharColor (chNext, i, clrForeground, clrBackground);
		}

		// -----------------------------------
		// Proceed end of same color fragment:
		// -----------------------------------
		
		if (clrForeground != clrOutText || clrBackground != clrOutBack)
		{
			clrOutText = clrForeground;
			clrOutBack = clrBackground;

			if (iIdx != 0)
			{
				CString strLine (lpszOutBuffer);
				OnPrepareRTFString (strLine);
				strRTF += strLine;

				iIdx = 0;
				memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
			}

			// the color tag
			if (GetDefaultBackColor () != (COLORREF)::GetSysColor (COLOR_WINDOW))
			{
				int nColorBk = AddColor (clrOutBack, arrClrTbl);
				CString strColorBkTag;
				strColorBkTag.Format (_T("\\highlight%d"), nColorBk + 1);
				strRTF += strColorBkTag;
			}
			int nColor = AddColor (clrOutText, arrClrTbl);
			CString strColorTag;
			strColorTag.Format (_T("\\cf%d "), nColor + 1);
			strRTF += strColorTag;

		}
		
		// --------------------
		// Proceed end of line:
		// --------------------
		if (chNext == g_chEOL)
		{
			{
				CString strLine (lpszOutBuffer);
				OnPrepareRTFString (strLine);
				strLine += _T("\\line"); // _T("\\par")
				strLine += g_chEOL;
				strRTF += strLine;

				iIdx = 0;
				memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
			}

			nCurrRow++;
			nCurrColumn = 0;
			continue;
		}

		// -------------
		// Replace Tabs:
		// -------------
		if (chNext == _T('\t'))
		{
			int nRestOfTab = m_nTabSize - nCurrColumn % m_nTabSize;
			nCurrColumn += nRestOfTab;

			for (int k = 0; k < nRestOfTab; k++)
			{
				if (iIdx + k + 1 > m_nMaxScrollWidth - 1)
				{
					CString strLine (lpszOutBuffer);
					OnPrepareRTFString (strLine);
					strRTF += strLine;

					iIdx = 0;
					memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
				}

				lpszOutBuffer [iIdx] = _T(' ');
				lpszOutBuffer [iIdx + 1] = _T ('\0');
				iIdx ++;
			}
		}

		else
		{
			nCurrColumn++;

			if (iIdx + 1 > m_nMaxScrollWidth - 1)
			{
				CString strLine (lpszOutBuffer);
				OnPrepareRTFString (strLine);
				strRTF += strLine;

				iIdx = 0;
				memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
			}

			lpszOutBuffer [iIdx] = chNext;
			lpszOutBuffer [iIdx + 1] = _T ('\0');
			iIdx++;
		}
	}

	// --------------------------
	// Draw the last of the text:
	// --------------------------
	if (iIdx != 0)
	{
		CString strLine (lpszOutBuffer);
		OnPrepareRTFString (strLine);
		strRTF += strLine;

		iIdx = 0;
		memset (lpszOutBuffer, 0, sizeof (TCHAR) * m_nMaxScrollWidth);
	}
}
//*************************************************************************************
void CBCGPEditCtrl::OnDestroy() 
{
#ifndef _BCGPEDIT_STANDALONE
	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);
#else
	if (m_pToolTip != NULL)
	{
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->DestroyWindow ();
		}
		
		delete m_pToolTip;
	}
#endif
	CWnd::OnDestroy();
}
//**************************************************************************************
LRESULT CBCGPEditCtrl::OnBCGUpdateToolTips (WPARAM wp, LPARAM)
{
#ifndef _BCGPEDIT_STANDALONE

	UINT nTypes = (UINT) wp;

	if (nTypes & BCGP_TOOLTIP_TYPE_DEFAULT)
	{
		CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
			BCGP_TOOLTIP_TYPE_EDIT);

		if (m_pToolTip != NULL)
		{
			CRect rectClient;
			GetClientRect (&rectClient);

			if (m_nMaxToolTipWidth != -1)
			{
				m_pToolTip->SetMaxTipWidth (m_nMaxToolTipWidth);
			}
			
			m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectClient, GetDlgCtrlID ());
		}
	}
#else
	UNUSED_ALWAYS(wp);
#endif
	return 0;
}

#endif	// BCGP_EXCLUDE_EDIT_CTRL

