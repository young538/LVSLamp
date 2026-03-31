#include "pch.h"
#include "framework.h"
#include "LightController.h"
#include "LightControllerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CLightControllerApp, CBCGPWinApp)
END_MESSAGE_MAP()

CLightControllerApp theApp;

CLightControllerApp::CLightControllerApp()
{
}

BOOL CLightControllerApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CBCGPWinApp::InitInstance();
	AfxEnableControlContainer();

	// BCG Visual Theme: Office 2013 Dark Gray
	CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2013));
	CBCGPVisualManager2013::SetStyle(CBCGPVisualManager2013::Office2013_DarkGray);

	CLightControllerDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}
