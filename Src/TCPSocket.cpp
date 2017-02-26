#include "TCPSocket.h"


TCPClient::TCPClient():m_isOpen(false),
m_recvThreadState(0),
m_listener(NULL)
{

};

TCPClient::~TCPClient()
{
	close();
};

void TCPClient::setListener(IListener* pListener)
{
	m_listener = pListener;
}

DWORD WINAPI recvProc(_In_ LPVOID paramer)
{
	TCPClient *tcp = (TCPClient*)paramer;

	if(tcp)
	{
		char recvBuf[1024];
		int recvLen;
		while(tcp->m_recvThreadState)
		{
			memset(recvBuf,0,sizeof(recvBuf));
			recvLen=recv(tcp->m_sock,recvBuf,sizeof(recvBuf),0);
			if(recvLen>0)
			{
				printf("recv from[%s]->data=%s datalen=%d \n",tcp->m_serverIP,recvBuf,recvLen);
				if(tcp->m_listener)
				{
					tcp->m_listener->Recv(tcp->m_sock,tcp->m_serverIP,tcp->m_serverPort,recvBuf,recvLen);
				}
			}
			else if(recvLen==0)
			{
				printf("recv from[%s]->server has closed \n",tcp->m_serverIP);
				if(tcp->m_listener)
				{
					tcp->m_listener->Recv(tcp->m_sock,tcp->m_serverIP,tcp->m_serverPort,NULL,0);
				}
				tcp->m_recvThreadState=0;
				break;
			}
		}
	}
	return 0;
	
}

int TCPClient::open(const char* serverIP,const int serverPort)
{
	if(serverIP)
	{
		strcpy_s(m_serverIP,serverIP);
	}
	m_serverPort=serverPort;

	int res=0;
	if(!m_isOpen)
	{
		WSADATA wsaData;

		if(WSAStartup(MAKEWORD(2,2),&wsaData)==NO_ERROR)
		{
			m_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			if(m_sock!=INVALID_SOCKET)
			{
				SOCKADDR_IN server_addr;
				server_addr.sin_family = AF_INET;
				server_addr.sin_port = htons(m_serverPort);
				server_addr.sin_addr.S_un.S_addr=inet_addr(m_serverIP);
				if(connect(m_sock,(sockaddr*)&server_addr,sizeof(server_addr))==NO_ERROR)
				{
					m_isOpen = true;
					m_recvThreadState=1;
					m_recvThreadID = CreateThread(NULL,0,recvProc,(void*)this,0,0);
				}
				else
				{
					res=4;//connect error
				}


			}
			else
			{
				res = 3;// socket init error
			}
		}
		else
		{
			res = 2;//socket Version error
		}

	}
	else
	{
		res = 1;// isOpened
	}
	return res;

}

int TCPClient::close()
{
	if(m_isOpen)
	{
		m_isOpen=false;
		CloseHandle(m_recvThreadID);
		closesocket(m_sock);
	}
	return 0;
}

int TCPClient::sendData(void* data,int dataLength)
{
	int res=0;
	if(m_isOpen)
	{
		int nsend = send(m_sock,(char*)data,dataLength,0);
		if(nsend!=dataLength)
			res=2;
	}
	else
	{
		res=1;
	}
	return res;
}


/*********************************************/

ITCPClient* ITCPClient::getInstance()
{
	TCPClient * pInst= new TCPClient();
	return (ITCPClient*)pInst;
}

void ITCPClient::releaseInstance(ITCPClient* pInst)
{
	if(pInst)
	{
		delete (TCPClient*)pInst;
		pInst = NULL;
	}
}