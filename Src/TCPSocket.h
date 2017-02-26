#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>  

#pragma comment(lib, "ws2_32.lib")  

class IListener
{
public:
	//virtual void acceptFrom(SOCKET sock,const char* ip,const int port)=0;
	virtual void Recv(SOCKET sock,const char* ip,const int port,char* data,int dataLength)=0;
};

class ITCPClient
{
public:
	static ITCPClient* getInstance();
	static void releaseInstance(ITCPClient* pInst);
	virtual void setListener(IListener* pListener)=0;
	virtual int open(const char* serverIP,const int serverPort)=0;
	virtual int close()=0;
	virtual int sendData(void* data,int dataLength)=0;
protected:
	virtual ~ITCPClient(){}
};


/*********************************************/

class TCPClient :public ITCPClient
{
public:
	TCPClient();
	~TCPClient();
	void setListener(IListener* pListener);
	int open(const char* serverIP,const int serverPort);
	int close();
	int sendData(void* data,int dataLength);
protected:
	IListener*				m_listener;
	bool					m_isOpen;
	SOCKET					m_sock;
	char					m_serverIP[16];
	unsigned short			m_serverPort;
	HANDLE                  m_recvThreadID;
	int						m_recvThreadState;
private:
	friend DWORD WINAPI recvProc(_In_ LPVOID paramer);
};