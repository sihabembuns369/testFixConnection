#ifndef __GSOCKET_H__
#define __GSOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define SOCKET_TIMEOUT -2
const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;

class CGSocket
{
public:
	CGSocket();
	CGSocket(int s);
	~CGSocket();

	void CloseSocket();
	int GetSocket() const;
	void SetSocket(int s);
	int Create(int sockType = SOCK_STREAM);
	int Connect(const char *pszServerIP, unsigned short serverPort);
	void GetLocalIPAddress(char *pszIPAddr, int bufLen);
	void GetLocalComputerName(char *pszHostName, int bufLen);
	int GetPeerInfo(unsigned short &peerPort, char *pszPeerIP, int bufLen);
	int GetPeerComputerName(char *pszPeerName, int bufLen);
	int Bind(unsigned short socketPort);
	int Bind(unsigned short socketPort, const char *pszSocketAddress);
	int Bind(const struct sockaddr *pSockAddr, int nSockAddrLen);
	int Listen(int socketBackLog = 5);
	int Accept(CGSocket *pConnectedSocket, struct sockaddr *pSockAddr = NULL);
	long Read(char *pszBuf, unsigned long nBytesToRead, int flags = 0, int nSecTimeOut = -1); // -1 : never timeout; 0 : nonblocking read
	long ReadBlock(char *pszBuf, unsigned long nBytesToRead, int flags = 0);
	long IDXReadLine(char *data, int maxDataLen, int flags = 0);
	long ReadLine(char *data, int maxDataLen, int flags = 0, int nSecTimeOut = -1);
	long Write(const char *pszBuf, unsigned long nBytesToWrite, int flags = 0);
	long WriteEx(const char *pszBuf, unsigned long nBytesToWrite, int flags = 0);
	long WriteBlock(const char *pszBuf, unsigned long nBytesToWrite, int flags = 0);
	
	int GetRecvBufSize();
	int GetSendBufSize();

	int IsDataWaiting() const;
	int ClearReceivedBuffer();
private:
	int hSocket, cnt;
	char b[87380], *bp;
	pthread_mutex_t readLock, writeLock;
};

#endif
