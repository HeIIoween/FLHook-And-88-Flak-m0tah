#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkLoadNewCharacter()
{
	set_lstNewCharRep.clear();
	INI_Reader ini;
	if(ini.open("mpnewcharacter.fl", false))
	{
		ini.read_header();
		if(ini.is_header("Player"))
		{
			while(ini.read_value())
			{
				if(ini.is_value("house"))
				{
					NEW_CHAR_REP rep;
					rep.fRep = ini.get_value_float(0);
					uint iRepGroupID;
					pub::Reputation::GetReputationGroup(iRepGroupID, ini.get_value_string(1));
					rep.iRepGroupID = iRepGroupID;
					set_lstNewCharRep.push_back(rep);
				}
			}
		}
		ini.close();
	}
}

void HkLoadEmpathy(const char *szFLConfigFile)
{
	set_btEmpathy->Clear();
	INI_Reader ini2;
	string scDataPath = "..\\data";
	if(ini2.open(szFLConfigFile, false))
	{
		while(ini2.read_header())
		{
			if(ini2.is_header("Freelancer"))
			{
				while(ini2.read_value())
				{
					if(ini2.is_value("data path"))
					{
						scDataPath = ini2.get_value_string(0);
						break;
					}
				}
				break;
			}
		}
		ini2.close();
	}
	INI_Reader ini;
	char *szEmpathyPath = (char*)((char*)hModContent + 0x117CEC);
	if(ini.open((scDataPath + "\\" + szEmpathyPath).c_str(), false))
	{
		while(ini.read_header())
		{
			if(ini.is_header("RepChangeEffects"))
			{
				uint iChangeGroupID = 0;
				float fDestructChange = 0.0f;
				BinaryTree<EMPATHY_RATE> *btEmpathyRates = new BinaryTree<EMPATHY_RATE>();
				while(ini.read_value())
				{
					if(ini.is_value("empathy_rate"))
					{
						uint iGroupID;
						pub::Reputation::GetReputationGroup(iGroupID, ini.get_value_string(0));
						float fEmpathy = ini.get_value_float(1);
						EMPATHY_RATE *rate = new EMPATHY_RATE(iGroupID, fEmpathy);
						btEmpathyRates->Add(rate);
					}
					else if(ini.is_value("event"))
					{
						if(ini.get_value_string(0) == string("object_destruction"))
						{
							fDestructChange = ini.get_value_float(1);
						}
					}
					else if(ini.is_value("group"))
					{
						pub::Reputation::GetReputationGroup(iChangeGroupID, ini.get_value_string(0));
					}
				}
				if(iChangeGroupID)
				{
					REP_CHANGE_EFFECT *rce = new REP_CHANGE_EFFECT(iChangeGroupID, fDestructChange, btEmpathyRates);
					set_btEmpathy->Add(rce);
				}
				else
					delete btEmpathyRates;
			}
		}
		ini.close();
	}
}

void HkLoadDLLConf(const char *szFLConfigFile)
{
	for(uint i=0; i<vDLLs.size(); i++)
	{
		FreeLibrary(vDLLs[i]);
	}
	vDLLs.clear();
	HINSTANCE hDLL = LoadLibraryEx((char*)((char*)GetModuleHandle(0) + 0x256C4), NULL, LOAD_LIBRARY_AS_DATAFILE); //typically resources.dll
	if(hDLL)
		vDLLs.push_back(hDLL);
	INI_Reader ini;
	if(ini.open(szFLConfigFile, false))
	{
		while(ini.read_header())
		{
			if(ini.is_header("Resources"))
			{
				while(ini.read_value())
				{
					if(ini.is_value("DLL"))
					{
						hDLL = LoadLibraryEx(ini.get_value_string(0), NULL, LOAD_LIBRARY_AS_DATAFILE);
						if(hDLL)
							vDLLs.push_back(hDLL);
					}
				}
			}
		}
		ini.close();
	}
}