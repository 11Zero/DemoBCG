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

// BCGPKeyboardManager.cpp: implementation of the CBCGPKeyboardManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPKeyboardManager.h"
#include "BCGPMultiDocTemplate.h"
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPRegistry.h"
#include "BCGPKeyHelper.h"
#include "BCGPToolBar.h"
#include "RegPath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define REG_SECTION_FMT		_T("%sBCGKeyboard-%d")
#define REG_ENTRY_DATA		_T("Accelerators")

BCGCBPRODLLEXPORT CBCGPKeyboardManager*	g_pKeyboardManager = NULL;

static const CString strKbProfile = _T("BCGPKeyboardManager");

LPACCEL CBCGPKeyboardManager::m_lpAccel = NULL;
LPACCEL CBCGPKeyboardManager::m_lpAccelDefault = NULL;
int	CBCGPKeyboardManager::m_nAccelDefaultSize = 0;
int	CBCGPKeyboardManager::m_nAccelSize = 0;
HACCEL CBCGPKeyboardManager::m_hAccelDefaultLast = NULL;
HACCEL CBCGPKeyboardManager::m_hAccelLast = NULL;
BOOL CBCGPKeyboardManager::m_bAllAccelerators = FALSE;
CString CBCGPKeyboardManager::m_strDelimiter = _T("; ");
BOOL CBCGPKeyboardManager::m_bIsReassignAllowed = TRUE;

// a special struct that will cleanup automatically
class _KBD_TERM
{
public:
	~_KBD_TERM()
	{
		CBCGPKeyboardManager::CleanUp ();
	}
};

static const _KBD_TERM kbdTerm;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPKeyboardManager::CBCGPKeyboardManager()
{
	ASSERT (g_pKeyboardManager == NULL);
	g_pKeyboardManager = this;
}
//******************************************************************
CBCGPKeyboardManager::~CBCGPKeyboardManager()
{
	g_pKeyboardManager = NULL;
}
//******************************************************************
BOOL CBCGPKeyboardManager::UpdateAcellTable (CMultiDocTemplate* pTemplate,
											LPACCEL lpAccel, int nSize,
											CFrameWnd* pDefaultFrame)
{
	ASSERT (lpAccel != NULL);

	//--------------------------------
	// Create a new accelerator table:
	//--------------------------------
    HACCEL hAccelNew = ::CreateAcceleratorTable(lpAccel, nSize);
	if (hAccelNew == NULL)
	{
		TRACE(_T ("Can't create accelerator table!\n"));
		return FALSE;
	}

	if (!UpdateAcellTable (pTemplate, hAccelNew, pDefaultFrame))
	{
		::DestroyAcceleratorTable (hAccelNew);
		return FALSE;
	}

	return TRUE;
}
//******************************************************************
BOOL CBCGPKeyboardManager::UpdateAcellTable (CMultiDocTemplate* pTemplate,
											HACCEL hAccelNew,
											CFrameWnd* pDefaultFrame)
{
	ASSERT (hAccelNew != NULL);

	//-------------------------------------------------------------
	// Find an existing accelerator table associated with template:
	//-------------------------------------------------------------
	HACCEL hAccelTable = NULL;

	if (pTemplate != NULL)
	{
		ASSERT (pDefaultFrame == NULL);

		ASSERT_VALID (pTemplate);
		hAccelTable = pTemplate->m_hAccelTable;
		ASSERT (hAccelTable != NULL);

		pTemplate->m_hAccelTable = hAccelNew;

		//--------------------------------------------------
		// Walk trougth all template's documents and change
		// frame's accelerator tables:
		//--------------------------------------------------
		for (POSITION pos = pTemplate->GetFirstDocPosition(); pos != NULL;)
		{
			CDocument* pDoc = pTemplate->GetNextDoc (pos);
			ASSERT_VALID (pDoc);

			for (POSITION posView = pDoc->GetFirstViewPosition(); 
				posView != NULL;)
			{
				CView* pView = pDoc->GetNextView (posView);
				ASSERT_VALID (pView);

				CFrameWnd* pFrame = pView->GetParentFrame ();
				ASSERT_VALID (pFrame);

				if (pFrame->m_hAccelTable == hAccelTable)
				{
					pFrame->m_hAccelTable = hAccelNew;
				}
			}
		}
	}
	else
	{
		if (pDefaultFrame == NULL)
		{
			pDefaultFrame = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ());
		}

		if (pDefaultFrame != NULL)
		{
			hAccelTable = pDefaultFrame->m_hAccelTable;
			pDefaultFrame->m_hAccelTable = hAccelNew;
		}
	}

	if (hAccelTable == NULL)
	{
		TRACE(_T ("Accelerator table not found!\n"));
		return FALSE;
	}

	::DestroyAcceleratorTable (hAccelTable);
	return TRUE;
}
//************************************************************************************************
BOOL CBCGPKeyboardManager::SaveAccelaratorState (LPCTSTR lpszProfileName, 
	UINT uiResId, HACCEL hAccelTable)
{
	ASSERT (hAccelTable != NULL);

	CString strSection;
	strSection.Format (REG_SECTION_FMT, lpszProfileName, uiResId);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	int nAccelSize = ::CopyAcceleratorTable (hAccelTable, NULL, 0);
	if (nAccelSize == 0)
	{
		return FALSE;
	}

	if (!reg.CreateKey (strSection))
	{
		return FALSE;
	}

	LPACCEL lpAccel = new ACCEL [nAccelSize];
	ASSERT (lpAccel != NULL);

	::CopyAcceleratorTable (hAccelTable, lpAccel, nAccelSize);

	reg.Write (REG_ENTRY_DATA, (LPBYTE) lpAccel, nAccelSize * sizeof (ACCEL));

	delete [] lpAccel;
	return TRUE;
}
//************************************************************************************************
BOOL CBCGPKeyboardManager::LoadAccelaratorState (LPCTSTR lpszProfileName, 
	UINT uiResId, HACCEL& hAccelTable)
{
	ASSERT (hAccelTable == NULL);

	CString strSection;
	strSection.Format (REG_SECTION_FMT, lpszProfileName, uiResId);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	UINT uiSize;
	LPACCEL lpAccel;

	if (reg.Read (REG_ENTRY_DATA, (LPBYTE*) &lpAccel, &uiSize))
	{
		int nAccelSize = uiSize / sizeof (ACCEL);

		ASSERT (lpAccel != NULL);

		for (int i = 0; i < nAccelSize; i ++)
		{
			if (!CBCGPToolBar::IsCommandPermitted (lpAccel [i].cmd))
			{
				lpAccel [i].cmd = 0;
			}
		}

		hAccelTable = ::CreateAcceleratorTable(lpAccel, nAccelSize);
	}

	delete [] lpAccel;
	return hAccelTable != NULL;
}
//************************************************************************************************
BOOL CBCGPKeyboardManager::LoadState (LPCTSTR lpszProfileName, CFrameWnd* pDefaultFrame)
{
	CString strProfileName = ::BCGPGetRegPath (strKbProfile, lpszProfileName);

	CDocManager* pDocManager = AfxGetApp ()->m_pDocManager;
	if (pDocManager != NULL)
	{
		//---------------------------------------
		// Walk all templates in the application:
		//---------------------------------------
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition (); pos != NULL;)
		{
			CBCGPMultiDocTemplate* pTemplate = 
				(CBCGPMultiDocTemplate*) pDocManager->GetNextDocTemplate (pos);
			ASSERT_VALID (pTemplate);
			ASSERT_KINDOF (CDocTemplate, pTemplate);

			//-----------------------------------------------------
			// We are interessing CMultiDocTemplate objects with
			// the sahred menu only....
			//-----------------------------------------------------
			if (!pTemplate->IsKindOf (RUNTIME_CLASS (CMultiDocTemplate)) ||
				pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			UINT uiResId = pTemplate->GetResId ();
			ASSERT (uiResId != 0);

			HACCEL hAccellTable = NULL;
			if (LoadAccelaratorState (strProfileName, uiResId, hAccellTable))
			{
				UpdateAcellTable (pTemplate, hAccellTable);
			}
		}
	}

	//--------------------------------
	// Save default accelerator table:
	//--------------------------------
	if (pDefaultFrame == NULL)
	{
		pDefaultFrame = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ());
	}

	if (pDefaultFrame != NULL && pDefaultFrame->m_hAccelTable != NULL)
	{
		HACCEL hAccelTable = NULL;
		if (LoadAccelaratorState (strProfileName, 0, hAccelTable))
		{
			UpdateAcellTable (NULL, hAccelTable, pDefaultFrame);
		}
	}

	return TRUE;
}
//************************************************************************************************
BOOL CBCGPKeyboardManager::SaveState (LPCTSTR lpszProfileName, CFrameWnd* pDefaultFrame)
{
	CString strProfileName = ::BCGPGetRegPath (strKbProfile, lpszProfileName);

	CDocManager* pDocManager = AfxGetApp ()->m_pDocManager;
	if (pDocManager != NULL)
	{
		//---------------------------------------
		// Walk all templates in the application:
		//---------------------------------------
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition (); pos != NULL;)
		{
			CBCGPMultiDocTemplate* pTemplate = 
				(CBCGPMultiDocTemplate*) pDocManager->GetNextDocTemplate (pos);
			ASSERT_VALID (pTemplate);
			ASSERT_KINDOF (CDocTemplate, pTemplate);

			//-----------------------------------------------------
			// We are interessing CMultiDocTemplate objects in
			// the shared accelerator table only....
			//-----------------------------------------------------
			if (!pTemplate->IsKindOf (RUNTIME_CLASS (CMultiDocTemplate)) ||
				pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			UINT uiResId = pTemplate->GetResId ();
			ASSERT (uiResId != 0);

			SaveAccelaratorState (strProfileName, uiResId, pTemplate->m_hAccelTable);
		}
	}

	//--------------------------------
	// Save default accelerator table:
	//--------------------------------
	if (pDefaultFrame == NULL)
	{
		pDefaultFrame = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ());
	}

	if (pDefaultFrame != NULL && pDefaultFrame->m_hAccelTable != NULL)
	{
		SaveAccelaratorState (strProfileName, 0, pDefaultFrame->m_hAccelTable);
	}

	return TRUE;
}
//******************************************************************
void CBCGPKeyboardManager::ResetAll ()
{
	CDocManager* pDocManager = AfxGetApp ()->m_pDocManager;
	if (pDocManager != NULL)
	{
		//---------------------------------------
		// Walk all templates in the application:
		//---------------------------------------
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition (); pos != NULL;)
		{
			CBCGPMultiDocTemplate* pTemplate = 
				(CBCGPMultiDocTemplate*) pDocManager->GetNextDocTemplate (pos);
			ASSERT_VALID (pTemplate);
			ASSERT_KINDOF (CDocTemplate, pTemplate);

			//-----------------------------------------------------
			// We are interessing CMultiDocTemplate objects in
			// the shared accelerator table only....
			//-----------------------------------------------------
			if (!pTemplate->IsKindOf (RUNTIME_CLASS (CMultiDocTemplate)) ||
				pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			UINT uiResId = pTemplate->GetResId ();
			ASSERT (uiResId != 0);

			HINSTANCE hInst = AfxFindResourceHandle(
				MAKEINTRESOURCE (uiResId), RT_MENU);

			HACCEL hAccellTable = ::LoadAccelerators(hInst, MAKEINTRESOURCE (uiResId));
			if (hAccellTable != NULL)
			{
				UpdateAcellTable (pTemplate, hAccellTable);
			}
		}
	}

	//-----------------------------------
	// Restore default accelerator table:
	//-----------------------------------
	CFrameWnd* pWndMain = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ());
	if (pWndMain != NULL && pWndMain->m_hAccelTable != NULL)
	{
		UINT uiResId = 0;

		CBCGPMDIFrameWnd* pMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, AfxGetMainWnd ());
		if (pMDIFrame != NULL)
		{
			uiResId = pMDIFrame->GetDefaultResId ();
		}
		else	// Maybe, SDI frame...
		{
			CBCGPFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, AfxGetMainWnd ());
			if (pFrame != NULL)
			{
				uiResId = pFrame->GetDefaultResId ();
			}
			else	// Maybe, OLE frame...
			{
				CBCGPOleIPFrameWnd* pOleFrame = 
					DYNAMIC_DOWNCAST (CBCGPOleIPFrameWnd, AfxGetMainWnd ());
				if (pOleFrame != NULL)
				{
					uiResId = pOleFrame->GetDefaultResId ();
				}
			}
		}
		
		if (uiResId != 0)
		{
			HINSTANCE hInst = AfxFindResourceHandle(
				MAKEINTRESOURCE (uiResId), RT_MENU);

			HACCEL hAccellTable = ::LoadAccelerators(hInst, MAKEINTRESOURCE (uiResId));
			if (hAccellTable != NULL)
			{
				UpdateAcellTable (NULL, hAccellTable);
			}
		}
	}
}
//******************************************************************
BOOL CBCGPKeyboardManager::FindDefaultAccelerator (UINT uiCmd, CString& str, 
												CFrameWnd* pWndFrame,
												BOOL bIsDefaultFrame)
{
	str.Empty ();

	if (pWndFrame == NULL)
	{
		return FALSE;
	}

	HACCEL hAccelTable = pWndFrame->GetDefaultAccelerator ();
	if (hAccelTable == NULL)
	{
		return FALSE;
	}

	int& nSize = bIsDefaultFrame ? m_nAccelDefaultSize : m_nAccelSize;
	LPACCEL& lpAccel = bIsDefaultFrame ? m_lpAccelDefault : m_lpAccel;

	SetAccelTable (	lpAccel,
					bIsDefaultFrame ? m_hAccelDefaultLast : m_hAccelLast,
					nSize, hAccelTable);

	ASSERT (lpAccel != NULL);

	BOOL bFound = FALSE;
	for (int i = 0; i < nSize; i ++)
	{
		if (lpAccel [i].cmd == uiCmd)
		{
			bFound = TRUE;

			CBCGPKeyHelper helper (&lpAccel [i]);
			
			CString strKey;
			helper.Format (strKey);

			if (!str.IsEmpty ())
			{
				str += m_strDelimiter;
			}

			str += strKey;

			if (!m_bAllAccelerators)
			{
				break;
			}
		}
	}

	return bFound;
}
//*********************************************************************************************
void CBCGPKeyboardManager::SetAccelTable (LPACCEL& lpAccel, HACCEL& hAccelLast, 
										 int& nSize, const HACCEL hAccelCur)
{
	ASSERT (hAccelCur != NULL);
	if (hAccelCur == hAccelLast)
	{
		ASSERT (lpAccel != NULL);
		return;
	}

	//--------------------------------
	// Destroy old acceleration table:
	//--------------------------------
	if (lpAccel != NULL)
	{
		delete [] lpAccel;
		lpAccel = NULL;
	}

	nSize = ::CopyAcceleratorTable (hAccelCur, NULL, 0);

	lpAccel = new ACCEL [nSize];
	ASSERT (lpAccel != NULL);

	::CopyAcceleratorTable (hAccelCur, lpAccel, nSize);

	hAccelLast = hAccelCur;
}
//************************************************************************************
BOOL CBCGPKeyboardManager::IsKeyPrintable (const UINT nChar)
{
	// ----------------------------
	// Ensure the key is printable:
	// ----------------------------
	BYTE lpKeyState [256];
	::GetKeyboardState (lpKeyState);
	
	#ifndef _UNICODE
		WORD wChar = 0;
		int nRes = ::ToAsciiEx (nChar,
								MapVirtualKey (nChar, 0),
								lpKeyState,
								&wChar,
								0,
								::GetKeyboardLayout (AfxGetThread()->m_nThreadID));
	
	#else
		TCHAR szChar [2];
		memset (szChar, 0, sizeof (TCHAR) * 2);

		int nRes = ::ToUnicodeEx (nChar,
								MapVirtualKey (nChar, 0),
								lpKeyState,
								szChar,
								2,
								0,
								::GetKeyboardLayout (AfxGetThread()->m_nThreadID));
	#endif // _UNICODE

	return nRes > 0;
}
//************************************************************************************
UINT CBCGPKeyboardManager::TranslateCharToUpper (const UINT nChar)
{
	if (nChar < VK_NUMPAD0 || nChar > VK_NUMPAD9 ||
		(::GetAsyncKeyState (VK_MENU) & 0x8000))
	{
		if (!CBCGPToolBar::m_bExtCharTranslation)
		{
			// locale independent code:
			if ((nChar < 0x41 || nChar > 0x5A))
			{
				if (::GetAsyncKeyState (VK_MENU) & 0x8000)
				{
					return nChar;
				}
				else
				{
					return toupper (nChar);
				}
			}
			else // virt codes (A - Z)
			{
				return nChar;
			}
		}
	}

	// locale dependent code:
	#ifndef _UNICODE
		WORD wChar = 0;
		BYTE lpKeyState [256];
		::GetKeyboardState (lpKeyState);

		::ToAsciiEx (nChar,
					MapVirtualKey (nChar, 0),
					lpKeyState,
					&wChar,
					1,
					::GetKeyboardLayout (AfxGetThread()->m_nThreadID));

		TCHAR szChar [2] = {(TCHAR) wChar, '\0'};
	#else
		TCHAR szChar [2];
		memset (szChar, 0, sizeof (TCHAR) * 2);
		BYTE lpKeyState [256];
		::GetKeyboardState (lpKeyState);

		::ToUnicodeEx (nChar,
					MapVirtualKey (nChar, 0),
					lpKeyState,
					szChar,
					2,
					1,
					::GetKeyboardLayout (AfxGetThread()->m_nThreadID));
	#endif // _UNICODE
	
	CharUpper(szChar);

	return (UINT)szChar [0];
}
//************************************************************************************
void CBCGPKeyboardManager::CleanUp ()
{
	if (m_lpAccel != NULL)
	{
		delete [] m_lpAccel;
		m_lpAccel = NULL;

	}

	if (m_lpAccelDefault != NULL)
	{
		delete [] m_lpAccelDefault;
		m_lpAccelDefault = NULL;
	}
}
//******************************************************************
BOOL CBCGPKeyboardManager::IsKeyHandled (WORD nKey, BYTE fVirt,
										CFrameWnd* pWndFrame,
										BOOL bIsDefaultFrame)
{
	if (pWndFrame == NULL)
	{
		return FALSE;
	}

	HACCEL hAccelTable = pWndFrame->GetDefaultAccelerator ();
	if (hAccelTable == NULL)
	{
		return FALSE;
	}

	int& nSize = bIsDefaultFrame ? m_nAccelDefaultSize : m_nAccelSize;
	LPACCEL& lpAccel = bIsDefaultFrame ? m_lpAccelDefault : m_lpAccel;

	SetAccelTable (	lpAccel,
					bIsDefaultFrame ? m_hAccelDefaultLast : m_hAccelLast,
					nSize, hAccelTable);

	ASSERT (lpAccel != NULL);

	for (int i = 0; i < nSize; i ++)
	{
		if (lpAccel [i].key == nKey &&
			lpAccel [i].fVirt == fVirt)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//************************************************************************
void CBCGPKeyboardManager::ShowAllAccelerators (BOOL bShowAll, LPCTSTR lpszDelimiter)
{
	if (bShowAll)
	{
		m_bAllAccelerators = TRUE;

		if (lpszDelimiter != NULL)
		{
			m_strDelimiter = lpszDelimiter;
		}
	}
	else
	{
		m_bAllAccelerators = FALSE;
	}
}
//************************************************************************
void CBCGPKeyboardManager::AllowReassign(BOOL bAllow)
{
	m_bIsReassignAllowed = bAllow;
}

