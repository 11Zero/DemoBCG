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
// BCGPGanttItemStorage.cpp: implementation of the CBCGPGanttItemStorage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPGanttItemStorage.h"
#include "BCGPGanttItem.h"
#include "BCGPGanttChart.h"

#ifndef BCGP_EXCLUDE_PLANNER
#include "BCGPAppointment.h"
#endif


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

const UINT BCGM_GANTT_STORAGE_CHANGED = ::RegisterWindowMessage (_T("BCGM_NM_GANTT_STORAGECHANGED"));
const UINT BCGM_GANTT_CONNECTION_ADDED = ::RegisterWindowMessage (_T("BCGM_NM_GANTT_LINKADDED"));
const UINT BCGM_GANTT_CONNECTION_REMOVED = ::RegisterWindowMessage (_T("BCGM_NM_GANTT_LINKREMOVED"));

IMPLEMENT_SERIAL (CBCGPGanttItemStorageBase, CObject, 1)

CBCGPGanttItemStorageBase::CBCGPGanttItemStorageBase ()
	: m_pOwnerWnd (NULL)
{
}

CBCGPGanttItemStorageBase::CBCGPGanttItemStorageBase (CWnd* pOwnerWnd)
	: m_pOwnerWnd (pOwnerWnd)
{

}

CBCGPGanttItemStorageBase::~CBCGPGanttItemStorageBase ()
{
}

void CBCGPGanttItemStorageBase::Serialize (CArchive& ar)
{
	if (ar.IsLoading ())
	{
		RemoveAll ();
		int nCount = 0;

		// Loading items
		ar >> nCount;

		while (nCount-- > 0)
		{
			CBCGPGanttItem* pItem = NULL;
			ar >> pItem;
			if (pItem != NULL)
			{
				ASSERT_VALID (pItem);
				pItem->m_pStorage = this;
				DoAdd (pItem);
			}
		}

		int nItems = GetCount ();

		// Loading connections
		ar >> nCount;

		while (nCount-- > 0)
		{
			int indexSrc = 0, indexDest = 0, linkType, reserved;

			ar >> indexSrc;
			ar >> indexDest;
			ar >> linkType;
			ar >> reserved;

			if (indexSrc >= 0 && indexSrc < nItems && indexDest >= 0 && indexDest < nItems)
			{
				ASSERT (indexSrc != indexDest);

				if (indexSrc != indexDest)
				{
					AddConnection (GetItem (indexSrc), GetItem (indexDest), linkType);
				}
			}
		}

		UpdateAll (BCGP_GANTT_ITEM_PROP_ALL);
	}
	else
	{
		// Saving items
		int nCount = GetCount ();
		ar << nCount;
		POSITION pos = GetHeadPosition ();

		while (pos != NULL && nCount-- > 0)
		{
			CBCGPGanttItem* pItem = GetNext (pos);
			ar << pItem;
		}

		ASSERT (nCount == 0);

		// Saving connections
		CArray <CBCGPGanttConnection*, CBCGPGanttConnection*> arrConnections;
		GetAllConnections (arrConnections);

		nCount = (int)arrConnections.GetSize ();
		ar << nCount;

		for (int i = 0; i < nCount; ++i)
		{
			int indexSrc = IndexOf (arrConnections[i]->m_pSourceItem);
			int indexDest = IndexOf (arrConnections[i]->m_pDestItem);
			int reserved = 0;

			ar << indexSrc;
			ar << indexDest;
			ar << arrConnections[i]->m_LinkType;
			ar << reserved;
		}
	}
}

void CBCGPGanttItemStorageBase::NotifyUpdateSingleItem (UINT uiAction, CBCGPGanttItem* pItem, int iItemIndex, DWORD dwFlags) const
{
	if (m_pOwnerWnd->GetSafeHwnd () != NULL)
	{
		BCGP_GANTT_STORAGE_UPDATE_INFO notify;
		notify.uiAction = uiAction;
		notify.iFirstItemIndex = iItemIndex;
		notify.iLastItemIndex = iItemIndex;
		notify.pItem = pItem;
		notify.dwFlags = dwFlags;
		notify.dwReserved = 0;

		m_pOwnerWnd->SendNotifyMessage (BCGM_GANTT_STORAGE_CHANGED, 0, (LPARAM)&notify);
	}
}

void CBCGPGanttItemStorageBase::NotifyUpdateItems (UINT uiAction, int iFirstItem, int iLastItem, DWORD dwFlags) const
{
	if (m_pOwnerWnd != NULL)
	{
		BCGP_GANTT_STORAGE_UPDATE_INFO notify;
		notify.uiAction = uiAction;
		notify.iFirstItemIndex = iFirstItem;
		notify.iLastItemIndex = iLastItem;
		notify.pItem = NULL;
		notify.dwFlags = dwFlags;
		notify.dwReserved = 0;

		m_pOwnerWnd->SendNotifyMessage (BCGM_GANTT_STORAGE_CHANGED, 0, (LPARAM)&notify);
	}
}


CBCGPGanttItem* CBCGPGanttItemStorageBase::FindByData (DWORD_PTR dwData)
{
	POSITION pos = GetHeadPosition ();
	while (pos != NULL)
	{
		CBCGPGanttItem* pItem = GetNext (pos);
		ASSERT_VALID (pItem);
		if (pItem->GetData () == dwData)
		{
			return pItem;
		}
	}
	return NULL;
}

void CBCGPGanttItemStorageBase::UpdateItem (CBCGPGanttItem* pItem, DWORD dwFlags) const
{
	NotifyUpdateSingleItem (BCGP_GANTT_STORAGE_UPDATE_ITEM, pItem, IndexOf (pItem), dwFlags);
}

void CBCGPGanttItemStorageBase::UpdateAll (DWORD dwFlags) const
{
	int nCount = GetCount ();
	if (nCount > 0)
	{
		NotifyUpdateItems (BCGP_GANTT_STORAGE_UPDATE_ITEM, 0, nCount - 1, dwFlags);
	}
}

void CBCGPGanttItemStorageBase::GetItemReferrers (const CBCGPGanttItem*, CConnectionArray& array)
{
	array.RemoveAll ();
}

void CBCGPGanttItemStorageBase::GetItemLinks (const CBCGPGanttItem*, CConnectionArray& array)
{
	array.RemoveAll ();
}

void CBCGPGanttItemStorageBase::GetAllConnections (CConnectionArray& array)
{
	array.RemoveAll ();
}

// default stubs

#ifdef _DEBUG
	#define IMPLEMENT_IN_DERIVED_CLASS  ASSERT (FALSE)
#else
	#define IMPLEMENT_IN_DERIVED_CLASS
#endif

int CBCGPGanttItemStorageBase::GetCount () const
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return (int)0;
}

POSITION CBCGPGanttItemStorageBase::GetHeadPosition () const
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return (POSITION)NULL;
}

POSITION CBCGPGanttItemStorageBase::GetItemPosition  (const CBCGPGanttItem*) const
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return (POSITION)NULL;
}

CBCGPGanttItem* CBCGPGanttItemStorageBase::GetNext (POSITION&) const
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return NULL;
}

CBCGPGanttItem* CBCGPGanttItemStorageBase::GetPrev (POSITION&) const
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return NULL;
}

void CBCGPGanttItemStorageBase::DoAdd (CBCGPGanttItem*)
{
	IMPLEMENT_IN_DERIVED_CLASS;
}

void CBCGPGanttItemStorageBase::DoInsert (int, CBCGPGanttItem*)
{
	IMPLEMENT_IN_DERIVED_CLASS;
}

void CBCGPGanttItemStorageBase::DoSwap (int, int)
{
	IMPLEMENT_IN_DERIVED_CLASS;
}

void CBCGPGanttItemStorageBase::DoRemove (CBCGPGanttItem*)
{
	IMPLEMENT_IN_DERIVED_CLASS;
}

void CBCGPGanttItemStorageBase::DoRemoveAt (int)
{
	IMPLEMENT_IN_DERIVED_CLASS;
}

void CBCGPGanttItemStorageBase::DoRemoveAll ()
{
	IMPLEMENT_IN_DERIVED_CLASS;
}

int CBCGPGanttItemStorageBase::IndexOf (const CBCGPGanttItem*) const
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return -1;
}

bool CBCGPGanttItemStorageBase::Contains (const CBCGPGanttItem*) const
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return false;
}

CBCGPGanttItem* CBCGPGanttItemStorageBase::GetItem (int) const
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return NULL;
}

CBCGPGanttConnection* CBCGPGanttItemStorageBase::AddConnection (CBCGPGanttItem*, CBCGPGanttItem*, int)
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return NULL;
}

CBCGPGanttConnection* CBCGPGanttItemStorageBase::FindConnection (const CBCGPGanttItem*, const CBCGPGanttItem*)
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return NULL;
}

BOOL CBCGPGanttItemStorageBase::RemoveConnection (CBCGPGanttItem*, CBCGPGanttItem*)
{
	IMPLEMENT_IN_DERIVED_CLASS;
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//              public methods implementation
//////////////////////////////////////////////////////////////////////////

void CBCGPGanttItemStorageBase::Add (CBCGPGanttItem* pItem)
{
	ASSERT (pItem->m_pStorage == NULL);

	if (pItem->m_pStorage == NULL)
	{
		pItem->m_pStorage = this;
		DoAdd (pItem);
		NotifyUpdateSingleItem (BCGP_GANTT_STORAGE_INSERT_ITEM, pItem, GetCount () - 1, BCGP_GANTT_ITEM_PROP_ALL);
	}
}

void CBCGPGanttItemStorageBase::Insert (int pos, CBCGPGanttItem* pItem)
{
	ASSERT (pItem->m_pStorage == NULL);

	if (pItem->m_pStorage == NULL)
	{
		pItem->m_pStorage = this;
		int nCount = GetCount ();
		if (pos < nCount)
		{
			DoInsert (pos, pItem);
			NotifyUpdateSingleItem (BCGP_GANTT_STORAGE_INSERT_ITEM, pItem, pos, BCGP_GANTT_ITEM_PROP_ALL);
		}
		else
		{
			DoAdd (pItem);
			NotifyUpdateSingleItem (BCGP_GANTT_STORAGE_INSERT_ITEM, pItem, nCount, BCGP_GANTT_ITEM_PROP_ALL);
		}
	}
}

void CBCGPGanttItemStorageBase::Move (int posOld, int posInsert)
{
	int nCount = GetCount ();
	if (posOld >= 0 && posInsert >= 0 && posOld < nCount && posInsert <= nCount && posOld != posInsert)
	{
		CBCGPGanttItem* pItem = GetItem (posOld);

		DoRemoveAt (posOld);

		if (posOld < posInsert)
		{
			if (posInsert < nCount)
			{
				DoInsert (posInsert - 1, pItem);
				NotifyUpdateItems (BCGP_GANTT_STORAGE_UPDATE_ITEM, posOld, posInsert, BCGP_GANTT_ITEM_PROP_ALL);
			}
			else
			{
				DoAdd (pItem);
				NotifyUpdateItems (BCGP_GANTT_STORAGE_UPDATE_ITEM, posOld, nCount, BCGP_GANTT_ITEM_PROP_ALL);
			}
		}
		else
		{
			DoInsert (posInsert, pItem);
			NotifyUpdateItems (BCGP_GANTT_STORAGE_UPDATE_ITEM, posInsert, posOld, BCGP_GANTT_ITEM_PROP_ALL);
		}
	}
}

void CBCGPGanttItemStorageBase::Swap (int pos1, int pos2)
{
	int nCount = GetCount ();
	if (pos1 >= 0 && pos2 >= 0 && pos1 < nCount && pos2 < nCount && pos1 != pos2)
	{
		DoSwap (pos1, pos2);
		NotifyUpdateSingleItem (BCGP_GANTT_STORAGE_UPDATE_ITEM, GetItem (pos1), pos1, BCGP_GANTT_ITEM_PROP_ALL);
		NotifyUpdateSingleItem (BCGP_GANTT_STORAGE_UPDATE_ITEM, GetItem (pos2), pos2, BCGP_GANTT_ITEM_PROP_ALL);
	}
}

void CBCGPGanttItemStorageBase::RemoveRange (int iFirst, int iLast)
{
		int nConnectionsRemoved = 0;

		if (iFirst == iLast)    // Single item
		{
			CBCGPGanttItem* pItem = GetItem (iFirst);
			NotifyUpdateSingleItem (BCGP_GANTT_STORAGE_BEFORE_REMOVE_ITEM, pItem, iFirst, BCGP_GANTT_ITEM_PROP_NONE);

			nConnectionsRemoved = RemoveItemConnections (pItem);
			DoRemove (pItem);

			NotifyUpdateSingleItem (BCGP_GANTT_STORAGE_REMOVED_ITEM, pItem, iFirst, BCGP_GANTT_ITEM_PROP_NONE);
			delete pItem;
		}
		else
		{
			NotifyUpdateItems (BCGP_GANTT_STORAGE_BEFORE_REMOVE_ITEM, iFirst, iLast, BCGP_GANTT_ITEM_PROP_NONE);

			CList <CBCGPGanttItem*, CBCGPGanttItem*> lstDelete;
			for (int k = iLast; k >= iFirst; --k)
			{
				CBCGPGanttItem* pItem = GetItem (k);
				lstDelete.AddTail (pItem);

				nConnectionsRemoved += RemoveItemConnections (pItem);
				DoRemoveAt (k);
			}

			NotifyUpdateItems (BCGP_GANTT_STORAGE_REMOVED_ITEM, iFirst, iLast, BCGP_GANTT_ITEM_PROP_NONE);

			for (POSITION pos = lstDelete.GetHeadPosition (); pos != NULL;)
			{
				CBCGPGanttItem* p = lstDelete.GetNext (pos);
				delete p;
			}
		}

		if (nConnectionsRemoved > 0)
		{
			UpdateAll (BCGP_GANTT_ITEM_PROP_NONE); // connections changed
		}
}


void CBCGPGanttItemStorageBase::Remove (CBCGPGanttItem* pItem)
{
	if (pItem != NULL)
	{
		int iFirst = IndexOf (pItem);
		int iLast = iFirst;

		if (pItem->IsGroupItem ())
		{
			iLast = iFirst + GetSubItemsCount (pItem);
		}

		RemoveRange (iFirst, iLast);
	}
}

void CBCGPGanttItemStorageBase::RemoveAt(int iFirst)
{
	CBCGPGanttItem* pItem = GetItem (iFirst);
	ASSERT_VALID (pItem);

	int iLast = iFirst;
	if (pItem != NULL && pItem->IsGroupItem ())
	{
		iLast = iFirst + GetSubItemsCount (pItem);
	}

	RemoveRange (iFirst, iLast);
}

void CBCGPGanttItemStorageBase::RemoveAll()
{
	int nCount = GetCount ();
	if (nCount <= 0)
	{
		return;
	}

	RemoveRange (0, nCount - 1);
}

#ifndef BCGP_EXCLUDE_PLANNER
BOOL CBCGPGanttItemStorageBase::InsertAppointment (int pos, const CBCGPAppointment* pAppointment)
{
	if (pAppointment == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pAppointment);

	CBCGPGanttItem* pItem = new CBCGPGanttItem;
	pItem->SetInterval (pAppointment->GetStart (), pAppointment->GetFinish ());
	pItem->SetName (pAppointment->GetDescription ());
	Insert (pos, pItem);
	return TRUE;
}
#endif

int CBCGPGanttItemStorageBase::GetSubItemsCount (const CBCGPGanttItem* pGroupItem, CBCGPGanttItem** ppLastSubItem) const
{
	CBCGPGanttItem* pItem = NULL;
	int nCount = 0;

	if (pGroupItem != NULL && pGroupItem->IsGroupItem ())
	{
		DWORD dwLevel = pGroupItem->GetHierarchyLevel ();
		POSITION pos = GetItemPosition (pGroupItem);
		GetNext (pos);

		while (pos != NULL)
		{
			pItem = GetNext (pos);
			ASSERT_VALID (pItem);

			if (pItem->GetHierarchyLevel () <= dwLevel)
			{
				break;
			}

			nCount ++;
		}
	}

	if (ppLastSubItem != NULL)
	{
		*ppLastSubItem = pItem;
	}

	return nCount;
}

CBCGPGanttItem* CBCGPGanttItemStorageBase::GetParentGroupItem (const CBCGPGanttItem* pSubItem) const
{
	if (pSubItem != NULL)
	{
		ASSERT_VALID (pSubItem);

		DWORD dwLevel = pSubItem->GetHierarchyLevel ();
		POSITION pos = GetItemPosition (pSubItem);

		while (pos != NULL)
		{
			CBCGPGanttItem* pItem = GetPrev (pos);
			ASSERT_VALID (pItem);

			if (pItem->IsGroupItem () && pItem->GetHierarchyLevel () < dwLevel)
			{
				return pItem;
			}
		}        
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//				CBCGPGanttItemStorage implementation
//////////////////////////////////////////////////////////////////////////

CBCGPGanttItemStorage::CBCGPGanttItemStorage (CWnd* pOwnerWnd)
	: CBCGPGanttItemStorageBase (pOwnerWnd)
{
}

CBCGPGanttItemStorage::~CBCGPGanttItemStorage ()
{
	RemoveAll ();
}

int CBCGPGanttItemStorage::GetCount () const
{
	return (int)m_arrItems.GetSize ();
}

POSITION CBCGPGanttItemStorage::GetHeadPosition () const
{
	return (POSITION)(INT_PTR)(m_arrItems.GetSize () == 0 ? 0 : 1);
}

POSITION CBCGPGanttItemStorage::GetItemPosition  (const CBCGPGanttItem* pItem) const
{
	return (POSITION)(INT_PTR)(IndexOf (pItem) + 1);
}

CBCGPGanttItem* CBCGPGanttItemStorage::GetNext (POSITION& rPosition) const
{
	INT_PTR index = (INT_PTR)rPosition;
	INT_PTR nItems = m_arrItems.GetSize ();
	if (index <= 0 || index > nItems)
	{
		rPosition = NULL;
		return NULL;
	}
	rPosition = (POSITION)(index == nItems ? 0 : index + 1);
	return m_arrItems[index - 1];
}

CBCGPGanttItem* CBCGPGanttItemStorage::GetPrev (POSITION& rPosition) const
{
	INT_PTR index = (INT_PTR)rPosition;
	INT_PTR nItems = m_arrItems.GetSize ();
	if (index <= 0 || index > nItems)
	{
		rPosition = NULL;
		return NULL;
	}
	rPosition = (POSITION)(index - 1);
	return m_arrItems[index - 1];
}

void CBCGPGanttItemStorage::DoAdd (CBCGPGanttItem* pItem)
{
	m_arrItems.Add (pItem);
}

void CBCGPGanttItemStorage::DoInsert (int pos, CBCGPGanttItem* pItem)
{
	m_arrItems.InsertAt (pos, pItem, 1);
}

void CBCGPGanttItemStorage::DoSwap (int pos1, int pos2)
{
	if (pos1 == pos2)
	{
		return;
	}

	CBCGPGanttItem* p = m_arrItems[pos1];
	m_arrItems[pos1] = m_arrItems[pos2];
	m_arrItems[pos2] = p;
}

void CBCGPGanttItemStorage::DoRemove (CBCGPGanttItem* pItem)
{
	int index = IndexOf (pItem);
	if (index >= 0)
	{
		m_arrItems.RemoveAt (index, 1);
	}
}

void CBCGPGanttItemStorage::DoRemoveAt (int pos)
{
	m_arrItems.RemoveAt (pos, 1);
}

void CBCGPGanttItemStorage::DoRemoveAll ()
{
	m_arrItems.RemoveAll ();
	m_arrLinks.RemoveAll ();
}

int CBCGPGanttItemStorage::IndexOf (const CBCGPGanttItem* pItem) const
{
	if (pItem != NULL)
	{
		int n = GetCount ();
		for (int i = 0; i < n; ++i)
		{
			if (m_arrItems[i] == pItem)
			{
				return i;
			}
		}
	}
	return -1;
}

bool CBCGPGanttItemStorage::Contains (const CBCGPGanttItem* pItem) const
{
	return IndexOf (pItem) >= 0;
}

CBCGPGanttItem* CBCGPGanttItemStorage::GetItem (int pos) const
{
	return m_arrItems[pos];
}

void CBCGPGanttItemStorage::GetItemReferrers (const CBCGPGanttItem* pItem, CConnectionArray& arrRefs)
{
	arrRefs.RemoveAll ();
	int n = (int)m_arrLinks.GetSize ();
	for (int i = 0; i < n; ++i)
	{
		CBCGPGanttConnection& link = m_arrLinks[i];
		if (link.m_pDestItem == pItem)
		{
			arrRefs.Add (&link);
		}
	}
}

void CBCGPGanttItemStorage::GetItemLinks (const CBCGPGanttItem* pItem, CConnectionArray& arrLinks)
{
	arrLinks.RemoveAll ();
	int n = (int)m_arrLinks.GetSize ();
	for (int i = 0; i < n; ++i)
	{
		CBCGPGanttConnection& link = m_arrLinks[i];
		if (link.m_pSourceItem == pItem)
		{
			arrLinks.Add (&link);
		}
	}
}

void CBCGPGanttItemStorage::GetAllConnections (CConnectionArray& arrLinks)
{
	int n = (int)m_arrLinks.GetSize ();
	for (int i = 0; i < n; ++i)
	{
		arrLinks.Add (&m_arrLinks[i]);
	}
}

int CBCGPGanttItemStorage::RemoveItemConnections (const CBCGPGanttItem* pItem)
{
	if (pItem == NULL)
	{
		return 0;
	}

	int nRemoved = 0;
	int n = (int)m_arrLinks.GetSize ();
	for (int i = 0; i < n; ++i)
	{
		if (m_arrLinks[i].m_pSourceItem == pItem || m_arrLinks[i].m_pDestItem == pItem)
		{
			m_arrLinks.RemoveAt (i, 1);
			i--;
			n--;
			nRemoved ++;
		}
	}

	return nRemoved;
}


CBCGPGanttConnection* CBCGPGanttItemStorage::AddConnection (CBCGPGanttItem* pSourceItem, CBCGPGanttItem* pDestItem, int linkType)
{
	ASSERT (pDestItem != pSourceItem);
	ASSERT (pDestItem != NULL);
	ASSERT (pSourceItem != NULL);

	CBCGPGanttConnection* pExisting = FindConnection (pSourceItem, pDestItem);
	if (pExisting != NULL)
	{
		return NULL;
	}

	pExisting = FindConnection (pDestItem, pSourceItem);
	if (pExisting != NULL)
	{
		return NULL;
	}

	CBCGPGanttConnection link;
	link.m_pSourceItem = pSourceItem;
	link.m_pDestItem = pDestItem;
	link.m_LinkType = linkType;
	int i = (int)m_arrLinks.Add (link);

	if (m_pOwnerWnd->GetSafeHwnd () != NULL)
	{
		m_pOwnerWnd->SendNotifyMessage (BCGM_GANTT_CONNECTION_ADDED, 0, (LPARAM)&link);
	}

	return &m_arrLinks[i];
}

CBCGPGanttConnection* CBCGPGanttItemStorage::FindConnection (const CBCGPGanttItem* pSourceItem, const CBCGPGanttItem* pDestItem)
{
	int n = (int)m_arrLinks.GetSize ();
	for (int i = 0; i < n; ++i)
	{
		if (m_arrLinks[i].m_pSourceItem == pSourceItem && m_arrLinks[i].m_pDestItem == pDestItem)
		{
			return &m_arrLinks[i];
		}
	}
	return NULL; // not found
}

BOOL CBCGPGanttItemStorage::RemoveConnection (CBCGPGanttItem* pSourceItem, CBCGPGanttItem* pDestItem)
{
	int n = (int)m_arrLinks.GetSize ();
	for (int i = 0; i < n; ++i)
	{
		if (m_arrLinks[i].m_pSourceItem == pSourceItem && m_arrLinks[i].m_pDestItem == pDestItem)
		{
			if (m_pOwnerWnd->GetSafeHwnd () != NULL)
			{
				m_pOwnerWnd->SendNotifyMessage (BCGM_GANTT_CONNECTION_ADDED, 0, (LPARAM)&m_arrLinks[i]);
			}

			m_arrLinks.RemoveAt (i, 1);
			return TRUE; // link removed successfully
		}
	}
	return FALSE; // link not found
}

#endif // #!defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)