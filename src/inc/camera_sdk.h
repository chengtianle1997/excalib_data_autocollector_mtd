#pragma once

#include <iostream>
#include "stdio.h"
#include <process.h>
//#include "afxwin.h"
#include  "cv.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "thread"
#include "MvCameraControl.h"

using namespace cv;
using namespace std;

typedef struct {
	uint devNum; //device No.
	int in_w; 
	int in_h;
	float ExposureTime = 2000;
	int ExposureAuto = 0;
	float Gain = 15;
	int GainAuto = 0;
	float AcquisitionFrameRate = 60; //The value is in effect when Trigger Mode is off
	//int DeviceId = 0; 
	unsigned char SerialNum[INFO_MAX_BUFFER_SIZE];
	int DeviceUserId = 0;
	int ROIHeight = 0;
	int ROIWidth = 0;
	int ROIOffsetX = 0;
	int ROIOffsetY = 0;
	int FrameSpecInfoSelector = 0;
	int FrameSpecInfo = 0;
	int TriggerSelector = 6; //The only Select is FrameBurstStart
	int TriggerMode = 0; // 0 for Software Trigger and 1 for Hardware Triger
	int TriggerSource = 0;
	int TriggerActivation = 0;
	float TriggerDelay = 0;
	int TriggerCacheEnable = 0;
	int DevPort = 0;
	MV_CC_DEVICE_INFO DevInfo;

}CameraInitParam;


class Camera {
public:
	bool GetCamera = 0;
	//Get Device List
	int GetDevList(CameraInitParam &camerainitparam);

	//Open Camera 
	int OpenCamera(CameraInitParam &camerainitparam);

	//Start Grabbing
	int StartGrabbing();

	//Set ExposureTime 
	int SetExposureTime(float newExposureTime);

	//Set Exposureauto
	int SetExposureAuto(int newExposureAuto);

	//Set AcquisitionFrameRate
	int SetAcquisitionFrameRate(float newAcquisitionFrameRate);

	//Set Gain
	int SetGain(float newGain);

	//Set GainAuto[GainSelector]
	int SetGainAuto(int newGainAuto);

	//Set DeviceUserID
	int SetDeviceUserID(const char* newDeviceUserID);

	//Set RegionSelector 
	int SetRegionSelector(int newRegionSelector);

	//Set Width
	int SetWidth(int newWidth);

	//Set Height
	int SetHeight(int newHeight);

	//Set OffsetX
	int SetOffsetX(int newOffsetX);

	//Set OffsetY
	int SetOffsetY(int newOffsetY);

	//Set FrameSpecInfoSelector
	int SetFrameSpecInfoSelector(int newFrameSpecInfoSelector);

	//Set FrameSpecInfo
	int SetFrameSpecInfo(int newFrameSpecInfo);

	//Set TriggerSelector
	int SetTriggerSelector(int newTriggerSelector);

	//Set TriggerMode
	int SetTriggerMode(int newTriggerMode);

	//Set TriggerSource
	int SetTriggerSource(int newTriggerSource);

	//Set TriggerActivation
	int SetTriggerActivation(int newTriggerActivation);

	//Set TriggerDelay
	int SetTriggerDelay(float newTriggerDelay);

	//Set TriggerCacheEnable
	int SetTriggerCacheEnable(int newTriggerCacheEnable);

	//Get Image
	int GetImage(MV_FRAME_OUT &stOutFrame);

	//Stop Grabing Image
	int StopGrabbing();

	//Close Device
	int CloseDevice();

	//Get DeviceInfo
	bool GetDevInfo(int devNum, MV_CC_DEVICE_INFO &pstMVDevInfo);

private:
	int nRet = MV_OK;  //Return

	MV_CC_DEVICE_INFO_LIST stDevList;  //Device_List

	//MV_FRAME_OUT stOutFrame = { 0 }; //Frame_Out

	void* handle = NULL;  //Camera_Handle

	unsigned int g_nPayloadSize = 0;

	//Print Device Info
	bool  PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);

	


};