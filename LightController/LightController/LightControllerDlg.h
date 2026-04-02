#pragma once

#include "SerialComm.h"


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

	// Log
	CEdit m_edtLog;

	// 읽기 상태
	enum ReadState
	{
		READ_IDLE = 0,           // 대기
		READ_DEVICE_DATA,        // :00R 응답 대기 (ON TIME + MaxPage + 반복 + Section)
		READ_MULTIPLIER,         // :1RU 응답 대기 (배수)
		READ_CUR_PAGE,           // :00G 응답 대기 (현재 페이지)
		READ_PAGE_ONTIME,        // :00R 응답에서 특정 페이지만 읽기
	};

	// 수신 버퍼 & 읽기 상태
	CString m_strRecvBuffer;
	ReadState m_nReadState;
	int m_nPageLineCount;  // 페이지별 읽기 시 수신 라인 카운터

	// 상태바
	CString m_strConnPort;     // 연결된 포트명
	CString m_strLastReadTime; // 마지막 읽기 시각

	// 페이지별 읽기
	int m_nReadTargetPage;     // 읽기 대상 페이지 번호

	// Helpers
	void EnumerateComPorts();
	void SendCommand(const CString& strCmd);
	void AddLog(const CString& strMsg);
	void UpdateUIState();
	void InitChannelControls();
	void ReadDeviceParams();
	void ProcessReceivedLine(const CString& strLine);
	void UpdateStatusBar();

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
	afx_msg void OnBnClickedSetStartPage();
	afx_msg void OnBnClickedSetHold();

	afx_msg void OnBnClickedSetSection();
	afx_msg void OnBnClickedSetSkip();

	afx_msg void OnBnClickedClearLog();
	afx_msg void OnBnClickedDevReadPage();
	afx_msg void OnBnClickedDevReadMul();
	afx_msg void OnBnClickedSetMaxPage();
	afx_msg void OnBnClickedGetCurPage();
	afx_msg void OnBnClickedApplyAllOntime();
	afx_msg void OnBnClickedApplyAllMul();

	afx_msg LRESULT OnSerialReceive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAutoFindLog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAutoFindDone(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();

	CBrush m_brTransparent;
};
