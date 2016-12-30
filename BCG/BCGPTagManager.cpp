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
// BCGPTagManager.cpp: implementation of the CBCGPTagManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPTagManager.h"
#include "BCGPMath.h"

#ifndef _BCGSUITE_
#include "BCGPToolBarImages.h"
#include "BCGPControlRenderer.h"
#include "BCGPToolTipCtrl.h"
#endif

#include "BCGPDrawManager.h"
#include "BCGPGraphicsManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

COLORREF CBCGPTagManager::m_clrBase    = (COLORREF)-1;
COLORREF CBCGPTagManager::m_clrTarget  = (COLORREF)-1;

LPCTSTR CBCGPTagManager::s_szXML_UTF8_Prefix = _T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
LPCTSTR CBCGPTagManager::s_szSLFCR     = _T("\n");
LPCTSTR CBCGPTagManager::s_szLFCR      = _T("\r\n");
LPCTSTR CBCGPTagManager::s_szTab       = _T("\t");
LPCTSTR CBCGPTagManager::s_szLFCRT     = _T("\r\n\t");
BOOL CBCGPTagManager::s_bFormatTags    = TRUE;

static LPCTSTR s_Point                 = _T("POINT");
static LPCTSTR s_Size                  = _T("SIZE");
static LPCTSTR s_Rect                  = _T("RECT");
static LPCTSTR s_Offset                = _T("OFFSET");
static LPCTSTR s_X                     = _T("X");
static LPCTSTR s_Y                     = _T("Y");
static LPCTSTR s_Z                     = _T("Z");
static LPCTSTR s_Width                 = _T("WIDTH");
static LPCTSTR s_Height                = _T("HEIGHT");
static LPCTSTR s_Angle                 = _T("ANGLE");

static LPCTSTR s_Left                  = _T("LEFT");
static LPCTSTR s_Right                 = _T("RIGHT");
static LPCTSTR s_Top                   = _T("TOP");
static LPCTSTR s_Bottom                = _T("BOTTOM");

static LPCTSTR s_LT                    = _T("LEFTTOP");
static LPCTSTR s_LB                    = _T("LEFTBOTTOM");
static LPCTSTR s_RT                    = _T("RIGHTTOP");
static LPCTSTR s_RB                    = _T("RIGHTBOTTOM");

static LPCTSTR s_A                     = _T("A");
static LPCTSTR s_R                     = _T("R");
static LPCTSTR s_G                     = _T("G");
static LPCTSTR s_B                     = _T("B");

static LPCTSTR s_True                  = _T("TRUE");
static LPCTSTR s_False                 = _T("FALSE");

static LPCTSTR s_FaceName              = _T("FACENAME");
static LPCTSTR s_CharSet               = _T("CHARSET");
static LPCTSTR s_Quality               = _T("QUALITY");
static LPCTSTR s_Weight                = _T("WEIGHT");
static LPCTSTR s_Escapement            = _T("ESCAPEMENT");
static LPCTSTR s_Orientation           = _T("ORIENTATION");
static LPCTSTR s_Italic                = _T("ITALIC");
static LPCTSTR s_Underline             = _T("UNDERLINE");
static LPCTSTR s_StrikeOut             = _T("STRIKEOUT");
static LPCTSTR s_OutPrecision          = _T("OUTPRECISION");
static LPCTSTR s_ClipPrecision         = _T("CLIPPRECISION");
static LPCTSTR s_PitchAndFamily        = _T("PITCHANDFAMILY");

static LPCTSTR s_Corners               = _T("CORNERS");
static LPCTSTR s_Sides                 = _T("SIDES");
static LPCTSTR s_Interior              = _T("INTERIOR");
static LPCTSTR s_Transparent           = _T("TRANSPARENT");
static LPCTSTR s_PreMltCheck           = _T("PREMLTCHECK");
static LPCTSTR s_MapTo3D               = _T("MAPTO3D");
static LPCTSTR s_ResourceID            = _T("RESOURCE_ID");
static LPCTSTR s_ResourceType          = _T("RESOURCE_TYPE");
static LPCTSTR s_Path                  = _T("PATH");
// static LPCTSTR s_IconSize              = _T("ICON_SIZE");
// static LPCTSTR s_AlphaIcon             = _T("ALPHA_ICON");
// static LPCTSTR s_IgnoreAlphaBitmap     = _T("IGNORE_ALPHA_BITMAP");
static LPCTSTR s_LightRatio            = _T("LIGHT_RATIO");
static LPCTSTR s_ColorTheme            = _T("COLOR_THEME");
static LPCTSTR s_SizeDest              = _T("SIZE_DEST");
static LPCTSTR s_Texture               = _T("TEXTURE");
static LPCTSTR s_WaterMark             = _T("WATERMARK");

static LPCTSTR s_ColorBackground       = _T("COLOR_BACKGROUND");
static LPCTSTR s_ColorFill             = _T("COLOR_FILL");
static LPCTSTR s_ColorFillGradient     = _T("COLOR_FILLGRADIENT");
static LPCTSTR s_ColorText             = _T("COLOR_TEXT");
static LPCTSTR s_ColorBorder           = _T("COLOR_BORDER");
static LPCTSTR s_GradientAngle         = _T("GRADIENT_ANGLE");
static LPCTSTR s_BrushType             = _T("TYPE");
static LPCTSTR s_BrushColors           = _T("COLORS");
static LPCTSTR s_BrushColor            = _T("COLOR");
static LPCTSTR s_BrushAngle            = s_Angle;
static LPCTSTR s_Opacity               = _T("OPACITY");

static LPCTSTR s_TextFormatFamily      = _T("FAMILY");
static LPCTSTR s_TextFormatSize        = s_Size;
static LPCTSTR s_TextFormatWeight      = s_Weight;
static LPCTSTR s_TextFormatStyle       = _T("STYLE");
static LPCTSTR s_TextFormatLocale      = _T("LOCALE");
static LPCTSTR s_TextFormatAlignHorz   = _T("ALIGN_HORZ");
static LPCTSTR s_TextFormatAlignVert   = _T("ALIGN_VERT");
static LPCTSTR s_TextFormatWordWrap    = _T("WORD_WRAP");
static LPCTSTR s_TextFormatClipping    = _T("CLIPPING");
static LPCTSTR s_TextFormatAngle       = s_Angle;
static LPCTSTR s_TextFormatUnderline   = _T("UNDERLINE");
static LPCTSTR s_TextFormatStrikethrough = _T("STRIKETHROUGH");

static LPCTSTR s_StrokeStyleStartCap   = _T("STARTCAP");
static LPCTSTR s_StrokeStyleEndCap     = _T("ENDCAP");
static LPCTSTR s_StrokeStyleDashCap    = _T("DASHCAP");
static LPCTSTR s_StrokeStyleLineJoin   = _T("LINEJOIN");
static LPCTSTR s_StrokeStyleMiterLimit = _T("MITERLIMIT");
static LPCTSTR s_StrokeStyleDashStyle  = _T("DASHSTYLE");
static LPCTSTR s_StrokeStyleDashOffset = _T("DASHOFFSET");

static LPCTSTR s_TTP_BallonTooltip     = _T("BALLON");
static LPCTSTR s_TTP_DrawIcon          = _T("DRAW_ICON");
static LPCTSTR s_TTP_DrawDescription   = _T("DRAW_DESCRIPTION");
static LPCTSTR s_TTP_DrawSeparator     = _T("DRAW_SEPARATOR");
static LPCTSTR s_TTP_MaxDescrWidth     = _T("MAX_DESC_WIDTH");
static LPCTSTR s_TTP_RoundedCorners    = _T("ROUNDED_CORNERS");
static LPCTSTR s_TTP_BoldLabel         = _T("BOLD_LABEL");

#ifndef BCGP_EXCLUDE_GRID_CTRL
static LPCTSTR s_Grid_EvenColors       = _T("EVEN_COLORS");
static LPCTSTR s_Grid_OddColors        = _T("ODD_COLORS");
static LPCTSTR s_Grid_GroupColors      = _T("GROUP_COLORS");
static LPCTSTR s_Grid_GroupSelColors   = _T("GROUP_SEL_COLORS");
static LPCTSTR s_Grid_SelColors        = _T("SEL_COLORS");
static LPCTSTR s_Grid_HeaderColors     = _T("HEADER_COLORS");
static LPCTSTR s_Grid_HeaderSelColors  = _T("HEADER_SEL_COLORS");
static LPCTSTR s_Grid_LeftOffsetColors = _T("OFFSET_COLORS");
static LPCTSTR s_Grid_ColorHeader      = _T("COLOR_HEADER");
static LPCTSTR s_Grid_ColorHorzLine    = _T("COLOR_H_LINE");
static LPCTSTR s_Grid_ColorVertLine    = _T("COLOR_V_LINE");
static LPCTSTR s_Grid_ColorPreviewText = _T("COLOR_PREVIEW_TEXT");
#endif

struct XFontWeight
{
	LPCTSTR name;
	LONG    weight;
};

static const XFontWeight s_FontWeight[] = 
{
	{_T("DONTCARE")  , FW_DONTCARE  },
	{_T("THIN")      , FW_THIN      },
	{_T("EXTRALIGHT"), FW_EXTRALIGHT},
	{_T("LIGHT")     , FW_LIGHT     },
	{_T("NORMAL")    , FW_NORMAL    },
	{_T("MEDIUM")    , FW_MEDIUM    },
	{_T("SEMIBOLD")  , FW_SEMIBOLD  },
	{_T("BOLD")      , FW_BOLD      },
	{_T("EXTRABOLD") , FW_EXTRABOLD },
	{_T("HEAVY")     , FW_HEAVY     },
	{_T("ULTRALIGHT"), FW_ULTRALIGHT},
	{_T("REGULAR")   , FW_REGULAR   },
	{_T("DEMIBOLD")  , FW_DEMIBOLD  },
	{_T("ULTRABOLD") , FW_ULTRABOLD },
	{_T("BLACK")     , FW_BLACK     }
};

struct XFontQuality
{
	LPCTSTR name;
	BYTE    quality;
};

static const XFontQuality s_FontQuality[] = 
{
	{_T("DEFAULT")          , DEFAULT_QUALITY          },
	{_T("DRAFT")            , DRAFT_QUALITY            },
	{_T("PROOF")            , PROOF_QUALITY            },
	{_T("NONANTIALIASED")   , NONANTIALIASED_QUALITY   },
	{_T("ANTIALIASED")      , ANTIALIASED_QUALITY      },
	{_T("CLEARTYPE")        , 5},//CLEARTYPE_QUALITY        
	{_T("CLEARTYPE_NATURAL"), 6} //CLEARTYPE_NATURAL_QUALITY
};

struct XEntity
{
	LPCTSTR szCode;
	LPCTSTR szSymbol;
};

static const XEntity s_EntityText [] = 
{
	{_T("&amp;")  , _T("&")},
	{_T("&lt;")   , _T("<")},
	{_T("&gt;")   , _T(">")},
	{_T("&quot;") , _T("\"")},
	{_T("&apos;") , _T("\'")},
	{_T("&circ;") , _T("^")},
	{_T("&tilde;"), _T("~")},
	{_T("&#09;")  , _T("\t")},
	{_T("&#0A;")  , _T("\r")},
	{_T("&#0D;")  , _T("\n")},
	{NULL         , NULL}
};

static const XEntity s_EntityCode [] = 
{
	{_T("\\n")    , _T("\n")},
	{_T("\\t")    , _T("\t")},
	{_T("\\r")    , _T("\r")},
	{NULL         , NULL}
};

static CString Entity_Forward (const CString& value, const XEntity* entity)
{
	CString str (value);

	while (entity->szCode != NULL)
	{
		if (str.Find (entity->szSymbol) != -1)
		{
			str.Replace (entity->szSymbol, entity->szCode);
		}

		entity++;
	}

	return str;
}

static void Entity_Backward (CString& value, const XEntity* entity)
{
	while (entity->szCode != NULL)
	{
		if (value.Find (entity->szCode) != -1)
		{
			value.Replace (entity->szCode, entity->szSymbol);
		}

		entity++;
	}
}

static COLORREF AddaptColor (COLORREF clrOrig, COLORREF clrBase, COLORREF clrTone)
{
	if (clrBase == (COLORREF)-1 || clrTone == (COLORREF)-1 ||
		clrOrig == (COLORREF)-1)
	{
		return clrOrig;
	}

	double dSrcH, dSrcS, dSrcL;
	CBCGPDrawManager::RGBtoHSL (clrBase, &dSrcH, &dSrcS, &dSrcL);

	double dDestH, dDestS, dDestL;
	CBCGPDrawManager::RGBtoHSL (clrTone, &dDestH, &dDestS, &dDestL);

	double DH = dDestH - dSrcH;
	double DL = dDestL - dSrcL;
	double DS = dDestS - dSrcS;

	double H,S,L;
	CBCGPDrawManager::RGBtoHSL (clrOrig, &H, &S, &L);

	if (fabs (dSrcH - H) >= 0.2)
	{
		return clrOrig;
	}

	double HNew = max (0, min (1., H + DH));
	double SNew = max (0, min (1., S + DS));
	double LNew = max (0, min (1., L + DL));

	return CBCGPDrawManager::HLStoRGB_ONE (
		HNew, dDestH > 0.5 ? L : LNew, SNew);
}

static BOOL _ParseBool (const CString& strItem, BOOL& value)
{
	if (strItem.IsEmpty())
	{
		return FALSE;
	}

	value = strItem.CompareNoCase (s_True) == 0;
	return TRUE;
}

static BOOL _ParseInt (const CString& strItem, int& value)
{
	if (strItem.IsEmpty())
	{
		return FALSE;
	}

	value = _ttol (strItem);
	return TRUE;
}

static BOOL _ParseDword (const CString& strItem, DWORD& value)
{
	int nValue = (int)value;
	if (!_ParseInt(strItem, nValue))
	{
		return FALSE;
	}

	value = (DWORD)nValue;
	return TRUE;
}

static BOOL _ParseUInt (const CString& strItem, UINT& value)
{
	int nValue = (int)value;
	if (!_ParseInt(strItem, nValue))
	{
		return FALSE;
	}

	value = (UINT)nValue;
	return TRUE;
}

static BOOL _ParseDouble (const CString& strItem, double& value)
{
	if (strItem.IsEmpty())
	{
		return FALSE;
	}

#ifndef _UNICODE
	value = atof(strItem);
#else
	int length = (int)strItem.GetLength () + 1;
	char* astring = new char[length];
	WideCharToMultiByte (CP_ACP, 0, strItem, -1, astring, length, NULL, NULL);
	value = atof(astring);
	delete [] astring;
#endif

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPTagManager::CBCGPTagManager(LPCTSTR lpszBuffer/* = NULL*/)
{
	SetBuffer (lpszBuffer);
}
//*******************************************************************************
CBCGPTagManager::~CBCGPTagManager()
{
}
//*******************************************************************************
void CBCGPTagManager::SetBuffer (LPCTSTR lpszBuffer)
{
	m_strBuffer = lpszBuffer == NULL ? _T("") : lpszBuffer;
}
//*******************************************************************************
BOOL CBCGPTagManager::LoadFromResource (UINT uiResID, LPCTSTR lpszResType)
{
	return LoadFromResource (MAKEINTRESOURCE (uiResID), lpszResType);
}
//*******************************************************************************
BOOL CBCGPTagManager::LoadFromResource (LPCTSTR lpszResID, LPCTSTR lpszResType)
{
	if (lpszResID == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (lpszResType == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;

	HINSTANCE hInst = AfxFindResourceHandle(lpszResID, lpszResType);
	HRSRC hRsrc = ::FindResource(hInst, lpszResID, lpszResType);
	
	if (hRsrc == NULL)
	{
		return FALSE;
	}

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);

	if (hGlobal == NULL)
	{
		return FALSE;
	}

#ifdef _UNICODE
	LPWSTR lpChars = NULL;

	LPSTR lpa = (LPSTR) LockResource (hGlobal);
	VERIFY(lpa != NULL);
	const size_t nChars = strlen (lpa) + 1;
	lpChars = (LPWSTR) new WCHAR[nChars];

	LPCTSTR lpszXML = AfxA2WHelper (lpChars, lpa, (int)nChars);
#else
	LPCTSTR lpszXML = (LPCTSTR) LockResource (hGlobal);
#endif

	if (lpszXML != NULL)
	{
		SetBuffer (lpszXML);
		bRes = TRUE;
	}

	UnlockResource (hGlobal);
	FreeResource (hGlobal);

#ifdef _UNICODE
	if (lpChars != NULL)
	{
		delete [] lpChars;
		lpChars = NULL;
	}
#endif

	return bRes;
}
//*******************************************************************************
BOOL CBCGPTagManager::LoadFromFile (LPCTSTR lpszFileName)
{
	ASSERT_VALID (this);
	ASSERT (lpszFileName != NULL);

	SetBuffer (NULL);

	CString strFileName = lpszFileName;

	CString strBuffer;
	CString strPath = strFileName;

	if (strFileName.Find (TCHAR('\\')) == -1 &&
		strFileName.Find (TCHAR('/')) == -1 &&
		strFileName.Find (TCHAR(':')) == -1)
	{
		TCHAR lpszFilePath [_MAX_PATH];
		if (::GetModuleFileName (NULL, lpszFilePath, _MAX_PATH) > 0)
		{
			TCHAR path_buffer[_MAX_PATH];   
			TCHAR drive[_MAX_DRIVE];   
			TCHAR dir[_MAX_DIR];
			TCHAR fname[_MAX_FNAME];   
			TCHAR ext[_MAX_EXT];

#if _MSC_VER < 1400
			_tsplitpath (lpszFilePath, drive, dir, NULL, NULL);
			_tsplitpath (strFileName, NULL, NULL, fname, ext);

			_tmakepath (path_buffer, drive, dir, fname, ext);
#else
			_tsplitpath_s (lpszFilePath, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
			_tsplitpath_s (strFileName, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

			_tmakepath_s (path_buffer, drive, dir, fname, ext);
#endif

			strPath = path_buffer;
		}
	}

	try
	{
		CStdioFile file;
		if (!file.Open (strPath, CFile::modeRead))
		{
			TRACE(_T("CBCGPTagManager::LoadFromFile: File not found: %s"), strFileName);
			return FALSE;
		}

		CString str;

		while (file.ReadString (str))
		{
			strBuffer += str;
		}
	}
	catch (CFileException* pEx)
	{
		pEx->ReportError ();
		pEx->Delete ();

		return FALSE;
	}

	SetBuffer (strBuffer);
	return TRUE;
}

BOOL CBCGPTagManager::ExcludeTag (LPCTSTR lpszTag, CString& strTag, 
					BOOL bIsCharsList)
{
	if (lpszTag == NULL)
	{
		return FALSE;
	}

	int nTagLen = (int)_tcsclen (lpszTag);
	if (nTagLen == 0)
	{
		return FALSE;
	}

	const int iBufLen = m_strBuffer.GetLength ();

	CString strTagStart;
	strTagStart.Format (_T("<%s>"), lpszTag);
	const int iTagStartLen = nTagLen + 2;

	int iIndexStart = m_strBuffer.Find (strTagStart);
	if (iIndexStart < 0)
	{
		return FALSE;
	}

	int iStart = iIndexStart + iTagStartLen;

	CString strTagEnd;
	strTagEnd.Format (_T("</%s>"), lpszTag);
	const int iTagEndLen = nTagLen + 3;

	const int iEnd = iBufLen - iTagEndLen;

	int iIndexEnd =  -1;
	int nBalance = 1;
	LPCTSTR pBuffer = m_strBuffer;
	for (int i = iStart; i <= iEnd; i ++)
	{
		LPCTSTR p = pBuffer + i;

		if (*p != TCHAR('<'))
		{
			continue;
		}

		if (i < iEnd && _tcsncmp (p, strTagStart, iTagStartLen) == 0)
		{
			i += iTagStartLen - 1;
			nBalance ++;
			continue;
		}

		if (_tcsncmp (p, strTagEnd, iTagEndLen) == 0)
		{
			nBalance --;
			if (nBalance == 0)
			{
				iIndexEnd = i;
				break;
			}

			i += iTagEndLen - 1;
		}
	}

	if (iIndexEnd == -1 || iStart > iIndexEnd)
	{
		return FALSE;
	}

	strTag = m_strBuffer.Mid (iStart, iIndexEnd - iStart);
	strTag.TrimLeft ();
	strTag.TrimRight ();

	m_strBuffer.Delete (iIndexStart, iIndexEnd + iTagEndLen - iIndexStart);

	if (bIsCharsList)
	{
		if (strTag.GetLength () > 1 && strTag [0] == TCHAR('\"'))
		{
			strTag = strTag.Mid (1, strTag.GetLength () - 2);
		}

		strTag.Replace (_T("\\t"), _T("\t"));
		strTag.Replace (_T("\\n"), _T("\n"));
		strTag.Replace (_T("\\r"), _T("\r"));
		strTag.Replace (_T("\\b"), _T("\b"));
		strTag.Replace (_T("LT"), _T("<"));
		strTag.Replace (_T("GT"), _T(">"));
		strTag.Replace (_T("AMP"), _T("&"));
	}

	return TRUE;
}

BOOL CBCGPTagManager::ParseString (const CString& str, const CString& sep, CStringArray& sa, BOOL bTrim, BOOL bIncludeEmpty)
{
	sa.RemoveAll ();

	if (str.IsEmpty ())
	{
		return FALSE;
	}

	CString s (str);
	if (bTrim)
	{
		s.TrimLeft ();
		s.TrimRight ();
	}
	
	if (s.IsEmpty ())
	{
		return FALSE;
	}

	if (sep.IsEmpty ())
	{
		return FALSE;
	}

	int pos = s.Find (sep);
	int sepLength = sep.GetLength ();

	while (pos != -1)
	{
		CString sp = s.Left (pos);
		s = s.Right (s.GetLength () - sepLength - pos);

		if (bTrim)
		{
			sp.TrimLeft ();
			sp.TrimRight ();
			s.TrimLeft ();
		}

		if ((sp.IsEmpty () && bIncludeEmpty) || !sp.IsEmpty ())
		{
			sa.Add (sp);
		}

		pos = s.Find (sep);

		if (pos == -1 && ((s.IsEmpty () && bIncludeEmpty) || !s.IsEmpty ()))
		{
			sa.Add (s);
		}
	}

	return sa.GetSize () > 0;
}

BOOL CBCGPTagManager::ParseBool  (const CString& strItem, BOOL& value)
{
	CString str (strItem);

	str.TrimLeft ();
	str.TrimRight ();

	return _ParseBool(str, value);
}

BOOL CBCGPTagManager::ParseInt (const CString& strItem, int& value)
{
	CString str (strItem);

	str.TrimLeft ();
	str.TrimRight ();

	return _ParseInt(str, value);
}

BOOL CBCGPTagManager::ParseDword (const CString& strItem, DWORD& value)
{
	CString str (strItem);

	str.TrimLeft ();
	str.TrimRight ();

	return _ParseDword(str, value);
}

BOOL CBCGPTagManager::ParseUInt (const CString& strItem, UINT& value)
{
	CString str (strItem);

	str.TrimLeft ();
	str.TrimRight ();

	return _ParseUInt(str, value);
}

BOOL CBCGPTagManager::ParseDouble (const CString& strItem, double& value)
{
	CString str (strItem);

	str.TrimLeft ();
	str.TrimRight ();

	return _ParseDouble(str, value);
}

BOOL CBCGPTagManager::ParseColor (const CString& strItem, COLORREF& value)
{
	CBCGPTagManager tm (strItem);

	CStringArray sa;

	CString strA, strR, strG, strB;

	tm.ExcludeTag (s_A, strA);
	strA.TrimLeft ();
	strA.TrimRight ();
	tm.ExcludeTag (s_R, strR);
	strR.TrimLeft ();
	strR.TrimRight ();
	tm.ExcludeTag (s_G, strG);
	strG.TrimLeft ();
	strG.TrimRight ();
	tm.ExcludeTag (s_B, strB);
	strB.TrimLeft ();
	strB.TrimRight ();

	if (strR.IsEmpty () || strG.IsEmpty () || strB.IsEmpty ())
	{
		if (!ParseString (strItem, _T(","), sa, TRUE, FALSE))
		{
			strR = tm.GetBuffer ();
			strR.TrimLeft ();
			strR.TrimRight ();

			sa.Add (strR);
		}
	}
	else
	{
		sa.Add (strR);
		sa.Add (strG);
		sa.Add (strB);

		if (!strA.IsEmpty ())
		{
			sa.Add (strA);
		}
	}

	if (sa.GetSize () > 0)
	{
		const int count = (int) sa.GetSize ();
		if (count == 3)
		{
			value = AddaptColor (RGB((BYTE)_ttol(sa[0]), (BYTE)_ttol(sa[1]), (BYTE)_ttol(sa[2])),
				m_clrBase, m_clrTarget);
			return TRUE;
		}
		else if (count == 4)
		{
			value = RGB((BYTE)_ttol(sa[0]), (BYTE)_ttol(sa[1]), (BYTE)_ttol(sa[2])) | (((DWORD)(BYTE)(_ttol(sa[3]))) << 24);
			return TRUE;
		}
		else if (count == 1)
		{
			value = (COLORREF)_ttol(sa[0]);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CBCGPTagManager::ParseColorHEX (const CString& strItem, COLORREF& value)
{
	CString str (strItem);

	str.MakeUpper ();
	str.TrimLeft ();
	str.TrimRight ();

	const int len = str.GetLength ();

	if (len < 6)
	{
		return FALSE;
	}

	BOOL bRes = TRUE;
	BYTE clr[3]	= {0, 0, 0};
	int nColor = 0;
	int nRead = 0;

	int val = 0;

	for (int i = 0; i < 6; i++)
	{
		TCHAR c = str[len - i - 1];

		if (TCHAR('A') <= c && c <= TCHAR('F'))
		{
			val = 10 + (c - TCHAR('A'));
		}
		else if (TCHAR('0') <= c && c <= TCHAR('9'))
		{
			val = c - TCHAR('0');
		}
		else
		{
			bRes = FALSE;
			break;
		}

		if (nRead == 0)
		{
			clr[nColor] = (BYTE)val;
		}
		else
		{
			clr[nColor] |= val << 4;
		}

		nRead++;

		if (nRead == 2)
		{
			nRead = 0;
			nColor++;
		}
	}

	if (bRes)
	{
		value = AddaptColor (RGB (clr[2], clr[1], clr[0]), m_clrBase, m_clrTarget);
	}

	return bRes;
}

BOOL CBCGPTagManager::ParsePoint (const CString& strItem, CPoint& value)
{
	CBCGPTagManager tm (strItem);

	CStringArray sa;

	CString strX, strY;

	tm.ExcludeTag (s_X, strX);
	strX.TrimLeft ();
	strX.TrimRight ();
	tm.ExcludeTag (s_Y, strY);
	strY.TrimLeft ();
	strY.TrimRight ();

	if (strX.IsEmpty () || strY.IsEmpty ())
	{
		if (!ParseString (tm.GetBuffer (), _T(","), sa, TRUE, FALSE))
		{
			return FALSE;
		}
	}
	else
	{
		sa.Add (strX);
		sa.Add (strY);
	}

	if (sa.GetSize () == 2)
	{
		value.x = _ttol(sa[0]);
		value.y = _ttol(sa[1]);
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGPTagManager::ParsePoint (const CString& strItem, CBCGPPoint& value)
{
	CBCGPTagManager tm (strItem);

	CStringArray sa;

	CString strX, strY, strZ;

	tm.ExcludeTag (s_X, strX);
	strX.TrimLeft ();
	strX.TrimRight ();
	tm.ExcludeTag (s_Y, strY);
	strY.TrimLeft ();
	strY.TrimRight ();
	tm.ExcludeTag (s_Z, strZ);
	strZ.TrimLeft ();
	strZ.TrimRight ();

	if (strX.IsEmpty () || strY.IsEmpty () || strZ.IsEmpty ())
	{
		if (!ParseString (tm.GetBuffer (), _T(","), sa, TRUE, FALSE))
		{
			return FALSE;
		}
	}
	else
	{
		sa.Add (strX);
		sa.Add (strY);
		sa.Add (strZ);
	}

	if (sa.GetSize () == 3)
	{
		return _ParseDouble(sa[0], value.x) && _ParseDouble(sa[1], value.y) && _ParseDouble(sa[2], value.z);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ParseSize (const CString& strItem, CSize& value)
{
	CBCGPTagManager tm (strItem);
	CStringArray sa;

	CString strW, strH;

	tm.ExcludeTag (s_Width, strW);
	strW.TrimLeft ();
	strW.TrimRight ();
	tm.ExcludeTag (s_Height, strH);
	strH.TrimLeft ();
	strH.TrimRight ();

	if (strW.IsEmpty () || strH.IsEmpty ())
	{
		if (!ParseString (tm.GetBuffer (), _T(","), sa, TRUE, FALSE))
		{
			return FALSE;
		}
	}
	else
	{
		sa.Add (strW);
		sa.Add (strH);
	}

	if (sa.GetSize () == 2)
	{
		value.cx = _ttol(sa[0]);
		value.cy = _ttol(sa[1]);
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGPTagManager::ParseSize (const CString& strItem, CBCGPSize& value)
{
	CBCGPTagManager tm (strItem);
	CStringArray sa;

	CString strW, strH;

	tm.ExcludeTag (s_Width, strW);
	strW.TrimLeft ();
	strW.TrimRight ();
	tm.ExcludeTag (s_Height, strH);
	strH.TrimLeft ();
	strH.TrimRight ();

	if (strW.IsEmpty () || strH.IsEmpty ())
	{
		if (!ParseString (tm.GetBuffer (), _T(","), sa, TRUE, FALSE))
		{
			return FALSE;
		}
	}
	else
	{
		sa.Add (strW);
		sa.Add (strH);
	}

	if (sa.GetSize () == 2)
	{
		return _ParseDouble(sa[0], value.cx) && _ParseDouble(sa[1], value.cy);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ParseRect (const CString& strItem, CRect& value)
{
	CBCGPTagManager tm (strItem);

	CString str1;
	CString str2;

	tm.ExcludeTag (s_Offset, str1);
	str1.TrimLeft ();
	str1.TrimRight ();
	tm.ExcludeTag (s_Size, str2);
	str2.TrimLeft ();
	str2.TrimRight ();

	CPoint pt (0, 0);
	CSize  sz (0, 0);

	if (ParsePoint (str1, pt) && ParseSize (str2, sz))
	{
		value = CRect (pt, sz);
		return TRUE;
	}

	tm.SetBuffer (strItem);
	tm.ExcludeTag (s_LT, str1);
	str1.TrimLeft ();
	str1.TrimRight ();
	tm.ExcludeTag (s_RB, str2);
	str2.TrimLeft ();
	str2.TrimRight ();

	CPoint pt2 (0, 0);
	if (ParsePoint (str1, pt) && ParsePoint (str2, pt2))
	{
		value = CRect (pt, pt2);
		return TRUE;
	}

	CStringArray sa;

	CString strL, strT, strR, strB;

	tm.SetBuffer (strItem);

	tm.ExcludeTag (s_Left, strL);
	strL.TrimLeft ();
	strL.TrimRight ();
	tm.ExcludeTag (s_Top, strT);
	strT.TrimLeft ();
	strT.TrimRight ();
	tm.ExcludeTag (s_Right, strR);
	strR.TrimLeft ();
	strR.TrimRight ();
	tm.ExcludeTag (s_Bottom, strB);
	strB.TrimLeft ();
	strB.TrimRight ();

	if (strL.IsEmpty () || strT.IsEmpty () || strR.IsEmpty () || strB.IsEmpty ())
	{
		if (!ParseString (tm.GetBuffer (), _T(","), sa, TRUE, FALSE))
		{
			return FALSE;
		}
	}
	else
	{
		sa.Add (strL);
		sa.Add (strT);
		sa.Add (strR);
		sa.Add (strB);
	}

	if (sa.GetSize () == 4)
	{
		value.left   = _ttol(sa[0]);
		value.top    = _ttol(sa[1]);
		value.right  = _ttol(sa[2]);
		value.bottom = _ttol(sa[3]);
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGPTagManager::ParseRect (const CString& strItem, CBCGPRect& value)
{
	CBCGPTagManager tm (strItem);

	CString str1;
	CString str2;

	tm.ExcludeTag (s_Offset, str1);
	str1.TrimLeft ();
	str1.TrimRight ();
	tm.ExcludeTag (s_Size, str2);
	str2.TrimLeft ();
	str2.TrimRight ();

	CBCGPPoint pt (0.0, 0.0);
	CBCGPSize  sz (0.0, 0.0);

	if (ParsePoint (str1, pt) && ParseSize (str2, sz))
	{
		value = CBCGPRect (pt, sz);
		return TRUE;
	}

	tm.SetBuffer (strItem);
	tm.ExcludeTag (s_LT, str1);
	str1.TrimLeft ();
	str1.TrimRight ();
	tm.ExcludeTag (s_RB, str2);
	str2.TrimLeft ();
	str2.TrimRight ();

	CBCGPPoint pt2 (0.0, 0.0);
	if (ParsePoint (str1, pt) && ParsePoint (str2, pt2))
	{
		value = CBCGPRect (pt, pt2);
		return TRUE;
	}

	CStringArray sa;

	CString strL, strT, strR, strB;

	tm.SetBuffer (strItem);

	tm.ExcludeTag (s_Left, strL);
	strL.TrimLeft ();
	strL.TrimRight ();
	tm.ExcludeTag (s_Top, strT);
	strT.TrimLeft ();
	strT.TrimRight ();
	tm.ExcludeTag (s_Right, strR);
	strR.TrimLeft ();
	strR.TrimRight ();
	tm.ExcludeTag (s_Bottom, strB);
	strB.TrimLeft ();
	strB.TrimRight ();

	if (strL.IsEmpty () || strT.IsEmpty () || strR.IsEmpty () || strB.IsEmpty ())
	{
		if (!ParseString (tm.GetBuffer (), _T(","), sa, TRUE, FALSE))
		{
			return FALSE;
		}
	}
	else
	{
		sa.Add (strL);
		sa.Add (strT);
		sa.Add (strR);
		sa.Add (strB);
	}

	if (sa.GetSize () == 4)
	{
		return _ParseDouble(sa[0], value.left) && _ParseDouble(sa[1], value.top) &&
			_ParseDouble(sa[2], value.right) && _ParseDouble(sa[3], value.bottom);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ParseFont (const CString& strItem, LOGFONT& value)
{
	CBCGPTagManager tm (strItem);

	CString strFontItem;

	if (tm.ExcludeTag (s_FaceName, strFontItem))
	{
		ASSERT(!strFontItem.IsEmpty ());

		if (!strFontItem.IsEmpty ())
		{
			memcpy (value.lfFaceName, (LPCTSTR)strFontItem, min(strFontItem.GetLength (), LF_FACESIZE) * sizeof(TCHAR));
		}
	}
	
	int nValue = 0;
	BOOL bValue = FALSE;

	nValue = 0;
	tm.ReadInt (s_Height, nValue);
	value.lfHeight = nValue;

	if (value.lfHeight > 0)
	{
		nValue = 0;
		tm.ReadInt (s_Width, nValue);
		value.lfWidth = nValue;
	}

	nValue = 0;
	tm.ReadInt (s_Escapement, nValue);
	value.lfEscapement = (LONG)nValue;

	nValue = 0;
	tm.ReadInt (s_Orientation, nValue);
	value.lfOrientation = (LONG)nValue;

	if (tm.ExcludeTag (s_Weight, strFontItem))
	{
		for(int i = 0; i < sizeof(s_FontWeight) / sizeof(XFontWeight); i++)
		{
			if(strFontItem.CompareNoCase (s_FontWeight[i].name) == 0)
			{
				value.lfWeight = s_FontWeight[i].weight;
				break;
			}
		}
	}

	bValue = FALSE;
	tm.ReadBool (s_Italic, bValue);
	value.lfItalic = (BYTE)bValue;

	bValue = FALSE;
	tm.ReadBool (s_Underline, bValue);
	value.lfUnderline = (BYTE)bValue;

	bValue = FALSE;
	tm.ReadBool (s_StrikeOut, bValue);
	value.lfStrikeOut = (BYTE)bValue;

	nValue = DEFAULT_CHARSET;
	tm.ReadInt (s_CharSet, nValue);
	value.lfCharSet = (BYTE)nValue;

	nValue = OUT_DEFAULT_PRECIS;
	tm.ReadInt (s_OutPrecision, nValue);
	value.lfOutPrecision = (BYTE)nValue;

	nValue = CLIP_DEFAULT_PRECIS;
	tm.ReadInt (s_ClipPrecision, nValue);
	value.lfClipPrecision = (BYTE)nValue;

	if(tm.ExcludeTag (s_Quality, strFontItem))
	{
		for(int i = 0; i < sizeof(s_FontQuality) / sizeof(XFontQuality); i++)
		{
			if(strFontItem.CompareNoCase (s_FontQuality[i].name) == 0)
			{
				if (s_FontQuality[i].quality <= ANTIALIASED_QUALITY)
				{
					value.lfQuality = s_FontQuality[i].quality;
				}
				else
				{
#if _MSC_VER < 1800
					OSVERSIONINFO osvi;
					osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
					::GetVersionEx (&osvi);

					if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
					   osvi.dwMajorVersion >= 5)
#endif
					{
						value.lfQuality = s_FontQuality[i].quality;
					}
				}
				break;
			}
		}
	}

	nValue = DEFAULT_PITCH;
	tm.ReadInt (s_PitchAndFamily, nValue);
	value.lfPitchAndFamily = (BYTE)nValue;

	return TRUE;
}

BOOL CBCGPTagManager::ParseToolBarImages (const CString& strItem, CBCGPToolBarImages& value, UINT ID)
{
	return ParseToolBarImages (strItem, value, MAKEINTRESOURCE(ID));
}

BOOL CBCGPTagManager::ParseToolBarImages (const CString& strItem, CBCGPToolBarImages& value, LPCTSTR lpszID)
{
	CBCGPTagManager tm (strItem);

	CSize size (value.GetImageSize ());

	value.Clear ();
	value.SetTransparentColor ((COLORREF)(-1));

	tm.ReadSize (s_Size, size);

	BOOL bPreMultiplyCheck = TRUE;
	tm.ReadBool (s_PreMltCheck, bPreMultiplyCheck);

	value.SetPreMultiplyAutoCheck (bPreMultiplyCheck);

	BOOL m_bMapTo3DColors = TRUE;
	tm.ReadBool (s_MapTo3D, m_bMapTo3DColors);

	value.SetMapTo3DColors (m_bMapTo3DColors);

	if (size != CSize (0, 0))
	{
		value.SetImageSize (size);
	}

	value.LoadStr (lpszID);

	if (size == CSize (0, 0))
	{
		value.SetSingleImage ();
	}

	COLORREF clrTransparent = CLR_DEFAULT;
	if (tm.ReadColor (s_Transparent, clrTransparent))
	{
		value.SetTransparentColor (clrTransparent);
	}

#ifndef _BCGSUITE_
	if (m_clrBase != (COLORREF)-1 &&
		m_clrTarget != (COLORREF)-1)
	{
		value.AddaptColors (m_clrBase, m_clrTarget);
	}
#endif

    if (CBCGPToolBarImages::IsRTL () && value.GetImageWell () != NULL &&
		clrTransparent == CLR_DEFAULT)
    {
		BITMAP bmp;
		if (::GetObject (value.GetImageWell (), sizeof (BITMAP), &bmp) != 0)
		{
			if (bmp.bmBitsPixel == 32)
			{
				value.Mirror ();
			}
		}
	}

	return TRUE;
}

BOOL CBCGPTagManager::ParseControlRendererParams (const CString& strItem, CBCGPControlRendererParams& value)
{
	CBCGPTagManager tm (strItem);

	CBCGPControlRendererParams params;
	params.SetResourceID (value.GetResourceID ());

#ifndef _BCGSUITE_
	params.SetBaseColor (m_clrBase, m_clrTarget);
#endif

	if (!tm.ReadRect (s_Rect, params.m_rectImage))
	{
		CSize size;

		if (tm.ReadSize (s_Size, size))
		{
			params.m_rectImage = CRect (CPoint (0, 0), size);
		}
	}

	if (params.m_rectImage.IsRectEmpty ())
	{
		return FALSE;
	}

	tm.ReadRect  (s_Corners, params.m_rectCorners);
	tm.ReadRect  (s_Sides, params.m_rectSides);
	tm.ReadRect  (s_Interior, params.m_rectInter);
	tm.ReadColor (s_Transparent, params.m_clrTransparent);
	tm.ReadBool  (s_PreMltCheck, params.m_bPreMultiplyCheck);
#ifndef _BCGSUITE_
	tm.ReadBool  (s_MapTo3D, params.m_bMapTo3DColors);
#endif
	value = params;

	return TRUE;
}

BOOL CBCGPTagManager::ParseControlRenderer (const CString& strItem, CBCGPControlRenderer& value, UINT ID)
{
	return ParseControlRenderer (strItem, value, MAKEINTRESOURCE (ID));
}

BOOL CBCGPTagManager::ParseControlRenderer (const CString& strItem, CBCGPControlRenderer& value, LPCTSTR lpszID)
{
	value.CleanUp ();

	CBCGPControlRendererParams params (lpszID, CRect (0, 0, 0, 0), CRect (0, 0, 0, 0));

	if (ParseControlRendererParams (strItem, params))
	{
		return value.Create (params);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ParseToolTipParams (const CString& strItem, CBCGPToolTipParams& value)
{
	CBCGPTagManager tm (strItem);

	CBCGPToolTipParams params;

#ifndef _BCGSUITE_
	tm.ReadBool  (s_TTP_BallonTooltip   , params.m_bBallonTooltip);
#else
	tm.ReadBool  (s_TTP_BallonTooltip   , params.m_bBalloonTooltip);
#endif

	tm.ReadBool  (s_TTP_DrawIcon        , params.m_bDrawIcon);
	tm.ReadBool  (s_TTP_DrawDescription , params.m_bDrawDescription);
	tm.ReadInt   (s_TTP_MaxDescrWidth   , params.m_nMaxDescrWidth);
	tm.ReadBool  (s_TTP_RoundedCorners  , params.m_bRoundedCorners);
	tm.ReadBool  (s_TTP_BoldLabel       , params.m_bBoldLabel);
	tm.ReadColor (s_ColorFill           , params.m_clrFill);
	tm.ReadColor (s_ColorFillGradient   , params.m_clrFillGradient);
	tm.ReadInt   (s_GradientAngle       , params.m_nGradientAngle);
	tm.ReadColor (s_ColorText           , params.m_clrText);
	tm.ReadColor (s_ColorBorder         , params.m_clrBorder);
	tm.ReadBool  (s_TTP_DrawSeparator   , params.m_bDrawSeparator);

	value = params;

	return TRUE;
}

BOOL CBCGPTagManager::ParseColor (const CString& strItem, CBCGPColor& value)
{
	CBCGPTagManager tm (strItem);

	CStringArray sa;

	CString strA, strR, strG, strB;

	tm.ExcludeTag (s_A, strA);
	strA.TrimLeft ();
	strA.TrimRight ();
	tm.ExcludeTag (s_R, strR);
	strR.TrimLeft ();
	strR.TrimRight ();
	tm.ExcludeTag (s_G, strG);
	strG.TrimLeft ();
	strG.TrimRight ();
	tm.ExcludeTag (s_B, strB);
	strB.TrimLeft ();
	strB.TrimRight ();

	if (strR.IsEmpty () || strG.IsEmpty () || strB.IsEmpty ())
	{
		if (!ParseString (strItem, _T(","), sa, TRUE, FALSE))
		{
			strR = tm.GetBuffer ();
			strR.TrimLeft ();
			strR.TrimRight ();

			sa.Add (strR);
		}
	}
	else
	{
		sa.Add (strR);
		sa.Add (strG);
		sa.Add (strB);

		if (!strA.IsEmpty ())
		{
			sa.Add (strA);
		}
	}

	if (sa.GetSize () > 0)
	{
		const int count = (int) sa.GetSize ();
		if (count >= 3)
		{
			value.r = bcg_clamp((double)_ttol(sa[0]) / 255.0, 0.0, 1.0);
			value.g = bcg_clamp((double)_ttol(sa[1]) / 255.0, 0.0, 1.0);
			value.b = bcg_clamp((double)_ttol(sa[2]) / 255.0, 0.0, 1.0);
			value.a = count == 4 ? bcg_clamp((double)_ttol(sa[3]) / 255.0, 0.0, 1.0) : 1.0;

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CBCGPTagManager::ParseImage (const CString& strItem, CBCGPImage& value, UINT ID/* = (UINT)-1*/)
{
	CBCGPTagManager tm (strItem);

	if (ID == (UINT)-1)
	{
		if (!tm.ReadUInt (s_ResourceID, ID))
		{
			ID = 0;
		}
	}

	CString strPath;
	tm.ReadEntityString (s_Path, strPath);

	if (ID == 0 && strPath.IsEmpty())
	{
		return FALSE;
	}
/*
	CSize sizeIcon(0, 0);
	tm.ReadSize (s_IconSize, sizeIcon);

	BOOL bIsAlphaIcon = FALSE;
	tm.ReadBool (s_AlphaIcon, bIsAlphaIcon);

	BOOL bIsIgnoreAlphaBitmap = FALSE;
	tm.ReadBool (s_IgnoreAlphaBitmap, bIsIgnoreAlphaBitmap);
*/
	BOOL bMap3dColorsInGDI = FALSE;
	if (tm.ReadBool (s_MapTo3D, bMap3dColorsInGDI))
	{
		value.SetMap3DColorsInGDI(bMap3dColorsInGDI);
	}

	double dblLightRatio = 0.0;
	if (tm.ReadDouble (s_LightRatio, dblLightRatio) && dblLightRatio > 0.0)
	{
		if (dblLightRatio <= 1.0)
		{
			value.MakeDarker(dblLightRatio);
		}
		else if (dblLightRatio <= 2.0)
		{
			value.MakeLighter(dblLightRatio - 1.0);
		}
		else
		{
			value.MakePale(dblLightRatio - 2.0);
		}
	}

	CBCGPColor clrTheme;
	if (tm.ReadColor (s_ColorTheme, clrTheme) && !clrTheme.IsNull())
	{
		value.Colorize(clrTheme);
	}

	CBCGPSize sizeDest;
	if (tm.ReadSize (s_SizeDest, sizeDest) && !sizeDest.IsNull())
	{
		value.Resize(sizeDest);
	}

	if (ID != 0)
	{
		CString strType;
		tm.ReadString (s_ResourceType, strType);

		value.Load (ID, strType.IsEmpty () ? NULL : (LPCTSTR)strType);
	}
	else if (!strPath.IsEmpty ())
	{
		value.Load (strPath);
	}

	return TRUE;
}

BOOL CBCGPTagManager::ParseBrush (const CString& strItem, CBCGPBrush& value)
{
	CBCGPTagManager tm (strItem);

	int type = CBCGPBrush::BCGP_NO_GRADIENT;
	tm.ReadInt(s_BrushType, type);
	type = bcg_clamp(type, CBCGPBrush::BCGP_GRADIENT_TYPE_FIRST, CBCGPBrush::BCGP_GRADIENT_TYPE_LAST);
	int opacity = 255;
	tm.ReadInt (s_Opacity, opacity);

	if (type == CBCGPBrush::BCGP_NO_GRADIENT)
	{
		CBCGPColor clr;
		tm.ReadColor (s_BrushColor, clr);

		value.SetColor (clr, bcg_clamp((double)opacity / 255.0, 0.0, 1.0));
	}
	else
	{
		CString strColors;

		if (tm.ExcludeTag (s_BrushColors, strColors))
		{
			CBCGPTagManager tmColors(strColors);

			CBCGPColor clr1;
			tmColors.ReadColor (s_BrushColor, clr1);
			CBCGPColor clr2;
			tmColors.ReadColor (s_BrushColor, clr2);

			value.SetColors (clr1, clr2, (CBCGPBrush::BCGP_GRADIENT_TYPE)type, bcg_clamp((double)opacity / 255.0, 0.0, 1.0));
		}
	}

	CString strTexture;
	if (tm.ExcludeTag(s_Texture, strTexture))
	{
		CBCGPImage image;
		if (ParseImage(strTexture, image))
		{
			CBCGPTagManager tmImage(strTexture);

			BOOL bWaterMark = FALSE;
			tmImage.ReadBool(s_WaterMark, bWaterMark);

			value.SetTextureImage(image, value.GetColor(), bWaterMark);
		}
	}

	return TRUE;
}

BOOL CBCGPTagManager::ParseTextFormat (const CString& strItem, CBCGPTextFormat& value)
{
	CBCGPTagManager tm (strItem);

	CString family;
	tm.ReadString (s_TextFormatFamily, family);
	if (family.IsEmpty ())
	{
		return FALSE;
	}

	double size = 0.0;
	tm.ReadDouble (s_TextFormatSize, size);
	if (size == 0.0)
	{
		return FALSE;
	}

	int nValue = 0;
	tm.ReadInt (s_TextFormatWeight, nValue);
	long weight = (long)bcg_clamp(nValue, FW_THIN, FW_HEAVY);

	nValue = (int)CBCGPTextFormat::BCGP_FONT_STYLE_NORMAL;
	tm.ReadInt (s_TextFormatStyle, nValue);
	CBCGPTextFormat::BCGP_FONT_STYLE style = (CBCGPTextFormat::BCGP_FONT_STYLE)
		bcg_clamp(nValue, (int)CBCGPTextFormat::BCGP_FONT_STYLE_NORMAL, (int)CBCGPTextFormat::BCGP_FONT_STYLE_ITALIC);

	CString locale;
	tm.ReadString (s_TextFormatLocale, locale);

	CBCGPTextFormat format(family, (float)size, weight, style, locale);

	nValue = (int)CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING;
	tm.ReadInt (s_TextFormatAlignHorz, nValue);
	format.SetTextAlignment ((CBCGPTextFormat::BCGP_TEXT_ALIGNMENT)
		bcg_clamp(nValue, (int)CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING, (int)CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER));

	nValue = (int)CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING;
	tm.ReadInt (s_TextFormatAlignVert, nValue);
	format.SetTextVerticalAlignment ((CBCGPTextFormat::BCGP_TEXT_ALIGNMENT)
		bcg_clamp(nValue, (int)CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING, (int)CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER));

	BOOL bValue = FALSE;
	tm.ReadBool (s_TextFormatUnderline, bValue);
	format.SetUnderline (bValue);

	bValue = FALSE;
	tm.ReadBool (s_TextFormatStrikethrough, bValue);
	format.SetStrikethrough (bValue);

	bValue = FALSE;
	tm.ReadBool (s_TextFormatWordWrap, bValue);
	format.SetWordWrap (bValue);

	bValue = FALSE;
	tm.ReadBool (s_TextFormatClipping, bValue);
	format.SetClipText (bValue);

	double dValue = 0.0;
	tm.ReadDouble (s_TextFormatAngle, dValue);
	format.SetDrawingAngle (dValue);

	value = format;

	return TRUE;
}

BOOL CBCGPTagManager::ParseStrokeStyle (const CString& strItem, CBCGPStrokeStyle& value)
{
	CBCGPTagManager tm (strItem);

	int nValue = CBCGPStrokeStyle::BCGP_CAP_STYLE_FLAT;
	tm.ReadInt (s_StrokeStyleStartCap, nValue);
	CBCGPStrokeStyle::BCGP_CAP_STYLE startCap = (CBCGPStrokeStyle::BCGP_CAP_STYLE)nValue;

	nValue = CBCGPStrokeStyle::BCGP_CAP_STYLE_FLAT;
	tm.ReadInt (s_StrokeStyleEndCap, nValue);
	CBCGPStrokeStyle::BCGP_CAP_STYLE endCap = (CBCGPStrokeStyle::BCGP_CAP_STYLE)nValue;

	nValue = CBCGPStrokeStyle::BCGP_CAP_STYLE_FLAT;
	tm.ReadInt (s_StrokeStyleDashCap, nValue);
	CBCGPStrokeStyle::BCGP_CAP_STYLE dashCap = (CBCGPStrokeStyle::BCGP_CAP_STYLE)nValue;

	nValue = CBCGPStrokeStyle::BCGP_LINE_JOIN_MITER;
	tm.ReadInt (s_StrokeStyleLineJoin, nValue);
	CBCGPStrokeStyle::BCGP_LINE_JOIN lineJoin = (CBCGPStrokeStyle::BCGP_LINE_JOIN)nValue;

	double dValue = 10.0;
	tm.ReadDouble (s_StrokeStyleMiterLimit, dValue);
	float miterLimit = (float)dValue;

	nValue = CBCGPStrokeStyle::BCGP_DASH_STYLE_SOLID;
	tm.ReadInt (s_StrokeStyleDashStyle, nValue);
	CBCGPStrokeStyle::BCGP_DASH_STYLE dashStyle = (CBCGPStrokeStyle::BCGP_DASH_STYLE)nValue;

	dValue = 0.0;
	tm.ReadDouble (s_StrokeStyleDashOffset, dValue);
	float dashOffset = (float)dValue;

	value = CBCGPStrokeStyle (startCap, endCap, dashCap, lineJoin,
		(float)miterLimit, dashStyle, (float)dashOffset);

	return TRUE;
}

BOOL CBCGPTagManager::ReadString (const CString& strValue, CString& value)
{
	BOOL bRes = FALSE;
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		value = strItem;
		bRes = TRUE;
	}

	return bRes;
}

BOOL CBCGPTagManager::ReadEntityString (const CString& strValue, CString& value)
{
	if (!ReadString(strValue, value))
	{
		return FALSE;
	}

	Entity_FromTag(value);
	return TRUE;
}

BOOL CBCGPTagManager::ReadBool (const CString& strValue, BOOL& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		strItem.TrimLeft ();
		strItem.TrimRight ();

		return _ParseBool(strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadInt (const CString& strValue, int& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		strItem.TrimLeft ();
		strItem.TrimRight ();

		return _ParseInt(strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadDword (const CString& strValue, DWORD& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		strItem.TrimLeft ();
		strItem.TrimRight ();

		return _ParseDword(strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadUInt (const CString& strValue, UINT& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		strItem.TrimLeft ();
		strItem.TrimRight ();

		return _ParseUInt(strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadDouble (const CString& strValue, double& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseDouble (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadPoint (const CString& strValue, CPoint& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParsePoint (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadPoint (const CString& strValue, CBCGPPoint& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParsePoint (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadSize  (const CString& strValue, CSize& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseSize (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadSize  (const CString& strValue, CBCGPSize& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseSize (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadRect  (const CString& strValue, CRect& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseRect (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadRect  (const CString& strValue, CBCGPRect& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseRect (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadColor (const CString& strValue, COLORREF& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseColor (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadFont (const CString& strValue, LOGFONT& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseFont (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadToolBarImages (const CString& strValue, CBCGPToolBarImages& value, UINT ID)
{
	return ReadToolBarImages (strValue, value, MAKEINTRESOURCE (ID));
}

BOOL CBCGPTagManager::ReadToolBarImages (const CString& strValue, CBCGPToolBarImages& value, LPCTSTR lpszID)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseToolBarImages (strItem, value, lpszID);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadControlRendererParams (const CString& strValue, CBCGPControlRendererParams& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseControlRendererParams (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadControlRenderer (const CString& strValue, CBCGPControlRenderer& value, UINT ID)
{
	return ReadControlRenderer (strValue, value, MAKEINTRESOURCE (ID));
}

BOOL CBCGPTagManager::ReadControlRenderer (const CString& strValue, CBCGPControlRenderer& value, LPCTSTR lpszID)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseControlRenderer (strItem, value, lpszID);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadToolTipParams (const CString& strValue, CBCGPToolTipParams& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseToolTipParams (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadColor (const CString& strValue, CBCGPColor& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseColor (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadImage (const CString& strValue, CBCGPImage& value, UINT ID/* = (UINT)-1*/)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseImage (strItem, value, ID);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadBrush (const CString& strValue, CBCGPBrush& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseBrush (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadTextFormat (const CString& strValue, CBCGPTextFormat& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseTextFormat (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadStrokeStyle (const CString& strValue, CBCGPStrokeStyle& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseStrokeStyle (strItem, value);
	}

	return FALSE;
}

#ifndef BCGP_EXCLUDE_GRID_CTRL
BOOL CBCGPTagManager::ParseGridColorData (const CString& strItem, BCGP_GRID_COLOR_DATA::ColorData& value)
{
	CBCGPTagManager tm (strItem);

	value.InitColors ();

	tm.ReadColor (s_ColorFill        , value.m_clrBackground);
	tm.ReadColor (s_ColorText        , value.m_clrText);
	tm.ReadColor (s_ColorFillGradient, value.m_clrGradient);
	tm.ReadInt   (s_GradientAngle    , value.m_nGradientAngle);
	tm.ReadColor (s_ColorBorder      , value.m_clrBorder);

	return TRUE;
}

BOOL CBCGPTagManager::ParseGridColors (const CString& strItem, BCGP_GRID_COLOR_DATA& value)
{
	CBCGPGridColors colors;

	CBCGPTagManager tm (strItem);

	tm.ReadGridColorData (s_Grid_EvenColors       , colors.m_EvenColors);
	tm.ReadGridColorData (s_Grid_OddColors        , colors.m_OddColors);
	tm.ReadGridColorData (s_Grid_GroupColors      , colors.m_GroupColors);
	tm.ReadGridColorData (s_Grid_GroupSelColors   , colors.m_GroupSelColors);
	tm.ReadGridColorData (s_Grid_SelColors        , colors.m_SelColors);
	tm.ReadGridColorData (s_Grid_HeaderColors     , colors.m_HeaderColors);
	tm.ReadGridColorData (s_Grid_HeaderSelColors  , colors.m_HeaderSelColors);
	tm.ReadGridColorData (s_Grid_LeftOffsetColors , colors.m_LeftOffsetColors);
	tm.ReadColor (s_ColorBackground      , colors.m_clrBackground);
	tm.ReadColor (s_ColorText            , colors.m_clrText);
	tm.ReadColor (s_Grid_ColorHeader     , colors.m_clrHeader);
	tm.ReadColor (s_Grid_ColorHorzLine   , colors.m_clrHorzLine);
	tm.ReadColor (s_Grid_ColorVertLine   , colors.m_clrVertLine);
	tm.ReadColor (s_Grid_ColorPreviewText, colors.m_clrPreviewText);

	value = colors;

	return TRUE;
}

BOOL CBCGPTagManager::ReadGridColorData (const CString& strValue, BCGP_GRID_COLOR_DATA::ColorData& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseGridColorData (strItem, value);
	}

	return FALSE;
}

BOOL CBCGPTagManager::ReadGridColors (const CString& strValue, BCGP_GRID_COLOR_DATA& value)
{
	CString strItem;

	if (ExcludeTag (strValue, strItem))
	{
		return ParseGridColors (strItem, value);
	}

	return FALSE;
}
#endif

int CBCGPTagManager::UTF8ToString (LPCSTR lpSrc, LPTSTR& lpDst, int nLength/* = -1*/)
{
	int count = ::MultiByteToWideChar (CP_UTF8, 0, lpSrc, nLength, NULL, 0);
	if (count <= 1)
	{
		return 0;
	}

	BOOL bTerm = nLength == -1;
	if (!bTerm)
	{
		count++;
	}

	LPWSTR lpWide = new WCHAR[count];
	memset (lpWide, 0, count * sizeof(WCHAR));

	::MultiByteToWideChar (CP_UTF8, 0, lpSrc, nLength, lpWide, count);
	count--;

#ifdef _UNICODE
	lpDst = lpWide;
#else
	count = ::WideCharToMultiByte (::GetACP (), 0, lpWide, -1, NULL, 0, NULL, 0);

	if (count > 1)
	{
		lpDst = new char[count];
		memset (lpDst, 0, count);

		::WideCharToMultiByte (::GetACP (), 0, lpWide, -1, lpDst, count, NULL, 0);
		count--;
	}

	delete [] lpWide;
#endif

	return count;
}

int CBCGPTagManager::StringToUTF8 (LPCTSTR lpSrc, LPSTR& lpDst, int nLength/* = -1*/)
{
	LPWSTR lpWide = NULL;
	int count = 0;
	BOOL bTerm = nLength == -1;

#ifdef _UNICODE
	lpWide = (LPWSTR)lpSrc;
#else
	count = ::MultiByteToWideChar (::GetACP (), 0, lpSrc, nLength, NULL, 0);
	if (!bTerm)
	{
		count++;
	}

	lpWide = new WCHAR[count];
	memset (lpWide, 0, count * sizeof(WCHAR));

	::MultiByteToWideChar (::GetACP (), 0, lpSrc, nLength, lpWide, count);
	count--;

	nLength = -1;
#endif

	count = ::WideCharToMultiByte (CP_UTF8, 0, lpWide, nLength, NULL, 0, NULL, 0);
	if (count > 1)
	{
		if (!bTerm)
		{
			count++;
		}

		lpDst = new char[count];
		memset (lpDst, 0, count);

		::WideCharToMultiByte (CP_UTF8, 0, lpWide, nLength, lpDst, count, NULL, 0);
		count--;
	}

	if (lpWide != (LPWSTR)lpSrc)
	{
		delete [] lpWide;
	}

	return count;
}

CString CBCGPTagManager::Entity_ToTag (const CString& value)
{
	return Entity_Forward (value, s_EntityText);
}

void CBCGPTagManager::Entity_FromTag (CString& value)
{
	Entity_Backward (value, s_EntityText);
}

CString CBCGPTagManager::WriteString (const CString& strTag, const CString& value, const CString& valueDefault/* = CString()*/, BOOL bEmpty/* = FALSE*/)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString str;

	if (bEmpty || !value.IsEmpty())
	{
		str.Format (_T("<%s>%s</%s>"), strTag, value, strTag);
	}

	return str;
}

CString CBCGPTagManager::WriteEntityString (const CString& strTag, const CString& value, const CString& valueDefault/* = CString()*/, BOOL bEmpty/* = FALSE*/)
{
	return WriteString(strTag, Entity_ToTag(value), Entity_ToTag(valueDefault), bEmpty);
}

CString CBCGPTagManager::WritePoint (const CString& strTag, const CPoint& value, const CPoint& valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString strValue;
	strValue.Format (_T("%d, %d"), value.x, value.y);

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WritePoint (const CString& strTag, const CBCGPPoint& value, const CBCGPPoint& valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString strValue;
	strValue.Format (_T("%f, %f, %f"), value.x, value.y, value.z);

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteSize (const CString& strTag, const CSize& value, const CSize& valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString strValue;
	strValue.Format (_T("%d, %d"), value.cx, value.cy);

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteSize (const CString& strTag, const CBCGPSize& value, const CBCGPSize& valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString strValue;
	strValue.Format (_T("%f, %f"), value.cx, value.cy);

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteRect (const CString& strTag, const CRect& value, const CRect& valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString strValue;
	strValue.Format (_T("%d, %d, %d, %d"), value.left, value.top, value.right, value.bottom);

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteRect (const CString& strTag, const CBCGPRect& value, const CBCGPRect& valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString strValue;
	strValue.Format (_T("%f, %f, %f, %f"), value.left, value.top, value.right, value.bottom);

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteBool (const CString& strTag, BOOL value, BOOL valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString strValue;
	if (value)
	{
		strValue = s_True;
	}
	else
	{
		strValue = s_False;
	}

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteInt (const CString& strTag, int value, int valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString strValue;
	strValue.Format (_T("%d"), value);

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteDword (const CString& strTag, DWORD value, DWORD valueDefault)
{
	return CBCGPTagManager::WriteUInt (strTag, (UINT)value, (UINT)valueDefault);
}

CString CBCGPTagManager::WriteUInt (const CString& strTag, UINT value, UINT valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString strValue;
	strValue.Format (_T("%u"), value);

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteDouble (const CString& strTag, double value, double valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	CString strValue;
	strValue.Format (_T("%f"), value);

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteColor (const CString& strTag, COLORREF value)
{
	CString strValue;
	strValue.Format (_T("%d, %d, %d"), GetRValue (value), GetGValue (value), GetBValue (value));

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteColor (const CString& strTag, COLORREF value, COLORREF valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	return WriteColor (strTag, value);
}

CString CBCGPTagManager::WriteColor (const CString& strTag, const CBCGPColor& value)
{
	CString strValue;
	COLORREF clr = value;

	if (value.a == 1.0)
	{
		strValue.Format (_T("%d, %d, %d"), GetRValue (clr), GetGValue (clr), GetBValue (clr));
	}
	else
	{
		strValue.Format (_T("%d, %d, %d, %d"), GetRValue (clr), GetGValue (clr), GetBValue (clr), bcg_clamp(bcg_round(value.a * 255.0), 0, 255));
	}

	return WriteString (strTag, strValue);
}

CString CBCGPTagManager::WriteColor (const CString& strTag, const CBCGPColor& value, const CBCGPColor& valueDefault)
{
	if (value == valueDefault)
	{
		return CString ();
	}

	return WriteColor (strTag, value);
}

CString CBCGPTagManager::FormatImage (const CBCGPImage& value, BOOL bEmpty, BOOL bID)
{
	CString strValue;

	if (!bEmpty && value.IsEmpty ())
	{
		return strValue;
	}

	if (bID)
	{
		WriteTag (strValue, WriteUInt (s_ResourceID, value.GetResourceID (), 0));
	}

	WriteTag (strValue, WriteString (s_ResourceType, value.GetResourceType ()));
	WriteTag (strValue, WriteEntityString (s_Path, value.GetPath ()));
// 	WriteTag (strValue, WriteSize (s_IconSize, value.GetIconSize (), CSize(0, 0)));
// 	WriteTag (strValue, WriteBool (s_AlphaIcon, value.IsAlphaIcon (), FALSE));
// 	WriteTag (strValue, WriteBool (s_IgnoreAlphaBitmap, value.IsIgnoreAlphaBitmap (), FALSE));
	WriteTag (strValue, WriteBool (s_MapTo3D, value.IsMap3dColorsInGDI (), FALSE));
	WriteTag (strValue, WriteDouble (s_LightRatio, value.GetLightRatio (), 1.0));
	WriteTag (strValue, WriteColor (s_ColorTheme, value.GetColorTheme (), CBCGPColor()));
	WriteTag (strValue, WriteSize (s_SizeDest, value.GetDestSize (), CBCGPSize()));

	return strValue;
}

CString CBCGPTagManager::WriteImage (const CString& strTag, const CBCGPImage& value, BOOL bEmpty, BOOL bID)
{
	CString str;
	WriteItem (str, strTag, FormatImage(value, bEmpty, bID));

	return str;
}

CString CBCGPTagManager::FormatBrush (const CBCGPBrush& value, BOOL bEmpty)
{
	CString strValue;

	if (!bEmpty && value.IsEmpty () && value.GetGradientType() == CBCGPBrush::BCGP_NO_GRADIENT)
	{
		return strValue;
	}

	WriteTag (strValue, WriteInt (s_BrushType, value.GetGradientType(), CBCGPBrush::BCGP_NO_GRADIENT));

	if (value.GetGradientType() == CBCGPBrush::BCGP_NO_GRADIENT)
	{
		WriteTag (strValue, WriteColor (s_BrushColor, value.GetColor()));
	}
	else
	{
		CString strColors;
		WriteTag (strColors, WriteColor (s_BrushColor, value.GetColor()));
		WriteTag (strColors, WriteColor (s_BrushColor, value.GetGradientColor()));
		WriteItem (strValue, s_BrushColors, strColors);
	}

	WriteTag (strValue, WriteInt (s_Opacity, bcg_clamp(bcg_round(value.GetOpacity() * 255.0), 0, 255), 255));

	if (value.HasTextureImage())
	{
		CString strTexture(FormatImage(value.GetTextureImage(), TRUE, TRUE));
		WriteTag (strTexture, WriteBool (s_WaterMark, value.IsWaterMarkImage(), FALSE));

		WriteItem (strValue, s_Texture, strTexture);
	}

	return strValue;
}

CString CBCGPTagManager::WriteBrush (const CString& strTag, const CBCGPBrush& value, BOOL bEmpty)
{
	CString str;
	WriteItem (str, strTag, FormatBrush(value, bEmpty));

	return str;
}

CString CBCGPTagManager::FormatFont (const LOGFONT& value)
{
	CString strValue;

	WriteTag (strValue, WriteString (s_FaceName, value.lfFaceName, CString("")));
	WriteTag (strValue, WriteInt (s_Height, value.lfHeight, 0));

	if (value.lfHeight > 0)
	{
		WriteTag (strValue, WriteInt (s_Width, value.lfWidth, 0));
	}

	WriteTag (strValue, WriteInt (s_Escapement, value.lfEscapement, 0));
	WriteTag (strValue, WriteInt (s_Orientation, value.lfOrientation, 0));

	int nIndex = 4;

	int i = 0;
	for(i = 0; i < sizeof(s_FontWeight) / sizeof(XFontWeight); i++)
	{
		if (s_FontWeight[i].weight == value.lfWeight)
		{
			nIndex = i;
			break;
		}
	}

	WriteTag (strValue, WriteString (s_Weight, s_FontWeight[nIndex].name, CString("")));

	WriteTag (strValue, WriteBool (s_Italic, value.lfItalic, FALSE));
	WriteTag (strValue, WriteBool (s_Underline, value.lfUnderline, FALSE));
	WriteTag (strValue, WriteBool (s_StrikeOut, value.lfStrikeOut, FALSE));
	WriteTag (strValue, WriteInt (s_CharSet, value.lfCharSet, DEFAULT_CHARSET));
	WriteTag (strValue, WriteInt (s_OutPrecision, value.lfOutPrecision, OUT_DEFAULT_PRECIS));
	WriteTag (strValue, WriteInt (s_ClipPrecision, value.lfClipPrecision, CLIP_DEFAULT_PRECIS));

	nIndex = 0;
	for(i = 0; i < sizeof(s_FontQuality) / sizeof(XFontQuality); i++)
	{
		if (s_FontQuality[i].quality == value.lfQuality)
		{
			nIndex = i;
			break;
		}
	}

	WriteTag (strValue, WriteString (s_Quality, s_FontQuality[nIndex].name, CString("")));

	WriteTag (strValue, WriteInt (s_PitchAndFamily, value.lfPitchAndFamily, DEFAULT_PITCH));

	return strValue;
}

CString CBCGPTagManager::WriteFont (const CString& strTag, const LOGFONT& value)
{
	CString str;
	WriteItem (str, strTag, FormatFont(value));

	return str;
}

CString CBCGPTagManager::FormatTextFormat (const CBCGPTextFormat& value)
{
	CString strValue;

	if (!value.IsEmpty ())
	{
		WriteTag (strValue, WriteString (s_TextFormatFamily, value.GetFontFamily()));
		WriteTag (strValue, WriteDouble (s_TextFormatSize, (double)value.GetFontSize(), 0.0));
		WriteTag (strValue, WriteInt (s_TextFormatWeight, value.GetFontWeight(), 0));
		WriteTag (strValue, WriteInt (s_TextFormatStyle, value.GetFontStyle(), CBCGPTextFormat::BCGP_FONT_STYLE_NORMAL));
		WriteTag (strValue, WriteString (s_TextFormatLocale, value.GetFontLocale()));
		WriteTag (strValue, WriteInt (s_TextFormatAlignHorz, value.GetTextAlignment(), CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING));
		WriteTag (strValue, WriteInt (s_TextFormatAlignVert, value.GetTextVerticalAlignment(), CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING));
		WriteTag (strValue, WriteBool (s_TextFormatUnderline, value.IsUnderline(), FALSE));
		WriteTag (strValue, WriteBool (s_TextFormatStrikethrough, value.IsStrikethrough(), FALSE));
		WriteTag (strValue, WriteBool (s_TextFormatWordWrap, value.IsWordWrap(), FALSE));
		WriteTag (strValue, WriteBool (s_TextFormatClipping, value.IsClipText(), FALSE));
		WriteTag (strValue, WriteDouble (s_TextFormatAngle, value.GetDrawingAngle(), 0.0));
	}

	return strValue;
}

CString CBCGPTagManager::WriteTextFormat (const CString& strTag, const CBCGPTextFormat& value)
{
	CString str;
	WriteItem (str, strTag, FormatTextFormat(value));

	return str;
}

CString CBCGPTagManager::FormatStrokeStyle (const CBCGPStrokeStyle& value)
{
	CString strValue;

	if (!value.IsEmpty ())
	{
		WriteTag (strValue, WriteInt (s_StrokeStyleStartCap, (int)value.GetStartCap(), (int)CBCGPStrokeStyle::BCGP_CAP_STYLE_FLAT));
		WriteTag (strValue, WriteInt (s_StrokeStyleEndCap, (int)value.GetEndCap(), (int)CBCGPStrokeStyle::BCGP_CAP_STYLE_FLAT));
		WriteTag (strValue, WriteInt (s_StrokeStyleDashCap, (int)value.GetDashCap(), (int)CBCGPStrokeStyle::BCGP_CAP_STYLE_FLAT));
		WriteTag (strValue, WriteInt (s_StrokeStyleLineJoin, (int)value.GetLineJoin(), (int)CBCGPStrokeStyle::BCGP_LINE_JOIN_MITER));
		WriteTag (strValue, WriteDouble (s_StrokeStyleMiterLimit, (double)value.GetMitterLimit(), 10.0));
		WriteTag (strValue, WriteInt (s_StrokeStyleDashStyle, (int)value.GetDashStyle(), (int)CBCGPStrokeStyle::BCGP_DASH_STYLE_SOLID));
		WriteTag (strValue, WriteDouble (s_StrokeStyleDashOffset, (double)value.GetDashOffset(), 0.0));
	}

	return strValue;
}
CString CBCGPTagManager::WriteStrokeStyle (const CString& strTag, const CBCGPStrokeStyle& value)
{
	CString str;
	WriteItem (str, strTag, FormatStrokeStyle(value));

	return str;
}

void CBCGPTagManager::WriteTag (CString& strTag, CString strAddTag)
{
	if (!strAddTag.IsEmpty())
	{
		if (CBCGPTagManager::s_bFormatTags && !strTag.IsEmpty())
		{
			strTag += s_szLFCR;
		}

		strTag += strAddTag;
	}
}

void CBCGPTagManager::WriteItem (CString& strTag, const CString& strTagName, CString strAddTag, BOOL bEmpty)
{
	if (strAddTag.IsEmpty () && !bEmpty)
	{
		return;
	}

	if (CBCGPTagManager::s_bFormatTags)
	{
		if (!strAddTag.IsEmpty ())
		{
			strAddTag = s_szLFCR + strAddTag;
			strAddTag.Replace (s_szLFCR, s_szLFCRT);
			strAddTag += s_szLFCR;
		}

		if (!strTag.IsEmpty ())
		{
			strTag += s_szLFCR;
		}
	}

	strTag += WriteString (strTagName, strAddTag, CString(), TRUE);
}

BOOL CBCGPTagManager::WriteStringTag(const CString& strTagName, const CString& strTagValue)
{
	WriteItem (m_strBuffer, strTagName, strTagValue);
	return TRUE;
}

BOOL CBCGPTagManager::WriteIntTag(const CString& strTagName, int nTagValue)
{
	CString strTagValue;
	strTagValue.Format(_T("%d"), nTagValue);

	WriteItem (m_strBuffer, strTagName, strTagValue);
	return TRUE;
}
