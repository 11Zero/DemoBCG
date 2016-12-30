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

#if !defined(AFX_KEYASSIGN_H__283E6046_54C6_11D2_B110_D085EB8D1B3C__INCLUDED_)
#define AFX_KEYASSIGN_H__283E6046_54C6_11D2_B110_D085EB8D1B3C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// KeyAssign.h : header file
//

#include "BCGCBPro.h"
#include "BCGPKeyHelper.h"

/////////////////////////////////////////////////////////////////////////////
// CKeyAssign window

class BCGCBPRODLLEXPORT CKeyAssign : public CEdit
{
// Construction
public:
	CKeyAssign();

// Attributes
public:
	BOOL IsKeyDefined () const
	{
		return m_bIsDefined;
	}

	BOOL IsFocused () const
	{
		return m_bIsFocused;
	}

	ACCEL const* GetAccel () const
	{
		return &m_Accel;
	}

protected:
	BOOL			m_bIsDefined;
	ACCEL			m_Accel;
	CBCGPKeyHelper	m_Helper;
	BOOL			m_bIsFocused;

// Operations
public:
	void ResetKey ();

protected:
	void SetAccelFlag (BYTE bFlag, BOOL bOn);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyAssign)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CKeyAssign();

	// Generated message map functions
protected:
	//{{AFX_MSG(CKeyAssign)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYASSIGN_H__283E6046_54C6_11D2_B110_D085EB8D1B3C__INCLUDED_)
