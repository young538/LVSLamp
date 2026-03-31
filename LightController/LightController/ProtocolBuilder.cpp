#include "pch.h"
#include "ProtocolBuilder.h"

// ============================================================
// IMS-LS12 호환 명령어 (국번 '1')
// ============================================================

// :1[PAGE][CH01],[CH02],...,[CH16]\r\n
CString CProtocolBuilder::BuildSetValue(int nPage, const int arrValue[16])
{
	CString strCmd;
	strCmd.Format(_T(":1%d"), nPage % 10);

	for (int i = 0; i < 16; i++)
	{
		CString strVal;
		int val = max(0, min(999, arrValue[i]));
		strVal.Format(_T("%s%03d"), (i == 0) ? _T("") : _T(","), val);
		strCmd += strVal;
	}
	strCmd += _T("\r\n");
	return strCmd;
}

// :1X[MUL1],[MUL2],...,[MUL16]\r\n
CString CProtocolBuilder::BuildSetMultiplier(const int arrMul[16])
{
	CString strCmd = _T(":1X");

	for (int i = 0; i < 16; i++)
	{
		int mul = max(1, min(5, arrMul[i]));
		CString strVal;
		strVal.Format(_T("%s%d"), (i == 0) ? _T("") : _T(","), mul);
		strCmd += strVal;
	}
	strCmd += _T("\r\n");
	return strCmd;
}

// :1RU\r\n
CString CProtocolBuilder::BuildReadMultiplier()
{
	return _T(":1RU\r\n");
}

// :1RA\r\n
CString CProtocolBuilder::BuildReadOntime()
{
	return _T(":1RA\r\n");
}

// :1P[MAXPAGE]\r\n
CString CProtocolBuilder::BuildSetMaxPage_LS12(int nMaxPage)
{
	CString strCmd;
	nMaxPage = max(1, min(32, nMaxPage));
	strCmd.Format(_T(":1P%02d\r\n"), nMaxPage);
	return strCmd;
}

// ============================================================
// DS-16 전용 명령어 (ADDR '00')
// ============================================================

// :00B[MAXPAGE][PAGE][RP][CH01],[CH02],...,[CH16]\r\n
CString CProtocolBuilder::BuildSetOnTime(int nMaxPage, int nPage, int nRepeat, const int arrValue[16])
{
	nMaxPage = max(1, min(99, nMaxPage));
	nPage = max(0, min(98, nPage));
	nRepeat = max(1, min(9, nRepeat));

	CString strCmd;
	strCmd.Format(_T(":00B%02d%02d%d"), nMaxPage, nPage, nRepeat);

	for (int i = 0; i < 16; i++)
	{
		int val = max(0, min(999, arrValue[i]));
		CString strVal;
		strVal.Format(_T("%s%03d"), (i == 0) ? _T("") : _T(","), val);
		strCmd += strVal;
	}
	strCmd += _T("\r\n");
	return strCmd;
}

// :00R\r\n
CString CProtocolBuilder::BuildReadData()
{
	return _T(":00R\r\n");
}

// :00S\r\n
CString CProtocolBuilder::BuildSaveData()
{
	return _T(":00S\r\n");
}

// :00TPP\r\n
CString CProtocolBuilder::BuildPageTrigger()
{
	return _T(":00TPP\r\n");
}

// :00TPS\r\n
CString CProtocolBuilder::BuildPageReset()
{
	return _T(":00TPS\r\n");
}

// :00P[PAGE]\r\n
CString CProtocolBuilder::BuildSetStartPage(int nPage)
{
	CString strCmd;
	nPage = max(0, min(99, nPage));
	strCmd.Format(_T(":00P%02d\r\n"), nPage);
	return strCmd;
}

// :00G\r\n
CString CProtocolBuilder::BuildGetCurrentPage()
{
	return _T(":00G\r\n");
}

// :00H[0/1]\r\n
CString CProtocolBuilder::BuildSetPageHold(BOOL bOn)
{
	CString strCmd;
	strCmd.Format(_T(":00H%d\r\n"), bOn ? 1 : 0);
	return strCmd;
}

// :00CO[0/1]\r\n
CString CProtocolBuilder::BuildSetPageSection(BOOL bOn)
{
	CString strCmd;
	strCmd.Format(_T(":00CO%d\r\n"), bOn ? 1 : 0);
	return strCmd;
}

// :00CPS[PAGE]\r\n
CString CProtocolBuilder::BuildSetSectionStartPage(int nPage)
{
	CString strCmd;
	nPage = max(0, min(99, nPage));
	strCmd.Format(_T(":00CPS%02d\r\n"), nPage);
	return strCmd;
}

// :00CPE[PAGE]\r\n
CString CProtocolBuilder::BuildSetSectionEndPage(int nPage)
{
	CString strCmd;
	nPage = max(0, min(99, nPage));
	strCmd.Format(_T(":00CPE%02d\r\n"), nPage);
	return strCmd;
}

// :00IO[0/1]\r\n
CString CProtocolBuilder::BuildSetTriggerSkip(BOOL bOn)
{
	CString strCmd;
	strCmd.Format(_T(":00IO%d\r\n"), bOn ? 1 : 0);
	return strCmd;
}

// :00IT[TIME]\r\n
CString CProtocolBuilder::BuildSetSkipTime(int nTimeMs)
{
	CString strCmd;
	nTimeMs = max(0, min(999, nTimeMs));
	strCmd.Format(_T(":00IT%03d\r\n"), nTimeMs);
	return strCmd;
}

// ============================================================
// 응답 파싱
// ============================================================

BOOL CProtocolBuilder::IsACK(BYTE bData)
{
	return (bData == 0x06);
}

BOOL CProtocolBuilder::IsNAK(BYTE bData)
{
	return (bData == 0x15);
}
