//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPGestureManager.h: interface for the CBCGPGestureManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGESTUREMANAGER_H__FDFFEB8A_F025_4AD6_9125_C69E85F2126F__INCLUDED_)
#define AFX_BCGPGESTUREMANAGER_H__FDFFEB8A_F025_4AD6_9125_C69E85F2126F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

// Gesture support:

#define BCGP_GID_BEGIN			1
#define BCGP_GID_END			2
#define BCGP_GID_ZOOM			3
#define BCGP_GID_PAN			4
#define BCGP_GID_ROTATE			5
#define BCGP_GID_TWOFINGERTAP	6
#define BCGP_GID_PRESSANDTAP	7

typedef HANDLE BCGP_HGESTUREINFO;

/////////////////////////////////////////////////////////////////////////////
// CBCGPGestureConfig

typedef struct tagBCGP_GESTURECONFIG {
    DWORD dwID;                     // gesture ID
    DWORD dwWant;                   // settings related to gesture ID that are to be turned on
    DWORD dwBlock;                  // settings related to gesture ID that are to be turned off
} BCGP_GESTURECONFIG, *PBCGP_GESTURECONFIG;

typedef struct tagBCG_GESTUREINFO {
    UINT cbSize;                    // size, in bytes, of this structure (including variable length Args field)
    DWORD dwFlags;                  // see GF_* flags
    DWORD dwID;                     // gesture ID, see GID_* defines
    HWND hwndTarget;                // handle to window targeted by this gesture
    POINTS ptsLocation;             // current location of this gesture
    DWORD dwInstanceID;             // internally used
    DWORD dwSequenceID;             // internally used
    ULONGLONG ullArguments;         // arguments for gestures whose arguments fit in 8 BYTES
    UINT cbExtraArgs;               // size, in bytes, of extra arguments, if any, that accompany this gesture
} BCGP_GESTUREINFO, *PBCGP_GESTUREINFO;

typedef	BOOL (__stdcall *BCGP_GETGESTUREINFO)(BCGP_HGESTUREINFO, PBCGP_GESTUREINFO);
typedef	BOOL (__stdcall *BCGP_CLOSEGESTUREINFOHANDLE)(BCGP_HGESTUREINFO);
typedef	BOOL (__stdcall *BCGP_SETGESTURECONFIG)(HWND, DWORD, UINT, PBCGP_GESTURECONFIG, UINT);
typedef	BOOL (__stdcall *BCGP_GETGESTURECONFIG)(HWND, DWORD, DWORD, PUINT, PBCGP_GESTURECONFIG, UINT);

#define BCGP_GC_ALLGESTURES                              0x00000001
#define BCGP_GC_ZOOM                                     0x00000001
#define BCGP_GC_PAN                                      0x00000001
#define BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY        0x00000002
#define BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY      0x00000004
#define BCGP_GC_PAN_WITH_GUTTER                          0x00000008
#define BCGP_GC_PAN_WITH_INERTIA                         0x00000010
#define BCGP_GC_ROTATE                                   0x00000001
#define BCGP_GC_TWOFINGERTAP                             0x00000001
#define BCGP_GC_PRESSANDTAP                              0x00000001

class BCGCBPRODLLEXPORT CBCGPGestureConfig : public CObject
{
public:
	CBCGPGestureConfig();
	virtual ~CBCGPGestureConfig();
	
	void EnableZoom(BOOL bEnable = TRUE);
	void EnableRotate(BOOL bEnable = TRUE);
	void EnableTwoFingerTap(BOOL bEnable = TRUE);
	void EnablePressAndTap(BOOL bEnable = TRUE);
	void EnablePan(BOOL bEnable = TRUE, DWORD dwFlags = BCGP_GC_PAN_WITH_GUTTER | BCGP_GC_PAN_WITH_INERTIA);
	
	BOOL IsZoomEnabled() const { return (Get(BCGP_GID_ZOOM) & BCGP_GC_ZOOM) == BCGP_GC_ZOOM; }
	BOOL IsRotateEnabled() const { return (Get(BCGP_GID_ROTATE) & BCGP_GC_ROTATE) == BCGP_GC_ROTATE; }
	BOOL IsTwoFingerTapEnabled() const { return (Get(BCGP_GID_TWOFINGERTAP) & BCGP_GC_TWOFINGERTAP) == BCGP_GC_TWOFINGERTAP; }
	BOOL IsPressAndTapEnabled() const { return (Get(BCGP_GID_PRESSANDTAP) & BCGP_GC_PRESSANDTAP) == BCGP_GC_PRESSANDTAP; }
	BOOL IsPanAllEnabled() const { return (Get(BCGP_GID_PAN) & BCGP_GC_PAN) == BCGP_GC_PAN; }
	BOOL IsPanVerticalEnabled() const { return (Get(BCGP_GID_PAN) & BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY) == BCGP_GC_PAN_WITH_SINGLE_FINGER_VERTICALLY; }
	BOOL IsPanHorizontalEnabled() const { return (Get(BCGP_GID_PAN) & BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY) == BCGP_GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY; }
	BOOL IsPanWithGutterEnabled() const { return (Get(BCGP_GID_PAN) & BCGP_GC_PAN_WITH_GUTTER) == BCGP_GC_PAN_WITH_GUTTER; }
	BOOL IsPanWithInertiaEnabled() const { return (Get(BCGP_GID_PAN) & BCGP_GC_PAN_WITH_INERTIA) == BCGP_GC_PAN_WITH_INERTIA; }
	
	BOOL Modify(DWORD dwID, DWORD dwWant = BCGP_GC_ALLGESTURES, DWORD dwBlock = 0);
	DWORD Get(DWORD dwID, BOOL bWant = TRUE) const;
	
	PBCGP_GESTURECONFIG m_pConfigs;
	int	m_nConfigs;
};

#ifndef GID_ROTATE_ANGLE_TO_ARGUMENT
#define GID_ROTATE_ANGLE_TO_ARGUMENT(_arg_)     ((USHORT)((((_arg_) + 2.0 * 3.14159265) / (4.0 * 3.14159265)) * 65535.0))
#endif

#ifndef GID_ROTATE_ANGLE_FROM_ARGUMENT
#define GID_ROTATE_ANGLE_FROM_ARGUMENT(_arg_)   ((((double)(_arg_) / 65535.0) * 4.0 * 3.14159265) - 2.0 * 3.14159265)
#endif

// Boundary feedback support:

typedef	BOOL (__stdcall *BCGP_BEGINPANNINGFEEDBACK)(HWND);
typedef	BOOL (__stdcall *BCGP_ENDPANNINGFEEDBACK)(HWND, BOOL);
typedef	BOOL (__stdcall *BCGP_UPDATEPANNINGFEEDBACK)(HWND, LONG, LONG, BOOL);

/////////////////////////////////////////////////////////////////////////////
// CBCGPGestureManager

class BCGCBPRODLLEXPORT CBCGPGestureManager
{
public:
	CBCGPGestureManager();
	virtual ~CBCGPGestureManager();

	void Resume()
	{
		UpdateUser32Wrappers();
		UpdateUxThemeWrappers();
	}

	// Gesture wrappers:
	BOOL GetGestureInfo(HWND hwnd, BCGP_HGESTUREINFO hGestureInfo, PBCGP_GESTUREINFO pInfo);
	BOOL CloseGestureInfoHandle(BCGP_HGESTUREINFO hGestureInfo);
	
	BOOL SetGestureConfig(HWND hwnd, const CBCGPGestureConfig& config);
	BOOL GetGestureConfig(HWND hwnd, CBCGPGestureConfig& config);
	
	// Boundary feedback wrappers:
	BOOL BeginPanningFeedback(HWND hwnd);
	BOOL EndPanningFeedback(HWND hwnd, BOOL fAnimateBack = TRUE);
	BOOL UpdatePanningFeedback(HWND hwnd, LONG lTotalOverpanOffsetX, LONG lTotalOverpanOffsetY, BOOL fInInertia);

protected:
	void UpdateUser32Wrappers();
	void UpdateUxThemeWrappers();

	HINSTANCE							m_hinstUser32;
	HINSTANCE							m_hinstUXThemeDLL;
	BCGP_GETGESTUREINFO					m_pfGetGestureInfo;
	BCGP_CLOSEGESTUREINFOHANDLE			m_pfCloseGestureInfoHandle;
	BCGP_SETGESTURECONFIG				m_pfSetGestureConfig;
	BCGP_GETGESTURECONFIG				m_pfGetGestureConfig;
	BCGP_BEGINPANNINGFEEDBACK			m_pfBeginPanningFeedback;
	BCGP_ENDPANNINGFEEDBACK				m_pfEndPanningFeedback;
	BCGP_UPDATEPANNINGFEEDBACK			m_pfUpdatePanningFeedback;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPGestureBase

class BCGCBPRODLLEXPORT CBCGPGestureBase
{
public:
	virtual ~CBCGPGestureBase();

protected:
	CBCGPGestureBase();

	tagBCG_GESTUREINFO*	m_pCurrentGestureInfo;
	CPoint				m_ptGestureFrom;
	ULONGLONG			m_ulGestureArg;

	BOOL ProcessGestureEvent(CWnd* pWndThis, WPARAM wp, LPARAM lp);

	virtual BOOL OnGestureEventZoom(const CPoint& ptCenter, double dblZoomFactor);
	virtual BOOL OnGestureEventPan(const CPoint& ptFrom, const CPoint& ptTo, CSize& sizeOverPan);
	virtual BOOL OnGestureEventRotate(const CPoint& ptCenter, double dblAngle);
	virtual BOOL OnGestureEventTwoFingerTap(const CPoint& ptCenter);
	virtual BOOL OnGestureEventPressAndTap(const CPoint& ptPress, long lDelta);
};

extern BCGCBPRODLLEXPORT CBCGPGestureManager bcgpGestureManager;

#endif // !defined(AFX_BCGPGESTUREMANAGER_H__FDFFEB8A_F025_4AD6_9125_C69E85F2126F__INCLUDED_)
