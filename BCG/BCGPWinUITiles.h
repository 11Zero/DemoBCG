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
// BCGPWinUITiles.h: interface for the CBCGPWinUITiles class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPWINUIUITILES_H__1274D27C_573D_4BC5_986D_A63658C8B32E__INCLUDED_)
#define AFX_BCGPWINUIUITILES_H__1274D27C_573D_4BC5_986D_A63658C8B32E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXMT_H__
	#include <afxmt.h>
#endif

#include "BCGPVisualContainer.h"
#include "BCGPVisualCtrl.h"

extern BCGCBPRODLLEXPORT UINT BCGM_ON_CLICK_WINUI_UI_TILE;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CLICK_WINUI_GROUP_CAPTION;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CLICK_WINUI_CAPTION_BUTTON;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CLICK_WINUI_NAV_BACK_BUTTON;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CREATE_WINUI_VIEW;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_CLOSE_WINUI_VIEW;

class CBCGPWinUITilesGroupCaption;
class CBCGPWinUITilesCaptionButton;

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUIBaseObject

class BCGCBPRODLLEXPORT CBCGPWinUIBaseObject : public CObject
{
	DECLARE_DYNAMIC(CBCGPWinUIBaseObject)

	friend class CBCGPWinUITiles;

// Construction
protected:
	CBCGPWinUIBaseObject();
	
// Attributes
public:
	const CString& GetName() const { return m_strName; }
	void SetName(const CString& strName);
	
	DWORD_PTR GetUserData() const { return m_dwUserData; }
	void SetUserData(DWORD_PTR dwUserData);

	BOOL IsVisible() const { return m_bIsVisible; }
	void SetVisible(BOOL bIsVisible = TRUE);

	const CBCGPRect& GetRect() const { return m_rect; }
	
	const CBCGPColor& GetTextColor() const { return m_colorText; }
	void SetTextColor(const CBCGPColor& color) { m_colorText = color; }
	
	const CString& GetToolTipText() const { return m_strToolTipText; }
	const CString& GetToolTipDescription() const { return m_strToolTipDescription; }
	
	void SetToolTipText(const CString& strToolTipText);
	void SetToolTipDescription(const CString& strToolTipDescription);

	void SetView(CRuntimeClass* pRTI = RUNTIME_CLASS(CBCGPVisualContainerCtrl), UINT nViewResID = 0, LPCTSTR lpszViewTitle = NULL);
	CRuntimeClass* GetViewRuntimeClass() const { return m_pRTIView; }
	
	UINT GetViewResID () const { return m_nViewResID; }
	const CString& GetViewTitle() const { return m_strViewTitle.IsEmpty() ? m_strName : m_strViewTitle; }

	BOOL IsDragged() const;

// Operation:
public:
	void Redraw();

// Overrides
public:
	virtual void DoDraw(CBCGPWinUITiles* pOwner, CBCGPGraphicsManager* pGM, BOOL bIsSelected, BOOL bIsHighlighted) = 0;
	virtual void OnChangeRect(const CBCGPRect& /* rectOld */) {};

	virtual CWnd* CreateView();

	virtual BOOL IsWide() const { return FALSE; }
	virtual BOOL IsHigh() const { return FALSE; }

	virtual LPCTSTR GetAccName() const;
	virtual LPCTSTR GetAccValue() const { return NULL; }
	virtual LPCTSTR GetAccDescription() const { return NULL; }
	virtual long GetAccRole() const { return ROLE_SYSTEM_PUSHBUTTON; }
	virtual LPCTSTR GetAccDefaultAction() const { return _T("Open"); }
	virtual UINT GetAccState() const;

	virtual void GetCustomProps(CString& /*strCustomProps*/) const {}
	virtual void SetCustomProps(const CString& /*strCustomProps*/) {}
	virtual void OnAfterLoad() {};

protected:
	CBCGPWinUITiles*		m_pOwner;
	CString					m_strName;
	DWORD_PTR				m_dwUserData;
	BOOL					m_bIsVisible;
	CBCGPRect				m_rect;
	CBCGPColor				m_colorText;
	CString					m_strToolTipText;
	CString					m_strToolTipDescription;
	CRuntimeClass*			m_pRTIView;
	UINT					m_nViewResID;
	CString					m_strViewTitle;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITile

class BCGCBPRODLLEXPORT CBCGPWinUITile : public CBCGPWinUIBaseObject
{
	DECLARE_DYNCREATE(CBCGPWinUITile)

	friend class CBCGPWinUITiles;
	friend class CBCGPVisualCollector;
	friend class CBCGPVisualConstructor;

public:
	enum BCGP_WINUI_BADGE_GLYPH
	{
		BCGP_WINUI_BADGE_GLYPH_FIRST = -1,
		BCGP_NONE                    = BCGP_WINUI_BADGE_GLYPH_FIRST,
		BCGP_ACTIVITY                = 0,
		BCGP_ALERT                   = 1,
		BCGP_AVAILABLE               = 2,
		BCGP_AWAY                    = 3,
		BCGP_BUSY                    = 4,
		BCGP_NEW_MESSAGE             = 5,
		BCGP_PAUSED                  = 6,
		BCGP_PLAYING                 = 7,
		BCGP_UNAVAILABLE             = 8,
		BCGP_ERROR                   = 9,
		BCGP_WINUI_BADGE_GLYPH_LAST  = BCGP_ERROR
	};
	
	enum BCGP_WINUI_IMAGE_EFFECT
	{
		BCGP_WINUI_IMAGE_EFFECT_FIRST = 0,
		BCGP_ANIMATION_NONE           = BCGP_WINUI_IMAGE_EFFECT_FIRST,
		BCGP_ANIMATION_FADE           = 1,
		BCGP_WINUI_IMAGE_EFFECT_LAST  = BCGP_ANIMATION_FADE
	};

	enum BCGP_WINUI_TILE_TYPE
	{
		BCGP_WINUI_TILE_TYPE_FIRST = 0,
		BCGP_TILE_REGULAR          = BCGP_WINUI_TILE_TYPE_FIRST,
		BCGP_TILE_WIDE             = 1,
		BCGP_TILE_DOUBLE_SIZE      = 2,
		BCGP_WINUI_TILE_TYPE_LAST  = BCGP_TILE_DOUBLE_SIZE
	};

// Construction
public:
	CBCGPWinUITile(const CString& strName,
		BCGP_WINUI_TILE_TYPE type = BCGP_TILE_REGULAR,
		const CBCGPColor& colorText = CBCGPColor(),
		const CBCGPBrush& brushBackground = CBCGPBrush(),
		const CBCGPColor& colorBorder = CBCGPColor());

	CBCGPWinUITile(const CString& strName,
		BOOL bIsWide = FALSE,
		const CBCGPColor& colorText = CBCGPColor(),
		const CBCGPBrush& brushBackground = CBCGPBrush(),
		const CBCGPColor& colorBorder = CBCGPColor());

	CBCGPWinUITile();
	
	virtual ~CBCGPWinUITile();

protected:
	void CommonInit();
	
// Attributes
public:
	int GetGroupID() const { return m_nGroup; }
	void SetGroupID(int nID, BOOL bRecalcLayout = TRUE);

	virtual BOOL IsWide() const { return m_Type == BCGP_TILE_WIDE || m_Type == BCGP_TILE_DOUBLE_SIZE; }
	virtual BOOL IsHigh() const { return m_Type == BCGP_TILE_DOUBLE_SIZE; }

	BCGP_WINUI_TILE_TYPE GetType() const { return m_Type; }

	int GetBadgeNumber() const { return m_nBadgeNumber; }
	void SetBadgeNumber(int nBadgeNumber);

	BCGP_WINUI_BADGE_GLYPH GetBadgeGlyph() const { return m_BadgeGlyph; }
	void SetBadgeGlyph(BCGP_WINUI_BADGE_GLYPH glyph);

	int GetCustomBadgeIndex() const { return m_nCustomBadgeIndex; }
	void SetCustomBadgeIndex(int nCustomBadgeIndex);

	void ClearBadge();
	
	const CString& GetHeader() const { return m_strHeader; }
	void SetHeader(const CString& strHeader);

	const CString& GetText() const { return m_strText; }
	void SetText(const CString& strText);
	
	const CBCGPImage& GetImage() const { return m_Image; }
	BOOL IsStretchImage() const { return m_bStretchImage; }
	void SetImage(const CBCGPImage& image, BCGP_WINUI_IMAGE_EFFECT effect = BCGP_ANIMATION_NONE, int nAnimationTime = 1000 /* 1 sec */, BOOL bStretch = FALSE);
	
	const CBCGPBrush& GetBackgroundBrush() const { return m_brushBackground; }
	void SetBackgroundBrush(const CBCGPBrush& brush) { m_brushBackground = brush; m_brushBackgroundDark.Empty(); }
	
	const CBCGPColor& GetBorderColor() const { return m_colorBorder; }
	void SetBorderColor(const CBCGPColor& color);
	
	double GetBorderWidth() const { return m_dblBorderWidth; }
	void SetBorderWidth(double dblWidth) { m_dblBorderWidth = dblWidth; }

	CBCGPSize GetSize() const;

// Overrides
public:
	virtual void DoDraw(CBCGPWinUITiles* pOwner, CBCGPGraphicsManager* pGM, BOOL bIsSelected, BOOL bIsHighlighted);
	
	virtual void OnDrawName(CBCGPGraphicsManager* pGM, const CBCGPRect& rectText, const CBCGPTextFormat& tf, const CBCGPColor& clrText);
	virtual CBCGPSize GetNameSize(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf, double dblWidth);
	
	virtual void OnDrawBadge(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBadge, const CBCGPTextFormat& tf, const CBCGPColor& clrText);
	virtual CBCGPSize GetBadgeSize(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf);

	virtual void OnDrawHeader(CBCGPGraphicsManager* pGM, const CBCGPRect& rectHeader, const CBCGPTextFormat& tf, const CBCGPColor& clrText);
	virtual CBCGPSize GetHeaderSize(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf, double dblWidth);

	virtual void OnDrawText(CBCGPGraphicsManager* pGM, const CBCGPRect& rectText, const CBCGPTextFormat& tf, const CBCGPColor& clrText);
	virtual CBCGPSize GetTextSize(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf, double dblWidth);
	
	virtual void OnDrawImage(CBCGPGraphicsManager* pGM, const CBCGPRect& rectImage);
	virtual CBCGPSize GetImageSize(CBCGPGraphicsManager* pGM);

protected:
	static VOID CALLBACK AnimTimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	virtual BOOL OnAnimation();
	void StopAnimation();

protected:
	CBCGPSize				m_sizePadding;
	int						m_nBadgeNumber;
	BCGP_WINUI_BADGE_GLYPH	m_BadgeGlyph;
	int						m_nCustomBadgeIndex;
	CString					m_strHeader;
	CString					m_strText;
	CBCGPImage				m_Image;
	BOOL					m_bStretchImage;
	double					m_dblImageOpacity;
	double					m_dblImageOpacityDelta;
	int						m_nGroup;
	int						m_nImportance;
	BOOL					m_bIsWide;			// Obsolete, use m_Type instead
	BCGP_WINUI_TILE_TYPE	m_Type;
	CBCGPBrush				m_brushBackground;
	CBCGPBrush				m_brushBackgroundDark;
	CBCGPColor				m_colorBorder;
	CBCGPColor				m_colorBorderSel;
	double					m_dblBorderWidth;
	UINT					m_nAnimationID;

	static CMap<UINT,UINT,CBCGPWinUITile*,CBCGPWinUITile*> m_mapAnimations;
	static CCriticalSection g_cs;			// For multi-thread applications
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITiles

class BCGCBPRODLLEXPORT CBCGPWinUITiles : public CBCGPBaseVisualObject  
{
	friend class CBCGPWinUIBaseObject;
	friend class CBCGPWinUITile;
	friend class CBCGPWinUITilesCtrl;
	friend class CBCGPWinUITilesNavigationButton;
	friend class CBCGPWinUITilesGroupCaption;
	friend class CBCGPWinUITilesCaptionButton;
	friend class CBCGPVisualCollector;
	friend class CBCGPVisualConstructor;
	
	DECLARE_DYNAMIC(CBCGPWinUITiles)

// Construction
public:
	CBCGPWinUITiles();
	virtual ~CBCGPWinUITiles();

// Attributes
public:
	const CBCGPRect& GetTilesArea() const { return m_rectTiles; }

	BOOL IsRoundedShapes() const { return m_bRoundedShapes; }
	void SetRoundedShapes(BOOL bSet = TRUE);

	BOOL IsHorizontalLayout() const { return m_bIsHorizontalLayout; }
	void SetHorizontalLayout(BOOL bIsHorizontalLayout);

	void SetHorzMargin(double nMargin);
	void SetVertMargin(double nMargin);

	double GetHorzMargin() const { return m_nHorzMargin; }
	double GetVertMargin() const { return m_nVertMargin; }

	const CBCGPSize& GetSquareSize() const { return m_sizeSquare; }
	void SetSquareSize(const CBCGPSize& size);

	const CBCGPBrush& GetFillBrush() const { return m_brFill; }
	void SetFillBrush(const CBCGPBrush& brFill, BOOL bRedraw = TRUE);

	const CBCGPColor& GetCaptionForegroundColor() const { return m_brCaptionForeground.GetColor(); }
	void SetCaptionForegroundColor(const CBCGPColor& colorCaptionForeground, BOOL bRedraw = TRUE);

	const CBCGPColor& GetTextColor() const { return m_colorTileText; }

	const CBCGPBrush& GetTileBrush() const { return m_brTileFill; }
	const CBCGPBrush& GetTileBrushDark() const { return m_brTileFillDark; }

	void SetCaption(const CString& strCaption, double dblCaptionExtraHeight = 0.0);
	const CString& GetCaption() const { return m_strCaption; }

	CBCGPWinUITilesGroupCaption* SetGroupCaption(int nGroupID, LPCTSTR lpszName, // Set lpszName = NULL to hide the group
		const CBCGPColor& colorText = CBCGPColor(), BOOL bIsClickable = FALSE,
		const CBCGPBrush& brGroupFill = CBCGPBrush(),
		const CBCGPBrush& brGroupOutline = CBCGPBrush());

	CBCGPWinUITilesGroupCaption* GetGroupCaption(int nGroupID);

	const CBCGPTextFormat& GetCaptionTextFormat() const { return m_textFormatCaption; }
	const CBCGPTextFormat& GetGroupCaptionTextFormat() const { return m_textFormatGroupCaption; }
	const CBCGPTextFormat& GetCaptionButtonTextFormat() const { return m_textFormatCaptionButton; }
	const CBCGPTextFormat& GetHeaderTextFormat() const { return m_textFormatHeader; }
	const CBCGPTextFormat& GetNameTextFormat() const { return m_textFormatName; }

	// Custom images list must have 15x15 icons!
	void SetCustomBadgeGlyphs(UINT nImageResID);
	void SetCustomBadgeGlyphs(const CBCGPImage& customBadgeGlyphs);

	const CBCGPImage& GetCustomBadgeGlyphs() const { return m_CustomBadgeGlyphs; }

	void SetScrollBarColorTheme(CBCGPVisualScrollBarColorTheme& theme);
	const CBCGPVisualScrollBarColorTheme& GetScrollBarColorTheme() const { return m_ScrollBar.GetColorTheme(); }

	CWnd* GetCurrentView () const { return m_pWndCurrView; }
	CBCGPWinUIBaseObject* GetViewParent() const { return m_pLastClicked; }

	// Tiles customization
	void EnableTilesDragAndDrop(BOOL bEnable = TRUE) { m_bTilesDragAndDrop = bEnable; }
	BOOL IsTilesDragAndDropEnabled() const { return m_bTilesDragAndDrop; }

	BOOL IsDraggingTile() const { return m_ptDragTile != CBCGPPoint(-1., -1.) && m_pPressed != NULL; }

// Operations
public:
	void Add(CBCGPWinUITile* pTile, int nGroup = 0);
	void AddCaptionButton(CBCGPWinUITilesCaptionButton* pButton);

	void Remove(CBCGPWinUIBaseObject* pObject, BOOL bDelete = TRUE);
	void RemoveTiles();
	void RemoveCaptionButtons();
	void RemoveAll();

	void EnableNavigationBackButton(BOOL bEnable = TRUE, CRuntimeClass* pRTC = NULL);
	BOOL IsNavigationBackButtonEnabled() const { return m_bHasNavigationBackButton; }

	const CList<CBCGPWinUITile*, CBCGPWinUITile*>& GetTilesList() const { return m_lstTiles; }

	CBCGPWinUITile* HitTestTile(const CBCGPPoint& point) const;
	CBCGPWinUITile* FindTileByName(const CString& strName) const;

	void SetCurSel(CBCGPWinUITile* pSelTile);
	CBCGPWinUITile* GetCurSel() const { return DYNAMIC_DOWNCAST(CBCGPWinUITile, m_pSelected); }

	void EnsureVisible(CBCGPWinUIBaseObject* pObject);

	virtual void OnCloseView();

	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

// Overrides
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual void OnDrawTilesGroup(CBCGPGraphicsManager* pGM, const CBCGPRect& rectGroup, CBCGPWinUITilesGroupCaption* pCaption);
	virtual void OnDrawCaption(CBCGPGraphicsManager* pGM, const CBCGPRect& rectCaption);
	virtual void OnDrawGroupCaption(CBCGPGraphicsManager* pGM, CBCGPWinUITilesGroupCaption* pCaption, BOOL bIsHighlighted, BOOL bIsPressed);
	virtual void OnDrawCaptionButton(CBCGPGraphicsManager* pGM, CBCGPWinUITilesCaptionButton* pCaptionButton, BOOL bIsHighlighted, BOOL bIsPressed);
	
	virtual void SetRect(const CBCGPRect& rect, BOOL bRedraw = FALSE)
	{
		SetDirty();
		CBCGPBaseVisualObject::SetRect(rect, bRedraw);
	}

	virtual void OnClickAndHoldEvent(UINT nID, const CBCGPPoint& point);
	virtual void OnScroll(CBCGPVisualScrollBar* pScrollBar, double dblDelta);

	virtual double GetCaptionHeight(CBCGPGraphicsManager* pGM) const;

protected:
	virtual BOOL OnClick(CBCGPWinUIBaseObject* pObject);
	virtual void OnClickTile(CBCGPWinUITile* pClickedTile);
	virtual void OnClickGroupCaption(CBCGPWinUITilesGroupCaption* pCaption);
	virtual void OnClickCaptionButton(CBCGPWinUITilesCaptionButton* pCaptionButton);

	virtual BOOL OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL OnMouseDown(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseUp(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseMove(const CBCGPPoint& pt);
	virtual void OnMouseLeave();
	virtual BOOL OnMouseWheel(const CBCGPPoint& pt, short zDelta);
	virtual BOOL OnSetMouseCursor(const CBCGPPoint& pt);
	virtual void OnCancelMode();
	virtual BOOL OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr);

	virtual BOOL GetGestureConfig(CBCGPGestureConfig& gestureConfig);
	virtual BOOL OnGestureEventPan(const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, CBCGPSize& sizeOverPan);

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);

	virtual CBCGPSize GetGroupCaptionSize(CBCGPGraphicsManager* pGM, int nGroup);

	void SetCurSelObject(CBCGPWinUIBaseObject* pObject);
	BOOL IsObjectSelected(CBCGPWinUIBaseObject* pObject) const { return pObject == m_pSelected; }

	void RecalcLayout(CBCGPGraphicsManager* pGM);
	void RedrawObject(CBCGPWinUIBaseObject* pObject);
	void AddSorted(CBCGPWinUITile* pTile);

	CBCGPWinUIBaseObject* HitTestObject(const CBCGPPoint& point) const;
	void ReposScrollBar(CBCGPGraphicsManager* pGM);

	CBCGPWinUITile* GetFirstVisibleTile() const;
	CBCGPWinUITile* GetLastVisibleTile() const;

	BOOL IsObjectOnScreen(CBCGPWinUIBaseObject* pObject) const;

	virtual CBCGPWinUITilesGroupCaption* OnCreateGroupCaption(int nGroupID);
	
	virtual void OnCreateView(CBCGPWinUIBaseObject* pParentObject);

	virtual BOOL OnCreateViewWnd(CWnd* /*pWnd*/, CBCGPWinUIBaseObject* /*pViewOwner*/)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	void StopTileDragging(BOOL bRepos);

	// MSAA support:	
public:
	CBCGPWinUIBaseObject* GetAccChild(int nIndex);
	long GetAccChildIndex(CBCGPWinUIBaseObject* pTile);

	virtual void NotifyAccessibility(CBCGPWinUIBaseObject* pTile);
	
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);
	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue);
	virtual HRESULT get_accDescription(VARIANT varChild, BSTR *pszDescription);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);

	virtual HRESULT accSelect(long flagsSelect, VARIANT varChild);
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accDoDefaultAction(VARIANT varChild);

// Attributes
protected:
	BOOL										m_bIsHorizontalLayout;
	BOOL										m_bRoundedShapes;

	CList<CBCGPWinUITile*, CBCGPWinUITile*>		m_lstTiles;
	CList<CBCGPWinUITile*, CBCGPWinUITile*>		m_lstTilesSorted;
	CBCGPWinUIBaseObject*						m_pHighlighted;
	CBCGPWinUIBaseObject*						m_pPressed;
	CBCGPWinUIBaseObject*						m_pSelected;
	CBCGPWinUIBaseObject*						m_pLastClicked;
	BOOL										m_bRestoreSel;

	CBCGPPoint									m_ptDragTile;
	CBCGPPoint									m_ptDragTileOffset;
	BOOL										m_bTilesDragAndDrop;
	BOOL										m_bDraggedOut;

	CList<CBCGPWinUITilesGroupCaption*, CBCGPWinUITilesGroupCaption*>	m_lstCaptions;
	CList<CBCGPWinUITilesCaptionButton*, CBCGPWinUITilesCaptionButton*>	m_lstCaptionButtons;
	
	CList<CBCGPWinUIBaseObject*, CBCGPWinUIBaseObject*> m_lstVisibleObjects;

	BOOL										m_bHasNavigationBackButton;

	double										m_nHorzMargin;
	double										m_nHorzMarginOriginal;

	double										m_nVertMargin;
	double										m_nVertMarginOriginal;

	double										m_dblCornerRadius;
	double										m_dblCornerRadiusOriginal;

	CBCGPSize									m_sizeSquare;
	CBCGPSize									m_sizeSquareOriginal;

	CBCGPRect									m_rectTiles;

	CBCGPRect									m_rectCaption;
	double										m_dblCaptionExtraHeight;
	double										m_xLeftCaptionButtons;
	double										m_xRightCaptionButtons;
	CString										m_strCaption;

	CBCGPBrush									m_brFill;
	CBCGPBrush									m_brFocus;
	CBCGPStrokeStyle							m_strokeFocus;
	CBCGPBrush									m_brCaptionForeground;
	CBCGPBrush									m_brTileFill;
	CBCGPBrush									m_brTileFillDark;
	CBCGPColor									m_colorTileText;

	CBCGPTextFormat								m_textFormatCaption;
	CBCGPTextFormat								m_textFormatGroupCaption;
	CBCGPTextFormat								m_textFormatCaptionButton;
	CBCGPTextFormat								m_textFormatHeader;
	CBCGPTextFormat								m_textFormatName;
	CBCGPTextFormat								m_textFormatBadge;
	CBCGPTextFormat								m_textFormatText;

	double										m_dblTotalSize;

	CBCGPVisualScrollBar						m_ScrollBar;

	CBCGPImage									m_BadgeGlyphs;
	CBCGPImage									m_CustomBadgeGlyphs;
	CBCGPImage									m_NavigationBack;

	CWnd*										m_pWndCurrView;
	CString										m_strTopLevelCaption;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITilesCtrl window

class BCGCBPRODLLEXPORT CBCGPWinUITilesCtrl : public CBCGPVisualCtrl
{
	// Construction
public:
	CBCGPWinUITilesCtrl();
	
	// Attributes
public:
	virtual CBCGPWinUITiles* GetWinUITiles() 
	{ 
		if (m_pWinUITiles == NULL)
		{
			m_pWinUITiles = new CBCGPWinUITiles();
		}
		
		return m_pWinUITiles;
	}
	
	// Operations
public:
	
	// Overrides
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetWinUITiles();
	}
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPWinUITilesCtrl)
	//}}AFX_VIRTUAL
	
	// Implementation
public:
	virtual ~CBCGPWinUITilesCtrl();
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPWinUITilesCtrl)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
	CBCGPWinUITiles* m_pWinUITiles;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITilesGroupCaption

class BCGCBPRODLLEXPORT CBCGPWinUITilesGroupCaption : public CBCGPWinUIBaseObject
{
	DECLARE_DYNCREATE(CBCGPWinUITilesGroupCaption)

	friend class CBCGPWinUITiles;

protected:
	CBCGPWinUITilesGroupCaption(int nGroupID = -1)
	{
		m_nID = nGroupID;
		m_bIsClickable = FALSE;
	}

public:	
	int GetID() const				{ return m_nID; }
	BOOL IsClickable() const		{ return m_bIsClickable; }

	const CBCGPBrush& GetFillBrush() const { return m_brFillGroup; }
	const CBCGPBrush& GetOutlineBrush() const { return m_brOutlineGroup; }

protected:
	virtual void DoDraw(CBCGPWinUITiles* pOwner, CBCGPGraphicsManager* pGM, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual long GetAccRole() const { return ROLE_SYSTEM_GROUPING; }

	int			m_nID;
	BOOL		m_bIsClickable;
	CBCGPRect	m_rectGroup;
	CBCGPBrush	m_brFillGroup;
	CBCGPBrush	m_brOutlineGroup;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITilesCaptionButton

class BCGCBPRODLLEXPORT CBCGPWinUITilesCaptionButton : public CBCGPWinUIBaseObject
{
	DECLARE_DYNCREATE(CBCGPWinUITilesCaptionButton)
		
	friend class CBCGPWinUITiles;
	friend class CBCGPVisualConstructor;
	
public:
	CBCGPWinUITilesCaptionButton(UINT nCommandID = 0)
	{
		m_nCommandID = nCommandID;
		m_dblNameHeight = 0.0;
		m_dblTextHeight = 0.0;
	}
	
public:	
	UINT GetCommandID() const { return m_nCommandID; }
	
	const CBCGPImage& GetImage() const { return m_Image; }
	void SetImage(const CBCGPImage& image);

	const CString& GetDescription() const { return m_strDescription; }
	void SetDescription(const CString& strDescription);
	
protected:
	virtual void DoDraw(CBCGPWinUITiles* pOwner, CBCGPGraphicsManager* pGM, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual CBCGPSize GetSize(CBCGPGraphicsManager* pGM);
	virtual BOOL IsRightAligned() const { return TRUE; }
	
	virtual long GetAccRole() const { return ROLE_SYSTEM_PUSHBUTTON; }

	UINT		m_nCommandID;
	CBCGPImage	m_Image;
	CString		m_strDescription;
	double		m_dblNameHeight;
	double		m_dblTextHeight;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPWinUITilesNavigationButton

class BCGCBPRODLLEXPORT CBCGPWinUITilesNavigationButton : public CBCGPWinUITilesCaptionButton
{
	DECLARE_DYNCREATE(CBCGPWinUITilesNavigationButton)
		
	virtual void DoDraw(CBCGPWinUITiles* pOwner, CBCGPGraphicsManager* pGM, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual CBCGPSize GetSize(CBCGPGraphicsManager* pGM);
	virtual BOOL IsRightAligned() const { return FALSE; }
};

#endif // !defined(AFX_BCGPWINUIUITILES_H__1274D27C_573D_4BC5_986D_A63658C8B32E__INCLUDED_)
