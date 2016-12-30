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
// BCGPGanttItem.h: interface for the CBCGPGanttItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGANTTITEM_H__2BFAAE00_9488_4A9D_B7C7_1EE7A2B5EF1E__INCLUDED_)
#define AFX_BCGPGANTTITEM_H__2BFAAE00_9488_4A9D_B7C7_1EE7A2B5EF1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBCGPGanttChart;
class CBCGPGanttItemStorageBase;
class CBCGPGanttItem;

#include "BCGCBPro.h"

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

class BCGCBPRODLLEXPORT CBCGPGanttItem : public CObject
{
    friend class CBCGPGanttChart;
    friend class CBCGPGanttItemStorageBase;

    DECLARE_SERIAL (CBCGPGanttItem)

public:
    CBCGPGanttItem();
    virtual ~CBCGPGanttItem();

    // Attributes
public:
    void	SetInterval (COleDateTime dtStart, COleDateTime dtFinish);

    COleDateTime   GetStartTime() const
    {
        return m_dtStart;
    }
    COleDateTime   GetFinishTime() const
    {
        return m_dtFinish;
    }

    BOOL IsMileStone () const
    {
        return m_dtStart == m_dtFinish;
    }

    void	SetName(const CString& strName);
    CString GetName() const
    {
        return m_strName;
    }

    void	SetProgress(double dPercents);
    double	GetProgress() const
    {
        return m_dProgress;
    }

    void    SetCompleted();
    BOOL    IsCompleted() const
    {
        return m_dProgress >= 1.0;
    }

    void	SetGroupItem(BOOL bGroup = TRUE);
    BOOL	IsGroupItem() const
    {
        return m_bGroupItem;
    }

    void    SetHierarchyLevel (DWORD dwLevel);
    DWORD   GetHierarchyLevel () const
    {
        return m_dwHierarchyLevel;
    }

    void	Show(BOOL bShow = TRUE);
    BOOL	IsVisible() const
    {
        return m_bVisible;
    }

    void    SetPriority(UINT nPriority);
    DWORD   GetPriority() const
    {
        return m_nPriority;
    }

    void    SetData(DWORD dwData);
    DWORD   GetData() const
    {
        return m_dwData;
    }

    void	Enable(BOOL bEnable = TRUE);
    BOOL	IsEnabled() const
    {
        return m_bEnabled;
    }

    void	Select(BOOL bSelect = TRUE);
    BOOL	IsSelected() const
    {
        return m_bSelected;
    }

    void	SetPrimaryColor(COLORREF clr);
    COLORREF GetPrimaryColor() const
    {
        return m_clrPrimaryColor;
    }

    void	SetCompleteColor(COLORREF clr);
    COLORREF GetCompleteColor() const
    {
        return m_clrCompleteColor;
    }

    // Overridables
public:
    virtual void Redraw();
    virtual void Serialize (CArchive& ar);

    void Update (DWORD dwFlags);

    // Data members
protected: 
    CBCGPGanttItemStorageBase*	m_pStorage; // Storage that owns this item

    BOOL			m_bVisible;
    BOOL			m_bEnabled;
    BOOL			m_bSelected;
    BOOL			m_bChecked;
    BOOL			m_bGroupItem;

    CString 		m_strName;
    CString 		m_strUserTooltip;
    COleDateTime	m_dtStart;
    COleDateTime	m_dtFinish;
    double			m_dProgress; // 0.0 .. 1.0

    COLORREF		m_clrPrimaryColor;
    COLORREF		m_clrCompleteColor;

    UINT            m_nPriority;
    DWORD           m_dwHierarchyLevel;
    DWORD           m_dwData;

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};

#define  BCGP_GANTT_ITEM_PROP_NAME              0x0001
#define  BCGP_GANTT_ITEM_PROP_START             0x0002
#define  BCGP_GANTT_ITEM_PROP_FINISH            0x0004
#define  BCGP_GANTT_ITEM_PROP_PROGRESS          0x0008
#define  BCGP_GANTT_ITEM_PROP_PRIORITY          0x0010
#define  BCGP_GANTT_ITEM_PROP_SELECTED          0x0020
#define  BCGP_GANTT_ITEM_PROP_GROUPITEM         0x0040
#define  BCGP_GANTT_ITEM_PROP_HIERARCHYLEVEL    0x0080
#define  BCGP_GANTT_ITEM_PROP_COLORS            0x0100
#define  BCGP_GANTT_ITEM_PROP_VISIBLE           0x0200
#define  BCGP_GANTT_ITEM_PROP_ENABLED           0x0400
#define  BCGP_GANTT_ITEM_PROP_CHECKED           0x0800
#define  BCGP_GANTT_ITEM_PROP_TOOLTIP           0x1000
#define  BCGP_GANTT_ITEM_PROP_DATA              0x2000

#define  BCGP_GANTT_ITEM_PROP_NONE              0
#define  BCGP_GANTT_ITEM_PROP_ALL               0xFFFFFFFF


struct BCGCBPRODLLEXPORT CBCGPGanttConnection
{
    CBCGPGanttItem* 			m_pSourceItem;
    CBCGPGanttItem* 			m_pDestItem;
    CList<CPoint,const CPoint&>	m_Points;

    int                         m_LinkType;

    CBCGPGanttConnection ();
    CBCGPGanttConnection& operator = (const CBCGPGanttConnection& link);

    void Reset ();
};

#define 	BCGPGANTTLINK_START_TO_START	1
#define 	BCGPGANTTLINK_START_TO_FINISH	2
#define 	BCGPGANTTLINK_FINISH_TO_START	3
#define 	BCGPGANTTLINK_FINISH_TO_FINISH	4


#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

#endif // !defined(AFX_BCGPGANTTITEM_H__2BFAAE00_9488_4A9D_B7C7_1EE7A2B5EF1E__INCLUDED_)
