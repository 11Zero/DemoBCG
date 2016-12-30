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

// BCGPKeyboardManager.h: interface for the CBCGPKeyboardManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPKEYBOARDMANAGER_H__916B7A45_556A_11D2_B110_FD3F2C81B152__INCLUDED_)
#define AFX_BCGPKEYBOARDMANAGER_H__916B7A45_556A_11D2_B110_FD3F2C81B152__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"

static CString strDelimiter = _T("; ");

class BCGCBPRODLLEXPORT CBCGPKeyboardManager : public CObject
{
	friend class _KBD_TERM;

// Constructor/Destructor:
public:
	CBCGPKeyboardManager();
	virtual ~CBCGPKeyboardManager();

// Operattion:
public:
	BOOL LoadState (LPCTSTR lpszProfileName = NULL, CFrameWnd* pDefaultFrame = NULL);
	BOOL SaveState (LPCTSTR lpszProfileName = NULL, CFrameWnd* pDefaultFrame = NULL);

	BOOL UpdateAcellTable (CMultiDocTemplate* pTemplate, LPACCEL lpAccel, int nSize,
		CFrameWnd* pDefaultFrame = NULL);
	BOOL UpdateAcellTable (CMultiDocTemplate* pTemplate, HACCEL hAccelNew, CFrameWnd* pDefaultFrame = NULL);

	virtual void ResetAll ();

	static BOOL FindDefaultAccelerator (UINT uiCmd, CString& str, CFrameWnd* pWndFrame,
										BOOL bIsDefaultFrame);
	static BOOL IsKeyPrintable (const UINT nChar);
	static UINT TranslateCharToUpper (const UINT nChar);
	static BOOL IsKeyHandled (WORD nKey, BYTE fVirt, CFrameWnd* pWndFrame, BOOL bIsDefaultFrame);

	static void ShowAllAccelerators (BOOL bShowAll = TRUE, LPCTSTR lpszDelimiter = strDelimiter);
	static BOOL IsShowAllAccelerators ()
	{
		return m_bAllAccelerators;
	}

	static void AllowReassign(BOOL bAllow = TRUE);
	static BOOL IsReassignAllowed()
	{
		return m_bIsReassignAllowed;
	}

	static void CleanUp ();

protected:
	BOOL SaveAccelaratorState (LPCTSTR lpszProfileName,
		UINT uiResId, HACCEL hAccelTable);
	BOOL LoadAccelaratorState (LPCTSTR lpszProfileName, 
		UINT uiResId, HACCEL& hAccelTable);
	
	static void SetAccelTable (LPACCEL& lpAccel, HACCEL& hAccelLast, int& nSize,
								const HACCEL hAccelCur);

	static LPACCEL	m_lpAccelDefault;	// Main frame acceleration table 
	static LPACCEL	m_lpAccel;			// Child frame acceleration table
	static int		m_nAccelDefaultSize;
	static int		m_nAccelSize;
	static HACCEL	m_hAccelDefaultLast;
	static HACCEL	m_hAccelLast;
	static BOOL		m_bAllAccelerators;
	static CString	m_strDelimiter;
	static BOOL		m_bIsReassignAllowed;
};

extern BCGCBPRODLLEXPORT CBCGPKeyboardManager*	g_pKeyboardManager;

#endif // !defined(AFX_BCGPKEYBOARDMANAGER_H__916B7A45_556A_11D2_B110_FD3F2C81B152__INCLUDED_)
