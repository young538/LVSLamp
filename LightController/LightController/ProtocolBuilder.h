#pragma once

class CProtocolBuilder
{
public:
	// === IMS-LS12 호환 명령어 (국번 '1') ===
	static CString BuildSetValue(int nPage, const int arrValue[16]);
	static CString BuildSetMultiplier(const int arrMul[16]);
	static CString BuildReadMultiplier();
	static CString BuildReadOntime();
	static CString BuildSetMaxPage_LS12(int nMaxPage);

	// === DS-16 전용 명령어 (ADDR '00') ===
	static CString BuildSetOnTime(int nMaxPage, int nPage, int nRepeat, const int arrValue[16]);
	static CString BuildReadData();
	static CString BuildSaveData();
	static CString BuildPageTrigger();
	static CString BuildPageReset();
	static CString BuildSetStartPage(int nPage);
	static CString BuildGetCurrentPage();
	static CString BuildSetPageHold(BOOL bOn);
	static CString BuildSetPageSection(BOOL bOn);
	static CString BuildSetSectionStartPage(int nPage);
	static CString BuildSetSectionEndPage(int nPage);
	static CString BuildSetTriggerSkip(BOOL bOn);
	static CString BuildSetSkipTime(int nTimeMs);

	// === 응답 파싱 ===
	static BOOL IsACK(BYTE bData);
	static BOOL IsNAK(BYTE bData);

	// 데이터 응답 파싱 (콤마 구분 숫자 → 배열)
	static BOOL ParseOntimeResponse(const CString& strLine, int arrValue[16]);
	static BOOL ParseMultiplierResponse(const CString& strLine, int arrMul[16]);
};
