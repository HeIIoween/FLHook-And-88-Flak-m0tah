#include "CCmds.h"
//#include "blowfish.h"

#ifndef _CSOCKET_
#define _CSOCKET_

class CSocket : public CCmds
{
public:
	SOCKET s;
	BLOWFISH_CTX *bfc;
	bool bAuthed;
	bool bEventMode;
	bool bUnicode;
	bool bEncrypted;

	CSocket() { bAuthed = false; bEventMode = false; bUnicode = false; bEncrypted = false; wscCharRestrict = L""; }
	void DoPrint(wstring wscText);
	wstring GetAdminName();
};


#endif
