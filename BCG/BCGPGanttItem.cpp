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
// BCGPGanttItem.cpp: implementation of the CBCGPGanttItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPGanttItem.h"
#include "BCGPGanttItemStorage.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

IMPLEMENT_SERIAL(CBCGPGanttItem, CObject, 1)

CBCGPGanttItem::CBCGPGanttItem()
    : m_pStorage (NULL)
    , m_bVisible (TRUE)
    , m_bEnabled (TRUE)
    , m_bChecked (FALSE)
    , m_bSelected (FALSE)
    , m_dProgress (0.0f)
    , m_nPriority (0)
    , m_bGroupItem (FALSE)
    , m_clrPrimaryColor (CLR_DEFAULT)
    , m_clrCompleteColor (CLR_DEFAULT)
    , m_dwHierarchyLevel (0)
    , m_dwData (0)
{
	m_dtStart = COleDateTime::GetCurrentTime ();
	m_dtFinish = m_dtStart + COleDateTimeSpan  (1, 0, 0, 0);
}

CBCGPGanttItem::~CBCGPGanttItem ()
{
}

void CBCGPGanttItem::Serialize (CArchive& ar)
{
	if (ar.IsLoading ())
	{
		ar >> m_strName;
		ar >> m_dtStart;
		ar >> m_dtFinish;
		ar >> m_dProgress;
		ar >> m_nPriority;
		ar >> m_bVisible;
		ar >> m_bEnabled;
		ar >> m_bChecked;
		ar >> m_strUserTooltip;
		ar >> m_clrPrimaryColor;
		ar >> m_clrCompleteColor;
        ar >> m_bGroupItem;
		ar >> m_dwHierarchyLevel;
		ar >> m_dwData;
	}
	else
	{
		ar << m_strName;
		ar << m_dtStart;
		ar << m_dtFinish;
		ar << m_dProgress;
		ar << m_nPriority;
		ar << m_bVisible;
		ar << m_bEnabled;
		ar << m_bChecked;
		ar << m_strUserTooltip;
		ar << m_clrPrimaryColor;
		ar << m_clrCompleteColor;
        ar << m_bGroupItem;
		ar << m_dwHierarchyLevel;
		ar << m_dwData;
	}
}

void CBCGPGanttItem::Update (DWORD dwFlags)
{
	if (m_pStorage != NULL)
	{
		ASSERT_VALID (m_pStorage);
		m_pStorage->UpdateItem (this, dwFlags);
	}
}


void CBCGPGanttItem::Redraw ()
{
	Update (BCGP_GANTT_ITEM_PROP_NONE);
}

void CBCGPGanttItem::Show (BOOL bShow)
{
	if (m_bVisible != bShow)
	{
		m_bVisible = bShow;
		Update (BCGP_GANTT_ITEM_PROP_VISIBLE);
	}
}

void CBCGPGanttItem::Enable (BOOL bEnable)
{
	if (m_bEnabled != bEnable)
	{
		m_bEnabled = bEnable;
		Update(BCGP_GANTT_ITEM_PROP_ENABLED);
	}
}

void CBCGPGanttItem::Select (BOOL bSelect)
{
	if (m_bSelected != bSelect)
	{
		m_bSelected = bSelect;
		Update(BCGP_GANTT_ITEM_PROP_SELECTED);
	}
}

void CBCGPGanttItem::SetInterval (COleDateTime dtStart, COleDateTime dtFinish)
{
	if (dtFinish < dtStart)
	{
		dtFinish = dtStart;
	}

	if (m_dtStart != dtStart || m_dtFinish != dtFinish)
	{
		m_dtStart = dtStart;
		m_dtFinish = dtFinish;
		Update (BCGP_GANTT_ITEM_PROP_START | BCGP_GANTT_ITEM_PROP_FINISH);
	}
}

void CBCGPGanttItem::SetName (const CString& strName)
{
	if (m_strName != strName)
	{
		m_strName = strName;
		Update (BCGP_GANTT_ITEM_PROP_NAME);
	}
}

void CBCGPGanttItem::SetData (DWORD dwData)
{
	if (m_dwData != dwData)
	{
		m_dwData = dwData;
		Update (BCGP_GANTT_ITEM_PROP_DATA);
	}
}

void CBCGPGanttItem::SetProgress (double dPercents)
{
	if (dPercents > 1.0) dPercents = 1.0;
	if (dPercents < 0.0) dPercents = 0.0;

	if (m_dProgress != dPercents)
	{
		m_dProgress = dPercents;
		Update (BCGP_GANTT_ITEM_PROP_PROGRESS);
	}
}

void CBCGPGanttItem::SetCompleted ()
{
	SetProgress (1.0);
}

void CBCGPGanttItem::SetPriority (UINT nPriority)
{
	if (m_nPriority != nPriority)
	{
		m_nPriority = nPriority;
		Update (BCGP_GANTT_ITEM_PROP_PRIORITY);
	}
}

void CBCGPGanttItem::SetGroupItem (BOOL bGroup)
{
	if (m_bGroupItem != bGroup)
	{
		m_bGroupItem = bGroup;
		Update (BCGP_GANTT_ITEM_PROP_GROUPITEM);
	}
}

void CBCGPGanttItem::SetHierarchyLevel (DWORD dwLevel)
{
	if (m_dwHierarchyLevel != dwLevel)
	{
		m_dwHierarchyLevel = dwLevel;
		Update (BCGP_GANTT_ITEM_PROP_HIERARCHYLEVEL);
	}
}

void CBCGPGanttItem::SetPrimaryColor (COLORREF clr)
{
	if (m_clrPrimaryColor != clr)
	{
		m_clrPrimaryColor = clr;
		Update (BCGP_GANTT_ITEM_PROP_COLORS);
	}
}

void CBCGPGanttItem::SetCompleteColor (COLORREF clr)
{
	if (m_clrCompleteColor != clr)
	{
		m_clrCompleteColor = clr;
		Update (BCGP_GANTT_ITEM_PROP_COLORS);
	}
}

#ifdef _DEBUG
void CBCGPGanttItem::AssertValid () const
{
	CObject::AssertValid ();
	ASSERT (m_dProgress >= 0.0 && m_dProgress <= 1.0);
	ASSERT (m_dtStart <= m_dtFinish);
}

void CBCGPGanttItem::Dump (CDumpContext& dc) const
{
	CObject::Dump (dc);
	dc << " \"" << m_strName << "\"";
	dc << " [" << m_dtStart.Format (_T("%x %H:%M")) << " - " << m_dtFinish.Format (_T("%x %H:%M")) << "] ";
	dc << m_dProgress * 100 << "% ";
}
#endif


CBCGPGanttConnection::CBCGPGanttConnection ()
	: m_pSourceItem (NULL)
    , m_pDestItem (NULL)
    , m_LinkType (BCGPGANTTLINK_FINISH_TO_START)
{
}

CBCGPGanttConnection& CBCGPGanttConnection::operator = (const CBCGPGanttConnection& link)
{
	m_pSourceItem = link.m_pSourceItem;
	m_pDestItem   = link.m_pDestItem;
	m_LinkType	  = link.m_LinkType;

	m_Points.RemoveAll ();
	CPoint pt;

	for (POSITION pos = link.m_Points.GetHeadPosition (); pos != NULL; )
	{
		pt = link.m_Points.GetNext (pos);
		m_Points.AddTail (pt);
	}

	return *this;
}

void CBCGPGanttConnection::Reset ()
{
	m_pDestItem = NULL;
	m_pSourceItem = NULL;
}


#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

