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

#if !defined(AFX_BCGPLAYOUT_H__C9173C62_9849_4E89_9782_417F5FC99D30__INCLUDED_)
#define AFX_BCGPLAYOUT_H__C9173C62_9849_4E89_9782_417F5FC99D30__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

class BCGCBPRODLLEXPORT CBCGPControlsLayout: public CObject
{
	DECLARE_DYNAMIC(CBCGPControlsLayout)

protected:
	CBCGPControlsLayout();

public:
	virtual ~CBCGPControlsLayout();

public:
	virtual BOOL Create(CWnd* pHostWnd) = 0;
	virtual BOOL AddLayout(CBCGPControlsLayout* pLayout) = 0;
	virtual void AdjustLayout() = 0;

	CWnd* GetHostWnd ()
	{
		return m_pHostWnd;
	}
	virtual void GetHostWndRect (CRect& rect) const;

	CBCGPControlsLayout* GetHostLayout ()
	{
		return m_pHostLayout;
	}

	virtual CSize GetMinSize() const
	{
		return m_MinSize;
	}
	virtual void SetMinSize(const CSize& size)
	{
		m_MinSize = size;
	}

protected:
	CWnd*					m_pHostWnd;
	CBCGPControlsLayout*	m_pHostLayout;
	CSize					m_MinSize;
};

class BCGCBPRODLLEXPORT CBCGPStaticLayout: public CBCGPControlsLayout
{
	DECLARE_DYNCREATE(CBCGPStaticLayout)

public:
	enum XMoveType
	{
		e_MoveTypeNone = 0,
		e_MoveTypeHorz = 1,
		e_MoveTypeVert = 2,
		e_MoveTypeBoth = 3
	};

	enum XSizeType
	{
		e_SizeTypeNone = 0,
		e_SizeTypeHorz = 1,
		e_SizeTypeVert = 2,
		e_SizeTypeBoth = 3
	};

	struct XWndItem
	{
		struct XPoint
		{
			XPoint()
				: x(0.0)
				, y(0.0)
			{

			}

			double x;
			double y;
		};

		XWndItem()
			: m_Handle     (NULL)
			, m_typeMove (e_MoveTypeNone)
			, m_typeSize (e_SizeTypeNone)
			, m_percMove (100, 100)
			, m_percSize (100, 100)
		{
		}

		LPVOID		m_Handle;
		XPoint		m_initPoint;
		XPoint		m_initSize;
		XMoveType	m_typeMove;
		XSizeType	m_typeSize;
		CPoint		m_percMove;
		CPoint		m_percSize;
	};

public:
	CBCGPStaticLayout();
	virtual ~CBCGPStaticLayout();

	static BOOL m_bDontCheckFormViewInitDlg;

	virtual BOOL Create(CWnd* pHostWnd);
	virtual BOOL AddLayout(CBCGPControlsLayout* /*pLayout*/) { return FALSE;}
	virtual void AdjustLayout();

	BOOL AddAnchor (UINT nID, XMoveType typeMove, XSizeType typeSize, 
					const CPoint& percMove = CPoint (100, 100), const CPoint& percSize = CPoint (100, 100));
	BOOL AddAnchor (HWND hWnd, XMoveType typeMove, XSizeType typeSize, 
					const CPoint& percMove = CPoint (100, 100), const CPoint& percSize = CPoint (100, 100));

protected:
	BOOL AddAnchor (LPVOID lpHandle, XMoveType typeMove, XSizeType typeSize, 
					const CPoint& percMove = CPoint (100, 100), const CPoint& percSize = CPoint (100, 100));
	BOOL FindItem (LPVOID lpHandle, XWndItem& item);

	virtual void CorrectItem (XWndItem& item) const;
	virtual BOOL CollectItem (XWndItem& item) const;
	virtual CRect GetItemRect(XWndItem& item) const;
	virtual UINT CalculateItem (XWndItem& item, CRect& rectItem) const;

protected:
	CList<XWndItem, XWndItem&> m_listWnd;
};

#endif // !defined(AFX_BCGPLAYOUT_H__C9173C62_9849_4E89_9782_417F5FC99D30__INCLUDED_)
