#if !defined(AFX_BCGPEDITCTRL_H__078F8590_E8CE_4F07_A08A_C41CEBE48D77__INCLUDED_)
#define AFX_BCGPEDITCTRL_H__078F8590_E8CE_4F07_A08A_C41CEBE48D77__INCLUDED_

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
// BCGPEditCtrl.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_EDIT_CTRL

#include "BCGPOutlineParser.h"

#ifndef _BCGPEDIT_STANDALONE
	#include "BCGPScrollBar.h"
#else
	#define CBCGPScrollBar CScrollBar
#endif

#pragma warning (disable : 4100) 
#pragma warning (disable : 4311)
#pragma warning (disable : 4312)

// UAT - undo action type
#define UAT_TYPE		0x000000FF
#define UAT_REASON		0xFFFFFF00

static const DWORD g_dwUATUndefined		= 0;
static const DWORD g_dwUATInsertData	= 0x01;
static const DWORD g_dwUATDeleteData	= 0x02;
static const DWORD g_dwUATIndentData	= 0x04;

static const DWORD g_dwUATTyping		= 0x100;
static const DWORD g_dwUATCut			= 0x200;
static const DWORD g_dwUATPaste			= 0x400;
static const DWORD g_dwUATDelete		= 0x800;
static const DWORD g_dwUATBackspace		= 0x1000;
static const DWORD g_dwUATDragDrop		= 0x2000;
static const DWORD g_dwUATEnter			= 0x4000;
static const DWORD g_dwUATIndent		= 0x8000;
static const DWORD g_dwUATUnindent		= 0x10000;
static const DWORD g_dwUATTab			= 0x20000;
static const DWORD g_dwUATReplace		= 0x40000;


static const DWORD g_dwUAFIns			= 0x1;
static const DWORD g_dwUAFOvr			= 0x2;
static const DWORD g_dwUAFSaved			= 0x4;

// canceled operations
static const DWORD g_dwOpUndo			= 0x1;
static const DWORD g_dwOpRedo			= 0x2;
static const DWORD g_dwOpSetOvrMode		= 0x4;
static const DWORD g_dwOpInvokeIS		= 0x8;
static const DWORD g_dwOpLeft			= 0x10;
static const DWORD g_dwOpRight			= 0x20;
static const DWORD g_dwOpUp				= 0x40;
static const DWORD g_dwOpDown			= 0x80;
static const DWORD g_dwOpPgUp			= 0x100;
static const DWORD g_dwOpPgDn			= 0x200;
static const DWORD g_dwOpHome			= 0x400;
static const DWORD g_dwOpNextWord		= 0x800;
static const DWORD g_dwOpPrevWord		= 0x1000;
static const DWORD g_dwOpPrevIndent		= 0x2000;
static const DWORD g_dwOpInsTab			= 0x4000;
static const DWORD g_dwOpLineConcat		= 0x8000;
static const DWORD g_dwOpDelete			= 0x10000;
static const DWORD g_dwOpPaste			= 0x20000;
static const DWORD g_dwOpCopy			= 0x40000;
static const DWORD g_dwOpCut			= 0x80000;
static const DWORD g_dwOpDelSelText		= 0x100000;
static const DWORD g_dwOpMakeSel		= 0x200000;
static const DWORD g_dwOpIndent			= 0x400000;
static const DWORD g_dwOpDragDrop		= 0x800000;
static const DWORD g_dwOpLineBroken		= 0x1000000;	

// canceled operation reasons
static const DWORD g_dwOpReasonLength	= 0x10000000;
static const DWORD g_dwOpReasonBufEmpty	= 0x20000000;	
static const DWORD g_dwOpReasonReadOnly	= 0x40000000;	
static const DWORD g_dwOpReasonError	= 0x80000000;	


static const DWORD g_dwBCGPEdit_BookMark		= 0x1;
static const DWORD g_dwBCGPEdit_NamedBookMark	= 0x2;
static const DWORD g_dwBCGPEdit_LineColorMarker	= 0x4;
static const DWORD g_dwBCGPEdit_HiliteMarker	= 0x8;
static const DWORD g_dwBCGPEdit_OutlineMarker	= 0x10;
static const DWORD g_dwBCGPEdit_MarkerReserved	= 0x0000FFE0;
static const DWORD g_dwBCGPEdit_FirstUserDefinedMarker = 0x00010000;

// CBCGPOutlineBaseNode flags:
static const DWORD g_dwOBFComplete		= 0x001;
static const DWORD g_dwOBFLeft			= 0x002;
static const DWORD g_dwOBFRight			= 0x004;
// Outline block types:
static const int g_nOBTUndefined			= -2;
static const int g_nOBTUserBlock			= -1;


/////////////////////////////////////////////////////////////////////////////
// CBCGPEditCtrl window

struct BCGCBPRODLLEXPORT BCGP_BASE_UNDO_DATA
{
	virtual ~BCGP_BASE_UNDO_DATA () {}
};

struct BCGCBPRODLLEXPORT BCGP_EDIT_UNDO_ACTION
{
	BCGP_EDIT_UNDO_ACTION ()
	{
		m_nActionOffset = -1;
		m_nCursorOffset = -1;
		m_dwActionType = g_dwUATUndefined;
		m_dwFlags = g_dwUAFIns;
		m_lpActionData = NULL;
		m_bDestroyActionData = FALSE;
		m_bForceNextUndo = FALSE;
	}

	~BCGP_EDIT_UNDO_ACTION ()
	{
		if (m_bDestroyActionData)
		{
			delete m_lpActionData;
		}
	}

	int						m_nActionOffset;
	int						m_nCursorOffset;
	DWORD					m_dwActionType;
	CString					m_strActionText;
	BCGP_BASE_UNDO_DATA*	m_lpActionData;
	DWORD					m_dwFlags;
	BOOL					m_bForceNextUndo;

	BOOL					m_bDestroyActionData;
};

struct BCGCBPRODLLEXPORT BCGP_EDIT_UNDO_INDENT_DATA : public BCGP_BASE_UNDO_DATA
{
	BCGP_EDIT_UNDO_INDENT_DATA () : m_nStartOffset (0), m_nEndOffset (0), m_bForward (FALSE)
	{
	}
	BCGP_EDIT_UNDO_INDENT_DATA (int nStart, int nEnd, int bFwd) :
					m_nStartOffset (nStart), m_nEndOffset (nEnd), m_bForward (bFwd)
	{
	}

	int				m_nStartOffset;
	int				m_nEndOffset;
	BOOL			m_bForward;
	CStringArray	m_arInsDelStrings;
};

struct BCGP_EDIT_COLOR_BLOCK  
{
	BCGP_EDIT_COLOR_BLOCK () :  m_clrForeground ((COLORREF)-1), 
								m_clrBackground ((COLORREF)-1), 
								m_clrForegroundLight ((COLORREF)-1),
								m_bWholeText (FALSE), m_bAllowNestedBlocks (FALSE),
								m_bCaseSensitive (FALSE), m_bWholeWord (FALSE)
	{
	}

	CString		m_strOpen;
	CString		m_strClose;
	COLORREF	m_clrForeground;
	COLORREF	m_clrForegroundLight;	// Used in dark background themes
	COLORREF	m_clrBackground;

	BOOL		m_bWholeText;
	BOOL		m_bAllowNestedBlocks;
	BOOL		m_bCaseSensitive;
	BOOL		m_bWholeWord;

	COLORREF GetForegroundColor(BOOL bIsDarkBackground)
	{
		return (!bIsDarkBackground || m_clrBackground != (COLORREF)-1) ? m_clrForeground : m_clrForegroundLight;
	}

	void SetForegroundColor(COLORREF clr);

	virtual bool operator==(BCGP_EDIT_COLOR_BLOCK& srcBlock)
	{
		return (m_bWholeText == srcBlock.m_bWholeText && 
				m_strOpen == srcBlock.m_strOpen && 
				m_strClose == srcBlock.m_strClose);
	}

	static COLORREF MakeLightColor(COLORREF clrIn);
};

struct BCGCBPRODLLEXPORT BCGP_EDIT_SYNTAX_COLOR
{
	BCGP_EDIT_SYNTAX_COLOR () : m_clrForeground ((COLORREF)-1), 
								m_clrBackground ((COLORREF)-1),
								m_clrForegroundLight ((COLORREF)-1),
								m_bCaseSensitive (TRUE)
	{
	}						

	BCGP_EDIT_SYNTAX_COLOR (COLORREF clrFore, COLORREF clrBk, BOOL bCaseSensitive = TRUE) : 
				m_clrForeground (clrFore), m_clrBackground (clrBk), 
				m_bCaseSensitive (bCaseSensitive)
	{
		m_clrForegroundLight = BCGP_EDIT_COLOR_BLOCK::MakeLightColor(m_clrForeground);
	}

	COLORREF GetForegroundColor(BOOL bIsDarkBackground)
	{
		return (!bIsDarkBackground || m_clrBackground != (COLORREF)-1) ? m_clrForeground : m_clrForegroundLight;
	}

	COLORREF	m_clrForeground;
	COLORREF	m_clrForegroundLight;	// Used in dark background themes
	COLORREF	m_clrBackground;
	BOOL		m_bCaseSensitive;
};

struct BCGP_EDIT_COLOR_AREA 
{
	int			m_nStart;
	int			m_nEnd;
	COLORREF	m_clrForeground;
	COLORREF	m_clrForegroundLight;
	COLORREF	m_clrBackground;

	COLORREF GetForegroundColor(BOOL bIsDarkBackground)
	{
		return (!bIsDarkBackground || m_clrBackground != (COLORREF)-1) ? m_clrForeground : m_clrForegroundLight;
	}

	BCGP_EDIT_COLOR_AREA& operator= (BCGP_EDIT_COLOR_BLOCK& srcBlock)
	{
		m_clrForeground = srcBlock.m_clrForeground;
		m_clrBackground = srcBlock.m_clrBackground;
		m_clrForegroundLight = srcBlock.m_clrForegroundLight;

		return *this;
	}
};

struct BCGP_EDIT_SYM_DEF
{
	BCGP_EDIT_SYM_DEF () : m_nIndex (-1), m_bAnimated (FALSE), m_bEnabled (TRUE),
							m_chReplacement (0)
	{
	}
	CString m_strSymText;
	CString m_strSymSequence;
	int		m_nIndex;
	BOOL	m_bAnimated;
	BOOL	m_bEnabled;
	TCHAR	m_chReplacement;
};

struct	BCGP_EDIT_HYPERLINK_DEF
{
	BCGP_EDIT_HYPERLINK_DEF () : m_nHyperlinkOffset (-1)
	{
	}
	BCGP_EDIT_HYPERLINK_DEF (CString& str, int nOffset) : m_strHyperlink (str), 
														m_nHyperlinkOffset (nOffset)
	{
	}
	BCGP_EDIT_HYPERLINK_DEF& operator= (BCGP_EDIT_HYPERLINK_DEF& srcHyperlink)
	{
		m_strHyperlink = srcHyperlink.m_strHyperlink;
		m_nHyperlinkOffset = srcHyperlink.m_nHyperlinkOffset;

		return *this;
	}

	CString		m_strHyperlink;
	int			m_nHyperlinkOffset;
};

enum CBCGPEditMarkerRange
{
	NOT_IN_RANGE,
	FULL_IN_RANGE,
	PARTIAL_IN_RANGE
};

class BCGCBPRODLLEXPORT CBCGPEditMarker : public CObject
{
	DECLARE_SERIAL(CBCGPEditMarker)
public:
	CBCGPEditMarker () : m_nLine (-1), m_pData (NULL), 
		m_dwMarkerType ((DWORD) -1), m_unPriority (0)
	{

	}

	CBCGPEditMarker (const CBCGPEditMarker& srcMarker)
	{
		Copy (srcMarker);
	}

	~CBCGPEditMarker ()
	{
		DestroyData ();
	}

	virtual CBCGPEditMarker& operator=(const CBCGPEditMarker& srcMarker)
	{
		Copy (srcMarker);
		return *this;
	}

	virtual void Copy (const CBCGPEditMarker& srcMarker)
	{
		m_nLine = srcMarker.m_nLine;
		m_dwMarkerType = srcMarker.m_dwMarkerType;
		m_unPriority = srcMarker.m_unPriority;
		CopyData ((DWORD) srcMarker.m_pData);
	}

	virtual CBCGPEditMarkerRange IsInRange (int nStartPos, int nEndPos) const
	{
		if (m_nLine >= nStartPos && m_nLine <= nEndPos)
		{
			return FULL_IN_RANGE;
		}

		return NOT_IN_RANGE;
	}

	virtual CBCGPEditMarkerRange IsInRangeByOffset (int nStartPos, int nEndPos) const
	{
		return NOT_IN_RANGE;
	}

	virtual void UpdateMarkerForDeletedRange (int /*nStartPos*/, int /*nEndPos*/) {}
	virtual void UpdateMarkerForDeletedRangeByOffset (int /*nStartPos*/, int /*nEndPos*/) {}

	virtual bool CompareData (DWORD dwData) const
	{
		return ((DWORD) m_pData) == dwData;
	}

	virtual void CopyData (DWORD dwData) {}
	virtual void DestroyData () {}

	virtual void Serialize (CArchive& ar)
	{
		CObject::Serialize (ar);
		if (ar.IsLoading ())
		{
			ar >> m_nLine;
			ar >> m_dwMarkerType;
			ar >> m_unPriority;
		}
		else
		{
			ar << m_nLine;
			ar << m_dwMarkerType;
			ar << m_unPriority;
		}

		SerializeData (ar);
	}

	virtual void SerializeData (CArchive& ar)
	{
		if (ar.IsLoading ())
		{
			DWORD dw = 0;
			ar >> dw;
			m_pData = (LPVOID) dw;
		}
		else
		{
			ar << (DWORD) m_pData;
		}
	}

public:	
	int			m_nLine;
	DWORD		m_dwMarkerType;
	UINT		m_unPriority;
	LPVOID		m_pData;
};

class BCGCBPRODLLEXPORT CBCGPLineColorMarker : public CBCGPEditMarker
{
	DECLARE_SERIAL(CBCGPLineColorMarker)

public:	
	CBCGPLineColorMarker () : m_clrForeground ((COLORREF)-1), 
							  m_clrBackground ((COLORREF)-1),
							  m_nLineCount (0), m_bFullLine (FALSE)
	{
		m_dwMarkerType = g_dwBCGPEdit_LineColorMarker;
	}							  

	CBCGPLineColorMarker (const CBCGPLineColorMarker& srcMarker)
	{
		Copy (srcMarker);
	}

	virtual void Copy (const CBCGPLineColorMarker& srcMarker)
	{
		CBCGPEditMarker::Copy (srcMarker);
		m_clrForeground = srcMarker.m_clrForeground;
		m_clrBackground = srcMarker.m_clrBackground;
		m_nLineCount	= srcMarker.m_nLineCount;
		m_bFullLine		= srcMarker.m_bFullLine;
	}

	virtual CBCGPEditMarkerRange IsInRange (int nStartPos, int nEndPos) const
	{
		int nEndLine = m_nLine + m_nLineCount;
		if (m_nLine > nEndPos || nEndLine < nStartPos)
		{
			return NOT_IN_RANGE;
		}

		if (m_nLine >= nStartPos && m_nLine <= nEndPos &&
			nEndLine <= nEndPos)
		{
			return FULL_IN_RANGE;
		}

		return PARTIAL_IN_RANGE;
	}

	virtual void UpdateMarkerForDeletedRange (int nStartPos, int nEndPos)
	{
		int nEndLine = m_nLine + m_nLineCount;

		if (m_nLine > nEndPos || nEndLine < nStartPos)
		{
			return;
		}

		if (m_nLine >= nStartPos && m_nLine <= nEndPos &&
			nEndLine <= nEndPos)
		{
			// the whole marker in the deleted range - nothing to update
			return;
		}
		
		if (m_nLine < nStartPos && nEndLine <= nEndPos)
		{
			// bottom of marker is deleted
			m_nLineCount -= nEndLine - nStartPos + 1;
		}
		else if (m_nLine >= nStartPos && nEndLine > nEndPos)
		{
			// top of marker is deleted
			m_nLineCount -= nEndPos - m_nLine + 1;
			m_nLine = nStartPos;
		}
		else if (m_nLine < nStartPos && nEndLine > nEndPos)
		{
			// middle of the marker is deleted
			m_nLineCount -= nEndPos - nStartPos + 1;
		}
	}

	virtual void Serialize (CArchive& ar)
	{
		CBCGPEditMarker::Serialize (ar);
		if (ar.IsLoading ())
		{
		}
		else
		{
		}
	}

	COLORREF	m_clrForeground;
	COLORREF	m_clrBackground;
	int			m_nLineCount;

	BOOL		m_bFullLine;
};

class BCGCBPRODLLEXPORT CBCGPHiliteMarker : public CBCGPEditMarker
{
	DECLARE_SERIAL(CBCGPHiliteMarker)
public:	
	CBCGPHiliteMarker () : m_nStart (-1),
							m_nEnd (-1),
							m_clrForeground ((COLORREF)-1), 
							m_clrBackground ((COLORREF)-1)
	{
		m_dwMarkerType = g_dwBCGPEdit_HiliteMarker;
	}							  

	CBCGPHiliteMarker (const CBCGPHiliteMarker& srcMarker)
	{
		Copy (srcMarker);
	}

	virtual void Copy (const CBCGPHiliteMarker& srcMarker)
	{
		CBCGPEditMarker::Copy (srcMarker);
		m_nStart		= srcMarker.m_nStart;
		m_nEnd			= srcMarker.m_nEnd;
		m_clrForeground = srcMarker.m_clrForeground;
		m_clrBackground = srcMarker.m_clrBackground;
	}

	virtual CBCGPEditMarkerRange IsInRange (int nStartPos, int nEndPos) const
	{
		return NOT_IN_RANGE;
	}

	virtual CBCGPEditMarkerRange IsInRangeByOffset (int nStartPos, int nEndPos) const
	{
		if (m_nStart > nEndPos || m_nEnd < nStartPos)
		{
			return NOT_IN_RANGE;
		}

		if (m_nStart >= nStartPos && m_nStart <= nEndPos &&
			m_nEnd <= nEndPos)
		{
			return FULL_IN_RANGE;
		}

		return PARTIAL_IN_RANGE;
	}

	virtual void UpdateMarkerForDeletedRange (int /*nStartPos*/, int /*nEndPos*/) {}
	virtual void UpdateMarkerForDeletedRangeByOffset (int nStartPos, int nEndPos)
	{
		if (m_nStart > nEndPos || m_nEnd < nStartPos)
		{
			return;
		}

		if (m_nStart >= nStartPos && m_nStart <= nEndPos &&
			m_nEnd <= nEndPos)
		{
			// the whole marker in the deleted range - nothing to update
			return;
		}
		
		if (m_nStart < nStartPos && m_nEnd <= nEndPos)
		{
			// bottom of marker is deleted
			m_nEnd = nEndPos;
		}
		else if (m_nStart >= nStartPos && m_nEnd > nEndPos)
		{
			// top of marker is deleted
			m_nStart = nStartPos;
		}
		// the middle of the marker is deleted - do nothing
	}

	virtual void Serialize (CArchive& ar)
	{
		CBCGPEditMarker::Serialize (ar);
		if (ar.IsLoading ())
		{
			ar >> m_nStart;
			ar >> m_nEnd;
		}
		else
		{
			ar << m_nStart;
			ar << m_nEnd;
		}
	}

	int			m_nStart;
	int			m_nEnd;
	COLORREF	m_clrForeground;
	COLORREF	m_clrBackground;
	//COLORREF	m_clrSelForeground;
	//COLORREF	m_clrSelBackground;
};

// ----------
// Outlining:
// ----------

struct BCGCBPRODLLEXPORT BCGP_EDIT_OUTLINE_CHANGES
{
	BCGP_EDIT_OUTLINE_CHANGES () :
					m_nStartOffset (0), m_nEndOffset (0),
					m_bDestroyRemData (TRUE), m_bDestroyInsData (FALSE)
	{
	}
	BCGP_EDIT_OUTLINE_CHANGES (int nStart, int nEnd) :
					m_nStartOffset (nStart), m_nEndOffset (nEnd),
					m_bDestroyRemData (TRUE), m_bDestroyInsData (FALSE)
	{
	}
	virtual ~BCGP_EDIT_OUTLINE_CHANGES ()
	{
		if (m_bDestroyInsData)
		{
			while (!m_lstInserted.IsEmpty ())
			{
				delete m_lstInserted.RemoveHead ();
			}
		}
		if (m_bDestroyRemData)
		{
			while (!m_lstRemoved.IsEmpty ())
			{
				delete m_lstRemoved.RemoveHead ();
			}
		}
	}

	CObList	m_lstRemoved;
	CObList	m_lstInserted;
	int		m_nStartOffset;
	int		m_nEndOffset;

	BOOL	m_bDestroyRemData;
	BOOL	m_bDestroyInsData;

};

struct BCGCBPRODLLEXPORT BCGP_EDIT_UNDO_OUTLINE_DATA : public BCGP_BASE_UNDO_DATA
{
	BCGP_EDIT_UNDO_OUTLINE_DATA () : 
					m_pData (NULL), m_bForward (FALSE), m_bDestroyData (FALSE)
	{
	}
	BCGP_EDIT_UNDO_OUTLINE_DATA (BCGP_EDIT_OUTLINE_CHANGES*	pChanges, BOOL bForward = FALSE) :
					m_pData (pChanges), m_bForward (bForward), m_bDestroyData (FALSE)
	{
	}
	virtual ~BCGP_EDIT_UNDO_OUTLINE_DATA ()
	{
		if (m_bDestroyData)
		{
			delete m_pData;
		}
	}

	BCGP_EDIT_OUTLINE_CHANGES*	m_pData;
	BOOL						m_bForward;
	BOOL						m_bDestroyData;
};

class BCGCBPRODLLEXPORT CBCGPOutlineBaseNode: public CObject
{
	DECLARE_SERIAL (CBCGPOutlineBaseNode)
	typedef CBCGPEditMarkerRange CBCGPEditOutlineRange;

	// Data:
public:
	int			m_nStart;
	int			m_nEnd;
	int			m_nNameOffset;
	BOOL		m_bCollapsed;
	CString		m_strReplace;
	CRect		m_rectTool;
	int			m_nBlockType;	// g_nOBTUndefined, g_nOBTUserBlock or valid block type index
	DWORD		m_dwFlags;		// g_dwOBFComplete, g_dwOBFLeft, g_dwOBFRight
	LPVOID		m_pData;
	
	static BOOL	m_bCollapsedDefault;

public:
	CBCGPOutlineBaseNode () : m_nStart (-1),
							   m_nEnd (-1),
							   m_nNameOffset (0),
							   m_strReplace (_T("...")),
							   m_nBlockType (g_nOBTUndefined),
							   m_pData (NULL)
	{
		m_bCollapsed	= m_bCollapsedDefault;
		m_dwFlags		= g_dwOBFComplete;
		m_rectTool.SetRectEmpty ();
	}

	CBCGPOutlineBaseNode (const CBCGPOutlineBaseNode& srcBlock)
	{
		Copy (srcBlock);
	}

	virtual ~CBCGPOutlineBaseNode ()
	{
		DestroyData ();
	}

	virtual CBCGPOutlineBaseNode& operator=(const CBCGPOutlineBaseNode& srcBlock)
	{
		Copy (srcBlock);
		return *this;
	}

	virtual	void Copy (const CBCGPOutlineBaseNode& srcBlock)
	{
		m_nStart		= srcBlock.m_nStart;
		m_nEnd			= srcBlock.m_nEnd;
		m_nNameOffset	= srcBlock.m_nNameOffset;
		m_bCollapsed	= srcBlock.m_bCollapsed;
		m_strReplace	= srcBlock.m_strReplace;
		m_rectTool.SetRectEmpty ();
		m_nBlockType	= srcBlock.m_nBlockType;
		m_dwFlags		= srcBlock.m_dwFlags;
		CopyData ((DWORD) srcBlock.m_pData);
	}
	
	virtual void CopyData (DWORD dwData) {}
	virtual void DestroyData ()
	{
		m_pData = NULL;
	}

	virtual void Serialize (CArchive& ar)
	{
		CObject::Serialize (ar);
		if (ar.IsLoading ())
		{
			ar >> m_nStart;
			ar >> m_nEnd;
			ar >> m_nNameOffset;
			ar >> m_bCollapsed;
			ar >> m_strReplace;
			ar >> m_nBlockType;
			ar >> m_dwFlags;
			m_rectTool.SetRectEmpty ();
		}
		else
		{
			ar << m_nStart;
			ar << m_nEnd;
			ar << m_nNameOffset;
			ar << m_bCollapsed;
			ar << m_strReplace;
			ar << m_nBlockType;
			ar << m_dwFlags;
		}

		SerializeData (ar);
	}

	virtual void SerializeData (CArchive& ar)
	{
		if (ar.IsLoading ())
		{
			DWORD dw = 0;
			ar >> dw;
			m_pData = (LPVOID) dw;
		}
		else
		{
			ar << (DWORD) m_pData;
		}
	}

	virtual void Move (int nDelta)
	{
		m_nStart += nDelta;
		m_nEnd += nDelta;
	}

	virtual CBCGPEditOutlineRange IsInRangeByOffset (int nStartPos, int nEndPos) const
	{
		if (m_nStart > nEndPos || m_nEnd < nStartPos)
		{
			return NOT_IN_RANGE;
		}

		if (m_nStart >= nStartPos && m_nStart <= nEndPos &&
			m_nEnd <= nEndPos)
		{
			return FULL_IN_RANGE;
		}

		return PARTIAL_IN_RANGE;
	}
	
	virtual void UpdateBlocksForDeletedRange (int nStart, int nEnd)
	{
		if ((m_nStart == -1 && m_nEnd == -1) || 
			m_nEnd < nStart)
		{
			return;
		}

		if (m_nStart > nEnd)
		{
			if (m_nStart - m_nNameOffset <= nEnd)
			{
				// update block name offset:
				if (m_nStart - m_nNameOffset <= nStart)
				{
					// the middle of the block name is deleted
					m_nNameOffset = max (0, m_nNameOffset - (nEnd - nStart));
				}
				else
				{
					// top of the block name is deleted
					m_nNameOffset = m_nStart - nEnd;
				}
			}

			Move (nStart - nEnd);
			return;
		}

		if (m_nStart >= nStart && m_nStart <= nEnd &&
			m_nEnd <= nEnd)
		{
			// the whole block is in the deleted range - nothing to update
			return;
		}
		
		if (m_nStart < nStart && m_nEnd <= nEnd)
		{
			// bottom of block is deleted
			m_nEnd = nStart - 1;
		}
		else if (m_nStart >= nStart && m_nEnd > nEnd)
		{
			// top of block is deleted
			m_nStart = nStart;

			if (m_nNameOffset > 0)
			{
				// update block name offset
				m_nNameOffset = max (0, nStart - m_nStart + m_nNameOffset);
			}

			m_nEnd -= nEnd - nStart;
		}
		else
		{
			// the middle of the block is deleted - update bottom
			m_nEnd -= nEnd - nStart;
		}
	}

	virtual void UpdateBlocksForInsertedText (int nOffset, int nCount)
	{
		ASSERT (nCount >= 0);

		if (m_nStart == -1 && m_nEnd == -1)
		{
			return;
		}

		if (m_nEnd < nOffset)
		{
			// text is inserted after the block - nothing to update
			return;
		}

		if (m_nStart >= nOffset)
		{
			if (m_nStart - m_nNameOffset < nOffset)
			{
				// update block name offset
				m_nNameOffset += nCount;
			}

			Move (nCount);
		}
		else if (m_nStart < nOffset && nOffset <= m_nEnd)
		{
			// the inserted text is in the middle of the block - update bottom
			m_nEnd += nCount;
		}
	}

	virtual void Collapse (BOOL bCollapsed = TRUE)
	{
		if ((m_dwFlags & g_dwOBFComplete) != 0)
		{
			m_bCollapsed = bCollapsed;
		}
	}
	
	BOOL IsCollapsed () const
	{
		return m_bCollapsed;
	}
};

class CBCGPOutlineParser;
class CBCGPEditCtrl;
class BCGCBPRODLLEXPORT CBCGPOutlineNode : public CBCGPOutlineBaseNode
{
	friend class CBCGPOutlineParser;
	DECLARE_SERIAL(CBCGPOutlineNode)

	// Data:
protected:
	CObList					m_lstNodes;
	CBCGPOutlineNode*		m_pParentNode;
	CBCGPEditCtrl*			m_pEditCtrl;

public:
	CBCGPOutlineNode(): CBCGPOutlineBaseNode (), m_pParentNode (NULL),
		m_pEditCtrl (NULL)
	{
	}
	CBCGPOutlineNode(const CBCGPOutlineBaseNode& srcBlock)
		: CBCGPOutlineBaseNode (srcBlock), m_pParentNode (NULL)
	{
	}
	virtual ~CBCGPOutlineNode () 
	{
		DeleteAllBlocks ();
		m_pParentNode = NULL;
	}

	void SetOwnerEditCtrl (CBCGPEditCtrl* pOwner)
	{
		m_pEditCtrl = pOwner;
	}

	const CBCGPEditCtrl* GetOwnerEditCtrl () const
	{
		return m_pEditCtrl;
	}

	// Functions:
public:
	CBCGPOutlineBaseNode*	GetValue ()				{return this;}
	CObList*				GetNodes ()				{return &m_lstNodes;}
	CBCGPOutlineNode*		GetParentNode () const	{return m_pParentNode;}

	virtual CBCGPOutlineNode* AddBlock (const CBCGPOutlineBaseNode& srcNode, 
										BCGP_EDIT_OUTLINE_CHANGES& changes)
	{
		CBCGPOutlineNode* pNewNode = new CBCGPOutlineNode (srcNode);

		return AddNode (pNewNode, changes);
	}
	
	void GetAllBlocks (CObList& lstBlocks, BOOL bRecursive = TRUE) const;
	void DeleteAllBlocks ()
	{
		while (!m_lstNodes.IsEmpty ())
		{
			delete m_lstNodes.RemoveHead ();
		}
	}

	int GetCount (BOOL bRecursive = TRUE) const
	{
		if (bRecursive)
		{
			int nCount = 0;
			for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL; nCount++)
			{
				CBCGPOutlineNode* pSubNode = DYNAMIC_DOWNCAST(CBCGPOutlineNode, m_lstNodes.GetNext (pos));
				ASSERT_VALID (pSubNode);
				nCount += pSubNode->GetCount (bRecursive);
			}
			return nCount;
		}
		else
		{
			return (int) m_lstNodes.GetCount ();
		}
	}

	// task-oriented:
public:
	CBCGPOutlineNode*	FindCollapsedBlock (int nOffset) const;
	CBCGPOutlineNode*	FindCollapsedBlockInRange (int nStart, int nEnd, BOOL bForward = TRUE) const;
	CBCGPOutlineNode*	FindCollapsedBlockByPoint (CPoint point, int nStart, int nEnd) const;
	CBCGPOutlineNode*	GetInnermostBlock (int nOffset) const;
	void				GetBlocksInRange (int nStart, int nEnd, CObList& lstOutlineBlocks, 
										  BOOL bFullInRangeOnly = TRUE, BOOL bRecursive = TRUE) const;
	void				GetBlocksByState (CObList& lstBlocks, BOOL bCollapsed, BOOL bRecursive = TRUE) const;
	void				GetBlocksByStateInRange (int nStart, int nEnd, CObList& lstBlocks, BOOL bCollapsed,
												BOOL bFullInRangeOnly = TRUE, BOOL bRecursive = TRUE) const;
	BOOL				DeleteInnermostBlock (int nOffset, BCGP_EDIT_OUTLINE_CHANGES& changes);
	int					DeleteBlocksInRange (int nStart, int nEnd, BCGP_EDIT_OUTLINE_CHANGES& changes);
	int					ToggleOutliningInRange (int nStart, int nEnd);
	int					ToggleAllOutlining ();

	virtual void Serialize (CArchive& ar)
	{
		CBCGPOutlineBaseNode::Serialize (ar);

		SerializeNodes (ar);
	}
	
	virtual void SerializeNodes (CArchive& ar)
	{
		m_lstNodes.Serialize (ar);

		if (ar.IsLoading ())
		{
		}
		else
		{
			for (POSITION pos = m_lstNodes.GetHeadPosition (); pos != NULL; )
			{
				CBCGPOutlineNode* pSubNode = DYNAMIC_DOWNCAST(CBCGPOutlineNode, m_lstNodes.GetNext (pos));
				ASSERT_VALID (pSubNode);
				pSubNode->m_pParentNode = this;
			}
		}
	}

	virtual void Collapse (BOOL bCollapsed = TRUE);

	int		Print (CString &str, int nLevel = 0, int nCount = 0) const;
	void	Verify () const;

	virtual void UpdateBlocksForDeletedRange (int nStart, int nEnd);
	virtual void UpdateBlocksForInsertedText (int nOffset, int nCount);

protected:
	virtual CBCGPOutlineNode* AddNode (CBCGPOutlineNode* pNewNode, BCGP_EDIT_OUTLINE_CHANGES& changes);
	virtual CBCGPOutlineNode* RemoveNode (POSITION posNode, BCGP_EDIT_OUTLINE_CHANGES& changes, BOOL bRemoveSubNodes = FALSE);

	void GetTopBlocksInRange (int nStart, int nEnd, CObList& lstBlocks) const;
	static void ToggleOutliningInternal (CObList &lstBlocks);
};

class COutlineCasheManager: public CObject
{
public:
	COutlineCasheManager ();
	virtual ~COutlineCasheManager ();

// Methods:
	CBCGPOutlineNode* Lookup (int nOffset, BOOL& bIsActual);
	void Rebuild (const CBCGPOutlineNode* pNodes, int nOffset);

	void UpdateEvent ()
	{
		m_bUpdateEvent = TRUE;
	}

	void SetCacheRange (int nValue = 1000)
	{
		m_nCacheRange = nValue;
	}

// Implementation:
protected:	
	void SetAt (int nOffsetStart, int nOffsetEnd, CObList& lstBlocks);
	void Clear ();

private:
	void DoSetAt (int nLeft, int nCur, CBCGPOutlineNode* pOutlineNode);
	
// Data:
protected:
	BOOL	m_bUpdateEvent;	// if TRUE, it is necessary to Rebuild
	int		m_nCacheRange;	// by default 1000

	int		m_nCachedRangeStart;
	int		m_nCachedRangeSize;
	CArray <CBCGPOutlineNode*, CBCGPOutlineNode*> m_array;
};


class CBCGPIntelliSenseWnd;
class CBCGPEditCtrl;
class CBCGPOutlineParser;

class BCGCBPRODLLEXPORT CBCGPEditCtrlDropTarget : public COleDropTarget
{
public:
	CBCGPEditCtrlDropTarget()
	{
		m_pOwner = NULL;
	}

// Attributes
protected:
	CBCGPEditCtrl* m_pOwner;

// Operations
public:
	BOOL Register (CBCGPEditCtrl* pOwner);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPTabDropTarget)
	public:
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropList, CPoint point);
	//}}AFX_VIRTUAL
};

class BCGCBPRODLLEXPORT CBCGPIntelliSenseData : public CObject
{
public:
	CString		m_strItemName;
	CString		m_strItemHelp;
	int			m_nImageListIndex;
	DWORD_PTR	m_dwData;
};

class BCGCBPRODLLEXPORT CBCGPEditCtrl : public CWnd
{
	friend class CBCGPIntelliSenseWnd;
	friend class CBCGPIntelliSenseLB;

	DECLARE_DYNAMIC(CBCGPEditCtrl)

// Construction
public:
	CBCGPEditCtrl();

// Attributes
public:
	CToolTipCtrl* m_pToolTip;

	CString		m_strWordDelimeters;
	CString		m_strSpecialDelimiters;
	CString		m_strNonSelectableChars;
	CString		m_strIntelliSenseChars;
	CString		m_strIndentChars;

	CString		m_strContinueStringSymbols;

	// Options
	COLORREF	m_clrBack;
	COLORREF	m_clrText;

	COLORREF	m_clrBackSelActive;
	COLORREF	m_clrBackSelInActive;
	COLORREF	m_clrTextSelActive;
	COLORREF	m_clrTextSelInActive;

	COLORREF	m_clrBackOutline;
	COLORREF	m_clrLineOutline;
	COLORREF	m_clrBackLineNumber;
	COLORREF	m_clrTextLineNumber;
	COLORREF	m_clrBackSidebar;

	BOOL		m_bReplaceTabsAndEOLOnCopy;

	int			m_nLineVertSpacing;
	int			m_nLeftMarginWidth;
	int			m_nLineNumbersMarginWidth;
	int			m_nOutlineMarginWidth;
	int			m_nIndentSize;
	int			m_nTabSize;

	int			m_nScrollRightOffset; // for how many symbols to scroll the text when scrolling
									  // right - in percents from the window width
	int			m_nScrollLeftOffset;  // for how many symbols to scroll the text when scrolling
									  // left
	UINT		m_nScrollTimeOut;

	BOOL		m_bEnableWholeTextCopy;	// Allow to copy the whole buffer to clipboard
	BOOL		m_bEnableCurrentLineCopy;// Allow to copy the current line to clipboard

	UINT		m_nDlgCode;

	BOOL		m_bDefaultIndent;

	BOOL		m_bSaveFileInUnicode;
	BOOL		m_bCopyRTFToClipboard;

	// graphical symbols support
	CList<BCGP_EDIT_SYM_DEF, BCGP_EDIT_SYM_DEF&> m_lstSymDefs;	
	BOOL		m_bEnableSymSupport;

	// hyperlink support
	BOOL		m_bColorHyperlink;
	COLORREF	m_clrHyperlink;
	CStringList m_lstURLPrefixes;

protected:
	CString		m_strBuffer;
	BOOL		m_bIsModified;

	int			m_nMaxScrollWidth;
	
	BOOL		m_bReadOnly;

	int			m_nLastKey;
	int			m_nPrevKey;

	HFONT		m_hFont;
	CFont		m_fontUnderline;
	int			m_nLineHeight;
	int			m_nMaxCharWidth;
	int			m_nAveCharWidth;

	CRect		m_rectView;
	CRect		m_rectText;

	int			m_nCurrOffset;
	CPoint		m_ptCaret;
	BOOL		m_bDisableSetCaret;

	int			m_iStartSel;
	int			m_iEndSel;

	CPoint		m_ptStartBlockSel;
	CPoint		m_ptEndBlockSel;

	BOOL		m_bAltPressedForBlockSel;

	BOOL		m_bBlockSelectionMode;

	BOOL		m_bEnableLineNumbersMargin;
	BOOL		m_bEnableOutlineMargin;
	int			m_nLineNumbersMarginAutoWidth;

	int			m_nScrollOffsetHorz;
	int			m_nScrollOffsetVert;
	int			m_nScrollHiddenVert;

	int			m_nTopVisibleOffset;
	int			m_nBottomVisibleOffset; // only used when printing
	int			m_nTotalLines;
	int			m_nHiddenLines;

	CSize		m_szTotalScroll;
	int			m_nScrollMouseWheelSpeed; // to increase rolling speed
	BOOL		m_bScrollVertEmptyPage;
	BOOL		m_bScrollBarHorz;

	// Scrollbars
	CBCGPScrollBar	m_wndScrollVert;		// Vertical scroll bar
	CBCGPScrollBar	m_wndScrollHorz;		// Horizontal scroll bar

#ifndef _BCGPEDIT_STANDALONE
	CBCGPScrollBar::BCGPSB_STYLE m_ScrollBarStyle;	// Scroll bars style
#endif

	int			m_nCurRow;
	int			m_nCurColumn;

	int			m_nSavedOffset;
	int			m_nDropOffset;
	CPoint		m_ptSavedBlockCaretOffset;

	int			m_nLastSelOffset; // to prevent flashing on MouseMove + sel
	int			m_nLastMaxColumn;

	BOOL		m_bDragTextMode;

	INT_PTR		m_nScrollTimer;

	int			m_nTabLogicalSize; // size of the tab in logical units
	BOOL		m_bKeepTabs;

	CMap<TCHAR, TCHAR&, int, int> m_mapCharWidth;

	CBCGPEditCtrlDropTarget	m_DropTarget;

	// tooltips
	BOOL		m_bEnableToolTips;
	CString		m_strLastDisplayedToolTip;
	BOOL		m_bToolTipCleared;
	int			m_nMaxToolTipWidth;

	// intellisense
	CBCGPIntelliSenseWnd*	m_pIntelliSenseWnd;
	CImageList*				m_pIntelliSenseImgList;
	CFont*					m_pIntelliSenseLBFont;
	BOOL					m_bIntelliSenseMode;

	// undo
	CList <BCGP_EDIT_UNDO_ACTION*, BCGP_EDIT_UNDO_ACTION*>	m_lstUndoActions;
	int			m_nUndoBufferSize;
	POSITION	m_posUndoBuffer;
	POSITION	m_posDocSavedUndoPos;
	BOOL		m_bSavedUndo;
	DWORD		m_dwLastUndoReason;

	BOOL		m_bUndoCharMode; // TRUE - save single chars in each node, FALSE - save words
	BOOL		m_bEOL;
	BOOL		m_bBOL;
	BOOL		m_bIsLastActionUndo;

	// syntax coloring
	CMap  <CString, LPCTSTR, BCGP_EDIT_SYNTAX_COLOR, BCGP_EDIT_SYNTAX_COLOR&>	m_mapWordColors;
	CList <BCGP_EDIT_COLOR_BLOCK, BCGP_EDIT_COLOR_BLOCK&>						m_lstColorBlocks;
	CStringList	m_lstEscapeSequences;

	COLORREF	m_clrOpenBlockFore;
	COLORREF	m_clrOpenBlockBk;
	int			m_nOpenBlockOffset;
	
	int			m_nColorBlockStrLenMax;	// by default _MAX_COLOR_BLOCK_STR_LEN

	// markers
	CObList		m_lstMarkers;
	BOOL		m_bGradientMarkers;

	// graphical symbols support
	CImageList*	m_pSymbolsImgList;

	// outlining
	CBCGPOutlineNode	m_OutlineNodes;
	CBCGPOutlineNode*	m_pOutlineNodeCurr;
	COutlineCasheManager m_OutlineManager;
	CBCGPOutlineParser*	m_pOutlineParser;
	BOOL				m_bOutlineParserEnabled;

	BOOL				m_bEnableOutlining;
	BOOL				m_bAutoOutlining;
	int					m_nMaxHintCharsNum;

	// hyperlink support
	BOOL		m_bEnableHyperlinkSupport;
	
	CArray <BCGP_EDIT_HYPERLINK_DEF, BCGP_EDIT_HYPERLINK_DEF&>	m_arrHyperlinks;

	int			m_nCurrHyperlink;
	int			m_nCurrHyperlinkHot;

	// print
	HFONT		m_hPrinterFont;	// if NULL, mirror display font
	HFONT		m_hMirrorFont;	// font object used when mirroring

	BOOL		m_bIsDarkBackground;
	COLORREF	m_clrHyperlinkLight;

private:
	BOOL		m_bIntelliSenseSupport;
	static BOOL	m_bOvrMode; // overwrite mode

	BOOL		m_bIsBorder;
	
// Operations
public:

	int GetCurRow () const
	{
		return RowFromOffset (m_nCurrOffset); //m_nCurRow;
	}

	int GetCurColumn () const
	{
		return m_nCurColumn;
	}

	int GetCurOffset () const
	{
		return m_nCurrOffset;
	}

	// -------
	// Options
	// -------
	int GetTabSize () const
	{
		return m_nTabSize;
	}

	void SetTabSize (int nTabSize)
	{
		ASSERT (nTabSize > 0);
		m_nTabSize = nTabSize;
		m_mapCharWidth.RemoveAll ();
	}
	
	int GetLineHeight () const
	{
		return m_nLineHeight;
	}

	void SetReadOnly (BOOL bReadOnly = TRUE)
	{
		m_bReadOnly = bReadOnly;
	}

	BOOL IsReadOnly () const
	{
		return m_bReadOnly;
	}

	// ----------------
	// File operations:
	// ----------------
	BOOL OpenFile (const CString& strFileName);
	BOOL OpenFile (CFile& file);

	BOOL SaveFile (const CString& strFileName);
	BOOL SaveFile (CFile& file);

	virtual BOOL IsModified () const
	{
		return m_bIsModified;
	}
	
	void SetModified (BOOL bIsModified = TRUE)
	{
		m_bIsModified = bIsModified;
	}

	// Export buffer
	HGLOBAL ExportBuffer (LPCTSTR lpszText, int nTextLen, BOOL bReplaceTabs = TRUE, 
			 			  BOOL bReplaceEOL = TRUE, BOOL bForceAnsi = FALSE);
	
	void ExportToHTML (CString& strBuffer);
	void ExportToRTF (CString& strBuffer);

	// ------------------
	// Scrolling methods:
	// ------------------
	BOOL ScrollText (BOOL bDirection);

	BOOL ScrollDown (int fnBar, BOOL bRedrawOnScroll);
	BOOL ScrollUp (int fnBar, BOOL bRedrawOnScroll);

	BOOL ScrollPageDown (int fnBar, BOOL bRedrawOnScroll);
	BOOL ScrollPageUp (int fnBar, BOOL bRedrawOnScroll);

	BOOL ScrollHome (int fnBar, BOOL bRedrawOnScroll);
	BOOL ScrollEnd (int fnBar, BOOL bRedrawOnScroll);
	
	void SetMouseWheelSpeed (int nSteps = 1);

#ifndef _BCGPEDIT_STANDALONE
	void SetScrollBarsStyle (CBCGPScrollBar::BCGPSB_STYLE style)
	{
		m_ScrollBarStyle = style;

		m_wndScrollVert.SetVisualStyle (style);
		m_wndScrollHorz.SetVisualStyle (style);
	}

	CBCGPScrollBar::BCGPSB_STYLE GetScrollBarsStyle () const
	{
		return m_ScrollBarStyle;
	}
#endif

	void SetScrollVertEmptyPage (BOOL bEmptyPage)
	{
		m_bScrollVertEmptyPage = bEmptyPage;
	}

	// ------------------
	// Searching methods:
	// ------------------
	virtual void FindWordStartFinish (int nCaretOffset, const CString& strBuffer,
		                      int& nStartOffset, int& nEndOffset, BOOL bSkipSpaces = TRUE) const;
	BOOL GetWordFromPoint (CPoint pt, CString& strWord);
	BOOL GetWordFromOffset (int nOffset, CString& strWord);
	
	int HitTest (CPoint& pt, BOOL bNormalize = FALSE, BOOL bIgnoreTextBounds = FALSE);
	BOOL OffsetToPoint (int nOffset, CPoint& pt, LPPOINT ptRowColumn = NULL,
					LPINT pnScrollOffset = NULL);

	int GetNumOfCharsInText (const CString& strText, TCHAR ch);
	int GetNumOfCharsInText (int nStart, int nEnd, TCHAR ch, BOOL bSkipHidden = FALSE) const;

	//-----------------
	// Syntax coloring:
	//-----------------
	void SetWordColor (CString strWord, 
						COLORREF clrFrgnd = -1, 
						COLORREF clrBkgnd = -1,
						BOOL bCaseSensitive = TRUE);
	BOOL RemoveWordFromColorTable (CString strWord);
	void RemoveAllWordsFromColorTable ();

	POSITION SetBlockColor (CString strBlockOpen, CString strBlockClose, 
							BOOL bWholeText = FALSE, 
							COLORREF clrFrgnd = (COLORREF) -1, 
							COLORREF clrBkgnd = (COLORREF) -1,
							BOOL bCaseSensitive = FALSE,
							BOOL bWholeWord = FALSE);

	BOOL RemoveBlockColor (POSITION posBlock);
	void RemoveAllBlocks ();

	BOOL IsInBlock (int nPos, TCHAR chOpen, TCHAR chClose, int& nBlockStart, int& nBlockEnd);

	void SetColorBlockStrLenMax (int nValue);

	void AddEscapeSequence (LPCTSTR lpszStr);
	BOOL IsEscapeSequence (const CString& strBuffer, int nOffset, BOOL bDirForward = TRUE) const;

	// -------------
	// Intellisense:
	// -------------
	inline BOOL IsIntelliSenseEnabled() const
	{
		return m_bIntelliSenseSupport;
	}

	void SetIntelliSenseImgList (CImageList* pImageList, BOOL bAutoDestroy = FALSE)
	{
		m_pIntelliSenseImgList = pImageList;
	}

	CImageList* GetIntelliSenseImgList () const
	{
		return m_pIntelliSenseImgList;
	}

	void SetIntelliSenseLBFont (CFont* pFont) 
	{
		m_pIntelliSenseLBFont = pFont;
	}

	BOOL EnableIntelliSense(BOOL bFl = TRUE);

	CFont* GetIntelliSenseLBFont () const
	{
		return m_pIntelliSenseLBFont;
	}

	void SetIntelliSenseWnd (CBCGPIntelliSenseWnd* pWnd)
	{
		m_pIntelliSenseWnd = pWnd;
	}

	// ----------------
	// Line operations:
	// ----------------
	int GetLineCount () const
	{
		return m_nTotalLines + 1;
	}
	
	int GetLine (int nLineIndex, LPTSTR lpszBuffer) const;
	int GetLine (int nLineIndex, LPTSTR lpszBuffer, int nMaxLength) const;
	BOOL GetCurLine (CString& strLine) const;
	int GetFirstVisibleLine () const;
	int LineIndex (int nLine = -1) const;
	int LineFromChar (int nIndex) const;
	int LineLength (int nLine = -1) const;
	void LineScroll (int nLines, int nChars = 0);

	// ------------------
	// Selection methods:
	// ------------------
	void GetSel (int& nStartChar, int& nEndChar) const;
	BOOL IsPointInSelectionRect (CPoint pt);
	BOOL SetCaret (int nOffset, BOOL bScrollToCaret = TRUE, BOOL bRedrawOnScroll = TRUE);

	void SelectLine (int nLine, BOOL bNoScroll = FALSE, BOOL bRedraw = TRUE);
	void SelectLineRange (int nFromLine, int nToLine, BOOL bNoScroll = FALSE, BOOL bRedraw = TRUE);
	
	CString GetSelText () const;
	void ReplaceSel (LPCTSTR lpszNewText, BOOL bCanUndo = FALSE);

	// --------------
	// Insert/delete:
	// --------------
	BOOL DeleteSelectedText (BOOL bRedraw = TRUE, BOOL bCopyToClipboard = FALSE, 
							 BOOL bForceNextUndo = FALSE);
	BOOL DeleteText (int nStartOffset, int nEndOffset, BOOL bRedraw = FALSE);

	BOOL IndentText (int nStartOffset, int nEndOffset, BOOL bForward, 
					BOOL bBlockSelectionMode, BOOL bSelectText, 
					CStringArray& arInsDelText, BOOL bUndoMode);
	
	const CString& GetText () const {return m_strBuffer;}
	TCHAR GetCharAt (int nPos, BOOL bCheckForErros = TRUE) const 
	{
		if (bCheckForErros && (nPos < 0 || nPos >= m_strBuffer.GetLength ()))
		{
			return 0;
		}
		return m_strBuffer.GetAt (nPos);
	}

	// ----------
	// Conversion
	// ----------
	void VirtToClient (CPoint& pt);
	void ClientToVirt (CPoint& pt);
	void VirtToClient (CRect& rect);
	void ClientToVirt (CRect& rect);

	// ---------
	// Undo/Redo
	// ---------
	void EnableUndoCharMode (BOOL bEnable = TRUE) {m_bUndoCharMode = bEnable;}

	void GetUndoActions(CDWordArray& dwaUAT) const;
	void GetRedoActions(CDWordArray& dwaUAT) const;

	// -----
	// Print
	// -----
	void SetPrinterFont(CFont* pFont);
	void Print ();

	//----------------
	// Tooltip Support
	//----------------
	void ShowContextTip();
	void EnableToolTips (BOOL bEnable = TRUE);
	void ToolTipPop ();
	
	void SetMaxToolTipWidth (int nMaxToolTipWidth)
	{
		m_nMaxToolTipWidth = nMaxToolTipWidth;
	}

	// -------------------------
	// Graphical symbols support
	// -------------------------
	CImageList* GetSymImgList () const {return m_pSymbolsImgList;}
	void SetSymImgList (CImageList* pImgList, BOOL bAutoDestroy = FALSE)
	{
		m_mapCharWidth.RemoveAll ();
		m_pSymbolsImgList = pImgList;
		if (m_pSymbolsImgList != NULL)
		{
			OnChangeFont (NULL);
		}
	}

	//---------------
	// Marker Support
	//---------------
	BOOL		ToggleMarker (int nRow, DWORD dwMarkerType, LPVOID lpData = NULL, 
								BOOL bRedraw = TRUE, UINT unPriority = 0);
	POSITION	SetMarker (CBCGPEditMarker* pMarker, int nRow = -1, BOOL bRedraw = TRUE);
	POSITION	GetMarker (int nRow, CBCGPEditMarker** ppMarker, DWORD dwMarkerType);

	BOOL		SetMarkerData (int nRow, LPVOID lpData, DWORD dwMarkerType, BOOL bRedraw = TRUE);
	void		SetMarkerData (POSITION pos, DWORD_PTR dwData);
	POSITION	InsertMarker (CBCGPEditMarker* pMarker, BOOL bRedraw = TRUE);

	void		DeleteMarker (POSITION posMarker);
	BOOL		DeleteMarker (int nRow, DWORD dwMarkerType, BOOL bRedraw = TRUE);
	void		DeleteAllMarkers (DWORD dwMarkerType = (DWORD) -1, BOOL bRedraw = TRUE);
	int			DeleteMarkersInRangeByOffset (int nStart, int nEnd, DWORD dwMarkerType = (DWORD) -1, BOOL bRedraw = TRUE);
	BOOL		GoToNextMarker (DWORD dwMarkerType, BOOL bDirection = TRUE);

	void		GetMarkerListByType (CObList& lstMarkers, DWORD dwMarkerType) const; 
	void		GetMarkersInRange (int nRowStart, CObList& lstMarkers, 
									DWORD dwMarkerType = (DWORD) -1, int nRowEnd = -1) const;
	void		GetMarkersInRangeByOffset (int nStart, int nEnd, CObList& lstMarkers, 
									DWORD dwMarkerType = (DWORD) -1) const;

	CBCGPEditMarker* GetMarker (POSITION pos) 
	{
		ASSERT_VALID (this);
		return DYNAMIC_DOWNCAST (CBCGPEditMarker, m_lstMarkers.GetAt (pos));
	}

	const CObList& GetMarkerList () const 
	{
		return m_lstMarkers;
	} 

	void CleanUpMarkers ();
	void RedrawMarkerArea (int nRow = -1, BOOL bFullLine = FALSE);

	POSITION SetLineColorMarker (int nRow = -1, COLORREF clrFore = (DWORD)-1, 
								 COLORREF clrBk = (DWORD) -1, BOOL bFullLine = TRUE, 
								 int nLineCount = 0, DWORD dwMarkerType = g_dwBCGPEdit_LineColorMarker,
								 UINT unPriority = 0);
	POSITION SetOffsetMarker (int nStart, int nEnd, 
							  DWORD dwMarkerType = g_dwBCGPEdit_LineColorMarker, 
							  UINT unPriority = 0) { return NULL; }
	POSITION SetHiliteMarker (int nStart, int nEnd, 
							  COLORREF clrFore = (DWORD)-1, COLORREF clrBk = (DWORD) -1,
							  DWORD dwMarkerType = g_dwBCGPEdit_HiliteMarker, 
							  UINT unPriority = 2);

	void EnableGradientMarkers (BOOL bEnable = TRUE)
	{
		m_bGradientMarkers = bEnable;
	}

	BOOL IsGradientMarkersEnabled () const
	{
		return m_bGradientMarkers;
	}
		
	// ------------------
	// Outlining Support:
	// ------------------
	void EnableOutlining (BOOL bEnable = TRUE);

	BOOL IsOutliningEnabled () const
	{
		return m_bEnableOutlining;
	}

	void EnableAutoOutlining (BOOL bEnable = TRUE)
	{
		m_bAutoOutlining = bEnable;
	}

	BOOL IsAutoOutliningEnabled () const
	{
		return m_bAutoOutlining;
	}
	
	void EnableOutlineParser (BOOL bEnable = TRUE)
	{
		m_bOutlineParserEnabled = bEnable;
	}
	
	void SetOutlineMargin (BOOL bShow = TRUE, int nMarginWidth = 20)
	{
		ASSERT (nMarginWidth >= 0);
		m_bEnableOutlineMargin = bShow;
		m_nOutlineMarginWidth = nMarginWidth;
		RecalcLayout ();
		SetCaret (m_nCurrOffset, TRUE, FALSE);
		RedrawWindow ();
	}
		
	BOOL IsOutlineMarginVisible () const
	{
		return m_bEnableOutlineMargin;
	}

	int GetOutlineMarginWidth () const
	{
		return m_nOutlineMarginWidth;
	}

	void SetLineNumbersMargin (BOOL bShow = TRUE, int nMarginWidth = 45);
		
	BOOL IsLineNumbersMarginVisible () const
	{
		return m_bEnableLineNumbersMargin;
	}

	int GetLineNumbersMarginWidth () const
	{
		return m_nLineNumbersMarginWidth;
	}

	void UpdateAutoOutlining (int nStartOffset = -1, int nEndOffset = -1);

	void HideSelection ();
	void ToggleOutlining ();
	void ToggleAllOutlining ();
	void StopHidingCurrent ();
	void CollapseToDefinitions ();
	void StopOutlining ();

	void ExpandLine (int nLineNum);
		
	BOOL GetHiddenTextFromPoint (CPoint pt, CString& strWord, BOOL bTrimEmptyLines = TRUE);
	BOOL GetHiddenTextFromOffset (int nOffset, CString& strWord);
	CString GetVisibleText (int nStartOffset, int nEndOffset) const;

	CBCGPOutlineBaseNode* AddOutlining (int nStartOffset, int nEndOffset, int nNameOffset = 0, 
										BOOL bCollapsed = TRUE, LPCTSTR lpszReplace = NULL);
	void RedrawOutlineArea (int nRow = -1);

	COutlineCasheManager& GetOutlineManager ()
	{
		return m_OutlineManager;
	}

	//------------------
	// Hyperlink Support
	//------------------
	void EnableHyperlinkSupport (BOOL bEnabled = TRUE) {m_bEnableHyperlinkSupport = bEnabled;}
	BOOL IsHyperlinksSupportEnabled () const {return m_bEnableHyperlinkSupport;}
	void SetHyperlinkColor (BOOL bUseColor = TRUE, COLORREF clr = -1);
	
	void HyperlinkClick ();
	BOOL GetHyperlinkString (CString& strURL);
	BOOL GetHyperlinkToolTip (CString& strToolTip);

// Overrides
public:
	//---------------------
	// IntelliSense Support
	//---------------------
	virtual BOOL FillIntelliSenseList (CObList& lstIntelliSenseData, LPCTSTR lpszIntelliSense) const  { return FALSE; }

	virtual BOOL FindText(LPCTSTR lpszFind, BOOL bNext = TRUE, BOOL bCase = TRUE, BOOL bWholeWord = FALSE);
	virtual BOOL ReplaceText(LPCTSTR lpszFind, LPCTSTR lpszReplace, BOOL bNext = TRUE, 
							 BOOL bCase = TRUE, BOOL bWholeWord = FALSE);
	virtual int ReplaceAll(LPCTSTR lpszFind, LPCTSTR lpszReplace, BOOL bNext = TRUE, 
						   BOOL bCase = TRUE, BOOL bWholeWord = FALSE);
	virtual CSize GetCaretSize () const;
	virtual BOOL OnSetOvrMode ();

	//------------
	// Navigation:
	//------------
	virtual BOOL Left (BOOL bRedrawOnScroll = TRUE);
	virtual BOOL Right (BOOL bRedrawOnScroll = TRUE);
	virtual BOOL Up (BOOL bSetCaretToMaxColumn = TRUE, BOOL bRedrawOnScroll = TRUE);
	virtual BOOL Down (BOOL bSetCaretToMaxColumn = TRUE, BOOL bRedrawOnScroll = TRUE);
	virtual BOOL Home (BOOL bRedrawOnScroll = TRUE);
	virtual BOOL End (BOOL bRedrawOnScroll = TRUE);
	virtual BOOL StartOfText (BOOL bRedraw = TRUE);
	virtual BOOL EndOfText (BOOL bRedrawOnScroll = TRUE);
	virtual BOOL PageUp (BOOL bRedrawOnScroll = TRUE);
	virtual BOOL PageDown (BOOL bRedrawOnScroll = TRUE);

	virtual BOOL GoToLine (int nLineNum, BOOL bExpandBlock = TRUE, BOOL bRedrawOnScroll = TRUE);

	virtual BOOL NextWord (BOOL bRedrawOnScroll = TRUE);
	virtual BOOL PrevWord (BOOL bRedrawOnScroll = TRUE);

	virtual BOOL PrevIndent ();

	virtual void OnSetCaret () {}

	virtual BOOL OnScroll (int fnBar, UINT nSBCode, int nPos =-1);
	virtual void UpdateScrollBars ();
	virtual void AdjustScrollBars (CRect* pRectView = NULL);

	virtual BOOL SetCaretToLastMaxColumn (int nOffset, BOOL bRedrawOnScroll = TRUE);
	virtual BOOL OnUpdateLineNumbersMarginWidth (CDC* pDC = NULL);

	//---------------
	// Insert/delete:
	//---------------
	virtual BOOL InsertChar (TCHAR nChar, BOOL bRedraw = FALSE);
	virtual BOOL InsertText (LPCTSTR lpszText, int nInsertFrom = -1, BOOL bRedraw = FALSE, 
							 BOOL bSuppressUndo = FALSE, BOOL bUpdateLineData = TRUE,
							 BOOL bForceNextUndo = FALSE, BOOL bAlwaysAtEnd = TRUE);
	virtual BOOL InsertTextAsBlock (LPCTSTR lpszText, int nInsertFrom = -1, BOOL bRedraw = FALSE, 
									BOOL bSuppressUndo = FALSE, BOOL bUpdateLineData = TRUE,
									BOOL bForceNextUndo = FALSE);
	virtual BOOL InsertNewLine (BOOL bRedraw = FALSE, BOOL bForceNextUndo = FALSE, 
								int nOffset = -1, BOOL bSetCaret = TRUE);
	virtual BOOL InsertTab (BOOL bRedraw = FALSE);
	virtual BOOL OnDelete (BOOL bRedraw = FALSE, BOOL bForceNextUndo = FALSE);
	virtual BOOL ProcessTextBeforeInsert (CString& strText);

	virtual BOOL OnInsertNewLine (BOOL bForceNextUndo = FALSE, int nOffset = -1, 
									 BOOL bSetCaret = FALSE);
	virtual CString GetInsertNewLineString (int nOffset);

	virtual void OnDeleteTextFromBuffer (int nStartOffset, int nEndOffset, 
										  LPCTSTR lpcszDeletedText);
	virtual void OnInsertTextToBuffer (int nStartOffset, const CString& strInsertedText, 
										BOOL bUpdateLineData = TRUE);

	virtual void OnPrepareHTMLString (CString& str) const;
	virtual void OnPrepareRTFString (CString& str) const;


	//---------------
	// Text changing notifications:
	//---------------
	virtual BOOL OnBeforeTextInserted (CString& strInsertedText, int nOffset) {return TRUE;}
	virtual BOOL OnBeforeTextDeleted (int nOffset, const CString& strDeletedText) {return TRUE;}
	virtual void OnAfterTextChanged (int nOffsetFrom, const CString& strText, BOOL bInserted);
	
	virtual void UpdateLineRelatedData (int nStartLine, int nNumLines);
	virtual void RemoveLineDataInRange (int nStartRow, int nEndRow);

	//----------
	// Undo/Redo
	//----------
	virtual void SetOverrideMode (BOOL bSet = FALSE) {m_bOvrMode = bSet;}
	virtual BOOL GetOverrideMode () const {return m_bOvrMode;}
	//----------
	// Undo/Redo
	//----------
	virtual BOOL CanUndo () const;
	virtual BOOL CanRedo () const;

	virtual BOOL OnUndo (BOOL bSetCaret = TRUE, BOOL bRedraw = TRUE);
	virtual BOOL OnRedo (BOOL bSetCaret = TRUE, BOOL bRedraw = TRUE);

	virtual void AddUndoAction (CString& strText, DWORD dwAction, int nCurrOffset, BOOL bForceInsertMode, 
								BOOL bForceNextUndo = FALSE);
	virtual void AddUndoAction (TCHAR ch, DWORD dwAction, int nCurrOffset, BOOL bForceInsertMode);
	virtual void AddUndoAction (BCGP_BASE_UNDO_DATA* lpActionData, DWORD dwAction, BOOL bDestroyData = TRUE);
	virtual void AddUndoAction (BCGP_EDIT_UNDO_ACTION* pUndoAction);

	virtual BOOL OnBeforeUndoRedo (BCGP_EDIT_UNDO_ACTION* /*pUndoAction*/) {return TRUE;}	
	virtual void OnAfterUndoRedo (BCGP_EDIT_UNDO_ACTION* /*pUndoAction*/) {}	
	
	virtual BOOL OnBeforeAddUndoAction (BCGP_EDIT_UNDO_ACTION* pUndoAction) {return TRUE;}
	virtual void OnRemoveUndoAction (BCGP_EDIT_UNDO_ACTION* pUndoAction) {};

	virtual BOOL ProcessUndoRedoAction (BCGP_EDIT_UNDO_ACTION* pUndoAction, BOOL bUndo, BOOL bSetCaret);

	virtual void EmptyUndoList ();

	virtual void SetLastUndoReason (DWORD dwUndoReason) {m_dwLastUndoReason = dwUndoReason;}
	virtual DWORD GetLastUndoReason () const {return m_dwLastUndoReason;}

	//----------
	// Selection
	//----------
	enum BCGP_EDIT_SEL_TYPE
	{
		ST_PREV_WORD,
		ST_NEXT_WORD,
		ST_PREV_SYMBOL,
		ST_NEXT_SYMBOL,
		ST_PREV_LINE,
		ST_NEXT_LINE,
		ST_PREV_PAGE,
		ST_NEXT_PAGE,
		ST_HOME,
		ST_END,
		ST_START_OF_TEXT,
		ST_END_OF_TEXT,
		ST_ALL_TEXT
	};

	virtual BOOL MakeSelection (BCGP_EDIT_SEL_TYPE selType);
	virtual BOOL RemoveSelection (BOOL bSetCaretToSelStart = TRUE, 
									BOOL bKeepCurrPos = FALSE, BOOL bRedraw = TRUE);
	virtual BOOL IndentSelection (BOOL bForward);
	virtual void SetSel (int nStartSel, int nEndSel, BOOL bNoScroll = FALSE)
	{
		SetSel2 (nStartSel, nEndSel, bNoScroll);
	}
	virtual void SetSel2 (int nStartSel, int nEndSel, BOOL bNoScroll = FALSE, BOOL bRedraw = TRUE);

	//----------------
	// Copy/Cut/Paste:
	//----------------
	virtual BOOL Copy ();
	virtual BOOL Cut ();
	virtual BOOL Paste (int iPos = -1);
	virtual BOOL Clear ();

	virtual BOOL IsPasteEnabled () const;

	virtual BOOL IsCopyEnabled () const
	{
		return m_bEnableWholeTextCopy || m_bEnableCurrentLineCopy || 
			(m_iStartSel >= 0 && m_iEndSel >= 0 && (m_iStartSel != m_iEndSel));
	}

	virtual BOOL IsCutEnabled () const
	{
		return m_iStartSel >= 0 && m_iEndSel >= 0 && (m_iStartSel != m_iEndSel) && !m_bReadOnly;
	}

	virtual BOOL IsIndentEnabled (BOOL bForward);

	//--------
	// Drawing
	//--------
	virtual void OnDraw(CDC* pDC, LPCRECT lpRectClip);
	virtual void RedrawText (int nStartRow, int nEndRow = 0);
	virtual void RedrawTextOffsets (int nStartOffset, int nEndOffset);

	virtual void PrepareDrawText(CDC* pDC);

	virtual void OnFillBackground (CDC* pDC);
	virtual void OnDrawText (CDC* pDC);
	virtual void OnDrawSideBar (CDC* pDC, CRect rect);
	virtual void OnDrawLineNumbersBar (CDC* pDC, CRect rect);
	virtual void OnFillOutlineArea (CDC* pDC, CRect rectLeft);
	virtual void OnDrawOutlineBar (CDC* pDC, CRect rectLeft);

	virtual void OnDrawMarker (CDC* pDC, CRect rectMarker, const CBCGPEditMarker* pMarker);
	virtual void OnDrawLineNumber (CDC* pDC, CRect rect, const int nLineNumber);
	virtual void OnDrawOutlineButton (CDC* pDC, CRect rectButton, CBCGPOutlineBaseNode* pRowOutlineBlock,
									  BOOL bInsideOpenBlockAtStart, BOOL bInsideOpenBlockAtEnd, BOOL bEndOfBlock);
	virtual void OnDrawBorder (CDC* pDC, CRect rect);

	// -----
	// Print
	// -----
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrintPageHeader(CDC* pDC, CPrintInfo* pInfo) {}
	virtual void OnPrintPageFooter(CDC* pDC, CPrintInfo* pInfo) {}

	//--------------------
	// Drag & Drop support
	//--------------------
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

	virtual void OnDragLeave();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

	//----------------
	// Syntax coloring
	//----------------
	virtual BOOL OnGetWordColor (const CString& strWord, COLORREF& clrText, COLORREF& clrBk, int nPos);
	virtual void OnGetCharColor (TCHAR ch, int nOffset, COLORREF& clrText, COLORREF& clrBk) {}

	virtual BOOL OnGetTextColor (int nStart, int nEnd, 
		COLORREF& clrText, COLORREF& clrBk, BOOL bColorFound) const 
	{
		return FALSE;
	}
	
	//-------------
	// XML settings
	//-------------
	virtual BOOL LoadXMLSettings (const CString& strFileName);
	virtual BOOL LoadXMLSettingsFromBuffer (const CString& strBuffer);
	virtual void RemoveXMLSettings ();

	//---------------------
	// IntelliSense Support
	//---------------------
	virtual BOOL OnFillIntelliSenseList (int& nCurrOffset, CObList& lstIntelliSenseData) const;
	virtual BOOL OnBeforeInvokeIntelliSense (const CString& strBuffer, int& nCurrOffset, CString& strIntelliSense) const {return TRUE;}
	virtual BOOL OnIntelliSenseComplete (int nIdx, CBCGPIntelliSenseData* pData, CString& strText)	{ return TRUE; }
	virtual BOOL InvokeIntelliSense (CObList& lstIntelliSenseData, CPoint ptTopLeft);
	virtual BOOL IntelliSenseCharUpdate(const CString& strBuffer, int nCurrOffset, TCHAR nChar, CString& strIntelliSense) { return FALSE; }
	virtual BOOL InvokeIntelliSense ();

	//---------------------
	// Graphical symbols support
	//---------------------
	virtual BOOL OnDrawSymbol (CDC* pDC, CRect rectRow, BCGP_EDIT_SYM_DEF& symDef)
	{
		return FALSE;
	}
	virtual BOOL LookUpSymbol (LPCTSTR lpcszBuffer, int nOffset, int nCount, TCHAR chSymbol, 
								BCGP_EDIT_SYM_DEF& symDef, int& nSymExtraLen, BOOL bForward = TRUE);
	virtual BOOL OnFillSymListData (int& nCurrOffset, CObList& lstIntelliSenseData) const;
	virtual BOOL InvokeSymList (CObList& lstIntelliSenseData, CPoint ptTopLeft);
	virtual BOOL InvokeSymList ();
	virtual int  GetSymbolExtraLen (CString& strBuffer, int nOffset, BOOL bForward = TRUE);

	//---------------
	// Marker Support
	//---------------
	virtual CBCGPEditMarker* FindMarkerByData (DWORD dwData) const;
	virtual void GetMarkerListByData (CObList& lstMarkers, DWORD dwData) const;
	
	virtual BOOL GoToMarker (const CBCGPEditMarker* pMarker);
	
	virtual BOOL HasMarkers (DWORD dwMarkerType = (DWORD) -1);

	virtual BOOL CanUpdateMarker (CBCGPEditMarker* pMarker) const  {return TRUE;}
	virtual BOOL CanRemoveMarker (CBCGPEditMarker* pMarker) const  {return TRUE;}
	
	virtual BOOL OnUpdateMarkerLine (int nNewLine) {return TRUE;}
	virtual BOOL OnRemoveMarker (CBCGPEditMarker* pMarker, POSITION posMarker) {return TRUE;}

	//------------------
	// Outlining Support
	//------------------
	virtual void OnCalcOutlineSymbol (CDC* pDC, CPoint ptCharOffset, CBCGPOutlineBaseNode* pHiddenText);
	virtual void OnDrawOutlineSymbol (CDC* pDC, CBCGPOutlineBaseNode* pHiddenText,
									  COLORREF clrFore, COLORREF clrBack);

	virtual BOOL OnOutlineButtonClick (int nOffset);
	virtual BOOL OnOutlineButtonClick (CBCGPOutlineNode *pOutlineNode);

	virtual CBCGPOutlineParser* CreateOutlineParser () const
	{
		return new CBCGPOutlineParser;
	}

	virtual BOOL LoadOutlineParserXMLSettings (CString& strInBuffer);
	
	virtual void OnOutlineChanges (BCGP_EDIT_OUTLINE_CHANGES& changes, BOOL bRedraw = TRUE);
	virtual void OnInsertOutlineBlock (CBCGPOutlineBaseNode* pBlock) {}
	virtual void OnRemoveOutlineBlock (CBCGPOutlineBaseNode* pBlock) {}

	virtual void OnUpdateAutoOutlining (int nOffsetFrom, int nCharsCount, BOOL bRedraw = TRUE);
	
	//------------------
	// Hyperlink Support
	//------------------
	virtual BOOL FindHyperlinkString (int nStartOffset, int nEndOffset, CString& strURL, int &nOffset);

	virtual void OnHyperlinkClick (int nOffset, LPCTSTR lpszURL);
	virtual BOOL OnGetHyperlinkString (int nStartOffset, CString& strURL) {return TRUE;}
	virtual BOOL OnGetHyperlinkToolTip (int nStartOffset, CString& strToolTip) {return TRUE;}

	virtual BOOL LoadHyperlinkXMLSettings (CString& strInBuffer);
	
	//----------------
	// Tooltip Support
	//----------------
	virtual BOOL OnGetTipText (CString& strTipString) {return TRUE;}

	//--------
	// Notifications
	//--------
	virtual void OnFailedOperation (DWORD dwOpType) {}
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPEditCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void Serialize(CArchive& ar);
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPEditCtrl();

protected:
	virtual void Initialize ();
	virtual void OnChangeFont (CDC* pDC = NULL);
	virtual void InitColors ();
	virtual void UpdateFonts();
	virtual void CleanUpFonts ();
	COLORREF GetDefaultTextColor () const;
	COLORREF GetDefaultBackColor () const;

	int GetCurrRowStart (BOOL bSkipHidden = FALSE) const;
	int GetCurrRowEnd (BOOL bSkipHidden = FALSE) const;

	int GetCurrRowTextStart (BOOL bSkipHidden = FALSE) const;
	int GetRowTextStart (int nRowOffset, BOOL bSkipHidden = FALSE) const;
	
	int GetRowStart (int nRow, BOOL bSkipHidden = FALSE) const;
	int RowFromOffset (int nOffset, BOOL bCalcAll = FALSE, BOOL bSkipHidden = FALSE) const;
	int GetNumOfColumnsInRowByOffset (int nOffset, int& nRowEndOffset, BOOL bSkipHidden = FALSE) const;

	int GetOffsetOfColumnInRow (int nColumn, int nRowEndOffset, BOOL bEndOffsetSpecified = TRUE, BOOL bSkipHidden = FALSE);
	int GetColumnFromOffset (int nOffset, BOOL bSkipHidden = FALSE) const;

	int GetRowStartByOffset (int nOffset, BOOL bSkipHidden = FALSE) const;
	int GetRowEndByOffset (int nOffset, BOOL bSkipHidden = FALSE) const;

	int GetHorzRowExtent (CDC* pDC, int nOffset);

	int GetVirtualRow (int nRow, BOOL bCalcAll = FALSE) const;
	int GetRowFromVirtual (int nRowVirt, BOOL bCalcAll = FALSE) const;


	CFont* SelectFont (CDC* pDC, BOOL bUnderline = FALSE);

	void RedrawRestOfLine (int nOffset);
	void RedrawRestOfText (int nOffset);
	void DrawColorLine (CDC* pDC, int nRow, CRect rectRow);

	BOOL CopyTextToClipboard (LPCTSTR lpszText, int nLen = -1);
	HGLOBAL CopyTextToClipboardInternal (LPCTSTR lpszText, CLIPFORMAT cFormat, int nLen, BOOL bForceAnsi = FALSE);
	int GetLineInternal (int nLine, LPTSTR lpszBuffer, int nMaxLength) const;

	BOOL GetSelectionRect (CRect& rect1, CRect& rect2, CRect& rect3);

	void BuildColorAreas (CList <BCGP_EDIT_COLOR_AREA, BCGP_EDIT_COLOR_AREA&>& colorAreas,
						  int nStartOffset, int nEndOffset, 
						  BCGP_EDIT_COLOR_BLOCK* pColorBlockToExclude, 
						  BOOL bRedraw = TRUE);
	BOOL IsOffsetAtColorBlock (int nOffset);

	inline int ReverseFindOneOf (const CString& strBuffer, int nPos, const CString& strChars) const;
	void ReverseFindOneOf(const CString& strCharSet, const int ciStartPos, int& nCurrOffset) const;
	int FindOneOf (const CString& strBuffer, int nPos, const CString& strChars) const;
	int WhileOneOf (const CString& strBuffer, int nOffset, bool bForward, 
		 			const CString& strChars);
	virtual BOOL DoFindText(int& nPos, int& nFindLength, 
					LPCTSTR lpszFind, BOOL bNext = TRUE, BOOL bCase = TRUE, 
					BOOL bWholeWord = FALSE);

	BOOL IsIqual (const CString& strBuffer, int nOffset, BOOL bDirForward, const CString& str) const;
	BOOL DoCompare (const CString& strBuffer, const int nLeft, const int nCount, const CString& strWith) const;

	virtual int DrawString (CDC* pDC, LPCTSTR str, CRect rect, int nOrigin, 
							COLORREF clrBack);
	virtual CSize GetStringExtent (CDC* pDC, LPCTSTR lpszString, int nCount);
	CSize GetOutputTabbedTextExtent (CDC* pDC, LPCTSTR lpszString, int nCount);

	BOOL IsInsideSelection (int nOffset, CPoint& ptCharOffset, CRect rectSel, int nCharWidth);
	virtual BOOL IsColoredLine (int nRow, COLORREF& clrFore, COLORREF& clrBack);
	virtual BOOL IsHilitedText (int nOffset, COLORREF& clrFore, COLORREF& clrBack);
	
	CBCGPOutlineNode* FindCollapsedBlock (int nOffset) const;
	CBCGPOutlineNode* FindCollapsedBlockInRange (int nStart, int nEnd, BOOL bForward = TRUE) const;
	CBCGPOutlineNode* FindCollapsedBlockByPoint(CPoint point, int nOffset) const;
	void NextRow (int& nRow, int& nRowVirtual, int& nRowStartOffset) const;
	

	void PrepareBlock (CString& strOut);
	void RetrieveSelectedBlock (CStringArray& arStrings);

	void BuildTabString (CString& str, int nOffset = 0);
	void TrimEmptyLinesFromLeft (CString& str) const;

	virtual void UpdateOffsetRelatedData (int nStartOffset, int nEndOffset);
	virtual void RemoveOffsetDataInRange (int nStartOffset, int nEndOffset);

	BOOL FindOpenBlock (int nStartOffset, BCGP_EDIT_COLOR_BLOCK* pFoundBlock);
	int FindCloseBlock (int nStartOffset, BCGP_EDIT_COLOR_BLOCK* pFoundBlock);

	BOOL IsCharBlockSymbol (TCHAR ch);
	BOOL IsStringHasBlockSymbols (const CString& str);

	BOOL PasteFromDataObject (COleDataObject* pDataObject, 
							  int nInsertFrom = -1, BOOL bRedraw = FALSE, 
							  BOOL bSuppressUndo = FALSE, 
							  BOOL bUpdateLineData = TRUE,
							  BOOL bForceNextUndo = FALSE);

	void RecalcLayout ();
	void UpdateHiddenLines (BCGP_EDIT_OUTLINE_CHANGES& changes);

	// Hyperlink Support:
	void AddHyperlink (CString& str, int nOffset);
	void ClearHyperlinks ();
	int LookupHyperlinkByOffset(int m_nCurrOffset);

	void ExportToRTF (CString& strRTF, CArray <COLORREF, COLORREF&> & arrClrTbl);
	int AddColor (COLORREF clr, CArray <COLORREF, COLORREF&> & arrClrTbl) const;

	static COLORREF GetColorFromBuffer (const CString strBuffer);

	// Generated message map functions
protected:
	BOOL m_bIntelliSenseUpdate;
	//{{AFX_MSG(CBCGPEditCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSysColorChange();
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg BOOL OnTTNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/);
	afx_msg LRESULT OnSetText (WPARAM, LPARAM);
	afx_msg LRESULT OnGetText (WPARAM, LPARAM);
	afx_msg LRESULT OnStyleChanging (WPARAM, LPARAM);
	afx_msg LRESULT OnGetTextLength (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGUpdateToolTips(WPARAM, LPARAM);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////
// CBCGPEditCtrl notification messages:

extern BCGCBPRODLLEXPORT UINT BCGM_ON_EDITCHANGE;

#pragma warning (default : 4100) 
#pragma warning (default : 4311)
#pragma warning (default : 4312)

#endif // BCGP_EXCLUDE_EDIT_CTRL

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPEDITCTRL_H__078F8590_E8CE_4F07_A08A_C41CEBE48D77__INCLUDED_)
