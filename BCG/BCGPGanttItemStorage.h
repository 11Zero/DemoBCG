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
// BCGPGanttItemStorage.h: interface for the CBCGPGanttItemStorage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGANTTITEMSTORAGE_H__42B641A5_AA09_48C5_92C3_DC258BC1C8E8__INCLUDED_)
#define AFX_BCGPGANTTITEMSTORAGE_H__42B641A5_AA09_48C5_92C3_DC258BC1C8E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

class CBCGPGanttItem;
struct CBCGPGanttConnection;
class CBCGPAppointment;

class BCGCBPRODLLEXPORT CBCGPGanttItemStorageBase : public CObject
{
    DECLARE_SERIAL (CBCGPGanttItemStorageBase)
public:
    typedef CArray <CBCGPGanttConnection*, CBCGPGanttConnection*> CConnectionArray;

    CBCGPGanttItemStorageBase ();
    CBCGPGanttItemStorageBase (CWnd* pOwnerWnd);
	virtual ~CBCGPGanttItemStorageBase ();

	virtual POSITION	GetHeadPosition  () const;
    virtual POSITION	GetItemPosition  (const CBCGPGanttItem* pItem) const;
	virtual CBCGPGanttItem* GetNext    (POSITION& rPosition) const;
    virtual CBCGPGanttItem* GetPrev    (POSITION& rPosition) const;

	virtual bool		Contains   (const CBCGPGanttItem* pItem) const;
	virtual int 		IndexOf    (const CBCGPGanttItem* pItem) const;
	virtual CBCGPGanttItem* GetItem    (int pos) const;
	virtual int 		GetCount   () const;

	virtual void		GetItemReferrers  (const CBCGPGanttItem* pItem, CConnectionArray& arrReferrers);
	virtual void		GetItemLinks      (const CBCGPGanttItem* pItem, CConnectionArray& arrLinks);
	virtual void		GetAllConnections (CConnectionArray& arrLinks);

    virtual int         RemoveItemConnections (const CBCGPGanttItem* /*pItem*/) { return 0; }

	virtual CBCGPGanttConnection* AddConnection     (CBCGPGanttItem* pSourceItem, CBCGPGanttItem* pDestItem, int linkType);
	virtual CBCGPGanttConnection* FindConnection    (const CBCGPGanttItem* pSourceItem, const CBCGPGanttItem* pDestItem);
	virtual BOOL                  RemoveConnection  (CBCGPGanttItem* pSourceItem, CBCGPGanttItem* pDestItem);

    virtual CBCGPGanttItem*       FindByData (DWORD_PTR dwData);

	// Forces owner control to update specified item
	virtual void    UpdateItem (CBCGPGanttItem* pItem, DWORD dwFlags) const;

	// Forces owner control to update all items
	virtual void    UpdateAll  (DWORD dwFlags) const;

	// Adds a new item and notifies owner control about it.
	void Add (CBCGPGanttItem* pItem);

	// Inserts a new item at the specified position and notifies owner control about it.
	void Insert (int pos, CBCGPGanttItem* pItem);

    void Move (int posOld, int posInsert);

    void Swap (int pos1, int pos2);

	// Removes specified item and notifies owner control about it.
	void Remove (CBCGPGanttItem* pItem);

	// Removes specified an item by index and notifies owner control about it.
	void RemoveAt (int pos);

	void RemoveAll ();

#ifndef BCGP_EXCLUDE_PLANNER
	BOOL InsertAppointment (int pos, const CBCGPAppointment* pAppointment);
#endif

	CBCGPGanttItem& operator [] (int index) const
	{
		return *(GetItem (index));
	}

    int GetSubItemsCount (const CBCGPGanttItem* pGroupItem, CBCGPGanttItem** ppLastSubItem = NULL) const;

    CBCGPGanttItem* GetParentGroupItem (const CBCGPGanttItem* pSubItem) const;

protected:
	friend class CBCGPGanttItem;

	// Notifies owner control that changes were made to a single item
	void NotifyUpdateSingleItem (UINT uiAction, CBCGPGanttItem* pItem, int iItemIndex, DWORD dwFlags) const;

    // Notifies owner control that changes were made to multiple items
    void NotifyUpdateItems (UINT uiAction, int iFirstItem, int iLastItem, DWORD dwFlags) const;

	// Implements adding a new item
	virtual void DoAdd		  (CBCGPGanttItem* pItem);

	// Implements inserting a new item
	virtual void DoInsert	  (int pos, CBCGPGanttItem* pItem);

    // Implements exchanging two items
    virtual void DoSwap       (int pos1, int pos2);

	// Implements removing specified item
	virtual void DoRemove	  (CBCGPGanttItem* pItem);

	// Implements removing specified item by its index
	virtual void DoRemoveAt   (int pos);

	// Implements removing of all items
	virtual void DoRemoveAll  ();

    // Implementation
protected:
    void RemoveRange (int iFirst, int iLast);

    // Overridables
public:
    virtual void Serialize (CArchive& ar);

protected:
    CWnd*   m_pOwnerWnd;
};


//////////////////////////////////////////////////////////////////////////
//          Default item storage implementation
//////////////////////////////////////////////////////////////////////////

class BCGCBPRODLLEXPORT CBCGPGanttItemStorage : public CBCGPGanttItemStorageBase
{
public:
	CBCGPGanttItemStorage ();
    CBCGPGanttItemStorage (CWnd* pOwnerWnd);
	virtual ~CBCGPGanttItemStorage();

	virtual POSITION  GetHeadPosition () const;
    virtual POSITION  GetItemPosition  (const CBCGPGanttItem* pItem) const;
	virtual CBCGPGanttItem* GetNext (POSITION& rPosition) const;
    virtual CBCGPGanttItem* GetPrev (POSITION& rPosition) const;
	virtual CBCGPGanttItem* GetItem (int pos) const;

    virtual 	 int  GetCount () const;
    virtual 	 bool Contains (const CBCGPGanttItem* pItem) const;
    virtual 	 int  IndexOf  (const CBCGPGanttItem* pItem) const;

    virtual 	 void GetItemReferrers (const CBCGPGanttItem* pItem, CConnectionArray& arrReferrers);
    virtual 	 void GetItemLinks	 (const CBCGPGanttItem* pItem, CConnectionArray& arrLinks);
    virtual 	 void GetAllConnections (CConnectionArray& arrLinks);

    virtual      int  RemoveItemConnections (const CBCGPGanttItem* pItem);

	virtual CBCGPGanttConnection* AddConnection  (CBCGPGanttItem* pSourceItem, CBCGPGanttItem* pDestItem, int linkType);
	virtual CBCGPGanttConnection* FindConnection (const CBCGPGanttItem* pSourceItem, const CBCGPGanttItem* pDestItem);
	virtual      BOOL RemoveConnection  (CBCGPGanttItem* pSourceItem, CBCGPGanttItem* pDestItem);

protected:
    virtual      void DoAdd    (CBCGPGanttItem* pItem);
    virtual      void DoInsert (int pos, CBCGPGanttItem* pItem);
    virtual      void DoSwap   (int pos1, int pos2);
    virtual      void DoRemove (CBCGPGanttItem* pItem);
    virtual      void DoRemoveAt (int pos);
    virtual      void DoRemoveAll ();

    CArray <CBCGPGanttItem*, CBCGPGanttItem*> m_arrItems; // Owns items. Suppose items are created with new operator.
	CArray <CBCGPGanttConnection, CBCGPGanttConnection&> m_arrLinks; // Stores a graph of links between items.
};

#define  BCGP_GANTT_STORAGE_INSERT_ITEM         1
#define  BCGP_GANTT_STORAGE_BEFORE_REMOVE_ITEM  2
#define  BCGP_GANTT_STORAGE_REMOVED_ITEM        3
#define  BCGP_GANTT_STORAGE_UPDATE_ITEM         4

struct BCGP_GANTT_STORAGE_UPDATE_INFO
{
    UINT            uiAction;        // one of BCGP_GANTT_STORAGE_xxx constants
    int             iFirstItemIndex;
    int             iLastItemIndex;   
    CBCGPGanttItem* pItem;           // NULL for multiple items
    DWORD           dwFlags;         // one of BCGP_GANTT_ITEM_PROP_xxx constants
    DWORD           dwReserved;
};

// Notifies owner window about changes in a storage.
//  wParam: not used
//  lParam: a pointer to BCGP_GANTT_STORAGE_UPDATE_INFO struct
//  return value: 0
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_STORAGE_CHANGED;

// Notifies owner window that a new connection between two items were added.
//  wParam: not used
//  lParam: a pointer to CBCGPGanttConnection being added
//  return value: 0
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CONNECTION_ADDED;

// Notifies owner window that specified connection between two items were removed.
//  wParam: not used
//  lParam: a pointer to CBCGPGanttConnection being removed
//  return value: 0
BCGCBPRODLLEXPORT extern const UINT BCGM_GANTT_CONNECTION_REMOVED;

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

#endif // !defined(AFX_BCGPGANTTITEMSTORAGE_H__42B641A5_AA09_48C5_92C3_DC258BC1C8E8__INCLUDED_)
