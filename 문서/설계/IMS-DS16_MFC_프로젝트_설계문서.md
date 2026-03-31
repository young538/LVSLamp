# IMS-DS-16 조명 컨트롤러 시리얼통신 MFC 프로그램 설계 문서

## 1. 프로젝트 개요

IMS-DS-16 (16채널 스트로브 컨트롤러)을 RS232 시리얼 통신으로 제어하는 Windows MFC 데스크톱 애플리케이션을 개발한다.

### 1.1 대상 장비 정보
- **장비명**: IMS-DS-16 (주식회사 LVS)
- **기능**: 16채널 스트로브 조명 컨트롤러 (FPGA 기반)
- **최대 페이지**: 99페이지, 각 페이지당 16채널
- **ON TIME 범위**: 0~999us (배수 적용 시 최대 4,995us)
- **트리거 응답 속도**: 16us 이내

---

## 2. 통신 사양

| 항목 | 설정값 |
|------|--------|
| 통신 방식 | RS232C |
| Baud Rate | 19,200 bps |
| Data Bit | 8 bit |
| Parity | **Even** |
| Stop Bit | 1 |

### 2.1 프로토콜 공통 규칙
- **시작 코드**: `:` (0x3A)
- **종료 코드**: CR LF (0x0D 0x0A)
- **정상 응답**: ACK (0x06)
- **비정상 응답**: NAK (0x15)

---

## 3. 통신 프로토콜 명령어 정리

### 3.1 IMS-LS12 호환 명령어 (국번 '1' 사용)

#### A. ON TIME 값 설정 (Value 값 조정)
```
:1[PAGE][CH01_VALUE],[CH02_VALUE],...,[CH16_VALUE][CR][LF]
```
- PAGE: 0~9 (1자리)
- 각 채널 VALUE: 000~999 (3자리, us 단위)
- 예시: `:10255,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000\r\n`
  → 0번 페이지, 1CH=255us, 나머지=000us

#### B. 배수 값 설정 (출력 시간 증폭)
```
:1X[CH01_MUL],[CH02_MUL],...,[CH16_MUL][CR][LF]
```
- 각 채널 배수: 1~5
- 예시: `:1X5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1\r\n`
  → 1CH 5배, 나머지 1배

#### C. 배수 데이터 확인
```
:1RU[CR][LF]
```

#### D. 출력 Ontime 설정 확인
```
:1RA[CR][LF]
```

#### E. Max Page 설정
```
:1P[MAX_PAGE][CR][LF]
```
- MAX_PAGE: 01~32 (2자리)
- 예시: `:1P15\r\n` → Max Page를 15로 설정

---

### 3.2 DS-16 전용 명령어 (ADDR '00' 사용)

#### A. ON TIME 조정 (확장)
```
:00B[MAX_PAGE][PAGE][RP][CH01_VALUE],[CH02_VALUE],...,[CH16_VALUE][CR][LF]
```
- MAX_PAGE: 01~32 (2자리, 최대 99)
- PAGE: 00~98 (2자리)
- RP: 스트로브 반복 횟수 (1자리)
- 각 채널 VALUE: 000~999 (3자리)
- 예시: `:00B01002999,999,999,999,999,999,999,999,999,999,999,999,999,999,999,999\r\n`
  → 최대페이지 1, 0번페이지, 반복 2회, 전 채널 999us

#### B. 데이터 확인
```
:00R[CR][LF]
```

#### C. 데이터 저장 (플래시 메모리)
```
:00S[CR][LF]
```

#### D. 순차적 페이지 트리거 / 리셋
```
:00TP[P or S][CR][LF]
```
- `P`: 페이지 트리거 발생 (순차 출력)
- `S`: 리셋 (첫번째 페이지로 복귀)

#### E. 순차적 페이지 시작 페이지 설정
```
:00P[PAGE][CR][LF]
```
- PAGE: 00~99 (2자리)
- 예시: `:00P15\r\n` → 시작 페이지를 15로 설정

#### F. 현재 페이지 확인
```
:00G[CR][LF]
```

#### G. 순차적 페이지 HOLD
```
:00H[0 or 1][CR][LF]
```
- `1`: Hold ON, `0`: Hold OFF

#### H. Page Section ON/OFF
```
:00CO[0 or 1][CR][LF]
```
- `1`: ON, `0`: OFF

#### I. Page Section 첫번째 페이지 설정
```
:00CPS[PAGE][CR][LF]
```
- PAGE: 00~99 (2자리)

#### J. Page Section 마지막 페이지 설정
```
:00CPE[PAGE][CR][LF]
```
- PAGE: 00~99 (2자리)

#### K. 트리거 Skip ON/OFF
```
:00IO[0 or 1][CR][LF]
```
- `1`: ON, `0`: OFF

#### L. 트리거 Skip Time 설정
```
:00IT[TIME][CR][LF]
```
- TIME: 000~999 (3자리, ms 단위)

---

## 4. Visual Studio MFC 프로젝트 생성 절차

### 4.1 개발 환경
- **IDE**: Visual Studio 2019 / 2022
- **프로젝트 유형**: MFC 애플리케이션 (대화 상자 기반)
- **빌드 시스템**: Visual Studio 기본 솔루션 (.sln / .vcxproj)
- **문자 세트**: 유니코드
- **플랫폼**: x86 (32bit) 또는 x64

### 4.2 프로젝트 생성 단계

1. Visual Studio 실행 → **새 프로젝트 만들기**
2. 템플릿 검색: **MFC 앱** 선택
3. 프로젝트 이름: `LightController` (또는 원하는 이름)
4. 위치: `D:\WorkSpace\006. 조명시리얼통신\`
5. MFC 애플리케이션 마법사 설정:
   - **애플리케이션 종류**: 대화 상자 기반
   - **MFC 사용**: 공유 DLL에서 MFC 사용
   - **고급 기능**: ActiveX 컨트롤 체크 해제
6. **마침** 클릭하여 프로젝트 생성

### 4.3 MFC 구성 요소가 설치되지 않은 경우
Visual Studio Installer에서 다음을 설치:
- **C++를 사용한 데스크톱 개발** 워크로드
- 개별 구성 요소에서 **최신 v14x 빌드 도구용 C++ MFC** 선택

---

## 5. 프로그램 구조 설계

### 5.1 클래스 구성

```
LightController/
├── LightControllerApp        (CWinApp 파생 - 앱 클래스)
├── CLightControllerDlg       (CDialogEx 파생 - 메인 대화상자)
├── CSerialComm               (시리얼 통신 관리 클래스)
└── CProtocolBuilder           (IMS-DS-16 프로토콜 생성/파싱 클래스)
```

### 5.2 CSerialComm 클래스 (시리얼 통신)

Win32 API 기반 시리얼 통신 클래스. MFC CWinThread 또는 별도 스레드로 수신 처리.

```cpp
class CSerialComm
{
public:
    BOOL Open(CString strPort, DWORD dwBaudRate = 19200);
    void Close();
    BOOL IsOpen() const;

    BOOL Send(const CString& strData);
    BOOL Send(const BYTE* pData, int nLength);

    // 수신 콜백 (메인 다이얼로그에 WM_USER 메시지로 전달)
    void SetOwner(HWND hWnd, UINT nMsgID);

private:
    HANDLE m_hComm;          // COM 포트 핸들
    HANDLE m_hThread;         // 수신 스레드 핸들
    BOOL   m_bRunning;        // 스레드 실행 플래그
    HWND   m_hOwner;          // 수신 데이터 전달 대상 윈도우
    UINT   m_nMsgID;          // 사용자 정의 메시지 ID

    static DWORD WINAPI ReadThread(LPVOID lpParam);
    void ConfigurePort();     // DCB 설정 (19200, 8, Even, 1)
};
```

#### 포트 설정 핵심 코드
```cpp
void CSerialComm::ConfigurePort()
{
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(DCB);
    GetCommState(m_hComm, &dcb);

    dcb.BaudRate = CBR_19200;
    dcb.ByteSize = 8;
    dcb.Parity   = EVENPARITY;    // Even 패리티 필수!
    dcb.StopBits = ONESTOPBIT;

    dcb.fBinary  = TRUE;
    dcb.fParity  = TRUE;

    SetCommState(m_hComm, &dcb);

    // 타임아웃 설정
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout         = 50;
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    timeouts.ReadTotalTimeoutConstant    = 100;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant   = 100;
    SetCommTimeouts(m_hComm, &timeouts);
}
```

### 5.3 CProtocolBuilder 클래스 (프로토콜 생성)

```cpp
class CProtocolBuilder
{
public:
    // === IMS-LS12 호환 명령어 (국번 '1') ===
    // ON TIME 값 설정
    static CString BuildSetValue(int nPage, int arrValue[16]);
    // 배수 값 설정
    static CString BuildSetMultiplier(int arrMul[16]);
    // 배수 데이터 확인
    static CString BuildReadMultiplier();
    // Ontime 설정 확인
    static CString BuildReadOntime();
    // Max Page 설정
    static CString BuildSetMaxPage_LS12(int nMaxPage);

    // === DS-16 전용 명령어 (ADDR '00') ===
    // ON TIME 조정 (확장)
    static CString BuildSetOnTime(int nMaxPage, int nPage, int nRepeat, int arrValue[16]);
    // 데이터 확인
    static CString BuildReadData();
    // 데이터 저장
    static CString BuildSaveData();
    // 순차적 페이지 트리거
    static CString BuildPageTrigger();
    // 순차적 페이지 리셋
    static CString BuildPageReset();
    // 시작 페이지 설정
    static CString BuildSetStartPage(int nPage);
    // 현재 페이지 확인
    static CString BuildGetCurrentPage();
    // 페이지 HOLD
    static CString BuildSetPageHold(BOOL bOn);
    // Page Section ON/OFF
    static CString BuildSetPageSection(BOOL bOn);
    // Page Section 첫번째 페이지
    static CString BuildSetSectionStartPage(int nPage);
    // Page Section 마지막 페이지
    static CString BuildSetSectionEndPage(int nPage);
    // 트리거 Skip ON/OFF
    static CString BuildSetTriggerSkip(BOOL bOn);
    // 트리거 Skip Time 설정
    static CString BuildSetSkipTime(int nTimeMs);

    // === 응답 파싱 ===
    static BOOL IsACK(BYTE bData);   // 0x06 확인
    static BOOL IsNAK(BYTE bData);   // 0x15 확인
};
```

#### 프로토콜 생성 예시
```cpp
// DS-16 ON TIME 설정
CString CProtocolBuilder::BuildSetOnTime(int nMaxPage, int nPage, int nRepeat, int arrValue[16])
{
    CString strCmd;
    strCmd.Format(_T(":00B%02d%02d%d"), nMaxPage, nPage, nRepeat);

    for (int i = 0; i < 16; i++)
    {
        CString strVal;
        strVal.Format(_T("%s%03d"), (i == 0) ? _T("") : _T(","), arrValue[i]);
        strCmd += strVal;
    }
    strCmd += _T("\r\n");
    return strCmd;
}

// IMS-LS12 Value 설정
CString CProtocolBuilder::BuildSetValue(int nPage, int arrValue[16])
{
    CString strCmd;
    strCmd.Format(_T(":1%d"), nPage);

    for (int i = 0; i < 16; i++)
    {
        CString strVal;
        strVal.Format(_T("%s%03d"), (i == 0) ? _T("") : _T(","), arrValue[i]);
        strCmd += strVal;
    }
    strCmd += _T("\r\n");
    return strCmd;
}
```

---

## 6. UI 설계

### 6.1 메인 대화상자 레이아웃

```
┌─────────────────────────────────────────────────────────┐
│  IMS-DS-16 조명 컨트롤러                           [_][X]│
├─────────────────────────────────────────────────────────┤
│ ┌── 시리얼 포트 ────────────────────────────────────┐   │
│ │ COM 포트: [COM1 ▼]  [연결]  [해제]   ● 연결상태  │   │
│ └───────────────────────────────────────────────────┘   │
│                                                         │
│ ┌── 페이지/채널 설정 ──────────────────────────────┐   │
│ │ 명령모드: (●)DS-16 확장  ( )LS-12 호환            │   │
│ │                                                   │   │
│ │ Max Page: [01 ▲▼]  현재 Page: [00 ▲▼]           │   │
│ │ 반복 횟수: [1 ▲▼]                                │   │
│ │                                                   │   │
│ │  CH  │ ON TIME(us) │ 배수  │                      │   │
│ │──────┼─────────────┼───────│                      │   │
│ │ CH01 │ [  000  ▲▼] │ [1 ▼] │                      │   │
│ │ CH02 │ [  000  ▲▼] │ [1 ▼] │                      │   │
│ │  ... │     ...     │  ...  │                      │   │
│ │ CH16 │ [  000  ▲▼] │ [1 ▼] │                      │   │
│ │                                                   │   │
│ │ [전체 전송] [데이터 저장] [데이터 확인]           │   │
│ └───────────────────────────────────────────────────┘   │
│                                                         │
│ ┌── 순차 페이지 제어 ─────────────────────────────┐   │
│ │ [트리거] [리셋] [현재페이지확인]                  │   │
│ │ 시작페이지: [00 ▲▼]  HOLD: [□]                   │   │
│ │ Section: [□]  시작: [00 ▲▼]  끝: [00 ▲▼]        │   │
│ └───────────────────────────────────────────────────┘   │
│                                                         │
│ ┌── 트리거 Skip ──────────────────────────────────┐   │
│ │ Skip: [□]  Skip Time: [000 ▲▼] ms               │   │
│ └───────────────────────────────────────────────────┘   │
│                                                         │
│ ┌── 통신 로그 ────────────────────────────────────┐   │
│ │ [TX] :00B01002999,999,...,999\r\n                 │   │
│ │ [RX] ACK (0x06)                                   │   │
│ │                                          [지우기] │   │
│ └───────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

### 6.2 주요 UI 컨트롤 목록

| 컨트롤 | 타입 | 변수명 | 설명 |
|--------|------|--------|------|
| COM 포트 선택 | CComboBox | m_cmbPort | 사용 가능한 COM 포트 목록 |
| 연결 버튼 | CButton | - | 시리얼 포트 연결 |
| 해제 버튼 | CButton | - | 시리얼 포트 해제 |
| 연결 상태 | CStatic | m_stcStatus | 연결/해제 상태 표시 |
| 명령 모드 라디오 | CButton (Radio) | m_nCmdMode | DS-16 확장 / LS-12 호환 |
| Max Page 스핀 | CSpinButtonCtrl + CEdit | m_nMaxPage | 1~99 |
| 현재 Page 스핀 | CSpinButtonCtrl + CEdit | m_nCurrentPage | 0~98 |
| 반복 횟수 스핀 | CSpinButtonCtrl + CEdit | m_nRepeat | 1~9 |
| CH01~16 ON TIME | CEdit + CSpinButtonCtrl | m_nOnTime[16] | 0~999 |
| CH01~16 배수 | CComboBox | m_nMultiplier[16] | 1~5 |
| 통신 로그 | CListBox 또는 CEdit (Multiline) | m_edtLog | 송수신 로그 |

---

## 7. 핵심 기능 구현 가이드

### 7.1 COM 포트 자동 검색
레지스트리에서 사용 가능한 시리얼 포트를 검색하여 콤보박스에 표시한다.

```cpp
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
```

### 7.2 수신 스레드 처리
별도 스레드에서 COM 포트를 모니터링하고, 데이터 수신 시 메인 다이얼로그에 사용자 정의 메시지를 전달한다.

```cpp
#define WM_SERIAL_RECEIVE  (WM_USER + 100)

DWORD WINAPI CSerialComm::ReadThread(LPVOID lpParam)
{
    CSerialComm* pThis = (CSerialComm*)lpParam;
    BYTE  buffer[256];
    DWORD dwRead;

    while (pThis->m_bRunning)
    {
        if (ReadFile(pThis->m_hComm, buffer, sizeof(buffer), &dwRead, NULL))
        {
            if (dwRead > 0 && pThis->m_hOwner)
            {
                // 메인 스레드로 데이터 전달 (복사본 할당)
                BYTE* pData = new BYTE[dwRead];
                memcpy(pData, buffer, dwRead);
                PostMessage(pThis->m_hOwner, pThis->m_nMsgID,
                    (WPARAM)dwRead, (LPARAM)pData);
            }
        }
    }
    return 0;
}

// 메인 다이얼로그에서 수신 메시지 처리
LRESULT CLightControllerDlg::OnSerialReceive(WPARAM wParam, LPARAM lParam)
{
    DWORD dwSize = (DWORD)wParam;
    BYTE* pData = (BYTE*)lParam;

    for (DWORD i = 0; i < dwSize; i++)
    {
        if (CProtocolBuilder::IsACK(pData[i]))
            AddLog(_T("[RX] ACK (0x06) - 정상 수신"));
        else if (CProtocolBuilder::IsNAK(pData[i]))
            AddLog(_T("[RX] NAK (0x15) - 비정상 수신"));
        else
        {
            CString strHex;
            strHex.Format(_T("[RX] 0x%02X"), pData[i]);
            AddLog(strHex);
        }
    }

    delete[] pData;
    return 0;
}
```

### 7.3 데이터 송신 및 로그

```cpp
void CLightControllerDlg::SendCommand(const CString& strCmd)
{
    if (!m_serial.IsOpen())
    {
        AfxMessageBox(_T("시리얼 포트가 연결되지 않았습니다."));
        return;
    }

    // CString → ANSI 변환 후 전송 (프로토콜이 ASCII 기반)
    CStringA strAnsi(strCmd);
    m_serial.Send((const BYTE*)(LPCSTR)strAnsi, strAnsi.GetLength());

    // 로그 기록
    CString strLog;
    strLog.Format(_T("[TX] %s"), strCmd.Left(strCmd.GetLength() - 2)); // CR/LF 제거 표시
    AddLog(strLog);
}
```

---

## 8. 파일 구성 (예상)

```
LightController/
├── LightController.sln                    # 솔루션 파일
├── LightController/
│   ├── LightController.vcxproj            # 프로젝트 파일
│   ├── LightController.rc                 # 리소스 파일
│   ├── resource.h                         # 리소스 ID 정의
│   ├── LightController.h / .cpp           # 앱 클래스
│   ├── LightControllerDlg.h / .cpp        # 메인 대화상자
│   ├── SerialComm.h / .cpp                # 시리얼 통신 클래스
│   ├── ProtocolBuilder.h / .cpp           # 프로토콜 생성 클래스
│   ├── stdafx.h / stdafx.cpp              # 미리 컴파일된 헤더
│   └── res/
│       ├── LightController.ico            # 아이콘
│       └── LightController.rc2            # 리소스
```

---

## 9. 빌드 및 실행

### 9.1 빌드 순서
1. Visual Studio에서 솔루션 열기 (`.sln`)
2. 구성: `Debug` 또는 `Release`
3. 플랫폼: `x86` 또는 `x64`
4. **빌드** → **솔루션 빌드** (Ctrl + Shift + B)

### 9.2 실행 전 확인사항
- IMS-DS-16 전원 ON 확인
- RS232 케이블 연결 확인 (또는 USB-to-Serial 변환기 사용)
- 장치 관리자에서 COM 포트 번호 확인
- **패리티 설정이 Even인지 반드시 확인** (통신 불량의 가장 흔한 원인)

---

## 10. 주의사항

1. **패리티 설정**: 반드시 **Even**으로 설정해야 정상 통신 가능
2. **배수 설정 주의**: 무리한 배수 증가 시 조명 파손 위험 (최대 5배)
3. **ON TIME 범위**: 0~999us, 배수 적용 시 최대 4,995us
4. **데이터 저장**: `00S` 명령으로 저장하지 않으면 전원 OFF 시 설정 소실
5. **순차 트리거**: 통신을 통한 트리거(`00TPP`)는 테스트 용도로만 사용 권장. 실제 운용 시 외부 트리거 신호 사용
6. **문자열 인코딩**: 프로토콜이 ASCII 기반이므로 CString → CStringA 변환 후 전송 필수
7. **스레드 안전**: 수신 스레드에서 직접 UI 업데이트 금지, PostMessage로 메인 스레드에 전달

---

## 부록: 명령어 빠른 참조표

| 기능 | 명령어 형식 | 예시 |
|------|-----------|------|
| ON TIME 설정 (LS12) | `:1[P][V],[V],...\r\n` | `:10255,000,...,000\r\n` |
| ON TIME 설정 (DS16) | `:00B[MP][PG][RP][V],[V],...\r\n` | `:00B01002999,...,999\r\n` |
| 배수 설정 | `:1X[M],[M],...\r\n` | `:1X5,1,...,1\r\n` |
| 배수 확인 | `:1RU\r\n` | `:1RU\r\n` |
| Ontime 확인 | `:1RA\r\n` | `:1RA\r\n` |
| Max Page (LS12) | `:1P[MP]\r\n` | `:1P15\r\n` |
| 데이터 확인 | `:00R\r\n` | `:00R\r\n` |
| 데이터 저장 | `:00S\r\n` | `:00S\r\n` |
| 페이지 트리거 | `:00TPP\r\n` | `:00TPP\r\n` |
| 페이지 리셋 | `:00TPS\r\n` | `:00TPS\r\n` |
| 시작 페이지 설정 | `:00P[PG]\r\n` | `:00P15\r\n` |
| 현재 페이지 확인 | `:00G\r\n` | `:00G\r\n` |
| 페이지 HOLD | `:00H[0/1]\r\n` | `:00H1\r\n` |
| Page Section ON/OFF | `:00CO[0/1]\r\n` | `:00CO1\r\n` |
| Section 시작 페이지 | `:00CPS[PG]\r\n` | `:00CPS01\r\n` |
| Section 끝 페이지 | `:00CPE[PG]\r\n` | `:00CPE03\r\n` |
| 트리거 Skip ON/OFF | `:00IO[0/1]\r\n` | `:00IO1\r\n` |
| Skip Time 설정 | `:00IT[T]\r\n` | `:00IT030\r\n` |
