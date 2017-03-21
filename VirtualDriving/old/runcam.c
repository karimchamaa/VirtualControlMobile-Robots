// Compile:  g++ runcam.c `pkg-config --libs opencv` -o runcam

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std;
using namespace cv;


int main(int argc, char *argv[])
{
    Mat frame;

    VideoCapture cap(0);
    cap >> frame;
//  cap.set(CV_CAP_PROP_FOURCC,CV_FOURCC('H','2','6','4'));
//  cap.set(CV_CAP_PROP_FOURCC,CV_FOURCC('M','J','P','G'));
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,360);
    cap.set(CV_CAP_PROP_FPS, 30);

    namedWindow("Frame");

    int f = 0;
    bool rec = false;

    time_t start,end;
    time(&start);
    int counter=0;

    for(;;)
    {
        cap >> frame;

        if(rec) {
            ostringstream filename;
            filename << "frame" << f << ".png";
            imwrite(filename.str().c_str(),frame);
            f++;
            ellipse( frame, Point( 100, 100 ),Size( 50,50),0, 0,360,Scalar( 0, 0, 255 ),20,8 );
        }
        imshow("Frame",frame);

        time(&end);
        ++counter;
        double sec=difftime(end,start);
        double fps=counter/sec;
//	if (fps != "inf") {
		printf("\n%.2lf fps",fps);
//	}

        if(waitKey(30) == 27) break;
        if(waitKey(30) == 'r') rec = !rec;
    }
    return 0;
}


