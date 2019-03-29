/*************************
OpenNI2 Deep, Color and Fusion Image
Author: Xin Chen, 2013.2
Blog: http://blog.csdn.net/chenxin_130
*************************/

#include <stdlib.h>
#include <iostream>
#include <string>
#include "OpenNI.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace std;
using namespace cv;
using namespace openni;

void CheckOpenNIError(Status result, string status)
{
	if (result != STATUS_OK)
		cerr << status << " Error: " << OpenNI::getExtendedError() << endl;
}

int main(int argc, char** argv)
{
	Status result = STATUS_OK;

	//OpenNI2 image
	VideoFrameRef oniDepthImg;
	VideoFrameRef oniColorImg;

	//OpenCV image
	cv::Mat cvDepthImg;
	cv::Mat cvBGRImg;
	cv::Mat cvFusionImg;

	cv::namedWindow("depth");
	cv::namedWindow("image");
	cv::namedWindow("fusion");
	char key = 0;

	//¡¾1¡¿
	// initialize OpenNI2
	result = OpenNI::initialize();
	CheckOpenNIError(result, "initialize context");

	// open device
	Device device;
	result = device.open(openni::ANY_DEVICE);

	//¡¾2¡¿
	// create depth stream
	VideoStream oniDepthStream;
	result = oniDepthStream.create(device, openni::SENSOR_DEPTH);

	//¡¾3¡¿
	// set depth video mode
	VideoMode modeDepth;
	modeDepth.setResolution(640, 480);
	modeDepth.setFps(30);
	modeDepth.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);
	oniDepthStream.setVideoMode(modeDepth);
	// start depth stream
	result = oniDepthStream.start();

	// create color stream
	VideoStream oniColorStream;
	result = oniColorStream.create(device, openni::SENSOR_COLOR);
	// set color video mode
	VideoMode modeColor;
	modeColor.setResolution(640, 480);
	modeColor.setFps(30);
	modeColor.setPixelFormat(PIXEL_FORMAT_RGB888);
	oniColorStream.setVideoMode(modeColor);

	//¡¾4¡¿
	// set depth and color imge registration mode
	if (device.isImageRegistrationModeSupported(IMAGE_REGISTRATION_DEPTH_TO_COLOR))
	{
		device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	}
	// start color stream
	result = oniColorStream.start();

	while (key != 27)
	{
		// read frame
		if (oniColorStream.readFrame(&oniColorImg) == STATUS_OK)
		{
			// convert data into OpenCV type
			cv::Mat cvRGBImg(oniColorImg.getHeight(), oniColorImg.getWidth(), CV_8UC3, (void*)oniColorImg.getData());
			cv::cvtColor(cvRGBImg, cvBGRImg, CV_RGB2BGR);
			cv::imshow("image", cvBGRImg);
		}

		if (oniDepthStream.readFrame(&oniDepthImg) == STATUS_OK)
		{
			cv::Mat cvRawImg16U(oniDepthImg.getHeight(), oniDepthImg.getWidth(), CV_16UC1, (void*)oniDepthImg.getData());
			cvRawImg16U.convertTo(cvDepthImg, CV_8U, 255.0 / (oniDepthStream.getMaxPixelValue()));
			//¡¾5¡¿
			// convert depth image GRAY to BGR
			cv::cvtColor(cvDepthImg, cvFusionImg, CV_GRAY2BGR);
			cv::imshow("depth", cvDepthImg);
		}
		//¡¾6¡¿
		cv::addWeighted(cvBGRImg, 0.5, cvFusionImg, 0.5, 0, cvFusionImg);
		cv::imshow("fusion", cvFusionImg);
		key = cv::waitKey(20);
	}

	//cv destroy
	cv::destroyWindow("depth");
	cv::destroyWindow("image");
	cv::destroyWindow("fusion");

	//OpenNI2 destroy
	oniDepthStream.destroy();
	oniColorStream.destroy();
	device.close();
	OpenNI::shutdown();

	return 0;
}