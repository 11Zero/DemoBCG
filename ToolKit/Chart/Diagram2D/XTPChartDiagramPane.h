// XTPChartDiagramPane.h
//
// This file is a part of the XTREME TOOLKIT PRO MFC class library.
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
#if !defined(__XTPCHARTDIAGRAMPANE_H__)
#define __XTPCHARTDIAGRAMPANE_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CXTPChartAxis;
class CXTPChartDiagram2D;
class CXTPChartElementView;
class CXTPChartDeviceContext;
class CXTPChartDeviceCommand;
class CXTPChartAxisView;
class CXTPChartFillStyle;
class CXTPChartDiagram2DAppearance;
class CXTPChartAxisView;
class CXTPChartBoundElementView;
class CXTPChartSeriesView;

#include "../XTPChartElement.h"

namespace Gdiplus
{
	class Matrix;
};


class _XTP_EXT_CLASS CXTPChartDiagramPane : public CXTPChartElement
{
	DECLARE_DYNAMIC(CXTPChartDiagramPane);

public:
	CXTPChartDiagramPane(CXTPChartDiagram* pDiagram);
	virtual ~CXTPChartDiagramPane();

public:
	virtual CXTPChartElementView* CreateView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParent);

	CXTPChartDiagram2D* GetDiagram() const;

public:
	CXTPChartColor GetBackgroundColor() const;
	CXTPChartColor GetBackgroundColor2() const;
	CXTPChartColor GetBorderColor() const;

	CXTPChartColor GetActualBackgroundColor() const;
	CXTPChartColor GetActualBackgroundColor2() const;
	CXTPChartColor GetActualBorderColor() const;

	void SetBackgroundColor(const CXTPChartColor& color);
	void SetBackgroundColor2(const CXTPChartColor& color);
	void SetBorderColor(const CXTPChartColor& color);


	CXTPChartFillStyle* GetFillStyle() const;

public:
	void DoPropExchange(CXTPPropExchange* pPX);

public:

	CXTPChartDiagram2DAppearance* GetAppearance() const;

	CXTPChartFillStyle* GetActualFillStyle() const;


protected:
	CXTPChartColor m_clrBackgroundColor;
	CXTPChartColor m_clrBackgroundColor2;
	CXTPChartColor m_clrBorderColor;
	CXTPChartFillStyle* m_pBackgroundFillStyle;
};



class _XTP_EXT_CLASS CXTPChartDiagramPaneView : public CXTPChartElementView
{
public:
	CXTPChartDiagramPaneView(CXTPChartDiagramPane* pPane, CXTPChartElementView* pParent);
	~CXTPChartDiagramPaneView();

public:
	CRect GetBounds() const;

	CXTPChartPointF GetScreenPoint(const CXTPChartSeriesView* pView, double x, double y) const;

	void CalculateView(CRect rcBounds);

public:
	CXTPChartAxisView* GetAxisView(CXTPChartAxis* pAxis) const;

	CXTPChartElementView* CreateSeriesView();

protected:
	virtual CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

	virtual CXTPChartDeviceCommand* CreateGridLinesDeviceCommand(CXTPChartDeviceContext* pDC, CXTPChartAxisView* pAxis);
	virtual CXTPChartDeviceCommand* CreateInterlacedDeviceCommand(CXTPChartDeviceContext* pDC, CXTPChartAxisView* pAxis);
	virtual CXTPChartDeviceCommand* CreateConstantLinesDeviceCommand(CXTPChartDeviceContext* pDC, CXTPChartAxisView* pAxis, BOOL bBehind);
	virtual CXTPChartDeviceCommand* CreateStripsDeviceCommand(CXTPChartDeviceContext* pDC, CXTPChartAxisView* pAxis);

public:
	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL OnSetCursor(CPoint point);

public:
	CXTPChartDiagramPane* m_pPane;
	CXTPChartElementView* m_pSeriesView;
	CRect m_rcBounds;
	CPoint m_ptOldPosition;
};

AFX_INLINE CXTPChartDiagram2D* CXTPChartDiagramPane::GetDiagram() const {
	return (CXTPChartDiagram2D*)m_pOwner;
}
AFX_INLINE CRect CXTPChartDiagramPaneView::GetBounds() const {
	return m_rcBounds;
}
AFX_INLINE CXTPChartFillStyle* CXTPChartDiagramPane::GetFillStyle() const {
	return m_pBackgroundFillStyle;
}
AFX_INLINE void CXTPChartDiagramPane::SetBackgroundColor(const CXTPChartColor& color) {
	m_clrBackgroundColor = color;
	OnChartChanged();
}
AFX_INLINE void CXTPChartDiagramPane::SetBackgroundColor2(const CXTPChartColor& color) {
	m_clrBackgroundColor2 = color;
	OnChartChanged();
}
AFX_INLINE void CXTPChartDiagramPane::SetBorderColor(const CXTPChartColor& color) {
	m_clrBorderColor = color;
	OnChartChanged();
}


#endif //#if !defined(__XTPCHARTDIAGRAMPANE_H__)
