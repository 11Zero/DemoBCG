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
// BCGPTagManager.h: interface for the CBCGPTagManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTAGMANAGER_H__33558FB6_5E38_4E3B_96B8_118AD083C0FF__INCLUDED_)
#define AFX_BCGPTAGMANAGER_H__33558FB6_5E38_4E3B_96B8_118AD083C0FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPDrawManager.h"

class CBCGPToolBarImages;
class CBCGPControlRendererParams;
class CBCGPControlRenderer;
class CBCGPToolTipParams;

class CBCGPPoint;
class CBCGPSize;
class CBCGPRect;
class CBCGPColor;
class CBCGPImage;
class CBCGPBrush;
class CBCGPTextFormat;
class CBCGPStrokeStyle;

#ifndef BCGP_EXCLUDE_GRID_CTRL
	#include "BCGPGridCtrl.h"
#endif

class BCGCBPRODLLEXPORT CBCGPTagManager : public CObject  
{
// Construction
public:
	CBCGPTagManager(LPCTSTR lpszBuffer = NULL);
	virtual ~CBCGPTagManager();

// Attributes:
public:
	LPCTSTR GetBuffer () const
	{
		return m_strBuffer;
	}

	void SetBuffer (LPCTSTR lpszBuffer);

	BOOL IsEmpty () const
	{
		return m_strBuffer.IsEmpty ();
	}

// Operations:
public:
	BOOL LoadFromResource (UINT uiResID, LPCTSTR lpszResType);
	BOOL LoadFromResource (LPCTSTR lpszResID, LPCTSTR lpszResType);
	BOOL LoadFromFile (LPCTSTR lpszFileName);

	BOOL ExcludeTag (LPCTSTR lpszTag, CString& strTag, 
					BOOL bIsCharsList = FALSE);

// Global helpers:
	static void SetBaseColor (COLORREF clrBase, COLORREF clrTarget)
	{
		m_clrBase = clrBase;
		m_clrTarget = clrTarget;
	}

	static BOOL ParseString (const CString& str, const CString& sep, CStringArray& sa, BOOL bTrim, BOOL bIncludeEmpty);
	static BOOL ParseBool  (const CString& strValue, BOOL& value);
	static BOOL ParseInt (const CString& strValue, int& value);
	static BOOL ParseDword (const CString& strValue, DWORD& value);
	static BOOL ParseUInt (const CString& strValue, UINT& value);
	static BOOL ParseDouble (const CString& strItem, double& value);
	static BOOL ParseColor (const CString& strItem, COLORREF& value);
	static BOOL ParseColorHEX (const CString& strItem, COLORREF& value);
	static BOOL ParsePoint (const CString& strItem, CPoint& value);
	static BOOL ParseSize (const CString& strItem, CSize& value);
	static BOOL ParseRect (const CString& strItem, CRect& value);
	static BOOL ParseFont (const CString& strItem, LOGFONT& value);
	static BOOL ParseToolBarImages (const CString& strItem, CBCGPToolBarImages& value, UINT ID);
	static BOOL ParseToolBarImages (const CString& strItem, CBCGPToolBarImages& value, LPCTSTR lpszID);
	static BOOL ParseControlRendererParams (const CString& strItem, CBCGPControlRendererParams& value);
	static BOOL ParseControlRenderer (const CString& strItem, CBCGPControlRenderer& value, UINT ID);
	static BOOL ParseControlRenderer (const CString& strItem, CBCGPControlRenderer& value, LPCTSTR lpszID);
	static BOOL ParseToolTipParams (const CString& strItem, CBCGPToolTipParams& value);

	static BOOL ParsePoint (const CString& strItem, CBCGPPoint& value);
	static BOOL ParseSize (const CString& strItem, CBCGPSize& value);
	static BOOL ParseRect (const CString& strItem, CBCGPRect& value);
	static BOOL ParseColor (const CString& strItem, CBCGPColor& value);
	static BOOL ParseImage (const CString& strItem, CBCGPImage& value, UINT ID = (UINT)-1);
	static BOOL ParseBrush (const CString& strItem, CBCGPBrush& value);
	static BOOL ParseTextFormat (const CString& strItem, CBCGPTextFormat& value);
	static BOOL ParseStrokeStyle (const CString& strItem, CBCGPStrokeStyle& value);

	// Read tag methods:
	BOOL ReadString  (const CString& strValue, CString& value);
	BOOL ReadEntityString  (const CString& strValue, CString& value);
	BOOL ReadBool  (const CString& strValue, BOOL& value);
	BOOL ReadInt (const CString& strValue, int& value);
	BOOL ReadDword (const CString& strValue, DWORD& value);
	BOOL ReadUInt (const CString& strValue, UINT& value);
	BOOL ReadDouble (const CString& strValue, double& value);
	BOOL ReadPoint (const CString& strValue, CPoint& value);
	BOOL ReadSize  (const CString& strValue, CSize& value);
	BOOL ReadRect  (const CString& strValue, CRect& value);
	BOOL ReadColor (const CString& strValue, COLORREF& value);
	BOOL ReadFont (const CString& strValue, LOGFONT& value);
	BOOL ReadToolBarImages (const CString& strValue, CBCGPToolBarImages& value, UINT ID);
	BOOL ReadToolBarImages (const CString& strValue, CBCGPToolBarImages& value, LPCTSTR lpszID);
	BOOL ReadControlRendererParams (const CString& strValue, CBCGPControlRendererParams& value);
	BOOL ReadControlRenderer (const CString& strValue, CBCGPControlRenderer& value, UINT ID);
	BOOL ReadControlRenderer (const CString& strValue, CBCGPControlRenderer& value, LPCTSTR lpszID);
	BOOL ReadToolTipParams (const CString& strValue, CBCGPToolTipParams& value);

	BOOL ReadPoint (const CString& strValue, CBCGPPoint& value);
	BOOL ReadSize  (const CString& strValue, CBCGPSize& value);
	BOOL ReadRect  (const CString& strValue, CBCGPRect& value);
	BOOL ReadColor (const CString& strValue, CBCGPColor& value);
	BOOL ReadImage (const CString& strValue, CBCGPImage& value, UINT ID = (UINT)-1);
	BOOL ReadBrush (const CString& strValue, CBCGPBrush& value);
	BOOL ReadTextFormat (const CString& strValue, CBCGPTextFormat& value);
	BOOL ReadStrokeStyle (const CString& strValue, CBCGPStrokeStyle& value);

	BOOL WriteStringTag(const CString& strTagName, const CString& strTagValue);
	BOOL WriteIntTag(const CString& strTagName, int nTagValue);

#ifndef BCGP_EXCLUDE_GRID_CTRL
	static BOOL ParseGridColorData (const CString& strItem, BCGP_GRID_COLOR_DATA::ColorData& value);
	static BOOL ParseGridColors (const CString& strItem, BCGP_GRID_COLOR_DATA& value);

	BOOL ReadGridColorData (const CString& strValue, BCGP_GRID_COLOR_DATA::ColorData& value);
	BOOL ReadGridColors (const CString& strValue, BCGP_GRID_COLOR_DATA& value);
#endif

	static LPCTSTR	s_szXML_UTF8_Prefix;
	static LPCTSTR	s_szSLFCR;
	static LPCTSTR	s_szLFCR;
	static LPCTSTR	s_szTab;
	static LPCTSTR	s_szLFCRT;
	static BOOL		s_bFormatTags;

	static int UTF8ToString (LPCSTR lpSrc, LPTSTR& lpDst, int nLength = -1);
	static int StringToUTF8 (LPCTSTR lpSrc, LPSTR& lpDst, int nLength = -1);

	static CString Entity_ToTag (const CString& value);
	static void Entity_FromTag (CString& value);
	static CString WriteString (const CString& strTag, const CString& value, const CString& valueDefault = CString(), BOOL bEmpty = FALSE);
	static CString WriteEntityString (const CString& strTag, const CString& value, const CString& valueDefault = CString(), BOOL bEmpty = FALSE);
	static CString WritePoint (const CString& strTag, const CPoint& value, const CPoint& valueDefault);
	static CString WriteSize (const CString& strTag, const CSize& value, const CSize& valueDefault);
	static CString WriteRect (const CString& strTag, const CRect& value, const CRect& valueDefault);
	static CString WriteBool (const CString& strTag, BOOL value, BOOL valueDefault);
	static CString WriteInt (const CString& strTag, int value, int valueDefault);
	static CString WriteDword (const CString& strTag, DWORD value, DWORD valueDefault);	
	static CString WriteUInt (const CString& strTag, UINT value, UINT valueDefault);
	static CString WriteDouble (const CString& strTag, double value, double valueDefault);
	static CString WriteColor (const CString& strTag, COLORREF value);
	static CString WriteColor (const CString& strTag, COLORREF value, COLORREF valueDefault);

	static CString FormatImage (const CBCGPImage& value, BOOL bEmpty = TRUE, BOOL bID = FALSE);
	static CString FormatBrush (const CBCGPBrush& value, BOOL bEmpty = TRUE);
	static CString FormatFont (const LOGFONT& value);
	static CString FormatTextFormat (const CBCGPTextFormat& value);
	static CString FormatStrokeStyle (const CBCGPStrokeStyle& value);

	static CString WritePoint (const CString& strTag, const CBCGPPoint& value, const CBCGPPoint& valueDefault);
	static CString WriteSize (const CString& strTag, const CBCGPSize& value, const CBCGPSize& valueDefault);
	static CString WriteRect (const CString& strTag, const CBCGPRect& value, const CBCGPRect& valueDefault);
	static CString WriteColor (const CString& strTag, const CBCGPColor& value);
	static CString WriteColor (const CString& strTag, const CBCGPColor& value, const CBCGPColor& valueDefault);
	static CString WriteImage (const CString& strTag, const CBCGPImage& value, BOOL bEmpty = TRUE, BOOL bID = FALSE);
	static CString WriteBrush (const CString& strTag, const CBCGPBrush& value, BOOL bEmpty = TRUE);
	static CString WriteTextFormat (const CString& strTag, const CBCGPTextFormat& value);
	static CString WriteStrokeStyle (const CString& strTag, const CBCGPStrokeStyle& value);
	static CString WriteFont (const CString& strTag, const LOGFONT& value);

	static void WriteTag (CString& strTag, CString strAddTag);
	static void WriteItem (CString& strTag, const CString& strTagName, CString strAddTag, BOOL bEmpty = FALSE);

protected:
	CString	m_strBuffer;

	static COLORREF m_clrBase;
	static COLORREF m_clrTarget;
};

#endif // !defined(AFX_BCGPTAGMANAGER_H__33558FB6_5E38_4E3B_96B8_118AD083C0FF__INCLUDED_)
