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

// BCGPDockBarRow.h: interface for the CBCGPDockBarRow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDOCKBARROW_H__233112C0_FD5E_48EF_AF12_941920CCFBE3__INCLUDED_)
#define AFX_BCGPDOCKBARROW_H__233112C0_FD5E_48EF_AF12_941920CCFBE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

class CBCGPDockBar;
class CBCGPControlBar;

/////////////////////////////////////////////////////////////////////////////
// CBCGPDockBar window

enum BCGP_ROW_ALIGNMENT
{
	BCGP_ROW_ALIGN_TOP = 0,
	BCGP_ROW_ALIGN_BOTTOM = 1
};

class BCGCBPRODLLEXPORT CBCGPDockBarRow : public CObject
{
	friend class CBCGPDockManager;
	friend class CBCGPToolBar;
	DECLARE_DYNAMIC(CBCGPDockBarRow);
public:
	CBCGPDockBarRow (CBCGPDockBar* pParentDockBar, int nOffset, int nHeight);
	virtual ~CBCGPDockBarRow ();

	virtual BOOL Create ()
	{
		return TRUE;
	}

	virtual void GetAvailableSpace (CRect& rect);
	virtual int  GetVisibleCount ();

	virtual CSize CalcFixedLayout (BOOL bStretch, BOOL bHorz);

	virtual void AddControlBar (CBCGPControlBar* pControlBar, BCGP_DOCK_METHOD dockMethod, LPCRECT lpRect = NULL, BOOL bAddLast = FALSE); 
	virtual void AddControlBarFromRow (CBCGPControlBar* pControlBar, BCGP_DOCK_METHOD dockMethod); 
	virtual void RemoveControlBar (CBCGPControlBar* pControlBar); 

	virtual void ArrangeControlBars (int nMargin, int nSpacing);

	virtual int  ResizeBySlider (int /*nOffset*/) {return 0;} // can'te be resized by slider
	virtual int  Resize (int nOffset);
	virtual void Move	(int nOffset);

	virtual void ArrangeBars (CBCGPControlBar* pInitialBar);
	virtual void RepositionBars (CRect& rectNewParentBarArea, UINT nSide = (UINT)-1, BOOL bExpand = FALSE, int nOffset = 0);
	virtual BOOL IsEmpty () const {return m_lstControlBars.IsEmpty ();}

	virtual void UpdateVisibleState (BOOL bDelay);
	virtual void ShowDockBarRow (BOOL bShow, BOOL bDelay);
	
	void	MoveControlBar (CBCGPControlBar* pControlBar, CPoint ptOffset, BOOL bSwapControlBars, 
							HDWP& hdwp);
	void	MoveControlBar (CBCGPControlBar* pControlBar, CRect rectTarget, HDWP& hdwp);
	void	MoveControlBar (CBCGPControlBar* pControlBar, int nOffset, bool bForward, HDWP& hdwp);
	void	MoveControlBar (CBCGPControlBar* pControlBar, int nAbsolutOffset, HDWP& hdwp);

	void	RedrawAll ();

	BOOL	HasControlBar (CBCGPBaseControlBar* pControlBar) 
	{
		return m_lstControlBars.Find (pControlBar) != NULL;
	}

	virtual BOOL ReplaceControlBar (CBCGPControlBar* pBarOld, CBCGPControlBar* pBarNew);
	virtual BOOL ShowControlBar (CBCGPControlBar* pControlBar, BOOL bShow, BOOL bDelay = FALSE);

	virtual void OnResizeControlBar (CBCGPBaseControlBar* pControlBar);

	void	FixupVirtualRects (bool bMoveBackToVirtualRect, CBCGPControlBar* pBarToExclude = NULL);
	void	ExpandStretchedBars ();
	void	ExpandStretchedBarsRect ();

	virtual BOOL IsVisible () const {return m_bVisible;}

	int		GetRowHeight () const {return m_nRowHeight;}
	int		GetRowOffset () const {return m_nRowOffset;}

	int		GetBarCount () const {return (int) m_lstControlBars.GetCount ();}
	void	GetWindowRect (CRect& rect) const;
	void	GetClientRect (CRect& rect) const;
	void	ScreenToClient (CRect& rect) const;

	bool	IsHorizontal () const {return (m_dwRowAlignment & CBRS_ALIGN_TOP || m_dwRowAlignment & CBRS_ALIGN_BOTTOM);}

	DWORD	GetRowAlignment () const {return m_dwRowAlignment;}

	int		GetID () const {return 0;}

	CBCGPDockBar* GetDockBar () const {return m_pParentDockBar;}

	const CObList& GetControlBarList() const { return m_lstControlBars; }

	virtual int GetAvailableLength (BOOL bUseVirtualRect = FALSE) const;

	void SetExtra (int nExtraSpace, BCGP_ROW_ALIGNMENT rowExtraAlign)
	{
		m_nExtraSpace = nExtraSpace;
		m_nExtraAlignment = rowExtraAlign;
	}

	int GetExtraSpace () const {return m_nExtraSpace;}

	// exclusive row does not allows more than one bar that does allow sibling bars
	virtual BOOL IsExclusiveRow () const;

	void GetGroupFromBar (CBCGPControlBar* pBar, CObList& lst);
	int  GetMaxBarSize (BOOL bSkipHiddenBars = TRUE) const;

	BOOL			m_bIgnoreBarVisibility;

protected:
	BOOL			m_bVisible;
	int				m_nRowHeight;
	int				m_nRowSavedHeight;
	int				m_nMinHeight;
	int				m_nRowOffset;
	CObList			m_lstControlBars;
	DWORD			m_dwRowAlignment;
	CBCGPDockBar*	m_pParentDockBar;

	int				m_nExtraSpace; // specifies how the row is bigger than its bars
	int				m_nExtraAlignment; // how the bars are aligned if extra space

	BOOL CheckControlBars (CRect& rectCurrentBar, CBCGPControlBar* pCurrentBar, bool bForward, 
							CPoint ptOffset, BOOL bSwapControlBars, HDWP& hdwp);
	
	CBCGPControlBar* FindFirstVisibleBar (BOOL bForward);
	

protected:
	virtual void OnInsertControlBar (CBCGPControlBar* pControlBar);
	void AdjustBarToRowArea (CBCGPControlBar* pBar, const CRect& rectRow, HDWP& hdwp);
	int CalcLastBarOffset ();

	void BeginTrans ();
	void CommitTrans ();
	int GetAvailableLengthRect ();
	int StretchBarRect (CBCGPControlBar* pBar, int nStretchSize);
	void ArrangeBarsRect (CBCGPControlBar* pInitialBar);
	void ResolveIntersectionRect (CBCGPControlBar* pBar, bool bForward);
	int GetOutOfBoundsOffsetRect (CBCGPControlBar* pBar, BOOL bLeftTopBound);
	void ShiftControlBarsRect (CBCGPControlBar* pControlBar, int nOffset, BOOL bForward);
	void MoveControlBarRect (CBCGPControlBar* pControlBar, int nOffset, bool bForward);
	
private:
	void OffsetFromRect (const CRect& rect, CPoint& pt, bool bForward);
	BOOL IsEnoughSpaceToMove (CBCGPControlBar* pControlBar, bool bForward, int& nAllowedOffset);
	void MoveTrailingBars (POSITION posStart, CPoint ptOffset, bool bForward, CBCGPControlBar* pBarToSkip, HDWP& hdwp);
	void ResolveIntersection (CBCGPControlBar* pBar, bool bForward, HDWP& hdwp);
	int  GetOutOfBoundsOffset (CBCGPControlBar* pBar, BOOL bLeftTopBound);
	void ShiftControlBars (CBCGPControlBar* pControlBar, int nOffset, BOOL bForward);
};

#endif // !defined(AFX_BCGPDOCKBARROW_H__233112C0_FD5E_48EF_AF12_941920CCFBE3__INCLUDED_)
