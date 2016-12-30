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
// BCGPGlobalUtils.h: interface for the CBCGPGlobalUtils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGLOBALUTILS_H__FD3579BF_C46C_439C_AD13_5DFDA777F240__INCLUDED_)
#define AFX_BCGPGLOBALUTILS_H__FD3579BF_C46C_439C_AD13_5DFDA777F240__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
class CBCGPDockManager;
class CBCGPBaseControlBar;
class CBCGPDockingControlBar;
class CBCGPBarContainerManager;
#endif

class BCGCBPRODLLEXPORT CBCGPGlobalUtils  
{
	friend class CBCGPFrameImpl;

public:
	CBCGPGlobalUtils();
	virtual ~CBCGPGlobalUtils();

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
	BOOL CheckAlignment (CPoint point, CBCGPBaseControlBar* pBar,
						 int nSencitivity,
                         const CBCGPDockManager* pDockManager,
                         BOOL bOuterEdge,
						 DWORD& dwAlignment, 
						 DWORD dwEnabledDockBars = CBRS_ALIGN_ANY, 
						 LPCRECT lpRectBounds = NULL) const;

	CBCGPDockManager* GetDockManager (CWnd* pWnd);
	
	void FlipRect (CRect& rect, int nDegrees);
	DWORD GetOppositeAlignment (DWORD dwAlign);
	void SetNewParent (CObList& lstControlBars, CWnd* pNewParent, 
						BOOL bCheckVisibility = TRUE);

	void CalcExpectedDockedRect (CBCGPBarContainerManager& barContainerManager, 
			CWnd* pWndToDock, CPoint ptMouse, CRect& rectResult, BOOL& bDrawTab, 
			CBCGPDockingControlBar** ppTargetBar);
	BOOL GetCBAndAlignFromPoint (CBCGPBarContainerManager& barContainerManager, 
								 CPoint pt, CBCGPDockingControlBar** ppTargetControlBar, 
							     DWORD& dwAlignment, BOOL& bTabArea, BOOL& bCaption);
	void ForceAdjustLayout (CBCGPDockManager* pDockManager, BOOL bForce = FALSE, BOOL bForceInvisible = FALSE, BOOL bForceNcArea = FALSE);
	BOOL IsWndCanFloatMulti (CWnd* pWnd) const;
	void AdjustRectToWorkArea (CRect& rect, CRect* pRectDelta = NULL);
	BOOL CanBeAttached (CWnd* pWnd) const;
#endif

	BOOL StringFromCy(CString& str, CY& cy);
	BOOL CyFromString(CY& cy, LPCTSTR psz);
	BOOL StringFromDecimal(CString& str, DECIMAL& decimal);
	BOOL DecimalFromString(DECIMAL& decimal, LPCTSTR psz);

	void StringAddItemQuoted (CString& strOutput, LPCTSTR pszItem, BOOL bInsertBefore, TCHAR charDelimiter = _T(';'), TCHAR charQuote = _T('\"'));
	CString StringExtractItemQuoted (const CString& strSource, LPCTSTR pszDelimiters, int& iStart, TCHAR charQuote = _T('\"'));

	HICON GetWndIcon (CWnd* pWnd, BOOL* bDestroyIcon = NULL, BOOL bNoDefault = TRUE);
	HICON GrayIcon(HICON hIcon);

	void EnableWindowShadow(CWnd* pWnd, BOOL bEnable);

	BOOL EnableEditCtrlAutoComplete (HWND hwndEdit, BOOL bDirsOnly);
	
	CSize GetSystemBorders(CWnd *pWnd);
	CSize GetSystemBorders(DWORD dwStyle);

	static CRuntimeClass* RuntimeClassFromName(LPCSTR lpszClassName);
	static CRuntimeClass* RuntimeClassFromName(LPCWSTR lpszClassName);

	static BOOL ProcessMouseWheel(WPARAM wp, LPARAM lp);

	BOOL	m_bDialogApp;
	BOOL	m_bIsDragging;
	BOOL	m_bUseMiniFrameParent;

protected:
	BOOL	m_bIsAdjustLayout;
};

extern BCGCBPRODLLEXPORT CBCGPGlobalUtils globalUtils;

void BCGCBPRODLLEXPORT BCGPShowAboutDlg (LPCTSTR lpszAppName);
void BCGCBPRODLLEXPORT BCGPShowAboutDlg (UINT uiAppNameResID);

#endif // !defined(AFX_BCGPGLOBALUTILS_H__FD3579BF_C46C_439C_AD13_5DFDA777F240__INCLUDED_)
