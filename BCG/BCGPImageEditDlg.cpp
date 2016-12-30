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

// BCGPImageEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include <afxpriv.h>
#include "bcgprores.h"
#include "BCGGlobals.h"
#include "BCGPImageEditDlg.h"
#include "BCGPLocalResource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void Create16ColorsStdPalette (CPalette& pal);

/////////////////////////////////////////////////////////////////////////////
// CBCGPImageEditDlg dialog

#pragma warning (disable : 4355)

CBCGPImageEditDlg::CBCGPImageEditDlg(CBitmap* pBitmap, CWnd* pParent /*=NULL*/,
									 int nBitsPixel /* = -1 */) :
	CBCGPDialog(CBCGPImageEditDlg::IDD, pParent),
	m_pBitmap (pBitmap),
	m_wndLargeDrawArea (this)
{
	ASSERT_VALID (m_pBitmap);

	BITMAP bmp;
	m_pBitmap->GetBitmap (&bmp);

	m_sizeImage = CSize (bmp.bmWidth, bmp.bmHeight);

	m_nBitsPixel = (nBitsPixel == -1) ? bmp.bmBitsPixel : nBitsPixel;
	ASSERT (m_nBitsPixel >= 4);	// Monochrome bitmaps are not supported

	m_bIsLocal = TRUE;

	//{{AFX_DATA_INIT(CBCGPImageEditDlg)
	//}}AFX_DATA_INIT

	EnableVisualManagerStyle (globalData.m_bUseVisualManagerInBuiltInDialogs, TRUE);
}

#pragma warning (default : 4355)

void CBCGPImageEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPImageEditDlg)
	DDX_Control(pDX, IDC_BCGBARRES_COLORS, m_wndColorPickerLocation);
	DDX_Control(pDX, IDC_BCGBARRES_PALETTE, m_wndPaletteBarLocation);
	DDX_Control(pDX, IDC_BCGBARRES_PREVIEW_AREA, m_wndPreview);
	DDX_Control(pDX, IDC_BCGBARRES_DRAW_AREA, m_wndLargeDrawArea);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPImageEditDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPImageEditDlg)
	ON_WM_PAINT()
	ON_COMMAND(ID_BCG_TOOL_CLEAR, OnBcgToolClear)
	ON_COMMAND(ID_BCG_TOOL_COPY, OnBcgToolCopy)
	ON_COMMAND(ID_BCG_TOOL_PASTE, OnBcgToolPaste)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_PASTE, OnUpdateBcgToolPaste)
	ON_COMMAND(ID_BCG_TOOL_ELLIPSE, OnBcgToolEllipse)
	ON_COMMAND(ID_BCG_TOOL_FILL, OnBcgToolFill)
	ON_COMMAND(ID_BCG_TOOL_LINE, OnBcgToolLine)
	ON_COMMAND(ID_BCG_TOOL_PEN, OnBcgToolPen)
	ON_COMMAND(ID_BCG_TOOL_PICK, OnBcgToolPick)
	ON_COMMAND(ID_BCG_TOOL_RECT, OnBcgToolRect)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_ELLIPSE, OnUpdateBcgToolEllipse)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_FILL, OnUpdateBcgToolFill)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_LINE, OnUpdateBcgToolLine)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_PEN, OnUpdateBcgToolPen)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_PICK, OnUpdateBcgToolPick)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_RECT, OnUpdateBcgToolRect)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_COMMAND(IDC_BCGBARRES_COLORS, OnColors)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPImageEditDlg message handlers

BOOL CBCGPImageEditDlg::OnInitDialog() 
{
	const int iBorderWidth = 10;
	const int iBorderHeight = 5;
	const int iPreviewBorderSize = 4;

	CBCGPDialog::OnInitDialog();

	if (AfxGetMainWnd () != NULL && (AfxGetMainWnd ()->GetExStyle () & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx (0, WS_EX_LAYOUTRTL);
	}

	int xPaletteLeft = -1;

	//------------------------
	// Create the palette bar:
	//------------------------	
	{
		CBCGPLocalResource locaRes;

		m_wndPaletteBarLocation.ShowWindow(SW_HIDE);

		CRect rectPaletteBarWnd;
		m_wndPaletteBarLocation.GetWindowRect (&rectPaletteBarWnd);

		CRect rectPaletteBar;

		m_wndPaletteBarLocation.GetClientRect (&rectPaletteBar);
		m_wndPaletteBarLocation.MapWindowPoints (this, &rectPaletteBar);
		rectPaletteBar.DeflateRect(1, 1);

		m_wndPaletteBar.EnableLargeIcons (FALSE);
		m_wndPaletteBar.Create (this);
		m_wndPaletteBar.SetControlVisualMode (this);
		m_wndPaletteBar.ModifyStyle(0, TBSTYLE_TRANSPARENT);

		const UINT uiToolbarHotID = globalData.Is32BitIcons () ? IDR_BCGRES_PALETTE32 : 0;

		m_wndPaletteBar.LoadToolBar (	IDR_BCGRES_PALETTE, 0, 0, 
										TRUE /* Locked bar */, 0, 0, uiToolbarHotID);

		m_wndPaletteBar.SetBarStyle(m_wndPaletteBar.GetBarStyle() |
			CBRS_TOOLTIPS | CBRS_FLYBY);
			
		m_wndPaletteBar.SetBarStyle (
			m_wndPaletteBar.GetBarStyle () & 
				~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

		m_wndPaletteBar.SetBorders (iBorderWidth, iBorderHeight, 
									iBorderWidth, iBorderHeight, TRUE /* Transparent */);

		const int nButtonWidth = m_wndPaletteBar.GetButtonSize ().cx;
		m_wndPaletteBar.WrapToolBar (nButtonWidth * 3);
		
		const CSize szLayout = m_wndPaletteBar.CalcSize (FALSE);
		rectPaletteBar.bottom = rectPaletteBar.top + szLayout.cy + iBorderHeight * 2;
		
		m_wndPaletteBar.MoveWindow (rectPaletteBar);

		m_wndPaletteBar.SetWindowPos (&wndTop, -1, -1, -1, -1,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		if (rectPaletteBar.Height () > rectPaletteBarWnd.Height ())
		{
			m_wndPaletteBarLocation.SetWindowPos (NULL, -1, -1, 
				rectPaletteBarWnd.Width (), rectPaletteBar.Height () + iBorderHeight + 2,
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}

		m_wndPaletteBar.SetOwner (this);

		// All commands will be routed via this dialog, not via the parent frame:
		m_wndPaletteBar.SetRouteCommandsViaFrame (FALSE);

		// Adjust palette border in High DPI mode:
		if (globalData.GetRibbonImageScale ())
		{
			CRect rectPalette;
			m_wndPaletteBarLocation.GetWindowRect (rectPalette);

			m_wndPaletteBarLocation.SetWindowPos (NULL, -1, -1,
				rectPalette.Width (), szLayout.cy + 4 * iBorderHeight,
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
		}

		xPaletteLeft = rectPaletteBar.left;
	}

	// Create color picker:
	{
		m_wndColorPickerLocation.ShowWindow(SW_HIDE);
		CRect rectColorBar;
		m_wndColorPickerLocation.GetClientRect (&rectColorBar);

		m_wndColorPickerLocation.MapWindowPoints (this, &rectColorBar);
		rectColorBar.DeflateRect(1, 1);

		m_wndColorBar.m_bInternal = TRUE;

		int nColumns = 4;

		// If bitmap has 256 or less colors, create 16 colors palette:
		CPalette pal;
		if (m_nBitsPixel <= 8)
		{
			Create16ColorsStdPalette (pal);
		}
		else
		{
			m_wndColorBar.EnableOtherButton (_T("Other"));

			nColumns = 5;
			m_wndColorBar.SetVertMargin (1);
			m_wndColorBar.SetHorzMargin (1);
		}
		
		m_wndColorBar.CreateControl (this, rectColorBar, IDC_BCGBARRES_COLORS,
			nColumns, m_nBitsPixel <= 8 ? &pal : NULL);

		m_wndColorBar.SetControlVisualMode (this);
		m_wndColorBar.SetColor (RGB (0, 0, 0));

		xPaletteLeft = min(xPaletteLeft, rectColorBar.left);
	}

	//----------------------------
	// Adjust image area location:
	//----------------------------
	CRect rectImage;
	m_wndLargeDrawArea.GetWindowRect(rectImage);
	ScreenToClient(rectImage);
	
	if (rectImage.Width() < rectImage.Height() && rectImage.left + rectImage.Height() + iBorderWidth < xPaletteLeft)
	{
		m_wndLargeDrawArea.SetWindowPos(NULL, -1, -1, rectImage.Height(), rectImage.Height(),
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	
	m_wndLargeDrawArea.SetBitmap (m_pBitmap);
	
	//---------------------
	// Define preview area:
	//---------------------
	m_wndPreview.GetClientRect (&m_rectPreviewImage);
	m_wndPreview.MapWindowPoints (this, &m_rectPreviewImage);

	m_rectPreviewImage.left = (m_rectPreviewImage.left + m_rectPreviewImage.right - m_sizeImage.cx) / 2;
	m_rectPreviewImage.right = m_rectPreviewImage.left + m_sizeImage.cx;

	m_rectPreviewImage.top = (m_rectPreviewImage.top + m_rectPreviewImage.bottom - m_sizeImage.cy) / 2;
	m_rectPreviewImage.bottom = m_rectPreviewImage.top + m_sizeImage.cy;

	m_rectPreviewFrame = m_rectPreviewImage;
	m_rectPreviewFrame.InflateRect (iPreviewBorderSize, iPreviewBorderSize);

	m_wndLargeDrawArea.m_rectParentPreviewArea = m_rectPreviewImage;
	m_wndLargeDrawArea.ModifyStyle (WS_TABSTOP, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//********************************************************************************
void CBCGPImageEditDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (IsVisualManagerStyle ())
	{
		dc.FillRect (m_rectPreviewFrame, &globalData.brBarFace);
	}
	else
	{
		dc.FillRect (m_rectPreviewFrame, &globalData.brBtnFace);
	}

	CBitmap* pbmOld = NULL;
	CDC dcMem;
		
	dcMem.CreateCompatibleDC (&dc);
	pbmOld = dcMem.SelectObject (m_pBitmap);

	CBCGPToolBarImages::TransparentBlt(dc.GetSafeHdc(), m_rectPreviewImage.left, m_rectPreviewImage.top,
				m_sizeImage.cx, m_sizeImage.cy, &dcMem,
				0, 0, RGB(192, 192, 192));

	if (IsVisualManagerStyle ())
	{
		dc.Draw3dRect (&m_rectPreviewFrame,
						globalData.clrBarHilite,
						globalData.clrBarShadow);
	}
	else
	{
		dc.Draw3dRect (&m_rectPreviewFrame,
						globalData.clrBtnHilite,
						globalData.clrBtnShadow);
	}

	dcMem.SelectObject(pbmOld);
	dcMem.DeleteDC();

	COLORREF clrFrame = IsVisualManagerStyle() ? CBCGPVisualManager::GetInstance()->GetSeparatorColor() : globalData.clrBtnShadow;

	CRect rectPalFrame;
	m_wndPaletteBar.GetWindowRect(&rectPalFrame);
	ScreenToClient(rectPalFrame);

	rectPalFrame.InflateRect(1, 1);

	dc.Draw3dRect(rectPalFrame, clrFrame, clrFrame);

	CRect rectColorFrame;
	m_wndColorBar.GetWindowRect(&rectColorFrame);
	ScreenToClient(rectColorFrame);
	
	rectColorFrame.InflateRect(1, 1);
	
	dc.Draw3dRect(rectColorFrame, clrFrame, clrFrame);
}
//****************************************************************************************
LRESULT CBCGPImageEditDlg::OnKickIdle(WPARAM, LPARAM)
{
	m_wndPaletteBar.OnUpdateCmdUI ((CFrameWnd*) this, TRUE);
    return 0;
}
//********************************************************************************
void CBCGPImageEditDlg::OnColors() 
{
	m_wndLargeDrawArea.SetColor(m_wndColorBar.GetColor ());
}
//********************************************************************************
BOOL CBCGPImageEditDlg::OnPickColor (COLORREF color)
{
	m_wndColorBar.SetColor (color);
	m_wndLargeDrawArea.SetColor (color);

	//-----------------------------------------
	// Move to the pen mode (not so good :-(!):
	//-----------------------------------------
	m_wndLargeDrawArea.SetMode (CImagePaintArea::IMAGE_EDIT_MODE_PEN);
	return TRUE;
}
//********************************************************************************
void CBCGPImageEditDlg::OnBcgToolClear() 
{
	CWindowDC	dc (this);
	CDC 		memDC;	

	memDC.CreateCompatibleDC (&dc);
	
	CBitmap* pOldBitmap = memDC.SelectObject (m_pBitmap);

	CRect rect (0, 0, m_sizeImage.cx, m_sizeImage.cy);
	memDC.FillSolidRect (&rect, globalData.clrBtnFace);

	memDC.SelectObject (pOldBitmap);

	InvalidateRect (m_rectPreviewImage);
	m_wndLargeDrawArea.Invalidate ();
}
//********************************************************************************
void CBCGPImageEditDlg::OnBcgToolCopy() 
{
	CBCGPLocalResource locaRes;

	if (m_pBitmap == NULL)
	{
		return;
	}

	try
	{
		CWindowDC dc (this);

		//----------------------
		// Create a bitmap copy:
		//----------------------
		CDC memDCDest;
		memDCDest.CreateCompatibleDC (NULL);
		
		CDC memDCSrc;
		memDCSrc.CreateCompatibleDC (NULL);
		
		CBitmap bitmapCopy;
		if (!bitmapCopy.CreateCompatibleBitmap (&dc, m_sizeImage.cx, m_sizeImage.cy))
		{
			AfxMessageBox (IDP_BCGBARRES_CANT_COPY_BITMAP);
			return;
		}

		CBitmap* pOldBitmapDest = memDCDest.SelectObject (&bitmapCopy);
		CBitmap* pOldBitmapSrc = memDCSrc.SelectObject (m_pBitmap);

		memDCDest.BitBlt (0, 0, m_sizeImage.cx, m_sizeImage.cy,
						&memDCSrc, 0, 0, SRCCOPY);

		memDCDest.SelectObject (pOldBitmapDest);
		memDCSrc.SelectObject (pOldBitmapSrc);

		if (!OpenClipboard ())
		{
			AfxMessageBox (IDP_BCGBARRES_CANT_COPY_BITMAP);
			return;
		}

		if (!::EmptyClipboard ())
		{
			AfxMessageBox (IDP_BCGBARRES_CANT_COPY_BITMAP);
			::CloseClipboard ();
			return;
		}


		HANDLE hclipData = ::SetClipboardData (CF_BITMAP, bitmapCopy.Detach ());
		if (hclipData == NULL)
		{
			AfxMessageBox (IDP_BCGBARRES_CANT_COPY_BITMAP);
			TRACE (_T ("CBCGPImageEditDlg::Copy() error. Error code = %x\n"), GetLastError ());
		}

		::CloseClipboard ();
	}
	catch (...)
	{
		CBCGPLocalResource locaRes;
		AfxMessageBox (IDP_BCGBARRES_INTERLAL_ERROR);
	}
}
//********************************************************************************
void CBCGPImageEditDlg::OnBcgToolPaste() 
{
	CBCGPLocalResource locaRes;

	COleDataObject data;
	if (!data.AttachClipboard ())
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		return;
	}

	if (!data.IsDataAvailable (CF_BITMAP))
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		return;
	}

	tagSTGMEDIUM dataMedium;
	if (!data.GetData (CF_BITMAP, &dataMedium))
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		return;
	}

	CBitmap* pBmpClip = CBitmap::FromHandle (dataMedium.hBitmap);
	if (pBmpClip == NULL)
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		return;
	}

	BITMAP bmp;
	pBmpClip->GetBitmap (&bmp);

	CDC memDCDst;
	CDC memDCSrc;

	memDCSrc.CreateCompatibleDC (NULL);
	memDCDst.CreateCompatibleDC (NULL);
	
	CBitmap* pSrcOldBitmap = memDCSrc.SelectObject (pBmpClip);
	if (pSrcOldBitmap == NULL)
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		return;
	}

	CBitmap* pDstOldBitmap = memDCDst.SelectObject (m_pBitmap);
	if (pDstOldBitmap == NULL)
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		
		memDCSrc.SelectObject (pSrcOldBitmap);
		return;
	}

	memDCDst.FillRect (CRect (0, 0, m_sizeImage.cx, m_sizeImage.cy), 
						&globalData.brBtnFace);

	int x = max (0, (m_sizeImage.cx - bmp.bmWidth) / 2);
	int y = max (0, (m_sizeImage.cy - bmp.bmHeight) / 2);

	CBCGPToolBarImages::TransparentBlt (memDCDst.GetSafeHdc (),
		x, y, m_sizeImage.cx, m_sizeImage.cy,
		&memDCSrc, 0, 0, RGB (192, 192, 192));

	memDCDst.SelectObject (pDstOldBitmap);
	memDCSrc.SelectObject (pSrcOldBitmap);

	InvalidateRect (m_rectPreviewImage);
	m_wndLargeDrawArea.Invalidate ();
}
//********************************************************************************
void CBCGPImageEditDlg::OnUpdateBcgToolPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(::IsClipboardFormatAvailable (CF_BITMAP));
}
//********************************************************************************
void CBCGPImageEditDlg::OnBcgToolEllipse() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_ELLIPSE);
}
//********************************************************************************
void CBCGPImageEditDlg::OnBcgToolFill() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_FILL);
}
//********************************************************************************
void CBCGPImageEditDlg::OnBcgToolLine() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_LINE);
}
//********************************************************************************
void CBCGPImageEditDlg::OnBcgToolPen() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_PEN);
}
//********************************************************************************
void CBCGPImageEditDlg::OnBcgToolPick() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_COLOR);
}
//********************************************************************************
void CBCGPImageEditDlg::OnBcgToolRect() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_RECT);
}
//********************************************************************************
void CBCGPImageEditDlg::OnUpdateBcgToolEllipse(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_ELLIPSE);
}
//********************************************************************************
void CBCGPImageEditDlg::OnUpdateBcgToolFill(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_FILL);
}
//********************************************************************************
void CBCGPImageEditDlg::OnUpdateBcgToolLine(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_LINE);
}
//********************************************************************************
void CBCGPImageEditDlg::OnUpdateBcgToolPen(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_PEN);
}
//********************************************************************************
void CBCGPImageEditDlg::OnUpdateBcgToolPick(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_COLOR);
}
//********************************************************************************
void CBCGPImageEditDlg::OnUpdateBcgToolRect(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_RECT);
}
//********************************************************************************
void Create16ColorsStdPalette (CPalette& pal)
{
	const int nStdColorCount = 20;
	CPalette* pPalDefault = CPalette::FromHandle ((HPALETTE) ::GetStockObject (DEFAULT_PALETTE));
	if (pPalDefault == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	const int nColors = 16;
	UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
	LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

	pLP->palVersion = 0x300;
	pLP->palNumEntries = (USHORT) nColors;

	pal.CreatePalette (pLP);

	delete[] pLP;

	PALETTEENTRY palEntry;
	int iDest = 0;

	for (int i = 0; i < nStdColorCount; i++)
	{
		if (i < 8 || i >= 12)
		{
			pPalDefault->GetPaletteEntries (i, 1, &palEntry);
			pal.SetPaletteEntries (iDest++, 1, &palEntry);
		}
	}
}

