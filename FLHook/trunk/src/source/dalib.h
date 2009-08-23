#ifndef _DALIB_
#define _DALIB_

class IMPORT CDPClientProxy
{
public:
	bool GetConnectionStats(struct _DPN_CONNECTION_INFO *);
	double GetLinkSaturation(void);
	bool Send(void *,unsigned long);
};

class CDPServer
{
	char szBuf[1024];

protected:
IMPORT 	static class CDPServer * m_pServer;
IMPORT 	void GetHostAddresses(void);

public:
	static CDPServer* getServer() { return m_pServer; };
IMPORT 	bool SendTo(class CDPClientProxy *,void *,unsigned long);
};


#endif
