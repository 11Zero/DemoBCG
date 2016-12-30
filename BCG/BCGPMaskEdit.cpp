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
// BCGPMaskEdit.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPMaskEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPMaskEdit

IMPLEMENT_DYNAMIC(CBCGPMaskEdit, CBCGPEdit)

BEGIN_MESSAGE_MAP(CBCGPMaskEdit, CBCGPEdit)
	//{{AFX_MSG_MAP(CBCGPMaskEdit)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()
	ON_CONTROL_REFLECT(EN_SETFOCUS, OnSetFocusR)
	//}}AFX_MSG_MAP
	ON_MESSAGE (WM_PASTE, OnPaste)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_GETTEXT, OnGetText)
	ON_MESSAGE(WM_GETTEXTLENGTH, OnGetTextLength)
	ON_MESSAGE(WM_CUT, OnCut)
	ON_MESSAGE(WM_CLEAR, OnClear)
END_MESSAGE_MAP()

CBCGPMaskEdit::CBCGPMaskEdit()
{
	m_bGetMaskedCharsOnly = TRUE;
	m_bSetMaskedCharsOnly = FALSE;
	m_bSelectByGroup = TRUE;
	m_bMaskKeyInProgress = FALSE;
	m_bPasteProcessing = FALSE;
	m_bSetTextProcessing = FALSE;
}

CBCGPMaskEdit::~CBCGPMaskEdit()
{
}

void CBCGPMaskEdit::EnableMask(LPCTSTR lpszMask, LPCTSTR lpszInputTemplate, 
						   TCHAR chMaskInputTemplate, LPCTSTR lpszValid)
{
	ASSERT(lpszMask != NULL);
	ASSERT(lpszInputTemplate != NULL);
	ASSERT(_istprint(chMaskInputTemplate));
	m_strMask = lpszMask;
	m_strInputTemplate = lpszInputTemplate;
	m_chMaskInputTemplate = chMaskInputTemplate;
	m_str = lpszInputTemplate;
	ASSERT(m_strMask.GetLength() == m_strInputTemplate.GetLength());

	if (lpszValid != NULL)
	{
		m_strValid = lpszValid;
	}
	else
	{
		m_strValid.Empty();
	}
}

void CBCGPMaskEdit::DisableMask()
{
	m_strMask.Empty();
	m_strInputTemplate.Empty();
}

void CBCGPMaskEdit::SetValidChars(LPCTSTR lpszValid)
{
	if (lpszValid != NULL)
	{
		m_strValid = lpszValid;
	}
	else
	{
		m_strValid.Empty();
	}
}

BOOL CBCGPMaskEdit::IsMaskedChar(TCHAR chChar, TCHAR chMaskChar) const
{
	// ------------------------------
	// Check the key against the mask
	// ------------------------------
	switch (chMaskChar)
	{
    case _T('D'):		// digit only
		if (_istdigit(chChar)) 
		{
			return TRUE;
		}
		break;
    case _T('d'):		// digit or space
		if (_istdigit(chChar)) 
		{
			return TRUE;
		}
		if (_istspace(chChar))
		{
			return TRUE;
		}
		break;
    case _T('+'):		// '+' or '-' or space
		if (chChar == _T('+') || chChar == _T('-')) 
		{
			return TRUE;
		}
		if (_istspace(chChar))
		{
			return TRUE;
		}
		break;
    case _T('C'):		// alpha only
		if (_istalpha(chChar)) 
		{
			return TRUE;
		}
		break;
    case _T('c'):		// alpha or space
		if (_istalpha(chChar)) 
		{
			return TRUE;
		}
		if (_istspace(chChar))
		{
			return TRUE;
		}
		break;
    case _T('A'):		// alpha numeric only
		if (_istalnum(chChar)) 
		{
			return TRUE;
		}
		break;
    case _T('a'):		// alpha numeric or space
		if (_istalnum(chChar)) 
		{
			return TRUE;
		}
		if (_istspace(chChar))
		{
			return TRUE;
		}
		break;
    case _T('*'):		// a printable character 
        if (_istprint(chChar)) 
        { 
            return TRUE; 
        } 
        break; 
	}
	return FALSE; // not allowed symbol
}

BOOL CBCGPMaskEdit::SetValue(LPCTSTR lpszString, BOOL bWithDelimiters)
{
	ASSERT(m_strMask.IsEmpty() == m_strInputTemplate.IsEmpty());
	ASSERT(m_strMask.GetLength() == m_strInputTemplate.GetLength());
	ASSERT(lpszString != NULL);

	// ------------------------------------------------
	// Make sure the string is not longer than the mask
	// ------------------------------------------------
	CString strSource = lpszString;
	if (!m_strMask.IsEmpty())
	{
		if (bWithDelimiters)
		{
			if (strSource.GetLength() > m_strMask.GetLength())
			{
				return FALSE;
			}
		}
		else
		{
			// Count _T('_') in m_strInputTemplate
			int nCount = 0;
			for (int i = 0; i < m_strInputTemplate.GetLength(); i++)
			{
				if (m_strInputTemplate[i] == _T('_'))
				{
					nCount++;
				}

			}
			if (strSource.GetLength() > nCount)
			{
				return FALSE;
			}
		}
	}

	// ----------------------------------------------------
	// Make sure the value has only valid string characters
	// ----------------------------------------------------
	if (!m_strValid.IsEmpty()) 
	{
		BOOL bOk = TRUE;
		for (int iPos = 0; bOk && iPos < strSource.GetLength(); iPos++)
		{
			if (m_strInputTemplate.IsEmpty () || m_strInputTemplate[iPos] == _T('_'))
			{
				if (m_strInputTemplate.IsEmpty () ||
					strSource[iPos] != m_chMaskInputTemplate) // allow m_chMaskInputTemplate
				{
					bOk = (m_strValid.Find (strSource[iPos]) != -1);
				}
			}
		}

		if (!bOk)
		{
			return FALSE;
		}
	}

	// -----------------------------------
	// Use mask, validate against the mask
	// -----------------------------------
	if (!m_strMask.IsEmpty())
	{
		ASSERT(m_str.GetLength() == m_strMask.GetLength());

		CString strResult = m_strInputTemplate;
		
		// Replace '_' with default char
		for (int i=0; i<strResult.GetLength(); i++)
		{
			if (m_strInputTemplate[i] == _T('_'))
			{
				strResult.SetAt(i, m_chMaskInputTemplate);
			}
		}

		int iSrcChar = 0;
		int iDstChar = 0;
		while ((iSrcChar<strSource.GetLength()) && 
				(iDstChar<m_strInputTemplate.GetLength()))
		{
			// iDstChar - character entry position ("_" char)
			if (m_strInputTemplate[iDstChar] == _T('_')) 
			{
				TCHAR chChar = strSource[iSrcChar];
				if (chChar != m_chMaskInputTemplate) // allow m_chMaskInputTemplate
				{
					if (!IsMaskedChar(chChar, m_strMask[iDstChar]))
					{
						return FALSE;
					}
				}
				strResult.SetAt(iDstChar, chChar);
				iSrcChar++;
				iDstChar++;
			}
			
			// iDstChar - delimeter
			else 
			{
				if (bWithDelimiters)
				{
					if (m_strInputTemplate[iDstChar] != strSource[iSrcChar])
					{
						return FALSE;
					}

					iSrcChar++;
					iDstChar++;
				}
				else
				{
					iDstChar++;
				}
			}
		}
		m_str = strResult;
	}
	// --------------
	// Don't use mask
	// --------------
	else
	{
		m_str = strSource;
	}

	return TRUE;
}

const CString CBCGPMaskEdit::GetMaskedValue(BOOL bWithSpaces) const
{
	ASSERT(m_strMask.IsEmpty() == m_strInputTemplate.IsEmpty());
	ASSERT(m_strMask.GetLength() == m_strInputTemplate.GetLength());

	// --------------
	// Don't use mask
	// --------------
	if (m_strMask.IsEmpty())
	{
		return m_str;
	}

	// --------
	// Use mask
	// --------
	ASSERT(m_str.GetLength() == m_strMask.GetLength());

	CString strResult;
	for (int iChar=0; iChar < m_strInputTemplate.GetLength(); iChar++)
	{
		if (m_strInputTemplate[iChar] == _T('_'))
		{
			TCHAR ch = m_str[iChar];
			if (ch == m_chMaskInputTemplate)
			{
				if (bWithSpaces)
				{
					strResult += ch;
				}
			}
			else
			{

				ASSERT((!m_strValid.IsEmpty()) ? (m_strValid.Find(ch) != -1) : TRUE);
				ASSERT(IsMaskedChar(ch, m_strMask[iChar]));
				strResult += ch;
			}
		}
	}
	return strResult;
}

///////////////////////////////////
// Replace standard CWnd operations

LRESULT CBCGPMaskEdit::OnSetText (WPARAM, LPARAM lParam)
{
	if (m_bSetTextProcessing || m_bPasteProcessing)
	{
		return Default ();
	}

	m_bSetTextProcessing = TRUE;

	BOOL bSetValueRes = SetValue((LPCTSTR)lParam, !m_bSetMaskedCharsOnly);
	if (bSetValueRes)
	{
		if (m_str.Compare ((LPCTSTR)lParam) != 0)
		{
			LRESULT lRes = ::SetWindowText (GetSafeHwnd (), m_str);

			m_bSetTextProcessing = FALSE;
			return lRes;
		}

		LRESULT lRes = Default ();

		m_bSetTextProcessing = FALSE;
		return lRes;
	}

	m_bSetTextProcessing = FALSE;
	return FALSE;
}

LRESULT CBCGPMaskEdit::OnGetText(WPARAM wParam, LPARAM lParam)
{
	if (m_bPasteProcessing)
	{
		return Default ();
	}

	int nMaxCount = (int)wParam;
	if (nMaxCount == 0)
	{
		return 0;       // nothing copied
	}

	LPTSTR lpszDestBuf = (LPTSTR)lParam;
	if (lpszDestBuf == NULL)
	{
		return 0;       // nothing copied
	}

	//-------------
	// Receive text
	//-------------
	CString strText;

	if (m_bGetMaskedCharsOnly)
	{
		strText = GetMaskedValue();
	}
	else
	{
		strText = GetValue();
	}

	//----------
	// Copy text
	//----------
	int nCount = min(nMaxCount, strText.GetLength());
	LPCTSTR lpcszTmp = strText;
	CopyMemory(lpszDestBuf, lpcszTmp, nCount*sizeof(TCHAR));

	// Add terminating null character if possible
	if (nMaxCount > nCount)
	{
		lpszDestBuf[nCount] = _T('\0'); 
	}
	
	return nCount*sizeof(TCHAR);
}

LRESULT CBCGPMaskEdit::OnGetTextLength(WPARAM, LPARAM)
{
	if (m_bPasteProcessing)
	{
		return Default ();
	}

	//-------------
	// Receive text
	//-------------
	CString strText;
	
	if (m_bGetMaskedCharsOnly)
	{
		strText = GetMaskedValue();
	}
	else
	{
		strText = GetValue();
	}

	return (LRESULT) strText.GetLength ();
}

///////////////////////////////////
// Handlers

int CBCGPMaskEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CWnd::SetWindowText(m_str);
	return 0;
}

void CBCGPMaskEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	const BOOL bIsReadOnly = ((GetStyle() & ES_READONLY) == ES_READONLY);

	// --------------------------------------
	// Make sure the mask has entry positions
	// --------------------------------------
	int nGroupStart, nGroupEnd;
	GetGroupBounds(nGroupStart, nGroupEnd);
	if (nGroupStart == -1)
	{
		// mask has no entry positions
		MessageBeep((UINT)-1);
		return;
	}

	switch (nChar)
	{
	case VK_END:
		{
			// ----------------------
			// Calc last group bounds
			// ----------------------
			int nGroupStart, nGroupEnd;
			CBCGPEdit::GetSel(nGroupStart, nGroupEnd);
			ASSERT(nGroupStart != -1);
			GetGroupBounds(nGroupStart, nGroupEnd, nGroupEnd, TRUE);
			if (nGroupStart == -1)
			{
				GetGroupBounds(nGroupStart, nGroupEnd, m_str.GetLength(), FALSE);
			}
			ASSERT(nGroupStart != -1);

			if (::GetKeyState(VK_SHIFT)&0x80)
			{	
				int nStart, nEnd;
				CBCGPEdit::GetSel(nStart, nEnd);
				if (m_bSelectByGroup)
				{
					nStart = max(nStart, nGroupStart);
				}
				CBCGPEdit::SetSel(nStart, nGroupEnd);
				return;
			}
			CBCGPEdit::SetSel(nGroupEnd, nGroupEnd);
			return;
		}
	case VK_HOME:
		{
			// -----------------------
			// Calc first group bounds
			// -----------------------
			int nGroupStart, nGroupEnd;
			CBCGPEdit::GetSel(nGroupStart, nGroupEnd);
			ASSERT(nGroupStart != -1);
			GetGroupBounds(nGroupStart, nGroupEnd, nGroupStart, FALSE);
			if (nGroupStart == -1)
			{
				GetGroupBounds(nGroupStart, nGroupEnd, 0, TRUE);
			}
			ASSERT(nGroupStart != -1);
			
			if (::GetKeyState(VK_SHIFT)&0x80)
			{	
				int nStart, nEnd;
				CBCGPEdit::GetSel(nStart, nEnd);
				if (m_bSelectByGroup)
				{
					nEnd = min(nEnd, nGroupEnd);
				}
				CBCGPEdit::SetSel(nGroupStart, nEnd);

				if (GetFocus ()->GetSafeHwnd () == GetSafeHwnd ())
				{
					CPoint ptCaret = PosFromChar(nGroupStart);
					SetCaretPos(ptCaret);
				}
				return;
			}
			CBCGPEdit::SetSel(nGroupStart, nGroupStart);
			return;
		}
	case VK_UP:
	case VK_LEFT:
		{
			// -----------------------
			// Calc first group bounds
			// -----------------------
			int nGroupStart, nGroupEnd;
			CBCGPEdit::GetSel(nGroupStart, nGroupEnd);
			ASSERT(nGroupStart != -1);
			GetGroupBounds(nGroupStart, nGroupEnd, nGroupStart, FALSE);
			if (nGroupStart == -1)
			{
				GetGroupBounds(nGroupStart, nGroupEnd, 0, TRUE);
			}
			ASSERT(nGroupStart != -1);

			if (::GetKeyState(VK_SHIFT)&0x80)
			{	
				int nStart, nEnd;
				CBCGPEdit::GetSel(nStart, nEnd);
				if (m_bSelectByGroup)
				{
					int nNewStart = max(nStart-1, nGroupStart);
					// additional
					nStart = min(nNewStart, nGroupEnd);
				}
				else
				{
					nStart = nStart-1;
				}

				CBCGPEdit::SetSel(nStart, nEnd);
	
				if (GetFocus ()->GetSafeHwnd () == GetSafeHwnd ())
				{
					CPoint ptCaret = PosFromChar(nStart);
					SetCaretPos(ptCaret);
				}
				return;
			}
			else if (::GetKeyState(VK_CONTROL)&0x80)
			{
				// move to the previous group
				int nStart, nEnd;
				CBCGPEdit::GetSel(nStart, nEnd);
				ASSERT(nStart != -1);
				
				if (nStart > 1)  // can search previous group
				{
					GetGroupBounds(nGroupStart, nGroupEnd, nStart-1, FALSE);
				}
				if ((nGroupStart != -1) &&						  // if previous group was found
					(nGroupStart != nStart || nGroupEnd != nEnd)) // and it's not the same
				{
					CBCGPEdit::SetSel(nGroupStart, nGroupEnd);
				}
				else // no more groups
				{
					MessageBeep((UINT)-1);
				}
				return;
			}
			else
			{
				int nStart, nEnd;
				CBCGPEdit::GetSel(nStart, nEnd);
				// move to the previous group
				if ((nStart==nEnd) && (nStart==nGroupStart))
				{
					if (nStart > 1)  // can search previous group
					{
						GetGroupBounds(nGroupStart, nGroupEnd, nStart-1, FALSE);
					}
					if ((nGroupStart != -1) && (nGroupEnd < nStart))  // if previous group was found
					{
						CBCGPEdit::SetSel(nGroupEnd, nGroupEnd);
					}
					else // no more groups
					{
						MessageBeep((UINT)-1);
					}
				}
				else
				{
					int nNewStart = max(nStart-1, nGroupStart);
					// additional
					nNewStart = min(nNewStart, nGroupEnd);
					CBCGPEdit::SetSel(nNewStart, nNewStart);
				}
				return;
			}
		}

	case VK_DOWN:
	case VK_RIGHT:
		{
			// ----------------------
			// Calc last group bounds
			// ----------------------
			int nGroupStart, nGroupEnd;
			CBCGPEdit::GetSel(nGroupStart, nGroupEnd);
			ASSERT(nGroupStart != -1);
			GetGroupBounds(nGroupStart, nGroupEnd, nGroupEnd, TRUE);
			if (nGroupStart == -1)
			{
				GetGroupBounds(nGroupStart, nGroupEnd, m_str.GetLength(), FALSE);
			}
			ASSERT(nGroupStart != -1);

			if (::GetKeyState(VK_SHIFT)&0x80)
			{
				int nStart, nEnd;
				CBCGPEdit::GetSel(nStart, nEnd);
				if (m_bSelectByGroup)
				{
					int nNewEnd = min(nEnd+1, nGroupEnd);
					// additional
					nNewEnd = max(nNewEnd, nGroupStart);
					CBCGPEdit::SetSel(nStart, nNewEnd);
				}
				else
				{
					CBCGPEdit::SetSel(nStart, nEnd+1);
				}
				return;
			}
			else if (::GetKeyState(VK_CONTROL)&0x80)
			{
				// move to the next group
				int nStart, nEnd;
				CBCGPEdit::GetSel(nStart, nEnd);
				ASSERT(nStart != -1);
				
				if (nEnd < m_str.GetLength()-1) // can search next group
				{
					GetGroupBounds(nGroupStart, nGroupEnd, nEnd+1, TRUE);
				}
				if ((nGroupStart != -1) &&						  // if previous group was found
					(nGroupStart != nStart || nGroupEnd != nEnd)) // and it's not the same
				{
					CBCGPEdit::SetSel(nGroupStart, nGroupEnd);
				}
				else // no more groups
				{
					MessageBeep((UINT)-1);
				}
				return;
			}
			else
			{
				int nStart, nEnd;
				CBCGPEdit::GetSel(nStart, nEnd);
				// move to the next group
				if ((nStart==nEnd) && (nEnd==nGroupEnd))
				{
					if (nEnd < m_str.GetLength()-1) // can search next group
					{
						GetGroupBounds(nGroupStart, nGroupEnd, nStart+1, TRUE);
					}
					if ((nGroupStart != -1) && (nGroupStart > nEnd)) // if next group was found
					{
						CBCGPEdit::SetSel(nGroupStart, nGroupStart);
					}
					else // no more groups
					{
						MessageBeep((UINT)-1);
					}
				}
				else
				{
					int nNewEnd = min(nEnd+1, nGroupEnd);
					// additional
					nNewEnd = max(nNewEnd, nGroupStart);
					CBCGPEdit::SetSel(nNewEnd, nNewEnd);
				}
				return;
			}
		}

	case VK_BACK:
		if (!bIsReadOnly)
		{
			// Special processing
			OnCharBackspace(nChar, nRepCnt, nFlags);
			return;
		}
		break;

	case VK_DELETE:
		if (!bIsReadOnly)
		{
			if (::GetKeyState(VK_SHIFT)&0x80)
			{
				break;
			}
			// Special processing
			OnCharDelete(nChar, nRepCnt, nFlags);
			return;
		}
		break;

	case VK_INSERT:
		if (!bIsReadOnly)
		{
			if ((::GetKeyState(VK_CONTROL)&0x80) || (::GetKeyState(VK_SHIFT)&0x80))
			{
				break;
			}
			if (!m_strMask.IsEmpty())
			{	
				return;
			}
		}
		break;
	}

	CBCGPEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CBCGPMaskEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bSelectByGroup)
	{
		// -----------------
		// Calc group bounds
		// -----------------
		int nGroupStart, nGroupEnd;
		CBCGPEdit::GetSel(nGroupStart, nGroupEnd);
		GetGroupBounds(nGroupStart, nGroupEnd, nGroupStart, TRUE);
		if (nGroupStart == -1)
		{
			CBCGPEdit::GetSel(nGroupStart, nGroupEnd);
			GetGroupBounds(nGroupStart, nGroupEnd, nGroupStart, FALSE);
		}

		// -----------------
		// Correct selection
		// -----------------
		int nStart, nEnd;
		CBCGPEdit::GetSel(nStart, nEnd);

		int nNewStart = max(nStart, nGroupStart);
		int nNewEnd = min(nEnd, nGroupEnd);
		// additional
		nNewStart = min(nNewStart, nGroupEnd);
		nNewEnd = max(nNewEnd ,nGroupStart);
		if ((nNewEnd != nEnd) || (nNewStart != nStart))
		{
			CBCGPEdit::SetSel(nNewStart, nNewEnd);
		}
	}
	
	CBCGPEdit::OnLButtonUp(nFlags, point);
}

void CBCGPMaskEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	const BOOL bIsReadOnly = ((GetStyle() & ES_READONLY) == ES_READONLY);
	if (bIsReadOnly)
	{
		CBCGPEdit::OnChar(nChar, nRepCnt, nFlags);
		return;
	}

	TCHAR chChar = (TCHAR) nChar;
	if (_istprint(chChar) && !(::GetKeyState(VK_CONTROL)&0x80))
	{
		OnCharPrintchar(nChar, nRepCnt, nFlags);	
		return;
	}
	else if ((nChar == VK_DELETE || nChar == VK_BACK) && (!m_strMask.IsEmpty()))
	{
		return;
	}

	int nBeginOld, nEndOld;
	CBCGPEdit::GetSel(nBeginOld, nEndOld);

	CBCGPEdit::OnChar(nChar, nRepCnt, nFlags);

	DoUpdate (TRUE, nBeginOld, nEndOld);
} 

//////////////////////////////
// Char routines

BOOL CBCGPMaskEdit::CheckChar(TCHAR chChar, int nPos) // returns TRUE if the symbol is valid
{	
	ASSERT(m_strMask.IsEmpty() == m_strInputTemplate.IsEmpty());
	ASSERT(m_strMask.GetLength() == m_strInputTemplate.GetLength());
	ASSERT(_istprint(chChar) != FALSE);

	ASSERT(nPos >= 0);

	// --------------
	// Don't use mask
	// --------------
	if (m_strMask.IsEmpty())
	{
		// Use valid string characters
		if (!m_strValid.IsEmpty())
		{
			return (m_strValid.Find(chChar) != -1);
		}
		// Don't use valid string characters
		else
		{
			return TRUE;
		}
	}
	else
	{
		ASSERT(nPos < m_strMask.GetLength());
	}

	// --------
	// Use mask
	// --------
	ASSERT(m_str.GetLength() == m_strMask.GetLength());
	if (m_strInputTemplate[nPos] == _T('_'))
	{
		BOOL bIsMaskedChar = IsMaskedChar(chChar, m_strMask[nPos]);

		// Use valid string characters
		if (!m_strValid.IsEmpty())
		{
			return bIsMaskedChar && (m_strValid.Find(chChar) != -1);
		}
		// Don't use valid string characters
		else
		{
			return bIsMaskedChar;
		}
	}
	else
	{
		return FALSE;
	}
}

void CBCGPMaskEdit::OnCharPrintchar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	ASSERT(m_strMask.IsEmpty() == m_strInputTemplate.IsEmpty());
	ASSERT(m_strMask.GetLength() == m_strInputTemplate.GetLength());

	TCHAR chChar = (TCHAR) nChar;
	ASSERT(_istprint(chChar) != FALSE);

	// -----------------------------------------------
	// Processing ES_UPPERCASE and ES_LOWERCASE styles
	// -----------------------------------------------
	DWORD dwStyle = GetStyle ();
	if (dwStyle & ES_UPPERCASE)
	{
		chChar = (TCHAR)_totupper(chChar);
	}
	else if (dwStyle & ES_LOWERCASE)
	{
		chChar = (TCHAR)_totlower(chChar);
	}
	
	int nStartPos, nEndPos;

	CBCGPEdit::GetSel(nStartPos, nEndPos);

	ASSERT(nStartPos>=0);
	ASSERT(nEndPos>=0);
	ASSERT(nEndPos>=nStartPos);

	// -----------------
	// Calc group bounds
	// -----------------
	int nGroupStart, nGroupEnd;
	GetGroupBounds(nGroupStart, nGroupEnd, nStartPos);

	// ------------
	// Out of range
	// ------------
	if ((nStartPos<0) && (nEndPos > m_str.GetLength()) ||
		(nStartPos > nGroupEnd))
	{
		MessageBeep((UINT)-1);
		CBCGPEdit::SetSel(nGroupStart, nGroupEnd);
		return;
	}

	if (nEndPos < nGroupStart)
	{
		CBCGPEdit::SetSel(nGroupStart, nGroupStart);
		nStartPos = nGroupStart;
		nEndPos = nGroupStart;
	}

	if ((nStartPos < nGroupStart) || (nEndPos > nGroupEnd))
	{
		CBCGPEdit::SetSel(nGroupStart, nGroupEnd);
		nStartPos = nGroupStart;
		nEndPos = nGroupEnd;
	}

	// -----------------
	// No selected chars
	// -----------------
	if (nStartPos == nEndPos)
	{
		// Use m_strMask
		if (!m_strMask.IsEmpty())
		{
			// ----------------------------------------------
			// Automaticaly move the cursor to the next group
			// ----------------------------------------------
			if (nEndPos==nGroupEnd || // at the end of group
				nStartPos < nGroupStart || nStartPos > nGroupEnd) // not in the middle of a group
			{
				// no space for new char
				if (nEndPos >= m_str.GetLength()-1)
				{
					MessageBeep((UINT)-1);
					return;
				}

				// can search next group
				else if (nEndPos < m_str.GetLength()-1)
				{
					GetGroupBounds(nGroupStart, nGroupEnd, nEndPos+1, TRUE);
				}

				// if next group was found
				if ((nGroupStart != -1) && (nGroupStart > nEndPos))
				{
					CBCGPEdit::SetSel(nGroupStart, nGroupStart);
					nStartPos = nGroupStart;
					nEndPos = nGroupStart;
				}

				// no more groups
				else
				{
					MessageBeep((UINT)-1);
					return;
				}
			}

			// ----------------------
			// Check char in position
			// ----------------------
			if (!CheckChar(chChar, nStartPos)) 
			{
				MessageBeep((UINT)-1);
				return;
			}

			// ---------------------------------
			// Replace char in Editbox and m_str
			// ---------------------------------
			CBCGPEdit::SetSel(nStartPos, nEndPos+1);
			CBCGPEdit::ReplaceSel(CString(chChar), TRUE);
			m_str.SetAt(nEndPos, chChar);          
			CBCGPEdit::SetSel(nEndPos+1, nEndPos+1);

			// ----------------------------------------------
			// Automaticaly move the cursor to the next group
			// ----------------------------------------------
			CBCGPEdit::GetSel(nStartPos, nEndPos);
			if (nEndPos==nGroupEnd) // at the end of group
			{
				// can search next group
				if (nEndPos < m_str.GetLength()-1)
				{
					GetGroupBounds(nGroupStart, nGroupEnd, nEndPos+1, TRUE);
				}

				// if next group was found
				if ((nGroupStart != -1) && (nGroupStart > nEndPos))
				{
					CBCGPEdit::SetSel(nGroupStart, nGroupStart);
					nStartPos = nGroupStart;
					nEndPos = nGroupStart;
				}
			}
		}

		// Don't use m_strMask
		else
		{
			// ----------------------
			// Check char in position
			// ----------------------
			if (!CheckChar(chChar, nStartPos)) 
			{
				MessageBeep((UINT)-1);
				return;
			}

			// Don't use m_chMask
			int nBeginOld, nEndOld;
			CBCGPEdit::GetSel(nBeginOld, nEndOld);

			CBCGPEdit::OnChar(nChar, nRepCnt, nFlags);

			DoUpdate (TRUE, nBeginOld, nEndOld);
		}
	}

	// -------------------------------
	// Have one or more chars selected
	// -------------------------------
	else
	{
		// ----------------------
		// Check char in position
		// ----------------------
		if (!CheckChar(chChar, nStartPos)) 
		{
			MessageBeep((UINT)-1);
			return;
		}

		// ----------------------------------
		// Replace chars in Editbox and m_str
		// ----------------------------------
		if (!m_strInputTemplate.IsEmpty()) // Use m_strInputTemplate
		{
			// ---------------------------------------------------
			// Calc the number of literals with the same mask char
			// ---------------------------------------------------
			ASSERT(nStartPos >= 0);
			ASSERT(nEndPos > 0);
			ASSERT(nStartPos <= m_strInputTemplate.GetLength());

			int nSameMaskCharsNum = 1;
			int nIndex = nStartPos; // an index of the first selected char
			TCHAR chMaskChar = m_strMask[nIndex];
			BOOL bScanMore = TRUE;
			while (bScanMore && (nIndex + nSameMaskCharsNum < nGroupEnd))
			{
				if (m_strMask[nIndex + nSameMaskCharsNum] == chMaskChar)
				{
					nSameMaskCharsNum++;
				}
				else
				{
					bScanMore = FALSE;
				}
			}
			
			// Make sure the selection has the same mask char
			if (nEndPos - nStartPos > nSameMaskCharsNum)
			{
				MessageBeep((UINT)-1);
				CBCGPEdit::SetSel(nIndex, nIndex+nSameMaskCharsNum);
				return;
			}

			// -------------------------------
			// Form the shifted replace string
			// -------------------------------
			ASSERT(nIndex >= nGroupStart);
			ASSERT(nIndex + nSameMaskCharsNum <= nGroupEnd);
			
			CString strReplace = m_str.Mid(nIndex, nSameMaskCharsNum);
			if (nSameMaskCharsNum > 0)
			{
				ASSERT(nStartPos <= m_strInputTemplate.GetLength());
				ASSERT(nEndPos <= m_strInputTemplate.GetLength());
				int nRange = nEndPos - nStartPos;
				ASSERT(nRange>0);

				strReplace = strReplace.Right(nSameMaskCharsNum - nRange + 1);
				strReplace += CString(m_chMaskInputTemplate, nRange - 1);
				ASSERT(strReplace.GetLength() > 0);
				strReplace.SetAt(0, chChar);
			}

			// -------------------------------------------
			// Replace the content with the shifted string
			// -------------------------------------------
			CBCGPEdit::SetSel(nIndex, nIndex+nSameMaskCharsNum);
			CBCGPEdit::ReplaceSel(strReplace, TRUE);
			CBCGPEdit::SetSel(nIndex, nIndex);
			for(int i=0; i < strReplace.GetLength(); i++)
			{
				m_str.SetAt(nIndex+i, strReplace[i]);
			}
			CBCGPEdit::SetSel(nStartPos+1, nStartPos+1);
		}
		else
		{
			// Don't use m_chMaskInputTemplate
			int nBeginOld, nEndOld;
			CBCGPEdit::GetSel(nBeginOld, nEndOld);

			CBCGPEdit::OnChar(nChar, nRepCnt, nFlags);

			DoUpdate (TRUE, nBeginOld, nEndOld);
		}
		
	}
}

void CBCGPMaskEdit::OnCharBackspace(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	ASSERT(m_strMask.IsEmpty() == m_strInputTemplate.IsEmpty());
	ASSERT(m_strMask.GetLength() == m_strInputTemplate.GetLength());

	int nStartPos, nEndPos;
	CBCGPEdit::GetSel(nStartPos, nEndPos);

	ASSERT(nStartPos>=0);
	ASSERT(nEndPos>=0);
	ASSERT(nEndPos>=nStartPos);

	// -----------------
	// Calc group bounds
	// -----------------
	int nGroupStart, nGroupEnd;
	GetGroupBounds(nGroupStart, nGroupEnd, nStartPos);

	// ---------------------------------
	// Special processing for Delete All
	// ---------------------------------
	if (nStartPos == 0 && nEndPos > 0 && nEndPos == m_str.GetLength () && !m_strMask.IsEmpty())
	{
		if (SetValue(_T(""), !m_bSetMaskedCharsOnly))
		::SetWindowText (GetSafeHwnd (), m_str);

		if (nGroupStart >= 0)
		{
			SetSel (nGroupStart, nGroupStart);
		}
		return;
	}

	// ------------
	// Out of range
	// ------------
	if ((nStartPos<0) && (nEndPos > m_str.GetLength()) ||
		(nStartPos < nGroupStart) || (nStartPos > nGroupEnd) ||
		(nEndPos < nGroupStart) || (nEndPos > nGroupEnd))
	{
		MessageBeep((UINT)-1);
		CBCGPEdit::SetSel(nGroupStart, nGroupEnd);
		return;
	}

	// -----------------
	// No selected chars
	// -----------------
	if (nStartPos == nEndPos)
	{

		// Use m_strMask
		if (!m_strMask.IsEmpty())
		{
			// --------------------------------------------------
			// Automaticaly move the cursor to the previous group
			// --------------------------------------------------
			if (nEndPos==nGroupStart) // at the start of group
			{
				// can search previous group
				if (nEndPos > 1)
				{
					GetGroupBounds(nGroupStart, nGroupEnd, nEndPos-1, FALSE);
				}

				// if previous group was found
				if ((nGroupStart != -1) && (nGroupEnd < nEndPos))
				{
					CBCGPEdit::SetSel(nGroupEnd, nGroupEnd);
					return;
				}

				// no more group
				else
				{
					MessageBeep((UINT)-1);
					return;
				}
			}

			// ---------------------------------------------------
			// Calc the number of literals with the same mask char
			// ---------------------------------------------------
			ASSERT(nStartPos > 0);
			ASSERT(nEndPos > 0);
			ASSERT(nGroupEnd <= m_strInputTemplate.GetLength());

			int nSameMaskCharsNum = 1;
			int nIndex = nStartPos-1; // an index of the char to delete
			TCHAR chMaskChar = m_strMask[nIndex];
			BOOL bScanMore = TRUE;
			while (bScanMore && (nIndex + nSameMaskCharsNum < nGroupEnd))
			{
				if (m_strMask[nIndex + nSameMaskCharsNum] == chMaskChar)
				{
					nSameMaskCharsNum++;
				}
				else
				{
					bScanMore = FALSE;
				}
			}

			// ---------------------------------
			// Validate new string (dispensable)
			// ---------------------------------
			int i = nIndex;
			for (; (i + nSameMaskCharsNum < nGroupEnd); i++)
			{
				if (m_str[i] != m_chMaskInputTemplate) // allow m_chMaskInputTemplate
				{
					if (!IsMaskedChar(m_str[i], m_strMask[i]))
					{
						MessageBeep((UINT)-1);
						return;
					}
				}
			}
			
			// -----------------------
			// Form the shifted string
			// -----------------------
			ASSERT(nIndex >= nGroupStart);
			ASSERT(nIndex + nSameMaskCharsNum <= nGroupEnd);
			
			CString strReplace = m_str.Mid(nIndex, nSameMaskCharsNum);
			if (nSameMaskCharsNum > 0)
			{
				strReplace = strReplace.Right(nSameMaskCharsNum - 1);
				strReplace += m_chMaskInputTemplate;
			}

			// -------------------------------------------
			// Replace the content with the shifted string
			// -------------------------------------------
			CBCGPEdit::SetSel(nIndex, nIndex+nSameMaskCharsNum);
			CBCGPEdit::ReplaceSel(strReplace, TRUE);
			CBCGPEdit::SetSel(nIndex, nIndex);
			for(i=0; i < strReplace.GetLength(); i++)
			{
				m_str.SetAt(nIndex+i, strReplace[i]);
			}
			
		}
		else // Don't use m_chMaskInputTemplate - delete symbol
		{
			int nBeginOld, nEndOld;
			CBCGPEdit::GetSel(nBeginOld, nEndOld);

			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

			DoUpdate (TRUE, nBeginOld, nEndOld);
		}
	}
	
	// --------------------------------
	// Have one or more chars selected
	// --------------------------------
	else
	{
		if (!m_strInputTemplate.IsEmpty()) // Use m_strInputTemplate
		{
			// ---------------------------------------------------
			// Calc the number of literals with the same mask char
			// ---------------------------------------------------
			ASSERT(nStartPos >= 0);
			ASSERT(nEndPos > 0);
			ASSERT(nStartPos <= m_strInputTemplate.GetLength());

			int nSameMaskCharsNum = 1;
			int nIndex = nStartPos; // an index of the first selected char
			TCHAR chMaskChar = m_strMask[nIndex];
			BOOL bScanMore = TRUE;
			while (bScanMore && (nIndex + nSameMaskCharsNum < nGroupEnd))
			{
				if (m_strMask[nIndex + nSameMaskCharsNum] == chMaskChar)
				{
					nSameMaskCharsNum++;
				}
				else
				{
					bScanMore = FALSE;
				}
			}
			
			// Make sure the selection has the same mask char
			if (nEndPos - nStartPos > nSameMaskCharsNum)
			{
				MessageBeep((UINT)-1);
				CBCGPEdit::SetSel(nIndex, nIndex+nSameMaskCharsNum);
				return;
			}

			// -------------------------------
			// Form the shifted replace string
			// -------------------------------
			ASSERT(nIndex >= nGroupStart);
			ASSERT(nIndex + nSameMaskCharsNum <= nGroupEnd);
			
			CString strReplace = m_str.Mid(nIndex, nSameMaskCharsNum);
			if (nSameMaskCharsNum > 0)
			{
				ASSERT(nStartPos <= m_strInputTemplate.GetLength());
				ASSERT(nEndPos <= m_strInputTemplate.GetLength());
				int nRange = nEndPos - nStartPos;
				ASSERT(nRange>0);
				
				strReplace = strReplace.Right(nSameMaskCharsNum - nRange);
				strReplace += CString(m_chMaskInputTemplate, nRange);
			}

			// -------------------------------------------
			// Replace the content with the shifted string
			// -------------------------------------------
			CBCGPEdit::SetSel(nIndex, nIndex+nSameMaskCharsNum);
			CBCGPEdit::ReplaceSel(strReplace, TRUE);
			CBCGPEdit::SetSel(nIndex, nIndex);
			for(int i=0; i < strReplace.GetLength(); i++)
			{
				m_str.SetAt(nIndex+i, strReplace[i]);
			}
		}
		else
		{
			// Don't use m_chMaskInputTemplate - delete symbols
			int nBeginOld, nEndOld;
			CBCGPEdit::GetSel(nBeginOld, nEndOld);

			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

			DoUpdate (TRUE, nBeginOld, nEndOld);
		}
	}
}

void CBCGPMaskEdit::OnCharDelete(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	ASSERT(m_strMask.IsEmpty() == m_strInputTemplate.IsEmpty());
	ASSERT(m_strMask.GetLength() == m_strInputTemplate.GetLength());

	int nStartPos, nEndPos;
	CBCGPEdit::GetSel(nStartPos, nEndPos);
	ASSERT(nStartPos>=0);
	ASSERT(nEndPos>=0);
	ASSERT(nEndPos>=nStartPos);

	// -----------------
	// Calc group bounds
	// -----------------
	int nGroupStart, nGroupEnd;
	CBCGPEdit::GetSel(nGroupStart, nGroupEnd);
	GetGroupBounds(nGroupStart, nGroupEnd, nGroupStart);

	// ---------------------------------
	// Special processing for Delete All
	// ---------------------------------
	if (nStartPos == 0 && nEndPos > 0 && nEndPos == m_str.GetLength () && !m_strMask.IsEmpty())
	{
		if (SetValue(_T(""), !m_bSetMaskedCharsOnly))
		::SetWindowText (GetSafeHwnd (), m_str);

		if (nGroupStart >= 0)
		{
			SetSel (nGroupStart, nGroupStart);
		}
		return;
	}

	// ------------
	// Out of range
	// ------------
	if ((nStartPos<0) && (nEndPos > m_str.GetLength()) ||
		(nStartPos < nGroupStart) || (nStartPos > nGroupEnd) ||
		(nEndPos < nGroupStart) || (nEndPos > nGroupEnd))
	{
		MessageBeep((UINT)-1);
		CBCGPEdit::SetSel(nGroupStart, nGroupEnd);
		return;
	}

	// -----------------
	// No selected chars
	// -----------------
	if (nStartPos == nEndPos)
	{
		if (!m_strMask.IsEmpty()) // Don't use m_strMask
		{
			// -----------------------------------------------
			// Make sure the cursor is not at the end of group
			// -----------------------------------------------
			if (nEndPos==nGroupEnd)
			{
				MessageBeep((UINT)-1);
				return;
			}

			// ---------------------------------------------------
			// Calc the number of literals with the same mask char
			// ---------------------------------------------------
			ASSERT(nStartPos >= 0);
			ASSERT(nEndPos >= 0);
			ASSERT(nGroupEnd <= m_strInputTemplate.GetLength());

			int nSameMaskCharsNum = 1;
			int nIndex = nStartPos; // an index of the char to delete
			TCHAR chMaskChar = m_strMask[nIndex];
			BOOL bScanMore = TRUE;
			while (bScanMore && (nIndex + nSameMaskCharsNum < nGroupEnd))
			{
				if (m_strMask[nIndex + nSameMaskCharsNum] == chMaskChar)
				{
					nSameMaskCharsNum++;
				}
				else
				{
					bScanMore = FALSE;
				}
			}

			// ---------------------------------
			// Validate new string (dispensable)
			// ---------------------------------
			int i = nIndex;
			for (; (i + nSameMaskCharsNum < nGroupEnd); i++)
			{
				if (m_str[i] != m_chMaskInputTemplate) // allow m_chMaskInputTemplate
				{
					if (!IsMaskedChar(m_str[i], m_strMask[i]))
					{
						MessageBeep((UINT)-1);
						return;
					}
				}
			}
			
			// -----------------------
			// Form the shifted string
			// -----------------------
			ASSERT(nIndex >= nGroupStart);
			ASSERT(nIndex + nSameMaskCharsNum <= nGroupEnd);
			
			CString strReplace = m_str.Mid(nIndex, nSameMaskCharsNum);
			if (nSameMaskCharsNum > 0)
			{
				strReplace = strReplace.Right(nSameMaskCharsNum - 1);
				strReplace += m_chMaskInputTemplate;
			}

			// -------------------------------------------
			// Replace the content with the shifted string
			// -------------------------------------------
			CBCGPEdit::SetSel(nIndex, nIndex+nSameMaskCharsNum);
			CBCGPEdit::ReplaceSel(strReplace, TRUE);
			CBCGPEdit::SetSel(nIndex, nIndex);
			for(i=0; i < strReplace.GetLength(); i++)
			{
				m_str.SetAt(nIndex+i, strReplace[i]);
			}

		}
		else // Don't use m_chMaskInputTemplate - delete symbol
		{
			int nBeginOld, nEndOld;
			CBCGPEdit::GetSel(nBeginOld, nEndOld);

			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

			DoUpdate (TRUE, nBeginOld, nEndOld);
		}
	}

	// --------------------------------
	// Have one or more chars selected
	// --------------------------------
	else
	{
		if (!m_strInputTemplate.IsEmpty()) // Use m_strInputTemplate
		{
			// ---------------------------------------------------
			// Calc the number of literals with the same mask char
			// ---------------------------------------------------
			ASSERT(nStartPos >= 0);
			ASSERT(nEndPos > 0);
			ASSERT(nStartPos <= m_strInputTemplate.GetLength());

			int nSameMaskCharsNum = 1;
			int nIndex = nStartPos; // an index of the first selected char
			TCHAR chMaskChar = m_strMask[nIndex];
			BOOL bScanMore = TRUE;
			while (bScanMore && (nIndex + nSameMaskCharsNum < nGroupEnd))
			{
				if (m_strMask[nIndex + nSameMaskCharsNum] == chMaskChar)
				{
					nSameMaskCharsNum++;
				}
				else
				{
					bScanMore = FALSE;
				}
			}

			// Make sure the selection has the same mask char
			if (nEndPos - nStartPos > nSameMaskCharsNum)
			{
				MessageBeep((UINT)-1);
				CBCGPEdit::SetSel(nIndex, nIndex+nSameMaskCharsNum);
				return;
			}

			// -------------------------------
			// Form the shifted replace string
			// -------------------------------
			ASSERT(nIndex >= nGroupStart);
			ASSERT(nIndex + nSameMaskCharsNum <= nGroupEnd);
			
			CString strReplace = m_str.Mid(nIndex, nSameMaskCharsNum);
			if (nSameMaskCharsNum > 0)
			{
				ASSERT(nStartPos <= m_strInputTemplate.GetLength());
				ASSERT(nEndPos <= m_strInputTemplate.GetLength());
				int nRange = nEndPos - nStartPos;
				ASSERT(nRange>0);
				
				strReplace = strReplace.Right(nSameMaskCharsNum - nRange);
				strReplace += CString(m_chMaskInputTemplate, nRange);
			}

			// -------------------------------------------
			// Replace the content with the shifted string
			// -------------------------------------------
			CBCGPEdit::SetSel(nIndex, nIndex+nSameMaskCharsNum);
			CBCGPEdit::ReplaceSel(strReplace, TRUE);
			CBCGPEdit::SetSel(nIndex, nIndex);
			for(int i=0; i < strReplace.GetLength(); i++)
			{
				m_str.SetAt(nIndex+i, strReplace[i]);
			}
		}
		else
		{
			// Don't use m_chMaskInputTemplate - delete symbols
			int nBeginOld, nEndOld;
			CBCGPEdit::GetSel(nBeginOld, nEndOld);

			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

			DoUpdate (TRUE, nBeginOld, nEndOld);
		}
	}
}

void CBCGPMaskEdit::GetGroupBounds(int &nBegin, int &nEnd, int nStartPos, BOOL bForward)
{
	ASSERT(m_strMask.IsEmpty() == m_strInputTemplate.IsEmpty());
	ASSERT(m_strMask.GetLength() == m_strInputTemplate.GetLength());

	if (!m_strInputTemplate.IsEmpty()) // use mask
	{
		ASSERT(m_str.GetLength() == m_strMask.GetLength());
		ASSERT(nStartPos >= 0);
		ASSERT(nStartPos <= m_strInputTemplate.GetLength());

		if (bForward)
		{
			// ----------------------------------------
			// If nStartPos is in the middle of a group
			// Reverse search for the begin of a group
			// ----------------------------------------
			int i = nStartPos;
			if (nStartPos>0)
			{
				if (m_strInputTemplate[nStartPos-1] == _T('_'))
				{
					do
					{
						i--;
					}
					while ((i>0) && (m_strInputTemplate[i]==_T('_')));
				}
			}
			if (i == m_strInputTemplate.GetLength())
			{
				nBegin = -1; // no group
				nEnd = 0;
				return;
			}

			// --------------------------------------------------
			// i points between groups or to the begin of a group
			// Search for the begin of a group
			// --------------------------------------------------
			if (m_strInputTemplate[i] != _T('_'))
			{
				i = m_strInputTemplate.Find(_T('_'), i);
				if (i == -1)
				{
					nBegin = -1; // no group
					nEnd = 0;
					return;
				}
			}
			nBegin = i;

			// -----------------------------
			// Search for the end of a group
			// -----------------------------
			while((i < m_strInputTemplate.GetLength()) && (m_strInputTemplate[i]==_T('_')))
			{
				i++;
			}
			nEnd = i;
		}

		else // backward
		{
			// ----------------------------------------
			// If nStartPos is in the middle of a group
			// Search for the end of a group
			// ----------------------------------------
			int i = nStartPos;
			while ((i < m_str.GetLength()) && (m_strInputTemplate[i] == _T('_')))
			{
				i++;
			}
			if (i==0)
			{
				nBegin = -1; // no group
				nEnd = 0;
				return;
			}

			// ------------------------------------------------
			// i points between groups or to the end of a group
			// Reverse search for the end of a group
			// ------------------------------------------------
			if (m_strInputTemplate[i-1] != _T('_'))
			{
				do
				{
					i--;
				}
				while ((i>0) && (m_strInputTemplate[i-1] != _T('_')));
				if (i==0)
				{
					nBegin = -1; // no group
					nEnd = 0;
					return;
				}
			}
			nEnd = i;

			// -------------------------------
			// Search for the begin of a group
			// -------------------------------
			do
			{
				i--;
			}
			while ((i>0) && (m_strInputTemplate[i-1]==_T('_')));
			nBegin = i;
		}
	}

	else // don't use mask
	{
		// nStartPos ignored
		nBegin = 0; 
		nEnd = m_str.GetLength();
	}
}

BOOL CBCGPMaskEdit::DoUpdate (BOOL bRestoreLastGood, int nBeginOld, int nEndOld)
{
	if (m_bPasteProcessing)
	{
		return FALSE;
	}

	m_bPasteProcessing = TRUE;

	CString strNew;
	GetWindowText (strNew);

	BOOL bOk = SetValue (strNew, TRUE);
	if (!bOk)
	{
		MessageBeep((UINT)-1);
	}

	if (!bOk && bRestoreLastGood)
	{
		CString strOld = m_str;
		SetWindowText (strOld);

		if (nBeginOld != -1)
		{
			CBCGPEdit::SetSel(nBeginOld, nEndOld);
		}
	}

	m_bPasteProcessing = FALSE;

	return bOk;
}

void CBCGPMaskEdit::OnSetFocusR() 
{
	if (m_bSelectByGroup)
	{
		int nBegin, nEnd;
		GetGroupBounds(nBegin, nEnd, 0, TRUE);
		if (nBegin == -1)
		{
		}
		CBCGPEdit::SetSel(nBegin, nEnd);
	}
	else
	{
		CBCGPEdit::SetSel(0, -1);
	}
}

LRESULT CBCGPMaskEdit::OnPaste (WPARAM, LPARAM)
{
	m_bPasteProcessing = TRUE;

	int nBeginOld, nEndOld;
	CBCGPEdit::GetSel(nBeginOld, nEndOld);

	Default ();

	int nBegin, nEnd;
	CBCGPEdit::GetSel(nBegin, nEnd);
	nEnd = max (nBegin, nEnd);

	CString str;
	CWnd::GetWindowText(str);

	CString strPaste = str.Mid (nBeginOld, nEnd - nBeginOld);
	CString strOld;
	int nLeft = nBeginOld;

	if (m_bSetMaskedCharsOnly)
	{
		strOld = GetMaskedValue();

		if (!m_strMask.IsEmpty ())
		{
			for (int iChar = 0;
				iChar < m_strInputTemplate.GetLength() && iChar < nBeginOld;
				iChar++)
			{
				if (m_strInputTemplate[iChar] != _T('_'))
				{
					nLeft--;
				}
			}
		}
	}
	else
	{
		strOld = GetValue();
	}

	CString strNew = strOld.Left (nLeft) + strPaste;
	BOOL bOverwrite = !m_strMask.IsEmpty ();
	int nRight = nLeft + (bOverwrite ? strPaste.GetLength () : 0);
	if (nRight < strOld.GetLength ())
	{
		strNew += strOld.Mid (nRight);
	}

	if (!SetValue(strNew, !m_bSetMaskedCharsOnly))
	{
		MessageBeep((UINT)-1);
	}

	CWnd::SetWindowText(m_str);

	if (m_bSelectByGroup)
	{
		GetGroupBounds(nBeginOld, nEndOld, nBeginOld, TRUE);
	}
	CBCGPEdit::SetSel(nBeginOld, nBeginOld);

	m_bPasteProcessing = FALSE;

	return 0L;
}

LRESULT CBCGPMaskEdit::OnCut(WPARAM, LPARAM)
{
	m_bPasteProcessing = TRUE;

	int nBeginOld, nEndOld;
	CBCGPEdit::GetSel(nBeginOld, nEndOld);

	Default ();

	CString strNew;
	CWnd::GetWindowText(strNew);

	if (!SetValue(strNew, TRUE))
	{
		MessageBeep((UINT)-1);
	}

	CWnd::SetWindowText(m_str);

	if (m_bSelectByGroup)
	{
		GetGroupBounds(nBeginOld, nEndOld, nBeginOld, TRUE);
	}

	CBCGPEdit::SetSel(nBeginOld, nBeginOld);

	m_bPasteProcessing = FALSE;

	return 0;
}

LRESULT CBCGPMaskEdit::OnClear(WPARAM, LPARAM)
{
	m_bPasteProcessing = TRUE;

	int nBeginOld, nEndOld;
	CBCGPEdit::GetSel(nBeginOld, nEndOld);

	Default ();

	CString strNew;
	CWnd::GetWindowText(strNew);

	if (!SetValue(strNew, TRUE))
	{
		MessageBeep((UINT)-1);
	}

	CWnd::SetWindowText(m_str);

	if (m_bSelectByGroup)
	{
		GetGroupBounds(nBeginOld, nEndOld, nBeginOld, TRUE);
	}

	CBCGPEdit::SetSel(nBeginOld, nBeginOld);

	m_bPasteProcessing = FALSE;

	return 0;
}

BOOL CBCGPMaskEdit::IsDrawPrompt()
{
	if (!m_bHasPrompt || (GetFocus() == this))
	{
		return FALSE;
	}

	return GetMaskedValue(FALSE).IsEmpty();
}
