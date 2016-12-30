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
// BCGPTreeMap.h: interface for the CBCGPTreeMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTREEMAP_H__BCBF4D06_1F59_4372_93E6_542BA8CCDCFB__INCLUDED_)
#define AFX_BCGPTREEMAP_H__BCBF4D06_1F59_4372_93E6_542BA8CCDCFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPVisualContainer.h"
#include "BCGPVisualCtrl.h"

class CBCGPTreeMapNode;
class CBCGPTreeMap;

extern BCGCBPRODLLEXPORT UINT BCGM_ON_CLICK_TREEMAP_NODE;

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseTreeMapNode

class BCGCBPRODLLEXPORT CBCGPBaseTreeMapNode : public CObject
{
	DECLARE_DYNAMIC(CBCGPBaseTreeMapNode)

	friend class CBCGPTreeMapGroup;

// Construction
protected:
	CBCGPBaseTreeMapNode(LPCTSTR lpszLabel = NULL)
	{
		m_dblVal = 0.0;
		m_strLabel = (lpszLabel == NULL) ? _T("") : lpszLabel;
		m_pParent = NULL;
		m_pOwner = NULL;
		m_sizeMargin = CBCGPSize(-1., -1.);
	}

public:
	virtual ~CBCGPBaseTreeMapNode()
	{
	}

// Attributes:
public:
	double GetValue() const
	{
		return m_dblVal;
	}

	const CString& GetLabel() const
	{
		return m_strLabel;
	}

	const CBCGPRect& GetRect() const
	{
		return m_rect;
	}

// Overrides:
protected:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, const CBCGPBrush& brBorder) = 0;
	virtual void RecalcLayout(CBCGPGraphicsManager* /*pGM*/) {}
	virtual void SetFontScale(double /*dblScale*/) {}

// Operations:
public:
	virtual void SetRect(const CBCGPRect& rect)
	{
		m_rect = rect;
	}

	virtual const CBCGPBaseTreeMapNode* HitTest(const CBCGPPoint& pt) const
	{
		return m_rect.PtInRect(pt) ? this : NULL;
	}

protected:
	void DrawTextWidthShadow(CBCGPGraphicsManager* pGM, const CString& str, const CBCGPRect& rect, const CBCGPBrush& br, const CBCGPTextFormat& tf);

protected:
	double					m_dblVal;
	CString					m_strLabel;
	CBCGPRect				m_rect;
	CBCGPBaseTreeMapNode*	m_pParent;
	CBCGPTreeMap*			m_pOwner;
	CBCGPBrush				m_brFill;
	CBCGPSize				m_sizeMargin;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeMapGroup

class BCGCBPRODLLEXPORT CBCGPTreeMapGroup : public CBCGPBaseTreeMapNode
{
	DECLARE_DYNAMIC(CBCGPTreeMapGroup)

	friend class CBCGPTreeMap;
	friend class CBCGPTreeMapNode;

// Construction
public:
	CBCGPTreeMapGroup(const CBCGPBrush& brushFill, LPCTSTR lpszLabel = NULL, const CBCGPColor& colorText = CBCGPColor::White, CBCGPTextFormat* pTF = NULL);
	virtual ~CBCGPTreeMapGroup();

protected:
	CBCGPTreeMapGroup();

// Attributes:
public:
	const CBCGPSize& GetMargin() const
	{
		return m_sizeMargin;
	}

	void SetMargin(const CBCGPSize& sizeMargin)
	{
		m_sizeMargin.cx = max(0., sizeMargin.cx);
		m_sizeMargin.cy = max(0., sizeMargin.cy);
	}

// Operations:
public:
	void AddSubNode(CBCGPBaseTreeMapNode* pNode);
	void RemoveAll();

	const CArray<CBCGPBaseTreeMapNode*, CBCGPBaseTreeMapNode*>& GetSubNodes() const
	{
		return m_arSubNodes;
	}

// Overrides:
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, const CBCGPBrush& brBorder);
	virtual const CBCGPBaseTreeMapNode* HitTest(const CBCGPPoint& pt) const;
	virtual void RecalcLayout(CBCGPGraphicsManager* pGM);

protected:
	void RecalcSliceLayout(CBCGPGraphicsManager* pGM, int nStart, int nFinish, const CBCGPRect& rect, BOOL bIsVertical);
	void RecalcSquarifiedLayout(CBCGPGraphicsManager* pGM, int nStart, int nFinish, const CBCGPRect& rect);

	double GetChildrenTotal(int nStart, int nFinish);

	void InitTextFormat(CBCGPTextFormat* pTF);

	virtual void SetFontScale(double dblScale);
	virtual void SetRect(const CBCGPRect& rect);

protected:
	CArray<CBCGPBaseTreeMapNode*, CBCGPBaseTreeMapNode*>	m_arSubNodes;
	CBCGPBrush												m_brText;
	CBCGPTextFormat											m_tf;
	CBCGPRect												m_rectCaption;
	CBCGPBrush												m_brFillCaption;
	BOOL													m_bIsRoot;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeMapNode

class BCGCBPRODLLEXPORT CBCGPTreeMapNode : public CBCGPBaseTreeMapNode
{
	DECLARE_DYNAMIC(CBCGPTreeMapNode)

public:
	CBCGPTreeMapNode(double dblVal, LPCTSTR lpszLabel = NULL);
	virtual ~CBCGPTreeMapNode();

public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, const CBCGPBrush& brBorder);
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeMap

class BCGCBPRODLLEXPORT CBCGPTreeMap  : public CBCGPBaseVisualObject
{
	DECLARE_DYNAMIC(CBCGPTreeMap)

public:
	enum LayoutType
	{
		LayoutType_First = 0,
		Slice            = LayoutType_First,
		Squarified       = 1,
		LayoutType_Last  = Squarified
	};

// Construction
public:
	CBCGPTreeMap();
	virtual ~CBCGPTreeMap();

// Attributes:
public:
	LayoutType GetLayoutType() const
	{
		return m_LayoutType;
	}

	void SetLayoutType(LayoutType layoutType);

	const CBCGPBrush& GetFillBrush() const
	{
		return m_brFill;
	}

	void SetFillBrush(const CBCGPBrush& br);

	const CBCGPSize& GetGroupMargin() const
	{
		return m_Root.GetMargin();
	}

	void SetGroupMargin(const CBCGPSize& sizeMargin);

	const CBCGPTreeMapGroup& GetRootGroup() const
	{
		return m_Root;
	}

// Operations:
public:
	void AddGroup(CBCGPTreeMapGroup* pGroup);
	void RemoveAll();

	const CBCGPBaseTreeMapNode* HitTestTreeMapNode(const CBCGPPoint& pt) const;

// Overrides:
	virtual void OnClickTreeMapNode(const CBCGPBaseTreeMapNode* pClickedNode);

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual BOOL OnMouseDown(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseUp(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseLeave();
	virtual void OnCancelMode();
	virtual BOOL OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr);
	virtual void SetRect(const CBCGPRect& rect, BOOL bRedraw = FALSE)
	{
		m_Root.SetRect(rect);
		SetDirty();
		CBCGPBaseVisualObject::SetRect(rect, bRedraw);
	}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);

protected:
	CBCGPTreeMapGroup			m_Root;
	CBCGPBrush					m_brFill;
	LayoutType					m_LayoutType;
	const CBCGPBaseTreeMapNode*	m_pClicked;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeMapCtrl window

class BCGCBPRODLLEXPORT CBCGPTreeMapCtrl : public CBCGPVisualCtrl
{
	DECLARE_DYNAMIC(CBCGPTreeMapCtrl)

// Construction
public:
	CBCGPTreeMapCtrl();

// Attributes
public:
	virtual CBCGPTreeMap* GetTreeMap() 
	{ 
		if (m_pTreeMap == NULL)
		{
			m_pTreeMap = new CBCGPTreeMap();
		}

		return m_pTreeMap;
	}

// Operations
public:

// Overrides
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetTreeMap();
	}

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPTreeMapCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPTreeMapCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPTreeMapCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPTreeMap* m_pTreeMap;
};

#endif // !defined(AFX_BCGPTREEMAP_H__BCBF4D06_1F59_4372_93E6_542BA8CCDCFB__INCLUDED_)
