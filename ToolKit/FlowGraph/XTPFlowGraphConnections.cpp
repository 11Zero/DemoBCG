// XTPFlowGraphConnections.cpp : implementation of the CXTPFlowGraphConnections class.
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

#include "XTPFlowGraphPage.h"
#include "XTPFlowGraphConnection.h"
#include "XTPFlowGraphConnections.h"
#include "XTPFlowGraphControl.h"
#include "XTPFlowGraphUndoManager.h"
#include "XTPFlowGraphMessages.h"


CXTPFlowGraphConnections::CXTPFlowGraphConnections(CXTPFlowGraphPage* pPage)
{
	m_pPage = pPage;

}

CXTPFlowGraphConnections::~CXTPFlowGraphConnections()
{
	RemoveAll();

}

void CXTPFlowGraphConnections::RemoveAll()
{
	if (m_arrConnections.GetSize() == 0)
		return;

	for (int i = 0; i < m_arrConnections.GetSize(); i++)
	{
		m_arrConnections[i]->OnRemoved();
		m_arrConnections[i]->m_pPage = NULL;
		m_arrConnections[i]->InternalRelease();
	}
	m_arrConnections.RemoveAll();

	m_pPage->OnGraphChanged();

}

CXTPFlowGraphConnection* CXTPFlowGraphConnections::AddConnection(CXTPFlowGraphConnection* pConnection)
{

	CXTPFlowGraphControl* pControl = m_pPage->GetControl();
	if (!pControl)
		return pConnection;

	XTP_NM_FLOWGRAPH_CONNECTIONCHANGED cc;
	cc.pConnection = pConnection;
	cc.nAction = 0;

	if (pControl->SendNotifyMessage(XTP_FGN_CONNECTIONCHANGED, &cc.hdr) == -1)
		return pConnection;

	m_arrConnections.Add(pConnection);
	pConnection->m_pPage = m_pPage;

	pControl->GetUndoManager()->AddUndoCommand(new CXTPFlowGraphUndoAddConnectionCommand(pConnection));

	m_pPage->OnGraphChanged();

	return pConnection;
}

void CXTPFlowGraphConnections::Remove(CXTPFlowGraphConnection* pConnection)
{
	if (pConnection->m_pPage != m_pPage)
		return;

	for (int i = 0; i < m_arrConnections.GetSize(); i++)
	{
		if (m_arrConnections[i] == pConnection)
		{
			RemoveAt(i);
			return;
		}
	}
}

void CXTPFlowGraphConnections::RemoveAt(int nIndex)
{
	CXTPFlowGraphConnection* pConnection = GetAt(nIndex);
	if (!pConnection)
		return;

	CXTPFlowGraphControl* pControl = m_pPage->GetControl();
	if (!pControl)
		return;

	XTP_NM_FLOWGRAPH_CONNECTIONCHANGED cc;
	cc.pConnection = pConnection;
	cc.nAction = 1;

	if (pControl->SendNotifyMessage(XTP_FGN_CONNECTIONCHANGED, &cc.hdr) == -1)
		return;


	pControl->GetUndoManager()->AddUndoCommand(new CXTPFlowGraphUndoDeleteConnectionCommand(pConnection));

	m_arrConnections.RemoveAt(nIndex);
	pConnection->OnRemoved();

	pConnection->m_pPage = NULL;
	pConnection->InternalRelease();



	m_pPage->OnGraphChanged();
}

void CXTPFlowGraphConnections::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPPropExchangeEnumeratorPtr pEnumRecords(pPX->GetEnumerator(_T("Connection")));

	if (pPX->IsStoring())
	{
		int nCount = (int)GetCount();
		POSITION pos = pEnumRecords->GetPosition((DWORD)nCount);

		for (int i = 0; i < nCount; i++)
		{
			CXTPFlowGraphConnection* pConnection = GetAt(i);
			ASSERT(pConnection);

			CXTPPropExchangeSection sec(pEnumRecords->GetNext(pos));
			PX_Object(&sec, pConnection, RUNTIME_CLASS(CXTPFlowGraphConnection));
		}
	}
	else
	{
		RemoveAll();

		POSITION pos = pEnumRecords->GetPosition();

		while (pos)
		{
			CXTPFlowGraphConnection* pConnection = NULL;

			CXTPPropExchangeSection sec(pEnumRecords->GetNext(pos));
			PX_Object(&sec, pConnection, RUNTIME_CLASS(CXTPFlowGraphConnection));

			if (!pConnection)
				AfxThrowArchiveException(CArchiveException::badClass);

			pConnection->m_pPage = m_pPage;
			m_arrConnections.Add(pConnection);

			pConnection->RestoreConnection();

		}
	}

}



