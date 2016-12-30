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
// BCGPGridSerialize.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPGridCtrl.h"
#include "BCGPGridSerialize.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarkedGridRange - Helper class to implement MOVE operation

void CMarkedGridRange::OnInsertRange (int nOffset, int nRowCount)
{
	if (m_range.m_nTop == -1 || nOffset < 0 || nRowCount <= 0)
	{
		return;
	}
	
	if (m_range.m_nTop + m_nShiftRowOffset >= nOffset)
	{
		m_nShiftRowOffset += nRowCount;
	}
}

void CMarkedGridRange::OnDeleteRange (int nOffset, int nRowCount)
{
	if (m_range.m_nBottom == -1 || nOffset < 0 || nRowCount <= 0)
	{
		return;
	}

	if ((m_bInserted ? m_range.m_nTop : m_range.m_nBottom) + m_nShiftRowOffset > nOffset)
	{
		m_nShiftRowOffset -= nRowCount;
	}
}

void CMarkedGridRange::SetRows (int nOffset, int nRowCount, BOOL bInserted)
{
	m_range.Set (0, nOffset, 0, nOffset + nRowCount - 1);
	m_nShiftRowOffset = 0;
	m_bInserted = bInserted;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridSerializeManager object

CLIPFORMAT	CBCGPGridSerializeManager::s_ClpFormat1 = 0;
CLIPFORMAT	CBCGPGridSerializeManager::s_ClpFormat2 = 0;
CString		CBCGPGridSerializeManager::s_ClpFormatName1 = _T("BCGPGridCtrlFmt1");
CString		CBCGPGridSerializeManager::s_ClpFormatName2 = _T("BCGPGridCtrlFmt2");

CBCGPGridSerializeManager::CBCGPGridSerializeManager (CBCGPGridCtrl* pOwnerGrid)
{
	m_pOwnerGrid = pOwnerGrid;

	if (s_ClpFormat1 == 0)
	{
		s_ClpFormat1 = (CLIPFORMAT)
			::RegisterClipboardFormat (s_ClpFormatName1);
	}
	if (s_ClpFormat2 == 0)
	{
		s_ClpFormat2 = (CLIPFORMAT)
			::RegisterClipboardFormat (s_ClpFormatName2);
	}
	
	m_ClipboardFormatType = CF_Items;
	m_Operation = OP_CopyPaste;
	m_bSkipData = FALSE;
	
	m_idRangesOffset = CBCGPGridItemID (0, 0);
	m_bWholeRowSelected = FALSE;
	m_bWholeColSelected = FALSE;
	m_nLastColumn = 0;
	m_nLastRow = 0;
}

CBCGPGridSerializeManager::~CBCGPGridSerializeManager ()
{
	CleanUp ();
}
//****************************************************************************************
void CBCGPGridSerializeManager::AddRange (const CBCGPGridRange& range, UINT nDataSize, BYTE* lpData)
{
	SerializedRange* pSR = new SerializedRange;
	pSR->range = range;
	pSR->nDataSize = nDataSize;
	pSR->pData = lpData;

	m_lstRanges.AddTail (pSR);
}
//****************************************************************************************
void CBCGPGridSerializeManager::CleanUp ()
{
	while (!m_lstRanges.IsEmpty ())
	{
		SerializedRange* pSR = m_lstRanges.RemoveTail ();
		
		if (pSR != NULL)
		{
			delete [] pSR->pData;
			pSR->pData = NULL;

			delete pSR;
		}
	}

	m_idRangesOffset = CBCGPGridItemID (0, 0);
	m_bWholeRowSelected = FALSE;
	m_bWholeColSelected = FALSE;
	m_nLastColumn = 0;
	m_nLastRow = 0;

	CleanUpImplementationData ();
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::SetClipboardFormatType (UINT nClipboardFormatType)
{
	if ((nClipboardFormatType & CF_Rows) != 0)
	{
		m_ClipboardFormatType = CF_Rows;
		return TRUE;
	}

	if ((nClipboardFormatType & CF_Items) != 0)
	{
		m_ClipboardFormatType = CF_Items;
		return TRUE;
	}

	return FALSE;
}
//****************************************************************************************
CLIPFORMAT CBCGPGridSerializeManager::GetClipboardFormat () const
{
	return (m_ClipboardFormatType == CF_Rows) ? s_ClpFormat1 : s_ClpFormat2;
}
//****************************************************************************************
LPCTSTR CBCGPGridSerializeManager::GetClipboardFormatName () const
{
	return (m_ClipboardFormatType == CF_Rows) ? s_ClpFormatName1 : s_ClpFormatName2;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::PrepareDataFromSelection ()
{
	ASSERT (m_pOwnerGrid != NULL);

	CleanUp ();

	if (m_pOwnerGrid != NULL)
	{
		m_nLastColumn = m_pOwnerGrid->GetColumnsInfo ().GetColumnCount (TRUE) - 1;
		m_nLastRow = m_pOwnerGrid->GetTotalRowCount () - 1;
	}

	if (m_ClipboardFormatType == CF_Rows)
	{
		if (!m_pOwnerGrid->NormalizeSelectionList ())
		{
			return FALSE;
		}
	}

	//---------------------
	// Calculate min offset
	//---------------------
	int i = 0;
	CBCGPGridItemID idOffset (-1, -1);
	for (i = 0; i < m_pOwnerGrid->GetSelectionCount (); i++)
	{
		CBCGPGridRange rangeIndex, rangeOrder;
		if (m_pOwnerGrid->GetSelection (i, rangeIndex) && 
			IndexToOrder (rangeIndex, rangeOrder))
		{
			const int nRow = (rangeOrder.m_nTop != -1) ? rangeOrder.m_nTop: 0;
			const int nCol = (rangeOrder.m_nLeft != -1) ? rangeOrder.m_nLeft: 0;
			
			if (idOffset.m_nRow == -1 || nRow <= idOffset.m_nRow)
			{
				idOffset.m_nRow = nRow;
			}
			
			if (idOffset.m_nColumn == -1 || nCol <= idOffset.m_nColumn)
			{
				idOffset.m_nColumn = nCol;
			}

			if (idOffset.m_nRow == -1 || 
				idOffset.m_nRow == 0 && idOffset.m_nRow == m_nLastRow)
			{
				m_bWholeColSelected = TRUE;
			}

			if (idOffset.m_nColumn == -1 || 
				idOffset.m_nColumn == 0 && idOffset.m_nColumn == m_nLastColumn)
			{
				m_bWholeRowSelected = TRUE;
			}
		}
	}
	
	if (idOffset.m_nRow == -1)
	{
		idOffset.m_nRow = 0;
	}
	if (idOffset.m_nColumn == -1)
	{
		idOffset.m_nColumn = 0;
	}

	ASSERT (idOffset.m_nRow >= 0);
	ASSERT (idOffset.m_nColumn >= 0);
	m_idRangesOffset = idOffset;

	//------------------------------
	// Prepare serialized selection:
	//------------------------------
	for (i = 0; i < m_pOwnerGrid->GetSelectionCount (); i++)
	{
		CBCGPGridRange rangeIndex, rangeOrder;
		if (m_pOwnerGrid->GetSelection (i, rangeIndex) &&
			IndexToOrder (rangeIndex, rangeOrder))
		{
			const int nRowOffset = (rangeOrder.m_nTop != -1) ? rangeOrder.m_nTop - m_idRangesOffset.m_nRow: 0;
			const int nColOffset = (rangeOrder.m_nLeft != -1) ? rangeOrder.m_nLeft - m_idRangesOffset.m_nColumn: 0;
		
			CBCGPGridRange rangeRelative (
				nColOffset, 
				nRowOffset, 
				nColOffset + (rangeOrder.m_nRight - rangeOrder.m_nLeft), 
				nRowOffset + (rangeOrder.m_nBottom - rangeOrder.m_nTop));

			try
			{
				CMemFile f;

				CArchive archive (&f, CArchive::store | CArchive::bNoFlushOnDelete);
				if (m_ClipboardFormatType == CF_Items)
				{
					WriteItemsToArchive (archive, rangeOrder);
				}
				else if (m_ClipboardFormatType == CF_Rows)
				{
					WriteRowsToArchive (archive, rangeOrder);
				}
				archive.Close ();

				if (f.GetLength () <= 0)
				{
					return FALSE;
				}
				
				UINT	cbSize = (UINT)f.GetLength ();
				BYTE*	pData = new BYTE[cbSize];
				
				if (NULL == pData)
				{
					delete [] pData;
					return FALSE;
				}
				
				f.SeekToBegin ();
				if (f.Read (pData, cbSize) != cbSize)
				{
					delete [] pData;
					return FALSE;
				}
				
				AddRange (rangeRelative, cbSize, pData);
			}
			catch (CFileException* pEx)
			{
				TRACE(_T("CBCGPGridSerializeManager::PrepareDataFromSelection. File exception\r\n"));
				pEx->Delete ();
				
				return FALSE;
			}
			catch (CException* e)
			{
				TRACE(_T("CBCGPGridSerializeManager::PrepareDataFromSelection. Exception\r\n"));
				e->Delete ();
				return FALSE;
			}
		}
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::SerializeTo (CFile& file)
{
	ASSERT (m_pOwnerGrid != NULL);

	CArchive archive (&file, CArchive::store | CArchive::bNoFlushOnDelete);
	BOOL bResult = WriteToArchive (archive);
	archive.Close ();
	
	return bResult;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::SerializeFrom (CFile& file)
{
	ASSERT (m_pOwnerGrid != NULL);
	
	CArchive archive (&file, CArchive::load | CArchive::bNoFlushOnDelete);
	BOOL bResult = ReadFromArchive (archive);
	archive.Close ();

	return bResult;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::CanDrop (CBCGPGridItemID idDropTo, CBCGPGridItemID idDragFrom,
										 BOOL bMove, BOOL bInsertAfter)
{
	m_Operation = OP_DragDrop;
	m_bSkipData = TRUE;
	
	if (m_ClipboardFormatType == CF_Rows)
	{
		return DoDropRows (idDropTo, idDragFrom, bMove, bInsertAfter);
	}

	return DoDropItems (idDropTo, idDragFrom, bMove);
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::Drop (CBCGPGridItemID idDropTo, CBCGPGridItemID idDragFrom,
									  BOOL bMove, BOOL bInsertAfter)
{
	m_Operation = OP_DragDrop;
	m_bSkipData = FALSE;

	if (m_ClipboardFormatType == CF_Rows)
	{
		return DoDropRows (idDropTo, idDragFrom, bMove, bInsertAfter);
	}

	return DoDropItems (idDropTo, idDragFrom, bMove);
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::CanPaste (CBCGPGridItemID idPasteTo, BOOL bCut)
{
	m_Operation = OP_CopyPaste;
	m_bSkipData = TRUE;

	CBCGPGridItemID idFrom;
	idFrom.SetNull ();

	if (m_ClipboardFormatType == CF_Rows)
	{
		return DoDropRows (idPasteTo, idFrom, bCut, FALSE);
	}
	
	return DoDropItems (idPasteTo, idFrom, bCut);
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::Paste (CBCGPGridItemID idPasteTo, BOOL bCut)
{
	m_Operation = OP_CopyPaste;
	m_bSkipData = FALSE;
	
	CBCGPGridItemID idFrom;
	idFrom.SetNull ();

	if (m_ClipboardFormatType == CF_Rows)
	{
		return DoDropRows (idPasteTo, idFrom, bCut, FALSE);
	}
	
	return DoDropItems (idPasteTo, idFrom, bCut);
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::DoDropItems (CBCGPGridItemID idDropTo, CBCGPGridItemID idDragFrom,
										BOOL bMove)
{
	// idDropTo - index
	// idDragFrom - index
	ASSERT (m_pOwnerGrid != NULL);
	ASSERT (m_ClipboardFormatType == CF_Items);

	if (GetRangeCount () == 0)
	{
		return FALSE; // no data
	}

	//---------------------
	// Calculate new offset
	//---------------------
	CBCGPGridItemID idTo = GetDropOffset (idDragFrom, idDropTo);
	ASSERT (idTo.m_nRow >= 0);
	ASSERT (idTo.m_nColumn >= 0);

	//------------------------------------
	// Can replace content of target area:
	//------------------------------------
	if (!m_bSkipData)
	{
		if (!CanReplaceNewSelection (idTo, TRUE))
		{
			return FALSE; // can't clear non empty items 
		}
	}

	if (!m_bSkipData && bMove)
	{
		//-------------------------------------
		// Clear content of previous selection:
		//-------------------------------------
		if (!ClearPreviousSelection (FALSE))
		{
			return FALSE; // can't clear non empty items 
		}
	}

	//-------------
	// Drop ranges:
	//-------------
	int i = 0;
	for (i = 0; i < GetRangeCount (); i++)
	{
		CBCGPGridRange rangeOrder;
		if (!GetRange (i, rangeOrder))
		{
			return FALSE;
		}

		rangeOrder.m_nLeft += idTo.m_nColumn;
		rangeOrder.m_nRight += idTo.m_nColumn;
		rangeOrder.m_nTop += idTo.m_nRow;
		rangeOrder.m_nBottom += idTo.m_nRow;

		if (rangeOrder.m_nBottom > m_nLastRow || rangeOrder.m_nRight > m_nLastColumn)
		{
			return FALSE; // out of grid range
		}

		//------------
		// Drop range:
		//------------
		UINT	nDataSize = 0;
		BYTE*	pData = GetRangeData (i, nDataSize);
		
		if (pData == NULL || nDataSize <= 0)
		{
			return FALSE;
		}
		
		CMemFile f(pData, nDataSize);

		CArchive archive (&f, CArchive::load | CArchive::bNoFlushOnDelete);
		if (!ReadItemsFromArchive (archive, rangeOrder))
		{
			return FALSE;
		}
		archive.Close ();
	}

 	if (!m_bSkipData)
	{
		//---------------------------
		// Remove previous selection:
		//---------------------------
		UpdateSelectionRect (m_idRangesOffset);

		//---------------
		// Set selection:
		//---------------
		BOOL bFirst = TRUE;
		for (i = 0; i < GetRangeCount (); i++)
		{
			CBCGPGridRange rangeOrder;
			if (!GetRange (i, rangeOrder))
			{
				continue;
			}

			rangeOrder.m_nLeft += idTo.m_nColumn;
			rangeOrder.m_nRight += idTo.m_nColumn;
			rangeOrder.m_nTop += idTo.m_nRow;
			rangeOrder.m_nBottom += idTo.m_nRow;

			CBCGPGridRange rangeIndex;
			if (!OrderToIndex (rangeOrder, rangeIndex))
			{
				continue;
			}
			
			CBCGPGridItemID idFirst (rangeIndex.m_nTop, rangeIndex.m_nLeft);
			CBCGPGridItemID idSecond (rangeIndex.m_nBottom, rangeIndex.m_nRight);
			
			DWORD dwSelMode = bFirst ? SM_SINGE_SEL_GROUP : SM_ADD_SEL_GROUP;
			bFirst = FALSE;
			
			m_pOwnerGrid->SetCurSel (idFirst, SM_FIRST_CLICK | dwSelMode, FALSE);
			m_pOwnerGrid->SetCurSel (idSecond, SM_SECOND_CLICK | SM_CONTINUE_SEL_GROUP, FALSE);
		}

		UpdateSelectionRect (idTo);
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::DoDropRows (CBCGPGridItemID idDropTo, CBCGPGridItemID idDragFrom,
											BOOL bMove, BOOL bInsertAfter)
{
	ASSERT (m_pOwnerGrid != NULL);
	ASSERT (m_ClipboardFormatType == CF_Rows);
	
	if (GetRangeCount () == 0)
	{
		return FALSE; // no data
	}

	// Copied rows are already removed if called from Paste or dragged from another app
	BOOL bNoRemove = idDragFrom.IsNull ();

	//---------------------
	// Calculate new offset
	//---------------------
	idDragFrom.SetNull ();
	if (bInsertAfter)
	{
		idDropTo.m_nRow++;
	}
	
	CBCGPGridItemID idTo = GetDropOffset (idDragFrom, idDropTo);
	ASSERT (idTo.m_nRow >= 0);
	ASSERT (idTo.m_nColumn >= 0);

	//----------------
	// Mark selection:	(save ranges to remove selection later)
	//----------------
	CleanUpImplementationData ();
	if (bNoRemove)
	{
		m_arrCutRanges.SetSize (0);
	}
	else
	{
		m_arrCutRanges.SetSize (GetRangeCount ());
		
		for (int i = 0; i < GetRangeCount (); i++)
		{
			CBCGPGridRange rangeOrder;
			if (!GetRange (i, rangeOrder))
			{
				return FALSE;
			}
			
			
			rangeOrder.m_nTop += m_idRangesOffset.m_nRow; 
			rangeOrder.m_nBottom += m_idRangesOffset.m_nRow;
			m_arrCutRanges [i] = (CMarkedGridRange) rangeOrder;
			
			if (rangeOrder.m_nTop < idDropTo.m_nRow &&
				rangeOrder.m_nBottom >= idDropTo.m_nRow)
			{
				//idDropTo = rangeOrder.m_nTop;
				return FALSE; // can't insert inside range
			}
		}
	}

	//-------------
	// Drop ranges:
	//-------------
	for (int i = 0; i < GetRangeCount (); i++)
	{
		CBCGPGridRange rangeOrder;
		if (!GetRange (i, rangeOrder))
		{
			return FALSE;
		}

		if (rangeOrder.m_nTop < 0 || rangeOrder.m_nBottom < rangeOrder.m_nTop)
		{
			return FALSE;
		}

		const int nRowOffset = -rangeOrder.m_nTop + idTo.m_nRow + (int)m_lstNewRows.GetCount ();
		rangeOrder.m_nTop += nRowOffset;
		rangeOrder.m_nBottom += nRowOffset;

		//------------
		// Drop range:
		//------------
		UINT	nDataSize = 0;
		BYTE*	pData = GetRangeData (i, nDataSize);
		
		if (pData == NULL || nDataSize <= 0)
		{
			return FALSE;
		}
		
		CMemFile f(pData, nDataSize);
		
		CArchive archive (&f, CArchive::load);
		if (!ReadRowsFromArchive (archive, rangeOrder))
		{
			return FALSE;
		}
		archive.Close ();
	}

 	if (!m_bSkipData)
 	{
		//---------------------------
		// Remove previous selection:
		//---------------------------
		CBCGPGridItemID id;
		m_pOwnerGrid->SetCurSel (id, SM_NONE, FALSE);

 		//-------------------------
 		// Insert new rows to grid:
 		//-------------------------
 		InsertNewSelection (idTo.m_nRow, m_lstNewRows);

		if (bMove)
		{
			//---------------------
			// Remove marked items (previous selection):
			//---------------------
			if (!RemovePreviosSelection ())
			{
				return FALSE; // can't clear non empty items 
			}
		}

		UpdateSelectionRect (idTo);
		m_pOwnerGrid->AdjustLayout ();

		//-------------------
		// Set new selection:
		//-------------------
		CBCGPGridRange rangeSel = m_InsertRange;
		if (rangeSel.IsValid ())
		{
			CBCGPGridItemID idFirst (rangeSel.m_nTop);
			CBCGPGridItemID idSecond (rangeSel.m_nBottom);
			
			m_pOwnerGrid->SetCurSel (idFirst, SM_FIRST_CLICK | SM_SINGE_SEL_GROUP, FALSE);
			m_pOwnerGrid->SetCurSel (idSecond, SM_SECOND_CLICK | SM_CONTINUE_SEL_GROUP, TRUE);
		}
	}

 	CleanUpImplementationData ();

	return TRUE;
}
//****************************************************************************************
void CBCGPGridSerializeManager::UpdateSelectionRect (const CBCGPGridItemID& idTo)
{
	// idTo is order
	ASSERT (m_pOwnerGrid != NULL);

	CBCGPGridRange range;
	if (!GetBoundingRange (range, idTo))
	{
		return;
	}

	CRect rectSelection = m_pOwnerGrid->GetRect (range);
	if (!rectSelection.IsRectEmpty ())
	{
		rectSelection.InflateRect (2, 1, 2, 2);
	}
	m_pOwnerGrid->InvalidateRect (rectSelection);
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::ClearPreviousSelection (BOOL bQueryNonEmptyItems)
{
	ASSERT (m_pOwnerGrid != NULL);
	
	BOOL bPrevQuery = FALSE;	// call OnQueryClearNonEmptyItem only once

	for (int i = 0; i < GetRangeCount (); i++)
	{
		CBCGPGridRange rangeOrder;
		if (!GetRange (i, rangeOrder))
		{
			return FALSE;
		}

		rangeOrder.m_nLeft += m_idRangesOffset.m_nColumn;
		rangeOrder.m_nRight += m_idRangesOffset.m_nColumn;
		rangeOrder.m_nTop += m_idRangesOffset.m_nRow;
		rangeOrder.m_nBottom += m_idRangesOffset.m_nRow;
		
		if (rangeOrder.m_nBottom > m_nLastRow || rangeOrder.m_nRight > m_nLastColumn)
		{
			return FALSE; // out of grid range
		}

		CBCGPGridRange rangeIndex;
		if (!OrderToIndex (rangeOrder, rangeIndex))
		{
			return FALSE;
		}

		if (!m_pOwnerGrid->ClearRange (rangeIndex, TRUE, bQueryNonEmptyItems, &bPrevQuery))
		{
			return FALSE;
		}
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::RemovePreviosSelection ()
{
	ASSERT (m_pOwnerGrid != NULL);

	if (m_arrCutRanges.GetSize () == 0)
	{
		m_arrCutRanges.SetSize (GetRangeCount ());
		
		int i = 0;
		for (i = 0; i < GetRangeCount (); i++)
		{
			CBCGPGridRange rangeOrder;
			if (!GetRange (i, rangeOrder))
			{
				return FALSE;
			}
			
			CMarkedGridRange markedRange = rangeOrder;
			markedRange.m_range.m_nTop += m_idRangesOffset.m_nRow; 
			markedRange.m_range.m_nBottom += m_idRangesOffset.m_nRow;
			
			m_arrCutRanges [i] = markedRange; 
		}
	}

	int nCount = 0;

 	//-------------
 	// Remove rows:
 	//-------------
 	for (int i = 0; i < m_arrCutRanges.GetSize (); i++)
 	{
		CBCGPGridRange rangeOrder = m_arrCutRanges [i];

		const int nRowOffset = rangeOrder.m_nTop;
		const int nRowCount = rangeOrder.m_nBottom - rangeOrder.m_nTop + 1;
		
		//--------------------------------
		// Collect list of items to delete
		//--------------------------------
		CList <POSITION, POSITION> lstDelItemsPos;
		CList <CBCGPGridRow*, CBCGPGridRow*> lstDelItemsPtr;
		
		for (int nRow = 0; nRow < nRowCount; nRow++)
		{
			int nRowIndex = nRow + nRowOffset;
			ASSERT(nRowIndex >= 0);
			
			m_pOwnerGrid->PrepareRemoveRow (nRowIndex, lstDelItemsPos, lstDelItemsPtr);
		}

		//--------------
		// Remove items:
		//--------------
		nCount += m_pOwnerGrid->DoRemoveRows (lstDelItemsPos, lstDelItemsPtr);

		//--------------------
		// Shift marked ranges
		//--------------------
		m_InsertRange.OnDeleteRange (nRowOffset, nRowCount);
		for (int j = 0; j < m_arrCutRanges.GetSize (); j++)
		{
			m_arrCutRanges [j].OnDeleteRange (nRowOffset, nRowCount);
		}
	}

	return (nCount > 0);
}
//****************************************************************************************
int CBCGPGridSerializeManager::InsertNewSelection (int nInsertPos, CList<CBCGPGridRow*, CBCGPGridRow*> & lst)
{
	ASSERT (m_pOwnerGrid != NULL);

	int nCount = 0;

	for (POSITION pos = lst.GetHeadPosition (); pos != NULL; )
	{
		POSITION posSave = pos;
		CBCGPGridRow* pRow = lst.GetNext (pos);

		if (pRow != NULL)
		{
			ASSERT_VALID (pRow);

			//-----------------------------------------
			// Insert new row at the specified position
			//-----------------------------------------
			int nRowIndex = -1;
			if (m_pOwnerGrid->GetRowCount () <= nInsertPos + nCount)
			{
				nRowIndex = m_pOwnerGrid->InsertRowAfter (nInsertPos + nCount, pRow, FALSE);
			}
			else
			{
				nRowIndex = m_pOwnerGrid->InsertRowBefore (nInsertPos + nCount, pRow, FALSE);
			}

			if (nRowIndex >= 0)
			{
				nCount++;
				lst.GetAt (posSave) = NULL;

				ASSERT (nRowIndex >= nInsertPos);
				ASSERT (nRowIndex <= nInsertPos + nCount);
			}
		}
	}

	if (nCount > 0)
	{
		// Save inserted range
		m_InsertRange.SetRows (nInsertPos, nCount, TRUE);

		// Shift marked ranges
		for (int j = 0; j < m_arrCutRanges.GetSize (); j++)
		{
			m_arrCutRanges [j].OnInsertRange (nInsertPos, nCount);
		}
	}

	return nCount;
}
//****************************************************************************************
void CBCGPGridSerializeManager::CleanUpImplementationData ()
{
	m_arrCutRanges.RemoveAll ();

	while (!m_lstNewRows.IsEmpty ())
	{
		CBCGPGridRow* pDelRow = m_lstNewRows.RemoveTail ();
		if (pDelRow != NULL)
		{
			delete pDelRow;
		}
	}
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::CanReplaceNewSelection (CBCGPGridItemID idTo, BOOL bQueryNonEmptyItems)
{
	ASSERT (m_pOwnerGrid != NULL);
	
	BOOL bPrevQuery = FALSE;	// call OnQueryClearNonEmptyItem only once
	
	for (int i = 0; i < GetRangeCount (); i++)
	{
		CBCGPGridRange rangeOrder;
		if (!GetRange (i, rangeOrder))
		{
			return FALSE;
		}
		
		rangeOrder.m_nLeft += idTo.m_nColumn;
		rangeOrder.m_nRight += idTo.m_nColumn;
		rangeOrder.m_nTop += idTo.m_nRow;
		rangeOrder.m_nBottom += idTo.m_nRow;
		
		if (rangeOrder.m_nBottom > m_nLastRow || rangeOrder.m_nRight > m_nLastColumn)
		{
			return FALSE; // out of grid range
		}

		CBCGPGridRange rangeIndex;
		if (!OrderToIndex (rangeOrder, rangeIndex))
		{
			return FALSE;
		}

		if (!m_pOwnerGrid->CanClearRange (rangeIndex, bQueryNonEmptyItems, &bPrevQuery, TRUE))
		{
			return FALSE;
		}
	}
	
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::WriteToArchive (CArchive& archive)
{
	if (GetRangeCount () == 0)
	{
		return FALSE; // no data
	}

	ASSERT (m_idRangesOffset.m_nRow >= 0);
	ASSERT (m_idRangesOffset.m_nColumn >= 0);

	//----------------------
	// Store serialized data
	//----------------------
	// int	- format type
	// int	- ranges common offset: row
	// int	- ranges common offset: column
	// int	- range count
	// ranges array:
	// int - range offset: row				\	
	// int - range offset: column			|	range
	// int - range size: row count			|
	// int - range size: columns count		|
	// UINT - size in bytes	| range data	|
	// BYTE - range data	|				/
	try
	{
		archive << (int) m_ClipboardFormatType;

		//------------------------------------
		// Store common offset for all ranges:
		//------------------------------------
		archive << m_idRangesOffset.m_nRow;
		archive << m_idRangesOffset.m_nColumn;

		//----------------------
		// Store list of ranges:
		//----------------------
		archive << (int) GetRangeCount ();

		for (int i = 0; i < GetRangeCount (); i++)
		{
			CBCGPGridRange range;
			if (!GetRange (i, range))
			{
				return FALSE;
			}

			//-------------
			// Store range:
			//-------------
			if (range.m_nTop < 0 || range.m_nLeft < 0)
			{
				return FALSE;
			}

			const int nRowCount = range.m_nBottom - range.m_nTop + 1;
			if (nRowCount <= 0)
			{
				return FALSE;
			}
			
			const int nColCount = range.m_nRight - range.m_nLeft + 1;
			if (nColCount <= 0)
			{
				return FALSE;
			}
			
			archive << (int) range.m_nTop;
			archive << (int) range.m_nLeft;
			archive << nRowCount;
			archive << nColCount;

			//------------------
			// Store range data:
			//------------------
			UINT	nDataSize = 0;
			BYTE*	pData = GetRangeData (i, nDataSize);

			if (pData == NULL || nDataSize <= 0)
			{
				return FALSE;
			}

			archive << nDataSize;
			archive.Write (pData, nDataSize);
		}
	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("CBCGPGridSerializeManager::WriteToArchive. Archive exception\r\n"));
		pEx->Delete ();
		return FALSE;
	}
	
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::ReadFromArchive (CArchive& archive)
{
	CleanUp ();

	if (m_pOwnerGrid != NULL)
	{
		m_nLastColumn = m_pOwnerGrid->GetColumnsInfo ().GetColumnCount (TRUE) - 1;
		m_nLastRow = m_pOwnerGrid->GetTotalRowCount () - 1;
	}

	//----------------------
	// Read serialized data:
	//----------------------
	// int	- format type
	// int	- ranges common offset: row
	// int	- ranges common offset: column
	// int	- range count
	// ranges array:
	// int - range offset: row				\	
	// int - range offset: column			|	range
	// int - range size: row count			|
	// int - range size: columns count		|
	// UINT - size in bytes	| range data	|
	// BYTE - range data	|				/
	try
	{
		int nClipboardFormatType = 0;;
		archive >> nClipboardFormatType;
		if (nClipboardFormatType != (int) m_ClipboardFormatType)
		{
			return FALSE;
		}

		//-----------------------------------
		// Read common offset for all ranges:
		//-----------------------------------
		archive >> m_idRangesOffset.m_nRow;
		archive >> m_idRangesOffset.m_nColumn;

		//---------------------
		// Read list of ranges:
		//---------------------
		int nRangeCount = 0;
		archive >> nRangeCount;
		if (nRangeCount <= 0)
		{
			return FALSE;
		}
		
		for (int nRange = 0; nRange < nRangeCount; nRange++)
		{
			//------------
			// Read range:
			//------------
			CBCGPGridItemID idOffset;
			archive >> idOffset.m_nRow;
			archive >> idOffset.m_nColumn;
			
			int nRowCount = 0;
			archive >> nRowCount;
			if (nRowCount <= 0)
			{
				return FALSE;
			}
			
			int nColCount = 0;
			archive >> nColCount;
			if (nColCount <= 0)
			{
				return FALSE;
			}
			
			CBCGPGridRange range (
				idOffset.m_nColumn, 
				idOffset.m_nRow,
				idOffset.m_nColumn + nColCount - 1,
				idOffset.m_nRow + nRowCount - 1);

			if (idOffset.m_nRow == 0 && nRowCount - 1 == m_nLastRow)
			{
				m_bWholeColSelected = TRUE;
			}

			if (idOffset.m_nColumn == 0 && nColCount - 1 == m_nLastColumn)
			{
				m_bWholeRowSelected = TRUE;
			}

			//-----------------
			// Read range data:
			//-----------------
			UINT nDataSize = 0;
			archive >> nDataSize;

			if (nDataSize <= 0)
			{
				return FALSE;
			}

			BYTE* pData = new BYTE[nDataSize];
			
			if (NULL == pData)
			{
				delete [] pData;
				return FALSE;
			}
			
			if (archive.Read (pData, nDataSize) != nDataSize)
			{
				delete [] pData;
				return FALSE;
			}
			
			AddRange (range, nDataSize, pData);
		}
	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("CBCGPGridSerializeManager::ReadFromArchive. Archive exception\r\n"));
		pEx->Delete ();
		return FALSE;
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::WriteItemsToArchive (CArchive& archive, const CBCGPGridRange& range)
{
	// range is order
	ASSERT (m_pOwnerGrid != NULL);
	ASSERT (m_ClipboardFormatType == CF_Items);
	
	const int nRowCount = range.m_nBottom - range.m_nTop + 1;
	if (nRowCount <= 0)
	{
		return FALSE;
	}
	
	const int nColCount = range.m_nRight - range.m_nLeft + 1;
	if (nColCount <= 0)
	{
		return FALSE;
	}

	const int nRowOffset = max (0, range.m_nTop);
	const int nColOffset = max (0, range.m_nLeft);

	const int nLastColumn = m_pOwnerGrid->GetColumnsInfo ().GetColumnCount (TRUE) - 1;

	try 
	{
		// array of items by rows:
		// int	- rowitems count		|	row
		// persistent CBCGPGridItem		|

		//---------------------------------
		// Serialize items inside the range
		//---------------------------------
		for (int nRow = 0; nRow < nRowCount; nRow++)
		{
			CBCGPGridRow* pRow = m_pOwnerGrid->GetRow (nRow + nRowOffset);
			if (pRow == NULL)
			{
				return FALSE;
			}
			ASSERT_VALID (pRow);
			
			const int nItemCount = min (range.m_nRight - range.m_nLeft + 1, pRow->GetItemCount ());
			archive << nItemCount;
			
			for (int nCol = 0; nCol < nItemCount; nCol++)
			{
				//-----------------
				// Store item data:
				//-----------------
				if (nCol + nColOffset > nLastColumn)
				{
					return FALSE;
				}

				int nColIndex = m_pOwnerGrid->GetColumnsInfo ().OrderToIndex (nCol + nColOffset);
				if (nColIndex == -1)
				{
					return FALSE;
				}
				
				CBCGPGridItem* pItem = pRow->GetItem (nColIndex);
				if (pItem == NULL)
				{
					return FALSE;
				}

				ASSERT_VALID (pItem);
				archive << pItem;
			}
			
		}

	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("CBCGPGridSerializeManager::WriteItemsToArchive. Archive exception\r\n"));
		pEx->Delete ();
		return FALSE;
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::ReadItemsFromArchive (CArchive& archive, const CBCGPGridRange& range)
{
	// range is order
	ASSERT (m_pOwnerGrid != NULL);
	ASSERT (m_ClipboardFormatType == CF_Items);

	const int nRowCount = range.m_nBottom - range.m_nTop + 1;
	if (nRowCount <= 0)
	{
		return FALSE;
	}
	
	const int nColCount = range.m_nRight - range.m_nLeft + 1;
	if (nColCount <= 0)
	{
		return FALSE;
	}

	const int nRowOffset = max (0, range.m_nTop);
	const int nColOffset = max (0, range.m_nLeft);

	const int nLastColumn = m_pOwnerGrid->GetColumnsInfo ().GetColumnCount (TRUE) - 1;
	const int nLastRow = m_pOwnerGrid->GetTotalRowCount () - 1;

	try
	{
		// array of items by rows:
		// int	- rowitems count		|	row
		// persistent CBCGPGridItem		|

		//---------------------------------
		// Serialize items inside the range
		//---------------------------------
		for (int nRow = 0; nRow < nRowCount; nRow++)
		{
			int nItemCount = 0;
			archive >> nItemCount;

			if (nItemCount > nLastColumn + 1)
			{
				return FALSE;
			}

			if (nRow + nRowOffset > nLastRow + 1)
			{
				return FALSE;
			}
			
			CBCGPGridRow* pRow = m_pOwnerGrid->GetRow (nRow + nRowOffset);
			if (pRow == NULL)
			{
				return FALSE;
			}
			ASSERT_VALID (pRow);
			
			for (int nCol = 0; nCol < nItemCount; nCol++)
			{
				//----------------
				// Read item data:
				//----------------
				if (nCol + nColOffset > nLastColumn)
				{
					return FALSE;
				}

				int nColIndex = m_pOwnerGrid->GetColumnsInfo ().OrderToIndex (nCol + nColOffset);
				if (nColIndex == -1)
				{
					return FALSE;
				}
				
				if (pRow->GetItem (nColIndex) == NULL)
				{
					return FALSE;
				}

				if (!m_pOwnerGrid->ReplaceItemFromArchive (archive, pRow, nColIndex, m_bSkipData))
				{
					return FALSE;
				}
			}
		}

	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("CBCGPGridSerializeManager::ReadItemsFromArchive. Archive exception\r\n"));
		pEx->Delete ();
		return FALSE;
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::WriteRowsToArchive (CArchive& archive, const CBCGPGridRange& range)
{
	ASSERT (m_pOwnerGrid != NULL);
	ASSERT (m_ClipboardFormatType == CF_Rows);
	
	const int nRowCount = range.m_nBottom - range.m_nTop + 1;
	if (nRowCount <= 0)
	{
		return FALSE;
	}
	
	const int nColCount = m_pOwnerGrid->GetColumnCount ();
	if (nColCount <= 0)
	{
		return FALSE;
	}
	
	const int nRowOffset = max (0, range.m_nTop);
	
	try 
	{
		// array of rows:
		// persistent CBCGPGridRow		\
		// int - items count			|	row
		// array of items in the row:	|
		// persistent CBCGPGridItem		/
		
		//--------------------------------
		// Serialize rows inside the range
		//--------------------------------
		for (int nRow = 0; nRow < nRowCount; nRow++)
		{
			//----------------
			// Store row data:
			//----------------
			CBCGPGridRow* pRow = m_pOwnerGrid->GetRow (nRow + nRowOffset);
			if (pRow == NULL)
			{
				return FALSE;
			}
			
			ASSERT_VALID (pRow);
			archive << pRow;
			
			const int nItemCount = min (nColCount, pRow->GetItemCount ());
			archive << nItemCount;
			
			for (int nCol = 0; nCol < nItemCount; nCol++)
			{
				//-----------------
				// Store item data:
				//-----------------
				CBCGPGridItem* pItem = pRow->GetItem (nCol);
				if (pItem == NULL)
				{
					return FALSE;
				}
				
				ASSERT_VALID (pItem);
				archive << pItem;
			}
			
		}
		
	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("CBCGPGridSerializeManager::WriteRowsToArchive. Archive exception\r\n"));
		pEx->Delete ();
		return FALSE;
	}
	
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::ReadRowsFromArchive (CArchive& archive, const CBCGPGridRange& range)
{
	ASSERT (m_pOwnerGrid != NULL);
	ASSERT (m_ClipboardFormatType == CF_Rows);
	
	const int nRowCount = range.m_nBottom - range.m_nTop + 1;
	if (nRowCount <= 0)
	{
		return FALSE;
	}
	
	const int nColCount = m_pOwnerGrid->GetColumnCount ();
	if (nColCount <= 0)
	{
		return FALSE;
	}
	
	const int nRowOffset = max (0, range.m_nTop);
	const int nLastRow = m_pOwnerGrid->GetTotalRowCount () - 1;
	
	try
	{
		// array of rows:
		// persistent CBCGPGridRow		\
		// int	- items count			|	row
		// array of items in the row:	|
		// persistent CBCGPGridItem		/
		
		//--------------------------------
		// Serialize rows inside the range
		//--------------------------------
		for (int nRow = 0; nRow < nRowCount; nRow++)
		{
			//---------------
			// Read row data:
			//---------------
			if (nRowOffset + nRow > nLastRow + 1 + (int)m_lstNewRows.GetCount ())
			{
				return FALSE;
			}
			
			CBCGPGridRow* pNewRow = m_pOwnerGrid->CreateRowFromArchive (archive, nRowOffset + nRow);
			
			if (pNewRow == NULL)
			{
				return FALSE;
			}
			ASSERT_VALID (pNewRow);
			
			int nItemCount = 0;
			archive >> nItemCount;
			
			if (nItemCount > m_pOwnerGrid->GetColumnCount ())
			{
				delete pNewRow;
				return FALSE;
			}
			
			for (int nCol = 0; nCol < nItemCount; nCol++)
			{
				//----------------
				// Read item data:
				//----------------
				if (!m_pOwnerGrid->AddItemFromArchive (archive, pNewRow, nCol, m_bSkipData))
				{
					delete pNewRow;
					return FALSE;
				}
			}
			
			//--------------
			// Save new row:
			//--------------
			m_lstNewRows.AddTail (pNewRow);
		}
		
	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("CBCGPGridSerializeManager::ReadRowsFromArchive. Archive exception\r\n"));
		pEx->Delete ();
		return FALSE;
	}
	
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::OrderToIndex (const CBCGPGridRange& rangeOrder, CBCGPGridRange& rangeIndex) const
{
	if (m_pOwnerGrid->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	rangeIndex = rangeOrder;

	if (rangeIndex.m_nLeft < 0 || rangeIndex.m_nLeft > m_pOwnerGrid->GetColumnCount () - 1 || 
		rangeIndex.m_nRight < 0 || rangeIndex.m_nRight > m_pOwnerGrid->GetColumnCount () - 1)
	{
		return FALSE;
	}

	rangeIndex.m_nLeft = m_pOwnerGrid->GetColumnsInfo ().OrderToIndex (rangeOrder.m_nLeft);
	if (rangeIndex.m_nLeft == -1)
	{
		return FALSE;
	}

	rangeIndex.m_nRight = m_pOwnerGrid->GetColumnsInfo ().OrderToIndex (rangeOrder.m_nRight);
	if (rangeIndex.m_nRight == -1)
	{
		return FALSE;
	}
	
	rangeIndex.Normalize ();
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::IndexToOrder (const CBCGPGridRange& rangeIndex, CBCGPGridRange& rangeOrder) const
{
	if (m_pOwnerGrid->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}
	
	rangeOrder = rangeIndex;

	int nColumnCount = m_pOwnerGrid->GetColumnCount ();
	if (rangeOrder.m_nLeft < 0 || rangeOrder.m_nLeft > nColumnCount || 
		rangeOrder.m_nRight < 0 || rangeOrder.m_nRight > nColumnCount)
	{
		return FALSE;
	}

	rangeOrder.m_nLeft = m_pOwnerGrid->GetColumnsInfo ().IndexToOrder (rangeIndex.m_nLeft);
	if (rangeOrder.m_nLeft == -1)
	{
		return FALSE;
	}

	rangeOrder.m_nRight = m_pOwnerGrid->GetColumnsInfo ().IndexToOrder (rangeIndex.m_nRight);
	if (rangeOrder.m_nRight == -1)
	{
		return FALSE;
	}
	
	rangeOrder.Normalize ();
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::IndexToOrder (const CBCGPGridItemID& idIndex, CBCGPGridItemID& idOrder) const
{
	if (m_pOwnerGrid->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}
	
	idOrder = idIndex;

	if (idOrder.m_nColumn < 0 || idOrder.m_nColumn > m_pOwnerGrid->GetColumnCount ())
	{
		return FALSE;
	}
	
	idOrder.m_nColumn = m_pOwnerGrid->GetColumnsInfo ().IndexToOrder (idIndex.m_nColumn);
	if (idOrder.m_nColumn == -1)
	{
		return FALSE;
	}
	
	return TRUE;
}
//****************************************************************************************
int CBCGPGridSerializeManager::GetRangeCount () const
{
	return (int) m_lstRanges.GetCount ();
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::GetRange (int nIndex, CBCGPGridRange& range)
{
	return ArrangeRange (nIndex, range);
}
//****************************************************************************************
BYTE* CBCGPGridSerializeManager::GetRangeData (int nIndex, UINT& nDataSize)
{
	POSITION pos = m_lstRanges.FindIndex (nIndex);
	if (pos != NULL)
	{
		SerializedRange* pSR = m_lstRanges.GetAt (pos);
		ASSERT (pSR != NULL);
		
		nDataSize = pSR->nDataSize;
		return pSR->pData;
	}
	
	nDataSize = 0;
	return NULL;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::GetBoundingRange (CBCGPGridRange& rangeIndex, const CBCGPGridItemID& idTo)
{
	// range is index
	// idTo is order
	ASSERT (m_pOwnerGrid != NULL);

	BOOL bFirst = TRUE;
	CBCGPGridRange rangeOrder;

	for (int i = 0; i < GetRangeCount (); i++)
	{
		CBCGPGridRange r2;
		if (ArrangeRange (i, r2))
		{
			if (bFirst)
			{
				rangeOrder = r2;
			}
			else
			{
				m_pOwnerGrid->UnionRange (&rangeOrder, &r2);
			}

			bFirst = FALSE;
		}
		else
		{
			return FALSE;
		}
	}

	if (idTo.m_nColumn != -1 && idTo.m_nRow != -1)
	{
		rangeOrder.m_nLeft += idTo.m_nColumn;
		rangeOrder.m_nRight += idTo.m_nColumn;
		rangeOrder.m_nTop += idTo.m_nRow;
		rangeOrder.m_nBottom += idTo.m_nRow;

		if (rangeOrder.m_nBottom > m_nLastRow || rangeOrder.m_nRight > m_nLastColumn)
		{
			return FALSE; // out of grid range
		}
	}

	if (!OrderToIndex (rangeOrder, rangeIndex))
	{
		return FALSE;
	}

	return !bFirst;
}
//****************************************************************************************
CBCGPGridItemID CBCGPGridSerializeManager::GetDropOffset (const CBCGPGridItemID& idDragFrom,
														  const CBCGPGridItemID& idDropTo) const
{
	if (idDragFrom.IsNull ())
	{
		CBCGPGridItemID idOffset;
		if (!IndexToOrder (idDropTo, idOffset))
		{
			idOffset.m_nRow = idDropTo.m_nRow;
			idOffset.m_nColumn = 0;
		}

		idOffset.m_nColumn = max (0, idOffset.m_nColumn);
		idOffset.m_nRow = max (0, idOffset.m_nRow);
		return idOffset;
	}

	CBCGPGridItemID idDelta(0 ,0);
	if (idDragFrom.m_nColumn != -1 && idDropTo.m_nColumn != -1 && !m_bWholeRowSelected)
	{
		CBCGPGridItemID idDragFromOrder, idDropToOrder;
		if (IndexToOrder (idDragFrom, idDragFromOrder) &&
			IndexToOrder (idDropTo, idDropToOrder))
		{
			idDelta.m_nColumn = idDropToOrder.m_nColumn - idDragFromOrder.m_nColumn;
		}
	}
	if (idDragFrom.m_nRow != -1 && idDropTo.m_nRow != -1 && !m_bWholeColSelected)
	{
		idDelta.m_nRow = idDropTo.m_nRow - idDragFrom.m_nRow;
	}
	
	CBCGPGridItemID idOffset = m_idRangesOffset;
	idOffset.m_nColumn = max (0, idOffset.m_nColumn + idDelta.m_nColumn);
	idOffset.m_nRow = max (0, idOffset.m_nRow + idDelta.m_nRow);
	
	return idOffset;
}
//****************************************************************************************
BOOL CBCGPGridSerializeManager::ArrangeRange (int nIndex, CBCGPGridRange& range)
{
	POSITION pos = m_lstRanges.FindIndex (nIndex);
	if (pos != NULL)
	{
		SerializedRange* pSR = m_lstRanges.GetAt (pos);
		ASSERT (pSR != NULL);

		range = pSR->range;
		return TRUE;
	}
	
	return FALSE;
}

#endif // BCGP_EXCLUDE_GRID_CTRL
