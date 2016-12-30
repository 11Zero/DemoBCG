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
//
// BCGPOutlineParser.cpp: implementation of the CBCGPOutlineParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"

#ifndef BCGP_EXCLUDE_EDIT_CTRL

#include "BCGPOutlineParser.h"
#include "BCGPEditCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPOutlineParser::CBCGPOutlineParser()
{
	m_strDelimiters = _T(" \t\n,./?<>;:\"'{[}]~`%^&*()-+=!");
	m_bCaseSensitive = TRUE;
	m_bWholeWords = FALSE;
}
//************************************************************************************
CBCGPOutlineParser::~CBCGPOutlineParser()
{
	RemoveAllBlockTypes ();
}
//************************************************************************************
void CBCGPOutlineParser::RemoveAllBlockTypes ()
{
	for (int i = 0; i <= m_arrBlockTypes.GetUpperBound (); i++)
	{
		delete m_arrBlockTypes.GetAt (i);
	}
	m_arrBlockTypes.RemoveAll ();
}
//************************************************************************************
void CBCGPOutlineParser::Init ()
{
	RemoveAllBlockTypes ();

	// Default init for C++ outline blocks:
	AddEscapeSequence (_T("\\\""));
	AddEscapeSequence (_T("\\\\"));
	AddBlockType (_T("\""),	_T("\""),	_T("\"\""),	FALSE,	TRUE);
	AddBlockType (_T("//"),	_T("\n"),	_T("/**/"),	FALSE,	TRUE);
	AddBlockType (_T("/*"),	_T("*/"),	_T("/**/"),	FALSE,	FALSE);

	CStringList lstKeywords; 
	lstKeywords.AddTail (_T("if"));
	lstKeywords.AddTail (_T("else"));
	lstKeywords.AddTail (_T("while"));
	lstKeywords.AddTail (_T("for"));
	lstKeywords.AddTail (_T("do"));
	lstKeywords.AddTail (_T("switch"));
	lstKeywords.AddTail (_T("class"));
	lstKeywords.AddTail (_T("union"));
	lstKeywords.AddTail (_T("struct"));
	lstKeywords.AddTail (_T("namespace"));
	lstKeywords.AddTail (_T("catch"));
	lstKeywords.AddTail (_T("try"));
	lstKeywords.AddTail (_T("operator"));
	AddBlockType (_T("{"),	_T("}"),	_T("..."),	TRUE,	FALSE, &lstKeywords);
}
//************************************************************************************
void CBCGPOutlineParser::AddBlockType (LPCTSTR lpszOpen, LPCTSTR lpszClose, LPCTSTR lpszReplace, BOOL bNested, BOOL bIgnore, CStringList* pKeywordsList)
{
	ASSERT (lpszOpen != NULL);
	ASSERT (lpszClose != NULL);

	CString strClose = lpszClose;
	if (strClose.IsEmpty ())
	{
		AddEscapeSequence (lpszOpen);
		return;
	}

	BlockType* pNewBlockType = new BlockType (
		lpszOpen, lpszClose, (lpszReplace != NULL) ? lpszReplace : _T("..."), bNested, bIgnore, pKeywordsList);

	m_arrBlockTypes.Add (pNewBlockType);
}
//************************************************************************************
void CBCGPOutlineParser::AddEscapeSequence (LPCTSTR lpszStr)
{
	ASSERT (lpszStr != NULL);
	m_lstEscapeSequences.AddTail (lpszStr);
}
//************************************************************************************
const BlockType* CBCGPOutlineParser::GetBlockType (int nIndex) const
{
	if (nIndex >= 0 && nIndex <= m_arrBlockTypes.GetUpperBound ())
	{
		return m_arrBlockTypes [nIndex];
	}
	else
	{
		ASSERT (FALSE);
		return NULL;
	}
}
//************************************************************************************
BOOL CBCGPOutlineParser::Compare (const CString& strBuffer, const int nBufferOffset,
								  const CString& strCompareWith, int& nEndOffset) const
{
	// Returns equal chars num:
	const int nLength = strCompareWith.GetLength ();
	const int nBufLength = strBuffer.GetLength ();
	int i = 0;

	for (i = 0; i < nLength && nBufferOffset + i < nBufLength; i++)
	{
		if (m_bCaseSensitive ? 
			(strCompareWith [i] != strBuffer [nBufferOffset + i]) :
			(_tcsnicmp (((LPCTSTR) strCompareWith) + i, 
						((LPCTSTR) strBuffer) + nBufferOffset + i, 1) != 0))
		{
			nEndOffset = nBufferOffset;
			return FALSE;
		}
	}

	if (m_bWholeWords /*&& strCompareWith.FindOneOf (m_strDelimiters) == -1*/)
	{
		if ((nBufferOffset > 0 &&
			 m_strDelimiters.Find (strBuffer [nBufferOffset - 1]) == -1) ||
			(nBufferOffset + i < nBufLength &&
			 m_strDelimiters.Find (strBuffer [nBufferOffset + i]) == -1))
		{
			nEndOffset = nBufferOffset;
			return FALSE;
		}
	}

	nEndOffset = nBufferOffset + i - 1;
	return (i > 0) && (i == nLength);
}
//************************************************************************************
BOOL CBCGPOutlineParser::IsEscapeSequence (const CString& strBuffer, int& nBufferOffset) const
{
	for (POSITION pos = m_lstEscapeSequences.GetHeadPosition (); pos != NULL; )
	{
		const CString& str = m_lstEscapeSequences.GetNext (pos);
		int nEndOffset;
		if (str.GetLength () > 0 &&
			Compare (strBuffer, nBufferOffset, str, nEndOffset))
		{
			nBufferOffset += str.GetLength ();
			return TRUE;
		}
	}

	return FALSE;
}
//************************************************************************************
BOOL CBCGPOutlineParser::IsValuedChar (TCHAR ch) const
{
	// Test escape sequences
	for (POSITION pos = m_lstEscapeSequences.GetHeadPosition (); pos != NULL; )
	{
		const CString& str = m_lstEscapeSequences.GetNext (pos);
		if (str.Find (ch) >= 0)
		{
			// a char can be a part of the escape sequence
			return TRUE;
		}
	}

	// Test all block types
	for (int i = 0; i <= m_arrBlockTypes.GetUpperBound (); i++)
	{
		BlockType* pBlockType = m_arrBlockTypes [i];
		ASSERT (pBlockType != NULL);

		if (pBlockType->m_strOpen.Find (ch) >= 0)
		{
			// a char can be a part of the start token of the outlining block
			return TRUE;
		}
		if (pBlockType->m_strClose.Find (ch) >= 0)
		{
			// a char can be a part of the end token of the outlining block
			return TRUE;
		}
	}

	// the char has no value for the parser and can be ignored
	return FALSE;
}
//************************************************************************************
Lexeme CBCGPOutlineParser::GetNext (const CString& strIn, int& nOffset, const int nSearchTo)
{
	while (nOffset >= 0 && nOffset < strIn.GetLength () && nOffset <= nSearchTo)
	{
		if (IsEscapeSequence (strIn, nOffset))
		{
			continue;
		}

		for (int i = 0; i <= m_arrBlockTypes.GetUpperBound (); i++)
		{
			BlockType* pBlockType = m_arrBlockTypes [i];
			ASSERT (pBlockType != NULL);

			// Nested blocks:
			if (pBlockType->m_bAllowNestedBlocks)
			{
				int nEndOffset;
				if (Compare (strIn, nOffset, pBlockType->m_strOpen, nEndOffset))
				{
					Lexeme lexem (i, LT_BlockStart, nOffset, nEndOffset);
					nOffset += pBlockType->m_strOpen.GetLength ();
					return lexem;
				}
				else if (Compare (strIn, nOffset, pBlockType->m_strClose, nEndOffset))
				{
					Lexeme lexem (i, LT_BlockEnd, nOffset, nEndOffset);
					nOffset += pBlockType->m_strClose.GetLength ();
					return lexem;
				}
			}
			// NonNested blocks:
			else
			{
				int nEndOffset;
				if (Compare (strIn, nOffset, pBlockType->m_strOpen, nEndOffset))
				{
					Lexeme lexem (i, LT_CompleteBlock, nOffset, nEndOffset);
					nOffset += pBlockType->m_strOpen.GetLength ();

					if (!pBlockType->m_strClose.IsEmpty ())
					{
						// find close token skipping escape sequences:
						while  (nOffset >= 0 && 
								nOffset < strIn.GetLength () && 
								nOffset <= nSearchTo)
						{
							if (IsEscapeSequence (strIn, nOffset))
							{
								continue;
							}

							if (Compare (strIn, nOffset, pBlockType->m_strClose, nEndOffset))
							{
								nOffset += pBlockType->m_strClose.GetLength ();
								if (pBlockType->m_strClose == _T("\n"))
								{
									nOffset--;
								}

								lexem.m_nEnd = nOffset - 1;
								return lexem;
							}

							nOffset++;
						}
					}
					
					if (pBlockType->m_bIgnore)
					{
						nOffset = lexem.m_nStart;
					}
					else
					{
						lexem.m_nEnd = strIn.GetLength () - 1;
						return lexem;
					}
				}
			}

		}

		nOffset++;
	}

	return Lexeme (0, LT_EndOfText, 0, 0);
}
//************************************************************************************
void CBCGPOutlineParser::PushResult (Lexeme lexem, CObList& lstResults)
{
#ifdef _DEBUG
	CString str;
#endif // _DEBUG

	const BlockType* pBlockType = GetBlockType (lexem.m_nBlockType);
	ASSERT (pBlockType != NULL);

	switch (lexem.m_nType)
	{
	case LT_CompleteBlock:
		{
			CBCGPOutlineBaseNode block;
			block.m_nStart		= lexem.m_nStart;
			block.m_nEnd		= lexem.m_nEnd;
			block.m_strReplace	= pBlockType->m_strReplace;
			block.m_nBlockType	= lexem.m_nBlockType;
			block.m_dwFlags		= g_dwOBFComplete;

			CBCGPOutlineNode* pNode = new CBCGPOutlineNode (block);
			ASSERT_VALID (pNode);

			lstResults.AddTail (pNode);
		}
		DEBUG_ONLY (str.Format (_T("%s_%d_%d, "), pBlockType->m_strReplace, lexem.m_nStart, lexem.m_nEnd));
		break;
	case LT_BlockStart:
		{
			CBCGPOutlineBaseNode block;
			block.m_nStart		= lexem.m_nStart;
			block.m_nEnd		= lexem.m_nEnd;
			block.m_strReplace	= pBlockType->m_strReplace;
			block.m_nBlockType	= lexem.m_nBlockType;
			block.m_dwFlags		= g_dwOBFLeft;

			CBCGPOutlineNode* pNode = new CBCGPOutlineNode (block);
			ASSERT_VALID (pNode);

			lstResults.AddTail (pNode);
		}
		DEBUG_ONLY (str.Format (_T("{_%d, "), lexem.m_nStart));
		break;
	case LT_BlockEnd:
		{
			CBCGPOutlineBaseNode block;
			block.m_nStart		= lexem.m_nStart;
			block.m_nEnd		= lexem.m_nEnd;
			block.m_strReplace	= pBlockType->m_strReplace;
			block.m_nBlockType	= lexem.m_nBlockType;
			block.m_dwFlags		= g_dwOBFRight;

			CBCGPOutlineNode* pNode = new CBCGPOutlineNode (block);
			ASSERT_VALID (pNode);

			lstResults.AddTail (pNode);
		}
		DEBUG_ONLY (str.Format (_T("}_%d, "), lexem.m_nEnd));
		break;
	case LT_Eps:
		DEBUG_ONLY (str = _T("Finished"));
		break;
	default:
		DEBUG_ONLY (str = _T("Error! "));
	}
	DEBUG_ONLY (m_strOut += str);
}
//************************************************************************************
void CBCGPOutlineParser::DoParse (const CString& strBuffer, 
								  const int nStartOffset, const int nEndOffset, 
								  CObList& lstResults)
{
	ASSERT (nStartOffset >= 0);
	ASSERT (nEndOffset < strBuffer.GetLength ());
	ASSERT (nStartOffset <= nEndOffset);

	m_strOut.Empty ();

	CList <Lexeme, Lexeme&> lstStack;
	Lexeme lexemStackTop (0, LT_Eps, 0, 0);
	lstStack.AddTail (lexemStackTop);

	int nOffset	= nStartOffset;

	while (nOffset <= nEndOffset)
	{
		// Get next lexem:
		Lexeme lexemNext = GetNext (strBuffer, nOffset, nEndOffset);
		Lexeme lexemTop = lstStack.GetTail ();

		if (lexemNext.m_nType == LT_EndOfText)
		{
			break;
		}

		// Parser logic:
		switch (lexemNext.m_nType)
		{
		case LT_BlockStart:
			lstStack.AddTail (lexemNext);
			break;
		case LT_BlockEnd:
			if (lexemTop.m_nType == LT_BlockStart &&
				lexemTop.m_nBlockType == lexemNext.m_nBlockType)
			{
				// Push Block:
				lstStack.RemoveTail ();
				Lexeme lexemRes (lexemTop.m_nBlockType, LT_CompleteBlock, lexemTop.m_nStart, lexemNext.m_nEnd);
				PushResult (lexemRes, lstResults);
			}
			else
			{
				lstStack.AddTail (lexemNext);
			}
			break;
		case LT_CompleteBlock:
			{
				// Push Comment:
				PushResult (lexemNext, lstResults);
			}
			break;

		case LT_CustomBlock:
			break;
		}
	}
	
	// Finish parsing:
	while (!lstStack.IsEmpty ())
	{
		Lexeme lexem = lstStack.RemoveTail ();
		PushResult (lexem, lstResults);
	}
}
//************************************************************************************
static void CalcMinMaxBounds (const CObList& lst, int& nMin, int& nMax)
{
	for (POSITION pos = lst.GetHeadPosition (); pos != NULL; )
	{
		CBCGPOutlineBaseNode* pNode = (CBCGPOutlineBaseNode*) lst.GetNext (pos);
		ASSERT_VALID (pNode);

		if (pNode->m_nStart < nMin)
		{
			nMin = pNode->m_nStart;
		}
		if (pNode->m_nEnd > nMax)
		{
			nMax = pNode->m_nEnd;
		}
	}
}
//************************************************************************************
void CBCGPOutlineParser::UpdateOutlining (CString& strBuffer, int nOffsetFrom, int nCharsCount,
							  CBCGPOutlineNode* pOutlineNode, BCGP_EDIT_OUTLINE_CHANGES& changes)
{
	ASSERT_VALID (this);
	ASSERT (nOffsetFrom >= 0);
	ASSERT (nCharsCount >= 0);
	ASSERT (nOffsetFrom + nCharsCount <= strBuffer.GetLength ());
	ASSERT_VALID (pOutlineNode);

	int nStartOffset = nOffsetFrom;
	int nEndOffset = nOffsetFrom + nCharsCount - 1;
	CBCGPOutlineNode* pChangedNode = GetRangeToReparse (pOutlineNode, nStartOffset, nEndOffset);
	nStartOffset = max (0, nStartOffset);
	nEndOffset = min (strBuffer.GetLength () - 1, nEndOffset);

	if (nStartOffset > nEndOffset)
	{
		return;	// no reparse needed
	}

	changes.m_nStartOffset = min (nStartOffset, changes.m_nStartOffset);
	changes.m_nEndOffset = max (nEndOffset, changes.m_nEndOffset);

	// --------------------------------------------------------------
	// 1) Parse text and find new blocks and part of blocks (markers)
	// --------------------------------------------------------------
	CObList lstBlocks;
	DoParse (strBuffer, nStartOffset, nEndOffset, lstBlocks);

		// ----------------------------------------------------------
		// Reparse text once more
		// if parser found blocks that are greater then parsing range 
		// ----------------------------------------------------------
		int nMin = nStartOffset, nMax = nEndOffset;
		CalcMinMaxBounds (lstBlocks, nMin, nMax);

		if (nMin < nStartOffset || nMax > nEndOffset)
		{
			nStartOffset = min (nMin, nStartOffset);
			nEndOffset = max (nMax, nEndOffset);

			pChangedNode = GetRangeToReparse (pOutlineNode, nStartOffset, nEndOffset);
			nStartOffset = max (0, nStartOffset);
			nEndOffset = min (strBuffer.GetLength () - 1, nEndOffset);

			changes.m_nStartOffset = min (nStartOffset, changes.m_nStartOffset);
			changes.m_nEndOffset = max (nEndOffset, changes.m_nEndOffset);

			while (!lstBlocks.IsEmpty ())
			{
				delete lstBlocks.RemoveTail ();
			}
			DoParse (strBuffer, nStartOffset, nEndOffset, lstBlocks);
		}

	// Update name offsets, start offsets and end offsets:
	DoUpdateOffsets (strBuffer, nStartOffset, nEndOffset, lstBlocks);

	// ------------------------
	// Reconstruct outline tree
	// ------------------------

	// 2) Del old blocks
	pOutlineNode->DeleteBlocksInRange (nStartOffset, nEndOffset, changes); // do not remove user blocks

	for (POSITION pos = lstBlocks.GetHeadPosition (); pos != NULL; )
	{
		POSITION posSave = pos;
		CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) lstBlocks.GetNext (pos);
		ASSERT_VALID (pNode);

		BlockType* pBlockType = m_arrBlockTypes [pNode->m_nBlockType];
		if (pBlockType != NULL && !pBlockType->m_bIgnore)
		{
			if ((pNode->m_dwFlags & g_dwOBFComplete) != 0)
			{
				// 3) Add new blocks
				AddNode (pNode, pOutlineNode, changes);
			}
			else if ((pNode->m_dwFlags & g_dwOBFLeft) != 0 ||
					 (pNode->m_dwFlags & g_dwOBFRight) != 0)
			{
				// it's possible that pChangedNode was removed from outline tree
				POSITION posChangedNode = changes.m_lstRemoved.Find (pChangedNode);
				if (posChangedNode != NULL)
				{
					if ((pNode->m_dwFlags & g_dwOBFLeft) != 0)
					{
						pChangedNode = pOutlineNode->GetInnermostBlock (pNode->m_nStart);
					}
					else if ((pNode->m_dwFlags & g_dwOBFRight) != 0)
					{
						pChangedNode = pOutlineNode->GetInnermostBlock (pNode->m_nEnd);
					}
					if (pChangedNode == NULL)
					{
						pChangedNode = pOutlineNode;
					}
				}

				ASSERT_VALID (pChangedNode);

				// 4) Reconstruct outline tree (add new markers)
				if (!AddMarker (pNode, pChangedNode, changes))
				{
					lstBlocks.RemoveAt (posSave);
					delete pNode;	// to prevent memory leak
				}
			}
		}
		else
		{
			// delete temporary blocks:
			lstBlocks.RemoveAt (posSave);
			delete pNode;
		}
	}
}
//************************************************************************************
CBCGPOutlineNode* CBCGPOutlineParser::GetRangeToReparse (CBCGPOutlineNode* pOutlineNode,
														int& nStartOffset, int& nEndOffset) const
{
	ASSERT_VALID (pOutlineNode);
	
	// --------------------------------------
	// Enlarge to start close string lengths:
	// --------------------------------------
	int nMaxBlockOpenStrLen = 1;
	int nMaxBlockCloseStrLen = 1;
	for (int i = 0; i <= m_arrBlockTypes.GetUpperBound (); i++)
	{
		BlockType* pBlockType = m_arrBlockTypes [i];
		ASSERT (pBlockType != NULL);

		if (pBlockType->m_strOpen.GetLength () > nMaxBlockOpenStrLen)
		{
			nMaxBlockOpenStrLen = pBlockType->m_strOpen.GetLength ();
		}
		if (pBlockType->m_strClose.GetLength () > nMaxBlockCloseStrLen)
		{
			nMaxBlockCloseStrLen = pBlockType->m_strClose.GetLength ();
		}
	}
	nStartOffset -= nMaxBlockOpenStrLen - 1;
	nEndOffset += nMaxBlockCloseStrLen - 1;

	// ----------------------
	// Enlarge to block size:
	// ----------------------
	CBCGPOutlineNode* pParentNode = pOutlineNode;
	CBCGPOutlineNode* pNode = NULL;
	POSITION pos = pParentNode->GetNodes ()->GetHeadPosition ();

	// By offset:
	if (nStartOffset > nEndOffset) // no range - use nStartOffset:
	{
		pParentNode = pOutlineNode->GetInnermostBlock (nStartOffset);

		// Ensure the block it's not user block:
		while (pParentNode != NULL && pParentNode != pOutlineNode && 
			   pParentNode->m_nBlockType < 0)
		{
			pParentNode = pParentNode->GetParentNode ();
		}

		if (pParentNode != NULL && pParentNode != pOutlineNode)
		{
			nStartOffset = pParentNode->m_nStart - pParentNode->m_nNameOffset;
			nEndOffset = pParentNode->m_nEnd;
			return pParentNode;
		}
		else
		{
			return pOutlineNode;
		}
	}

	// By range:
	while (pos != NULL)
	{
		pNode = (CBCGPOutlineNode*) pParentNode->GetNodes ()->GetNext (pos);
		ASSERT_VALID (pNode);

		CBCGPOutlineBaseNode::CBCGPEditOutlineRange range = 
			pNode->IsInRangeByOffset (nStartOffset, nEndOffset);
		if (range == FULL_IN_RANGE ||
			(pNode->m_nStart - pNode->m_nNameOffset >= nStartOffset && 
			 pNode->m_nStart - pNode->m_nNameOffset <= nEndOffset))
		{
			// full in range 
			// or left side in range - enlarge nEndOffset:
			nStartOffset = min (nStartOffset, pNode->m_nStart - pNode->m_nNameOffset);
			nEndOffset = max (nEndOffset, pNode->m_nEnd);
		}
		else if (range == PARTIAL_IN_RANGE)
		{
			if (pNode->m_nEnd >= nStartOffset && 
				pNode->m_nEnd <= nEndOffset)
			{
				// right side in range - enlarge nStartOffset:
				nStartOffset = min (nStartOffset, pNode->m_nStart - pNode->m_nNameOffset);
				nEndOffset = max (nEndOffset, pNode->m_nEnd);
			}
			else
			{
				// middle in range - level down:
				pParentNode = pNode;
				pNode = NULL;
				pos = pParentNode->GetNodes ()->GetHeadPosition ();

				const BlockType* pBlockType = GetBlockType (pParentNode->m_nBlockType);
				if (pBlockType == NULL || !pBlockType->m_bAllowNestedBlocks)
				{
					nStartOffset = min (nStartOffset, pParentNode->m_nStart - pParentNode->m_nNameOffset);
					nEndOffset = max (nEndOffset, pParentNode->m_nEnd);
				}
			}
		}
	}

	if (pParentNode != NULL && pParentNode->GetParentNode () != NULL)
	{
		nStartOffset = min (nStartOffset, pParentNode->m_nStart - pParentNode->m_nNameOffset);
		nEndOffset = max (nEndOffset, pParentNode->m_nEnd);
	}
	return pParentNode;
}
//************************************************************************************
void CBCGPOutlineParser::DoUpdateOffsets (const CString& strBuffer, 
										  const int nStartOffset, const int nEndOffset, 
										  CObList& lstBlocks)
{
	// Update name offsets, start offsets and end offsets:
	CBCGPOutlineNode* pPreviousNode = NULL;
	CObList lstIgnoreBlocks;
	for (POSITION pos = lstBlocks.GetHeadPosition (); pos != NULL; )
	{
		CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) lstBlocks.GetNext (pos);
		ASSERT_VALID (pNode);

		const BlockType* pBlockType = GetBlockType (pNode->m_nBlockType);
		if (pBlockType != NULL && !pBlockType->m_bIgnore)
		{
			// --------------------------------------
			// Update name offsets and start offsets:
			// --------------------------------------
			CString strName;
			int nSearchTo = nStartOffset;
			if (pPreviousNode != NULL && 
				pPreviousNode->m_nEnd < pNode->m_nStart &&
				pPreviousNode->m_nEnd > nStartOffset)
			{
				nSearchTo = pPreviousNode->m_nEnd;
			}
			pNode->m_nNameOffset = GetNameOffset (strBuffer, pNode->m_nStart, nSearchTo,
												  pBlockType, lstIgnoreBlocks, strName);
			int nNewStart = GetStartOffset (strBuffer, pNode->m_nStart, pNode->m_nStart - pNode->m_nNameOffset, lstIgnoreBlocks);
			ASSERT (nNewStart >= nSearchTo);

			int nDelta = nNewStart - pNode->m_nStart;
			pNode->m_nNameOffset += nDelta;
			pNode->m_nStart = nNewStart;
			// strName = strName.Left (strName.GetLength () + nDelta);
			// pNode->m_strName = strName;

			// -------------------
			// Update end offsets:
			// -------------------
			nSearchTo = nEndOffset;
			if (pos != NULL)
			{
				CBCGPOutlineNode* pNextNode = (CBCGPOutlineNode*) lstBlocks.GetAt (pos);
				ASSERT_VALID (pNextNode);

				if (pNextNode->m_nStart > pNode->m_nEnd &&
					pNextNode->m_nStart < nEndOffset)
				{
					nSearchTo = pNextNode->m_nStart;
				}
			}
			int nNewEnd = GetEndOffset (strBuffer, pNode->m_nEnd, nSearchTo);
			ASSERT (nNewEnd <= nSearchTo);
			pNode->m_nEnd = nNewEnd;

			pPreviousNode = pNode;
			lstIgnoreBlocks.AddTail (pNode);
		}
		else
		{
			lstIgnoreBlocks.AddTail (pNode);
		}
	}
}
//************************************************************************************
static BOOL IsInRange(CObList& lstIgnore, int nStart, int nEnd)
{
	for (POSITION pos = lstIgnore.GetTailPosition (); pos != NULL; )
	{
		CBCGPOutlineNode* pPrevNode = (CBCGPOutlineNode*) lstIgnore.GetPrev (pos);
		ASSERT_VALID (pPrevNode);

		if (pPrevNode->IsInRangeByOffset (nStart, nEnd) != NOT_IN_RANGE)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//************************************************************************************
int CBCGPOutlineParser::GetNameOffset (const CString& strIn, int nStartFrom, int nSearchTo,
									   const BlockType* pBlockType, CObList& lstIgnore,
									   CString& strName)
{
	ASSERT (nStartFrom >= 0);
	ASSERT (nSearchTo >= 0);
	ASSERT (pBlockType != NULL);

	int nOffset = nStartFrom - 1;
	const int nCharsMaxNum = 256;
	nSearchTo = max (nSearchTo, nStartFrom - nCharsMaxNum);
	LPCTSTR lpszDelimiters = _T(";{}");

	const CStringList* pListKeywords = &(pBlockType->m_lstKeywords);
	ASSERT_VALID (pListKeywords);

	if (pListKeywords->GetCount () == 0)
	{
		// if no keywords then nameoffset = 0
		nSearchTo = nStartFrom;
	}	

	while (nOffset >= nSearchTo)
	{
		CString strWord;
		nOffset = GetPrevWord (strIn, nOffset, nSearchTo, lpszDelimiters, strWord);
		if (nOffset >= nSearchTo &&
			!IsInRange (lstIgnore, nOffset, nOffset + strWord.GetLength ()))
		{
			// Compare strWord with keywords
			if (pListKeywords->Find (strWord) != NULL)
			{
				strName = strWord;
				return nStartFrom - nOffset;
			}
			else if (IsBlockName (strIn, strWord, nOffset, nStartFrom))
			{
				strName = strWord;
				return nStartFrom - nOffset;
			}
		}
		nOffset--;
	}

	strName = _T("");
	return 0;
}
//************************************************************************************
int CBCGPOutlineParser::GetPrevWord (const CString& strIn, int nStartFrom, 
							int nSearchTo, LPCTSTR lpszStopDelimiters, CString& strWord)
{
	ASSERT (nStartFrom >= 0);
	ASSERT (nStartFrom < strIn.GetLength ());
	ASSERT (nSearchTo >= 0);
	ASSERT (lpszStopDelimiters != NULL);

	CString strStopDelimiters = lpszStopDelimiters;
	CString strWordDelimeters = _T(" \t\n,./?><;:\"'{[}]~`%^&*()-+=!");

	// -------------------------------------------------------------------
	// Reverse search for a word till nSearchTo or one of spec delimiters:
	// -------------------------------------------------------------------
	strWord.Empty ();
	int i = nStartFrom;

	if (i >= nSearchTo &&
		strWordDelimeters.Find (strIn [i]) != -1)
	{
		// Inside delimiters - move to end of delimiters string:
		for (; i >= nSearchTo; i--)
		{
			TCHAR chNext = strIn.GetAt (i);
			if (strStopDelimiters.Find (chNext) != -1)
			{
				return -1;
			}
			if (strWordDelimeters.Find (chNext) == -1)
			{
				break;
			}
		}
	}
	if (i >= nSearchTo &&
		strWordDelimeters.Find (strIn [i]) == -1)
	{
		// Inside word - move to end of word:
		int iWordEnd = i;

		for (; i >= nSearchTo; i--)
		{
			TCHAR chNext = strIn.GetAt (i);
			if (strStopDelimiters.Find (chNext) != -1)
			{
				break;
			}
			if (strWordDelimeters.Find (chNext) != -1)
			{
				break;
			}
		}

		if (i < iWordEnd)
		{
			strWord = strIn.Mid (i + 1, iWordEnd - i);
			return i + 1;
		}
	}

	return -1;
}
//************************************************************************************
BOOL CBCGPOutlineParser::IsBlockName (const CString& strIn, CString& strWord, const int nOffset, const int nSearchTo)
{
	ASSERT (nOffset >= 0);
	ASSERT (nSearchTo >=0);

	CString strDelimeters = _T(" \t\n");
	int const nWordLength = strWord.GetLength ();
	
	// skip delimiters:
	int i = nOffset + nWordLength;
	int iMax = min (nSearchTo, strIn.GetLength ());
	while (i < iMax && strDelimeters.Find (strIn [i]) >= 0)
	{
		i++;
	}

	if (strIn [i] == _T('('))
	{
		strWord += strIn.Mid (i, iMax - i);
		return TRUE;
	}

	return FALSE;
}
//************************************************************************************
int CBCGPOutlineParser::GetStartOffset (const CString& strIn, int nStartFrom, int nSearchTo, CObList& lstIgnore)
{
	ASSERT (nStartFrom >= 0);
	ASSERT (nSearchTo >= 0);
	
	CString strDelimeters = _T(" \t\n");
	int i = nStartFrom;

	while (i > nSearchTo)
	{
		i--;
		if (strDelimeters.Find (strIn[i]) == -1 && 
			!IsInRange (lstIgnore, i, i))
		{
			nStartFrom = i + 1;
			break;
		}
	}
		
	return nStartFrom;
}
//************************************************************************************
int CBCGPOutlineParser::GetEndOffset (const CString& /*strIn*/, int nStartFrom, int /*nSearchTo*/)
{
	return nStartFrom;
}
//************************************************************************************
BOOL CBCGPOutlineParser::AddMarker (CBCGPOutlineNode* pMarkerBlock, CBCGPOutlineNode* pParentNode,
									BCGP_EDIT_OUTLINE_CHANGES& changes) const
{
	ASSERT_VALID (pMarkerBlock);
	ASSERT_VALID (pParentNode);
	ASSERT ((pMarkerBlock->m_dwFlags & g_dwOBFLeft) != 0 ||
			(pMarkerBlock->m_dwFlags & g_dwOBFRight) != 0);

	// ---------------------------------------
	// Search for marker fitting for this one:
	// ---------------------------------------
	CBCGPOutlineNode* pComposeWith = FindFittingBlock (pMarkerBlock, pParentNode);

	if (pComposeWith != NULL)
	{
		ASSERT_VALID (pComposeWith);

		// ---------------------------------------------------------
		// Combine new marker with existed block or with pair marker
		// ---------------------------------------------------------
		CBCGPOutlineBaseNode* pLeft = NULL;
		CBCGPOutlineBaseNode* pRight = NULL;

		if ((pMarkerBlock->m_dwFlags & g_dwOBFLeft) != 0)
		{
			ASSERT ((pComposeWith->m_dwFlags & g_dwOBFComplete) != 0 ||
					(pComposeWith->m_dwFlags & g_dwOBFRight) != 0);

			pLeft = pMarkerBlock;
			pRight = pComposeWith;
		}
		else if ((pMarkerBlock->m_dwFlags & g_dwOBFRight) != 0)
		{
			ASSERT ((pComposeWith->m_dwFlags & g_dwOBFComplete) != 0 ||
					(pComposeWith->m_dwFlags & g_dwOBFLeft) != 0);

			pLeft = pComposeWith;
			pRight = pMarkerBlock;
		}
		else
		{
			ASSERT (FALSE);
			return FALSE;
		}

		CBCGPOutlineNode* pParent = pComposeWith->GetParentNode ();
		ASSERT_VALID (pParent);

		if (pLeft != NULL && pRight != NULL)
		{
			ASSERT (pLeft->m_nStart < pRight->m_nEnd);
			ASSERT (pLeft->m_strReplace == pRight->m_strReplace);

			// ---------------
			// Make new block:
			// ---------------
			RemoveNode (pComposeWith, pParent, changes);
			CBCGPOutlineNode* pNewBlock = new CBCGPOutlineNode (*pLeft);
			pNewBlock->m_dwFlags &= ~g_dwOBFLeft;
			pNewBlock->m_dwFlags |= g_dwOBFComplete;
			pNewBlock->m_bCollapsed = FALSE;
			pNewBlock->m_nEnd = pRight->m_nEnd;

			AddNode (pNewBlock, pParent, changes);
		}
		else
		{
			ASSERT (FALSE);
			return FALSE;
		}

		if ((pComposeWith->m_dwFlags & g_dwOBFComplete) != 0)
		{
			// ----------------------------------------------------------------------
			// The rest of old block contains start/close sequence - add it as marker
			// ----------------------------------------------------------------------
			CBCGPOutlineNode* pNewMarker = new CBCGPOutlineNode (*pComposeWith->GetValue ());
			const BlockType* pBlockType = GetBlockType (pNewMarker->m_nBlockType);
			if ((pMarkerBlock->m_dwFlags & g_dwOBFLeft) != 0)
			{
				// add start marker (g_dwOBFLeft):
				const int nOpenStrLen = (pBlockType != NULL) ? pBlockType->m_strOpen.GetLength () : 1;
				pNewMarker->m_dwFlags &= ~g_dwOBFComplete;
				pNewMarker->m_dwFlags |= g_dwOBFLeft;
				pNewMarker->m_bCollapsed = FALSE;
				pNewMarker->m_nEnd = pNewMarker->m_nStart + nOpenStrLen - 1;
			}
			else if ((pMarkerBlock->m_dwFlags & g_dwOBFRight) != 0)
			{
				// add end marker (g_dwOBFRight):
				const int nCloseStrLen = (pBlockType != NULL) ? pBlockType->m_strClose.GetLength () : 1;
				pNewMarker->m_dwFlags &= ~g_dwOBFComplete;
				pNewMarker->m_dwFlags |= g_dwOBFRight;
				pNewMarker->m_bCollapsed = FALSE;
				pNewMarker->m_nStart = pNewMarker->m_nEnd - nCloseStrLen - 1;
				pNewMarker->DestroyData (); // end marker has no data
			}

			// Recursive call:
			AddMarker (pNewMarker, pParent, changes);
		}

		// ------------------
		// Delete old blocks:
		// ------------------
		delete pMarkerBlock;
	}
	else
	{
		// --------------------------------------------
		// Add non-completed block (marker) in the root
		// --------------------------------------------
		CBCGPOutlineNode* pRoot = pParentNode;
		while (pRoot->GetParentNode () != NULL)
		{
			ASSERT_VALID (pRoot);
			pRoot = pRoot->GetParentNode ();
		}
		AddNode (pMarkerBlock, pRoot, changes);
	}

	return TRUE;
}
//************************************************************************************
CBCGPOutlineNode* CBCGPOutlineParser::FindFittingBlock (CBCGPOutlineNode* pBlock, CBCGPOutlineNode* pParentNode) const
{
	ASSERT_VALID (pBlock);

	if ((pBlock->m_dwFlags & g_dwOBFComplete) != 0)
	{
		return NULL;
	}

	if (pParentNode == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pParentNode);

	// ----------------------------------------------------------------------
	// Search through the current level for the block fitting for this marker
	// ----------------------------------------------------------------------
	if ((pBlock->m_dwFlags & g_dwOBFLeft) != 0)
	{
		for (POSITION pos = pParentNode->GetNodes ()->GetHeadPosition (); pos != NULL;)
		{
			CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) pParentNode->GetNodes ()->GetNext (pos);
			ASSERT_VALID (pNode);

			if (pNode->m_nBlockType == pBlock->m_nBlockType)
			{
				if ((pNode->m_dwFlags & g_dwOBFComplete) != 0 &&
					pBlock->m_nEnd < pNode->m_nEnd &&
					pBlock->m_nStart > pNode->m_nStart)
				{
					return pNode;
				}
				else if ((pNode->m_dwFlags & g_dwOBFRight) != 0 &&
						 pBlock->m_nStart < pNode->m_nStart)
				{
					return pNode;
				}
			}
		}

		// Recursive call:
		return FindFittingBlock (pBlock, pParentNode->GetParentNode ());
	}
	else if ((pBlock->m_dwFlags & g_dwOBFRight) != 0)
	{
		for (POSITION pos = pParentNode->GetNodes ()->GetTailPosition (); pos != NULL;)
		{
			CBCGPOutlineNode* pNode = (CBCGPOutlineNode*) pParentNode->GetNodes ()->GetPrev (pos);
			ASSERT_VALID (pNode);

			if (pNode->m_nBlockType == pBlock->m_nBlockType)
			{
				if ((pNode->m_dwFlags & g_dwOBFComplete) != 0 &&
					pBlock->m_nStart > pNode->m_nStart &&
					pBlock->m_nEnd < pNode->m_nEnd)
				{
					return pNode;
				}
				else if ((pNode->m_dwFlags & g_dwOBFLeft) != 0 &&
						 pBlock->m_nStart > pNode->m_nStart)
				{
					return pNode;
				}
			}
		}

		// Recursive call:
		return FindFittingBlock (pBlock, pParentNode->GetParentNode ());
	}

	return NULL;
}

CBCGPOutlineNode* CBCGPOutlineParser::AddNode (CBCGPOutlineNode* pNewNode, CBCGPOutlineNode* pParentNode, 
											   BCGP_EDIT_OUTLINE_CHANGES& changes) const
{
	ASSERT_VALID (pParentNode);

	const BlockType* pBlockTypeDef = GetBlockType (pNewNode->m_nBlockType);
	if (pBlockTypeDef != NULL && !pBlockTypeDef->m_bAllowNestedBlocks)
	{
		// Nested blocks are not allowed - delete nested blocks:
		pParentNode->DeleteBlocksInRange (
			pNewNode->m_nStart - pNewNode->m_nNameOffset, 
			pNewNode->m_nEnd, changes);
	}

	return pParentNode->AddNode (pNewNode, changes);
}

CBCGPOutlineNode* CBCGPOutlineParser::RemoveNode (CBCGPOutlineNode* pNode, CBCGPOutlineNode* pParentNode, 
												  BCGP_EDIT_OUTLINE_CHANGES& changes,
												  BOOL bRemoveSubNodes) const
{
	ASSERT_VALID (pNode);
	ASSERT_VALID (pParentNode);
	POSITION posNode = pParentNode->m_lstNodes.Find (pNode);
	ASSERT (posNode != NULL);

	return pParentNode->RemoveNode (posNode, changes, bRemoveSubNodes);
}

#endif	// BCGP_EXCLUDE_EDIT_CTRL
