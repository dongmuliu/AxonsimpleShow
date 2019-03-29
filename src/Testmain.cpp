/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
#include <stdio.h>
#include <OpenNI.h>
#include<opencv2/opencv.hpp>

//#include "OniSampleUtilities.h"

#define SAMPLE_READ_WAIT_TIMEOUT 2000 //2000ms

using namespace openni;
using namespace cv;
Mat mImageDepth;


//将 CV_16UC1 深度图 转换成伪彩色图
Mat gray2rainbow(const Mat& scaledGray, int min, int max)
{
	Mat outputRainbow(scaledGray.size(), CV_8UC3);       //初始化了一个outputRainbow的彩色图像
	unsigned short grayValue;
	float tempvalue;

	float par = (float)255 / (max - min);


	for (int y = 0; y < scaledGray.rows; y++)
		for (int x = 0; x < scaledGray.cols; x++)
		{

			grayValue = scaledGray.at<ushort>(y, x);
			if ((grayValue > 0) && (grayValue < min))        //可能会出现找到的min并不是真正的最小值
			{
				tempvalue = (float)min;
			}
			else if (grayValue > max)                     //也可能会出现找到的max并不是真正的最大值
			{
				tempvalue = 0;
			}
			else
			{
				tempvalue = (float)(grayValue - min);
			}
			tempvalue = tempvalue*par;          //为了把深度值规划到(0~255之间)
			/*
			* color    R   G   B   gray
			* red      255 0   0   255
			* orange   255 127 0   204
			* yellow   255 255 0   153
			* green    0   255 0   102
			* cyan     0   255 255 51
			* blue     0   0   255 0
			*/

			Vec3b& pixel = outputRainbow.at<Vec3b>(y, x);
			tempvalue = 256 - tempvalue;

			if ((tempvalue <= 0) || (tempvalue >= 255))
			{
				pixel[0] = 0;
				pixel[1] = 0;
				pixel[2] = 0;
			}
			else if (tempvalue <= 51)
			{
				pixel[0] = 255;
				pixel[1] = (unsigned char)(tempvalue * 5);
				pixel[2] = 0;
			}
			else if (tempvalue <= 102)
			{
				tempvalue -= 51;
				pixel[0] = 255 - (unsigned char)(tempvalue * 5);
				pixel[1] = 255;
				pixel[2] = 0;
			}
			else if (tempvalue <= 153)
			{
				tempvalue -= 102;
				pixel[0] = 0;
				pixel[1] = 255;
				pixel[2] = (unsigned char)(tempvalue * 5);
			}
			else if (tempvalue <= 204)
			{
				tempvalue -= 153;
				pixel[0] = 0;
				pixel[1] = 255 - static_cast<unsigned char>(tempvalue * 128.0 / 51 + 0.5);
				pixel[2] = 255;
			}
			else if (tempvalue < 255)
			{
				tempvalue -= 204;
				pixel[0] = 0;
				pixel[1] = 127 - static_cast<unsigned char>(tempvalue * 127.0 / 51 + 0.5);
				pixel[2] = 255;
			}
		}

	return outputRainbow;
}
int main(int argc, char* argv[])
{
	Status rc = OpenNI::initialize();
	if (rc != STATUS_OK)
	{
		printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
		return 1;
	}

	Device device;

    if (argc < 2)
        rc = device.open(ANY_DEVICE);
    else
        rc = device.open(argv[1]);

    if (rc != STATUS_OK)
	{
		printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
		return 2;
	}
	OniVersion drver;
	int nsize;
	nsize = sizeof(drver);
	device.getProperty(ONI_DEVICE_PROPERTY_DRIVER_VERSION, &drver, &nsize);
	printf("AXon driver version V%d.%d.%d.%d\n", drver.major, drver.minor, drver.maintenance, drver.build);
	VideoStream depth;

	if (device.getSensorInfo(SENSOR_DEPTH) != NULL)
	{
		rc = depth.create(device, SENSOR_DEPTH);
		if (rc != STATUS_OK)
		{
			printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
			return 3;
		}
	}

	rc = depth.start();
	if (rc != STATUS_OK)
	{
		printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
		return 4;
	}

	VideoFrameRef frame;

	while (true)
	{
		int changedStreamDummy;
		VideoStream* pStream = &depth;
		rc = OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);
		if (rc != STATUS_OK)
		{
			printf("Wait failed! (timeout is %d ms)\n%s\n", SAMPLE_READ_WAIT_TIMEOUT, OpenNI::getExtendedError());
			continue;
		}

		rc = depth.readFrame(&frame);
		if (rc != STATUS_OK)
		{
			printf("Read failed!\n%s\n", OpenNI::getExtendedError());
			continue;
		}

		if (frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_1_MM && 
		    frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_100_UM 
			//&&frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_1_3_M
			)
		{
			printf("Unexpected frame format\n");
			continue;
		}

		DepthPixel* pDepth = (DepthPixel*)frame.getData();
		mImageDepth = cv::Mat(frame.getHeight(), frame.getWidth(), CV_16UC1, (void*)frame.getData());
		cv::Mat mRgbDepth = gray2rainbow(mImageDepth, 600, 3600);         //将mImageDepth变成看似的深度图(伪彩色图)
		
		cv::imshow("test", mRgbDepth);
		cv::waitKey(1);

		int middleIndex = (frame.getHeight()+1)*frame.getWidth()/2;

		printf("[%08llu] %8d\n", (long long)frame.getTimestamp(), pDepth[middleIndex]);
	}

	depth.stop();
	depth.destroy();
	device.close();
	OpenNI::shutdown();

	return 0;
}
