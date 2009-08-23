#ifndef _CCONSOLE_
#define _CCONSOLE_

#include "CCmds.h"

class CConsole : public CCmds
{
public:
	CConsole() { this->rights = RIGHT_SUPERADMIN; this->wscCharRestrict = L""; };
	void DoPrint(wstring wscText);
	wstring GetAdminName();
};

#endif
