
// XTPReportPaintManager.h: interface for the CXTPReportPaintManager class.
//
// This file is a part of the XTREME REPORTCONTROL MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

//{{AFX_CODEJOCK_PRIVATE
#if !defined(__XTPREPORTPAINTMANAGER_H__)
#define __XTPREPORTPAINTMANAGER_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XTPReportDefines.h"
#include "Common/XTPColorManager.h"
#include "Common/XTPWinThemeWrapper.h"


class CXTPReportControl;
class CXTPReportRow;
class CXTPReportHeader;
class CXTPReportColumn;
class CXTPReportGroupRow;
class CXTPReportInplaceButton;
class CXTPReportHyperlink;
class CXTPReportRecordItem;
class CXTPReportRecordItemControl;
class CXTPMarkupUIElement;
class CXTPImageManager;

struct XTP_REPORTRECORDITEM_DRAWARGS;
struct XTP_REPORTRECORDITEM_METRICS;

//===========================================================================
// Summary:
//      This class used to store MulDivC formula parameters and perform calculation.
//      MulDivC formula if the folowing:
//      x = y * Mul / Div + C
//===========================================================================
class _XTP_EXT_CLASS CXTPFormulaMulDivC : public CXTPCmdTarget
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Default object constructor.
	//-----------------------------------------------------------------------
	CXTPFormulaMulDivC();

	//-----------------------------------------------------------------------
	// Summary:
	//     Default object destructor.
	//-----------------------------------------------------------------------
	virtual ~CXTPFormulaMulDivC();

	//-----------------------------------------------------------------------
	// Summary:
	//     Perform formula calculation.
	// Parameters:
	//     nY - A 'y' formula parameter value.
	// Returns:
	//      Returns value calculated by formula: y * Mul / Div + C
	//-----------------------------------------------------------------------
	virtual int Calculate(int nY) const;

	//-----------------------------------------------------------------------
	// Summary:
	//      Set formula parameters.
	// Parameters:
	//      nMul - A multiplier value.
	//      nDiv - A divisor value.
	//      nC   - A constant value.
	//-----------------------------------------------------------------------
	virtual void SetStandardValue(int nMul, int nDiv, int nC);

	//-----------------------------------------------------------------------
	// Summary:
	//      Get formula parameters.
	// Parameters:
	//      rnMul - An int reference to receive multiplier value.
	//      rnDiv - An int reference to receive divisor value.
	//      rnC   - An int reference to receive constant value.
	//-----------------------------------------------------------------------
	virtual void GetStandardValue(int& rnMul, int& rnDiv, int& rnC);

protected:
	int m_nMultiplier;  // Store a multiplier value.
	int m_nDivisor;     // Store a divisor value.
	int m_nConstant;    // Store a constant value.

};


//===========================================================================
// Summary:
//     Represents predefined grid line styles:
// Example:
//     <code>m_wndReport.SetGridStyle(FALSE, xtpReportGridLargeDots);</code>
// See Also: CXTPReportControl::SetGridStyle
//
// <KEYWORDS xtpReportGridNoLines, xtpReportGridSmallDots, xtpReportGridLargeDots, xtpReportGridDashes, xtpReportGridSolid>
//===========================================================================
enum XTPReportGridStyle
{
	xtpReportGridNoLines,       // empty line
	xtpReportGridSmallDots,     // line is drawn with small dots
	xtpReportGridLargeDots,     // line is drawn with large dots
	xtpReportGridDashes,        // line is drawn with dashes
	xtpReportGridSolid          // draws solid line
};

//===========================================================================
// Summary:
//     Represents predefined freeze column line styles:
//===========================================================================
enum XTPReportFreezeColsDividerStyle
{
	xtpReportFreezeColsDividerThin      = 0x01,  // thin line style
	xtpReportFreezeColsDividerBold      = 0x02,  // bold line style
	xtpReportFreezeColsDividerHeader    = 0x04,  // header line style
	xtpReportFreezeColsDividerShade     = 0x08,  // shade line style
	xtpReportFreezeColsDividerShowAlways = 0x10,  // show event if AutoColumnSizing is On
};

//{{AFX_CODEJOCK_PRIVATE

// deprecated
#define xtpGridNoLines      xtpReportGridNoLines
#define xtpGridSmallDots    xtpReportGridSmallDots
#define xtpGridLargeDots    xtpReportGridLargeDots
#define xtpGridDashes       xtpReportGridDashes
#define xtpGridSolid        xtpReportGridSolid

//}}AFX_CODEJOCK_PRIVATE

//===========================================================================
// Summary:
//     Represents predefined column styles.
// Example:
//     <code>m_wndReport.GetPaintManager()->m_columnStyle = xtpReportColumnFlat;</code>
// See Also: CXTPReportPaintManager::m_columnStyle, CXTPReportPaintManager
//
// <KEYWORDS xtpReportColumnShaded, xtpReportColumnFlat>
//===========================================================================
enum XTPReportColumnStyle
{
	xtpReportColumnShaded,      // Columns are gray shaded.
	xtpReportColumnFlat,        // Flat style for drawing columns.
	xtpReportColumnExplorer,    // Explorer column style
	xtpReportColumnOffice2003,  // Gradient column style
	xtpReportColumnResource   // Office 2007/2010 column style
};

//{{AFX_CODEJOCK_PRIVATE

// deprecated
#define xtpColumnShaded xtpReportColumnShaded
#define xtpColumnFlat   xtpReportColumnFlat
#define xtpReportColumnOffice2007 xtpReportColumnResource

//}}AFX_CODEJOCK_PRIVATE

//-----------------------------------------------------------------------
// Summary:
//     Represent the style of lines used to draw a hierarchical tree structure.
// Example:
//     <code>m_wndReport.GetPaintManager()->m_treeStructureStyle = xtpReportTreeStructureSolid;</code>
// See Also: CXTPReportPaintManager::m_treeStructureStyle, CXTPReportPaintManager
//
// <KEYWORDS xtpReportTreeStructureNone, xtpReportTreeStructureSolid>
//-----------------------------------------------------------------------
enum XTPReportTreeStructureStyle
{
	xtpReportTreeStructureNone,     // No lines will be drawn from the parent node to each child node.
	xtpReportTreeStructureSolid,    // Lines will be drawn from the parent node to each child node at the next level.
	xtpReportTreeStructureDots      // Dots will be drawn from the parent node to each child node at the next level.
};

//-----------------------------------------------------------------------
// Summary:
//     Represent the Header/Footer rows divider style.
//-----------------------------------------------------------------------
enum XTPReportFixedRowsDividerStyle
{
	xtpReportFixedRowsDividerNone       = 0x00, // no line
	xtpReportFixedRowsDividerThin       = 0x01, // thin style
	xtpReportFixedRowsDividerBold       = 0x02, // bold style
	xtpReportFixedRowsDividerShade      = 0x04, // shaded line style
	xtpReportFixedRowsDividerOutlook    = 0x08  // outlook line style
};

//-----------------------------------------------------------------------
// Summary:
//     Represent the Report Draw Sort Triangle logic
//-----------------------------------------------------------------------
enum XTPReportDrawSortTriangleStyle
{
	xtpReportDrawSortTriangleDefault = 0,   // The triangle will be removed when the column size is too small to display
	                                        // the full caption and triangle
	xtpReportDrawSortTriangleAlways  = 1,   // The sort triangle displayed in column header when a column is sorted and
	                                        // the caption will cut will ellipsis
	xtpReportDrawSortTriangleNever   = 2    //Never draw the triangle.
};

//===========================================================================
// Summary:
//     Utility class, handling most of the drawing activities. It stores
//     all settings, needed by a control to perform drawing operations:
//     fonts, colors, styles for all other classes of control.
//     It also implements all functions for drawing typical graphical
//     primitives, functions that directly work with device context.
// Remarks:
//     Could be overridden to provide another look and feel for the control.
//     Thus you have an easy way to change "skin" of your control; just provide
//     your own implementation of CXTPReportPaintManager and there's no need to touch
//     functionality all others classes of control
//
//     You create ReportPaintManager by calling its constructor,
//     further you can call getter and setter functions to change
//     every setting as you wish.
// See Also: CXTPReportControl overview, CXTPReportRow, CXTPReportGroupRow,
//           CXTPReportColumn, CXTPReportHeader
//===========================================================================
class _XTP_EXT_CLASS CXTPReportPaintManager : public CXTPCmdTarget
{
	//{{AFX_CODEJOCK_PRIVATE
	friend class CXTPReportControl;
	//}}AFX_CODEJOCK_PRIVATE

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Default paint manager constructor, handles properties initialization.
	// See Also: RefreshMetrics
	//-----------------------------------------------------------------------
	CXTPReportPaintManager();

	//-----------------------------------------------------------------------
	// Summary:
	//     Default paint manager destructor, handles member items deallocation.
	//-----------------------------------------------------------------------
	virtual ~CXTPReportPaintManager();

	//-----------------------------------------------------------------------
	// Summary:
	//     SetReportControl
	// Parameters:
	//     pControl - pointer to Report Control
	//-----------------------------------------------------------------------
	virtual void SetReportControl(CXTPReportControl* pControl);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Initializes all drawings defaults (fonts, colors, etc.). Most
	//     of them are system defaults.
	// See Also: CXTPReportPaintManager
	//-----------------------------------------------------------------------
	virtual void RefreshMetrics();

	//-----------------------------------------------------------------------
	// Summary:
	//     Returns default row height for print mode.
	// Parameters:
	//     pDC   - Pointer to the used Device Context.
	//     pRow  - Pointer to CXTPReportRow object.
	//     nTotalWidth - Width of the row
	// Returns:
	//     The height of the default rectangle where row's items will draw.
	// Example:
	//     <code>int nRowHeight = GetRowHeight(pDC, pRow)</code>
	//-----------------------------------------------------------------------
	virtual int GetRowHeight(CDC* pDC, CXTPReportRow* pRow);
	virtual int GetRowHeight(CDC* pDC, CXTPReportRow* pRow, int nTotalWidth); // <combine CXTPReportPaintManager::GetRowHeight@CDC*@CXTPReportRow*>

	//-----------------------------------------------------------------------
	// Summary:
	//     Returns preview item height for print mode.
	// Parameters:
	//     pDC   - Pointer to the used Device Context.
	//     pRow  - Pointer to CXTPReportRow object.
	//     nWidth - Width of the preview item.
	//     nHeight - Height of the preview item.
	// Returns:
	//     The height of the default rectangle where preview item will draw.
	// Example:
	//     <code>int nPreviewItemHeight = GetPreviewItemHeight(pDC, pRow, nWidth, nHeight)</code>
	//-----------------------------------------------------------------------
	virtual int GetPreviewItemHeight(CDC* pDC, CXTPReportRow* pRow, int nWidth, int nHeight);

	//-----------------------------------------------------------------------
	// Summary:
	//     Returns the default column header height.
	// Returns:
	//     The current height of the rectangle where column's headers draw.
	//-----------------------------------------------------------------------
	virtual int GetHeaderHeight();

	//-----------------------------------------------------------------------
	// Summary:
	//     Calculate optimal header height.
	// Parameters:
	//     pDC          - Pointer to device context.
	//     pControl     - Pointer to a report control.
	//     nTotalWidth  - Total header with. Used for printing only.
	// Returns:
	//     Optimal header height.
	//-----------------------------------------------------------------------
	virtual int GetHeaderHeight(CXTPReportControl* pControl, CDC* pDC, int nTotalWidth = 0);

	//-----------------------------------------------------------------------
	// Summary:
	// Summary:
	//     Calculate optimal footer height.
	// Parameters:
	//     pDC          - Pointer to device context.
	//     pControl     - Pointer to a report control.
	//     nTotalWidth  - Total header with. Used for printing only.
	// Returns:
	//     Optimal footer height.
	//-----------------------------------------------------------------------
	virtual int GetFooterHeight(CXTPReportControl* pControl, CDC* pDC, int nTotalWidth = 0);

	//-----------------------------------------------------------------------
	// Summary:
	//     Returns control background color.
	// Parameters:
	//     pControl - Pointer to the parent report control object.
	// Returns:
	//     Color that is used for filling client rect.
	//-----------------------------------------------------------------------
	virtual COLORREF GetControlBackColor(CXTPReportControl* pControl);

	//-----------------------------------------------------------------------
	// Summary:
	//     Fills the provided row with its background color.
	// Parameters:
	//     pDC     - Pointer to the used Device Context.
	//     pRow    - Pointer to the Row.
	//     rcRow   - Row area rectangle coordinates.
	// Example:
	// <code>
	// // paint row background
	// pPaintManager->FillRow(pDC, this, rcRow);
	// </code>
	//-----------------------------------------------------------------------
	virtual void FillRow(CDC* pDC, CXTPReportRow* pRow, CRect rcRow);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws control's grid.
	// Parameters:
	//     pDC       - Pointer to the used Device Context.
	//     bVertical - BOOL value determines orientation of grid
	//     rc        - Row area rectangle coordinates.
	// Example:
	// <code>
	// // draw vertical grid in Row
	// CRect rcGridItem(rcRowRect);
	// pPaintManager->DrawGrid(pDC, TRUE, rcGridItem);
	//
	// // draw horizontal grid in Row
	// CPaintDC pDC(this);
	// CRect rcGridItem(rcRowRect);
	// pPaintManager->DrawGrid(pDC, FALSE, rcGridItem);
	// </code>
	//-----------------------------------------------------------------------
	virtual void DrawGrid(CDC* pDC, BOOL bVertical, CRect rc);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws the freeze column divider.
	// Parameters:
	//     pDC   - Pointer to the used Device Context.
	//     rc - Rectangle coordinates.
	//     pControl - Pointer to a report control.
	//     pRow  - Pointer to the Row.
	//-----------------------------------------------------------------------
	virtual void DrawFreezeColsDivider(CDC* pDC, const CRect& rc, CXTPReportControl* pControl,
									   CXTPReportRow* pRow = NULL);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws the vertical TimeLine Marker
	// Parameters:
	//     pDC      - Pointer to the used Device Context.
	//     rc       - Rectangle coordinates.
	//     pControl - Pointer to a report control.
	//     color    - The color of the marker.
	//-----------------------------------------------------------------------
	virtual void DrawVerticalTimeLineMarker(CDC* pDC, int xPos, CXTPReportControl* pControl, COLORREF color = RGB(0,255,0));

	//Return current Print Page Width after Preview mode was call and set value
	int GetPrintPageWidth() { return m_PrintPageWidth; }
	//-----------------------------------------------------------------------
	// Summary:
	//     Draws group row.
	// Parameters:
	//     pDC   - Pointer to the used Device Context.
	//     pRow  - Pointer to the Group Row.
	//     rcRow - Group Row area rectangle coordinates.
	//     pMetrics - Pointer to a XTP_REPORTRECORDITEM_METRICS object.
	// Example:
	// <code>
	// CXTPReportPaintManager* pPaintManager = m_pControl->GetPaintManager();
	// pDC->SetBkMode(TRANSPARENT);
	// pPaintManager->DrawGroupRow(pDC, this, rcRow);
	// </code>
	//-----------------------------------------------------------------------
	virtual void DrawGroupRow(CDC* pDC, CXTPReportGroupRow* pRow, CRect rcRow, XTP_REPORTRECORDITEM_METRICS* pMetrics);

	//-----------------------------------------------------------------------
	// Summary:
	//      Fill font and colors members of the metrics structure
	//      for the specified group row.
	// Parameters:
	//      pRow        - Pointer to the Group Row.
	//      pMetrics    - Pointer to the metrics structure to be filled.
	//      bPrinting   - Is printing mode.
	//-----------------------------------------------------------------------
	virtual void FillGroupRowMetrics(CXTPReportGroupRow* pRow, XTP_REPORTRECORDITEM_METRICS* pMetrics, BOOL bPrinting);

	//-----------------------------------------------------------------------
	// Summary:
	//     Fills indent area by its background.
	// Parameters:
	//     pDC   - Pointer to the used Device Context.
	//     rcRow - Row area rectangle coordinates.
	// Example:
	// <code>
	// CXTPReportPaintManager* pPaintManager = m_pControl->GetPaintManager();
	// CRect rcRow(rcClient);
	// // Get default row height
	// int nRowHeight = m_pControl->GetRowHeight(FALSE)
	// rcRow.bottom = rcRow.top + nRowHeight;
	// pPaintManager->FillIndent(pDC, rcRow);
	// </code>
	//-----------------------------------------------------------------------
	virtual void FillIndent(CDC* pDC, CRect rcRow);

	//-----------------------------------------------------------------------
	// Summary:
	//     Fills item shade by its background if necessary.
	// Parameters:
	//     pDC    - Pointer to the used Device Context.
	//     rcItem - Item area rectangle coordinates.
	// Example:
	// <code>
	// CXTPReportPaintManager* pPaintManager = m_pControl->GetPaintManager();
	// CRect rcItem(m_rcRow);
	// pPaintManager->FillItemShade(pDC, rcItem);
	// </code>
	//-----------------------------------------------------------------------
	virtual void FillItemShade(CDC* pDC, CRect rcItem);

	//-----------------------------------------------------------------------
	// Summary:
	//     Fills column header control area by its background.
	// Parameters:
	//     pDC      -  Pointer to the used Device Context.
	//     rcHeader -  Header area rectangle coordinates.
	// Example:
	// <code>
	// CXTPReportPaintManager* pPaintManager = m_pControl->GetPaintManager();
	// CRect rcHeader(rcClient);
	// int nHeaderHeight = m_pControl->GetHeaderHeight();
	// rcHeader.bottom = rcHeader.top + nHeaderHeight;
	// pPaintManager->FillHeaderControl(pDC, rcHeader);
	// </code>
	//-----------------------------------------------------------------------
	virtual void FillHeaderControl(CDC* pDC, CRect rcHeader);

	//-----------------------------------------------------------------------
	// Summary:
	//     Fills column footer control area by its background.
	// Parameters:
	//     pDC      -  Pointer to the used Device Context.
	//     rcFooter -  Footer area rectangle coordinates.
	//-----------------------------------------------------------------------
	virtual void FillFooter(CDC* pDC, CRect rcFooter);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws column header with all related attributes (sort order, icon, etc).
	// Parameters:
	//     pDC           - Pointer to the used Device Context.
	//     pColumn       - Column header area rectangle coordinates.
	//     pHeader       - Pointer to report header.
	//     rcColumn      - Column area rectangle coordinates.
	//     bDrawExternal - whether draw Column on Header area, default is FALSE.
	// Example:
	// <code>
	// CXTPReportPaintManager* pPaintManager = m_pControl->GetPaintManager();
	// CXTPReportColumn* pColumn = CXTPReportColumn* pColumn = m_pColumns->GetAt(1);
	// CRect rcHeader(rcClient);
	// int nHeaderHeight = m_pControl->GetHeaderHeight();
	// rcHeader.bottom = rcHeader.top + nHeaderHeight;
	// GetPaintManager()->DrawColumn(pDC, pColumn, this, rcHeader);
	// </code>
	//-----------------------------------------------------------------------
	virtual void DrawColumn(CDC* pDC, CXTPReportColumn* pColumn, CXTPReportHeader* pHeader,
							CRect rcColumn, BOOL bDrawExternal = FALSE);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws column footer with all related attributes (footer text).
	// Parameters:
	//     pDC           - Pointer to the used Device Context.
	//     pColumn       - Column header area rectangle coordinates.
	//     pHeader       - Pointer to report header.
	//     rcColumn      - Column area rectangle coordinates.
	//-----------------------------------------------------------------------
	virtual void DrawColumnFooter(CDC* pDC, CXTPReportColumn* pColumn, CXTPReportHeader* pHeader, CRect rcColumn);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this method to determine if hot tracking is enabled for current column style.
	// Returns:
	//     TRUE if enabled; FALSE otherwise
	// Remarks:
	//     Hot Tracking can be used only for xtpReportColumnOffice2003 and xtpReportColumnExplorer styles.
	// See Also:
	//     m_columnStyle, m_bHotTracking
	//-----------------------------------------------------------------------
	BOOL IsColumHotTrackingEnabled() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws rows that were focused.
	// Parameters:
	//     pDC     - Pointer to the used Device Context.
	//     rcRow   - Area rectangle coordinates.
	// Example:
	// <code>
	// CRect rcClient(GetParent()->GetClientRect());
	// CXTPReportHeader* pHeader = m_pColumns->GetReportHeader();
	// CXTPReportPaintManager* pPaintManager = m_pControl->GetPaintManager();
	// GetPaintManager()->DrawResizingRect(pDC, pHeader, rcClient);
	// </code>
	//-----------------------------------------------------------------------
	virtual void DrawFocusedRow(CDC* pDC, CRect rcRow);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws Group By box.
	// Parameters:
	//     pDC       - Pointer to the used Device Context.
	//     rcGroupBy - Reference to Group By box area rectangle coordinates.
	// Example:
	// <code>
	// CXTPReportPaintManager* pPaintManager = m_pControl->GetPaintManager();
	// pPaintManager->FillGroupByControl(pDC, rcGroupBy);
	// </code>
	//-----------------------------------------------------------------------
	virtual void FillGroupByControl(CDC* pDC, CRect& rcGroupBy);


	//-----------------------------------------------------------------------
	// Summary:
	//     Returns point to additional image list with Glyphs
	// Remarks:
	//     Additional image list usually used for bitmaps of collapsed icons, etc
	//     You call this member function to get a pointer to list and manipulate
	//     its content as you wish
	// Returns:
	//     Pointer to the instantiated object of CXTPImageManager class
	//-----------------------------------------------------------------------
	CXTPImageManager* GetGlyphs() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws Horizontal line.
	// Parameters:
	//     pDC - Pointer to the used Device Context.
	//     xPos   - Horizontal coordinate of the beginning of line.
	//     yPos   - Vertical coordinate of the beginning of line.
	//     cx  - Length of line.
	//     clr - Color of line.
	// Remarks:
	//     This member function is called in the control everywhere we need to
	//     draw a simple horizontal line
	//-----------------------------------------------------------------------
	void DrawHorizontalLine(CDC* pDC, int xPos, int yPos, int cx, COLORREF clr);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws vertical line.
	// Parameters:
	//     pDC - Pointer to the used Device Context.
	//     xPos   - Horizontal coordinate of the beginning of line.
	//     yPos   - Vertical coordinate of the beginning of line.
	//     cy  - Length of line.
	//     clr - Color of line.
	// Remarks:
	//     This member function is called in the control everywhere we need to
	//     draw a simple vertical line
	//-----------------------------------------------------------------------
	void DrawVerticalLine(CDC* pDC, int xPos, int yPos, int cy, COLORREF clr);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws triangle that shows the sort order of column.
	// Parameters:
	//     pDC        - Pointer to the used Device Context.
	//     rcTriangle - Triangle area rectangle coordinates.
	//     bToDown    - If TRUE the top of triangle turn downward, otherwise upward.
	// Remarks:
	//     You use this function to draw a triangle image on the header of the column
	//     that is sorted. Triangle represents the direction of sorting: ascending or descending.
	//     Sizes of the triangle depend on the rectangle area that is provided to draw
	// Example:
	// <code>
	// // draw ascendant triangle
	// CRect rcTriangle;
	// rcTriangle.CopyRect(rcHeader);
	// rcTriangle.DeflateRect(40, 5, 5, 5);
	// DrawTriangle(pDC, rcTriangle, TRUE);
	//
	// // draw descendant triangle
	// CRect rcTriangle;
	// rcTriangle.CopyRect(rcHeader);
	// rcTriangle.DeflateRect(40, 5, 5, 5);
	// DrawTriangle(pDC, rcTriangle, FALSE);
	// </code>
	//-----------------------------------------------------------------------
	void DrawTriangle(CDC* pDC, CRect rcTriangle, BOOL bToDown, int nShadowWidth = 0);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws a column connector for Group By box.
	// Parameters:
	//     pDC    - Pointer to the used Device Context.
	//     ptFrom - Source point of the connector.
	//     ptTo   - Target point of the connector.
	// Remarks:
	//     You use this function to draw a line that represents a
	//     chain of columns in GroupBy box. It connects columns
	//     in GroupBy box altogether
	// Example:
	//     <code>DrawConnector(pDC, CPoint(rcItem.right - 5, rcItem.bottom), CPoint(x, rcItem.bottom + 4));</code>
	//-----------------------------------------------------------------------
	void DrawConnector(CDC* pDC, CPoint ptFrom, CPoint ptTo);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws collapsed bitmap.
	// Parameters:
	//     pDC      - Pointer to the used Device Context.
	//     pRow     - Pointer to the Row.
	//     rcBitmap - Bitmap area rectangle coordinates.
	// Returns:
	//     Size of drawn bitmap.
	//-----------------------------------------------------------------------
	CSize DrawCollapsedBitmap(CDC* pDC, const CXTPReportRow* pRow, CRect& rcBitmap);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws image of Report Control.
	// Parameters:
	//     pDC      - Pointer to the used Device Context.
	//     pControl - Pointer to report control which image will be drawn.
	//     rcColumn - Bitmap area rectangle coordinates.
	//     iImage   - Image index.
	// Remarks
	//     You use this function to draw a bitmap in the column rectangle.
	//     If the provided rectangle is too small, the bitmap will not be drawn.
	//     Generally, the column rectangle must be big enough to contain
	//     text (if presented) and bitmap
	// Returns:
	//     Width of drawn bitmap.
	// Example:
	// <code>
	// // draws bitmap with id ID_READ
	// pPaintManager->DrawBitmap(pDC, pControl, rcColumn, ID_READ);
	// </code>
	//-----------------------------------------------------------------------
	virtual int DrawBitmap(CDC* pDC, CXTPReportControl* pControl, CRect rcColumn, int iImage);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws image for a column.
	// Parameters:
	//      pDC      - Pointer to the used Device Context.
	//      pColumn  - Pointer to a column object.
	//      rcColumn - Column rectangle.
	//      rcIcon   - Image bounding rectangle.
	//      iIcon    - Image index.
	// Returns:
	//      Width of drawn bitmap.
	//-----------------------------------------------------------------------
	virtual int DrawColumnIcon(CDC* pDC, CXTPReportColumn* pColumn, CRect rcColumn, CRect rcIcon, int iIcon);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw standard glyphs of report control
	// Parameters:
	//     pDC      - Pointer to the used Device Context.
	//     rcColumn - Bitmap area rectangle coordinates.
	//     iImage   - Image index.
	// Returns:
	//     Size of drawn bitmap.
	//-----------------------------------------------------------------------
	virtual CSize DrawGlyph(CDC* pDC, CRect rcColumn, int iImage);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to draw the shell icon.
	// Parameters:
	//     pDC      - Pointer to the used Device Context.
	//     rcColumn - Bitmap area rectangle coordinates.
	//     iImage   - Image index.
	// Returns:
	//     Size of drawn bitmap.
	virtual CSize DrawShellIcon(CDC* pDC, CRect rcColumn, int iImage);

	//-----------------------------------------------------------------------
	// Summary:
	//     Changes report control grid lines color.
	// Parameters:
	//     clrGridLine - New grid color.
	// Returns:
	//     Previous grid color
	// Example:
	// <code>
	// // Set new color, save old one
	// COLORREF clrNew;
	// clrNew = RGB(0, 255, 0);
	// COLORREF clrOld;
	// clrOld = SetGridColor(clrNew);
	// </code>
	//-----------------------------------------------------------------------
	COLORREF SetGridColor(COLORREF clrGridLine);

	//-----------------------------------------------------------------------
	// Summary:
	//     Changes the preview mode state for the control.
	// Parameters:
	//     bIsPreviewMode - New preview mode (TRUE or FALSE).
	// Remarks:
	//     Preview mode has two states: enable and disabled. When preview is
	//     enabled, the control tries to show additional bands with preview text
	// Example:
	// <code>
	// // enable preview mode
	// EnablePreviewMode(TRUE);
	//
	// // disable preview mode
	// EnablePreviewMode(FALSE);
	// </code>
	//-----------------------------------------------------------------------
	void EnablePreviewMode(BOOL bIsPreviewMode);

	//-----------------------------------------------------------------------
	// Summary:
	//     Returns preview mode state for the control.
	// Remarks:
	//     You use this member function to programmatically identify
	//     in which mode the control is
	// Returns:
	//     Current preview mode state.
	//-----------------------------------------------------------------------
	BOOL IsPreviewMode() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Returns the default text font for the control items.
	// Returns:
	//     Pointer to the current font used for drawing test.
	//-----------------------------------------------------------------------
	CFont* GetTextFont();

	//-----------------------------------------------------------------------
	// Summary:
	//     Set count of max lines of the preview text.
	// Parameters:
	//     nMaxLines - New value of max lines of text for a preview item.
	// Remarks:
	//     You use this member function to bound the height of a textPreview item
	//     by setting the maximum rows of text for it
	// Returns:
	//     Previous value of max lines of text for a preview item.
	// Example:
	//     <code>int nOldMaxLines = SetMaxPreviewLines(4);</code>
	//-----------------------------------------------------------------------
	int SetMaxPreviewLines(int nMaxLines);

	//-----------------------------------------------------------------------
	// Summary:
	//     Get the count of max lines of  preview text.
	// Returns:
	//     Current value of max lines of text for a preview item.
	//-----------------------------------------------------------------------
	int GetMaxPreviewLines();

	//-----------------------------------------------------------------------
	// Summary:
	//     Calculates the line count for preview text, takes into account max lines.
	// Parameters:
	//     pDC     - Pointer to the used Device Context.
	//     rcText  - Reference to text area rectangle coordinates.
	//     strText - String of text.
	// Remarks:
	//     This function calculates the count of preview text lines based on the length
	//     of preview text string, preview text font and width of provided rectangle.
	//     If default max lines count less than count of calculate lines,
	//     returns default max lines
	// Returns:
	//     Returns the count of lines needed to draw preview text.
	//-----------------------------------------------------------------------
	int GetPreviewLinesCount(CDC* pDC, CRect& rcText, const CString& strText);

	//-----------------------------------------------------------------------
	// Summary:
	//     Return visibility of grid line
	// Parameters:
	//     bVertical - determines which grid lines will be tested,
	//                 vertical or horizontal.
	// Returns:
	//     BOOL value which identifies the visibility of the line
	//-----------------------------------------------------------------------
	BOOL IsGridVisible(BOOL bVertical) const;

	//-----------------------------------------------------------------------
	// Summary:
	//      Use this member function to set a header style.
	// Parameters:
	//      columnStyle - A value from XTPReportColumnStyle enum.
	// See Also: GetColumnStyle, XTPReportColumnStyle
	//-----------------------------------------------------------------------
	void SetColumnStyle(XTPReportColumnStyle columnStyle);

	//-----------------------------------------------------------------------
	// Summary:
	//      Use this member function to get a header style.
	// Returns:
	//      A value from XTPReportColumnStyle enum.
	// See Also: SetColumnStyle, XTPReportColumnStyle
	//-----------------------------------------------------------------------
	XTPReportColumnStyle GetColumnStyle() const;

	//-----------------------------------------------------------------------
	// Summary:
	//      Use this member function to set a header's custom theme base color
	//      for Office2007 column style.
	// Parameters:
	//      clrBaseColor - A COLORREF value representing the custom theme base color.
	//                 Set this value as -1 to use standard theme.
	// See Also: GetColumnOffice2007CustomThemeBaseColor
	//-----------------------------------------------------------------------
	void SetColumnOffice2007CustomThemeBaseColor(COLORREF clrBaseColor);

	//-----------------------------------------------------------------------
	// Summary:
	//      Use this member function to get a header's custom theme base color
	//      for Office2007 column style.
	// Returns:
	//      A COLORREF value representing the custom theme base color for
	//      Office2007 column style. Value -1 means that standard theme is used.
	// See Also: SetColumnOffice2007CustomThemeBaseColor
	//-----------------------------------------------------------------------
	COLORREF GetColumnOffice2007CustomThemeBaseColor() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     Sets one of the predefined grid line styles
	// Parameters:
	//     bVertical - Determines which grid lines settings,
	//                 vertical or horizontal
	//     gridStyle - Grid style. Can be one of the values listed
	//                 in the remarks section.
	// Remarks:
	//     Grid style can be one of the following:
	//          * <b>xtpReportGridNoLines</b>   Empty line
	//          * <b>xtpReportGridSmallDots</b> Line drawn by small dots
	//          * <b>xtpReportGridLargeDots</b> Line drawn by large dots
	//          * <b>xtpReportGridDashes</b>    Line drawn by dashes
	//          * <b>xtpReportGridSolid</b>     Draws solid line
	// See Also: XTPReportGridStyle
	//-----------------------------------------------------------------------
	void SetGridStyle(BOOL bVertical, XTPReportGridStyle gridStyle);

	//--------------------------------------------------------------------------
	// Summary:
	//     Call this function to get the grid style.
	// Parameters:
	//     bVertical - TRUE to get the vertical grid style and FALSE to get the
	//                 horizontal grid style.
	// Returns:
	//     A XTPReportGridStyle denoting the grid style.
	//-----------------------------------------------------------------------
	XTPReportGridStyle GetGridStyle(BOOL bVertical);
	//--------------------------------------------------------------------------
	// Summary:
	//     Creates and sets default text font
	// Parameters:
	//     lf - Reference to LOGFONT structure
	// See Also: LOGFONT, SetCaptionFont
	//-----------------------------------------------------------------------
	void SetTextFont(LOGFONT& lf);

	//-----------------------------------------------------------------------
	// Summary:
	//     Creates and sets the default caption font
	// Parameters:
	//     lf - Reference to LOGFONT structure
	// See Also: LOGFONT, SetTextFont
	//-----------------------------------------------------------------------
	void SetCaptionFont(LOGFONT& lf);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this method to set the preview item indent.
	// Parameters:
	//     nLeft   - Left indent to be used
	//     nTop    - Top indent to be used
	//     nRight  - Right indent to be used
	//     nBottom - Bottom indent to be used
	//-----------------------------------------------------------------------
	void SetPreviewIndent(int nLeft, int nTop, int nRight, int nBottom);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw in-place button of the report
	// Parameters:
	//     pDC     - Pointer to the device context
	//     pButton - Pointer to in-place button to draw
	//-----------------------------------------------------------------------
	virtual void DrawInplaceButton(CDC* pDC, CXTPReportInplaceButton* pButton);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw Combo Button of the report
	// Parameters:
	//     pDC - Pointer to the device context
	//     pRc - Pointer to rectangle to draw
	// Returns:
	//     TRUE if the operation is successful and FALSE else.
	//-----------------------------------------------------------------------
	virtual BOOL DrawInplaceComboButton(CDC* pDC, CRect* pRc);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw Expand Button of the report
	// Parameters:
	//     pDC - Pointer to the device context
	//     pRc - Pointer to rectangle to draw
	// Returns:
	//     TRUE if the operation is successful and FALSE else.
	//-----------------------------------------------------------------------
	virtual BOOL DrawInplaceExpandButton(CDC* pDC, CRect* pRc);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw Spin Button of the report
	// Parameters:
	//     pDC - Pointer to the device context
	//     pRc - Pointer to rectangle to draw
	// Returns:
	//     TRUE if the operation is successful and FALSE else.
	//-----------------------------------------------------------------------
	virtual BOOL DrawInplaceSpinButton(CDC* pDC, CRect* pRc);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw tree elements of report control.
	// Parameters:
	//     pDrawArgs - pointer to structure with drawing arguments
	//     pMetrics  - Metrics of the item
	//     rcItem    - Item area rectangle coordinates.
	//     sizeGlyph - Item glyph rectangle coordinates.
	//-----------------------------------------------------------------------
	virtual void DrawTreeStructure(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, XTP_REPORTRECORDITEM_METRICS* pMetrics, CRect rcItem, CSize sizeGlyph);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draw Item Bitmap. Override this method to draw custom bitmap.
	// Parameters:
	//     pDrawArgs - pointer to structure with drawing arguments
	//     rcItem    - Item rectangle.
	//     nImage    - Image index.
	//-----------------------------------------------------------------------
	virtual void DrawItemBitmap(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, CRect& rcItem, int nImage);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draw Item Caption. Override this method to draw custom caption.
	// Parameters:
	//     pDrawArgs - pointer to structure with drawing arguments
	//     pMetrics  - Metrics of the item
	//-----------------------------------------------------------------------
	virtual void DrawItemCaption(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, XTP_REPORTRECORDITEM_METRICS* pMetrics);

	//-----------------------------------------------------------------------
	// Summary:
	//      Draw row number. Override this method to draw differently.
	// Parameters:
	//      pDrawArgs - pointer to structure with drawing arguments
	//      rcItem    - rectangle to draw
	//      pRow      - Pointer to the report row.
	//-----------------------------------------------------------------------
	virtual void DrawRowNumber(CDC* pDC, CRect rcNumber, CXTPReportRow* pRow);
	//-----------------------------------------------------------------------
	// Summary:
	//      Draw Item Control. Override this method to draw custom control.
	// Parameters:
	//      pDrawArgs - pointer to structure with drawing arguments
	//      pControl  - control to draw
	//      rcItem - rectangle to draw
	//-----------------------------------------------------------------------
	virtual void DrawItemControl(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs,
		CXTPReportRecordItemControl* pControl, CRect& rcItem);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to determines if paint manager supports
	//     variable height of row
	// Returns:
	//     TRUE if GetRowHeight retrieves same value for each row.
	// See Also: GetRowHeight
	//-----------------------------------------------------------------------
	BOOL IsFixedRowHeight() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to set if paint manager supports
	//     variable height of row
	// Parameters:
	//     bFixedRowHeight - TRUE to set Fixed Row height, FALSE to set variable
	// See Also: GetRowHeight
	//-----------------------------------------------------------------------
	void SetFixedRowHeight(BOOL bFixedRowHeight);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to determines if paint manager supports
	//     Column Width WYSIWYG mode
	// Returns:
	//     TRUE if the column width support WYSIWYG mode FALSE else.
	//-----------------------------------------------------------------------
	BOOL IsColumnWidthWYSIWYG() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to determines if paint manager supports
	//     Last Column Width WYSIWYG mode
	// Returns:
	//     TRUE if the last column width support WYSIWYG mode FALSE else.
	//-----------------------------------------------------------------------
	BOOL IsLastColumnWidthWYSIWYG() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to determines if paint manager
	//     Show WYSIWYG Page-end Markers
	// Returns:
	//     TRUE if the WYSIWYG markers are shown FALSE else.
	//-----------------------------------------------------------------------
	BOOL CXTPReportPaintManager::IsShowWYSIWYGMarkers() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to set paint manager to
	//     Show WYSIWYG Page-end Markers
	// Parameters:
	//      bSet - BOOL flag to show or not
	//-----------------------------------------------------------------------
	void CXTPReportPaintManager::ShowWYSIWYGMarkers(BOOL bSet);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to set if paint manager supports
	//     Column Width WYSIWYG mode
	// Parameters:
	//      bColumnWidthWYSIWYG - BOOL flag to use or not
	//-----------------------------------------------------------------------
	void SetColumnWidthWYSIWYG(BOOL bColumnWidthWYSIWYG);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to set if paint manager supports
	//     Last Column Width WYSIWYG mode
	// Parameters:
	//      bColumnWidthWYSIWYG - BOOL flag to use or not
	//-----------------------------------------------------------------------
	void SetLastColumnWidthWYSIWYG(BOOL bColumnWidthWYSIWYG);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to determines if paint manager supports
	//     variable height of in place buttons.
	// Returns:
	//     TRUE if height of in place buttons is fixed, FALSE if it is the same
	//     as row height.
	// See Also: SetInplaceButtonHeight
	//-----------------------------------------------------------------------
	BOOL IsFixedInplaceButtonHeight() const;

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to set if paint manager supports
	//     fixed height of in place buttons.
	// Parameters:
	//     bFixedInplaceButtonHeight -  TRUE to set Fixed button height,
	//                                  FALSE to set variable (the same as row height).
	// See Also: IsFixedInplaceButtonHeight
	//-----------------------------------------------------------------------
	void SetInplaceButtonHeight(BOOL bFixedInplaceButtonHeight);

	//-----------------------------------------------------------------------
	// Summary:
	//      Get style of Freeze Columns Divider line.
	// Returns:
	//      A value from enum XTPReportFreezeColsDividerStyle.
	// See Also:
	//      XTPReportFreezeColsDividerStyle
	//-----------------------------------------------------------------------
	int GetFreezeColsDividerStyle() const;

	//-----------------------------------------------------------------------
	// Summary:
	//      Set style of Freeze Columns Divider line.
	// Parameters:
	//      nStyle - A value from enum XTPReportFreezeColsDividerStyle.
	// See Also:
	//      XTPReportFreezeColsDividerStyle
	//-----------------------------------------------------------------------
	void SetFreezeColsDividerStyle(int nStyle);

	//-----------------------------------------------------------------------
	// Summary:
	//     Override this method to do custom activities for the item before processing it.
	// Parameters:
	//     pDrawArgs - pointer to structure with drawing arguments
	// Returns:
	//     BOOL value -  TRUE means that the item needs further processing
	//-----------------------------------------------------------------------
	virtual BOOL OnDrawAction(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method used to draw "No items" text message in Group By area
	//     when there are no items inside it. Override it for custom drawing.
	// Parameters:
	//     pDC     - Pointer to the used Device Context.
	//     rcItem  - Reference to text area rectangle coordinates.
	// See Also:
	//     CXTPReportPaintManager::m_strNoGroupBy
	//-----------------------------------------------------------------------
	virtual void DrawNoGroupByText(CDC* pDC, CRect rcItem);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to determine if grid fills empty space after the body rows.
	// Returns:
	//     TRUE if grid fills empty space after the body rows, FALSE - otherwise (default).
	//-----------------------------------------------------------------------
	BOOL IsDrawGridForEmptySpace() const;

	//-----------------------------------------------------------------------
	// Summary:
	//  SetDrawGridForEmptySpace
	// Parameters:
	//  bDrawGrid - BOOL flag to use or not
	//-----------------------------------------------------------------------
	void SetDrawGridForEmptySpace(BOOL bDrawGrid);

	//-----------------------------------------------------------------------
	// Summary:
	// This method get flag is group row collapse-expand icon draw in print mode.
	// Returns:
	//     TRUE if GroupRowIcon used during printing
	BOOL IsPrintGroupRowIcon() const;

	//-----------------------------------------------------------------------
	// Summary:
	//  This method set flag is group row collapse-expand icon draw in print mode.
	// Parameters:
	//  bDraw - BOOL flag to use or not
	//-----------------------------------------------------------------------
	void SetPrintGroupRowIcon(BOOL bDraw);

	//-----------------------------------------------------------------------
	// Summary:
	// This method get flag of ColumnPlusMinusSign
	// Returns:
	//     TRUE if [+] or [-] icon used
	BOOL IsColumnPlusMinusSign() const;

	//-----------------------------------------------------------------------
	// Summary:
	// This method set ColumnPlusMinusSign flag (if TRUE) as [+] or [-] instead of arrows.
	// Parameters:
	//  bDraw - BOOL flag to use or not
	void SetColumnPlusMinusSign(BOOL bDraw);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Draw hyperlink string using hyperlinks text settings.
	// Parameters:
	//     pnCurrDrawPos - pointer to current drawing position (is updated after each call of function)
	//     pDrawArgs     - pointer to structure with drawing arguments
	//     pHyperlink    - pointer to hyperlink object
	//     strText       - string of text
	//     rcLink        - link hot spot area rectangle coordinates
	//     nFlag         - Flag can allow next values:
	//                     DT_END_ELLIPSIS - truncate text by ellipses
	//                     DT_WORDBREAK - - truncate text by word end
	// Remarks:
	//     Its member function is internally used by DrawTextLine and draws hyperlink
	//     chunk of text string, updates hyperlink's hot spot.
	// Returns:
	//     Position in CString where drawing of one link text was stopped
	//-----------------------------------------------------------------------
	virtual int DrawLink(int* pnCurrDrawPos,
		XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs,
		CXTPReportHyperlink* pHyperlink,
		CString strText,
		CRect rcLink,
		int nFlag);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draw a line of text with respect to the drawing rect
	// Parameters:
	//     pDrawArgs        - Pointer to structure with drawing arguments.
	//     strText          - String of text.
	//     rcItem           - Item area rectangle coordinates.
	//     nFlag            - Flag can allow next values:
	//                        DT_END_ELLIPSIS - truncate text by ellipses
	//                        DT_WORDBREAK - - truncate text by word end
	//     nCharCounter     - Reference to printed char counted, accepts value before, returns value after drawing
	//     nHyperlikCounter - Reference to printed hyperlinks counted, accepts value before, returns value after drawing
	// Remarks:
	//     Its member function is internally used by DrawText() and draws one line of text.
	//     It processes plain text/ hyperlink text for one line. For multi-line text
	//     process last line - the truncate by ellipses, otherwise truncate by word
	// Returns:
	//     Position in CString where drawing of one line of text was stopped
	//-----------------------------------------------------------------------
	virtual void DrawTextLine(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs,
		const CString& strText,
		CRect rcItem,
		int nFlag,
		int& nCharCounter,
		int& nHyperlikCounter);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draw simple string into rectangle
	// Parameters:
	//     pnCurrDrawPos - Pointer to current drawing position (is updated after each call of function)
	//     pDC           - Point to used Device Context
	//     strDraw       - String of text
	//     rcDraw        - Text area rectangle coordinates
	//     nFormat       - Flag can allow next values:
	//                     DT_END_ELLIPSIS - truncate text by ellipses
	//                     DT_WORDBREAK - truncate text by word end
	// Remarks:
	//     Its member function is internally used by DrawTextLine and draws a plain chunk of
	//     text string.
	// Returns:
	//     Position in CString where drawing of one string of text line was stopped
	//-----------------------------------------------------------------------
	virtual int DrawString(int* pnCurrDrawPos,
		CDC* pDC,
		const CString& strDraw,
		CRect rcDraw,
		UINT nFormat);

	//-----------------------------------------------------------------------
	// Summary:
	//     This method is called to draw background of the column using current column style
	// Parameters:
	//     pDC - Point to used Device Context
	//     pColumn - Column needs to draw
	//     rcColumn - Bounding rectangle of the column
	//     bColumnPressed - TRUE if column is pressed by user.
	//     pHeader - Pointer to a header the column belongs to.
	//-----------------------------------------------------------------------
	virtual void DrawColumnBackground(CDC* pDC,
		CXTPReportColumn* pColumn,
		CRect rcColumn,
		BOOL& bColumnPressed,
		CXTPReportHeader* pHeader = NULL);
protected:

	//-----------------------------------------------------------------------
	// Summary:
	//     Generates a custom grid pattern
	// Parameters:
	//     pDC      -  Pointer to device context.
	//     pBrush   -  Pointer to brush object.
	//     rc       -  Rectangle area.
	//     pPattern -  Points to a short-integer array that contains the
	//                 initial bitmap bit values. If it is NULL, the new
	//                 bitmap is left uninitialized.
	//     clr      -  COLORREF object representing the pattern color.
	//-----------------------------------------------------------------------
	virtual void DrawGridPat(CDC* pDC, CBrush* pBrush, CRect rc, CONST VOID*pPattern, COLORREF clr);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws in-place button frame
	// Parameters:
	//     pDC     - Pointer to device context.
	//     pButton - Pointer to in-place button
	//-----------------------------------------------------------------------
	virtual void DrawInplaceButtonFrame(CDC* pDC, CXTPReportInplaceButton* pButton);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws in-place button icon
	// Parameters:
	//     pDC     - Pointer to device context.
	//     pButton - Pointer to in-place button
	// Returns:
	//     TRUE if icon has been drawn, FALSE otherwise.
	//-----------------------------------------------------------------------
	virtual BOOL DrawInplaceButtonIcon(CDC* pDC, CXTPReportInplaceButton* pButton);

	//-----------------------------------------------------------------------
	// Summary:
	//     Calculate optimal height for specified column.
	// Parameters:
	//     pDC          - Pointer to device context.
	//     pColumn      - Pointer to a column object.
	//     nTotalWidth  - Total header with. Used for printing only.
	// Returns:
	//     Optimal height for specified column.
	//-----------------------------------------------------------------------
	virtual int CalcColumnHeight(CDC* pDC, CXTPReportColumn* pColumn, int nTotalWidth = 0);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws multi-line text (keeping hyperlinks active).
	// Parameters:
	//     pDrawArgs     - Pointer to structure with drawing arguments.
	//     strText       - String of text.
	//     rcItem        - Item area rectangle coordinates.
	//     bWordBreak    - Whether to break the line by words. If FALSE - only new line characters force new lines.
	//-----------------------------------------------------------------------
	virtual void DrawMultiLineText(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, const CString& strText,
								   CRect rcItem, BOOL bWordBreak = TRUE);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws single-line text (keeping hyperlinks active).
	// Parameters:
	//     pDrawArgs     - Pointer to structure with drawing arguments.
	//     strText       - String of text.
	//     rcItem        - Item area rectangle coordinates.
	//     nStartPos     - Start position in strText.
	//     nEndPos       - End position in strText.
	//     nActualWidth  - Width of text to be drawn.
	//-----------------------------------------------------------------------
	virtual void DrawSingleLineText(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs,const CString& strText,
									CRect rcItem, int nStartPos, int nEndPos,int nActualWidth);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Calculates height of the rectangle, bounding the multiline text, when the width is fixed.
	// Parameters:
	//     pDC        - Pointer to device context.
	//     strText    - String of text.
	//     nMaxWidth  - Max width of the rectangle, bounding the text.
	//     bWordBreak - Whether to break the line by words. If FALSE - only new line characters can increase the row height.
	// Returns:
	//     Necessary height to draw the multiline text.
	//-----------------------------------------------------------------------
	virtual int CalculateRowHeight(CDC* pDC, const CString& strText, int nMaxWidth, BOOL bWordBreak = TRUE);

//{{AFX_CODEJOCK_PRIVATE

	virtual CRect CalculateMaxTextRect(CDC* pDC, const CString& strText, const CRect& rcMaxRect,
		BOOL bWordBreak = TRUE, BOOL bLimitSize = TRUE, UINT uiFlags = 0);

//}}AFX_CODEJOCK_PRIVATE

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Draw hyperlink string using hyperlinks text settings (removing blanks).
	// Parameters:
	//     pnCurrDrawPos - pointer to current drawing position (is updated after each call of function)
	//     pDrawArgs     - pointer to structure with drawing arguments
	//     pHyperlink    - pointer to hyperlink object
	//     strText       - string of text
	//     rcLink        - link hot spot area rectangle coordinates
	//     nFlag         - Flag can allow next values:
	//                     DT_END_ELLIPSIS - truncate text by ellipses
	//                     DT_WORDBREAK - - truncate text by word end
	//     bTrim         - whether to trim the hyperlink
	// Remarks:
	//     Its member function is internally used by DrawTextLine and draws hyperlink
	//     chunk of text string, updates hyperlink's hot spot.
	//     If the hyperlink begins\ends with blanks, they are removed.
	// Returns:
	//     Position in CString where drawing of one link text was stopped
	//-----------------------------------------------------------------------
	virtual int DrawLink2(int* pnCurrDrawPos,
		XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs,
		CXTPReportHyperlink* pHyperlink,
		CString strText,
		CRect rcLink,
		int nFlag,
		BOOL bTrim = FALSE);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//      Get style of header divider line.
	// Returns:
	//      A value from enum XTPReportFixedRowsDividerStyle.
	// See Also:
	//      XTPReportFixedRowsDividerStyle
	//-----------------------------------------------------------------------
	int GetHeaderRowsDividerStyle() const;

	//-----------------------------------------------------------------------
	// Summary:
	//      Get style of footer divider line.
	// Returns:
	//      A value from enum XTPReportFixedRowsDividerStyle.
	// See Also:
	//      XTPReportFixedRowsDividerStyle
	//-----------------------------------------------------------------------
	int GetFooterRowsDividerStyle() const;

	//-----------------------------------------------------------------------
	// Summary:
	//      Set style of header divider line.
	// Parameters:
	//      nStyle - A value from enum XTPReportFixedRowsDividerStyle.
	// See Also:
	//      XTPReportFixedRowsDividerStyle
	//-----------------------------------------------------------------------
	void SetHeaderRowsDividerStyle(int nStyle);

	//-----------------------------------------------------------------------
	// Summary:
	//      Set style of footer divider line.
	// Parameters:
	//      nStyle - A value from enum XTPReportFixedRowsDividerStyle.
	// See Also:
	//      XTPReportFixedRowsDividerStyle
	//-----------------------------------------------------------------------
	void SetFooterRowsDividerStyle(int nStyle);

	//-----------------------------------------------------------------------
	// Summary:
	//     Draws the Fixed Rows divider (for ex. dividing header record rows and body rows).
	// Parameters:
	//     pDC   - Pointer to the used Device Context.
	//     rc - Rectangle coordinates.
	//     pControl - Pointer to a report control.
	//     bHeaderRows - whether the divider is under header rows or above footer rows.
	//     bVScrollBarVisible - TRUE, if vertical scrollbar is visible, FALSE otherwise.
	//-----------------------------------------------------------------------
	void DrawFixedRowsDivider(CDC* pDC,
		const CRect& rc,
		CXTPReportControl* pControl,
		BOOL bHeaderRows,
		BOOL bVScrollBarVisible = TRUE);

	//-----------------------------------------------------------------------
	// Summary:
	//      Get height of header divider line.
	// Returns:
	//      Header divider height.
	// See Also:
	//      XTPReportFixedRowsDividerStyle
	//-----------------------------------------------------------------------
	int GetHeaderRowsDividerHeight();

	//-----------------------------------------------------------------------
	// Summary:
	//      Get height of footer divider line.
	// Returns:
	//      Footer divider height.
	// See Also:
	//      XTPReportFixedRowsDividerStyle
	//-----------------------------------------------------------------------
	int GetFooterRowsDividerHeight();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to know whether a drop marker will be used when
	//     dragging and dropping.
	// Returns:
	//     FALSE if no markers are used.
	//-----------------------------------------------------------------------
	BOOL IsUseDropMarker() const;

//private:
//{{AFX_CODEJOCK_PRIVATE

	COLORREF MixColor(COLORREF clrLight, COLORREF clrDark, double dFactor);

	void Line(CDC* pDC, int x, int y, int cx, int cy, CPen* pPen);

	virtual void DrawTreeStructureLine(CDC* pDC, int x, int y, int cx, int cy, COLORREF clr);

	virtual void ReplaceInHyperLinks(CXTPReportRecordItem* pItem, CString& rstrText, TCHAR chReplace);

//}}AFX_CODEJOCK_PRIVATE

public:
	CFont m_fontText;           // Report items default text.
	CFont m_fontBoldText;       // Report items default bold text.
	CFont m_fontCaption;        // Column header caption font.
	CFont m_fontPreview;        // Column header caption font.

	CXTPPaintManagerColor m_clrHighlight;       // Background color of the highlighted row.
	CXTPPaintManagerColor m_clrGridLine;        // Grid lines color.
	CXTPPaintManagerColor m_clrWindowText;      // Report items default text color.
	CXTPPaintManagerColor m_clrHighlightText;   // Text color of the highlighted text.
	CXTPPaintManagerColor m_clrHeaderControl;   // Background color of the report header.
	CXTPPaintManagerColor m_clrCaptionText;     // Column header text color.
	CXTPPaintManagerColor m_clrControlBack;     // Background color of the control report area.
	CXTPPaintManagerColor m_clrGroupRowText;    // Row text color.
	CXTPPaintManagerColor m_clrGroupShadeBack;  // Group row background color when indentation shade is enabled.
	CXTPPaintManagerColor m_clrGroupShadeText;  // Group row foreground color when indentation shade is enabled.
	CXTPPaintManagerColor m_clrGroupShadeBorder;// Color for group border's shade.

	CXTPPaintManagerColor m_clrGroupBoxBack;    // Color of group box.
	CXTPPaintManagerColor m_clrControlDark;     // Dark background color of the report control (used on Group By area).
	CXTPPaintManagerColor m_clrControlLightLight;// Light background color used for drawing shades (used on column header area).
	CXTPPaintManagerColor m_clrHotDivider;      // Color of the column hot divider window (2 arrows).
	CXTPPaintManagerColor m_clrHyper;           // Hyperlink color.
	CXTPPaintManagerColor m_clrIndentControl;   // Color of the tree indentation area.
	CXTPPaintManagerColor m_clrItemShade;       // Color of the shade on sorted by column items.
	CXTPPaintManagerColor m_clrBtnFace;         // Standard button face color.
	CXTPPaintManagerColor m_clrBtnText;         // Standard button text color.
	CXTPPaintManagerColor m_clrPreviewText;     // Preview text color.
	CXTPPaintManagerColor m_clrSelectedRow;     // Selected row background color.
	CXTPPaintManagerColor m_clrSelectedRowText; // Selected row text color.

	CXTPPaintManagerColorGradient m_grcGradientColumn;          // Color of column used with xtpReportColumnOffice2003 style
	CXTPPaintManagerColorGradient m_grcGradientColumnHot;       // HotTracking Color of column used with xtpReportColumnOffice2003 style
	CXTPPaintManagerColorGradient m_grcGradientColumnPushed;    // Pushed Color of column used with xtpReportColumnOffice2003 style
	CXTPPaintManagerColor m_clrGradientColumnShadow;            // Bottom shadow of column used with xtpReportColumnOffice2003 style
	CXTPPaintManagerColor m_clrGradientColumnSeparator;         // Separator of column used with xtpReportColumnOffice2003 style
	CXTPPaintManagerColor m_crlNoGroupByText;           // A color for "No items" text in GroupBy area (see m_strNoGroupBy member)
	CXTPPaintManagerColor m_clrFreezeColsDivider;       // Freeze Columns Divider color.
	CXTPPaintManagerColor m_clrHeaderRowsDivider;       // Color of header divider.
	CXTPPaintManagerColor m_clrFooterRowsDivider;       // Color of footer divider.
	COLORREF m_clrColumnOffice2007CustomTheme;          // Base color for Office2007 custom theme.

	COLORREF m_clrFormulaSum;   // Color for Formula Sum fields
	COLORREF m_clrFormulaSubSum; // Color for Formula SubSum fields

	COLORREF m_clrAlternativeBackground; // Color for Alternative Row Background
	BOOL m_bUseAlternativeBackground;    // Flag to use for Alternative Row Background
	BOOL m_bAllowMergeCells;             // Flag to Allow or not Merge Cells

	int m_nTextIndentForGroupRowsWithNoIcon;  // Specifies Text indent if a group row icon is not used

	BOOL m_bGroupRowTextBold;       // TRUE to draw group rows with bold text.
	BOOL m_bShadeGroupHeadings;     // Show or not show the indentation shade.
	BOOL m_bShadeSortColumn;        // Show or not show the items shade in the sorted by column.
	BOOL m_bHotTracking;            // TRUE to allow column hot tracking
	BOOL m_bInvertColumnOnClick;    // TRUE to invert column on click

	CString m_strNoItems; // A string which contains customized "No Items" text for displaying in view when there are no visible rows.
	CString m_strSortBy; // A string which contains customized "Sort By" text for displaying in tooltip
	CString m_strIncreasing; // A string which contains customized "Increasing" text for displaying in tooltip
	CString m_strDecreasing; // A string which contains customized "Decreasing" text for displaying in tooltip
	CString m_strNoGroupBy; // A string which contains customized "No items" text message for displaying in Group By area when there are no items inside it.
	CString m_strNoFieldsAvailable; // A string which contains customized "No Fields Available" text message for displaying in Field Chooser.
	CString m_strExpand; // A string which contains customized "Expand" text for displaying in tooltip
	CString m_strCollapse; // A string which contains customized "Collapse" text for displaying in tooltip
	BOOL m_bHideSelection; // TRUE when hiding report control's selection by drawing selected items as others, FALSE otherwise.
	BOOL m_bPrintSortTriangle; //If true - show sort triangle in printout also
	XTPReportDrawSortTriangleStyle m_DrawSortTriangleStyle;
	//xtpReportDrawSortTriangleAlways - the sort triangle displayed in column header when a column is sorted and
	//the caption will cut will ellipsis
	//xtpReportDrawSortTriangleDefault - the triangle will be removed when the column size is too small to display
	//the full caption and triangle
	//xtpReportDrawSortTriangleNever - the sort triangle never used
	BOOL m_bUseColumnTextAlignment;     // TRUE to draw caption text accordingly to its alignment

	BOOL m_bRevertAlignment;            // TRUE to revert column alignments
	BOOL m_bUseEditTextAlignment;       //  TRUE to automatically apply edit alignment using column alignment

	BOOL m_bShowLockIcon;               //TRUE to show the lock icon FALSE to hide.
	BOOL m_bUseShellIcon;               //TRUE to use the shell icon FALSE else.
	int m_iRecOrRowStart;               //start position in numbering (0 or 1 or something user want)
	BOOL m_bRecOrRowNum;                // flag for Record (TRUE) or Row (FALSE) number to display if supported mode used

	int m_nTreeStructureStyle;          // Tree structure style.

	XTPReportTreeStructureStyle m_treeStructureStyle;   // Tree structure style
	XTPReportFixedRowsDividerStyle m_HeaderRowsDividerStyle; // Style of a divider line between the header and body rows.
	XTPReportFixedRowsDividerStyle m_FooterRowsDividerStyle; // Style of a divider line between the footer and body rows.

	CRect m_rcPreviewIndent;            // Preview indentation.
	int m_nTreeIndent;                  // Tree indentation.
	int m_nFixedColumnsIndent;          // Fixed Column total width.
	BOOL m_bMoveScrollbarOnFixedColumnsIndent;  //Move Scrollbar right follow FixedColumnsIndent

	BOOL m_bThemedInplaceButtons;       // Draw in-place buttons using current theme settings.
	BOOL m_bBaseTreeIndent;             // Flag to add Extra Tree indentation to cover initial TreeColumn.

	CUIntArray m_arStartCol;            //hold start column in each virtual page
	CUIntArray m_arStartRow;            //hold start row in each virtual page

	int m_PrintPageWidth;               //width of print page (run-time)
	BOOL m_bPrintVirtualPageNumber;     //print horizontal page #
	BOOL m_bPrintPageRectangle;         //flag to draw rectangle around page
	int m_RecordNumberWidth;            //column width for row / record numbers
	BOOL m_bForceDynamicMarkupForCell;  // TRUE if use Markup for Report Record Item
	BOOL m_bForceShowDropMarker;        // TRUE if Force Show Drop Marker
	                                    //used to support external (OLE) drag & drop handler and take care of drop marker line drawing and auto-scrolling
	BOOL m_bShowNonActiveInPlaceButton; // TRUE if use Show NonActive InPlace Button (Combo, Expand, Spin)
	BOOL m_bPrintWatermark;             //flag to print watermark
	BOOL m_bUseGroupRowSelectionState;  // TRUE if use GroupSelectionState Glyph
	BOOL m_bForceShowTooltip;           // TRUE if use show tooltip for any cell width case
	BOOL m_bCleanTooltip;               // TRUE if use show tooltip without markup
	BOOL m_bGrayIfDisable;              //TRUE is disable draw in gray (standard behavior)
	int m_nRowHeightDefault;            //The default row height.
	BOOL m_bCompactText;                // Flag to ignore Icon for Narrow column to show more text

	BOOL m_bUseDropMarker;                              // FALSE to not draw drop marked when dragging\dropping, TRUE by default
	BOOL m_bSortTriangleOffsetFromRight;    // Offset of sort triangle
	int m_nTreeColumnLineLength;            // This is the extra line length to apply to tree column lines if there is no expand button
	int m_nBaseParentRecordIndent;          // This is initial number of pixels a parent node will be indented when in a group

	CMap<CXTPReportColumn*, CXTPReportColumn*, CRect, CRect> m_mapColumnPrintPosition;

//{{AFX_CODEJOCK_PRIVATE

protected:
	int m_nPageCols;
	int m_nPageRows;
	int m_nPageCount;
	int m_nRowCount;
	int m_nColumnHeight;
	int* m_nColumns;
	int* m_nColumnsWidth;

//}}AFX_CODEJOCK_PRIVATE

	int m_nRowHeight;               // Report row default height.
	int m_nHeaderHeight;            // Column header height.
	int m_nFooterHeight;            // Column footer height.
	int m_nGroupGridLineHeight;     // The height of the group grid line.

	CXTPImageManager* m_pGlyphs;    // Contains an additional image list for report control
	BOOL m_bIsPreviewMode;          // Show or not show the item preview.
	int m_nResizingWidth;           // The width of the column resizing marker area.
	int m_nMaxPreviewLines;         // Restrict maximum lines for preview text
	CSize m_szGlyph;                // Glyph size
	CBrush m_brushVeriticalGrid;    // Stores brush to draw vertical grid lines
	CBrush m_brushHorizontalGrid;   // Stores brush to draw horizontal grid lines
	CBrush m_brushTreeStructure;    // Stores brush to draw tree nodes
	CBrush m_brushGroupGridLineStyle;       // Stores brush to draw group row grid lines

	int m_nFreezeColsDividerStyle;  // A set of flags from XTPReportFreezeColsDividerStyle

	BOOL m_bFixedRowHeight;         // TRUE if fixed rows used.
	BOOL m_bColumnWidthWYSIWYG;     // TRUE if columns width WYSIWYG mode used.
	BOOL m_bShowWYSIWYGMarkers;     // TRUE if in non-print mode header show red page-end markers
	BOOL m_bLastColumnWidthWYSIWYG; // TRUE if last column width WYSIWYG mode used.
	BOOL m_bDrawGridForEmptySpace;  // TRUE if grid fills the empty space (if any) after the body rows.
	BOOL m_bPrintGroupRowIcon;      // TRUE if printout draw collapse-expand icon for group row
	BOOL m_bColumnPlusMinusSign;    // TRUE if use Expand/Collapse icon, FALSE if use DirectionalArrows

	BOOL m_bFixedInplaceButtonHeight;           // TRUE if fixed In place Buttons height used.

	XTPReportGridStyle m_nGroupGridLineStyle;   // Stores current style for group row grid lines
	XTPReportGridStyle m_verticalGridStyle;     // Stores current style for vertical grid lines
	XTPReportGridStyle m_horizontalGridStyle;   // Stores current style for horizontal grid lines
	XTPReportColumnStyle m_columnStyle;         // Contains column drawing style.

	CXTPWinThemeWrapper m_themeWrapper;         // Header theme wrapper.
	CXTPWinThemeWrapper m_themeButton;          // In-place button theme wrapper.
	CXTPWinThemeWrapper m_themeCombo;           // Combo box in-place button theme wrapper.
	CXTPWinThemeWrapper m_themeSpin;            // Spin in-place button theme wrapper.

	CXTPFormulaMulDivC* m_pHeaderHeightFormula; // MulDivC formula for header height calculation

	CXTPReportControl* m_pControl;              //Pointer to the report control object.

	CXTPMarkupUIElement* m_pMarkupUIElement_NoGroupBy;  //The no group by markup UI element.
	CString m_strMarkupUIElement_NoGroupBy;             //The no group by markup UI element string.


};

AFX_INLINE BOOL CXTPReportPaintManager::IsUseDropMarker() const
{
	return m_bUseDropMarker;
}

AFX_INLINE int CXTPReportPaintManager::GetHeaderHeight()
{
	return m_nHeaderHeight;
}

AFX_INLINE BOOL CXTPReportPaintManager::IsGridVisible(BOOL bVertical) const
{
	return bVertical ? (m_verticalGridStyle != xtpReportGridNoLines) : (m_horizontalGridStyle != xtpReportGridNoLines);
}

AFX_INLINE void CXTPReportPaintManager::EnablePreviewMode(BOOL bIsPreviewMode)
{
	m_bIsPreviewMode = bIsPreviewMode;
}

AFX_INLINE BOOL CXTPReportPaintManager::IsPreviewMode() const
{
	return m_bIsPreviewMode;
}

AFX_INLINE COLORREF CXTPReportPaintManager::SetGridColor(COLORREF clrGridLine)
{
	COLORREF clrOldColor = m_clrGridLine;
	m_clrGridLine.SetCustomValue(clrGridLine);
	return clrOldColor;
}

AFX_INLINE CFont* CXTPReportPaintManager::GetTextFont()
{
	return &m_fontText;
}

AFX_INLINE int CXTPReportPaintManager::SetMaxPreviewLines(int nMaxLines)
{
	int nOldMaxLines = m_nMaxPreviewLines;
	m_nMaxPreviewLines = nMaxLines;
	return nOldMaxLines;
}

AFX_INLINE int CXTPReportPaintManager::GetMaxPreviewLines()
{
	return m_nMaxPreviewLines;
}

AFX_INLINE int CXTPReportPaintManager::GetPreviewItemHeight(CDC* /*pDC*/, CXTPReportRow* /*pRow*/, int /*nWidth*/, int nHeight)
{
	return nHeight;
}

AFX_INLINE CXTPImageManager* CXTPReportPaintManager::GetGlyphs() const
{
	return m_pGlyphs;
}

AFX_INLINE BOOL CXTPReportPaintManager::IsFixedRowHeight() const
{
	return m_bFixedRowHeight;
}

AFX_INLINE void CXTPReportPaintManager::SetFixedRowHeight(BOOL bFixedRowHeight)
{
	m_bFixedRowHeight = bFixedRowHeight;
}

AFX_INLINE BOOL CXTPReportPaintManager::IsLastColumnWidthWYSIWYG() const
{
	return m_bLastColumnWidthWYSIWYG;
}

AFX_INLINE BOOL CXTPReportPaintManager::IsColumnWidthWYSIWYG() const
{
	return m_bColumnWidthWYSIWYG;
}

AFX_INLINE BOOL CXTPReportPaintManager::IsShowWYSIWYGMarkers() const
{
	return m_bShowWYSIWYGMarkers;
}

AFX_INLINE void CXTPReportPaintManager::ShowWYSIWYGMarkers(BOOL bSet)
{
	m_bShowWYSIWYGMarkers = bSet;
}

AFX_INLINE void CXTPReportPaintManager::SetColumnWidthWYSIWYG(BOOL bColumnWidthWYSIWYG)
{
	m_bColumnWidthWYSIWYG = bColumnWidthWYSIWYG;
}

AFX_INLINE void CXTPReportPaintManager::SetLastColumnWidthWYSIWYG(BOOL bColumnWidthWYSIWYG)
{
	m_bLastColumnWidthWYSIWYG = bColumnWidthWYSIWYG;
}

AFX_INLINE BOOL CXTPReportPaintManager::IsFixedInplaceButtonHeight() const
{
	return m_bFixedInplaceButtonHeight;
}

AFX_INLINE void CXTPReportPaintManager::SetInplaceButtonHeight(BOOL bFixedInplaceButtonHeight)
{
	m_bFixedInplaceButtonHeight= bFixedInplaceButtonHeight;
}

AFX_INLINE int  CXTPReportPaintManager::GetFreezeColsDividerStyle() const
{
	return m_nFreezeColsDividerStyle;
}

AFX_INLINE void CXTPReportPaintManager::SetFreezeColsDividerStyle(int nStyle)
{
	m_nFreezeColsDividerStyle = nStyle;
}

AFX_INLINE int  CXTPReportPaintManager::GetHeaderRowsDividerStyle() const
{
	return m_HeaderRowsDividerStyle;
}

AFX_INLINE void CXTPReportPaintManager::SetHeaderRowsDividerStyle(int nStyle)
{
	m_HeaderRowsDividerStyle = (XTPReportFixedRowsDividerStyle) nStyle;
}

AFX_INLINE int  CXTPReportPaintManager::GetFooterRowsDividerStyle() const
{
	return m_FooterRowsDividerStyle;
}

AFX_INLINE void CXTPReportPaintManager::SetFooterRowsDividerStyle(int nStyle)
{
	m_FooterRowsDividerStyle = (XTPReportFixedRowsDividerStyle) nStyle;
}

AFX_INLINE COLORREF CXTPReportPaintManager::MixColor(COLORREF clrLight, COLORREF clrDark, double dFactor)
{
	return RGB(GetRValue(clrLight) - dFactor * (GetRValue(clrLight) - GetRValue(clrDark)),
		GetGValue(clrLight) - dFactor * (GetGValue(clrLight) - GetGValue(clrDark)),
		GetBValue(clrLight) - dFactor * (GetBValue(clrLight) - GetBValue(clrDark)));
}

AFX_INLINE void CXTPReportPaintManager::Line(CDC* pDC, int x, int y, int cx, int cy, CPen* pPen)
{
	CPen* pOldPen = pDC->SelectObject(pPen);
	pDC->MoveTo(x, y);
	pDC->LineTo(x + cx, y + cy);
	pDC->SelectObject(pOldPen);
}

AFX_INLINE BOOL CXTPReportPaintManager::OnDrawAction(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs)
{
	UNREFERENCED_PARAMETER(pDrawArgs);
	return TRUE;
}

AFX_INLINE XTPReportColumnStyle CXTPReportPaintManager::GetColumnStyle() const
{
	return m_columnStyle;
}

AFX_INLINE BOOL CXTPReportPaintManager::IsColumnPlusMinusSign() const
{
	return m_bColumnPlusMinusSign;
}

AFX_INLINE void CXTPReportPaintManager::SetColumnPlusMinusSign(BOOL bDraw)
{
	m_bColumnPlusMinusSign = bDraw;
}

AFX_INLINE BOOL CXTPReportPaintManager::IsPrintGroupRowIcon() const
{
	return m_bPrintGroupRowIcon;
}

AFX_INLINE void CXTPReportPaintManager::SetPrintGroupRowIcon(BOOL bDraw)
{
	m_bPrintGroupRowIcon = bDraw;
}

AFX_INLINE BOOL CXTPReportPaintManager::IsDrawGridForEmptySpace() const
{
	return m_bDrawGridForEmptySpace;
}

AFX_INLINE void CXTPReportPaintManager::SetDrawGridForEmptySpace(BOOL bDrawGrid)
{
	m_bDrawGridForEmptySpace = bDrawGrid;
}

AFX_INLINE void CXTPReportPaintManager::SetColumnOffice2007CustomThemeBaseColor(COLORREF clrBaseColor)
{
	m_clrColumnOffice2007CustomTheme = clrBaseColor;
	RefreshMetrics();
}

AFX_INLINE COLORREF CXTPReportPaintManager::GetColumnOffice2007CustomThemeBaseColor() const
{
	return m_clrColumnOffice2007CustomTheme;
}

AFX_INLINE void CXTPReportPaintManager::SetReportControl(CXTPReportControl* pControl)
{
	m_pControl = pControl;
}

AFX_INLINE XTPReportGridStyle CXTPReportPaintManager::GetGridStyle(BOOL bVertical)
{
	if (bVertical)
		return m_verticalGridStyle;

	return m_horizontalGridStyle;
}

#define  m_bDrawSortTriangleAlways m_bPrintSortTriangle

#endif //#if !defined(__XTPREPORTPAINTMANAGER_H__)
