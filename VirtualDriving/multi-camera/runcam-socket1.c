
// Compile:  g++ runcam-socket.c `pkg-config --libs opencv` -o runcam-socket

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Standard files
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "limits.h"
#include "float.h"
#include "time.h"
#include <cmath>
#include <string>
#include <assert.h>
#include <iostream>
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


//Linux Sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

// POSIX threads
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>

// Sleep function
#include <unistd.h>


using namespace std;
using namespace cv;

#define VIDEO_STREAM_PORT "17511"



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
	printf("\nUsage: ./runcam-socket <ip-address-of-receiving-pc>");
    }

    cv::Mat frame;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int sockfd, portno, bytes;
    std::string windowName = "Stream";

    VideoCapture cap(0);
    cap >> frame;
    cap.set(CV_CAP_PROP_FRAME_WIDTH,320); //640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,240); //360);
    cap.set(CV_CAP_PROP_FPS, 30);

    namedWindow(windowName, CV_WINDOW_AUTOSIZE);
    int f = 0;
    bool rec = false;

    time_t start,end;
    time(&start);
    int counter=0;


    portno = atoi(VIDEO_STREAM_PORT);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0)  {
        fprintf(stderr,"\nERROR opening socket");
    }
    			
    server = gethostbyname(argv[1]);

    if (server == NULL) {
	fprintf(stderr,"\nERROR, no such host");
	exit(0);
    }


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);


    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
       	fprintf(stderr,"\nERROR connecting");
    }



    //////////////////////////////////////////////////////////////////////////////////////////////////////
    for(;;)
    {
        cap >> frame;
//      imshow(windowName, frame);

        time(&end);
        ++counter;
        double sec=difftime(end,start);
        double fps=counter/sec;
	
//	if (fps != "inf") {
	printf("\n%.2lf fps",fps);
//	}


//	if(!frame.data) {
//        	cout <<  "\nCould not open or find the image" << std::endl ;
//       	return -1;
//    	}

//    	frame = (frame.reshape(0,1));
//	int imgSize = frame.total()*frame.elemSize();
	bytes = send(sockfd, frame.data, frame.total()*frame.elemSize(), 0);
    		
	if (bytes >= 0) {
//		printf("\nBytes sent: %d", bytes);
	}
	else if (bytes < 0) {
		fprintf(stderr,"\nERROR writing to socket");
		close(sockfd);
	}

//      cv::waitKey(1);
    }

    return 0;
}

