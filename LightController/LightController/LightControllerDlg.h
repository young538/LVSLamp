#pragma once

#include "SerialComm.h"

// 16채널 컨트롤 ID 범위 (리소스에 직접 정의)
#define IDC_EDT_ONTIME_BASE     1100   // 1100~1115
#define IDC_SPIN_ONTIME_BASE    1200   // 1200~1215
#define IDC_CMB_MUL_BASE        1300   // 1300~1315

// 일괄 설정 컨트롤
#define IDC_EDT_ALL_ONTIME      1116
#define IDC_CMB_ALL_MUL         1316
#define IDC_BTN_APPLY_ALL_ONTIME 1400
#define IDC_BTN_APPLY_ALL_MUL   1401

// HOLD 설정 버튼
#define IDC_BTN_SET_HOLD        1135

#define NUM_CHANNELS            16

class CLightControllerDlg : public CBCGPDialog
{
public:
	CLightControllerDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LIGHTCONTROLLER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	HICON m_hIcon;

	DECLARE_MESSAGE_MAP()

private:
	// Serial
	CSerialComm  m_serial;
	CComboBox    m_cmbPort;

	// Mode
	int m_nCmdMode;  // 0=DS-16, 1=LS-12

	// Log
	CEdit m_edtLog;

	// Helpers
	void EnumerateComPorts();
	void SendCommand(const CString& strCmd);
	void AddLog(const CString& strMsg);
	void UpdateUIState();
	void InitChannelControls();

	// Button handlers
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedDisconnect();
	afx_msg void OnBnClickedRefreshPort();
	afx_msg void OnBnClickedAutoFind();

	// 자동 찾기 스레드
	static DWORD WINAPI AutoFindThreadProc(LPVOID lpParam);
	BOOL m_bAutoFinding;

	afx_msg void OnBnClickedSendAll();
	afx_msg void OnBnClickedSendMul();
	afx_msg void OnBnClickedSaveData();
	afx_msg void OnBnClickedReadData();
	afx_msg void OnBnClickedReadMul();

	afx_msg void OnBnClickedTrigger();
	afx_msg void OnBnClickedReset();
	afx_msg void OnBnClickedGetPage();
	afx_msg void OnBnClickedSetStartPage();
	afx_msg void OnBnClickedSetHold();

	afx_msg void OnBnClickedSetSection();
	afx_msg void OnBnClickedSetSkip();

	afx_msg void OnBnClickedClearLog();
	afx_msg void OnBnClickedApplyAllOntime();
	afx_msg void OnBnClickedApplyAllMul();

	afx_msg LRESULT OnSerialReceive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAutoFindLog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAutoFindDone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
};
