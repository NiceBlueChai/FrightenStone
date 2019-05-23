#ifndef FRAME_APP_H_
#define FRAME_APP_H_

#pragma once


class CDlgRoot;
class CFrameApp :public CWinApp
{
protected:
	CDlgRoot		m_DlgRoot;
public:
	CFrameApp() {}

	// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual bool FreeResource();

	// ʵ��

public:
	//����Ŀ¼
	bool GetWorkDirectory(TCHAR szWorkDirectory[], WORD wBufferCount);

protected:
	bool AppNotStartedWithSamePath(I32& nPidExists);			//ͳһ·��ֻ����һ��App����
	static boost::atomic_bool m_bFreeResource;
	DECLARE_MESSAGE_MAP()
};

extern CFrameApp theApp;

#endif

