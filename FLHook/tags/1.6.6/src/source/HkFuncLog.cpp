#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkHandleCheater(uint iClientID, bool bBan, wstring wscReason, ...)
{
	wchar_t wszBuf[1024*8] = L"";
	va_list marker;
	va_start(marker, wscReason);

	_vsnwprintf(wszBuf, (sizeof(wszBuf) / 2) - 1, wscReason.c_str(), marker);

	if(!Players.GetActiveCharacterName(iClientID))
		return;

	wstring wscCharname = Players.GetActiveCharacterName(iClientID);
	HkAddCheaterLog(wscCharname, wszBuf);

	wchar_t wszBuf2[256];
	swprintf(wszBuf2, L"Possible cheating detected: %s", wscCharname.c_str());
	if(wscReason[0] != '#')
		HkMsgU(wszBuf2);
	if(bBan)
		HkBan(ARG_CLIENTID(iClientID), true);
	if(wscReason[0] != '#')
		HkKick(ARG_CLIENTID(iClientID));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkAddCheaterLog(wstring wscCharname, wstring wscReason)
{
	FILE *f = fopen((scAcctPath + "flhook_cheaters.log").c_str(), "at");
	if(!f)
		return false;

	CAccount *acc = HkGetAccountByCharname(wscCharname);
	wstring wscAccountDir;
	HkGetAccountDirName(acc, wscAccountDir);

	time_t tNow = time(0);
	struct tm *stNow = localtime(&tNow);
	fprintf(f, "%.2d/%.2d/%.4d %.2d:%.2d:%.2d Possible cheating detected (%s) by %s(%s)(%s)\n", stNow->tm_mon + 1, stNow->tm_mday, stNow->tm_year + 1900, stNow->tm_hour, stNow->tm_min, stNow->tm_sec, wstos(wscReason).c_str(), wstos(wscCharname).c_str(), wstos(wscAccountDir).c_str(), wstos(HkGetAccountID(acc)).c_str());
	fclose(f);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkAddKickLog(uint iClientID, wstring wscReason, ...)
{
	wchar_t wszBuf[1024*8] = L"";
	va_list marker;
	va_start(marker, wscReason);

	_vsnwprintf(wszBuf, (sizeof(wszBuf) / 2) - 1, wscReason.c_str(), marker);

	FILE *f = fopen((scAcctPath + "flhook_kicks.log").c_str(), "at");
	if(!f)
		return false;

	const wchar_t *wszCharname = Players.GetActiveCharacterName(iClientID);
	if(!wszCharname)
		wszCharname = L"";

	CAccount *acc = Players.FindAccountFromClientID(iClientID);
	wstring wscAccountDir;
	HkGetAccountDirName(acc, wscAccountDir);

	time_t tNow = time(0);
	struct tm *stNow = localtime(&tNow);
	fprintf(f, "%.2d/%.2d/%.4d %.2d:%.2d:%.2d Kick (%s): %s(%s)(%s)\n", stNow->tm_mon + 1, stNow->tm_mday, stNow->tm_year + 1900, stNow->tm_hour, stNow->tm_min, stNow->tm_sec, wstos(wszBuf).c_str(), wstos(wszCharname).c_str(), wstos(wscAccountDir).c_str(), wstos(HkGetAccountID(acc)).c_str());
	fclose(f);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ??? what have i done? 8[

bool HkAddConnectLog(uint iClientID)
{
	FILE *f = fopen((scAcctPath + "flhook_connects.log").c_str(), "at");
	if(!f)
		return false;

	const wchar_t *wszCharname = Players.GetActiveCharacterName(iClientID);
	if(!wszCharname)
		wszCharname = L"";

	CAccount *acc = Players.FindAccountFromClientID(iClientID);
	wstring wscAccountDir;
	HkGetAccountDirName(acc, wscAccountDir);

	time_t tNow = time(0);
	struct tm *stNow = localtime(&tNow);
//	fprintf(f, "%.2d/%.2d/%.4d %.2d:%.2d:%.2d Kick (%s): %s(%s)(%s)\n", stNow->tm_mon + 1, stNow->tm_mday, stNow->tm_year + 1900, stNow->tm_hour, stNow->tm_min, stNow->tm_sec, wstos(wscReason).c_str(), wstos(wszCharname).c_str(), wstos(wscAccountDir).c_str(), wstos(HkGetAccountID(acc)).c_str());
	fclose(f);
	return true;
}
