#include "header/GSocket.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>

// const int INVALID_SOCKET = -1;
// const int SOCKET_ERROR = -1;

CGSocket::CGSocket():
	cnt(0), bp(NULL)
{
	hSocket = INVALID_SOCKET;
	pthread_mutex_init(&readLock, NULL);
	pthread_mutex_init(&writeLock, NULL);
}

CGSocket::CGSocket(int s):
	cnt(0), bp(NULL)
{
	hSocket = s;
}

CGSocket::~CGSocket()
{
	CloseSocket();
	pthread_mutex_destroy(&readLock);
	pthread_mutex_destroy(&writeLock);
}

int CGSocket::Create(int sockType)
{
	hSocket = socket(AF_INET, sockType, 0);
	if (INVALID_SOCKET != hSocket)
	{
		int enable = 1;
		setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	}
	return (hSocket != INVALID_SOCKET);
}

void CGSocket::CloseSocket()
{
	if (hSocket != INVALID_SOCKET)
	{
		shutdown(hSocket, SHUT_WR);
		//pthread_mutex_lock(&readLock);
		pthread_mutex_lock(&writeLock);
		ClearReceivedBuffer();
		close(hSocket);
		pthread_mutex_unlock(&writeLock);
		//pthread_mutex_unlock(&readLock);
	}
	hSocket = INVALID_SOCKET;
}

int CGSocket::GetSocket() const
{
	return hSocket;
}

void CGSocket::SetSocket(int s)
{
	hSocket = s;
}

void CGSocket::GetLocalComputerName(char *pszHostName, int bufLen)
{
	memset(pszHostName, 0x00, bufLen);
	gethostname(pszHostName, bufLen);
}

void CGSocket::GetLocalIPAddress(char *pszIPAddr, int bufLen)
{
	char szHostName[256];
	struct in_addr sockAddr;
	memset(pszIPAddr, 0x00, bufLen);
	memset(&sockAddr, 0x00, sizeof(struct in_addr));
	GetLocalComputerName(szHostName, sizeof(szHostName));
	struct hostent *pHostEnt = gethostbyname(szHostName);
	memcpy(&sockAddr, pHostEnt->h_addr_list[0], sizeof(struct in_addr));
	strcpy(pszIPAddr, inet_ntoa(sockAddr));
}

int CGSocket::GetPeerInfo(unsigned short &peerPort, char *pszPeerIP, int bufLen)
{
	struct sockaddr_in sockAddr;
	memset(pszPeerIP, 0x00, bufLen);
	socklen_t sockAddrLen = sizeof(struct sockaddr);
	int errCode = getpeername(hSocket, (struct sockaddr*)&sockAddr, &sockAddrLen);
	if (errCode == 0)
	{
		peerPort = ntohs(sockAddr.sin_port);
		strcpy(pszPeerIP, inet_ntoa(sockAddr.sin_addr));
	}
	return errCode == 0;
}

int CGSocket::GetPeerComputerName(char *pszPeerName, int bufLen)
{
	struct sockaddr_in sockAddr;
	struct hostent *phostEnt = NULL;
	socklen_t sockAddrLen = sizeof(struct sockaddr_in);
	int errCode = getpeername(hSocket, (struct sockaddr*)&sockAddr, &sockAddrLen);
	if (errCode == 0)
	{
		phostEnt = gethostbyaddr((char*)&sockAddr.sin_addr.s_addr, sizeof(unsigned long), AF_INET);
		if (phostEnt)
		{
			memset(pszPeerName, 0x00, bufLen);
			strncpy(pszPeerName, phostEnt->h_name, bufLen-1);
		}
	}
	return phostEnt != NULL;
}

int CGSocket::Connect(const char *pszServerIP, unsigned short serverPort)
{
	int bRet;
	struct sockaddr_in sockAddr;
	memset(&sockAddr, 0x00, sizeof(struct sockaddr_in));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(serverPort);
	sockAddr.sin_addr.s_addr = inet_addr(pszServerIP);
	if (!(bRet = (connect(hSocket, (struct sockaddr*)&sockAddr, sizeof(struct sockaddr)) == 0)))
		CloseSocket();
	return bRet;
}

int CGSocket::Bind(unsigned short socketPort)
{
	struct sockaddr_in sockAddr;
	memset(&sockAddr, 0x00, sizeof(struct sockaddr_in));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port	= htons(socketPort);
	sockAddr.sin_addr.s_addr = INADDR_ANY;
	return (bind(hSocket, (struct sockaddr*) &sockAddr, sizeof(struct sockaddr)) != SOCKET_ERROR);
}

int CGSocket::Bind(unsigned short socketPort, const char *pszSocketAddress)
{
	struct sockaddr_in sockAddr;
	memset(&sockAddr, 0x00, sizeof(struct sockaddr_in));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(socketPort);
	sockAddr.sin_addr.s_addr = inet_addr(pszSocketAddress);
	return (bind(hSocket, (struct sockaddr*) &sockAddr, sizeof(struct sockaddr)) != SOCKET_ERROR);
}

int CGSocket::Bind(const struct sockaddr *pSockAddr, int nSockAddrLen)
{
	return (bind(hSocket, pSockAddr, nSockAddrLen) != SOCKET_ERROR);
}

int CGSocket::Listen(int socketBackLog)
{
	return (listen(hSocket,socketBackLog) != SOCKET_ERROR);
}

int CGSocket::Accept(CGSocket *pConnectedSocket, struct sockaddr *pSockAddr)
{
	socklen_t sockAddrLen = sizeof(struct sockaddr);
	int s = accept(hSocket, pSockAddr, &sockAddrLen);
	if (s != INVALID_SOCKET)
	{
		pConnectedSocket->SetSocket(s);
		return 1;
	}
	return 0;
}

long CGSocket::Read(char *pszBuf, unsigned long nBytesToRead, int flags, int nSecTimeOut)
{
	fd_set readfds;
	struct timeval *pTimeOut, timeOut;
	FD_ZERO(&readfds);
	FD_SET(hSocket, &readfds);
	if (nSecTimeOut < 0)
		pTimeOut = NULL;
	else
	{
		timeOut.tv_sec 	= nSecTimeOut;
		timeOut.tv_usec = 0;
		pTimeOut = &timeOut;
	}
	long lRet = select(hSocket + 1, &readfds, NULL, NULL, pTimeOut);
	if (0 == lRet)
		lRet = SOCKET_TIMEOUT;
	else if (lRet > 0)
	{
		if (FD_ISSET(hSocket, &readfds))
		{
			unsigned long nTotRead = 0;
			pthread_mutex_lock(&readLock);
			do
			{
				lRet = recv(hSocket, pszBuf + nTotRead, nBytesToRead - nTotRead, flags);
				if (lRet > 0)
					nTotRead += lRet;
				else if (0 == lRet)
					break;
				else
				{
					//if (ETIME == errno || EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno)
					if (EAGAIN == errno || EWOULDBLOCK == errno)
					{
						if (nTotRead > 0 && nTotRead < nBytesToRead)
						continue;
					}
					break;
				}
			} while (nTotRead < nBytesToRead);
			pthread_mutex_unlock(&readLock);
			return ( lRet <= 0 ) ? lRet : nTotRead;
		}
	}
	return lRet;
}

long CGSocket::ReadBlock(char *pszBuf, unsigned long nBytesToRead, int flags)
{
	long nTotBytesRead = 0, nBytesRead = 0;
	pthread_mutex_lock(&readLock);
	do
	{
		nBytesRead = recv(hSocket, pszBuf + nTotBytesRead, nBytesToRead - nTotBytesRead, flags);
		if (nBytesRead > 0)
			nTotBytesRead += nBytesRead;
		else
			break;
	}while ((unsigned long)nTotBytesRead < nBytesToRead);
	pthread_mutex_unlock(&readLock);
	return (nTotBytesRead > 0)?nTotBytesRead:nBytesRead;
}

long CGSocket::WriteEx(const char *pszBuf, unsigned long nBytesToWrite, int flags)
{
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(hSocket, &writefds);
	long nBytesSend = select (hSocket + 1, NULL, &writefds, NULL, NULL);
	switch (nBytesSend)
	{
	case SOCKET_ERROR:
		break;
	case 0:
		nBytesSend = SOCKET_TIMEOUT;
		break;
	default:
		if (nBytesSend > 0)
			if (FD_ISSET(hSocket, &writefds))
			{
				pthread_mutex_lock(&writeLock);
				nBytesSend = send(hSocket, pszBuf, nBytesToWrite, flags);
				pthread_mutex_unlock(&writeLock);
			}
	}
	return nBytesSend;
}

long CGSocket::Write(const char *pszBuf, unsigned long nBytesToWrite, int flags)
{
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(hSocket, &writefds);
	long nBytesSend = select (hSocket + 1, NULL, &writefds, NULL, NULL);
	switch (nBytesSend)
	{
	case SOCKET_ERROR:
		break;
	case 0:
		nBytesSend = SOCKET_TIMEOUT;
		break;
	default:
		if (nBytesSend > 0)
			if (FD_ISSET(hSocket, &writefds))
				nBytesSend = send(hSocket, pszBuf, nBytesToWrite, flags);
	}
	return nBytesSend;
}

long CGSocket::WriteBlock(const char *pszBuf, unsigned long nBytesToWrite, int flags)
{
	long nTotBytesSend = 0, nBytesSend = 0;
	pthread_mutex_lock(&writeLock);
	while ((unsigned long)nTotBytesSend < nBytesToWrite)
	{
		nBytesSend = send(hSocket, pszBuf + nTotBytesSend, nBytesToWrite - nTotBytesSend, flags);
		if (0 == nBytesSend)
			break;
		else if (-1 == nBytesSend)
		{
			if ((EAGAIN == errno) || (EWOULDBLOCK == errno) || (EINTR == errno) || (EMSGSIZE == errno) || (ENOBUFS == errno))
				continue;
			break;
		}
		nTotBytesSend += nBytesSend;
	};
	pthread_mutex_unlock(&writeLock);
	return (nTotBytesSend > 0)?nTotBytesSend:nBytesSend;
}

long CGSocket::IDXReadLine(char *data, int maxDataLen, int flags)
{
	//static int cnt = 0;
	//static char b[65536], *bp;
	char a = 0, c, *bufx = data;
	while (--maxDataLen > 0)
	{
		if (--cnt <= 0)
		{
			if ((cnt = recv(hSocket, b, sizeof(b), flags)) > 0)
				bp = b;
			else
			{
				int tcnt = cnt;
				cnt = 0;
				return tcnt;
			}
		}
		*data++ = c = *bp++;
		if (0x0d == a && 0x0a == c)
		{
			*(data) = 0x00;
			return data - bufx;
		}
		a = c;
	}
	return -1;
}

long CGSocket::ReadLine(char *data, int maxDataLen, int flags, int nSecTimeOut)
{
	//static int cnt = 0;
	//static char b[65536], *bp;
	char *bufx = data;
	while (--maxDataLen > 0)
	{
		if (--cnt <= 0)
		{
			if ((cnt = recv(hSocket, b, sizeof(b), flags)) > 0)
				bp = b;
			else
			{
				int tcnt = cnt;
				cnt = 0;
				return tcnt;
			}
		}
		if ('\n' == (*data++ = *bp++))
		{
			*(data) = 0x00;
			return data - bufx;
		}
	}
	return -1;
}

int CGSocket::IsDataWaiting() const
{
	int n = 0;
	if (ioctl(hSocket, FIONREAD, &n) < 0)
		return -1;
	return n;
}

int CGSocket::ClearReceivedBuffer()
{
	char szBuf[256];
	int nRet = 0, nTotCleared = 0;
	while ((nRet = IsDataWaiting()) > 0)
	{
		if (nRet < 0)
			return nRet;
		nRet = recv(hSocket, szBuf, sizeof(szBuf), 0);
		if (0 == nRet)
			return -3;
		else if (nRet < 0)
			return nRet;
		else 
			nTotCleared += nRet;
	}
	return (nRet < 0) ? nRet : nTotCleared;
}
