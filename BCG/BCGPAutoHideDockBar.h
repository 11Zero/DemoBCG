#if !defined(AFX_BCGPAutoHideDOCKBAR_H__620B757C_CEC2_4DFF_B715_C94454393938__INCLUDED_)
#define AFX_BCGPAutoHideDOCKBAR_H__620B757C_CEC2_4DFF_B715_C94454393938__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPAutoHideDockBar.h : header file
//

#include "BCGCBPro.h"
#include "BCGPDockBar.h"

class CBCGPAutoHideToolBar;

/////////////////////////////////////////////////////////////////////////////
// CBCGPAutoHideDockBar window

class BCGCBPRODLLEXPORT CBCGPAutoHideDockBar : public CBCGPDockBar
{
	DECLARE_DYNCREATE(CBCGPAutoHideDockBar)
// Construction
public:
	CBCGPAutoHideDockBar();

// Attributes
public:
	void SetOffsetLeft  (int nOffset) {m_nOffsetLeft = nOffset;}
	void SetOffsetRight (int nOffset) {m_nOffsetRight = nOffset;}

	void GetAlignRect (CRect& rect) const;
	virtual BOOL CanAcceptBar (const CBCGPBaseControlBar* pBar) const;
// Operations
public:
	virtual void DockControlBar	(CBCGPControlBar* pWnd, BCGP_DOCK_METHOD dockMethod, LPCRECT lpRect = NULL);
	virtual void RepositionBars	(CRect& rectNewClientArea);

	void UnSetAutoHideMode (CBCGPAutoHideToolBar* pAutohideToolbar);

	virtual BOOL AllowShowOnControlMenu () const			{	return FALSE;	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPAutoHideDockBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPAutoHideDockBar();
 
	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPAutoHideDockBar)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int m_nOffsetLeft;
	int m_nOffsetRight;

	static int m_nExtraSpace;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPAutoHideDOCKBAR_H__620B757C_CEC2_4DFF_B715_C94454393938__INCLUDED_)
