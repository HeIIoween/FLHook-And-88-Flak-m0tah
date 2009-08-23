#include "hook.h"

/**************************************************************************************************************
called when chat-text is being sent to a player, we reformat it(/set chatfont)
**************************************************************************************************************/

#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

void __stdcall HkCb_SendChat(uint iClientID, uint iTo, uint iSize, void *pRDL)
{
	try {
		if(HkIServerImpl::g_bInSubmitChat && (iTo != 0x10004) && set_bUserCmdSetChatFont) {
			wchar_t wszBuf[1024] = L"";
			// extract text from rdlReader
			BinaryRDLReader rdl;
			uint iRet;
			rdl.extract_text_from_buffer(wszBuf, sizeof(wszBuf), iRet, (const char*)pRDL, iSize);

			wstring wscBuf = wszBuf;
			wstring wscSender = wscBuf.substr(0, wscBuf.length() - HkIServerImpl::g_iTextLen - 2);
			wstring wscText = wscBuf.substr(wscBuf.length() - HkIServerImpl::g_iTextLen);

			if(ClientInfo[iClientID].bIgnoreUniverse && iTo == 0x10000)
				return;
			if(set_bUserCmdIgnore /*&& ((iTo & 0xFFFF) != 0)*/)
			{ // check ignores
				foreach(ClientInfo[iClientID].lstIgnore, IGNORE_INFO, it)
				{
					if(HAS_FLAG(*it, L"p") && (iTo & 0x10000))
						continue; // no privchat
					else if(!HAS_FLAG(*it, L"i") && !(ToLower(wscSender).compare(ToLower((*it).wscCharname))))
						return; // ignored
					else if(HAS_FLAG(*it, L"i") && (ToLower(wscSender).find(ToLower((*it).wscCharname)) != -1))
						return; // ignored
				}
			}

			uchar cFormat;
			// adjust chatsize
			switch(ClientInfo[iClientID].chatSize)
			{
			case CS_SMALL:
				cFormat = 0x90;
				break;

			case CS_BIG:
				cFormat = 0x10;
				break;

			default:
				cFormat = 0x00;
				break;
			}

			// adjust chatstyle
			if(HkIServerImpl::g_bChatAction)
			{
				wscSender = L"***" + wscSender;
				wscText = wscText.substr(4);
				if(ClientInfo[iClientID].chatStyle == CST_ITALIC)
					cFormat += 0x01; //Bold text
				else
					cFormat += 0x02; //Italic text
			}
			else
			{
				switch(ClientInfo[iClientID].chatStyle)
				{
				case CST_BOLD:
					cFormat += 0x01;
					break;

				case CST_ITALIC:
					cFormat += 0x02;
					break;

				case CST_UNDERLINE:
					cFormat += 0x04;
					break;

				default:
					cFormat += 0x00;
					break;
				}
			}

			wchar_t wszFormatBuf[8];
			swprintf(wszFormatBuf, L"%02X", (long)cFormat);
			wstring wscTRADataFormat = wszFormatBuf;
			wstring wscTRADataColor;
			wstring wscTRADataSenderColor = set_wscSenderColor;
			if(g_bMsg) {
				wscTRADataSenderColor = L"00FF00";
				wscTRADataColor = set_wscPMColor; // pm chatcolor
			} else if(g_bMsgS) {
				wscTRADataSenderColor = L"00FF00";
				wscTRADataColor = set_wscSystemColor; // system chatcolor
			} else if(g_bMsgU) {
				wscTRADataSenderColor = L"00FF00";
				wscTRADataColor = set_wscUniverseColor; // universe chatcolor
			} else if(iTo == 0x10000)
				wscTRADataColor = set_wscUniverseColor; // universe chatcolor
			else if(iTo == 0)
				wscTRADataColor = L"FFFFFF"; // console
			else if(iTo == 0x10003)
				wscTRADataColor = set_wscGroupColor; // group chat
			else if(iTo & 0x10000)
				wscTRADataColor = set_wscSystemColor; // system chatcolor
			else
				wscTRADataColor = set_wscPMColor; // pm chatcolor

			wstring wscXML = L"<TRA data=\"0x" + wscTRADataSenderColor + wscTRADataFormat + L"\" mask=\"-1\"/><TEXT>" + XMLText(wscSender) + (HkIServerImpl::g_bChatAction ? L" </TEXT>" : L": </TEXT>") + L"<TRA data=\"0x" + wscTRADataColor + wscTRADataFormat + L"\" mask=\"-1\"/><TEXT>" + XMLText(wscText) + L"</TEXT>";
			HkFMsg(iClientID, wscXML);

			//AFK Messages
			if(ClientInfo[iClientID].wscAfkMsg.size())
			{
				uint iClientIDTarget = HkGetClientIdFromCharname(wscSender);
				foreach(ClientInfo[iClientID].iSentIds, uint, idTarget)
				{
					if((*idTarget)==iClientIDTarget)
					{
						return;
					}
				}
				ClientInfo[iClientID].iSentIds.push_back(iClientIDTarget);
				wstring wscTo = Players.GetActiveCharacterName(iClientID);
				wscSender = wscTo;
				wscTo += L"(AFK)";

				//Format AFK Message
				if(set_bUserCmdIgnore /*&& ((iTo & 0xFFFF) != 0)*/)
				{ // check ignores
					foreach(ClientInfo[iClientIDTarget].lstIgnore, IGNORE_INFO, it)
					{
						if(HAS_FLAG(*it, L"p") && (iTo & 0x10000))
							continue; // no privchat
						else if(!HAS_FLAG(*it, L"i") && !(ToLower(wscSender).compare(ToLower((*it).wscCharname))))
							return; // ignored
						else if(HAS_FLAG(*it, L"i") && (ToLower(wscSender).find(ToLower((*it).wscCharname)) != -1))
							return; // ignored
					}
				}
				// adjust chatsize
				switch(ClientInfo[iClientIDTarget].chatSize)
				{
				case CS_SMALL:
					cFormat = 0x90;
					break;

				case CS_BIG:
					cFormat = 0x10;
					break;

				default:
					cFormat = 0x00;
					break;
				}
				// adjust chatstyle
				switch(ClientInfo[iClientIDTarget].chatStyle)
				{
				case CST_BOLD:
					cFormat += 0x01;
					break;

				case CST_ITALIC:
					cFormat += 0x02;
					break;

				case CST_UNDERLINE:
					cFormat += 0x04;
					break;

				default:
					cFormat += 0x00;
					break;
				}
				wszFormatBuf[8];
				swprintf(wszFormatBuf, L"%02X", (long)cFormat);
				wscTRADataFormat = wszFormatBuf;
				wscXML = L"<TRA data=\"0x" + wscTRADataSenderColor + wscTRADataFormat + L"\" mask=\"-1\"/><TEXT>" + XMLText(wscTo) + L": </TEXT>" + 
					L"<TRA data=\"0x19BD3A" + wscTRADataFormat + L"\" mask=\"-1\"/><TEXT>" + XMLText(ClientInfo[iClientID].wscAfkMsg) + L"</TEXT>";
				HkFMsg(iClientIDTarget, wscXML);
			}
		} else {
			__asm
			{
				pushad
				push [pRDL]
				push [iSize]
				push [iTo]
				push [iClientID]
				mov ecx, [pClient]
				add ecx, 4
				call [RCSendChatMsg]
				popad
			}
			//AFK Messages
			/*if(ClientInfo[iClientID].wscAfkMsg.size())
			{
				//PrintUniverseText(L"AFK!");
				wchar_t wszBuf[1024] = L"";
				// extract text from rdlReader
				BinaryRDLReader rdl;
				uint iRet;
				rdl.extract_text_from_buffer(wszBuf, sizeof(wszBuf), iRet, (const char*)pRDL, iSize);
				wstring wscBuf = wszBuf;
				wstring wscSender = wscBuf.substr(0, wscBuf.length() - HkIServerImpl::g_iTextLen - 2);	

				uint iClientIDTarget = HkGetClientIdFromCharname(wscSender);
				wstring awayMsg = Players.GetActiveCharacterName(iClientID);
				awayMsg += L": ";
				awayMsg += ClientInfo[iClientID].wscAfkMsg;
				HkMsg(iClientIDTarget, awayMsg);
			}*/
		}

	}  catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}
