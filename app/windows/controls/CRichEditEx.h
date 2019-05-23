

#ifndef CRICH_EDIT_EX_H_
#define CRICH_EDIT_EX_H_

#pragma once


#define ID_RICH_UNDO		201
#define ID_RICH_CUT			202
#define ID_RICH_COPY		203
#define ID_RICH_PASTE		204
#define ID_RICH_CLEAR		205
#define ID_RICH_SELECTALL	206
#define ID_RICH_SETFONT		207

#define MSG_COLOR_RED		RGB(0xff,0x00,0x00)
#define MSG_COLOR_BLUE		RGB(0x0c,0x01,0x80)
#define MSG_COLOR_GREEN		RGB(0x00,0xcc,0x00)
#define MSG_COLOR_YELLOW	RGB(0xcc,0xdd,0x00)
#define MSG_COLOR_GRAY		RGB(0xee,0xee,0xee)
#define MSG_COLOR_BLACK		RGB(0x00,0x00,0x00)

#if !(defined(linux)|defined(__CYGWIN__))

/////////////////////////////////////////////////////////////////////////////
// CRichEditEx window

//#define CONTROL_STORE_CLASS 

class CRichEditEx : public CRichEditCtrl
{
	// Construction
public:
	CRichEditEx();
	virtual ~CRichEditEx();


	// Attributes
public:

	// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRichEditEx)
	//}}AFX_VIRTUAL

	//���ܺ���
public:
	//������ɫ
	void SetBackColor(COLORREF bk_color);
	//�����ַ���
	void InsertString(CString strText);
	//�����ַ���
	void InsertString(CString strText, COLORREF fn_color, DWORD fn_effects);
	//�����������
	void SetMaxLines(long maxlines);
	//��ȡ�������
	long GetMaxLines(){ return m_lMaxLines; }
	void SetEditDefFormat();

	// Generated message map functions
protected:
	//�󶨿ؼ�
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//{{AFX_MSG(CRichEditEx)
	// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	//����
	afx_msg void OnCopy() { Copy(); }		
	//����
	afx_msg void OnCut() { Cut(); }	
	//ճ��
	afx_msg void OnPaste() { Paste(); }	
	//ȫѡ
	afx_msg void OnSelectall() { SetSel(0, -1); }	
	//����
	afx_msg void OnUndo() { Undo(); }
	//���
	afx_msg void OnClear() { Clear(); }		
	//�ı�����
	afx_msg void OnSelectfont();					
	//����
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//�˵���Ӧ
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);

	//���ݳ�Ա
protected:
	long		m_lMaxLines;		//�������
	CHARFORMAT	m_cfCurrentFont;	//��ǰ����

	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
#endif
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXRICHEDIT_H__01784CAD_9A89_481A_AC0E_63E1E5205AB3__INCLUDED_)
