#if !defined(AFX_BCGPGRIDSERIALIZE_H__573C359C_D1FB_446F_B60D_B5E331228238__INCLUDED_)
#define AFX_BCGPGRIDSERIALIZE_H__573C359C_D1FB_446F_B60D_B5E331228238__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPGridSerialize.h : header file
//

#ifndef BCGP_EXCLUDE_GRID_CTRL

#include "BCGPGridCtrl.h"

class CBCGPGridRange;

/////////////////////////////////////////////////////////////////////////////
// Helper class to implement MOVE operation

class CMarkedGridRange
{
public:
	CMarkedGridRange () : m_range (), m_nShiftRowOffset (0), m_bInserted (FALSE)
	{
	}

	CMarkedGridRange (const CBCGPGridRange &src)
	{
		m_range.Set (src);
		m_nShiftRowOffset = 0;
		m_bInserted = FALSE;
	}
	
	CMarkedGridRange& operator=(const CMarkedGridRange &src)
	{
		m_range.Set (src.m_range);
		m_nShiftRowOffset = src.m_nShiftRowOffset;
		m_bInserted = src.m_bInserted;
		return *this;
	}
	
	operator CBCGPGridRange () const
	{
		CBCGPGridRange range = m_range;
		range.m_nTop += m_nShiftRowOffset;
		range.m_nBottom += m_nShiftRowOffset;
		
		return range;
	}
	
	void OnInsertRange (int nOffset, int nRowCount);
	void OnDeleteRange (int nOffset, int nRowCount);
	void SetRows (int nOffset, int nRowCount, BOOL bInserted = FALSE);
	
public:
	CBCGPGridRange	m_range;
	int				m_nShiftRowOffset;
	BOOL			m_bInserted;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridSerializeManager object

class BCGCBPRODLLEXPORT CBCGPGridSerializeManager: public CObject
{
public:
	enum ClipboardFormatType
	{
		CF_Rows = 1,
		CF_Items = 2
	};

protected:
	enum OperationType
	{
		OP_DragDrop = 1,
		OP_CopyPaste = 2
	};

	struct SerializedRange
	{
		CBCGPGridRange	range;
		UINT			nDataSize;
		BYTE*			pData;
	};

public:
	CBCGPGridSerializeManager (CBCGPGridCtrl* pOwnerGrid);
	virtual ~CBCGPGridSerializeManager ();

	// Settings:
	virtual BOOL SetClipboardFormatType (UINT nClipboardFormatType);// CF_Rows, CF_Items
	virtual CLIPFORMAT GetClipboardFormat () const;
	virtual LPCTSTR GetClipboardFormatName () const;

	// Operations:
	BOOL PrepareDataFromSelection ();	// Serialize selection to the internal storage
	BOOL SerializeTo (CFile& file);		// Write serialized data to file

	BOOL SerializeFrom (CFile& file);	// Load serialized data from file
	BOOL CanDrop (CBCGPGridItemID idDropTo, CBCGPGridItemID idDragFrom, BOOL bMove = FALSE, BOOL bInsertAfter = FALSE);
	BOOL Drop (CBCGPGridItemID idDropTo, CBCGPGridItemID idDragFrom, BOOL bMove = FALSE, BOOL bInsertAfter = FALSE);
 	BOOL CanPaste (CBCGPGridItemID idPasteTo, BOOL bCut = FALSE);
 	BOOL Paste (CBCGPGridItemID idPasteTo, BOOL bCut = FALSE);

	BOOL ClearPreviousSelection (BOOL bQueryNonEmptyItems);
	BOOL RemovePreviosSelection ();

	void CleanUp ();					// Cleanup serialized data

	virtual BOOL GetBoundingRange (CBCGPGridRange& rangeIndex, const CBCGPGridItemID& idTo);
	CBCGPGridItemID GetDropOffset (const CBCGPGridItemID& idDragFrom, const CBCGPGridItemID& idDropTo) const;

protected:
	// Implementation:
	BOOL DoDropItems (CBCGPGridItemID idDropTo, CBCGPGridItemID idDragFrom, BOOL bMove); // DeSerialize selection from the internal storage
	BOOL DoDropRows (CBCGPGridItemID idDropTo, CBCGPGridItemID idDragFrom,		// DeSerialize selection from the internal storage
					 BOOL bMove, BOOL bInsertAfter);

	void UpdateSelectionRect (const CBCGPGridItemID& idToOffset);
	BOOL CanReplaceNewSelection (CBCGPGridItemID idToOffset, BOOL bQueryNonEmptyItems);
	int  InsertNewSelection (int nOffset, CList<CBCGPGridRow*, CBCGPGridRow*> & lst);
	void CleanUpImplementationData ();

	BOOL WriteToArchive (CArchive& archive);
	BOOL ReadFromArchive (CArchive& archive);

	BOOL WriteItemsToArchive (CArchive& archive, const CBCGPGridRange& range);	// serialize range data
	BOOL ReadItemsFromArchive (CArchive& archive, const CBCGPGridRange& range);	// serialize range data
	BOOL WriteRowsToArchive (CArchive& archive, const CBCGPGridRange& range);	// serialize range data
	BOOL ReadRowsFromArchive (CArchive& archive, const CBCGPGridRange& range);	// serialize range data

	BOOL OrderToIndex (const CBCGPGridRange& rangeOrder, CBCGPGridRange& rangeIndex) const;
	BOOL IndexToOrder (const CBCGPGridRange& rangeIndex, CBCGPGridRange& rangeOrder) const;
	BOOL IndexToOrder (const CBCGPGridItemID& idIndex, CBCGPGridItemID& idOrder) const;

	void AddRange (const CBCGPGridRange& range, UINT nDataSize, BYTE* lpData);
	// Accessors to serialized ranges:
	int GetRangeCount () const;
	BOOL GetRange (int nIndex, CBCGPGridRange& range);
	BYTE* GetRangeData (int nIndex, UINT& nDataSize);

	virtual BOOL ArrangeRange (int nIndex, CBCGPGridRange& range);

public:
	// Settings:
	ClipboardFormatType	m_ClipboardFormatType;

protected:
	// Data:
	CBCGPGridItemID		m_idRangesOffset;
	CList<SerializedRange*, SerializedRange*>	m_lstRanges;	// serialized selection

	OperationType	m_Operation;		// OP_DragDrop, OP_CopyPaste
	BOOL			m_bSkipData;		// If TRUE - skip data when loading from archive
	CBCGPGridCtrl*	m_pOwnerGrid;

	BOOL m_bWholeRowSelected;	// If TRUE - the whole row is selected
	BOOL m_bWholeColSelected;	// If TRUE - the whole column is selected
	int m_nLastColumn;			// Order of the last visible column
	int m_nLastRow;				// Order of the last visible row

	// implementation data:
	CMarkedGridRange							m_InsertRange;	// used in RemovePreviosSelection
	CArray<CMarkedGridRange, CMarkedGridRange&> m_arrCutRanges;	// used in InsertNewSelection, RemovePreviosSelection
	CList<CBCGPGridRow*, CBCGPGridRow*>			m_lstNewRows;	// used in ReadRowsFromArchive

	// clipboard
	static CLIPFORMAT s_ClpFormat1;
	static CLIPFORMAT s_ClpFormat2;
	static CString s_ClpFormatName1;
	static CString s_ClpFormatName2;
};

/////////////////////////////////////////////////////////////////////////////

#endif // BCGP_EXCLUDE_GRID_CTRL

#endif // !defined(AFX_BCGPGRIDSERIALIZE_H__573C359C_D1FB_446F_B60D_B5E331228238__INCLUDED_)
