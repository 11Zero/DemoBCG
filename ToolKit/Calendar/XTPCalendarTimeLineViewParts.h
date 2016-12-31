// XTPCalendarTimeLineViewParts.h: interface for the CXTPCalendarTimeLineView internal classes.
//
// This file is a part of the XTREME CALENDAR MFC class library.
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

//{{AFX_CODEJOCK_PRIVATE
#ifndef __XTPCALENDAR_TIMELINEVIEWPARTS_H__
#define __XTPCALENDAR_TIMELINEVIEWPARTS_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common/XTPDrawHelpers.h"
#include "XTPCalendarViewPart.h"

class CXTPCalendarTimeLineView;
class CXTPCalendarTimeLineViewEvent;

//===========================================================================
// Summary:
//     Helper base class  to implement parts for calendar paint manager.
//     objects.
//===========================================================================
class _XTP_EXT_CLASS CXTPCalendarTimeLineViewTimeScalePart : public CXTPCalendarViewPart
{
public:
	// -----------------
	// Summary:
	//     Default object constructor.
	// Parameters:
	//     pParentPart :  pointer to CXTPCalendarViewPart
	// --------------------------------
	CXTPCalendarTimeLineViewTimeScalePart(CXTPCalendarViewPart* pParentPart = NULL);

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function is used to set graphical related
	//     parameters equal to the system settings.
	//-----------------------------------------------------------------------
	virtual void RefreshMetrics();

	// ------------------------
	// Summary:
	//     This member function is used to calculate height utilizing
	//     the specified device context.
	// Parameters:
	//     pDC - Pointer to a valid device context.
	// Returns:
	//     calculated height as int
	// ------------------------
	virtual int CalcHeigt(CDC* pDC);

	// ---------------------------
	// Summary:
	//     This member function is used to draw the part content utilizing
	//     the specified device context.
	// Parameters:
	//     pDC - Pointer to a valid device context.
	//     rcRect :  rectangle to use
	//     pView :   pointer to CXTPCalendarTimeLineView
	// ---------------------------
	virtual void Draw(CDC* pDC, const CRect& rcRect, CXTPCalendarTimeLineView* pView);

	// -----------------
	// Summary:
	//     access function to get header height
	// Returns:
	//     current header height as int
	// -----------------
	virtual int GetHeaderHeight()
	{
		return m_nHeaderHeight;
	}

	// -----------------------------------
	// Summary:
	//     access function to get header date format
	// Parameters:
	//     nLabelInterval :  int param used to select format
	// Returns:
	//     current header date format as CString object
	// -----------------------------------
	virtual CString GetHeaderDateFormat(int nLabelInterval);

	// -----------------------------------
	// Summary:
	//     This member function is used to draw the part content utilizing
	//     the specified device context.
	// Parameters:
	//     pDC - Pointer to a valid device context.
	//     rcRect :  rectangle to use
	//     pView :   pointer to CXTPCalendarTimeLineView
	//     nLabelInterval :  param used to select format
	// -----------------------------------
	virtual void DrawHeader(CDC* pDC, const CRect& rcRect, CXTPCalendarTimeLineView* pView, int nLabelInterval);

public:
	CXTPCalendarViewPartFontValue m_fntScaleHeaderText; // Time scale header text font.

protected:
	int m_nHeaderHeight; // internal value of current header height
};

//===========================================================================
// Summary:
//     Helper base class  to implement parts for calendar paint manager.
//     objects.
//===========================================================================
class _XTP_EXT_CLASS CXTPCalendarTimeLineViewPart : public CXTPCalendarViewPart
{
public:
	// -----------------
	// Summary:
	//     Default object constructor.
	// Parameters:
	//     pParentPart :  pointer to CXTPCalendarViewPart
	// --------------------------------
	CXTPCalendarTimeLineViewPart(CXTPCalendarViewPart* pParentPart = NULL) : CXTPCalendarViewPart(pParentPart)
	{
		UNREFERENCED_PARAMETER(pParentPart);
	};

	// ---------------------------
	// Summary:
	//     This member function is used to draw the part content utilizing
	//     the specified device context.
	// Parameters:
	//     pDC - Pointer to a valid device context.
	//     rcRect :  rectangle to use
	//     pView :   pointer to CXTPCalendarTimeLineView
	// ---------------------------
	virtual void Draw(CDC* pDC, const CRect& rcRect, CXTPCalendarTimeLineView* pView)
	{
		UNREFERENCED_PARAMETER(pDC);
		UNREFERENCED_PARAMETER(rcRect);
		UNREFERENCED_PARAMETER(pView);
	};

	// -----------------------------------
	// Summary:
	//     This member function is used to calculate rectangle needed to draw event
	// Parameters:
	//     pDC : Pointer to a valid device context.
	//     pEventView :  pointer to CXTPCalendarTimeLineViewEvent
	// Returns:
	//     CSize object needed to draw event
	// -----------------------------------
	virtual CSize CalcEventSize(CDC* pDC, CXTPCalendarTimeLineViewEvent* pEventView)
	{
		UNREFERENCED_PARAMETER(pDC);
		UNREFERENCED_PARAMETER(pEventView);
		return CSize(0, 0);
	};

	// ---------------------------
	// Summary:
	//     This member function is used to draw the event utilizing
	//     the specified device context.
	// Parameters:
	//     pDC - Pointer to a valid device context.
	//     rcEvents :  rectangle to use
	//     pEventView : pointer to CXTPCalendarTimeLineViewEvent
	// -----------------------------------
	virtual void DrawEvent(CDC* pDC, const CRect& rcEvents, CXTPCalendarTimeLineViewEvent* pEventView)
	{
		UNREFERENCED_PARAMETER(pDC);
		UNREFERENCED_PARAMETER(rcEvents);
		UNREFERENCED_PARAMETER(pEventView);
	};
};

//===========================================================================
/*
class CXTPCalendarTLV_GroupFilter : public CXTPCmdTarget
{
public:
	CXTPCalendarTLV_GroupFilter(CXTPCalendarTLV_GroupFilter* pParent);
	virtual ~CXTPCalendarTLV_GroupFilter();

	virtual void RemoveAllChildren();

	virtual CString GetFilterTitle();

	virtual CXTPCalendarEventsPtr FilterEvents(CXTPCalendarEvents* pSource);

	virtual int GetChildrenCount() const;
	virtual CXTPCalendarTLV_GroupFilter* GetChild(int nIndex);
protected:
	typedef CArray<CXTPCalendarTLV_GroupFilter*, CXTPCalendarTLV_GroupFilter*>
		CXTPCalendarTLV_GroupFilters;

	CXTPCalendarTLV_GroupFilterPtrArray m_arChildren;
};

//============================================================================
class CXTPCalendarTLV_GrObject : public CXTPCmdTarget

{
	DECLARE_DYNAMIC(CXTPCalendarTLV_GrObject)
public:
	CXTPCalendarTLV_GrObject(CXTPCalendarTimeLineView* pView, CXTPCalendarTLV_GrObject* pParent = NULL);
	virtual ~CXTPCalendarTLV_GrObject();

	virtual void AdjustLayout(CRect rcParentRect);

	virtual CRect GetClientRect(int nYstart = 0);

	virtual int GetChildren(CXTPCalendarTLV_GrObjectPtrArray& rarChildren) = 0;
	virtual void RemoveAllChildren() = 0;

	virtual CXTPCalendarEventsPtr GetEvents();
	virtual void SetEvents( CXTPCalendarEvents* pSource,
							CXTPCalendarTLV_GroupFilter* pFilter);


protected:
	CXTPCalendarTLV_GrObject*   m_ptrParent;
	CXTPCalendarEvents          m_arEvents;
	CXTPCalendarTLV_GroupFilter*    m_ptrFilter;

	CXTPEmptyRect m_rcClientRect;
};

//============================================================================
class CXTPCalendarTLV_GroupObject : public CXTPCalendarTLV_GrObject

{
	DECLARE_DYNAMIC(CXTPCalendarTLV_GroupObject)
public:
	CXTPCalendarTLV_GroupObject(CXTPCalendarTLV_GrObject* pParent);
	virtual ~CXTPCalendarTLV_GroupObject();

	virtual void AdjustLayout(CRect rcParentRect);

	virtual CRect GetClientRect(int nYstart = 0);

	virtual int GetChildren(CXTPCalendarTLV_GrObjectPtrArray& rarChildren) = 0;
	virtual void RemoveAllChildren() = 0;

	virtual CXTPCalendarEventsPtr GetEvents();
	virtual void SetEvents( CXTPCalendarEvents* pSource,
		CXTPCalendarTLV_GroupFilter* pFilter);


protected:
};

//============================================================================
class CXTPCalendarTLV_EventsViewObject : public CXTPCalendarTLV_GrObject

{
	DECLARE_DYNAMIC(CXTPCalendarTLV_EventsViewObject)
public:
	CXTPCalendarTLV_EventsViewObject(CXTPCalendarTLV_GrObject* pParent);
	virtual ~CXTPCalendarTLV_EventsViewObject();

	virtual void AdjustLayout(CRect rcParentRect);

	virtual CRect GetClientRect(int nYstart = 0);

	virtual int GetChildren(CXTPCalendarTLV_GrObjectPtrArray& rarChildren);
	virtual void RemoveAllChildren();

	virtual CXTPCalendarEventsPtr GetEvents();
	virtual void SetEvents(CXTPCalendarEvents* pSource,
		CXTPCalendarTLV_GroupFilter* pFilter);


protected:
};
*/
//============================================================================
#endif // (__XTPCALENDAR_TIMELINEVIEWPARTS_H__)
