#include "pch.h"
#include "SerialComm.h"

CSerialComm::CSerialComm()
	: m_hComm(INVALID_HANDLE_VALUE)
	, m_hThread(NULL)
	, m_bRunning(FALSE)
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

	m_hComm = CreateFile(
		strDevice,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (m_hComm == INVALID_HANDLE_VALUE)
		return FALSE;

	ConfigurePort(dwBaudRate);

	// Start read thread
	m_bRunning = TRUE;
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
	m_bRunning = FALSE;

	if (m_hThread != NULL)
	{
		// Cancel any pending I/O
		if (m_hComm != INVALID_HANDLE_VALUE)
			CancelIoEx(m_hComm, NULL);

		WaitForSingleObject(m_hThread, 2000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
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

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 50;
	timeouts.ReadTotalTimeoutMultiplier  = 10;
	timeouts.ReadTotalTimeoutConstant    = 100;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant   = 100;
	SetCommTimeouts(m_hComm, &timeouts);

	PurgeComm(m_hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);
}

BOOL CSerialComm::Send(const BYTE* pData, int nLength)
{
	if (m_hComm == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwWritten = 0;
	return WriteFile(m_hComm, pData, nLength, &dwWritten, NULL);
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

DWORD WINAPI CSerialComm::ReadThread(LPVOID lpParam)
{
	CSerialComm* pThis = (CSerialComm*)lpParam;
	BYTE buffer[256];
	DWORD dwRead;

	while (pThis->m_bRunning)
	{
		if (ReadFile(pThis->m_hComm, buffer, sizeof(buffer), &dwRead, NULL))
		{
			if (dwRead > 0 && pThis->m_hOwner != NULL)
			{
				BYTE* pData = new BYTE[dwRead];
				memcpy(pData, buffer, dwRead);
				PostMessage(pThis->m_hOwner, pThis->m_nMsgID,
					(WPARAM)dwRead, (LPARAM)pData);
			}
		}
		else
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_OPERATION_ABORTED)
				break;
			Sleep(10);
		}
	}

	return 0;
}

// ============================================================
// 동기식 메서드 (자동 찾기용)
// ============================================================

BOOL CSerialComm::OpenSync(const CString& strPort, DWORD dwBaudRate)
{
	if (m_hComm != INVALID_HANDLE_VALUE)
		Close();

	CString strDevice;
	strDevice.Format(_T("\\\\.\\%s"), strPort);

	m_hComm = CreateFile(
		strDevice,
		GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL);

	if (m_hComm == INVALID_HANDLE_VALUE)
		return FALSE;

	ConfigurePort(dwBaudRate);
	// 수신 스레드를 시작하지 않음
	return TRUE;
}

int CSerialComm::ReadSync(BYTE* pBuffer, int nMaxLen, DWORD dwTimeoutMs)
{
	if (m_hComm == INVALID_HANDLE_VALUE)
		return -1;

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 50;
	timeouts.ReadTotalTimeoutMultiplier  = 0;
	timeouts.ReadTotalTimeoutConstant    = dwTimeoutMs;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant   = 100;
	SetCommTimeouts(m_hComm, &timeouts);

	DWORD dwRead = 0;
	if (ReadFile(m_hComm, pBuffer, nMaxLen, &dwRead, NULL))
		return (int)dwRead;

	return -1;
}
