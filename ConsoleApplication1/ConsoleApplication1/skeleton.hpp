#ifndef SKELETON_H_
#define SKELETON_H_

#include <Windows.h>
#include "C:\Program Files\Microsoft SDKs\Kinect\v2.0_1409\inc\Kinect.h"
#include <opencv2/opencv.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>



template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
    if(pInterfaceToRelease != 0)
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = 0;
    }
}


class Skeleton{
public:
	void skeletonTracking();
	void drawSkeleton(cv::Mat canvas, Joint joint[JointType::JointType_Count]);
	const cv::Scalar GREEN = cv::Scalar(0, 255, 0);
	cv::Point changeCoordinates(Joint joint[JointType::JointType_Count], int type);
	HandState handState;
	TrackingConfidence handConfidence;

private:
	int colorWidth;                                   // width for RGB Image
	int colorHeight;
	int depthWidth;                                   // width for Depth Image
	int depthHeight;
	IKinectSensor* pSensor;
	IColorFrameSource* pColorSource;
	IBodyFrameSource* pBodySource;
	IDepthFrameSource* pDepthSource;
	IColorFrameReader* pColorReader;
	IBodyFrameReader* pBodyReader;
	IDepthFrameReader* pDepthReader;
	IFrameDescription* pColorDescription;
	IFrameDescription* pDepthDescription;
	ICoordinateMapper* pCoordinateMapper;
};


#endif /* SKELETON_H_ */
