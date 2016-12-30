//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a sample for BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
// 
// BCGPChartLegend.h: declaration of the CBCGPChartLegend and related classes.
//
//////////////////////////////////////////////////////////////////////
#if !defined(AFX_BCGPCHARTLEGEND_H__1875ABCF_5225_490C_BBD3_9B6CFB20ADC0__INCLUDED_)
#define AFX_BCGPCHARTLEGEND_H__1875ABCF_5225_490C_BBD3_9B6CFB20ADC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPVisualContainer.h"
#include "BCGPChartFormat.h"

extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_LEGEND_AFTER_DRAW;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_LEGEND_CONTENT_CREATED;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_LEGEND_MOUSE_DOWN;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CHART_LEGEND_MOUSE_UP;

class CBCGPChartSeries;
class CBCGPChartLegendEntry;
class CBCGPChartLegendVisualObject;

class BCGCBPRODLLEXPORT CBCGPLegendScrollButton
{
public:
	CBCGPLegendScrollButton(BOOL bIsLeftTop)
	{
		m_bIsLeftTop = bIsLeftTop;
		m_bIsPressed = FALSE;
	}

	static CBCGPSize m_szScrollButtonSize;

	const CBCGPRect& GetRect() const {return m_rectBounds;}
	BOOL IsPressed() const {return m_bIsPressed;}
	BOOL IsLeftTop() const {return m_bIsLeftTop;}

	void SetRect(const CBCGPRect& rect) {m_rectBounds = rect;}
	void SetPosition(const CBCGPPoint& pt) {m_rectBounds.SetRect(pt, m_szScrollButtonSize);}
	BOOL SetPressed(BOOL bSet) 
	{
		BOOL bNeedRedraw = m_bIsPressed != bSet;
		m_bIsPressed = bSet;
		return bNeedRedraw;
	}

	void AdjustPosition(CBCGPGraphicsManager* pGM, CBCGPChartLegendVisualObject* pLegend);

	virtual void OnDraw(CBCGPGraphicsManager* pGM, CBCGPChartLegendVisualObject* pLegend);

protected:
	CBCGPRect	m_rectBounds;
	BOOL		m_bIsPressed;
	BOOL		m_bIsLeftTop;

	CBCGPChartLegendVisualObject* m_pParentLegend;
};

struct BCGCBPRODLLEXPORT BCGPChartCellParams
{
	BCGPChartCellParams()
	{
		m_szScaleRatio.SetSize(1., 1.);
		m_textFormat.Create(BCGPChartFormatLabel::m_strDefaultFontFamily, 12.0f);
	}

	CBCGPBrush				m_brContentFill; // fill color of legend entry

	CBCGPBrush				m_brFill; // series fill color used to draw legend key
	BCGPChartFormatLine		m_lineStyle;

	CBCGPBrush				m_brMarkerFill;
	BCGPChartFormatLine		m_markerLineStyle;
	BCGPChartMarkerOptions	m_markerOptions;

	CBCGPBrush				m_brTextColor;
	CBCGPTextFormat			m_textFormat;

	CBCGPSize GetMarkerSize(BOOL bScaled = TRUE) const
	{
		if (!bScaled)
		{
			return CBCGPSize(m_markerOptions.GetMarkerSize(), m_markerOptions.GetMarkerSize());
		}
		return CBCGPSize(m_markerOptions.GetMarkerSize() * m_szScaleRatio.cx, 
							m_markerOptions.GetMarkerSize() * m_szScaleRatio.cy);
	}

	void DetachFromGM()
	{
		m_brContentFill.Destroy();
		m_brFill.Destroy();
		m_lineStyle.DetachFromGM();

		m_brMarkerFill.Destroy();
		m_markerLineStyle.DetachFromGM();

		m_brTextColor.Destroy();
		m_textFormat.Destroy();
	}

	void SetContentPadding(const CBCGPSize& sz)
	{
		m_szContentPadding = sz;
	}
	
	CBCGPSize GetContentPadding(BOOL bScaled = FALSE) const
	{
		if (bScaled)
		{
			return CBCGPSize(m_szContentPadding.cx * m_szScaleRatio.cx, m_szContentPadding.cy * m_szScaleRatio.cy);
		}
		
		return m_szContentPadding;
	}

	const CBCGPSize& GetScaleRatio() const {return m_szScaleRatio;}
	void SetScaleRatio(const CBCGPSize& sz) 
	{
		m_lineStyle.SetScaleRatio(sz);
		m_markerLineStyle.SetScaleRatio(sz);
		m_textFormat.Scale(sz.cy);
		m_szScaleRatio = sz;
	}

	BCGPChartCellParams& operator=(const BCGPChartCellParams& src)
	{
		CopyFrom(src);
		return *this;
	}

	void CopyFrom(const BCGPChartCellParams& src)
	{
		m_brContentFill = src.m_brContentFill;
		m_brFill = src.m_brFill;
		m_lineStyle = src.m_lineStyle;

		m_brMarkerFill = src.m_brMarkerFill;
		m_markerLineStyle = src.m_markerLineStyle;
		m_markerOptions = src.m_markerOptions;

		m_brTextColor = src.m_brTextColor;
		m_textFormat = src.m_textFormat;

		m_szContentPadding = src.GetContentPadding(FALSE);
		m_szScaleRatio = src.GetScaleRatio();

		DetachFromGM();
	}
	
protected:
	CBCGPSize	m_szContentPadding;
	CBCGPSize	m_szScaleRatio;

};

//****************************************************************************************
// Legend Cell
//****************************************************************************************

class BCGCBPRODLLEXPORT CBCGPChartLegendCell : public CObject
{
	friend class CBCGPChartLegendVisualObject;
	friend class CBCGPChartLegendEntry;

	DECLARE_DYNCREATE(CBCGPChartLegendCell)
public:
	enum LegendCellType
	{
		LCT_KEY,
		LCT_LABEL,
		LCT_CHART_NAME,
		LCT_CUSTOM
	};

	CBCGPChartLegendCell();
	CBCGPChartLegendCell(CBCGPChartVisualObject* pChart, CBCGPChartSeries* pSeries, int	nDataPointIndex, 
		LegendCellType type);
	CBCGPChartLegendCell(const CString& strContent, CObject* pCustomData = NULL);
	void Init();

	virtual ~CBCGPChartLegendCell();

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld);

	LegendCellType	GetCellType() const {return m_cellType;}

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rect);

	BOOL ContainsSeries(CBCGPChartSeries* pSeries, int nDataPointIndex) const
	{
		return (m_pSeries == pSeries && m_nDataPointIndex == nDataPointIndex);
	}

	virtual CBCGPSize CalcSize(CBCGPGraphicsManager* pGM);
	virtual const CBCGPSize& GetSize() const {return m_szCellSize;}

	virtual void UpdateCellParams();

	CBCGPChartLegendEntry* GetParentLegendEntry() const {return m_pParentEntry;}
	CBCGPChartLegendVisualObject* GetParentLegend() const;
	CBCGPChartVisualObject* GetRelatedChrt() const {return m_pChart;}
	CBCGPChartSeries* GetRelatedSeries() const {return m_pSeries;}
	int	GetRelatedDataPointIndex() const {return m_nDataPointIndex;}

	void SetCustomData(CObject* pObject = NULL) {m_pCustomData = pObject;}
	CObject* GetCustomData() const {return m_pCustomData;}
	CBCGPRect GetHitRect() const {return m_rectHit;}

	const BCGPChartCellParams& GetCellParams() const {return m_cellParams;}
	void SetCellParams(const BCGPChartCellParams& params, BOOL bRedraw = TRUE);

	void SetTextFormat(const CBCGPTextFormat& format, BOOL bRedraw = TRUE);

	const CString& GetCellText() const {return m_strLabel;}
	void SetCellText(const CString& str, BOOL bRedraw = TRUE);

	void Redraw();

protected:
	void SetSize(const CBCGPSize& sz) {m_szCellSize = sz;}

	virtual CBCGPSize OnCalcCustomCellSize(CBCGPGraphicsManager* pGM);
	virtual void OnFillCellBackground(CBCGPGraphicsManager* pGM, const CBCGPRect& rectCell);
	virtual void OnDrawCustomCell(CBCGPGraphicsManager* pGM, const CBCGPRect& rectContent);

	virtual void OnMouseDown(const CBCGPPoint& /*pt*/) {}
	virtual void OnMouseUp(const CBCGPPoint& /*pt*/) {}

protected:
	BCGPChartCellParams		m_cellParams;
	LegendCellType			m_cellType;
	CBCGPChartVisualObject* m_pChart;
	CBCGPChartSeries*		m_pSeries;
	int						m_nDataPointIndex;
	CBCGPChartLegendEntry*	m_pParentEntry;

	CBCGPSize				m_szCellSize;
	CBCGPSize				m_szContentSize;
	CString					m_strLabel;
	CObject*				m_pCustomData;

	CBCGPRect				m_rectHit;
};

//****************************************************************************************
// Legend Entry
//****************************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartLegendEntry : public CObject
{
	friend class CBCGPChartLegendVisualObject;

	DECLARE_DYNCREATE(CBCGPChartLegendEntry)

protected:
	CBCGPChartLegendEntry()
	{
		m_pParentLegend = NULL;
	}

public:
	CBCGPChartLegendEntry(CBCGPChartLegendVisualObject* pLegend)
	{
		m_pParentLegend = pLegend;
	}

	virtual ~CBCGPChartLegendEntry();

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld);

	void AddLegendCell(CBCGPChartLegendCell* pCell);
	BOOL ContainsCell(CBCGPChartSeries* pSeries, int nDataPointIndex) const;

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rect);
	virtual void OnDrawGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rect);

	void AdjustLayout();
	virtual CBCGPSize CalcSize(CBCGPGraphicsManager* pGM);
	virtual const CBCGPSize& GetSize() const {return m_szEntrySize;}

	void SetOffset(const CBCGPPoint& ptOffset) {m_ptOffset = ptOffset;}
	const CBCGPPoint& GetOffset() const {return m_ptOffset;}

	int GetCellCount() const {return (int)m_lstLegendCells.GetCount();}
	CBCGPChartLegendCell* GetCell(int nIndex) const;
	double GetCellWidth(int nCellIndex) const;

	BOOL IsCustom() const;
	CBCGPChartLegendVisualObject* GetParentLegend() const {return m_pParentLegend;}

	CBCGPChartLegendCell* GetLegendCellFromPoint(const CBCGPPoint& pt) const;

protected:
	CObList		m_lstLegendCells;
	CBCGPSize	m_szEntrySize;
	CBCGPPoint	m_ptOffset;

	CBCGPChartLegendVisualObject* m_pParentLegend;
};

//****************************************************************************************
// Chart Legend
//****************************************************************************************
struct BCGCBPRODLLEXPORT BCGPChartLegendHitInfo
{
	enum LegendHitTest
	{
		LHT_NONE = 0x0,
		LHT_TITLE = 0x2,
		LHT_CELL = 0x4,
		LHT_SCROLL_UP = 0x8, // m_pCell contains a cell covered by scroll button
		LHT_SCROLL_DOWN = 0x10, // m_pCell contains a cell covered by scroll button
	};

	BCGPChartLegendHitInfo()
	{
		m_hitTest = LHT_NONE;
		m_nMouseButton = -1;
		m_pCell = NULL;
	}

	LegendHitTest			m_hitTest;
	CBCGPChartLegendCell*	m_pCell;
	CBCGPPoint				m_ptHit;
	int						m_nMouseButton;
};

class BCGCBPRODLLEXPORT CBCGPChartLegendVisualObject : public CBCGPBaseVisualObject
{
	DECLARE_DYNCREATE(CBCGPChartLegendVisualObject)
public:
	
	enum LegendAlignment
	{
		LA_NEAR,
		LA_CENTER,
		LA_FAR
	};
public:
	CBCGPChartLegendVisualObject(CBCGPVisualContainer* pContainer = NULL);

	virtual ~CBCGPChartLegendVisualObject();

	virtual CBCGPSize GetLegendSize(BOOL bWithPadding = TRUE, CBCGPGraphicsManager* pGM = NULL);

	virtual void AdjustLayout(CBCGPGraphicsManager* pGM);
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	const CBCGPRect& GetLegendBounds() const {return m_rectLegendBounds;}
	const CBCGPSize& GetTitleSize() const {return m_szTitleSize;}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);
	void UpdateLegendColors(BOOL bForceUpdate = FALSE, CBCGPChartVisualObject* pChartOrg = FALSE);

	//--- Legend entry management
	BOOL AddLegendEntry(CBCGPChartLegendEntry* pEntry);
	CBCGPChartLegendEntry* AddLegendEntry(CBCGPChartVisualObject* pChart, CBCGPChartSeries* pSeries, 
		int nDataPointIndex = -1);
	BOOL InsertLegendEntry(CBCGPChartLegendEntry* pEntry, int nIndex);
	int  GetLegendEntryCount() const {return (int)m_lstLegendEntries.GetCount();}

	CBCGPChartLegendEntry* FindLegendEntry(CBCGPChartSeries* pSeries, int nDataPointIndex = -1) const;
	CBCGPChartLegendEntry* GetLegendEntry(int nIndex) const;
	CBCGPChartLegendCell* GetLegendCell(int nEntryIndex, int nCellIndex) const;

	BOOL RemoveLegendEntry(CBCGPChartSeries* pSeries, int nDataPointIndex = -1);
	BOOL RemoveLegendEntry(CBCGPChartLegendEntry* pEntry);
	void RemoveAllEntries();
	//--- End Legend entry management

	void SetLegendCellRTC(CRuntimeClass* pRTC);
	CRuntimeClass* GetLegendCellRTC() const {return m_pLegendCellRTC;}

	//--- Related charts
	BOOL AddRelatedChart(CBCGPChartVisualObject* pChart, BOOL bRedraw = TRUE);
	BOOL RemoveRelatedChart(CBCGPChartVisualObject* pChart, BOOL bRedraw = TRUE);
	int GetRelatedChartCount() const {return (int)m_lstRelatedCharts.GetCount();}
	CBCGPChartVisualObject* GetRelatedChart(int nIndex) const;
	//--- End Related charts

	double GetColumnWidth(int nColumnIndex) const;
	int GetColumnCount() const {return (int)m_arColumnWidths.GetSize();}

	void SetVerticalLayout(BOOL bIsVertical = TRUE, BOOL bRedraw = TRUE);
	BOOL IsVerticalLayout() const {return m_bIsVertical;}

	void SetVerticalAlignment(LegendAlignment vertAlign, BOOL bRedraw = TRUE);
	void SetHorizontalAlignment(LegendAlignment horzAlign, BOOL bRedraw = TRUE);

	LegendAlignment GetVerticalAlignment() const {return m_vertAlignment;}
	LegendAlignment GetHorizontalAlignment() const {return m_horzAlignment;}

	void InvalidateLegendContent(BOOL bRedraw);
	void InvalidateLegendSize(BOOL bRedraw);

	//--- Title
	void SetLegendTitleFormat(const BCGPChartFormatLabel& titleFormat, BOOL bRedraw = TRUE);
	void ShowLegendTitle(BOOL bShow, const CString& strTitle = _T(""), BOOL bRedraw = TRUE);
	BOOL IsLegendTitleVisible() const {return m_legendStyle.m_bShowTitle;}
	const CString& GetLegendTitle() const {return m_strLegendTitle;}
	void SetAdjustLegendSizeByTitleSize(BOOL bSet = TRUE, BOOL bRedraw = TRUE);
	BOOL IsAdjustLegendSizeByTitleSize() const {return m_legendStyle.m_bAdjustLegendSizeByTitleSize;}
	//--- End Title

	void ShowChartNameInLegend(BOOL bShow = TRUE, BOOL bRedraw = TRUE);
	BOOL IsChartNameInLegendVisible() const {return m_legendStyle.m_bShowChartName;}

	// hit testing support
	CBCGPChartLegendCell* GetLegendCellFromPoint(const CBCGPPoint& pt) const;
	virtual BOOL HitTest(const CBCGPPoint& pt, BCGPChartLegendHitInfo* pHitInfo) const;
	virtual BOOL FireMouseMessage(UINT nMsg, int nButton, const CBCGPPoint& pt, BOOL bUseLastHit = FALSE);

	// scroll support
	virtual BOOL OnMouseDown(int nButton, const CBCGPPoint& pt);
 	virtual void OnMouseUp(int nButton, const CBCGPPoint& pt);
 	virtual void OnCancelMode();
	virtual void OnClickAndHoldEvent(UINT nID, const CBCGPPoint& point); 
	virtual BOOL OnScroll(const CBCGPPoint& pt);

	void SetScrollStep(double dblStep) {m_dblScrollStep = dblStep;}
	double GetScrollStep() const {return m_dblScrollStep;}

	virtual void SetRect(const CBCGPRect& rect, BOOL bRedraw = FALSE);

public:
	BCGPChartFormatLegendTable	m_legendStyle;

protected:
	CObList						m_lstLegendEntries;
	CBCGPSize					m_szLegendSize;
	CBCGPSize					m_szTitleSize;
	CBCGPRect					m_rectLegendBounds;
	BOOL						m_bIsVertical;

	CString						m_strLegendTitle;

	BOOL						m_bIsContentUpdated;
	BOOL						m_bIsSizeValid;

	BCGPChartLegendHitInfo		m_lastHit;

	// alignment
	LegendAlignment				m_horzAlignment;
	LegendAlignment				m_vertAlignment;

	// related charts
	CObList						m_lstRelatedCharts;

	// adjust entry size by max entry size, or calculate size separately
	BOOL						m_bIsVariableEntrySize;

	CRuntimeClass*				m_pLegendCellRTC;
	int							m_nLegendID;

	CArray<double, double>		m_arColumnWidths;
	double						m_dblMaxRowHeight;

	// scrolling support
	CBCGPLegendScrollButton		m_btnLeftTop;
	CBCGPLegendScrollButton		m_btnRightBottom;	
	
	double						m_dblScrollOffset;
	double						m_dblScrollStep;

	BOOL						m_bInsideOnDraw; 

	virtual CBCGPChartLegendCell* OnCreateLegendCell(CBCGPChartVisualObject* pChart, 
		CBCGPChartSeries* pSeries, int nDataPointIndex, CBCGPChartLegendCell::LegendCellType type);
	virtual CBCGPChartLegendEntry* OnCreateLegendEntry();
	virtual double OnCalcScrollStep();

	virtual void UpdateLegendContent();
	void AdjustScrollPos(CBCGPGraphicsManager* pGM);
	void AdjustScrollButtonPos(CBCGPGraphicsManager* pGM);
	void AdjustLegendBounds(CBCGPGraphicsManager* pGM);

protected:
	virtual CBCGPSize CalcSize(CBCGPGraphicsManager* pGM);
	virtual CBCGPSize OnCalcTitleSize(CBCGPGraphicsManager* pGM);
	virtual void OnLegendContentCreated();

	virtual void OnFillLegendControlBackground(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBounds);
	virtual void OnFillLegendBackground(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBounds);
	virtual void OnDrawLegendTitle(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBounds);
	virtual void OnDrawLegendBorders(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBorders);
	virtual void OnDrawLegendEntries(CBCGPGraphicsManager* pGM);
	virtual void OnDrawTitleGridLine(CBCGPGraphicsManager* pGM, const CBCGPRect& rectTitle);
	virtual void OnDrawLegendGridLines(CBCGPGraphicsManager* pGM);

	virtual void OnAfterLegendDraw(CBCGPGraphicsManager* pGM, DWORD dwFlags);
	virtual BOOL OnAdjustLegendBounds(const CBCGPSize& szLegendSize, const CBCGPRect& rectControl, 
		CBCGPRect& rectLegendBounds) 
	{
		UNREFERENCED_PARAMETER(szLegendSize);
		UNREFERENCED_PARAMETER(rectControl);
		UNREFERENCED_PARAMETER(rectLegendBounds);
		return FALSE;
	}
};

#define SetVericalLayout SetVerticalLayout
#define m_bDrawVericalGridLines m_bDrawVerticalGridLines

#endif // !defined(AFX_BCGPCHARTLEGEND_H__1875ABCF_5225_490C_BBD3_9B6CFB20ADC0__INCLUDED_)
