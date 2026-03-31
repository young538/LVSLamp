#pragma once

#define WM_SERIAL_RECEIVE  (WM_USER + 100)

class CSerialComm
{
public:
	CSerialComm();
	~CSerialComm();

	BOOL Open(const CString& strPort, DWORD dwBaudRate = 19200);
	void Close();
	BOOL IsOpen() const;

	BOOL Send(const BYTE* pData, int nLength);
	BOOL Send(const CString& strData);

	void SetOwner(HWND hWnd, UINT nMsgID = WM_SERIAL_RECEIVE);

	// 동기식 메서드 (자동 찾기용 — 수신 스레드 없이 동작)
	BOOL OpenSync(const CString& strPort, DWORD dwBaudRate = 19200);
	int  ReadSync(BYTE* pBuffer, int nMaxLen, DWORD dwTimeoutMs = 500);

private:
	HANDLE  m_hComm;
	HANDLE  m_hThread;
	BOOL    m_bRunning;
	HWND    m_hOwner;
	UINT    m_nMsgID;

	void ConfigurePort(DWORD dwBaudRate);
	static DWORD WINAPI ReadThread(LPVOID lpParam);
};
