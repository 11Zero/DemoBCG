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
// BCGPRibbonInfoWriter.cpp: implementation of the CBCGPRibbonInfoWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRibbonInfoWriter.h"
#include "BCGPPngImage.h"
#include "BCGPDrawManager.h"
#include "BCGPTagManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef BCGP_EXCLUDE_RIBBON

static LPCTSTR s_szComment        = _T("//");
static LPCTSTR s_szCommentLine    = _T("/////////////////////////////////////////////////////////////////////////////");

static LPCTSTR s_szExtensionRC2   = _T(".ribbon.rc2");
static LPCTSTR s_szExtensionXML   = _T(".xml");
static LPCTSTR s_szExtensionImage = _T(".png");

static LPCTSTR s_szTypeXML        = _T("BCGP_RIBBON_XML");
static LPCTSTR s_szTypeImage      = _T("PNG");

struct XFileID
{
	LPCTSTR szID;
	LPCTSTR szFile;
};

static XFileID s_DefaultXML                 = {_T("_RIBBON_XML"), _T("ribbon")};
static XFileID s_DefaultImageRibbonBar      = {_T("_IMAGE_RIBBON_BAR"), _T("ribbon_bar")};
static XFileID s_DefaultImageMain           = {_T("_IMAGE_MAIN"), _T("main")};
static XFileID s_DefaultImageMainScenic     = {_T("_IMAGE_MAIN_SCENIC"), _T("main_scenic")};
static XFileID s_DefaultMainImageSmall      = {_T("_IMAGE_CATEGORY_MAIN_SMALL"), _T("category_main_small")};
static XFileID s_DefaultMainImageLarge      = {_T("_IMAGE_CATEGORY_MAIN_LARGE"), _T("category_main_large")};
static XFileID s_DefaultBackstageImageSmall = {_T("_IMAGE_CATEGORY_BACKSTAGE_SMALL"), _T("category_backstage_small")};
static XFileID s_DefaultImageSmall          = {_T("_IMAGE_CATEGORY_SMALL"), _T("category_small")};
static XFileID s_DefaultImageLarge          = {_T("_IMAGE_CATEGORY_LARGE"), _T("category_large")};
static XFileID s_DefaultImageContextSmall   = {_T("_IMAGE_CATEGORY_CONTEXT_SMALL"), _T("category_context_small")};
static XFileID s_DefaultImageContextLarge   = {_T("_IMAGE_CATEGORY_CONTEXT_LARGE"), _T("category_context_large")};
static XFileID s_DefaultImageGroup          = {_T("_IMAGE_GROUP"), _T("group")};
static XFileID s_DefaultImagePalette        = {_T("_IMAGE_PALETTE"), _T("palette")};
static XFileID s_DefaultImageStatusBar      = {_T("_IMAGE_STATUS_BAR"), _T("status_bar")};
static XFileID s_DefaultImageStatusGroup    = {_T("_IMAGE_STATUS_GROUP"), _T("status_group")};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonInfoWriter::CBCGPRibbonInfoWriter(CBCGPRibbonInfo& info)
	: CBCGPBaseInfoWriter(info)
	, m_bConvertImage32  (TRUE)
{
}

CBCGPRibbonInfoWriter::~CBCGPRibbonInfoWriter()
{
	EmptyResources ();
}

BOOL CBCGPRibbonInfoWriter::Save (const CString& strFolder, 
								  const CString& strResourceName, 
								  const CString& strResourceFolder,
								  const CString& strResourcePrefix)
{
	if (strFolder.IsEmpty () ||
		strResourceName.IsEmpty ())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	EmptyResources ();

	m_Folder = CBCGPBaseInfoWriter::CorrectDirectoryName (strFolder);
	m_ResourceFolder = CBCGPBaseInfoWriter::CorrectDirectoryName (strResourceFolder);
	m_ResourceName  = strResourceName;
	m_ResourcePrefix = strResourcePrefix;

	if (m_ResourceName.Find (s_szExtensionRC2) == -1)
	{
		m_ResourceName += s_szExtensionRC2;
	}

	if (!Prepare ())
	{
		return FALSE;
	}

	if (!SaveXML ())
	{
		return FALSE;
	}

	if (!SaveImages ())
	{
		return FALSE;
	}

	if (!SaveRC ())
	{
		return FALSE;
	}

	return TRUE;
}

CString CBCGPRibbonInfoWriter::GetFileName (const CString& strFileName) const
{
	return CBCGPBaseInfoWriter::CorrectFileName (m_Folder, CBCGPBaseInfoWriter::PrepareFileName (strFileName));
}

void CBCGPRibbonInfoWriter::CorrectImage (CBCGPRibbonInfo::XImage& image, const CString& strID, const CString& strFile)
{
	if (image.m_Image.IsValid ())
	{
		CString str = m_ResourcePrefix + strID;

		if (!image.m_ID.m_Name.IsEmpty () && str != image.m_ID.m_Name)
		{
			image.m_ID.m_Name.Empty ();
		}

		if (image.m_ID.m_Name.IsEmpty ())
		{
			image.m_ID.m_Name = str;
			AddResource (XResourceFile (image.m_ID.m_Name, 
				s_szTypeImage,
				m_ResourceFolder + strFile + s_szExtensionImage));
		}
	}
	else
	{
		if (!image.m_ID.m_Name.IsEmpty ())
		{
			image.m_ID.m_Name.Empty ();
		}
	}
}

void CBCGPRibbonInfoWriter::CorrectImages (CBCGPRibbonInfo::XElement& info, const CString& strFmt, int& group, int& palette)
{
	CString strName (info.GetElementName ());
	BOOL bPalette = FALSE;

	if (strName.Compare (CBCGPRibbonInfo::s_szButton_Palette) == 0)
	{
		CString strID;
		CString strFile;

		strID.Format (strFmt, s_DefaultImagePalette.szID, palette);
		strFile.Format (strFmt, s_DefaultImagePalette.szFile, palette);

		CorrectImage (((CBCGPRibbonInfo::XElementButtonPalette&)info).m_Images, strID, strFile);
		palette++;

		bPalette = TRUE;
	}
	else if (info.GetElementName ().Compare (CBCGPRibbonInfo::s_szGroup) == 0)
    {
		CBCGPRibbonInfo::XElementGroup& infoGroup = (CBCGPRibbonInfo::XElementGroup&)info;

		CString strID;
		CString strFile;

		strID.Format (strFmt, s_DefaultImageGroup.szID, group);
		strFile.Format (strFmt, s_DefaultImageGroup.szFile, group);

		CorrectImage (infoGroup.m_Images, strID, strFile);
		group++;

		for (int i = 0; i < infoGroup.m_arButtons.GetSize (); i++)
		{
			CorrectImages (*(infoGroup.m_arButtons[i]), strFmt, group, palette);
		}
    }

	if (strName.Compare (CBCGPRibbonInfo::s_szButton) == 0 || 
		strName.Compare (CBCGPRibbonInfo::s_szButton_Color) == 0 || bPalette)
	{
		CBCGPRibbonInfo::XElementButton& infoBtn = (CBCGPRibbonInfo::XElementButton&)info;

		for (int i = 0; i < infoBtn.m_arSubItems.GetSize(); i++)
		{
			CorrectImages (*((CBCGPRibbonInfo::XElement*)(infoBtn.m_arSubItems[i])), strFmt, group, palette);
		}
	}
}

void CBCGPRibbonInfoWriter::CorrectImages ()
{
	CBCGPRibbonInfo::XRibbonBar& infoRB = GetRibbonInfo ().GetRibbonBar ();

	CorrectImage (infoRB.m_Images, s_DefaultImageRibbonBar.szID, s_DefaultImageRibbonBar.szFile);

	if (infoRB.m_btnMain != NULL)
	{
		CorrectImage (infoRB.m_btnMain->m_Image, s_DefaultImageMain.szID, s_DefaultImageMain.szFile);
		CorrectImage (infoRB.m_btnMain->m_ImageScenic, s_DefaultImageMainScenic.szID, s_DefaultImageMainScenic.szFile);
	}

	if (infoRB.m_MainCategory != NULL)
	{
		CorrectImage (infoRB.m_MainCategory->m_SmallImages, s_DefaultMainImageSmall.szID, s_DefaultMainImageSmall.szFile);
		CorrectImage (infoRB.m_MainCategory->m_LargeImages, s_DefaultMainImageLarge.szID, s_DefaultMainImageLarge.szFile);
	}

	if (infoRB.m_BackstageCategory != NULL)
	{
		CorrectImage (infoRB.m_BackstageCategory->m_SmallImages, s_DefaultBackstageImageSmall.szID, s_DefaultBackstageImageSmall.szFile);
	}

	int i = 0;
	int group = 0;
	int palette = 0;
	CString strFmt (_T("%s_%d"));

	for (i = 0; i < infoRB.m_TabElements.m_arButtons.GetSize (); i++)
	{
		CorrectImages (*(infoRB.m_TabElements.m_arButtons[i]), strFmt, group, palette);
	}

	for (i = 0; i < infoRB.m_arCategories.GetSize (); i++)
	{
		CBCGPRibbonInfo::XCategory* category = infoRB.m_arCategories[i];

		CString strID;
		CString strFile;

		strID.Format (_T("%s_%d"), s_DefaultImageSmall.szID, i);
		strFile.Format (_T("%s_%d"), s_DefaultImageSmall.szFile, i);
		CorrectImage (category->m_SmallImages, strID, strFile);

		strID.Format (_T("%s_%d"), s_DefaultImageLarge.szID, i);
		strFile.Format (_T("%s_%d"), s_DefaultImageLarge.szFile, i);
		CorrectImage (category->m_LargeImages, strID, strFile);

		strFmt.Format (_T("_%d_"), i);
		strFmt = _T("%s") + strFmt + _T("%d");
		group = 0;
		palette = 0;

		for (int k = 0; k < category->m_arPanels.GetSize (); k++)
		{
			CBCGPRibbonInfo::XPanel* panel = category->m_arPanels[k];

            for (int n = 0; n < panel->m_arElements.GetSize(); n++)
            {
				CorrectImages (*(panel->m_arElements[n]), strFmt, group, palette);
            }
		}
	}

	for (i = 0; i < infoRB.m_arContexts.GetSize (); i++)
	{
		CBCGPRibbonInfo::XContext* context = infoRB.m_arContexts[i];

		for (int j = 0; j < context->m_arCategories.GetSize (); j++)
		{
			CBCGPRibbonInfo::XCategory* category = context->m_arCategories[j];

			CString strID;
			CString strFile;

			strID.Format (_T("%s_%d_%d"), s_DefaultImageContextSmall.szID, i, j);
			strFile.Format (_T("%s_%d_%d"), s_DefaultImageContextSmall.szFile, i, j);
			CorrectImage (category->m_SmallImages, strID, strFile);

			strID.Format (_T("%s_%d_%d"), s_DefaultImageContextLarge.szID, i, j);
			strFile.Format (_T("%s_%d_%d"), s_DefaultImageContextLarge.szFile, i, j);
			CorrectImage (category->m_LargeImages, strID, strFile);

			strFmt.Format (_T("_%d_%d_"), i, j);
			strFmt = _T("%s") + strFmt + _T("%d");
			group = 0;
			palette = 0;

			for (int k = 0; k < category->m_arPanels.GetSize (); k++)
			{
				CBCGPRibbonInfo::XPanel* panel = category->m_arPanels[k];

				for (int n = 0; n < panel->m_arElements.GetSize(); n++)
				{
					CorrectImages (*(panel->m_arElements[n]), strFmt, group, palette);
				}
			}
		}
	}

	CBCGPRibbonInfo::XStatusBar& infoSB = GetRibbonInfo ().GetStatusBar ();

	CorrectImage (infoSB.m_Images, s_DefaultImageStatusBar.szID, s_DefaultImageStatusBar.szFile);

	int count = 0;
	for (i = 0; i < infoSB.m_Elements.m_arElements.GetSize (); i++)
	{
		if (infoSB.m_Elements.m_arElements[i]->GetElementName ().Compare (CBCGPRibbonInfo::s_szGroup) == 0)
		{
			CString strID;
			CString strFile;

			strID.Format (_T("%s_%d"), s_DefaultImageStatusGroup.szID, count);
			strFile.Format (_T("%s_%d"), s_DefaultImageStatusGroup.szFile, count);
			CorrectImage (((CBCGPRibbonInfo::XElementGroup*)infoSB.m_Elements.m_arElements[i])->m_Images, strID, strFile);
			count++;
		}
	}
	for (i = 0; i < infoSB.m_ExElements.m_arElements.GetSize (); i++)
	{
		if (infoSB.m_ExElements.m_arElements[i]->GetElementName ().Compare (CBCGPRibbonInfo::s_szGroup) == 0)
		{
			CString strID;
			CString strFile;

			strID.Format (_T("%s_%d"), s_DefaultImageStatusGroup.szID, count);
			strFile.Format (_T("%s_%d"), s_DefaultImageStatusGroup.szFile, count);
			CorrectImage (((CBCGPRibbonInfo::XElementGroup*)infoSB.m_ExElements.m_arElements[i])->m_Images, strID, strFile);
			count++;
		}
	}
}

void CBCGPRibbonInfoWriter::CorrectInfo ()
{
	CorrectImages ();
}

void CBCGPRibbonInfoWriter::CorrectNames ()
{
	AddResource (XResourceFile (m_ResourcePrefix + s_DefaultXML.szID, 
			s_szTypeXML, m_ResourceFolder + CString (s_DefaultXML.szFile) + s_szExtensionXML));
}

BOOL CBCGPRibbonInfoWriter::Prepare ()
{
	CString strDir (m_Folder + m_ResourceFolder);
	if (!CBCGPBaseInfoWriter::CreateDirectory (strDir))
	{
		ErrorReportFolder (strDir);

		return FALSE;
	}

	CorrectInfo ();
	CorrectNames ();

	CStringArray sa;

	// RC2 file
	sa.Add (strDir + m_ResourceName);

	// XML file
	{
		const XResourceFileArray* array = GetResourcesArray (s_szTypeXML);
		if (array == NULL || array->GetSize () == 0)
		{
			return FALSE;
		}

		sa.Add (GetFileName (array->GetAt (0)->GetPath ()));
	}

	// Image files
	{
		const XResourceFileArray* array = GetResourcesArray (s_szTypeImage);
		if (array != NULL)
		{
			for (int i = 0; i < array->GetSize (); i++)
			{
				sa.Add (GetFileName (array->GetAt (i)->GetPath ()));
			}
		}
	}

	return CheckFiles (sa);
}

BOOL CBCGPRibbonInfoWriter::SaveRC ()
{
	CStringArray text;

	text.Add (_T("// BCGSoft Designer automatically generated file."));
	text.Add (_T("// Do not edit."));
	text.Add (s_szComment);
	text.Add (_T(""));
	text.Add (_T("#ifdef APSTUDIO_INVOKED"));
	text.Add (_T("	#error this file is not editable by Microsoft Visual C++"));
	text.Add (_T("#endif //APSTUDIO_INVOKED"));
	text.Add (_T(""));

	{
		const XResourceFileArray* array = GetResourcesArray (s_szTypeXML);
		if (array != NULL && array->GetSize () == 1)
		{
			text.Add (s_szCommentLine);
			text.Add (s_szComment);
			text.Add (_T("// ") + CString (s_szTypeXML));
			text.Add (s_szComment);
			text.Add (_T(""));

			text.Add (array->GetAt (0)->Format ());

			text.Add (_T(""));
		}
	}

	{
		const XResourceFileArray* array = GetResourcesArray (s_szTypeImage);
		if (array != NULL && array->GetSize () > 0)
		{
			text.Add (s_szCommentLine);
			text.Add (s_szComment);
			text.Add (_T("// ") + CString (s_szTypeImage));
			text.Add (s_szComment);
			text.Add (_T(""));

			for (int i = 0; i < array->GetSize (); i++)
			{
				text.Add (array->GetAt (i)->Format ());
			}

			text.Add (_T(""));
		}
	}

	return WriteText (m_Folder + m_ResourceFolder + m_ResourceName, text);
}

BOOL CBCGPRibbonInfoWriter::SaveXML ()
{
	const XResourceFileArray* array = GetResourcesArray (s_szTypeXML);
	if (array == NULL || array->GetSize () != 1)
	{
		return FALSE;
	}

	return CBCGPRibbonInfoWriter::SaveInfo (GetFileName (array->GetAt (0)->GetPath ()));
}

BOOL CBCGPRibbonInfoWriter::SaveImages ()
{
	const XResourceFileArray* array = GetResourcesArray (s_szTypeImage);
	if (array == NULL || array->GetSize () == 0)
	{
		return TRUE;
	}

	CBCGPRibbonInfo::XArrayImages images;
	GetRibbonInfo ().GetArrayImages (images);

	BOOL bRes = TRUE;

	for (int i = 0; i < images.GetSize (); i++)
	{
		if (images[i]->m_ID.m_Name.IsEmpty())
		{
			continue;
		}

		int nIndex = GetResourceIndex (*array, images[i]->m_ID.m_Name);
		if (nIndex == -1)
		{
			continue;
		}

		CBitmap bmp;
		HBITMAP hBitmap = images[i]->m_Image.GetImageWell ();
		if (m_bConvertImage32 && images[i]->m_Image.GetBitsPerPixel () < 32)
		{
			COLORREF clr = images[i]->m_Image.GetTransparentColor ();
			hBitmap = CBCGPDrawManager::CreateBitmap_32 (hBitmap, clr == (COLORREF)-1 ? globalData.clrBarFace : clr);
			bmp.Attach (hBitmap);
		}

		CBCGPPngImage pngImage;
		pngImage.Attach (hBitmap);
		BOOL bSave = pngImage.SaveToFile (GetFileName (array->GetAt (nIndex)->GetPath ()));
		pngImage.Detach ();

		if (!bSave)
		{
			bRes = FALSE;
			break;
		}
	}

	return bRes;
}

CString CBCGPRibbonInfoWriter::XResourceFile::Format () const
{
	CString str;

	if (IsValid ())
	{
		CString strPath (m_Path);
		strPath.Replace (_T("\\"), _T("\\\\"));

		str.Format (_T("%s %s \"%s\""), GetID (), GetType (), strPath);
	}

	return str;
}

int CBCGPRibbonInfoWriter::GetResourceIndex (const XResourceFileArray& array, const CString& strID)
{
	int nRes = -1;

	for (int i = 0; i < array.GetSize (); i++)
	{
		if (array[i]->GetID () == strID)
		{
			nRes = i;
			break;
		}
	}

	return nRes;
}

CBCGPRibbonInfoWriter::XResourceFileArray* CBCGPRibbonInfoWriter::GetResourcesArray (const CString& strType) const
{
	XResourceFileArray* array = NULL;

	m_Files.Lookup (strType, (void*&)array);

	return array;
}

BOOL CBCGPRibbonInfoWriter::AddResource (const XResourceFile& resource)
{
	if (!resource.IsValid ())
	{
		return FALSE;
	}

	BOOL bAdd = FALSE;
	XResourceFileArray* array = GetResourcesArray (resource.GetType ());
	if (array == NULL)
	{
		array = new XResourceFileArray;
		m_Files[resource.GetType ()] = array;
		bAdd = TRUE;
	}

	if (bAdd || GetResourceIndex (*array, resource.GetID ()) == -1)
	{
		array->Add (new XResourceFile(resource.GetID (), resource.GetType (), resource.GetPath ()));

		return TRUE;
	}

	return FALSE;
}

void CBCGPRibbonInfoWriter::EmptyResources ()
{
	CString strKey;
	XResourceFileArray* value = NULL;

	POSITION pos = m_Files.GetStartPosition ();
	while (pos != NULL)
	{
		m_Files.GetNextAssoc (pos, strKey, (void*&)value);
		if (value != NULL)
		{
			for (int i = 0; i < value->GetSize (); i++)
			{
				if (value->GetAt (i) != NULL)
				{
					delete value->GetAt (i);
				}
			}

			value->RemoveAll ();
			delete value;
		}
	}

	m_Files.RemoveAll ();
}


CBCGPRibbonCustomizationInfoWriter::CBCGPRibbonCustomizationInfoWriter(CBCGPRibbonCustomizationInfo& info)
	: CBCGPBaseInfoWriter(info)
{
}

CBCGPRibbonCustomizationInfoWriter::~CBCGPRibbonCustomizationInfoWriter()
{
}

BOOL CBCGPRibbonCustomizationInfoWriter::Save (const CString& strFileName)
{
	if (strFileName.IsEmpty ())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CStringArray sa;
	sa.Add (strFileName);

	if (!CheckFiles (sa))
	{
		return FALSE;
	}

	return SaveInfo (strFileName);
}

#endif // BCGP_EXCLUDE_RIBBON
