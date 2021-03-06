#if C_GRAPHICSMEDIA_LAB_DMOROZ_C
#/*******************************************************************************
# * (C) Copyright 2003/2004 by Vladimir Vezhnevets <vvp@graphics.cs.msu.su>     *
# *******************************************************************************/
#endif

// MFC_GML3View.h : interface of the CImageView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MFC_GML3VIEW_H__EEEF5A93_FA1C_49E5_8E65_25FA407D814F__INCLUDED_)
#define AFX_MFC_GML3VIEW_H__EEEF5A93_FA1C_49E5_8E65_25FA407D814F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CImageView : public CView
{
protected: // create from serialization only
	CImageView();
	DECLARE_DYNCREATE(CImageView)

// Attributes
public:
	CImageDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CImageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNoiseWhite();
	afx_msg void OnNoiseImpulse();
	afx_msg void OnFilters3x3Sharpen();
	afx_msg void OnFilters3x3Blur();
	afx_msg void OnFilters3x3Findedges();
	afx_msg void OnGaussianblur1dversion();
	afx_msg void OnGaussianblur2dversion();
	afx_msg void OnMedianNormal();
	afx_msg void OnMedianVector();
	afx_msg void OnKnnManual();
	afx_msg void OnNlmManual();
	afx_msg void OnKnnAutomatic();
	afx_msg void OnNlmAutomatic();
};

#ifndef _DEBUG  // debug version in MFC_GML3View.cpp
inline CImageDoc* CImageView::GetDocument()
   { return (CImageDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFC_GML3VIEW_H__EEEF5A93_FA1C_49E5_8E65_25FA407D814F__INCLUDED_)
