/*----------------------------------------------------------------- */
/*    Skeleton Tracking Using Kinect V2 for Windows c++ version     */
/*                                                                  */
/*  Author:  Jawad Nagi                                             */
/*  Date:    09/18/2016                                             */
/*----------------------------------------------------------------- */


#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#undef UNICODE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <wspiapi.h>
#include "skeleton.hpp"
#include <semaphore.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <errno.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "51715"


// Global variables
std::string ipaddress;
int ConnectSocket, iResult;
pthread_t thSendData, thReceiveData;
extern std::string msgKinectData;

using namespace std;



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void *sendDataToPi(void *myptr)
{
	// create WSADATA object
	WSADATA wsaData;

	// socket
	ConnectSocket = INVALID_SOCKET;

	// holds address info for socket to connect to
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		exit(1);
	}


	// set address info
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;  //TCP connection!!!


	//resolve server address and port
	iResult = getaddrinfo(ipaddress.c_str(), DEFAULT_PORT, &hints, &result);

	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		exit(1);
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			exit(1);
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

		if (iResult == SOCKET_ERROR)
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			printf("The server is down... did not connect");
		}
	}


	// no longer need address info for server
	freeaddrinfo(result);


	// if connection failed
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		exit(1);
	}


	// Set the mode of the socket to be nonblocking
	u_long iMode = 1;

	iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		exit(1);
	}

	//disable nagle
	char value = 1;
	setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
	printf("Connected to server.\n");

/*
	stringstream strstream;
	strstream << "Handshake client-to-server";
	std::string msg = strstream.str();
	strstream.str(std::string());
	strstream.clear();

	// Send an initial buffer
	iResult = send(ConnectSocket, msg.c_str(), (int)strlen(msg.c_str()), 0);
	if (iResult == SOCKET_ERROR) {
		printf("*** Send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}
*/
	printf("Confirmation sent successfully to server.");


	while (true)
	{
		const char* szMsg = msgKinectData.c_str();
		iResult = send(ConnectSocket, szMsg, strlen(szMsg), 0);

		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			pthread_exit(0);
		}
		else
		{
//			printf("Bytes sent: %ld\n", iResult);
//			printf("Sent: %s\n", szMsg);
		}
		Sleep(200);
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void *receiveDataFromPi(void *ptr)
{
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	while (true)
	{
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			char msg[DEFAULT_BUFLEN];
			memset(&msg, 0, sizeof(msg));
			strncpy_s(msg, recvbuf, iResult);
//			printf("Bytes received: %d\n", iResult);
			printf("Received: %s\n", msg);
		}
		else if (iResult <= 0) {
			printf("Connection closed.\n");
		}
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
		}
		Sleep(100);
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("\nUsage: ConsoleApplication1.exe <ip-address-of-linux-server>");
	}

	// Create thread to send data to PI
	ipaddress = argv[1];
	pthread_create(&thSendData, NULL, sendDataToPi, NULL);
	//	pthread_create(&thReceiveData, NULL, receiveDataFromPi, NULL);

	// Run skeleton tracking system
	Skeleton mySkeleton;
	mySkeleton.skeletonTracking();

	return 0;
}

