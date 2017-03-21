/* A simple server in the internet domain using TCP.
myServer.c
D. Thiebaut
Adapted from http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
The port number used in 51717.
This code is compiled and run on the Raspberry as follows:
   

Compile:   g++ server-qbot.c `pkg-config --cflags --libs python2` -o server-qbot
Run:       ./server-qbot pythonCPPInterface getKinectData


Website:   https://github.com/Frogee/PythonCAPI_testing

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

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <python2.7/Python.h>
#include </usr/lib/python2.7/dist-packages/numpy/core/include/numpy/arrayobject.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 51715


pthread_t thConnectToClient;
pthread_t thReceiveFromKinect;
struct sockaddr_in client;
struct sockaddr_in server;
int ConnectSocket, ListenSocket, reuse = 1;


using namespace std;
static double xarr[] = {0,0,0,0,0,0,0,0};
PyObject *pName, *pModule, *pDict, *pFunc;
PyObject *pArgTuple, *pValue, *pXVec;



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int countchars(char* s, char c)
{
	return *s == '\0'
		? 0
		: countchars(s+1,c) + (*s == c);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int runPython(int argc, char* argv[])
{

//	std::cout << "Hello from runPython()" << std::endl;	
	Py_Initialize();
	
	// Set the path to include the current directory in case the module is located there. Found from
	// http://stackoverflow.com/questions/7624529/python-c-api-doesnt-load-module
	// and http://stackoverflow.com/questions/7283964/embedding-python-into-c-importing-modules
	PyObject *sys = PyImport_ImportModule("sys");
	PyObject *path = PyObject_GetAttrString(sys, "path");
	PyList_Append(path, PyString_FromString("."));
	
	pName = PyString_FromString(argv[1]);   //Get the name of the module
	pModule = PyImport_Import(pName);     //Get the module
	Py_DECREF(pName);	


	if (pModule != NULL) {
		pFunc = PyObject_GetAttrString(pModule, argv[2]);   //Get the function by its name
		/* pFunc is a new reference */
		
		if (pFunc && PyCallable_Check(pFunc)) {


			//Set up a tuple that will contain the function arguments. In this case, the
			//function requires two tuples, so we set up a tuple of size 2.
			pArgTuple = PyTuple_New(1);

			//Make some vectors containing the data
			std::vector<double> xvec (xarr, xarr + sizeof(xarr) / sizeof(xarr[0]) );


			//Transfer the C++ vector to a python tuple
			pXVec = PyTuple_New(xvec.size());	
			for (int i = 0; i < xvec.size(); ++i) {
				pValue = PyFloat_FromDouble(xvec[i]);
				if (!pValue) {
					Py_DECREF(pXVec);
					Py_DECREF(pModule);
					fprintf(stderr, "Cannot convert array value\n");
					return 1;
				}
				PyTuple_SetItem(pXVec, i, pValue);
			}

			//Set the argument tuple to contain the two input tuples
			PyTuple_SetItem(pArgTuple, 0, pXVec);
//		  	Py_DECREF(pArgTuple);

			//Call the python function
			pValue = PyObject_CallObject(pFunc, pArgTuple);		
			Py_DECREF(pArgTuple);
			Py_DECREF(pXVec);

			if (pValue != NULL) {
//				printf("Result of call: %ld\n", PyLong_AsLong(pValue));
				Py_DECREF(pValue);
			}

			//Some error catching
			else {
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				fprintf(stderr,"Call failed\n");
				return 1;
			}
		}
	}
/*
		else {
			if (PyErr_Occurred())
				PyErr_Print();
			fprintf(stderr, "Cannot find function \"%s\"\n", argv[2]);
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}

	else {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", argv[1]);
		return 1;
	}
//	Py_Finalize();
*/

	return 0;
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
	printf("Confirmation sent to client successfully");
	printf("Sent: %s\n", msg.c_str());
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

		// Print received data!
		printf("\nReceived C++: %s\n", buffer);

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
		

		for (int i=0; i<totalwords; i++)
		{
			xarr[i]=data[i];
//			printf("\nCopied data: %f", xarr[i]);
		}

	       //////////////////////////////////////////
               ///////////// PYTHON INTERFACE
               //////////////////////////////////////////
               runPython(argc, argv);

	}

	// Create a small delay in the loop
	usleep(200);
    }
}



