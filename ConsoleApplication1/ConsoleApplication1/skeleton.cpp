#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "skeleton.hpp"
#include <semaphore.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <errno.h>


using namespace std;
std::string msgKinectData;



cv::Point Skeleton::changeCoordinates(Joint joint[JointType::JointType_Count], int type)
{
	ColorSpacePoint colorSpacePoint = { 0 };
	pCoordinateMapper->MapCameraPointToColorSpace(joint[type].Position, &colorSpacePoint);
	int x = static_cast<int>(colorSpacePoint.X);
	int y = static_cast<int>(colorSpacePoint.Y);

	return cv::Point(x, y);
}



void Skeleton::drawSkeleton(cv::Mat canvas, Joint joint[JointType::JointType_Count])
{
	cv::line(canvas, changeCoordinates(joint, JointType_Head), changeCoordinates(joint, JointType_Neck), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_Neck), changeCoordinates(joint, JointType_SpineShoulder), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_SpineShoulder), changeCoordinates(joint, JointType_ShoulderLeft), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_SpineShoulder), changeCoordinates(joint, JointType_ShoulderRight), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_SpineShoulder), changeCoordinates(joint, JointType_SpineMid), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_ShoulderLeft), changeCoordinates(joint, JointType_ElbowLeft), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_ShoulderRight), changeCoordinates(joint, JointType_ElbowRight), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_ElbowLeft), changeCoordinates(joint, JointType_WristLeft), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_ElbowRight), changeCoordinates(joint, JointType_WristRight), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_WristLeft), changeCoordinates(joint, JointType_HandLeft), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_WristRight), changeCoordinates(joint, JointType_HandRight), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_HandLeft), changeCoordinates(joint, JointType_HandTipLeft), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_HandRight), changeCoordinates(joint, JointType_HandTipRight), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_WristLeft), changeCoordinates(joint, JointType_ThumbLeft), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_WristRight), changeCoordinates(joint, JointType_ThumbRight), GREEN, 6);
	cv::line(canvas, changeCoordinates(joint, JointType_SpineMid), changeCoordinates(joint, JointType_SpineBase), GREEN, 6);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// If the lower body is not in the frame, make sure skeleton lines are not drawn from the origin (0,0)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if ((changeCoordinates(joint, JointType_SpineBase).x > 0) && ((changeCoordinates(joint, JointType_HipLeft).y > 0))) {
		cv::line(canvas, changeCoordinates(joint, JointType_SpineBase), changeCoordinates(joint, JointType_HipLeft), GREEN, 6);
	}

	if ((changeCoordinates(joint, JointType_SpineBase).x > 0) && ((changeCoordinates(joint, JointType_HipRight).y > 0))) {
		cv::line(canvas, changeCoordinates(joint, JointType_SpineBase), changeCoordinates(joint, JointType_HipRight), GREEN, 6);
	}

	if ((changeCoordinates(joint, JointType_HipLeft).x > 0) && ((changeCoordinates(joint, JointType_KneeLeft).y > 0))) {
		cv::line(canvas, changeCoordinates(joint, JointType_HipLeft), changeCoordinates(joint, JointType_KneeLeft), GREEN, 6);
	}

	if ((changeCoordinates(joint, JointType_HipRight).x > 0) && ((changeCoordinates(joint, JointType_KneeRight).y > 0))) {
		cv::line(canvas, changeCoordinates(joint, JointType_HipRight), changeCoordinates(joint, JointType_KneeRight), GREEN, 6);
	}
	
	if ((changeCoordinates(joint, JointType_KneeLeft).x > 0) && ((changeCoordinates(joint, JointType_AnkleLeft).y > 0))) {
		cv::line(canvas, changeCoordinates(joint, JointType_KneeLeft), changeCoordinates(joint, JointType_AnkleLeft), GREEN, 6);
	}

	if ((changeCoordinates(joint, JointType_KneeRight).x > 0) && ((changeCoordinates(joint, JointType_AnkleRight).y > 0))) {
		cv::line(canvas, changeCoordinates(joint, JointType_KneeRight), changeCoordinates(joint, JointType_AnkleRight), GREEN, 6);
	}

	if ((changeCoordinates(joint, JointType_AnkleLeft).x > 0) && ((changeCoordinates(joint, JointType_FootLeft).y > 0))) {
		cv::line(canvas, changeCoordinates(joint, JointType_AnkleLeft), changeCoordinates(joint, JointType_FootLeft), GREEN, 6);
	}

	if ((changeCoordinates(joint, JointType_AnkleRight).x > 0) && ((changeCoordinates(joint, JointType_FootRight).y > 0))) {
		cv::line(canvas, changeCoordinates(joint, JointType_AnkleRight), changeCoordinates(joint, JointType_FootRight), GREEN, 6);
	}
} 



void Skeleton::skeletonTracking()
{
	// Variables
	double depthmin;
	double depthmax;
	cv::Mat adjMap;
	signed int move_direction = -1;
	signed int hand_state = -1;


	// Use optimization
	cv::setUseOptimized(true);

    // sensor
    HRESULT hResult = S_OK;
    hResult = GetDefaultKinectSensor(&pSensor);
    if (FAILED(hResult))
    {
        std::cerr << "Error: GetDefaultKinectSensor" << std::endl;
        exit(-1);
    }

    hResult = pSensor->Open();
    if (FAILED(hResult))
    {
        std::cerr << "Error: IKinectSensor::Open()" << std::endl;
        exit(-1);
    }

    // Source
    hResult = pSensor->get_ColorFrameSource(&pColorSource);
    if (FAILED(hResult))
    {
        std::cerr << "Error: IKinectSensor::get_ColorFrameSource()" << std::endl;
        exit(-1);
    }

    hResult = pSensor->get_BodyFrameSource(&pBodySource);
    if (FAILED(hResult))
    {
        std::cerr << "Error: IKinectSensor::get_BodyFrameSource()" << std::endl;
        exit(-1);
    }

    hResult = pSensor->get_DepthFrameSource(&pDepthSource);
    if (FAILED(hResult))
    {
        std::cerr << "Error: IKinectSensor::get_DepthFrameSource()" << std::endl;
        exit(-1);
    }

    // Reader
    hResult = pColorSource->OpenReader(&pColorReader);
    if (FAILED(hResult))
    {
        std::cerr << "Error: IColorFrameSource::OpenReader()" << std::endl;
        exit(-1);
    }

    hResult = pBodySource->OpenReader(&pBodyReader);
    if (FAILED(hResult))
    {
        std::cerr << "Error: IBodyFrameSource::OpenReader()" << std::endl;
        exit(-1);
    }

    hResult = pDepthSource->OpenReader(&pDepthReader);
    if (FAILED(hResult))
    {
        std::cerr << "Error: IDepthFrameSource::OpenReader()" << std::endl;
        exit(-1);
    }

    // Description
    hResult = pColorSource->get_FrameDescription(&pColorDescription);
    if(FAILED(hResult))
    {
        std::cerr << "Error: IColorFrameSource::get_FrameDescription()" << std::endl;
        exit(-1);
    }

    hResult = pDepthSource->get_FrameDescription(&pDepthDescription);
    if (FAILED(hResult))
    {
        std::cerr << "Error: IDepthFrameSource::get_FrameDescription()" << std::endl;
        exit(-1);
    }


    colorWidth = 0;
    colorHeight = 0;
    depthWidth = 0;
    depthHeight = 0;
	pColorDescription->get_Width(&colorWidth);   // 1920
	pColorDescription->get_Height(&colorHeight); // 1080
	pDepthDescription->get_Width(&depthWidth);   // 512
	pDepthDescription->get_Height(&depthHeight); // 424
	unsigned int colorbufferSize = colorWidth * colorHeight * 4 * sizeof(unsigned char);
	unsigned int depthbufferSize = depthWidth * depthHeight * 4 * sizeof(unsigned char);

	cv::Mat colorBufferMat(colorHeight, colorWidth, CV_8UC4);
	cv::Mat depthBufferMat(depthHeight, depthWidth, CV_16UC1);
	cv::Mat bodyMat(colorHeight / 2, colorWidth / 2, CV_8UC4);
	cv::Mat depthMat(depthHeight, depthWidth, CV_8UC1);
	std::string colorWinName = "Skeleton RGB";
	std::string depthWinName = "Skeleton Depth";
	cv::namedWindow(colorWinName);
	cv::namedWindow(depthWinName);

	// Color Table
	cv::Vec3b color[BODY_COUNT];
	color[0] = cv::Vec3b(255, 0, 0); //R
	color[1] = cv::Vec3b(0, 255, 0); //G
	color[2] = cv::Vec3b(0, 0, 255); //B
	color[3] = cv::Vec3b(255, 255, 0);
	color[4] = cv::Vec3b(255, 0, 255);
	color[5] = cv::Vec3b(0, 255, 255);

	// Range (Range of Depth is 500-8000[mm], Range of Detection is 500-45000[mm])
	unsigned short min = 0;
	unsigned short max = 0;
	pDepthSource->get_DepthMinReliableDistance(&min);
	pDepthSource->get_DepthMaxReliableDistance(&max);
	//  std::cout << "Range: " << min << " - " << max << std::endl;

	// Coordinate Mapper
	hResult = pSensor->get_CoordinateMapper(&pCoordinateMapper);
	if (FAILED(hResult))
	{
		std::cerr << "Error: IKinectSensor::get_CoordinateMapper()" << std::endl;
		exit(-1);
	}



while (1)
{
	// Add small delay
	Sleep(1);
	
	// Frame
	IColorFrame* pColorFrame = 0;
	IDepthFrame* pDepthFrame = 0;
	hResult = pDepthReader->AcquireLatestFrame(&pDepthFrame);
	if (SUCCEEDED(hResult))
	{
		hResult = pDepthFrame->AccessUnderlyingBuffer(&depthbufferSize, reinterpret_cast<UINT16**>(&depthBufferMat.data));
		if (SUCCEEDED(hResult))
			depthBufferMat.convertTo(depthMat, CV_8U, -255.0f / 8000.0f, 255.0f);
	}
	hResult = pColorReader->AcquireLatestFrame(&pColorFrame);
	if (SUCCEEDED(hResult))
	{
		hResult = pColorFrame->CopyConvertedFrameDataToArray(colorbufferSize, reinterpret_cast<BYTE*>(colorBufferMat.data), ColorImageFormat::ColorImageFormat_Bgra);
		if (SUCCEEDED(hResult))
			cv::resize(colorBufferMat, bodyMat, cv::Size(), 0.5, 0.5);
	}
	// SafeRelease(pColorFrame);

	IBodyFrame* pBodyFrame = 0;
	hResult = pBodyReader->AcquireLatestFrame(&pBodyFrame);
	if (SUCCEEDED(hResult))
	{
		IBody* pBody[BODY_COUNT] = { 0 };
		hResult = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, pBody);
		if (SUCCEEDED(hResult))
		{
			//for (int count = 0; count < 1; count++) //track one person only
			for (int count = 0; count < BODY_COUNT; count++) //track multiple people
			{
				BOOLEAN bTracked = false;
				hResult = pBody[count]->get_IsTracked(&bTracked);
				if (SUCCEEDED(hResult) && bTracked)
				{

					/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					Joint joint[JointType::JointType_Count];
					std::ostringstream strstream;


					// BOTH HANDS DOWN
					if ((joint[21].Position.Y < joint[0].Position.Y) && (joint[23].Position.Y < joint[0].Position.Y))
					{
						// SIGNAL: STOP
				//		printf("\nSTOP");
						move_direction = 0;
					}

					
					// HANDS BETWEEN NECK AND SPINE BASE
					else if ((joint[21].Position.Y > joint[0].Position.Y) && (joint[23].Position.Y > joint[0].Position.Y) && (joint[21].Position.Y < joint[3].Position.Y) && (joint[23].Position.Y < joint[3].Position.Y))
					{
							
						// SIGNAL: FORWARD or BACKWARD
						if ((abs(joint[21].Position.X - joint[23].Position.X) < 0.4) && (abs(joint[21].Position.X - joint[23].Position.X) > 0.1) && (abs(joint[21].Position.Y - joint[23].Position.Y) < 0.1))
						{

							
							// SIGNAL: FORWARD
							if ((abs(joint[21].Position.Z - joint[1].Position.Z) > 0.35) && (abs(joint[23].Position.Z - joint[1].Position.Z) > 0.35))
							{
					//			printf("\nFORWARD");
								move_direction = 1;
							}
							
							
							// SIGNAL: BACKWARD
							else if ((abs(joint[21].Position.Z - joint[1].Position.Z) < 0.35) && (abs(joint[23].Position.Z - joint[1].Position.Z) < 0.35))
							{
					//			printf("\nBACKWARD");
								move_direction = 4;
							}
						}



						// SIGNAL: LEFT OR RIGHT
						else if ((abs(joint[21].Position.Y - joint[23].Position.Y) > 0.05) && (abs(joint[21].Position.Y - joint[23].Position.Y) < 0.3))
						{
						
							// SIGNAL: LEFT
							if (joint[21].Position.Y > joint[23].Position.Y)
							{
					//			printf("\nLEFT");
								move_direction = 3;
							}


							// SIGNAL: RIGHT
							else if (joint[21].Position.Y < joint[23].Position.Y)
							{
					//			printf("\nRIGHT");
								move_direction = 2;
							}
						}
					}
					/////////////////////////////////////////////////////////////////////////////////////////////////////////////////



					hResult = pBody[count]->GetJoints(JointType::JointType_Count, joint);
					if (SUCCEEDED(hResult) && TrackingConfidence_High)
					{
						// Joints
						for (int type = 0; type < JointType::JointType_Count; type++)
						{
	
							// HAND STATE: OPEN or CLOSED
							if (joint[type].JointType == JointType::JointType_HandRight)
							{
								cv::Point jointPoint = changeCoordinates(joint, type);
								pBody[count]->get_HandRightState(&handState); // << "," << pBody[count]->get_HandRightConfidence(&handConfidence);

								// Hand is open
								if (handState == HandState::HandState_Open) {
									hand_state = 0;
									cv::circle(colorBufferMat, jointPoint, 80, static_cast<cv::Scalar>(color[0]), -1, CV_AA);
								}
								// Hand is closed
								else if (handState == HandState::HandState_Closed) {
									hand_state = 1;
									cv::circle(colorBufferMat, jointPoint, 80, static_cast<cv::Scalar>(color[2]), -1, CV_AA);
								}
							}


							/*
							// HAND STATE
							if (joint[type].JointType == JointType::JointType_HandLeft)
							{
							pBody[count]->get_HandLeftState(&handState); // << "," << pBody[count]->get_HandLeftConfidence(&handConfidence);

							if (handState == HandState::HandState_Open) {
							std::cout << "\nLeft: open";
							}
							else if (handState == HandState::HandState_Lasso) {
							std::cout << "\nLeft: lasso";
							}
							else if (handState == HandState::HandState_Closed) {
							std::cout << "\nLeft: closed";
							}
							}
							*/



							// Draw Skeleton
							cv::Point jointPoint = changeCoordinates(joint, type);
							if ((jointPoint.x >= 0) && (jointPoint.y >= 0) && (jointPoint.x < colorWidth) && (jointPoint.y < colorHeight))
							{
								cv::circle(colorBufferMat, jointPoint, 16, static_cast<cv::Scalar>(color[4]), -1, CV_AA);
							}
						}
						drawSkeleton(colorBufferMat, joint);
					}



					///////////////////////////////////////////////////////////////
					// Send values to Pi
					///////////////////////////////////////////////////////////////
					if (hand_state == 0) {
						// Hand is open
						strstream << move_direction << "," << joint[1].Position.Z - joint[23].Position.Z << "," << joint[23].Position.Y << "," << joint[10].Position.X << "," << joint[10].Position.Y << "," << joint[6].Position.X << "," << joint[6].Position.Y << "," << "0";
						std::cout << "\n" << move_direction << "," << joint[1].Position.Z - joint[23].Position.Z << "," << joint[23].Position.Y << "," << joint[10].Position.X << "," << joint[10].Position.Y << "," << joint[6].Position.X << "," << joint[6].Position.Y << "," << "0";
					}
					else if (hand_state == 1) {
						// Hand is closed
						strstream << move_direction << "," << joint[1].Position.Z - joint[23].Position.Z << "," << joint[23].Position.Y << "," << joint[10].Position.X << "," << joint[10].Position.Y << "," << joint[6].Position.X << "," << joint[6].Position.Y << "," << "1";
						std::cout << "\n" << move_direction << "," << joint[1].Position.Z - joint[23].Position.Z << "," << joint[23].Position.Y << "," << joint[10].Position.X << "," << joint[10].Position.Y << "," << joint[6].Position.X << "," << joint[6].Position.Y << "," << "1";
					}
					msgKinectData = strstream.str();
					strstream.str(std::string());
					strstream.clear();
					///////////////////////////////////////////////////////////////



					// Lean
					PointF amount;
					hResult = pBody[count]->get_Lean(&amount);
//					if (SUCCEEDED(hResult))
//						std::cout << "amount: " << amount.X << ", " << amount.Y << std::endl;
				}
			}
			cv::resize(colorBufferMat, bodyMat, cv::Size(), 0.5, 0.5);
		}
		for (int count=0; count<BODY_COUNT; count++)
		{
			SafeRelease(pBody[count]);
		}
	}
	SafeRelease(pColorFrame);
	SafeRelease(pBodyFrame);
	SafeRelease(pDepthFrame);


	// Depth image with colors
	cv::Mat depthImage;
	equalizeHist(depthMat, depthImage);

//	cv::minMaxIdx(depthImage, &depthmin, &depthmax);
	// expand your range to 0..255. Similar to histEq();
//	depthImage.convertTo(adjMap, CV_8UC1, 255 / (depthmax - depthmin), -depthmin);

	// this is great. It converts your grayscale image into a tone-mapped one, 
	// much more pleasing for the eye
	// function is found in contrib module, so include contrib.hpp 
	// and link accordingly
	cv::Mat falseColorsMap;
	applyColorMap(depthImage, falseColorsMap, cv::COLORMAP_JET);


	// save color image, depth image
	cv::imshow(colorWinName, bodyMat);
	cv::imshow(depthWinName, falseColorsMap);
//	cv::imshow(depthWinName, depthImage);


	if (cv::waitKey(10) == VK_ESCAPE)
			break;
    }
	
    SafeRelease(pColorSource);
    SafeRelease(pBodySource);
    SafeRelease(pColorReader);
    SafeRelease(pBodyReader);
    SafeRelease(pColorDescription);
    SafeRelease(pCoordinateMapper);
    SafeRelease(pDepthSource);
    SafeRelease(pDepthReader);
    SafeRelease(pDepthDescription);
    if (pSensor)
        pSensor->Close();
    SafeRelease(pSensor);
    cv::destroyAllWindows();
}

