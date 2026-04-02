#include "pch.h"
#include "framework.h"
#include "LightController.h"
#include "LightControllerDlg.h"
#include "ProtocolBuilder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_AUTOFIND_LOG     (WM_USER + 200)
#define WM_AUTOFIND_DONE    (WM_USER + 201)

CLightControllerDlg::CLightControllerDlg(CWnd* pParent /*=nullptr*/)
	: CBCGPDialog(IDD_LIGHTCONTROLLER_DIALOG, pParent)
	, m_nCmdMode(0)
	, m_bAutoFinding(FALSE)
	, m_nReadState(0)
	, m_nReadTargetPage(-1)
	, m_nPageLineCount(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLightControllerDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_PORT, m_cmbPort);
	DDX_Control(pDX, IDC_EDT_LOG, m_edtLog);
	DDX_Radio(pDX, IDC_RADIO_DS16, m_nCmdMode);
}

BEGIN_MESSAGE_MAP(CLightControllerDlg, CBCGPDialog)
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CLightControllerDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_BTN_DISCONNECT, &CLightControllerDlg::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_BTN_REFRESH_PORT, &CLightControllerDlg::OnBnClickedRefreshPort)
	ON_BN_CLICKED(IDC_BTN_AUTO_FIND, &CLightControllerDlg::OnBnClickedAutoFind)
	ON_BN_CLICKED(IDC_BTN_SEND_ALL, &CLightControllerDlg::OnBnClickedSendAll)
	ON_BN_CLICKED(IDC_BTN_SEND_MUL, &CLightControllerDlg::OnBnClickedSendMul)
	ON_BN_CLICKED(IDC_BTN_SAVE_DATA, &CLightControllerDlg::OnBnClickedSaveData)
	ON_BN_CLICKED(IDC_BTN_READ_DATA, &CLightControllerDlg::OnBnClickedReadData)
	ON_BN_CLICKED(IDC_BTN_READ_MUL, &CLightControllerDlg::OnBnClickedReadMul)
	ON_BN_CLICKED(IDC_BTN_TRIGGER, &CLightControllerDlg::OnBnClickedTrigger)
	ON_BN_CLICKED(IDC_BTN_RESET, &CLightControllerDlg::OnBnClickedReset)
	ON_BN_CLICKED(IDC_BTN_GET_PAGE, &CLightControllerDlg::OnBnClickedGetPage)
	ON_BN_CLICKED(IDC_BTN_SET_START_PAGE, &CLightControllerDlg::OnBnClickedSetStartPage)
	ON_BN_CLICKED(IDC_BTN_SET_HOLD, &CLightControllerDlg::OnBnClickedSetHold)
	ON_BN_CLICKED(IDC_BTN_SET_SECTION, &CLightControllerDlg::OnBnClickedSetSection)
	ON_BN_CLICKED(IDC_BTN_SET_SKIP, &CLightControllerDlg::OnBnClickedSetSkip)
	ON_BN_CLICKED(IDC_BTN_CLEAR_LOG, &CLightControllerDlg::OnBnClickedClearLog)
	ON_BN_CLICKED(IDC_BTN_DEV_READ_PAGE, &CLightControllerDlg::OnBnClickedDevReadPage)
	ON_BN_CLICKED(IDC_BTN_APPLY_ALL_ONTIME, &CLightControllerDlg::OnBnClickedApplyAllOntime)
	ON_BN_CLICKED(IDC_BTN_APPLY_ALL_MUL, &CLightControllerDlg::OnBnClickedApplyAllMul)
	ON_MESSAGE(WM_SERIAL_RECEIVE, &CLightControllerDlg::OnSerialReceive)
	ON_MESSAGE(WM_AUTOFIND_LOG, &CLightControllerDlg::OnAutoFindLog)
	ON_MESSAGE(WM_AUTOFIND_DONE, &CLightControllerDlg::OnAutoFindDone)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// ============================================================
// Initialization
// ============================================================

BOOL CLightControllerDlg::OnInitDialog()
{
	CBCGPDialog::OnInitDialog();

	EnableVisualManagerStyle(TRUE, TRUE);

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// COM port enumeration
	EnumerateComPorts();

	// Serial receive callback
	m_serial.SetOwner(GetSafeHwnd());

	// Default radio: DS-16
	m_nCmdMode = 0;
	UpdateData(FALSE);

	// Spin controls range setup
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_MAXPAGE))->SetRange32(1, 99);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_MAXPAGE))->SetPos32(1);

	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_CURPAGE))->SetRange32(0, 98);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_CURPAGE))->SetPos32(0);

	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_REPEAT))->SetRange32(1, 9);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_REPEAT))->SetPos32(1);

	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_START_PAGE))->SetRange32(0, 99);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_START_PAGE))->SetPos32(0);

	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_SECTION_START))->SetRange32(0, 99);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_SECTION_START))->SetPos32(0);

	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_SECTION_END))->SetRange32(0, 99);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_SECTION_END))->SetPos32(0);

	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_SKIP_TIME))->SetRange32(0, 999);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_SKIP_TIME))->SetPos32(0);

	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_DEV_PAGE))->SetRange32(0, 98);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_DEV_PAGE))->SetPos32(0);

	// Initialize 16-channel ON TIME spins and multiplier combos
	InitChannelControls();

	// Update UI state
	UpdateUIState();

	return TRUE;
}

void CLightControllerDlg::InitChannelControls()
{
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		// ON TIME spin range: 0~999
		CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_ONTIME_BASE + i);
		if (pSpin)
		{
			pSpin->SetRange32(0, 999);
			pSpin->SetPos32(0);
		}

		// Multiplier combo: 1~5
		CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_CMB_MUL_BASE + i);
		if (pCombo)
		{
			pCombo->ResetContent();
			pCombo->AddString(_T("1"));
			pCombo->AddString(_T("2"));
			pCombo->AddString(_T("3"));
			pCombo->AddString(_T("4"));
			pCombo->AddString(_T("5"));
			pCombo->SetCurSel(0);
		}
	}

	// All OnTime spin (using edit directly)
	SetDlgItemInt(IDC_EDT_ALL_ONTIME, 0);

	// All Multiplier combo
	CComboBox* pAllMul = (CComboBox*)GetDlgItem(IDC_CMB_ALL_MUL);
	if (pAllMul)
	{
		pAllMul->ResetContent();
		pAllMul->AddString(_T("1"));
		pAllMul->AddString(_T("2"));
		pAllMul->AddString(_T("3"));
		pAllMul->AddString(_T("4"));
		pAllMul->AddString(_T("5"));
		pAllMul->SetCurSel(0);
	}
}

// ============================================================
// COM Port
// ============================================================

void CLightControllerDlg::EnumerateComPorts()
{
	m_cmbPort.ResetContent();

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		TCHAR szName[256], szValue[256];
		DWORD dwNameSize, dwValueSize, dwType;
		DWORD dwIndex = 0;

		while (TRUE)
		{
			dwNameSize = 256;
			dwValueSize = sizeof(szValue);
			if (RegEnumValue(hKey, dwIndex++, szName, &dwNameSize,
				NULL, &dwType, (LPBYTE)szValue, &dwValueSize) != ERROR_SUCCESS)
				break;

			m_cmbPort.AddString(szValue);
		}
		RegCloseKey(hKey);
	}

	if (m_cmbPort.GetCount() > 0)
		m_cmbPort.SetCurSel(0);
}

void CLightControllerDlg::OnBnClickedConnect()
{
	CString strPort;
	int nSel = m_cmbPort.GetCurSel();
	if (nSel == CB_ERR)
	{
		AfxMessageBox(_T("COM 포트를 선택하세요."));
		return;
	}

	m_cmbPort.GetLBText(nSel, strPort);

	if (m_serial.Open(strPort, 19200))
	{
		m_strConnPort = strPort;
		CString strMsg;
		strMsg.Format(_T("연결됨 - %s (19200,8,E,1)"), strPort);
		SetDlgItemText(IDC_STC_STATUS, strMsg);
		AddLog(_T("[SYS] ") + strMsg);
		UpdateUIState();
		UpdateStatusBar();
		ReadDeviceParams();
	}
	else
	{
		AfxMessageBox(_T("COM 포트를 열 수 없습니다.\n다른 프로그램에서 사용 중인지 확인하세요."));
	}
}

void CLightControllerDlg::OnBnClickedDisconnect()
{
	m_serial.Close();
	m_strConnPort.Empty();
	m_strLastReadTime.Empty();
	m_nReadState = 0;
	SetDlgItemText(IDC_STC_STATUS, _T("미연결"));
	AddLog(_T("[SYS] 연결 해제됨"));
	UpdateUIState();
	UpdateStatusBar();
}

void CLightControllerDlg::OnBnClickedRefreshPort()
{
	EnumerateComPorts();
}

// ============================================================
// Auto Find
// ============================================================

struct AutoFindParam
{
	HWND         hWndOwner;
	CStringArray arrPorts;
};

void CLightControllerDlg::OnBnClickedAutoFind()
{
	if (m_bAutoFinding)
		return;

	if (m_serial.IsOpen())
	{
		AfxMessageBox(_T("먼저 연결을 해제하세요."));
		return;
	}

	// 사용 가능한 포트 목록 수집
	AutoFindParam* pParam = new AutoFindParam;
	pParam->hWndOwner = GetSafeHwnd();

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		_T("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
		0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		TCHAR szName[256], szValue[256];
		DWORD dwNameSize, dwValueSize, dwType;
		DWORD dwIndex = 0;
		while (TRUE)
		{
			dwNameSize = 256;
			dwValueSize = sizeof(szValue);
			if (RegEnumValue(hKey, dwIndex++, szName, &dwNameSize,
				NULL, &dwType, (LPBYTE)szValue, &dwValueSize) != ERROR_SUCCESS)
				break;
			pParam->arrPorts.Add(szValue);
		}
		RegCloseKey(hKey);
	}

	if (pParam->arrPorts.IsEmpty())
	{
		AfxMessageBox(_T("사용 가능한 COM 포트가 없습니다."));
		delete pParam;
		return;
	}

	m_bAutoFinding = TRUE;
	GetDlgItem(IDC_BTN_AUTO_FIND)->EnableWindow(FALSE);
	AddLog(_T("[SYS] 장비 자동 검색 시작..."));

	CreateThread(NULL, 0, AutoFindThreadProc, pParam, 0, NULL);
}

DWORD WINAPI CLightControllerDlg::AutoFindThreadProc(LPVOID lpParam)
{
	AutoFindParam* pParam = (AutoFindParam*)lpParam;
	HWND hWnd = pParam->hWndOwner;
	CString strFoundPort;

	for (INT_PTR i = 0; i < pParam->arrPorts.GetSize(); i++)
	{
		CString strPort = pParam->arrPorts[i];

		// 로그 전송
		CString* pLog = new CString;
		pLog->Format(_T("[SYS] %s 검색 중..."), strPort);
		if (!::PostMessage(hWnd, WM_AUTOFIND_LOG, 0, (LPARAM)pLog))
			delete pLog;

		CSerialComm probe;
		if (!probe.OpenSync(strPort, 19200))
		{
			CString* pLog2 = new CString;
			pLog2->Format(_T("[SYS] %s 열기 실패 (사용 중)"), strPort);
			if (!::PostMessage(hWnd, WM_AUTOFIND_LOG, 0, (LPARAM)pLog2))
				delete pLog2;
			continue;
		}

		// 현재 페이지 확인 명령 전송 (:00G\r\n)
		CStringA strCmd(":00G\r\n");
		probe.Send((const BYTE*)(LPCSTR)strCmd, strCmd.GetLength());

		// 응답 대기 (500ms)
		BYTE buffer[64] = { 0 };
		int nRead = probe.ReadSync(buffer, sizeof(buffer), 500);

		BOOL bFound = FALSE;
		if (nRead > 0)
		{
			for (int j = 0; j < nRead; j++)
			{
				if (buffer[j] == 0x06)  // ACK
				{
					bFound = TRUE;
					break;
				}
				// 숫자 응답도 장비 발견으로 간주 (현재 페이지 번호가 올 수 있음)
				if (buffer[j] >= '0' && buffer[j] <= '9')
				{
					bFound = TRUE;
					break;
				}
			}
		}

		probe.Close();

		if (bFound)
		{
			strFoundPort = strPort;
			CString* pLog3 = new CString;
			pLog3->Format(_T("[SYS] %s 에서 IMS-DS-16 장비 발견!"), strPort);
			if (!::PostMessage(hWnd, WM_AUTOFIND_LOG, 0, (LPARAM)pLog3))
				delete pLog3;
			break;
		}
		else
		{
			CString* pLog4 = new CString;
			pLog4->Format(_T("[SYS] %s 응답 없음"), strPort);
			if (!::PostMessage(hWnd, WM_AUTOFIND_LOG, 0, (LPARAM)pLog4))
				delete pLog4;
		}
	}

	// 결과 전달: WPARAM=성공여부, LPARAM=포트이름 문자열
	CString* pResult = new CString(strFoundPort);
	if (!::PostMessage(hWnd, WM_AUTOFIND_DONE, strFoundPort.IsEmpty() ? 0 : 1, (LPARAM)pResult))
		delete pResult;

	delete pParam;
	return 0;
}

// ============================================================
// Send Commands
// ============================================================

void CLightControllerDlg::SendCommand(const CString& strCmd)
{
	if (!m_serial.IsOpen())
	{
		AfxMessageBox(_T("시리얼 포트가 연결되지 않았습니다."));
		return;
	}

	m_serial.Send(strCmd);

	// Log (remove trailing \r\n for display)
	CString strDisplay = strCmd;
	strDisplay.TrimRight(_T("\r\n"));
	AddLog(_T("[TX] ") + strDisplay);
}

void CLightControllerDlg::OnBnClickedSendAll()
{
	UpdateData(TRUE);

	int arrValue[NUM_CHANNELS];
	for (int i = 0; i < NUM_CHANNELS; i++)
		arrValue[i] = GetDlgItemInt(IDC_EDT_ONTIME_BASE + i);

	CString strCmd;
	if (m_nCmdMode == 0)  // DS-16
	{
		int nMaxPage = GetDlgItemInt(IDC_EDT_MAXPAGE);
		int nPage = GetDlgItemInt(IDC_EDT_CURPAGE);
		int nRepeat = GetDlgItemInt(IDC_EDT_REPEAT);
		strCmd = CProtocolBuilder::BuildSetOnTime(nMaxPage, nPage, nRepeat, arrValue);

		CString strVal;
		strVal.Format(_T("%d"), nMaxPage);
		SetDlgItemText(IDC_STC_NQ_MAXPAGE, strVal);
	}
	else  // LS-12
	{
		int nPage = GetDlgItemInt(IDC_EDT_CURPAGE);
		strCmd = CProtocolBuilder::BuildSetValue(nPage, arrValue);
	}

	SendCommand(strCmd);
}

void CLightControllerDlg::OnBnClickedSendMul()
{
	int arrMul[NUM_CHANNELS];
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_CMB_MUL_BASE + i);
		arrMul[i] = (pCombo && pCombo->GetCurSel() != CB_ERR) ? pCombo->GetCurSel() + 1 : 1;
	}

	SendCommand(CProtocolBuilder::BuildSetMultiplier(arrMul));
}

void CLightControllerDlg::OnBnClickedSaveData()
{
	SendCommand(CProtocolBuilder::BuildSaveData());
}

void CLightControllerDlg::OnBnClickedReadData()
{
	UpdateData(TRUE);
	if (m_nCmdMode == 0)
		SendCommand(CProtocolBuilder::BuildReadData());
	else
		SendCommand(CProtocolBuilder::BuildReadOntime());
}

void CLightControllerDlg::OnBnClickedReadMul()
{
	SendCommand(CProtocolBuilder::BuildReadMultiplier());
}

// ============================================================
// Page Control
// ============================================================

void CLightControllerDlg::OnBnClickedTrigger()
{
	SendCommand(CProtocolBuilder::BuildPageTrigger());
}

void CLightControllerDlg::OnBnClickedReset()
{
	SendCommand(CProtocolBuilder::BuildPageReset());
}

void CLightControllerDlg::OnBnClickedGetPage()
{
	SendCommand(CProtocolBuilder::BuildGetCurrentPage());
}

void CLightControllerDlg::OnBnClickedSetStartPage()
{
	int nPage = GetDlgItemInt(IDC_EDT_START_PAGE);
	SendCommand(CProtocolBuilder::BuildSetStartPage(nPage));

	CString strVal;
	strVal.Format(_T("%d"), nPage);
	SetDlgItemText(IDC_STC_NQ_START_PAGE, strVal);
}

void CLightControllerDlg::OnBnClickedSetHold()
{
	BOOL bHold = ((CButton*)GetDlgItem(IDC_CHK_HOLD))->GetCheck() == BST_CHECKED;
	SendCommand(CProtocolBuilder::BuildSetPageHold(bHold));

	SetDlgItemText(IDC_STC_NQ_HOLD, bHold ? _T("ON") : _T("OFF"));
}

void CLightControllerDlg::OnBnClickedSetSection()
{
	BOOL bSection = ((CButton*)GetDlgItem(IDC_CHK_SECTION))->GetCheck() == BST_CHECKED;
	SendCommand(CProtocolBuilder::BuildSetPageSection(bSection));

	int nStart = GetDlgItemInt(IDC_EDT_SECTION_START);
	int nEnd = GetDlgItemInt(IDC_EDT_SECTION_END);
	SendCommand(CProtocolBuilder::BuildSetSectionStartPage(nStart));
	SendCommand(CProtocolBuilder::BuildSetSectionEndPage(nEnd));

	SetDlgItemText(IDC_STC_NQ_SECTION, bSection ? _T("ON") : _T("OFF"));
	CString strRange;
	strRange.Format(_T("%d ~ %d"), nStart, nEnd);
	SetDlgItemText(IDC_STC_NQ_SECTION_RANGE, strRange);
}

void CLightControllerDlg::OnBnClickedSetSkip()
{
	BOOL bSkip = ((CButton*)GetDlgItem(IDC_CHK_SKIP))->GetCheck() == BST_CHECKED;
	SendCommand(CProtocolBuilder::BuildSetTriggerSkip(bSkip));

	int nTime = GetDlgItemInt(IDC_EDT_SKIP_TIME);
	SendCommand(CProtocolBuilder::BuildSetSkipTime(nTime));

	SetDlgItemText(IDC_STC_NQ_SKIP, bSkip ? _T("ON") : _T("OFF"));
	CString strTime;
	strTime.Format(_T("%d ms"), nTime);
	SetDlgItemText(IDC_STC_NQ_SKIP_TIME, strTime);
}

// ============================================================
// Batch Apply
// ============================================================

void CLightControllerDlg::OnBnClickedApplyAllOntime()
{
	int nValue = GetDlgItemInt(IDC_EDT_ALL_ONTIME);
	nValue = max(0, min(999, nValue));

	for (int i = 0; i < NUM_CHANNELS; i++)
		SetDlgItemInt(IDC_EDT_ONTIME_BASE + i, nValue);
}

void CLightControllerDlg::OnBnClickedApplyAllMul()
{
	CComboBox* pAllMul = (CComboBox*)GetDlgItem(IDC_CMB_ALL_MUL);
	int nSel = (pAllMul) ? pAllMul->GetCurSel() : 0;

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_CMB_MUL_BASE + i);
		if (pCombo)
			pCombo->SetCurSel(nSel);
	}
}

// ============================================================
// Serial Receive
// ============================================================

LRESULT CLightControllerDlg::OnSerialReceive(WPARAM wParam, LPARAM lParam)
{
	DWORD dwSize = (DWORD)wParam;
	BYTE* pData = (BYTE*)lParam;

	for (DWORD i = 0; i < dwSize; i++)
	{
		if (CProtocolBuilder::IsACK(pData[i]))
		{
			AddLog(_T("[RX] ACK (0x06) - 정상 수신"));
		}
		else if (CProtocolBuilder::IsNAK(pData[i]))
		{
			AddLog(_T("[RX] NAK (0x15) - 비정상 수신"));
			m_nReadState = 0;  // 오류 시 상태 초기화
		}
		else if (pData[i] == 0x0A)  // LF → 한 줄 완성
		{
			CString strLine = m_strRecvBuffer;
			strLine.Trim();
			m_strRecvBuffer.Empty();

			if (!strLine.IsEmpty())
			{
				AddLog(_T("[RX] ") + strLine);
				ProcessReceivedLine(strLine);
			}
		}
		else if (pData[i] == 0x0D)
		{
			// CR은 무시 (LF에서 처리)
		}
		else if (pData[i] >= 0x20 && pData[i] <= 0x7E)
		{
			m_strRecvBuffer += (TCHAR)pData[i];
		}
		else
		{
			CString strByte;
			strByte.Format(_T("[RX HEX] 0x%02X"), pData[i]);
			AddLog(strByte);
		}
	}

	delete[] pData;
	return 0;
}

// ============================================================
// Log
// ============================================================

void CLightControllerDlg::AddLog(const CString& strMsg)
{
	CTime time = CTime::GetCurrentTime();
	CString strTime = time.Format(_T("[%H:%M:%S] "));

	CString strLine = strTime + strMsg + _T("\r\n");

	int nLen = m_edtLog.GetWindowTextLength();
	m_edtLog.SetSel(nLen, nLen);
	m_edtLog.ReplaceSel(strLine);
}

void CLightControllerDlg::OnBnClickedDevReadPage()
{
	if (!m_serial.IsOpen())
	{
		AfxMessageBox(_T("시리얼 포트가 연결되지 않았습니다."));
		return;
	}
	if (m_nReadState != 0)
	{
		AfxMessageBox(_T("이전 읽기가 진행 중입니다."));
		return;
	}

	m_nReadTargetPage = GetDlgItemInt(IDC_EDT_DEV_PAGE);
	m_strRecvBuffer.Empty();
	m_nPageLineCount = 0;
	m_nReadState = 4;  // WAITING_PAGE_ONTIME

	CString strLog;
	strLog.Format(_T("[SYS] Page %d ON TIME 읽기 (:00R)..."), m_nReadTargetPage);
	AddLog(strLog);
	UpdateStatusBar();

	// :00R로 전체 페이지 데이터 요청, 응답에서 대상 페이지 라인만 파싱
	SendCommand(CProtocolBuilder::BuildReadData());
}

void CLightControllerDlg::OnBnClickedClearLog()
{
	m_edtLog.SetWindowText(_T(""));
}

// ============================================================
// UI State
// ============================================================

LRESULT CLightControllerDlg::OnAutoFindLog(WPARAM wParam, LPARAM lParam)
{
	CString* pMsg = (CString*)lParam;
	if (pMsg)
	{
		AddLog(*pMsg);
		delete pMsg;
	}
	return 0;
}

LRESULT CLightControllerDlg::OnAutoFindDone(WPARAM wParam, LPARAM lParam)
{
	CString* pResult = (CString*)lParam;
	CString strPort = pResult ? *pResult : _T("");
	delete pResult;

	m_bAutoFinding = FALSE;
	GetDlgItem(IDC_BTN_AUTO_FIND)->EnableWindow(TRUE);

	if (wParam && !strPort.IsEmpty())
	{
		// 포트 콤보에서 해당 포트 선택
		int nIdx = m_cmbPort.FindStringExact(-1, strPort);
		if (nIdx == CB_ERR)
		{
			EnumerateComPorts();
			nIdx = m_cmbPort.FindStringExact(-1, strPort);
		}
		if (nIdx != CB_ERR)
			m_cmbPort.SetCurSel(nIdx);

		// 자동 연결
		if (m_serial.Open(strPort, 19200))
		{
			m_serial.SetOwner(GetSafeHwnd());
			CString strMsg;
			strMsg.Format(_T("연결됨 - %s (19200,8,E,1)"), strPort);
			SetDlgItemText(IDC_STC_STATUS, strMsg);
			m_strConnPort = strPort;
			AddLog(_T("[SYS] 자동 연결 완료: ") + strPort);
			UpdateUIState();
			UpdateStatusBar();
			ReadDeviceParams();
		}
	}
	else
	{
		AddLog(_T("[SYS] 장비를 찾을 수 없습니다."));
		AfxMessageBox(_T("IMS-DS-16 장비를 찾을 수 없습니다.\n케이블 연결 및 전원을 확인하세요."));
	}

	return 0;
}

// ============================================================
// UI State
// ============================================================

void CLightControllerDlg::UpdateStatusBar()
{
	CString strStatus;

	if (m_serial.IsOpen())
	{
		int nPage = GetDlgItemInt(IDC_EDT_CURPAGE);
		CString strRead = m_strLastReadTime.IsEmpty() ? _T("-") : m_strLastReadTime;
		CString strState;
		if (m_nReadState == 1)
			strState = _T("ON TIME 읽는 중...");
		else if (m_nReadState == 2)
			strState = _T("배수 읽는 중...");
		else if (m_nReadState == 3)
			strState = _T("페이지 읽는 중...");
		else if (m_nReadState == 4)
			strState.Format(_T("Page %d 읽는 중..."), m_nReadTargetPage);
		else
			strState = _T("16CH OK");

		strStatus.Format(_T("  ■ 연결: %s  |  Page %d  |  읽기: %s  |  %s"),
			m_strConnPort, nPage, strRead, strState);
	}
	else
	{
		strStatus = _T("  □ 미연결");
	}

	SetDlgItemText(IDC_STC_STATUSBAR, strStatus);
}

void CLightControllerDlg::UpdateUIState()
{
	BOOL bConnected = m_serial.IsOpen();

	// Disable connect button when connected, vice versa
	GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(!bConnected);
	GetDlgItem(IDC_BTN_DISCONNECT)->EnableWindow(bConnected);
	GetDlgItem(IDC_CMB_PORT)->EnableWindow(!bConnected);

	// Enable command buttons only when connected
	GetDlgItem(IDC_BTN_SEND_ALL)->EnableWindow(bConnected);
	GetDlgItem(IDC_BTN_SEND_MUL)->EnableWindow(bConnected);
	GetDlgItem(IDC_BTN_SAVE_DATA)->EnableWindow(bConnected);
	//GetDlgItem(IDC_BTN_READ_DATA)->EnableWindow(bConnected);
	//GetDlgItem(IDC_BTN_READ_MUL)->EnableWindow(bConnected);
	GetDlgItem(IDC_BTN_TRIGGER)->EnableWindow(bConnected);
	GetDlgItem(IDC_BTN_RESET)->EnableWindow(bConnected);
	GetDlgItem(IDC_BTN_GET_PAGE)->EnableWindow(bConnected);
	GetDlgItem(IDC_BTN_SET_START_PAGE)->EnableWindow(bConnected);
	GetDlgItem(IDC_BTN_SET_HOLD)->EnableWindow(bConnected);
	GetDlgItem(IDC_BTN_SET_SECTION)->EnableWindow(bConnected);
	GetDlgItem(IDC_BTN_SET_SKIP)->EnableWindow(bConnected);
	GetDlgItem(IDC_BTN_AUTO_FIND)->EnableWindow(!bConnected && !m_bAutoFinding);
}

// ============================================================
// Device Parameter Read (연결 후 자동 읽기)
// ============================================================

void CLightControllerDlg::ReadDeviceParams()
{
	if (!m_serial.IsOpen())
		return;

	m_strRecvBuffer.Empty();
	m_nReadState = 1;  // WAITING_ONTIME
	AddLog(_T("[SYS] 장비 파라미터 읽기 시작..."));
	UpdateStatusBar();
	SendCommand(CProtocolBuilder::BuildReadOntime());   // :1RA\r\n
}

void CLightControllerDlg::ProcessReceivedLine(const CString& strLine)
{
	if (m_nReadState == 1)  // WAITING_ONTIME 응답
	{
		int arrValue[NUM_CHANNELS] = { 0 };
		if (CProtocolBuilder::ParseOntimeResponse(strLine, arrValue))
		{
			// 입력 UI에 반영
			for (int i = 0; i < NUM_CHANNELS; i++)
				SetDlgItemInt(IDC_EDT_ONTIME_BASE + i, arrValue[i]);

			// 장비값 읽기전용 영역에 반영
			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				CString strVal;
				strVal.Format(_T("%d"), arrValue[i]);
				SetDlgItemText(IDC_STC_DEV_ONTIME_BASE + i, strVal);
			}

			AddLog(_T("[SYS] ON TIME 값 읽기 완료"));

			// 다음: 배수 읽기
			m_nReadState = 2;  // WAITING_MULTIPLIER
			UpdateStatusBar();
			SendCommand(CProtocolBuilder::BuildReadMultiplier());  // :1RU\r\n
		}
		// 숫자가 아닌 라인은 무시 (헤더 등), 계속 대기
	}
	else if (m_nReadState == 2)  // WAITING_MULTIPLIER 응답
	{
		int arrMul[NUM_CHANNELS];
		if (CProtocolBuilder::ParseMultiplierResponse(strLine, arrMul))
		{
			// 입력 UI에 반영
			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_CMB_MUL_BASE + i);
				if (pCombo)
				{
					int nSel = max(0, min(4, arrMul[i] - 1));
					pCombo->SetCurSel(nSel);
				}
			}

			// 장비값 읽기전용 영역에 반영
			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				CString strVal;
				strVal.Format(_T("x%d"), arrMul[i]);
				SetDlgItemText(IDC_STC_DEV_MUL_BASE + i, strVal);
			}

			AddLog(_T("[SYS] 배수 값 읽기 완료"));

			// 다음: 현재 페이지 읽기
			m_nReadState = 3;  // WAITING_PAGE
			UpdateStatusBar();
			SendCommand(CProtocolBuilder::BuildGetCurrentPage());  // :00G\r\n
		}
	}
	else if (m_nReadState == 3)  // WAITING_PAGE 응답
	{
		// 응답에서 숫자 추출 (현재 페이지 번호)
		CString strNum;
		for (int i = 0; i < strLine.GetLength(); i++)
		{
			TCHAR ch = strLine[i];
			if (ch >= '0' && ch <= '9')
				strNum += ch;
		}
		if (!strNum.IsEmpty())
		{
			int nPage = _ttoi(strNum);
			CString strVal;

			// 현재 Page 표시
			strVal.Format(_T("%d"), nPage);
			SetDlgItemText(IDC_STC_DEV_CURPAGE, strVal);

			// Max Page → 조회불가 영역에 표시
			int nMaxPage = GetDlgItemInt(IDC_EDT_MAXPAGE);
			strVal.Format(_T("%d"), nMaxPage);
			SetDlgItemText(IDC_STC_NQ_MAXPAGE, strVal);

			AddLog(_T("[SYS] 페이지 정보 읽기 완료"));
			m_nReadState = 0;  // IDLE
			m_strLastReadTime = CTime::GetCurrentTime().Format(_T("%H:%M:%S"));
			AddLog(_T("[SYS] 장비 파라미터 읽기 완료"));
			UpdateStatusBar();
		}
	}
	else if (m_nReadState == 4)  // WAITING_PAGE_ONTIME — :00R 응답 (여러 줄)
	{
		// :00R 응답은 페이지 0부터 순서대로 한 줄씩 ON TIME 데이터가 옴
		// 대상 페이지 라인만 파싱하여 장비값 영역에 표시
		int arrValue[NUM_CHANNELS] = { 0 };
		if (CProtocolBuilder::ParseOntimeResponse(strLine, arrValue))
		{
			if (m_nPageLineCount == m_nReadTargetPage)
			{
				// 대상 페이지 → 장비값 영역에 반영
				for (int i = 0; i < NUM_CHANNELS; i++)
				{
					CString strVal;
					strVal.Format(_T("%d"), arrValue[i]);
					SetDlgItemText(IDC_STC_DEV_ONTIME_BASE + i, strVal);
				}
				// 편집 UI에도 반영
				for (int i = 0; i < NUM_CHANNELS; i++)
					SetDlgItemInt(IDC_EDT_ONTIME_BASE + i, arrValue[i]);

				CString strLog;
				strLog.Format(_T("[SYS] Page %d ON TIME 읽기 완료"), m_nReadTargetPage);
				AddLog(strLog);

				m_nReadState = 0;  // IDLE
				m_strLastReadTime = CTime::GetCurrentTime().Format(_T("%H:%M:%S"));
				UpdateStatusBar();
			}
			m_nPageLineCount++;
		}
	}
	// m_nReadState == 0 (IDLE) → 일반 데이터, 로그에만 표시
}

void CLightControllerDlg::OnDestroy()
{
	m_serial.Close();
	CBCGPDialog::OnDestroy();
}
