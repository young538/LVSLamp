# BCGControlBar Pro 적용 가이드

## 1. BCGSoft 설치 정보

| 항목 | 값 |
|------|-----|
| 제품 | BCGControlBar Professional v27.10 |
| 설치 경로 | `C:\Program Files (x86)\BCGSoft\BCGControlBarPro\` |
| 헤더 경로 | `...\BCGCBPro\` |
| x64 라이브러리 | `...\Bin64\` (v141 toolset, VS2022 호환) |
| DLL PATH | Bin64 폴더가 시스템 PATH에 등록됨 |

### 1.1 라이브러리 파일 명명 규칙

```
BCGCBPRO2710[Static][U][D][s]141.lib
              │       │  │  │  └ VS2015+ toolset
              │       │  │  └── Static+shared CRT (s)
              │       │  └───── Debug (D)
              │       └──────── Unicode (U)
              └──────────────── Static linking
```

| 빌드 구성 | lib 파일 | dll 파일 |
|-----------|---------|---------|
| x64 Unicode Debug (DLL) | `BCGCBPRO2710ud141.lib` | `BCGCBPRO2710ud141.dll` |
| x64 Unicode Release (DLL) | `BCGCBPRO2710u141.lib` | `BCGCBPRO2710u141.dll` |
| x64 Unicode Debug (Static) | `BCGCBPRO2710StaticUD141.lib` | 없음 |
| x64 Unicode Release (Static) | `BCGCBPRO2710StaticU141.lib` | 없음 |

### 1.2 자동 링크 (Auto-Link)

BCGCBProInc.h 헤더에 `#pragma comment(lib, ...)` 가 포함되어 있어, 헤더 include와 라이브러리 검색 경로만 설정하면 자동으로 적절한 lib이 링크된다.

---

## 2. 프로젝트 적용 절차

### 2.1 플랫폼 변경: Win32 → x64

BCGSoft v141 pre-built 라이브러리가 x64만 제공되므로 프로젝트를 **x64**로 전환한다.

### 2.2 vcxproj 설정 추가

```xml
<!-- 모든 Configuration에 추가 -->
<ClCompile>
  <AdditionalIncludeDirectories>
    C:\Program Files (x86)\BCGSoft\BCGControlBarPro\BCGCBPro;%(AdditionalIncludeDirectories)
  </AdditionalIncludeDirectories>
</ClCompile>
<Link>
  <AdditionalLibraryDirectories>
    C:\Program Files (x86)\BCGSoft\BCGControlBarPro\Bin64;%(AdditionalLibraryDirectories)
  </AdditionalLibraryDirectories>
</Link>
```

### 2.3 framework.h 수정

기존 MFC 헤더 대신 BCGSoft 헤더를 사용한다.

```cpp
// 변경 전
#include <afxcontrolbars.h>

// 변경 후
#include "BCGCBProInc.h"
```

### 2.4 앱 클래스 변경

```cpp
// 변경 전
class CLightControllerApp : public CWinApp

// 변경 후
class CLightControllerApp : public CBCGPWinApp
```

`InitInstance()`에서 BCG 비주얼 매니저를 설정한다.

```cpp
BOOL CLightControllerApp::InitInstance()
{
    CBCGPWinApp::InitInstance();

    // 비주얼 테마 설정 (택 1)
    CBCGPVisualManager::SetDefaultManager(
        RUNTIME_CLASS(CBCGPVisualManager2013));   // Office 2013 스타일
    CBCGPVisualManager2013::SetStyle(
        CBCGPVisualManager2013::Office2013_DarkGray);

    // ... 다이얼로그 생성
}
```

### 2.5 다이얼로그 클래스 변경

```cpp
// 변경 전
class CLightControllerDlg : public CDialogEx

// 변경 후
class CLightControllerDlg : public CBCGPDialog
```

`OnInitDialog()`에서 비주얼 매니저 테마를 다이얼로그에 적용한다.

```cpp
BOOL CLightControllerDlg::OnInitDialog()
{
    CBCGPDialog::OnInitDialog();

    // BCG 비주얼 매니저 테마를 다이얼로그에 적용
    EnableVisualManagerStyle(TRUE, TRUE);

    // ... 나머지 초기화
}
```

---

## 3. 사용 가능한 비주얼 테마

| 클래스 | 스타일 | 설명 |
|--------|--------|------|
| `CBCGPVisualManager` | 기본 | 기본 플랫 스타일 |
| `CBCGPVisualManagerXP` | XP | Windows XP 스타일 |
| `CBCGPVisualManager2003` | Office 2003 | Office 2003 스타일 |
| `CBCGPVisualManager2007` | Luna/Aqua/Obsidian/Silver | Office 2007 리본 스타일 |
| `CBCGPVisualManager2010` | Blue/Black/White | Office 2010 스타일 |
| `CBCGPVisualManager2013` | White/LightGray/DarkGray | Office 2013 플랫 스타일 |
| `CBCGPVisualManagerVS2012` | Light/Dark/Blue | VS 2012 스타일 |
| `CBCGPVisualManagerCarbon` | - | 카본 다크 테마 |
| `CBCGPVisualManagerScenic` | - | Windows 7 스타일 |

### 3.1 테마 적용 코드 예시

```cpp
// Office 2013 Dark Gray
CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2013));
CBCGPVisualManager2013::SetStyle(CBCGPVisualManager2013::Office2013_DarkGray);

// Visual Studio 2012 Dark
CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManagerVS2012));
CBCGPVisualManagerVS2012::SetStyle(CBCGPVisualManagerVS2012::VS2012_Dark);

// Office 2007 Aqua
CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2007));
CBCGPVisualManager2007::SetStyle(CBCGPVisualManager2007::VS2007_Aqua);
```

---

## 4. BCG 향상 컨트롤 (선택 적용)

BCGSoft는 기본 MFC 컨트롤의 향상된 버전을 제공한다. `EnableVisualManagerStyle(TRUE)`만으로도 기존 컨트롤이 테마에 맞게 렌더링되지만, 추가 기능이 필요하면 BCG 전용 컨트롤을 사용할 수 있다.

| 기본 MFC | BCG 대체 | 추가 기능 |
|----------|---------|----------|
| CEdit | CBCGPEdit | 프롬프트 텍스트, 브라우즈 버튼 |
| CComboBox | CBCGPComboBox | 비주얼 테마 자동 적용 |
| CButton | CBCGPButton | 이미지 버튼, 플랫 스타일 |
| CSpinButtonCtrl | CBCGPSpinButtonCtrl | 테마 연동 |
| CListCtrl | CBCGPListCtrl | 그리드 스타일, 정렬 |
| CGroupBox | CBCGPGroup | 테마 연동 그룹박스 |

---

## 5. 주의사항

1. **x64 전용**: pre-built lib이 Bin64에만 있으므로 반드시 x64 플랫폼으로 빌드
2. **DLL 배포**: Release 빌드 시 `BCGCBPRO2710u141.dll`과 스타일 DLL을 함께 배포해야 함
3. **라이선스**: BCGControlBar Pro는 상용 라이브러리 (개발 PC에 라이선스 필요)
4. **자동 링크**: BCGCBProInc.h가 자동으로 올바른 lib을 선택하므로 수동 lib 추가 불필요
5. **MFC 공유 DLL**: BCG DLL 방식 사용 시 프로젝트도 "공유 DLL에서 MFC 사용" 설정 필요
