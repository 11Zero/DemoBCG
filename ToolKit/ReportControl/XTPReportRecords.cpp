// XTPReportRecords.cpp : implementation of the CXTPReportRecords class.
//
// This file is a part of the XTREME REPORTCONTROL MFC class library.
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
#include "Resource.h"

#include "Common/XTPPropExchange.h"
#include "Common/XTPMarkupRender.h"

#include "XTPReportRecordItem.h"
#include "XTPReportRecord.h"
#include "XTPReportRecords.h"
#include "XTPReportInplaceControls.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CXTPReportRecords, CCmdTarget)

void CXTPReportRecords::_Init()
{
	m_pOwnerRecord = NULL;

	m_pVirtualRecord = NULL;
	m_nVirtualRecordsCount = 0;
	m_bArray = FALSE;

	m_bCaseSensitive = TRUE;
	m_pMarkupContext = NULL;


}

CXTPReportRecords::CXTPReportRecords(BOOL bArray /*= FALSE*/)
{
	_Init();
	m_bArray = bArray;
}

CXTPReportRecords::CXTPReportRecords(CXTPReportRecord* pOwnerRecord)
{
	_Init();
	m_pOwnerRecord = pOwnerRecord;
}

void CXTPReportRecords::SetVirtualMode(CXTPReportRecord* pVirtualRecord, int nCount)
{
	// release old virtual record
	if (m_pVirtualRecord)
		m_pVirtualRecord->InternalRelease();

	// reset virtual mode
	if (!pVirtualRecord || nCount <= 0)
	{
		if (pVirtualRecord)
			pVirtualRecord->InternalRelease();

		m_pVirtualRecord = NULL;
		m_nVirtualRecordsCount = 0;
	}
	else // set new virtual record
	{
		m_pVirtualRecord = pVirtualRecord;
		m_nVirtualRecordsCount = nCount;
		if (m_pVirtualRecord)
			m_pVirtualRecord->m_pRecords = this;
	}
}

CXTPReportRecords::~CXTPReportRecords()
{
	RemoveAll();

	if (m_pVirtualRecord)
		m_pVirtualRecord->InternalRelease();

	XTPMarkupReleaseContext(m_pMarkupContext);
}

void CXTPReportRecords::RemoveAll()
{
	if (!m_bArray)
	{
		// array cleanup
		for (int nRecord = (int) m_arrRecords.GetSize() - 1; nRecord >= 0; nRecord--)
		{
			CXTPReportRecord* pRecord = m_arrRecords.GetAt(nRecord);
			if (pRecord)
				pRecord->InternalRelease();
		}
	}
	m_arrRecords.RemoveAll();
}

void CXTPReportRecords::UpdateIndexes(int nStart /*= 0*/)
{
	for (int i = nStart; i < GetCount(); i++)
	{
		if (GetAt(i))
			GetAt(i)->m_nIndex = i;
	}
}

CXTPReportRecord* CXTPReportRecords::Add(CXTPReportRecord* pRecord)
{
	int nIndex = (int) m_arrRecords.Add(pRecord);

	if (m_bArray)
	{
		ASSERT(pRecord->m_pRecords);
	}
	else
	{
		pRecord->m_nIndex = nIndex;
		pRecord->m_pRecords = this;
	}

	return pRecord;
}

void CXTPReportRecords::RemoveAt(int nIndex)
{
	if (m_bArray)
	{
		m_arrRecords.RemoveAt(nIndex);
	}
	else
	{
		if (nIndex < (int) m_arrRecords.GetSize())
		{
			if (m_arrRecords[nIndex] != NULL)
				m_arrRecords[nIndex]->InternalRelease();
			m_arrRecords.RemoveAt(nIndex);

			UpdateIndexes(nIndex);
		}
	}
}

int CXTPReportRecords::RemoveRecord(CXTPReportRecord* pRecord)
{
	ASSERT(!m_bArray);

	for (int i = 0; i < (int) m_arrRecords.GetSize(); i++)
	{
		CXTPReportRecord* pRec = m_arrRecords.GetAt(i);
		if (pRec == pRecord)
		{
			pRecord->InternalRelease();
			m_arrRecords.RemoveAt(i);

			UpdateIndexes(i);

			return i;
		}
	}

	return - 1;
}

void CXTPReportRecords::InsertAt(int nIndex, CXTPReportRecord* pRecord)
{
	ASSERT(!m_bArray);

	m_arrRecords.InsertAt(nIndex, pRecord);
	pRecord->m_pRecords = this;
	UpdateIndexes(nIndex);
}

int CXTPReportRecords::GetCount() const
{
	if (m_pVirtualRecord != NULL)
		return m_nVirtualRecordsCount;

	return (int)m_arrRecords.GetSize();
}

CXTPReportRecord* CXTPReportRecords::GetAt(int nIndex) const
{
	if (m_pVirtualRecord)
	{
		m_pVirtualRecord->m_nIndex = nIndex;
		return m_pVirtualRecord;
	}

	return nIndex >= 0 && nIndex < GetCount() ? m_arrRecords.GetAt(nIndex) : NULL;
}

void CXTPReportRecords::DoPropExchange(CXTPPropExchange* pPX)
{
	pPX->ExchangeSchemaSafe();

	_DoPropExchange(pPX);
}

void CXTPReportRecords::_DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPPropExchangeEnumeratorPtr pEnumRecords(pPX->GetEnumerator(_T("Record")));

	if (pPX->IsStoring())
	{
		int nCount = (int)GetCount();
		POSITION pos = pEnumRecords->GetPosition((DWORD)nCount);

		for (int i = 0; i < nCount; i++)
		{
			CXTPReportRecord* pRecord = GetAt(i);
			ASSERT(pRecord);

			CXTPPropExchangeSection sec(pEnumRecords->GetNext(pos));
			PX_Object(&sec, pRecord, RUNTIME_CLASS(CXTPReportRecord));
		}
	}
	else
	{
		RemoveAll();

		POSITION pos = pEnumRecords->GetPosition();

		while (pos)
		{
			CXTPReportRecord* pRecord = NULL;

			CXTPPropExchangeSection sec(pEnumRecords->GetNext(pos));
			PX_Object(&sec, pRecord, RUNTIME_CLASS(CXTPReportRecord));

			if (!pRecord)
				AfxThrowArchiveException(CArchiveException::badClass);

			Add(pRecord);
		}
	}
}

void CXTPReportRecords::MoveRecord(int nIndex, CXTPReportRecord* pRecord, BOOL bUpdateIndexes)
{
	if (nIndex < 0)
		nIndex = 0;

	if (nIndex > GetCount())
		nIndex = GetCount();

	if (pRecord)
	{
		int nRecordIndex = pRecord->GetIndex();

		if (nRecordIndex == nIndex)
			return;

		if (GetAt(nRecordIndex) == pRecord)
		{
			if (nRecordIndex < nIndex)
				nIndex--;

			if (nRecordIndex == nIndex)
				return;

			m_arrRecords.RemoveAt(nRecordIndex);
			m_arrRecords.InsertAt(nIndex, pRecord);

			if (bUpdateIndexes)
				UpdateIndexes();
		}
	}
}

void CXTPReportRecords::Move(int nIndex, CXTPReportRecords* pRecords)
{
	if (pRecords->m_bArray)
	{
		if (nIndex < 0)
			nIndex = 0;

		int N = GetCount();

		if (nIndex > N)
			nIndex = N;

		int nRecordsCount = (int) pRecords->GetCount(), i;
		for (i = 0; i < nRecordsCount; i++)
		{
			CXTPReportRecord* pRecord = pRecords->GetAt(i);
			if (pRecord)
			{
				int nRecordIndex = pRecord->GetIndex();

				if (pRecord->m_pRecords != this)
					continue;

				if (GetAt(nRecordIndex) == pRecord)
				{
					m_arrRecords.RemoveAt(nRecordIndex);

					if (nRecordIndex < nIndex)
						nIndex--;

					for (int j = i + 1; j < nRecordsCount; j++)
					{
						pRecord = pRecords->GetAt(j);
						if (pRecord->m_pRecords != this)
							continue;

						if (pRecord->GetIndex() > nRecordIndex)
							pRecord->m_nIndex--;
					}
				}
				else
				{
					for (int k = 0; k < m_arrRecords.GetSize(); k++)
					{
						CXTPReportRecord* paRr = m_arrRecords.GetAt(k);
						if (pRecord == paRr)
							m_arrRecords.RemoveAt(k);
					}
				}
			}
		}

		for (i = 0; i < nRecordsCount; i++)
		{
			CXTPReportRecord* pRecord = pRecords->GetAt(i);
			if (pRecord)
			{
				if (pRecord->m_pRecords != this)
					continue;

				m_arrRecords.InsertAt(nIndex, pRecord);
				nIndex++;
			}
		}

		UpdateIndexes();
	}
}

int CXTPReportRecords::Compare(const CString& str1, const CString& str2) const
{
	if (!IsCaseSensitive())
		return str1.CompareNoCase(str2);

	return str1.Compare(str2);
}

void CXTPReportRecords::SetSize(INT_PTR nNewSize, INT_PTR nGrowBy)
{
	int nSize = GetCount();
	if (!m_bArray && nNewSize < nSize)
	{
		for (int i = (int) nNewSize; i < nSize; i++)
		{
			CXTPReportRecord* pRecord = GetAt(i);
			if (pRecord)
				pRecord->InternalRelease();
		}
	}
	m_arrRecords.SetSize(nNewSize, nGrowBy);
}

void CXTPReportRecords::SetAt(INT_PTR nIndex, CXTPReportRecord* pRecord)
{
	ASSERT(pRecord);
	ASSERT(nIndex >= 0 && nIndex < GetCount());

	if (!pRecord || nIndex < 0 || nIndex >= GetCount())
		return;

	if (!m_bArray)
	{
		CXTPReportRecord* pRecord_prev = GetAt((int)nIndex);
		if (pRecord_prev)
			pRecord_prev->InternalRelease();

		pRecord->m_nIndex = (int)nIndex;
		pRecord->m_pRecords = this;
	}

	m_arrRecords.SetAt(nIndex, pRecord);
}

CXTPReportRecord* CXTPReportRecords::FindRecordByBookmark(VARIANT vtBookmark, BOOL bSearchInChildren)
{
	for (int i = 0; i < GetCount(); i++)
	{
		CXTPReportRecord* pRecord = GetAt(i);
		if (!pRecord)
			continue;
		if (pRecord->GetBookmark().dblVal == vtBookmark.dblVal)
			return pRecord;
		if (pRecord->HasChildren() && pRecord->GetChilds() && bSearchInChildren)
		{
			pRecord = pRecord->GetChilds()->FindRecordByBookmark(vtBookmark, bSearchInChildren);
			if (pRecord)
				return pRecord;
		}
	}

	return NULL;
}

CXTPReportRecordItem* CXTPReportRecords::FindRecordItem(int nStartRecord, int nEndRecord,
														int nStartColumn, int nEndColumn,
														int nRecord, int nItem,
														LPCTSTR pcszText, int nFlags)
{
	CString sFind(pcszText);
	BOOL bExactStart = (nFlags & xtpReportTextSearchExactStart) > 0;
	if (bExactStart)
		nFlags -= xtpReportTextSearchExactStart;
	// validate parameters
	int N = GetCount();
	if (N < 1)
		return NULL;
	nStartRecord = max(nStartRecord, 0);
	nStartRecord = min(nStartRecord, N - 1);

	nEndRecord = max(nStartRecord, nEndRecord);
	nEndRecord = min(nEndRecord, N - 1);

	nStartColumn = max(nStartColumn, 0);
	nEndColumn = max(nEndColumn, 0);

	nEndColumn = max(nStartColumn, nEndColumn);

	nRecord = max(nRecord, 0);
	nRecord = min(nRecord, N - 1);

	nItem = max(nItem, nStartColumn);
	nItem = min(nEndColumn, nItem);
//-------------------------------------------------------
	if (nFlags & xtpReportTextSearchBackward)
	{
		for (int i = nEndRecord; i >= nStartRecord; i--)
		{
			if (i < nRecord) continue;

			CXTPReportRecord* pTryRecord = GetAt(i);
			if (pTryRecord)
			{
				for (int j = nEndColumn; j >= nStartColumn; j--)
				{
					if (i == nRecord && j < nItem) continue;

					CXTPReportRecordItem* pTryItem = pTryRecord->GetItem(j);
					if (pTryItem)
					{
						CString strCaption = pTryItem->GetCaption(NULL);
						if (strCaption.IsEmpty()) continue;
						int k = strCaption.Find(sFind);
						if (k > -1)
						{
							if (nFlags == (xtpReportTextSearchExactPhrase |
								xtpReportTextSearchMatchCase |
								xtpReportTextSearchBackward))
							{
								if (strCaption == sFind)
									return pTryItem;
							}
							else if (nFlags == (xtpReportTextSearchExactPhrase |
								xtpReportTextSearchBackward))
							{
								CString sCAP(strCaption);
								sCAP.MakeUpper();
								CString sFIND(sFind);
								sFIND.MakeUpper();
								if (sCAP == sFIND)
									return pTryItem;
							}
							else if (nFlags == (xtpReportTextSearchMatchCase |
								xtpReportTextSearchBackward))
							{
								if (strCaption.Mid(k) == sFind)
									return pTryItem;
							}
							else if (nFlags == xtpReportTextSearchBackward)
							{
								return pTryItem;
							}
						}
						else
						{
							CString sCAP(strCaption);
							sCAP.MakeUpper();
							CString sFIND(sFind);
							sFIND.MakeUpper();

							k = sCAP.Find(sFIND);
							if (k > -1)
							{
								if (nFlags == (xtpReportTextSearchExactPhrase |
									xtpReportTextSearchBackward))
								{
									if (sCAP == sFIND)
										return pTryItem;
								}
								else if (nFlags == xtpReportTextSearchBackward)
								{
									return pTryItem;
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for (int i = nStartRecord; i <= nEndRecord; i++)
		{
			if (i < nRecord) continue;

			CXTPReportRecord* pTryRecord = GetAt(i);
			if (pTryRecord)
			{
				for (int j = nStartColumn; j <= nEndColumn; j++)
				{
					if (i == nRecord && j < nItem) continue;

					CXTPReportRecordItem* pTryItem = pTryRecord->GetItem(j);
					if (pTryItem)
					{
						CString strCaption = pTryItem->GetCaption(NULL);
						if (strCaption.IsEmpty()) continue;
						int k = strCaption.Find(sFind);
						//if (k > -1)
						if ((bExactStart && k == 0) || (!bExactStart && k > -1))
						{
							if (nFlags == (xtpReportTextSearchExactPhrase |
								xtpReportTextSearchMatchCase))
							{
								if (strCaption == sFind)
									return pTryItem;
							}
							else if (nFlags == xtpReportTextSearchExactPhrase)
							{
								CString sCAP(strCaption);
								sCAP.MakeUpper();
								CString sFIND(sFind);
								sFIND.MakeUpper();
								if (sCAP == sFIND)
									return pTryItem;
							}
							else if (nFlags == xtpReportTextSearchMatchCase)
							{
								if (strCaption.Mid(k) == sFind)
									return pTryItem;
							}
							else if (nFlags == 0)
							{
								return pTryItem;
							}
						}
						else if (!bExactStart)
						{
							CString sCAP(strCaption);
							sCAP.MakeUpper();
							CString sFIND(sFind);
							sFIND.MakeUpper();

							k = sCAP.Find(sFIND);
							if (k > -1)
							{
								if (nFlags == xtpReportTextSearchExactPhrase)
								{
									if (sCAP == sFIND)
										return pTryItem;
								}
								else if (nFlags == 0)
								{
									return pTryItem;
								}
							}
						}
					}
				}
			}
		}
	}
	return NULL;
}

