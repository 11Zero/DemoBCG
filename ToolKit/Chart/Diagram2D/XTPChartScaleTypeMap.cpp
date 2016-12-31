// XTPChartScaleTypeMap.cpp
//
// This file is a part of the XTREME TOOLKIT PRO MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XTPChartScaleTypeMap.h"
#include "XTPChartAxis.h"
#include "XTPChartAxisView.h"
#include "../XTPChartSeries.h"
#include "../XTPChartSeriesView.h"
#include "../XTPChartSeriesPoint.h"
#include "../XTPChartDiagram.h"
#include "XTPChartDiagram2D.h"

#include "../../Common/XTPResourceManager.h"

#include <math.h>

CXTPChartScaleTypeMap::CXTPChartScaleTypeMap(CXTPChartAxis* pAxis)
{
	m_pOwner = pAxis;
}

CXTPChartScaleTypeMap::~CXTPChartScaleTypeMap()
{
}

CXTPChartScaleTypeMap* CXTPChartScaleTypeMap::Create(CXTPChartAxis* pAxis)
{
	if (pAxis->GetScaleType() == xtpChartScaleNumerical)
		return new CXTPChartNumericalScaleTypeMap(pAxis);

	if (pAxis->GetScaleType() == xtpChartScaleQualitative)
		return new CXTPChartQualitativeScaleTypeMap(pAxis);

	if (pAxis->GetScaleType() == xtpChartScaleDateTime)
		return new CXTPChartDateTimeScaleTypeMap(pAxis);

	return NULL;
}


void CXTPChartScaleTypeMap::UpdateSeries(CXTPChartAxisView* pAxisView)
{
	CXTPChartAxis* pAxis = GetAxis();

	CArray<CXTPChartSeriesView*, CXTPChartSeriesView*>& arrSeries = pAxisView->m_arrSeries;

	int i;

	for (i = 0; i < arrSeries.GetSize(); i++)
	{
		UpdateSeries(arrSeries.GetAt(i)->GetSeries());
	}

	if (!pAxis->IsValuesAxis())
	{
		for (i = 0; i < arrSeries.GetSize(); i++)
		{
			CXTPChartSeries* pSeries = arrSeries.GetAt(i)->GetSeries();

			for (int j = 0; j < pSeries->GetPoints()->GetCount(); j++)
			{
				UpdateSeriesPointArgument(pSeries->GetPoints()->GetAt(j));
			}
		}
	}
}

void CXTPChartScaleTypeMap::UpdateSeries(CXTPChartSeries* /*pSeries*/)
{

}

void CXTPChartScaleTypeMap::UpdateSeriesPointArgument(CXTPChartSeriesPoint* /*pPoint*/)
{

}


//////////////////////////////////////////////////////////////////////////
// CXTPChartNumericalMap


CXTPChartNumericalScaleTypeMap ::CXTPChartNumericalScaleTypeMap(CXTPChartAxis* pAxis)
	: CXTPChartScaleTypeMap(pAxis)
{
	m_dMinValue = 0;
	m_dMaxValue = 0;
}


CXTPChartString CXTPChartNumericalScaleTypeMap::InternalToValue(const CXTPChartString& strFormat, double dMark) const
{
	// Special case for 0
	if (fabs(dMark) < 1e-8)
		return CXTPChartString("0");

	CString str;
	str.Format(strFormat.IsEmpty() ? _T("%g") : strFormat, (float)dMark);
	return CXTPChartString(str);
}

double CXTPChartNumericalScaleTypeMap::ValueToInternal(const CXTPChartString& strValue) const
{
	double dValue = atof(XTP_CT2CA(strValue));
	return dValue;
}

void CXTPChartNumericalScaleTypeMap::UpdateSeriesPointArgument(CXTPChartSeriesPoint* pPoint)
{
	pPoint->SetInternalArgument(pPoint->GetArgumentValue());
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartNumericalMap


CXTPChartDateTimeScaleTypeMap ::CXTPChartDateTimeScaleTypeMap(CXTPChartAxis* pAxis)
	: CXTPChartScaleTypeMap(pAxis)
{
}

double CXTPChartDateTimeScaleTypeMap::ValueToInternal(const CXTPChartString& strValue) const
{
	COleDateTime dt;

	if (dt.ParseDateTime(strValue))
	{
		return (DATE)dt;
	}

	return 0;
}


void CXTPChartDateTimeScaleTypeMap::UpdateSeriesPointArgument(CXTPChartSeriesPoint* pPoint)
{
	COleDateTime dt;

	if (!pPoint->GetArgument().IsEmpty() && dt.ParseDateTime(pPoint->GetArgument()))
	{
		pPoint->SetInternalArgument((DATE)dt);
	}
	else
	{
		pPoint->SetInternalArgument(pPoint->GetArgumentValue());

	}
}


CXTPChartString CXTPChartDateTimeScaleTypeMap::InternalToValue(const CXTPChartString& strFormat, double dMark) const
{
	COleDateTime dt((DATE)dMark);

	if (strFormat.IsEmpty())
	{
		CXTPChartString str(dt.Format(VAR_DATEVALUEONLY));
		return str;
	}
	else
	{
		CXTPChartString str(dt.Format(strFormat));
		return str;

	}
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartQualitativeScaleTypeMap

template<> AFX_INLINE UINT AFXAPI HashKey(const CXTPChartString& key)
{
	// default identity hash - works for most primitive values
	return HashKey((LPCTSTR)key);
}

//===========================================================================
// Summary:
//     This class abstracts a storage mechamism for strings which are used in
//     charts with qualitative scale types.
// Remarks:
//===========================================================================
class CXTPChartQualitativeScaleTypeMap::CStorage
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get the zero based index of an arbitrary string.
	// Parameter:
	//     strValue - CXTPChartString denoting the value.
	// Returns:
	//     An integer denoting the index of the string.
	// Remarks:
	//-----------------------------------------------------------------------
	int IndexOf(const CXTPChartString& strValue) const
	{
		int nValue = m_arrList.IndexOf(strValue);
		return nValue;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Use this function to find an arbitrary string in the storage.
	// Parameter:
	//     strValue - A reference to the CXTPChartString denoting the value.
	// Returns:
	//     TRUE if it could find the string and FALSE if not.
	// Remarks:
	//-----------------------------------------------------------------------
	BOOL Find(const CXTPChartString& strValue) const
	{
		return IndexOf(strValue) != -1;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to add a collection of strings to the storage.
	// Parameter:
	//     strArray - A reference to the CXTPChartStrings denoting the strings
	//     collection.
	// Remarks:
	//-----------------------------------------------------------------------
	void AddRange(const CXTPChartStrings& strArray)
	{
		for (int i = 0; i < strArray.GetSize(); i++)
			m_arrList.Add(strArray[i]);

	}
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to add a string to the storage.
	// Parameter:
	//     str - A reference to the CXTPChartString denoting the string to be
	//           added to the storage.
	// Remarks:
	//-----------------------------------------------------------------------
	void Add(const CXTPChartString& str)
	{
		m_arrList.Add(str);

	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to insert a collection of strings to the storage,
	//     after a particular index.
	// Parameters:
	//     index    - The index after which the strings to be inserted.
	//     strArray - A reference to the CXTPChartStrings denoting the strings
	//     collection.
	// Remarks:
	//-----------------------------------------------------------------------
	void InsertRange(int index, const CXTPChartStrings& strArray)
	{
		for (int i = 0; i < strArray.GetSize(); i++)
			m_arrList.InsertAt(index + i, strArray[i]);

	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to insert a string to the storage,after a
	//     particular index.
	// Parameters:
	//     index    - The index after which the strings to be inserted.
	//     str      - A reference to the CXTPChartString denoting the string
	//     to be added.
	// Remarks:
	//-----------------------------------------------------------------------
	void Insert(int index, const CXTPChartString& str)
	{
		m_arrList.InsertAt(index, str);
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to get a string at a particular index.
	// Parameters:
	//     index    - The index from which the string to be extracted.
	// Returns:
	//     A CXTPChartString value.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartString GetAt(int nIndex)
	{
		if (nIndex >= 0 && nIndex < m_arrList.GetSize())
			return m_arrList[nIndex];

		return CXTPChartString();
	}

protected:
	CXTPChartStrings m_arrList;//The collection of strings.
};


CXTPChartQualitativeScaleTypeMap::CXTPChartQualitativeScaleTypeMap(CXTPChartAxis* pAxis)
	: CXTPChartScaleTypeMap(pAxis)
{

	m_pStorage = new CStorage();
}

CXTPChartQualitativeScaleTypeMap::~CXTPChartQualitativeScaleTypeMap()
{
	delete m_pStorage;
}


int CXTPChartStrings::IndexOf(const CXTPChartString& strValue) const
{
	for (int i = 0; i < GetSize(); i++)
	{
		if (GetAt(i) == strValue)
			return i;
	}
	return -1;
}


int CXTPChartQualitativeScaleTypeMap::FillNextsList(CXTPChartSeries* pSeries, int i, CXTPChartStrings& nexts, const CXTPChartString& argument_i)
{
	nexts.RemoveAll();

	for (int j = i + 1; j < pSeries->GetPoints()->GetCount(); j++)
	{
		CXTPChartString argument_j = pSeries->GetPoints()->GetAt(j)->GetArgument();
		if (argument_j == argument_i)
			continue;

		if (nexts.IndexOf(argument_j) >= 0)
			continue;

		if (m_pStorage->Find(argument_j))
			return m_pStorage->IndexOf(argument_j);

		nexts.Add(argument_j);
	}
	return -1;
}


void CXTPChartQualitativeScaleTypeMap::UpdateSeries(CXTPChartSeries* pSeries)
{
	int i = 0;
	CXTPChartStrings nexts;

	while (i < pSeries->GetPoints()->GetCount())
	{
		CXTPChartString argument_i = pSeries->GetPoints()->GetAt(i)->GetArgument();

		if (m_pStorage->Find(argument_i))
		{
			i++;
			continue;
		}

		int index = FillNextsList(pSeries, i, nexts, argument_i);
		if (index == -1)
		{
			m_pStorage->Add(argument_i);
			m_pStorage->AddRange(nexts);
		}
		else
		{
			m_pStorage->InsertRange(index, nexts);
			m_pStorage->Insert(index, argument_i);
		}
		i += (int)nexts.GetSize() + 1;
	}
}

void CXTPChartQualitativeScaleTypeMap::UpdateSeriesPointArgument(CXTPChartSeriesPoint* pPoint)
{
	pPoint->SetInternalArgument(ValueToInternal(pPoint->GetArgument()));
}

double CXTPChartQualitativeScaleTypeMap::ValueToInternal(const CXTPChartString& str) const
{
	return m_pStorage->IndexOf(str);
}

CXTPChartString CXTPChartQualitativeScaleTypeMap::InternalToValue(const CXTPChartString& strFormat, double dMark) const
{
	CXTPChartString str = m_pStorage->GetAt((int)dMark);

	if (strFormat.IsEmpty())
		return str;


	CXTPChartString str2;
	str2.Format(strFormat, (LPCTSTR)str);

	return str2;
}
