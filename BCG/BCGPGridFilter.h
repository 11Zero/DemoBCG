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
// BCGPGridFilter.h 
//

#if !defined(AFX_BCGPGRIDFILTER_H__10CABCAB_AAB9_4933_906E_4097D230362A__INCLUDED_)
#define AFX_BCGPGRIDFILTER_H__10CABCAB_AAB9_4933_906E_4097D230362A__INCLUDED_

#include "BCGCBPro.h"

/////////////////////////////////////////////////////////////////////////////
// BCGP_FILTER_COLUMN_INFO

struct BCGCBPRODLLEXPORT BCGP_FILTER_COLUMN_INFO	// used by filter callback function
{
// Construction
	BCGP_FILTER_COLUMN_INFO () : nCol (-1), bAll (TRUE) {}
	BCGP_FILTER_COLUMN_INFO (const BCGP_FILTER_COLUMN_INFO& src)
	{
		Copy (src);
	}

	void Clear ();
	void Copy (const BCGP_FILTER_COLUMN_INFO& src);
	BOOL IsEmpty () const;

// Attributes
	int				nCol;		// Column index

	CString			strFilter;	// Filter string text

	BOOL			bAll;
	CStringList		lstValues;	// Allowed values
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridFilterParams - stores filter info for columns

class BCGCBPRODLLEXPORT CBCGPGridFilterParams
{
	friend class CBCGPGridFilter;

// Construction:
public:
	CBCGPGridFilterParams ();
	~CBCGPGridFilterParams ();

// Operations:
public:
	void AddColumn (int nColumn, BCGP_FILTER_COLUMN_INFO* pParam = NULL);
	void ClearColumn (int nColumn);
	void ClearAll ();

	BCGP_FILTER_COLUMN_INFO* Column(int nColumn)
	{
		BCGP_FILTER_COLUMN_INFO* pParam = NULL;

		if (m_mapColumns.Lookup(nColumn, pParam) && pParam != NULL)
		{
			ASSERT(pParam->nCol == nColumn);
		}

		return pParam;
	}

// Attributes:
protected:
	CMap<int, int, BCGP_FILTER_COLUMN_INFO*, BCGP_FILTER_COLUMN_INFO*> m_mapColumns;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridFilter
// Implements the default callback function, stores filter info for one or all columns.

class CBCGPGridCtrl;
class CBCGPGridRow;

class BCGCBPRODLLEXPORT CBCGPGridFilter : public CObject
{
// Construction:
public:
	CBCGPGridFilter ();
	virtual ~CBCGPGridFilter() {}

	void SetFilter (CBCGPGridCtrl* pOwnerGrid);

// Overrides:
public:
	virtual BCGP_FILTER_COLUMN_INFO* GetColumnInfo (int nColumn);
	virtual BOOL FilterFunc (CBCGPGridRow* pRow, BCGP_FILTER_COLUMN_INFO* pParam); // The default implementation

// Attributes:
protected:
	CBCGPGridCtrl*			m_pOwnerGrid;

public:
	CBCGPGridFilterParams	m_params;
	BOOL					m_bOneColumnOnly;

public:
	static LRESULT CALLBACK pfnFilterCallback (WPARAM wParam, LPARAM lParam);
};

#endif // !defined(AFX_BCGPGRIDFILTER_H__10CABCAB_AAB9_4933_906E_4097D230362A__INCLUDED_)
