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
// BCGPOutlineParser.h: interface for the CBCGPOutlineParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPOUTLINEPARSER_H__042FF594_A696_4902_A475_119896175319__INCLUDED_)
#define AFX_BCGPOUTLINEPARSER_H__042FF594_A696_4902_A475_119896175319__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_EDIT_CTRL

enum BCGCBPRODLLEXPORT LexemType {
	LT_CompleteBlock = 1,
	LT_BlockStart = 2, 
	LT_BlockEnd = 3,
	LT_CustomBlock = 4,
	LT_EndOfText = -1,
	LT_Eps = 0
};

struct BCGCBPRODLLEXPORT Lexeme
{
	Lexeme () : m_nBlockType (0), m_nType (LT_Eps), m_nStart (0), m_nEnd (0), m_dwData (0)
	{
	}
	Lexeme (int nBlockType, LexemType nLexemType, int nStart, int nEnd, DWORD dwData = 0) : 
		m_nBlockType (nBlockType), m_nType (nLexemType), m_nStart (nStart), m_nEnd (nEnd), m_dwData (dwData)
	{
	}

	LexemType	m_nType;
	int			m_nBlockType;
	int			m_nStart;
	int			m_nEnd;
	DWORD		m_dwData;
};

struct BCGCBPRODLLEXPORT BlockType
{
	BlockType () : m_bAllowNestedBlocks (TRUE), m_bIgnore (FALSE), m_dwData (0)
	{
	}
	BlockType ( LPCTSTR lpszOpen, LPCTSTR lpszClose, LPCTSTR lpszReplace, 
				BOOL bNested, BOOL bIgnore = FALSE, CStringList* pKeywords = NULL,
				DWORD dwData = 0) :
		m_strOpen (lpszOpen), m_strClose (lpszClose), m_strReplace (lpszReplace),
		m_bAllowNestedBlocks (bNested), m_bIgnore (bIgnore), m_dwData (dwData)
	{
		if (pKeywords != NULL)
		{
			m_lstKeywords.AddTail (pKeywords);
		}
	}

	CString		m_strOpen;
	CString		m_strClose;
	CString		m_strReplace;
	BOOL		m_bAllowNestedBlocks;
	BOOL		m_bIgnore;
	CStringList m_lstKeywords;
	DWORD		m_dwData;
};

/////////////////////////////////////////
// Default parser for outlining support:
//

class CBCGPOutlineBaseNode;
class CBCGPOutlineNode;
struct BCGP_EDIT_OUTLINE_CHANGES;

class BCGCBPRODLLEXPORT CBCGPOutlineParser : public CObject 
{
public:
	CBCGPOutlineParser();
	virtual ~CBCGPOutlineParser();

	virtual void UpdateOutlining (CString& strBuffer, int nOffsetFrom, int nCharsCount,
								  CBCGPOutlineNode* pOutlineNode, BCGP_EDIT_OUTLINE_CHANGES& changes);
	virtual void Init ();
	virtual void AddBlockType (
					LPCTSTR lpszOpen, LPCTSTR lpszClose, LPCTSTR lpszReplace = NULL, 
					BOOL bNested = TRUE, BOOL bIgnore = FALSE, 
					CStringList* pKeywordsList = NULL);
	virtual void AddEscapeSequence (LPCTSTR lpszStr);
	virtual void RemoveAllBlockTypes ();
	const BlockType* GetBlockType (int nIndex) const;

	BOOL IsValuedChar (TCHAR ch) const;

protected:
	// Text processing:
	virtual Lexeme GetNext (const CString& strIn, int& nOffset, const int nSearchTo);
	virtual void PushResult (Lexeme lexem, CObList& lstResults);
	virtual void DoParse (const CString& strBuffer, const int nStartOffset, const int nEndOffset, CObList& lstResults);

	BOOL IsEscapeSequence (const CString& strBuffer, int& nBufferOffset) const;
	virtual BOOL Compare (const CString& strBuffer, const int nBufferOffset,
						  const CString& strCompareWith, int& nEndOffset) const;

	virtual void DoUpdateOffsets (const CString& strBuffer, const int nStartOffset, const int nEndOffset, CObList& lstBlocks);
	virtual int GetNameOffset (const CString& strIn, int nStartFrom, int nSearchTo,
							   const BlockType* pBlockType, CObList& lstIgnore, CString& strName);
	virtual int GetStartOffset (const CString& strIn, int nStartFrom, int nSearchTo, CObList& lstIgnore);
	virtual int GetEndOffset (const CString& strIn, int nStartFrom, int nSearchTo);
	virtual int GetPrevWord (const CString& strIn, int nStartFrom, 
							int nSearchTo, LPCTSTR lpszStopDelimiters, CString& strWord);
	virtual BOOL IsBlockName (const CString& strIn, CString& strWord, const int nOffset, const int nSearchTo);

	// Outline tree updating methods:
	virtual BOOL AddMarker (CBCGPOutlineNode* pMarkerBlock, CBCGPOutlineNode* pParentNode,
							BCGP_EDIT_OUTLINE_CHANGES& changes) const;
	virtual CBCGPOutlineNode* FindFittingBlock (CBCGPOutlineNode* pBlock, CBCGPOutlineNode* pStartFrom) const;

	virtual CBCGPOutlineNode* GetRangeToReparse (CBCGPOutlineNode* pOutlineNode,
												int& nStartOffset, int& nEndOffset) const;
	

	inline CBCGPOutlineNode* AddNode (CBCGPOutlineNode* pNewNode, CBCGPOutlineNode* pParentNode, BCGP_EDIT_OUTLINE_CHANGES& changes) const;
	inline CBCGPOutlineNode* RemoveNode (CBCGPOutlineNode* pNode, CBCGPOutlineNode* pParentNode, BCGP_EDIT_OUTLINE_CHANGES& changes,
												BOOL bRemoveSubNodes = FALSE) const;

public:
	// data:
	CArray <BlockType*, BlockType*>	m_arrBlockTypes;
	CStringList						m_lstEscapeSequences;
	CString							m_strDelimiters;
	BOOL							m_bCaseSensitive;
	BOOL							m_bWholeWords;
	CString							m_strOut;	// for test purpose only
};

#endif	// BCGP_EXCLUDE_EDIT_CTRL

#endif // !defined(AFX_BCGPOUTLINEPARSER_H__042FF594_A696_4902_A475_119896175319__INCLUDED_)
