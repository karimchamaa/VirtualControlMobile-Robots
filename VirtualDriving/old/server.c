/* A simple server in the internet domain using TCP.
myServer.c
D. Thiebaut
Adapted from http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
The port number used in 51717.
This code is compiled and run on the Raspberry as follows:
   
    g++ -o myServer myServer.c
    g++ -pthread -o server server.c
    ./myServer

The server waits for a connection request from a client.
The server assumes the client will send positive integers, which it sends back multiplied by 2.
If the server receives -1 it closes the socket with the client.
If the server receives -2, it exits.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <cmath>
#include <assert.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <cstring>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 51718


pthread_t thConnectToClient;
pthread_t thReceiveFromKinect;
struct sockaddr_in client;
struct sockaddr_in server;
int ConnectSocket, ListenSocket, reuse = 1;


using namespace std;



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int countchars(char* s, char c)
{
	return *s == '\0'
		? 0
		: countchars(s+1,c) + (*s == c);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




int main(int argc, char *argv[])
{
    std::vector<float> data;
    char buffer[DEFAULT_BUFLEN];
    socklen_t addr_size = sizeof(server);
    int c = sizeof(struct sockaddr_in);
    memset(&client, 0, sizeof(client));
    memset(&server, 0, sizeof(server));



    //////////////////////////////////////////
    ///////////// RUN SERVER
    //////////////////////////////////////////

    //Prepare the sockaddr structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);
    
 
    //Create socket
    ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ListenSocket == -1)
    {
        printf("\nCould not create server socket.\n");
	return 0;
    }


//------------------------------------------------------------------------------------------------------------//
//	setsockopt(ListenSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&reuse, sizeof(reuse));
	if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0) {
		printf("SERVER: setsockopt(SO_REUSEADDR) failed.");
		return 0;
	}
#ifdef SO_REUSEPORT
	if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEPORT, (const char *)&reuse, sizeof(reuse)) < 0) {
		printf("SERVER: setsockopt(SO_REUSEPORT) failed.");
		return 0;
	}
#endif
//------------------------------------------------------------------------------------------------------------//

     
    //Bind
    if (bind(ListenSocket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("\nServer bind failed.\n");
	return 0;
    }


    //Listen
    listen(ListenSocket, 5);
     
    //Accept incoming connections
    printf("SERVER: Waiting for incoming client connection requests...\n");




    //////////////////////////////////////////
    ///////////// CONNECT TO CLIENT
    //////////////////////////////////////////
    if ((ConnectSocket = accept(ListenSocket, (struct sockaddr*)&client, (socklen_t*)&c)))
    {

//------------------------------------------------------------------------------------------------------------//
//	setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&reuse, sizeof(reuse));
	if (setsockopt(ConnectSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
	{
		printf("CLIENT: setsockopt(SO_REUSEADDR) failed.");
		return 0;
	}
#ifdef SO_REUSEPORT
	if (setsockopt(ConnectSocket, SOL_SOCKET, SO_REUSEPORT, (const char *)&reuse, sizeof(reuse)) < 0)
	{
		printf("CLIENT: setsockopt(SO_REUSEPORT) failed.");
		return 0;
	}
#endif
//------------------------------------------------------------------------------------------------------------//

	printf("\nClient successfully connected to server.\n");
    }

    
    else if (ConnectSocket < 0)
    {
	printf("\nClient request to connect to server failed.\n");
	close(ListenSocket);
	return 0;
    }



    //////////////////////////////////////////
    ///////////// SEND CONFIRMATION TO CLIENT
    /////////////////////////////////////////
    stringstream strstream;
    strstream << "Handshake server-to-client";
    std::string msg = strstream.str();
    strstream.str(std::string());
    strstream.clear();
    ssize_t bytes_sent = send(ConnectSocket, msg.c_str(), strlen(msg.c_str()), 0);


    // Unable to send
    if (bytes_sent <= 0) {
	printf("Send failed to client.\n");
	return 0;
		
    // Suceeded to send
    } else if (bytes_sent > 0) {
	printf("Confirmation sent to client successfully: %d bytes\n", bytes_sent);
	printf("Message sent: %s\n", msg.c_str());
    }




    //////////////////////////////////////////
    ///////////// RECEIVE DATA FROM CLIENT
    /////////////////////////////////////////

    while (true)
    {
    	memset(buffer, 0, DEFAULT_BUFLEN); // Very important! Clear junk and garbage in buffer
	ssize_t bytes_received = recv(ConnectSocket, buffer, DEFAULT_BUFLEN, 0);


	if (bytes_received <= 0) {
		printf("\nSERVER: Unable to receive data from CLIENT.\n");
		return 0;
	}


	else if (bytes_received > 0)
	{
/*
		// Count elements in string
		int c = countchars(buffer, ',');

		// Split csv string
		char *pch;
		data.resize(40); // max elements 40

		// Copy strings into std::vector
		int totalwords = 0;
		pch = strtok(buffer, ",");
		while (pch != NULL)
		{
			data[totalwords] = atof(pch);
			//printf("str: %f\n, idx: %d",data[totalwords], totalwords);
			pch = strtok(NULL, ",");
			totalwords++;
		}
		
		// Declare vector of size "totalwords" and copy data into that, and then assign to variables for processing
		data.resize(totalwords); //i<data.size();
*/		
		// Print received data!!
		printf("Message received: %s\n", buffer);
	}

	// Create a small delay in the loop
	usleep(500);
    }
}


