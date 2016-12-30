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
// PowerColorPicker.cpp : implementation file

#include "stdafx.h"

#include "BCGCBPro.h"
#include "BCGPMath.h"
#include "BCGGlobals.h"
#include "BCGPPowerColorPicker.h"
#include "BCGPDrawManager.h"
#include "bcgprores.h"
#include "ColorPage1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_WIDTH_OF_LUMINANCE_BAR	20
#define DEFAULT_OFFSET_OF_LUMINANCE_BAR	5
#define DEFAULT_LUMINANCE				0.50f
#define PICKER_CURSOR_SIZE				19
#define PICKER_CURSOR_SIZEH				9
#define LUM_CURSOR_SIZE					9

// Hex
#define NUM_LEVELS			7
#define CELL_EDGES			6
#define GRAY_CELLS_NUM		15	// + 2 (Black and white)
#define TAN30				0.57735026918962F
#define YOFFSET				(1.5F * TAN30)

static const float cfxOffset[] = { -0.5, -1.0, -0.5, 0.5, 1.0, 0.5 };
static const float cfyOffset[] = { YOFFSET, 0.0, -YOFFSET, -YOFFSET, 0.0, YOFFSET };

static const COLORREF colorWhite = RGB (255, 255, 255);
static const COLORREF colorBlack = RGB (0, 0, 0);

static long AlignColor (long lPart, const long lDelta)
{
	if (lDelta == 0)
	{
		return lPart;
	}

	if (lPart < lDelta)
	{
		return 0;
	}

	if (lPart > 255 - lDelta)
	{
		return 255;
	}

	if (abs (lPart - 128) < lDelta)
	{
		return 128;
	}

	if (abs (lPart - 192) < lDelta)
	{
		return 192;
	}

	return lPart;
}

class CBCGPCellObj : public CObject
{
	friend class CBCGPColorPickerCtrl;

	CBCGPCellObj (	CPalette* pPalette,
				const COLORREF color, 
				const int x, const int y, 
				const int nCellWidth,
				const long lDelta)
	{
		m_x = x;
		m_y = y;

		m_nCellWidth = nCellWidth;

		//-----------------------------------------------
		// Approximate color to one of "standard" colors:
		//-----------------------------------------------
		long lRed = AlignColor (GetRValue (color), lDelta);
		long lGreen = AlignColor (GetGValue (color), lDelta);
		long lBlue = AlignColor (GetBValue (color), lDelta);

		m_CellColor = RGB (lRed, lGreen, lBlue);

		if (globalData.m_nBitsPerPixel == 8) // 256 colors
		{
			ASSERT_VALID (pPalette);

			UINT uiPalIndex = pPalette->GetNearestPaletteIndex (color);
			m_CellDrawColor = PALETTEINDEX (uiPalIndex);
		}
		else
		{
			m_CellDrawColor = m_CellColor;
		}

		GetPoints (m_x, m_y, nCellWidth, m_CellPoints);
	}

	BOOL HitTest (POINT pt)
	{
		CRgn rgn;
		rgn.CreatePolygonRgn(m_CellPoints, CELL_EDGES, ALTERNATE);
		
		return rgn.PtInRegion (pt);
	}

	void GetPoints (int x, int y, int nCellWidth, POINT* pptArray)
	{
		// side length = half the height * sin(60)
		int nHalfWidth = nCellWidth / 2;
		int nSideLength = static_cast<int>(static_cast<float>(nCellWidth) * TAN30);
		int nTemp = nSideLength/2;
		
		pptArray[0].x = x - nHalfWidth;
		pptArray[0].y = y - nTemp;
		
		pptArray[1].x = x;
		pptArray[1].y = y - nHalfWidth;
		
		pptArray[2].x = x + nHalfWidth;
		pptArray[2].y = y - nTemp;
		
		pptArray[3].x = x + nHalfWidth;
		pptArray[3].y = y + nTemp;
		
		pptArray[4].x = x;
		pptArray[4].y = y + nHalfWidth;
		
		pptArray[5].x = x - nHalfWidth;
		pptArray[5].y = y + nTemp;
	}

	void Draw (CDC* pDC)
	{
		ASSERT_VALID (pDC);

		CBrush br (m_CellDrawColor);
		CPen pen (PS_SOLID, 1, m_CellDrawColor);
		
		CBrush* pOldBrush = pDC->SelectObject(&br);
		CPen* pOldPen = pDC->SelectObject(&pen);
		
		pDC->Polygon (m_CellPoints, CELL_EDGES);
		
		pDC->SelectObject (pOldPen);
		pDC->SelectObject (pOldBrush);
	}

	void DrawSelected (CDC* pDC, BOOL bIsFocused)
	{
		ASSERT_VALID (pDC);

		CBrush* pBrWhite = CBrush::FromHandle ((HBRUSH) ::GetStockObject (bIsFocused ? WHITE_BRUSH : GRAY_BRUSH));
		ASSERT_VALID (pBrWhite);
		
		CBrush* pBrBlack = CBrush::FromHandle ((HBRUSH) ::GetStockObject (BLACK_BRUSH));
		ASSERT_VALID (pBrBlack);

		CRgn rgnOne, rgnTwo, rgnThree;
		
		POINT ptArrayTwo[CELL_EDGES];
		GetPoints(m_x, m_y - 1, m_nCellWidth + 2, ptArrayTwo);
		
		rgnTwo.CreatePolygonRgn((POINT*)&ptArrayTwo, CELL_EDGES, ALTERNATE);
		pDC->FrameRgn(&rgnTwo, pBrWhite, 2, 2);
		
		POINT ptArrayThree[CELL_EDGES];
		GetPoints(m_x, m_y, m_nCellWidth + 2, ptArrayThree);
		
		rgnThree.CreatePolygonRgn((POINT*)&ptArrayThree, CELL_EDGES, ALTERNATE);
		pDC->FrameRgn(&rgnThree, pBrBlack, 1, 1);
		
		POINT ptArrayOne[CELL_EDGES];
		GetPoints(m_x, m_y, m_nCellWidth - 1, ptArrayOne);
		
		rgnOne.CreatePolygonRgn((POINT*)&ptArrayOne, CELL_EDGES, ALTERNATE);
		pDC->FrameRgn(&rgnOne, pBrBlack, 1, 1);
	}

	POINT		m_CellPoints [CELL_EDGES];
	COLORREF	m_CellColor;
	COLORREF	m_CellDrawColor;
	int			m_x;
	int			m_y;
	int			m_nCellWidth;
};


//----------------------------------------------------------------------
// CBCGPColorPickerCtrl class
//----------------------------------------------------------------------

IMPLEMENT_DYNAMIC(CBCGPColorPickerCtrl, CButton)

CBCGPColorPickerCtrl::CBCGPColorPickerCtrl()
{
	m_colorNew		= 0;
	m_colorOriginal	= 0;

	CBCGPDrawManager::RGBtoHSL (m_colorNew, &m_dblHue, &m_dblSat, &m_dblLum);

	m_nLumBarWidth  = DEFAULT_WIDTH_OF_LUMINANCE_BAR;
	m_COLORTYPE		= PICKER;
	
	m_dblLum = 0.500;
	m_pPalette = NULL;
}

CBCGPColorPickerCtrl::~CBCGPColorPickerCtrl()
{
	for (int i = 0; i < m_arCells.GetSize (); i ++)
	{
		delete m_arCells [i];
	}
}


BEGIN_MESSAGE_MAP(CBCGPColorPickerCtrl, CButton)
//{{AFX_MSG_MAP(CBCGPColorPickerCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//----------------------------------------------------------------------
// CBCGPColorPickerCtrl message handlers
//----------------------------------------------------------------------

int CBCGPColorPickerCtrl::GetAngleFromPoint(int nX, int nY)
{
	return (int)bcg_rad2deg(bcg_angle((double)nX, (double)nY));
}
//******************************************************************************
void CBCGPColorPickerCtrl::CreateHexGreyScaleBar ()
{
	if (m_arCells.GetSize () != 0)
	{
		// Already created
		return;
	}

	CRect area;
	GetClientRect (area);

	int nCellSize = min (area.Height () / 2 - 2, 
								area.Width () / (GRAY_CELLS_NUM  / 2 + 6));
	if ((nCellSize % 2) != 0)
	{
		nCellSize ++;
	}

	const int nCellLargeSize = nCellSize * 2;

	int yCenter = (area.top + area.bottom) / 2;
	int nSideLength = static_cast<int>(static_cast<float>(nCellSize) * TAN30 * 1.5);
	
	int y1 = yCenter - nSideLength / 2;
	int y2 = y1 + nSideLength;
		
	int nRGBOffset = 255 / (GRAY_CELLS_NUM + 2);
	
	int nStartOffset = area.left;
	
	for (int nRowNum = 0; nRowNum < 2; nRowNum++)
	{
		if (nRowNum == 1)
		{
			// Draw large white cell:
			int x1 = nStartOffset + (nCellLargeSize / 2);
			m_arCells.Add (new CBCGPCellObj (
				m_pPalette, colorWhite, x1, yCenter, nCellLargeSize, 0));
		}
		
		int x = nCellLargeSize + nCellSize + nStartOffset;
		int nCurry = y1;
		int nRGB = 255 - nRGBOffset;
		
		for (int i = 0; i < GRAY_CELLS_NUM; i++)
		{
			COLORREF color = RGB(nRGB, nRGB, nRGB);
			if (nRowNum == 1)
			{
				m_arCells.Add (new CBCGPCellObj (m_pPalette, color, x, nCurry, nCellSize, 7));
			}
			
			x += (nCellSize / 2);
			
			nCurry = (nCurry == y1) ? y2 : y1;	// Toggle Y
			nRGB -= nRGBOffset;
		}
		
		if (nRowNum == 1)
		{
			// Draw large black cell:
			int x1 = (x + nCellSize + (nCellSize / 2)) - 1;

			m_arCells.Add (new CBCGPCellObj (
				m_pPalette, colorBlack, x1, yCenter, nCellLargeSize, 0));
		}
	
		x += nCellLargeSize + (nCellSize / 2);

		if (nRowNum == 0)
		{
			nStartOffset = (area.right - x) / 2;
		}
	}
}
//******************************************************************************
void CBCGPColorPickerCtrl::SelectCellHexagon(BYTE R, BYTE G, BYTE B)
{
	SetColor (RGB (R, G, B));
}
//******************************************************************************
void CBCGPColorPickerCtrl::SetColor(COLORREF Color)
{	
	m_colorNew = Color;
	CBCGPDrawManager::RGBtoHSL (m_colorNew, &m_dblHue, &m_dblSat, &m_dblLum);

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
};
//******************************************************************************
BOOL CBCGPColorPickerCtrl::SelectCellHexagon(int x, int y)
{
	for (int i = 0; i < m_arCells.GetSize (); i ++)
	{
		CBCGPCellObj* pCell = (CBCGPCellObj*) m_arCells [i];
		ASSERT_VALID (pCell);

		if (pCell->HitTest (CPoint (x, y)))
		{
			m_colorNew = pCell->m_CellColor;
			CBCGPDrawManager::RGBtoHSL (m_colorNew, &m_dblHue, &m_dblSat, &m_dblLum);
			return TRUE;
		}
	}

	return FALSE;
}
//******************************************************************************
void CBCGPColorPickerCtrl::CreateHexagon ()
{
	if (m_arCells.GetSize () != 0)
	{
		// Already created, do nothing
		return;
	}

	CRect rectClient;
	GetClientRect (rectClient);

	// Normalize to squere:
	if (rectClient.Height () < rectClient.Width ())
	{
		rectClient.DeflateRect ((rectClient.Width () - rectClient.Height ()) / 2, 0);
	}
	else
	{
		rectClient.DeflateRect (0, (rectClient.Height () - rectClient.Width ()) / 2);
	}

	ASSERT (abs (rectClient.Height () - rectClient.Width ()) <= 1);

	int nCellSize = rectClient.Height () / (2 * NUM_LEVELS - 1) + 1;

	int x = (rectClient.left + rectClient.right) / 2;
	int y = (rectClient.top + rectClient.bottom) / 2;

	// Add center cell
	m_arCells.Add (new CBCGPCellObj (m_pPalette, colorWhite, x, y, nCellSize, 0));

	// for each level
	for (int nLevel = 1; nLevel < NUM_LEVELS; nLevel++)
	{
		// store the level start position
		int nPosX = x + (nCellSize * nLevel);
		int nPosY = y;
		
		// for each side
		for (int nSide = 0; nSide < NUM_LEVELS - 1; nSide++)
		{
			// set the deltas for the side
			int nDx = static_cast<int>(static_cast<float>(nCellSize) * cfxOffset[nSide]);
			int nDy = static_cast<int>(static_cast<float>(nCellSize) * cfyOffset[nSide]);
			
			// for each cell per side
			for (int nCell = 0; nCell < nLevel; nCell++)
			{
				int nAngle = GetAngleFromPoint(nPosX - x, nPosY - y);
				
				// TODO: Set the luminance and saturation properly
				double L = 1. * (NUM_LEVELS - nLevel) / NUM_LEVELS + .1;

				m_arCells.Add (new CBCGPCellObj (m_pPalette, 
					CBCGPDrawManager::HLStoRGB_TWO ((float) nAngle, L, 1.0F), 
					nPosX, nPosY, nCellSize, 16));
				
				// offset the position
				nPosX += nDx;
				nPosY += nDy;
			}
		}
	}
}
//******************************************************************************
void CBCGPColorPickerCtrl::DrawHex (CDC* pDC)
{
	ASSERT_VALID (pDC);

	globalData.DrawParentBackground (this, pDC);

	CBCGPCellObj* pSelCell = NULL;

	for (int i = 0; i < m_arCells.GetSize (); i ++)
	{
		CBCGPCellObj* pCell = (CBCGPCellObj*) m_arCells [i];
		ASSERT_VALID (pCell);

		pCell->Draw (pDC);

		if (pCell->m_CellColor == m_colorNew)
		{
			pSelCell = pCell;
		}
	}

	if (pSelCell != NULL)
	{
		pSelCell->DrawSelected (pDC, (GetFocus() == this));
	}
}
//******************************************************************************
void CBCGPColorPickerCtrl::DrawPicker (CDC* pDC)
{
	CRect rectClient;
	GetClientRect (rectClient);

	CSize szColorPicker = rectClient.Size ();

	if (m_bmpPicker.GetSafeHandle () == NULL)
	{
		// Prepare picker's bitmap:
		CDC dcMem;
		if (dcMem.CreateCompatibleDC (pDC) &&
			m_bmpPicker.CreateCompatibleBitmap (pDC, szColorPicker.cx, szColorPicker.cy))
		{
			CBitmap* pOldBmp = dcMem.SelectObject (&m_bmpPicker);

			int nStep = (globalData.m_nBitsPerPixel > 8) ? 1 : 4;

			for (int i= 0;i<szColorPicker.cy;i += nStep)
			{
				for(int j=0;j<szColorPicker.cx;j += nStep)
				{
					CPoint pt (j, szColorPicker.cy - i - nStep);
					COLORREF color = CBCGPDrawManager::HLStoRGB_ONE((double)j/(double)szColorPicker.cx, DEFAULT_LUMINANCE, (double)i/(double)szColorPicker.cy);

					if (globalData.m_nBitsPerPixel > 8) // High/True color
					{
						// Draw exact color:
						dcMem.SetPixelV (pt, color);
					}
					else
					{
						// Draw dithered rectangle:
						CBrush br (color);
						dcMem.FillRect (CRect (pt, CSize (nStep, nStep)), &br);
					}
				}
			}

			dcMem.SelectObject (pOldBmp);
		}
	}

	pDC->DrawState (CPoint (0, 0), szColorPicker, &m_bmpPicker, DSS_NORMAL);
}
//******************************************************************************
void CBCGPColorPickerCtrl :: DrawLuminanceBar(CDC* pDC)
{
	CRect rectClient;
	GetClientRect (rectClient);

	rectClient.DeflateRect (0, DEFAULT_OFFSET_OF_LUMINANCE_BAR);

	for (int y = rectClient.top; y <= rectClient.bottom; y ++)
	{
		COLORREF col = 	CBCGPDrawManager::HLStoRGB_ONE (m_dblHue, LumFromPoint (y), m_dblSat);

		CBrush br (col);
		pDC->FillRect(CRect (0, y, m_nLumBarWidth, y + 1), &br);
	}
}
//******************************************************************************
void CBCGPColorPickerCtrl::DrawCursor (CDC* pDC, const CRect& rect)
{
	const int nHalfSize = rect.Width () / 2;	// Assume square

	if (m_COLORTYPE == PICKER)
	{
		COLORREF colorFocus = (GetFocus() == this) ? colorBlack : colorWhite;
		
		pDC->FillSolidRect((rect.left + nHalfSize) - 1, rect.top, 3, 5, colorFocus);		// Top
		pDC->FillSolidRect((rect.left + nHalfSize) - 1, rect.bottom - 5, 3, 5, colorFocus);	// Bottom
		pDC->FillSolidRect(rect.left, (rect.top + nHalfSize) - 1, 5, 3, colorFocus);		// Left
		pDC->FillSolidRect(rect.right - 5, (rect.top + nHalfSize) - 1, 5, 3, colorFocus);	// Right
	}
	else if (m_COLORTYPE == PICKERH)
	{
		CRect rectArrow = rect;
		int iXMiddle = rectArrow.left + rectArrow.Width () / 2;

		rectArrow.DeflateRect (0, rectArrow.Height () / 3);
		rectArrow.left = iXMiddle - rectArrow.Height () - 1;
		rectArrow.right = iXMiddle + rectArrow.Height () + 1;

		int iHalfWidth =	(rectArrow.Width () % 2 != 0) ?
							(rectArrow.Width () - 1) / 2 :
							rectArrow.Width () / 2;

		CPoint pts [3];
		pts[0].x = rectArrow.left;
		pts[0].y = rectArrow.top;
		pts[1].x = rectArrow.right;
		pts[1].y = rectArrow.top;
		pts[2].x = rectArrow.left + iHalfWidth;
		pts[2].y = rectArrow.bottom + 1;

		CBrush brArrow (GetFocus() == this ? colorBlack : colorWhite);
		CPen penArrow (PS_SOLID, 1, colorBlack);

		CPen* pOldPen = (CPen*) pDC->SelectObject (&penArrow);
		CBrush* pOldBrush = (CBrush*) pDC->SelectObject(&brArrow);

		pDC->SetPolyFillMode (WINDING);
		pDC->Polygon (pts, 3);

		pDC->SelectObject (pOldBrush);
		pDC->SelectObject (pOldPen);
	}
	else if (m_COLORTYPE == LUMINANCE)
	{
		POINT pt[3];
		pt[0].x = rect.left;
		pt[0].y = rect.top + nHalfSize;
		
		pt[1].x = rect.right - 1;
		pt[1].y = rect.top;
		
		pt[2].x = rect.right - 1;
		pt[2].y = rect.bottom - 1;
		
		CPen pen (PS_SOLID, 1, globalData.clrBtnText);

		CBrush br (GetFocus() == this ? 
			globalData.clrBtnText : globalData.clrBtnShadow);
		
		CBrush* poldBrush = pDC->SelectObject (&br);
		CPen* poldPen = pDC->SelectObject(&pen);

		pDC->Polygon(pt, 3);

		pDC->SelectObject(poldBrush);
		pDC->SelectObject(poldPen);
	}
}
//******************************************************************************
void CBCGPColorPickerCtrl::SetRGB (COLORREF ref)
{
	// compute conversion to HSL
	// place the rectangle on the nearest color
	CBCGPDrawManager::RGBtoHSL (ref, &m_dblHue, &m_dblSat, &m_dblLum);
	m_colorNew = ref;

	if (GetSafeHwnd () != NULL)
	{
		Invalidate();
	}
}
//******************************************************************************
void CBCGPColorPickerCtrl::SetOriginalColor (COLORREF ref)
{
	ASSERT (m_COLORTYPE == CURRENT);
	m_colorOriginal = ref;
}
//******************************************************************************
void CBCGPColorPickerCtrl::SetHLS(double hue, double luminance, double saturation, BOOL bInvalidate)
{
	if (hue != -1)
		m_dblHue			= hue;
	
	if (saturation != -1)
		m_dblSat	= saturation;
	
	if (luminance != -1)
		m_dblLum	= luminance;
	
	m_colorNew = CBCGPDrawManager::HLStoRGB_TWO(m_dblHue, m_dblSat, m_dblLum);

	if (bInvalidate && GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}
//******************************************************************************
void CBCGPColorPickerCtrl::GetHLS(double *hue, double *luminance, double *saturation)
{
	*hue		= m_dblHue;
	*luminance	= m_dblLum;
	*saturation = m_dblSat;
}
//******************************************************************************
void CBCGPColorPickerCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture ();
	SetFocus();

	OnMouseMove (nFlags, point);
}
//*****************************************************************************
void CBCGPColorPickerCtrl::OnMouseMove (UINT nFlags, CPoint point) 
{
	if (GetCapture () != this)
	{
		return;
	}

	CRect rectClient;
	GetClientRect (rectClient);

	point.x = min (max (rectClient.left, point.x), rectClient.right);
	point.y = min (max (rectClient.top, point.y), rectClient.bottom);

	switch (m_COLORTYPE)
	{
	case LUMINANCE:
		{
			CRect rectCursorOld = GetCursorRect ();
			rectCursorOld.InflateRect (1, 1);

			m_dblLum = LumFromPoint (point.y);
			m_colorNew = CBCGPDrawManager::HLStoRGB_ONE(m_dblHue, m_dblLum, m_dblSat);

			InvalidateRect (rectCursorOld);
			InvalidateRect (GetCursorRect ());
		}
		break;

	case PICKER:
		{
			CRect rectCursorOld = GetCursorRect ();
			rectCursorOld.InflateRect (1, 1);

			if (nFlags & MK_CONTROL)
			{
				point.x = GetCursorPos ().x;
			}

			if (nFlags & MK_SHIFT)		
			{
				point.y = GetCursorPos ().y;
			}
			
			m_dblHue = (double) point.x / (double) rectClient.Width ();
			m_dblSat = 1. - (double) point.y / rectClient.Height ();
			m_colorNew = CBCGPDrawManager::HLStoRGB_ONE(m_dblHue, m_dblLum, m_dblSat);

			InvalidateRect (rectCursorOld);
			InvalidateRect (GetCursorRect ());
		}
		break;

	case PICKERH:
		{
			CRect rectCursorOld = GetCursorRect ();
			rectCursorOld.InflateRect (rectCursorOld.Width (), rectCursorOld.Height ());

			if (nFlags & MK_CONTROL)
			{
				point.x = GetCursorPos ().x;
			}

			m_dblHue = (double) point.x / (double) rectClient.Width ();
			m_colorNew = CBCGPDrawManager::HLStoRGB_ONE(m_dblHue, m_dblLum, m_dblSat);

			InvalidateRect (rectCursorOld);
			InvalidateRect (GetCursorRect ());
		}
		break;

	case HEX:
	case HEX_GREYSCALE:
		if (!SelectCellHexagon (point.x, point.y))
		{
			return;
		}

		Invalidate ();
		break;
	}

	NotifyParent ();
	UpdateWindow ();
}

void CBCGPColorPickerCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (GetCapture() == this)
		::ReleaseCapture();
	
	CButton::OnLButtonUp(nFlags, point);
}

// this function must be call before the first paint message
// on the picker control
void CBCGPColorPickerCtrl::SetLuminanceBarWidth(int w)
{
	CRect rectClient;
	GetClientRect (rectClient);

	w = min (w, rectClient.Width () * 3 / 4);

	m_nLumBarWidth = w;
	Invalidate();
}

void CBCGPColorPickerCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CButton::OnSize(nType, cx, cy);

	if (m_bmpPicker.GetSafeHandle () != NULL)
	{
		// picker's bitmap should be rebuild!
		::DeleteObject (m_bmpPicker.Detach ());
	}

	Invalidate ();
	UpdateWindow ();
}


void CBCGPColorPickerCtrl::SetType(COLORTYPE colorType)
{
	m_COLORTYPE = colorType;
}

UINT CBCGPColorPickerCtrl::OnGetDlgCode() 
{
	return DLGC_WANTARROWS;
}

void CBCGPColorPickerCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	const double dblDelta = .05;

	switch (m_COLORTYPE)
	{
	case PICKER:
		{
			CRect rectCursorOld = GetCursorRect ();
			rectCursorOld.InflateRect (1, 1);

			double dblSat = m_dblSat;
			double dblHue = m_dblHue;

			switch (nChar)
			{
			case VK_UP:
				m_dblSat += dblDelta;
				break;

			case VK_DOWN:
				m_dblSat -= dblDelta;
				break;

			case VK_LEFT:
				m_dblHue -= dblDelta;
				break;

			case VK_RIGHT:
				m_dblHue += dblDelta;
				break;
			}

			m_dblSat = min (1., max (0., m_dblSat));
			m_dblHue = min (1., max (0., m_dblHue));

			if (m_dblHue != dblHue || m_dblSat != dblSat)
			{
				m_colorNew = CBCGPDrawManager::HLStoRGB_ONE(m_dblHue, m_dblLum, m_dblSat);

				InvalidateRect (rectCursorOld);
				InvalidateRect (GetCursorRect ());

				NotifyParent ();
			}
		}
		break;

	case PICKERH:
		{
			CRect rectCursorOld = GetCursorRect ();
			rectCursorOld.InflateRect (1, 1);

			double dblHue = m_dblHue;

			switch (nChar)
			{
			case VK_LEFT:
				m_dblHue -= dblDelta;
				break;

			case VK_RIGHT:
				m_dblHue += dblDelta;
				break;
			}

			m_dblHue = min (1., max (0., m_dblHue));

			if (m_dblHue != dblHue)
			{
				m_colorNew = CBCGPDrawManager::HLStoRGB_ONE(m_dblHue, m_dblLum, m_dblSat);

				InvalidateRect (rectCursorOld);
				InvalidateRect (GetCursorRect ());

				NotifyParent ();
			}
		}
		break;

	case LUMINANCE:
		{
			CRect rectCursorOld = GetCursorRect ();
			rectCursorOld.InflateRect (1, 1);

			double dblLum = m_dblLum;

			switch (nChar)
			{
			case VK_UP:
				m_dblLum += dblDelta;
				break;

			case VK_DOWN:
				m_dblLum -= dblDelta;
				break;
			}

			m_dblLum = min (1., max (0., m_dblLum));
			if (dblLum != m_dblLum)
			{
				m_colorNew = CBCGPDrawManager::HLStoRGB_ONE(m_dblHue, m_dblLum, m_dblSat);

				InvalidateRect (rectCursorOld);
				InvalidateRect (GetCursorRect ());

				NotifyParent ();
			}
		}
		break;

	case HEX:
	case HEX_GREYSCALE:
		{
			for (int i = 0; i < m_arCells.GetSize(); i++)
			{
				CBCGPCellObj* pCell = (CBCGPCellObj*)m_arCells[i];
				ASSERT_VALID (pCell);

				if (pCell->m_CellColor == m_colorNew)
				{
					CBCGPCellObj* pNewCell = NULL;

					if (m_COLORTYPE == HEX)
					{
						int x = pCell->m_x;
						int y = pCell->m_y;

						switch (nChar)
						{
						case VK_LEFT:
							x -= pCell->m_nCellWidth;
							break;
						
						case VK_RIGHT:
							x += pCell->m_nCellWidth;
							break;

						case VK_UP:
							x -= pCell->m_nCellWidth / 2;
							y -= pCell->m_nCellWidth;
							break;

						case VK_DOWN:
							x += pCell->m_nCellWidth / 2;
							y += pCell->m_nCellWidth;
							break;
						}

						if (x != pCell->m_x || y != pCell->m_y)
						{
							if (SelectCellHexagon(x, y))
							{
								RedrawWindow();
								NotifyParent();
							}
							else if (nChar == VK_DOWN) 
							{
								// Activate Gray scale picker:
								CBCGPColorPage1* pPage = DYNAMIC_DOWNCAST(CBCGPColorPage1, GetParent());
								if (pPage != NULL && pPage->m_hexpicker_greyscale.m_arCells.GetSize() > 0)
								{
									pPage->m_hexpicker_greyscale.SetFocus();
									pNewCell = (CBCGPCellObj*)pPage->m_hexpicker_greyscale.m_arCells[0];
								}
							}
						}
					}
					else
					{
						switch (nChar)
						{
						case VK_LEFT:
						case VK_UP:
							if (i > 0)
							{
								pNewCell = (CBCGPCellObj*)m_arCells[i - 1];
							}
							else
							{
								// Activate hex picker:
								CBCGPColorPage1* pPage = DYNAMIC_DOWNCAST(CBCGPColorPage1, GetParent());
								if (pPage != NULL && pPage->m_hexpicker.m_arCells.GetSize() > 0)
								{
									pPage->m_hexpicker.SetFocus();
									pNewCell = (CBCGPCellObj*)pPage->m_hexpicker.m_arCells[1];
								}
							}
							break;
						
						case VK_RIGHT:
						case VK_DOWN:
							if (i < m_arCells.GetSize() - 1)
							{
								pNewCell = (CBCGPCellObj*)m_arCells[i + 1];
							}
							break;
						}
					}

					if (pNewCell != NULL)
					{
						ASSERT_VALID(pNewCell);

						SetColor(pNewCell->m_CellColor);
						RedrawWindow();
						NotifyParent ();
					}
					break;
				}
			}
		}
		break;

	default:
		break;
	}
	
	CButton::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CBCGPColorPickerCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CButton::OnSetFocus(pOldWnd);
	
	Invalidate();
}

void CBCGPColorPickerCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CButton::OnKillFocus(pNewWnd);
	
	Invalidate();
}

BOOL CBCGPColorPickerCtrl::OnQueryNewPalette() 
{
    Invalidate();    
	return CButton::OnQueryNewPalette();
}

void CBCGPColorPickerCtrl::OnPaletteChanged(CWnd* pFocusWnd) 
{
	CButton::OnPaletteChanged(pFocusWnd);
	
    if (pFocusWnd->GetSafeHwnd () != GetSafeHwnd ())
	{
        Invalidate ();
	}
}

BOOL CBCGPColorPickerCtrl::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= BS_OWNERDRAW;
	cs.style &= ~BS_DEFPUSHBUTTON;
	
	return CButton::PreCreateWindow(cs);
}

void CBCGPColorPickerCtrl::PreSubclassWindow() 
{
	ModifyStyle (BS_DEFPUSHBUTTON, BS_OWNERDRAW);
	CButton::PreSubclassWindow();
}

void CBCGPColorPickerCtrl::DrawItem (LPDRAWITEMSTRUCT lpDIS)
{
	if (m_pPalette == NULL)
	{
		return;
	}

	ASSERT (lpDIS != NULL);
	ASSERT (lpDIS->CtlType == ODT_BUTTON);
	ASSERT_VALID (m_pPalette);

	CDC* pDCDraw = CDC::FromHandle (lpDIS->hDC);
	ASSERT_VALID (pDCDraw);

	CPalette* pCurPalette = pDCDraw->SelectPalette (m_pPalette, FALSE);
	pDCDraw->RealizePalette();

	CRect rectClip;
	pDCDraw->GetClipBox (rectClip);

	CRect rectClient = lpDIS->rcItem;

	CDC*		pDC = pDCDraw;
	BOOL		m_bMemDC = FALSE;
	CDC			dcMem;
	CBitmap		bmp;
	CBitmap*	pOldBmp = NULL;
	CPalette*	pCurMemPalette = NULL;

	if (dcMem.CreateCompatibleDC (pDCDraw) &&
		bmp.CreateCompatibleBitmap (pDCDraw, rectClient.Width (),
								  rectClient.Height ()))
	{
		//-------------------------------------------------------------
		// Off-screen DC successfully created. Better paint to it then!
		//-------------------------------------------------------------
		m_bMemDC = TRUE;
		pOldBmp = dcMem.SelectObject (&bmp);
		pDC = &dcMem;

		pCurMemPalette = pDC->SelectPalette (m_pPalette, FALSE);
		pDC->RealizePalette ();

		globalData.DrawParentBackground (this, pDC);
	}

	switch (m_COLORTYPE)
	{
	case HEX:
		CreateHexagon ();
		DrawHex (pDC);
		break;

	case HEX_GREYSCALE:
		CreateHexGreyScaleBar ();
		DrawHex (pDC);
		break;

	case CURRENT:
		{
			COLORREF clrText = pDC->GetTextColor ();

			int nHalf = rectClient.Height () / 2;
			pDC->FillSolidRect (0, 0, rectClient.Width (), nHalf, m_colorNew);
			pDC->FillSolidRect(0, nHalf, rectClient.Width (), nHalf, m_colorOriginal);
			
			pDC->SetTextColor (clrText);	// Text color was changed by FillSolidRect

			// Draw frame
			pDC->Draw3dRect (rectClient,
				globalData.clrBtnDkShadow, globalData.clrBtnDkShadow);
		}
		break;

	case PICKER:
	case PICKERH:
		DrawPicker(pDC);
		DrawCursor (pDC, GetCursorRect ());
		pDC->Draw3dRect (rectClient, globalData.clrBtnDkShadow, globalData.clrBtnHilite);
		break;

	case LUMINANCE:
		DrawLuminanceBar (pDC);

		// Draw marker:
		globalData.DrawParentBackground (this, pDC,
			CRect (m_nLumBarWidth, 0, 
			rectClient.Width () - m_nLumBarWidth, rectClient.Height ()));
		DrawCursor (pDC, GetCursorRect ());
		break;
	}

	if (m_bMemDC)
	{
		//--------------------------------------
		// Copy the results to the on-screen DC:
		//-------------------------------------- 
		pDCDraw->BitBlt (rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
					   &dcMem, rectClip.left, rectClip.top, SRCCOPY);

		if (pCurMemPalette != NULL)
		{
			dcMem.SelectPalette (pCurMemPalette, FALSE);
		}

		dcMem.SelectObject(pOldBmp);
	}

	if (pCurPalette != NULL)
	{
		pDCDraw->SelectPalette (pCurPalette, FALSE);
	}
}

void CBCGPColorPickerCtrl::SetPalette (CPalette* pPalette)
{
	ASSERT_VALID (pPalette);
	m_pPalette = pPalette;

	if (m_bmpPicker.GetSafeHandle () != NULL)
	{
		// picker's bitmap should be rebuild!
		::DeleteObject (m_bmpPicker.Detach ());
	}

	if (GetSafeHwnd () != NULL)
	{
		Invalidate ();
		UpdateWindow ();
	}
}

BOOL CBCGPColorPickerCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

double CBCGPColorPickerCtrl::LumFromPoint (int nY)
{
	ASSERT (m_COLORTYPE == LUMINANCE);

	CRect rectClient;
	GetClientRect (rectClient);

	rectClient.DeflateRect (0, DEFAULT_OFFSET_OF_LUMINANCE_BAR);

	nY = min (max (rectClient.top, nY), rectClient.bottom);
	return ((double) rectClient.bottom - nY) / rectClient.Height ();
}

int CBCGPColorPickerCtrl::PointFromLum (double dblLum)
{
	ASSERT (m_COLORTYPE == LUMINANCE);

	CRect rectClient;
	GetClientRect (rectClient);

	rectClient.DeflateRect (0, DEFAULT_OFFSET_OF_LUMINANCE_BAR);
	return rectClient.top + (int) ((1. - dblLum) * rectClient.Height ());
}

CPoint CBCGPColorPickerCtrl::GetCursorPos ()
{
	CRect rectClient;
	GetClientRect (rectClient);

	CPoint point (0, 0);

	switch (m_COLORTYPE)
	{
	case LUMINANCE:
		point =  CPoint (rectClient.left + m_nLumBarWidth + 6, 
						PointFromLum (m_dblLum));
		break;

	case PICKER:
		point =  CPoint ((long)((double) rectClient.Width () * m_dblHue),
						(long)((1. - m_dblSat) * rectClient.Height ()));
		break;

	case PICKERH:
		point =  CPoint ((long)((double) rectClient.Width () * m_dblHue),
						rectClient.CenterPoint ().y);
		break;

	case HEX:
	case HEX_GREYSCALE:
	default:
		ASSERT (FALSE);
	}

	return point;
}

CRect CBCGPColorPickerCtrl::GetCursorRect ()
{
	CRect rect;

	switch (m_COLORTYPE)
	{
	case PICKER:
		rect = CRect (GetCursorPos (), 
				CSize (PICKER_CURSOR_SIZE, PICKER_CURSOR_SIZE));
		break;

	case PICKERH:
		GetClientRect (rect);
		rect.left = GetCursorPos ().x - PICKER_CURSOR_SIZEH / 2;
		rect.right = rect.left + PICKER_CURSOR_SIZEH;
		return rect;

	case LUMINANCE:
		rect = CRect (GetCursorPos (),
					CSize (LUM_CURSOR_SIZE, LUM_CURSOR_SIZE));
		break;

	case HEX:
	case HEX_GREYSCALE:
	default:
		ASSERT (FALSE);
		rect.SetRectEmpty ();
	}

	rect.OffsetRect (-rect.Width () / 2, -rect.Height () / 2);
	return rect;
}

void CBCGPColorPickerCtrl::OnCancelMode() 
{
	CButton::OnCancelMode();
}

void CBCGPColorPickerCtrl::NotifyParent ()
{
	CWnd* pParent = GetParent ();
	if (pParent != NULL)
	{
		pParent->SendMessage (	WM_COMMAND,
								MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED), 
								(LPARAM) GetSafeHwnd ());
	}
}

void CBCGPColorPickerCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	switch (m_COLORTYPE)
	{
	case PICKER:
	case PICKERH:
	case HEX:
	case HEX_GREYSCALE:
		{
			CWnd* pParent = GetParent ();
			if (pParent != NULL)
			{
				pParent->SendMessage (	WM_COMMAND,
										MAKEWPARAM (GetDlgCtrlID (), BN_DOUBLECLICKED), 
										(LPARAM) GetSafeHwnd ());
			}
		}

		return;
	}
	
	CButton::OnLButtonDblClk(nFlags, point);
}
