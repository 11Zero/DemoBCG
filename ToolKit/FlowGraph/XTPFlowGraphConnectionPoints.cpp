// XTPFlowGraphConnectionPoints.cpp : implementation of the CXTPFlowGraphConnectionPoints class.
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

#include "Common/XTPPropExchange.h"

#include "XTPFlowGraphNode.h"
#include "XTPFlowGraphConnectionPoint.h"
#include "XTPFlowGraphConnectionPoints.h"


CXTPFlowGraphConnectionPoints::CXTPFlowGraphConnectionPoints(CXTPFlowGraphNode* pNode)
{
	m_pNode = pNode;

}

CXTPFlowGraphConnectionPoints::~CXTPFlowGraphConnectionPoints()
{
	RemoveAll();
}

CXTPFlowGraphConnectionPoint* CXTPFlowGraphConnectionPoints::AddConnectionPoint(CXTPFlowGraphConnectionPoint* pConnectionPoint)
{
	m_arrConnectionPoints.Add(pConnectionPoint);
	pConnectionPoint->m_pNode = m_pNode;

	return pConnectionPoint;
}

void CXTPFlowGraphConnectionPoints::RemoveAll()
{
	if (m_arrConnectionPoints.GetSize() == 0)
		return;

	for (int i = 0; i < m_arrConnectionPoints.GetSize(); i++)
	{
		m_arrConnectionPoints[i]->m_pNode = NULL;
		m_arrConnectionPoints[i]->InternalRelease();
	}
	m_arrConnectionPoints.RemoveAll();

	m_pNode->OnGraphChanged();
}

void CXTPFlowGraphConnectionPoints::Remove(CXTPFlowGraphConnectionPoint* pConnectionPoint)
{
	if (!pConnectionPoint || pConnectionPoint->GetNode() != m_pNode)
		return;

	for (int i = 0; i < m_arrConnectionPoints.GetSize(); i++)
	{
		if (m_arrConnectionPoints[i] == pConnectionPoint)
		{
			RemoveAt(i);
			return;
		}
	}
}

void CXTPFlowGraphConnectionPoints::RemoveAt(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_arrConnectionPoints.GetSize())
		return;

	CXTPFlowGraphConnectionPoint* pConnectionPoint = m_arrConnectionPoints[nIndex];

	m_arrConnectionPoints.RemoveAt(nIndex);

	pConnectionPoint->m_pNode = NULL;
	pConnectionPoint->InternalRelease();

	m_pNode->OnGraphChanged();
}

void CXTPFlowGraphConnectionPoints::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPPropExchangeEnumeratorPtr pEnumRecords(pPX->GetEnumerator(_T("ConnectionPoint")));

	if (pPX->IsStoring())
	{
		int nCount = (int)GetCount();
		POSITION pos = pEnumRecords->GetPosition((DWORD)nCount);

		for (int i = 0; i < nCount; i++)
		{
			CXTPFlowGraphConnectionPoint* pConnectionPoint = GetAt(i);
			ASSERT(pConnectionPoint);

			CXTPPropExchangeSection sec(pEnumRecords->GetNext(pos));
			PX_Object(&sec, pConnectionPoint, RUNTIME_CLASS(CXTPFlowGraphConnectionPoint));
		}
	}
	else
	{
		RemoveAll();

		POSITION pos = pEnumRecords->GetPosition();

		while (pos)
		{
			CXTPFlowGraphConnectionPoint* pConnectionPoint = NULL;

			CXTPPropExchangeSection sec(pEnumRecords->GetNext(pos));

			if (!sec->ExchangeObjectInstance((CObject*&)pConnectionPoint, RUNTIME_CLASS(CXTPFlowGraphConnectionPoint)))
				AfxThrowArchiveException(CArchiveException::badClass);

			pConnectionPoint->m_pNode = m_pNode;
			pConnectionPoint->DoPropExchange(&sec);

			m_arrConnectionPoints.Add(pConnectionPoint);
		}
	}

}

CXTPFlowGraphConnectionPoint* CXTPFlowGraphConnectionPoints::FindConnectionPoint(LPCTSTR lpszCaption) const
{
	for (int i = 0; i < m_arrConnectionPoints.GetSize(); i++)
	{
		if (m_arrConnectionPoints[i]->GetCaption() == lpszCaption)
			return m_arrConnectionPoints[i];
	}
	return NULL;
}

CXTPFlowGraphConnectionPoint* CXTPFlowGraphConnectionPoints::FindConnectionPoint(int nId) const
{
	for (int i = 0; i < m_arrConnectionPoints.GetSize(); i++)
	{
		if (m_arrConnectionPoints[i]->GetID() == nId)
			return m_arrConnectionPoints[i];
	}
	return NULL;
}


