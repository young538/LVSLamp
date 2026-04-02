#include "pch.h"
#include "SerialComm.h"

CSerialComm::CSerialComm()
	: m_hComm(INVALID_HANDLE_VALUE)
	, m_hThread(NULL)
	, m_hStopEvent(NULL)
	, m_hOwner(NULL)
	, m_nMsgID(WM_SERIAL_RECEIVE)
{
}

CSerialComm::~CSerialComm()
{
	Close();
}

BOOL CSerialComm::Open(const CString& strPort, DWORD dwBaudRate)
{
	if (m_hComm != INVALID_HANDLE_VALUE)
		Close();

	CString strDevice;
	strDevice.Format(_T("\\\\.\\%s"), strPort);

	// FILE_FLAG_OVERLAPPED로 비동기 I/O 모드로 열기
	m_hComm = CreateFile(
		strDevice,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (m_hComm == INVALID_HANDLE_VALUE)
		return FALSE;

	ConfigurePort(dwBaudRate);

	// 스레드 종료 이벤트
	m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// 수신 스레드 시작
	DWORD dwThreadId;
	m_hThread = CreateThread(NULL, 0, ReadThread, this, 0, &dwThreadId);
	if (m_hThread == NULL)
	{
		Close();
		return FALSE;
	}

	return TRUE;
}

void CSerialComm::Close()
{
	// 스레드 종료 신호
	if (m_hStopEvent != NULL)
		SetEvent(m_hStopEvent);

	if (m_hThread != NULL)
	{
		WaitForSingleObject(m_hThread, 2000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	if (m_hStopEvent != NULL)
	{
		CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}

	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

BOOL CSerialComm::IsOpen() const
{
	return (m_hComm != INVALID_HANDLE_VALUE);
}

void CSerialComm::ConfigurePort(DWORD dwBaudRate)
{
	DCB dcb = { 0 };
	dcb.DCBlength = sizeof(DCB);
	GetCommState(m_hComm, &dcb);

	dcb.BaudRate = dwBaudRate;
	dcb.ByteSize = 8;
	dcb.Parity   = EVENPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fBinary  = TRUE;
	dcb.fParity  = TRUE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl  = DTR_CONTROL_ENABLE;
	dcb.fRtsControl  = RTS_CONTROL_ENABLE;
	dcb.fOutX = FALSE;
	dcb.fInX  = FALSE;

	SetCommState(m_hComm, &dcb);

	// Overlapped 모드에서는 타임아웃을 0으로 설정 (이벤트 기반)
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier  = 0;
	timeouts.ReadTotalTimeoutConstant    = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant   = 1000;
	SetCommTimeouts(m_hComm, &timeouts);

	// 수신 이벤트 마스크 설정
	SetCommMask(m_hComm, EV_RXCHAR);

	PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);
}

BOOL CSerialComm::Send(const BYTE* pData, int nLength)
{
	if (m_hComm == INVALID_HANDLE_VALUE)
		return FALSE;

	OVERLAPPED ov = { 0 };
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	DWORD dwWritten = 0;
	BOOL bResult = WriteFile(m_hComm, pData, nLength, &dwWritten, &ov);

	if (!bResult && GetLastError() == ERROR_IO_PENDING)
		bResult = GetOverlappedResult(m_hComm, &ov, &dwWritten, TRUE);

	CloseHandle(ov.hEvent);
	return bResult;
}

BOOL CSerialComm::Send(const CString& strData)
{
	CStringA strAnsi(strData);
	return Send((const BYTE*)(LPCSTR)strAnsi, strAnsi.GetLength());
}

void CSerialComm::SetOwner(HWND hWnd, UINT nMsgID)
{
	m_hOwner = hWnd;
	m_nMsgID = nMsgID;
}

// ============================================================
// Overlapped I/O 수신 스레드
// — WaitCommEvent로 데이터 도착을 감지, 즉시 읽기
// — m_hStopEvent 신호로 깔끔하게 종료
// ============================================================

DWORD WINAPI CSerialComm::ReadThread(LPVOID lpParam)
{
	CSerialComm* pThis = (CSerialComm*)lpParam;

	OVERLAPPED ovWait = { 0 };
	ovWait.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	OVERLAPPED ovRead = { 0 };
	ovRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE hEvents[2] = { pThis->m_hStopEvent, ovWait.hEvent };

	while (TRUE)
	{
		DWORD dwEvtMask = 0;
		ResetEvent(ovWait.hEvent);

		// 비동기 COM 이벤트 대기 (데이터 수신 시 이벤트 발생)
		if (!WaitCommEvent(pThis->m_hComm, &dwEvtMask, &ovWait))
		{
			if (GetLastError() != ERROR_IO_PENDING)
				break;
		}

		// 종료 이벤트 또는 COM 이벤트 대기
		DWORD dwWait = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

		if (dwWait == WAIT_OBJECT_0)  // m_hStopEvent → 종료
			break;

		if (dwWait != WAIT_OBJECT_0 + 1)  // 오류
			break;

		// COM 이벤트 완료 확인
		DWORD dwTemp;
		GetOverlappedResult(pThis->m_hComm, &ovWait, &dwTemp, FALSE);

		// 수신 데이터 읽기 (있는 만큼 즉시 읽기)
		BYTE buffer[512];
		DWORD dwRead = 0;
		ResetEvent(ovRead.hEvent);

		if (ReadFile(pThis->m_hComm, buffer, sizeof(buffer), &dwRead, &ovRead))
		{
			// 즉시 완료
		}
		else if (GetLastError() == ERROR_IO_PENDING)
		{
			GetOverlappedResult(pThis->m_hComm, &ovRead, &dwRead, TRUE);
		}

		if (dwRead > 0 && pThis->m_hOwner != NULL)
		{
			BYTE* pData = new BYTE[dwRead];
			memcpy(pData, buffer, dwRead);
			if (!::PostMessage(pThis->m_hOwner, pThis->m_nMsgID,
				(WPARAM)dwRead, (LPARAM)pData))
			{
				delete[] pData;  // PostMessage 실패 시 메모리 해제
			}
		}
	}

	CloseHandle(ovWait.hEvent);
	CloseHandle(ovRead.hEvent);
	return 0;
}

// ============================================================
// 동기식 메서드 (자동 찾기용 — Overlapped 사용 안 함)
// ============================================================

BOOL CSerialComm::OpenSync(const CString& strPort, DWORD dwBaudRate)
{
	if (m_hComm != INVALID_HANDLE_VALUE)
		Close();

	CString strDevice;
	strDevice.Format(_T("\\\\.\\%s"), strPort);

	// 동기식: FILE_FLAG_OVERLAPPED 없이 열기
	m_hComm = CreateFile(
		strDevice,
		GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL);

	if (m_hComm == INVALID_HANDLE_VALUE)
		return FALSE;

	// 동기식용 타임아웃 재설정
	DCB dcb = { 0 };
	dcb.DCBlength = sizeof(DCB);
	GetCommState(m_hComm, &dcb);
	dcb.BaudRate = dwBaudRate;
	dcb.ByteSize = 8;
	dcb.Parity   = EVENPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fBinary  = TRUE;
	dcb.fParity  = TRUE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl  = DTR_CONTROL_ENABLE;
	dcb.fRtsControl  = RTS_CONTROL_ENABLE;
	dcb.fOutX = FALSE;
	dcb.fInX  = FALSE;
	SetCommState(m_hComm, &dcb);

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 10;
	timeouts.ReadTotalTimeoutMultiplier  = 0;
	timeouts.ReadTotalTimeoutConstant    = 500;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant   = 100;
	SetCommTimeouts(m_hComm, &timeouts);

	PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);
	return TRUE;
}

int CSerialComm::ReadSync(BYTE* pBuffer, int nMaxLen, DWORD dwTimeoutMs)
{
	if (m_hComm == INVALID_HANDLE_VALUE)
		return -1;

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 10;
	timeouts.ReadTotalTimeoutMultiplier  = 0;
	timeouts.ReadTotalTimeoutConstant    = dwTimeoutMs;
	SetCommTimeouts(m_hComm, &timeouts);

	DWORD dwRead = 0;
	if (ReadFile(m_hComm, pBuffer, nMaxLen, &dwRead, NULL))
		return (int)dwRead;

	return -1;
}
