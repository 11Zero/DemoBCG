//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPTagCloud.h: interface for the CBCGPTagCloud class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTAGCLOUD_H__7E085052_5727_4AF2_B925_312BBDC7ACA8__INCLUDED_)
#define AFX_BCGPTAGCLOUD_H__7E085052_5727_4AF2_B925_312BBDC7ACA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPVisualContainer.h"
#include "BCGPVisualCtrl.h"

class CBCGPTagCloud;

extern BCGCBPRODLLEXPORT UINT BCGM_ON_CLICK_TAG_CLOUD;

/////////////////////////////////////////////////////////////////////////////
// CBCGPTagCloudElement

class BCGCBPRODLLEXPORT CBCGPTagCloudElement : public CObject
{
	friend class CBCGPTagCloud;

// Construction
public:
	CBCGPTagCloudElement(const CString& strLabel, double dblValue, 
		const CBCGPColor& colorText = CBCGPColor(),
		const CBCGPBrush& brushBackground = CBCGPBrush(),
		const CBCGPColor& colorBorder = CBCGPColor());
	CBCGPTagCloudElement();

protected:
	void CommonInit();

// Attributes
public:
	double GetValue() const { return m_dblValue; }
	void SetValue(double value);

	const CString& GetLabel() const { return m_strLabel; }
	void SetLabel(const CString& label);

	const CBCGPColor& GetTextColor() const { return m_colorText; }
	void SetTextColor(const CBCGPColor& color) { m_colorText = color; }

	const CBCGPColor& GetHighlightedColor() const { return m_colorHighlightedText; }
	void SetHighlightedColor(const CBCGPColor& color) { m_colorHighlightedText = color; }

	const CBCGPBrush& GetBackgroundBrush() const { return m_brushBackground; }
	void SetBackgroundBrush(const CBCGPBrush& brush) { m_brushBackground = brush; }

	const CBCGPColor& GetBorderColor() const { return m_colorBorder; }
	void SetBorderColor(const CBCGPColor& color) { m_colorBorder = color; }

	int	GetWeight() const { return m_nWeight; }
	const CBCGPRect& GetRect() const { return m_rect; }

	BOOL IsSelected() const;

// Operation:
public:
	void Redraw();

// Overrides
public:
	virtual void DoDraw(CBCGPTagCloud* pTagCloud, CBCGPGraphicsManager* pGM, CBCGPTextFormat& tf, BOOL bIsPressed, BOOL bIsHighlighted);

protected:
	CBCGPTagCloud*	m_pOwner;
	double			m_dblValue;
	int				m_nWeight;
	CString			m_strLabel;
	CBCGPColor		m_colorText;
	CBCGPColor		m_colorHighlightedText;
	CBCGPBrush		m_brushBackground;
	CBCGPColor		m_colorBorder;
	CBCGPRect		m_rect;
	double			m_dblTextHeight;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPTagCloud

class BCGCBPRODLLEXPORT CBCGPTagCloud : public CBCGPBaseVisualObject  
{
	friend class CBCGPTagCloudElement;
	
	DECLARE_DYNAMIC(CBCGPTagCloud)

public:
	enum SortOrder
	{
		SortOrder_First  = 0,
		NoSort           = SortOrder_First,
		AlphabeticalSort = 1,
		SortByValue      = 2,
		SortOrder_Last   = SortByValue
	};

// Construction
public:
	CBCGPTagCloud();
	virtual ~CBCGPTagCloud();

// Attributes
public:
	SortOrder GetSortOrder() const { return m_SortOrder; }
	void SetSortOrder(SortOrder nOrder);

	BOOL IsSortDescending() const { return m_bDescending; }
	void SetSortDescending(BOOL bSet = TRUE);

	int GetMaxWeight() const { return m_nMaxWeight; }
	void SetMaxWeight(int nMaxWeight);

	const CBCGPTextFormat& GetTextFormat() const { return m_tfBase; }
	void SetTextFormat(const CBCGPTextFormat& tf);

	double GetFontSizeStep() const { return m_dblFontSizeStep; }
	void SetFontSizeStep(double dblStep);

	void SetHorzMargin(double nMargin);
	void SetVertMargin(double nMargin);

	double GetHorzMargin() const { return m_nHorzMargin; }
	double GetVertMargin() const { return m_nVertMargin; }

	const CBCGPBrush& GetFillBrush() const { return m_brFill; }
	void SetFillBrush(const CBCGPBrush& brFill);

	const CBCGPColor& GetTextColor() const { return m_clrText; }
	void SetTextColor(const CBCGPColor& clrText);

	const CBCGPColor& GetHighlightedTextColor() const { return m_clrHighlightedText; }
	void SetHighlightedTextColor(const CBCGPColor& clrHighlightedText);

// Operations
public:
	void Add(CBCGPTagCloudElement* pTag);
	void RemoveAll();

	const CList<CBCGPTagCloudElement*, CBCGPTagCloudElement*>& GetTagList() const { return m_lstTags; }

	CBCGPTagCloudElement* HitTestTag(const CBCGPPoint& point) const;

// Overrides
protected:
	virtual void OnClickTag(CBCGPTagCloudElement* pClickedTag);

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual BOOL OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL OnMouseDown(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseUp(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseMove(const CBCGPPoint& pt);
	virtual void OnMouseLeave();
	virtual BOOL OnSetMouseCursor(const CBCGPPoint& pt);
	virtual void OnCancelMode();
	virtual BOOL OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr);
	virtual void SetRect(const CBCGPRect& rect, BOOL bRedraw = FALSE)
	{
		SetDirty();
		CBCGPBaseVisualObject::SetRect(rect, bRedraw);
	}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);

	void RecalcWeights();
	void AdjustRowHeight(CArray<CBCGPTagCloudElement*, CBCGPTagCloudElement*>& arRow, double nRowHeight);
	void RecalcLayout(CBCGPGraphicsManager* pGM);
	void RedrawTag(CBCGPTagCloudElement* pTag);
	void RebuildTextFormats();
	void AddSorted(CBCGPTagCloudElement* pTag);

	// MSAA support:	
	CBCGPTagCloudElement* GetAccChild(int nIndex);
	long GetAccChildIndex(CBCGPTagCloudElement* pTile);
	
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);
	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);
	
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);

// Attributes
protected:
	SortOrder											m_SortOrder;
	BOOL												m_bDescending;
	int													m_nMaxWeight;
	BOOL												m_bCalcWeights;
	BOOL												m_bSortTags;

	double												m_dblMinVal;
	double												m_dblMaxVal;

	CList<CBCGPTagCloudElement*, CBCGPTagCloudElement*>	m_lstTags;
	CList<CBCGPTagCloudElement*, CBCGPTagCloudElement*>	m_lstTagsSorted;
	CArray<CBCGPTextFormat, const CBCGPTextFormat&>		m_Fonts;
	CBCGPTextFormat										m_tfBase;
	double												m_dblFontSizeStep;
	CBCGPTagCloudElement*								m_pHighlighted;
	CBCGPTagCloudElement*								m_pPressed;
	CBCGPTagCloudElement*								m_pSelected;

	double												m_nHorzMargin;
	double												m_nVertMargin;

	double												m_nHorzMarginOriginal;
	double												m_nVertMarginOriginal;

	CBCGPBrush											m_brFill;
	CBCGPColor											m_clrText;
	CBCGPColor											m_clrHighlightedText;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPTagCloudCtrl window

class BCGCBPRODLLEXPORT CBCGPTagCloudCtrl : public CBCGPVisualCtrl
{
// Construction
public:
	CBCGPTagCloudCtrl();

// Attributes
public:
	virtual CBCGPTagCloud* GetTagCloud() 
	{ 
		if (m_pCloud == NULL)
		{
			m_pCloud = new CBCGPTagCloud();
		}

		return m_pCloud;
	}

// Operations
public:

// Overrides
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetTagCloud();
	}

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPTagCloudCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPTagCloudCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPTagCloudCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPTagCloud* m_pCloud;
};

#endif // !defined(AFX_BCGPTAGCLOUD_H__7E085052_5727_4AF2_B925_312BBDC7ACA8__INCLUDED_)
