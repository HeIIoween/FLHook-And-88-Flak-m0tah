#ifndef _REMOTECLIENT_
#define _REMOTECLIENT_

extern "C" IMPORT class ChatClientInterface* GetChatClientInterface(void);
IMPORT int __cdecl GetClientStats(struct client_stats_t *,int *);
IMPORT int __cdecl GetNumClients(void);

#endif
