// XTPFlowGraphConnectionPoint.h: interface for the CXTPFlowGraphConnectionPoint class.
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

//{{AFX_CODEJOCK_PRIVATE
#if !defined(__XTPFLOWGRAPHCONNECTIONPOINT_H__)
#define __XTPFLOWGRAPHCONNECTIONPOINT_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XTPFlowGraphElement.h"

class CXTPFlowGraphNode;
class CXTPFlowGraphImage;
class CXTPMarkupUIElement;

// ------------------------------------------------------------------------------
// Summary:
//     Node Connection Point types (input\\output).
// Remarks:
//     The xtpFlowGraphConnectionPointType is used to specify whether a
//     CXTPFlowGraphConnection will have any connection points.
//     Connection Points are used to make connections between a
//     connection in different nodes.
// See Also:
//     CXTPFlowGraphConnectionPoint::SetType@XTPFlowGraphConnectionPointType
//
// ------------------------------------------------------------------------------
enum XTPFlowGraphConnectionPointType
{
	xtpFlowGraphPointNone = 0, // Connection has no connection points.
	xtpFlowGraphPointInput = 1, // Connection only has an input point.
	xtpFlowGraphPointOutput = 2, // Connection only has an output point.
	xtpFlowGraphPointInputAndOutput= 3 // Connection has both input and output points.
};

// ------------------------------------------------------------------
//
// Summary:
//     Represents a connection point for a node.
// Remarks:
//     Connection points are added to a Node and can no input points,
//     \only input or output points, or can have both.
// ------------------------------------------------------------------
class _XTP_EXT_CLASS CXTPFlowGraphConnectionPoint : public CXTPFlowGraphElement
{
	DECLARE_SERIAL(CXTPFlowGraphConnectionPoint)

public:
	// -----------------------------------------------------
	// Summary:
	//     Constructs a CXTPFlowGraphCOnnectionPoint object.
	// -----------------------------------------------------
	CXTPFlowGraphConnectionPoint();
	// -------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPFlowGraphConnectionPoint object, handles cleanup
	//     and deallocation.
	// -------------------------------------------------------------------
	virtual ~CXTPFlowGraphConnectionPoint();

public:
	// --------------------------------------------------------------------
	// Summary:
	//     Sets the caption of the connection point.
	// Parameters:
	//     lpszCaption :  String caption of the connection point.
	// Remarks:
	//     You can use FindConnectionPoint to search for a connection point
	//     by caption.
	// --------------------------------------------------------------------
	void SetCaption(LPCTSTR lpszCaption);
	// --------------------------------------------------------------------
	// Summary:
	//     Gets the caption of the connection point.
	// Remarks:
	//     You can use FindConnectionPoint to search for a connection point
	//     by caption.
	// Returns:
	//     \Returns the caption of the connection point.
	// --------------------------------------------------------------------
	CString GetCaption() const;

	// ---------------------------------------------------------------------
	// Summary:
	//     Sets the tooltip for this connection point.
	// Parameters:
	//     lpszTooltip :  Tooltip for this connection point.
	//
	// Remarks:
	//     This is the text that is displayed when the mouse is hovered over
	//     the connection point.
	// ---------------------------------------------------------------------
	void SetTooltip(LPCTSTR lpszTooltip);
	// ---------------------------------------------------------------------
	// Summary:
	//     Gets the tooltip for this connection point.
	// Remarks:
	//     This is the text that is displayed when the mouse is hovered over
	//     the connection point.
	// Returns:
	//     The tooltip for this connection point.
	// ---------------------------------------------------------------------
	CString GetTooltip() const;

	// ----------------------------------------------
	// Summary:
	//     Sets the color of the connection point.
	// Parameters:
	//     clr :  Color to make the connection point.
	// ----------------------------------------------
	void SetColor(COLORREF clr);
	// -----------------------------------------------
	// Summary:
	//     Gets the color of the connection point.
	// Returns:
	//     \Returns the color of the connection point.
	// -----------------------------------------------
	COLORREF GetColor() const;

	// ----------------------------------------------------------------------
	// Summary:
	//     Reserved for future use. Currently can be used as a "tag" property
	//     if needed.
	// ----------------------------------------------------------------------
	void SetStyle(int nStyle);
	// ----------------------------------------------------------------------
	// Summary:
	//     Reserved for future use. Currently can be used as a "tag" property
	//     if needed.
	// ----------------------------------------------------------------------
	int GetStyle() const;

	// -------------------------------------------------------------------
	// Summary:
	//     Gets whether the FlowGraphConnection will have any connection
	//     points (input\\output).
	// Remarks:
	//     An option from FlowGraphConnectionPointType that specifies the
	//     included connection points, if any.
	// Returns:
	//     The ConnectionPoint Type is used to specify whether a
	//     FlowGraphConnection will have any connection points. Connection
	//     Points are used to make connections between a connection in
	//     different nodes.
	// -------------------------------------------------------------------
	XTPFlowGraphConnectionPointType GetType() const;
	// ----------------------------------------------------------------------
	// Summary:
	//     Specifies whether the FlowGraphConnection will have any connection
	//     points (input\\output).
	// Parameters:
	//     type :  An option from FlowGraphConnectionPointType that specifies
	//             the included connection points.
	//
	// Remarks:
	//     The ConnectionPoint Type is used to specify whether a
	//     FlowGraphConnection will have any connection points. Connection
	//     Points are used to make connections between a connection in
	//     different nodes.
	// ----------------------------------------------------------------------
	void SetType(XTPFlowGraphConnectionPointType type);

	// -------------------------------------------------
	// Summary:
	//     Gets the bounding rectangle for this element.
	// Returns:
	//     The bounding rectangle for this element.
	// -------------------------------------------------
	CRect GetRect() const;

	// -------------------------------------------------------
	// Summary:
	//     Sets the ID used to identify this connection point.
	// Parameters:
	//     nId :  New id for this connection point.
	// -------------------------------------------------------
	void SetID(int nId);
	// -------------------------------------------------------
	// Summary:
	//     Gets the ID used to identify this connection point.
	// Returns:
	//     The ID used to identify this connection point.
	//
	//
	// -------------------------------------------------------
	int GetID() const;

	// ----------------------------------------------------------------
	//
	// Summary:
	//     Gets whether the connection's points are locked.
	// Returns:
	//     True if the connections points are locked, False if they can
	//     accept connections.
	// ----------------------------------------------------------------
	BOOL IsLocked() const;
	// ---------------------------------------------------------------------
	//
	// Summary:
	//     Specifies whether the connection's points are locked.
	// Parameters:
	//     bLocked :  True if the connections points are to be locked, False
	//                if they can accept connections.
	//
	// Remarks:
	//     If True, the connection's points can't accept a connection line
	//     and any connection lines that were added can't be removed.
	// ---------------------------------------------------------------------
	void SetLocked(BOOL bLocked = TRUE);

	// ----------------------------------------------------------------------
	// Summary:
	//     Gets the maximum number of connections (connection lines) that the
	//     connection point can accept.
	// Returns:
	//     The maximum number of connections (connection lines) that the
	//     connection point can accept.
	// ----------------------------------------------------------------------
	int GetMaxConnections() const;
	// -----------------------------------------------------------------------
	// Summary:
	//     Specifies the maximum number of connections (connection lines)
	//     that the connection point can accept.
	// Parameters:
	//     nMaxConnections :  Maximum number of connections (connection lines)
	//                        that the connection point can accept.
	// -----------------------------------------------------------------------
	void SetMaxConnections(int nMaxConnections);

	// --------------------------------------------------------------------
	// Summary:
	//     Gets the number of connection lines connected to this connection
	//     point.
	// Returns:
	//     \Returns the number of connection lines connected to this
	//     connection point.
	// --------------------------------------------------------------------
	int GetConnectionsCount() const;

	// ----------------------------------------------------------------------
	// Summary:
	//     Sets the ID of the Image to display to the left of the connection
	//     point's caption.
	// Parameters:
	//     nImageIndex :  The ID of the Image that will be used to display to
	//                    the left of the connection point's caption.
	//
	// Remarks:
	//     Use the Image Index to specify the index of the previously loaded
	//     image to display next to the connections caption.
	// ----------------------------------------------------------------------
	void SetImageIndex(int nImageIndex);
	// ---------------------------------------------------------------------
	// Summary:
	//     Gets the ID of the Image to display to the left of the connection
	//     point's caption.
	// Returns:
	//     The ID of the Image to display to the left of the connection
	//     point's caption.
	// ---------------------------------------------------------------------
	int GetImageIndex() const;

public:
	// ----------------------------------------------------------------------
	// Summary:
	//     Call this method to get Markup element that renders caption of the
	//     control.
	// Returns:
	//     Pointer to CXTPMarkupUIElement element.
	// ----------------------------------------------------------------------
	CXTPMarkupUIElement* GetMarkupUIElement() const;

public:
	// -------------------------------------------------------------------
	// Summary:
	//     Gets a reference to the node that this connection point belongs
	//     to.
	// Returns:
	//     A reference to the node that this connection point belongs to.
	// -------------------------------------------------------------------
	CXTPFlowGraphNode* GetNode() const;

	// -------------------------------------------------------------------
	// Summary:
	//     Gets a reference to the page that this connection point belongs
	//     to.
	// Returns:
	//     A reference to the page that this connection point belongs to.
	// -------------------------------------------------------------------
	CXTPFlowGraphPage* GetPage() const;

	// -----------------------------------------------------
	// Summary:
	//     Gets a reference to the parent FlowGraph control.
	// Returns:
	//     A reference to the parent FlowGraph control.
	// -----------------------------------------------------
	CXTPFlowGraphControl* GetControl() const;

	// ---------------------------------------------------------------------
	// Summary:
	//     Gets the ID of the Image to display to the left of the connection
	//     point's caption.
	// Returns:
	//     The ID of the Image to display to the left of the connection
	//     point's caption.
	// ---------------------------------------------------------------------
	CXTPFlowGraphImage* GetImage() const;

	// -----------------------------------------------------------------
	// Summary:
	//     This method is called when the flow graph layout has changed.
	// Remarks:
	//     This is called when the page zoom level, page scroll offset,
	//     window\\node size, window handle, node style, node color, or
	//     connection style is changed.
	// -----------------------------------------------------------------
	void OnGraphChanged();

public:
	// ------------------------------------------------------------------------
	// Summary:
	//     Call this member function to Store/Load a connection point using
	//     the specified data object.
	// Parameters:
	//     pPX :  Source or destination CXTPPropExchange data object reference.
	// Remarks:
	//     This member function is used to store or load connection point
	//     data to or from a storage.
	// ------------------------------------------------------------------------
	virtual void DoPropExchange(CXTPPropExchange* pPX);

public:
	// --------------------------------------------------------------------
	// Summary:
	//     Determines if the specified point is within the connection area.
	// Parameters:
	//     pt :  Point to test.
	// Returns:
	//     True if the point is located within the connection area, False
	//     \otherwise.
	// --------------------------------------------------------------------
	virtual BOOL HitTestConnectionArea(CPoint point) const;

protected:
	CXTPFlowGraphNode* m_pNode; // Reference to the node that this connection point belongs to.

	CString m_strCaption;  // Caption of the connection point.
	CString m_strTooltip; // Tooltip text for the item.
	COLORREF m_clrPoint; // Color of the connection point.
	XTPFlowGraphConnectionPointType m_nType;  // Type of connection point, Specifies whether the FlowGraphConnection will have any connection points (input\\output).

	int m_nStyle; // Reserved for future use. Currently can be used as a "tag" property if needed.
	int m_nId;  // ID of the connection point.
	BOOL m_bLocked;  // Specifies if the connection point is locked.  True if the connection point is locked, False if it can accept connections.

	int m_nMaxConnections; // The maximum number of connections (connection lines) that the connection point can accept.
	int m_nConnectionsCount; // The number of connection lines connected to this connection point.

	int m_nImageIndex; // ID of the Image to display to the left of the connection point's caption.
	CXTPMarkupUIElement* m_pMarkupUIElement; // Pointer to CXTPMarkupUIElement element.

	friend class CXTPFlowGraphConnectionPoints;
	friend class CXTPFlowGraphPaintManager;
	friend class CXTPFlowGraphConnection;
	friend class CXTPFlowGraphPage;

//{{AFX_CODEJOCK_PRIVATE
// Paint Manager Properties:
public:
	CRect m_rcPoint;
//}}AFX_CODEJOCK_PRIVATE


};


AFX_INLINE CString CXTPFlowGraphConnectionPoint::GetCaption() const {
	return m_strCaption;
}
AFX_INLINE void CXTPFlowGraphConnectionPoint::SetTooltip(LPCTSTR lpszTooltip) {
	m_strTooltip = lpszTooltip;
}
AFX_INLINE CString CXTPFlowGraphConnectionPoint::GetTooltip() const {
	return m_strTooltip;
}
AFX_INLINE void CXTPFlowGraphConnectionPoint::SetColor(COLORREF clr) {
	m_clrPoint = clr;
}
AFX_INLINE COLORREF CXTPFlowGraphConnectionPoint::GetColor() const {
	return m_clrPoint;
}
AFX_INLINE void CXTPFlowGraphConnectionPoint::SetStyle(int nStyle) {
	m_nStyle = nStyle;
}
AFX_INLINE int CXTPFlowGraphConnectionPoint::GetStyle() const {
	return m_nStyle;
}
AFX_INLINE CXTPFlowGraphNode* CXTPFlowGraphConnectionPoint::GetNode() const {
	return m_pNode;
}
AFX_INLINE XTPFlowGraphConnectionPointType CXTPFlowGraphConnectionPoint::GetType() const {
	return m_nType;
}
AFX_INLINE void CXTPFlowGraphConnectionPoint::SetType(XTPFlowGraphConnectionPointType type) {
	m_nType = type;
}
AFX_INLINE CRect CXTPFlowGraphConnectionPoint::GetRect() const {
	return m_rcPoint;
}
AFX_INLINE void CXTPFlowGraphConnectionPoint::SetID(int nId) {
	m_nId = nId;
}
AFX_INLINE int CXTPFlowGraphConnectionPoint::GetID() const {
	return m_nId;
}
AFX_INLINE BOOL CXTPFlowGraphConnectionPoint::IsLocked() const {
	return m_bLocked;
}
AFX_INLINE void CXTPFlowGraphConnectionPoint::SetLocked(BOOL bLocked) {
	m_bLocked = bLocked;
}
AFX_INLINE int CXTPFlowGraphConnectionPoint::GetMaxConnections() const {
	return m_nMaxConnections;
}
AFX_INLINE void CXTPFlowGraphConnectionPoint::SetMaxConnections(int nMaxConnections) {
	m_nMaxConnections = nMaxConnections;
}
AFX_INLINE int CXTPFlowGraphConnectionPoint::GetConnectionsCount() const {
	return m_nConnectionsCount;
}
AFX_INLINE void CXTPFlowGraphConnectionPoint::SetImageIndex(int nImageIndex) {
	m_nImageIndex = nImageIndex;
}
AFX_INLINE int CXTPFlowGraphConnectionPoint::GetImageIndex() const {
	return m_nImageIndex;
}
AFX_INLINE CXTPMarkupUIElement* CXTPFlowGraphConnectionPoint::GetMarkupUIElement() const {
	return m_pMarkupUIElement;
}



#endif //#if !defined(__XTPFLOWGRAPHCONNECTIONPOINT_H__)
