#include "inc\cameractrl.h"


CCameraCtrl::CCameraCtrl()
{
}

CCameraCtrl::~CCameraCtrl()
{
}


int CCameraCtrl::CameraInit(Camera *camera, CameraInitParam& camerainitparam)
{
	int ret;

	ret = camera->GetDevList(camerainitparam);
	if (ret)
	{
		return ret;
	}


	//printf("Please choose a camera:");
	//scanf("%d", &camerainitparam.devNum);
	ret = camera->OpenCamera(camerainitparam);
	if (ret)
	{
		return ret;
	}

	if (!camerainitparam.ROIWidth)
	{
		camerainitparam.ROIWidth = camerainitparam.in_w;
	}
	if (!camerainitparam.ROIHeight)
	{
		camerainitparam.ROIHeight = camerainitparam.in_h;
	}

	//Get Device info
	camera->GetDevInfo(camerainitparam.devNum, camerainitparam.DevInfo);
	if (ret)
	{
		printf("Get DeviceInfo Failed");
	}

	//Set prop
	//Set Exposureauto
	ret = camera->SetExposureAuto(camerainitparam.ExposureAuto);
	if (ret)
	{
		return ret;
	}
	if (!camerainitparam.ExposureAuto)
	{
		//Set ExposureTime 
		ret = camera->SetExposureTime(camerainitparam.ExposureTime);
		if (ret)
		{
			return ret;
		}
	}


	//Set AcquisitionFrameRate
	ret = camera->SetAcquisitionFrameRate(camerainitparam.AcquisitionFrameRate);
	if (ret)
	{
		return ret;
	}


	//Set GainAuto[GainSelector]
	ret = camera->SetGainAuto(camerainitparam.GainAuto);
	if (ret)
	{
		return ret;
	}
	if (!camerainitparam.GainAuto)
	{
		//Set Gain
		ret = camera->SetGain(camerainitparam.Gain);
		if (ret)
		{
			return ret;
		}
	}


	//Start Grabing
	ret = camera->StartGrabbing();
	if (ret)
	{
		return ret;
	}

	ImageWidth = camerainitparam.in_w;
	ImageHeight = camerainitparam.in_h;
	SizeofPixels = camerainitparam.in_w * camerainitparam.in_h;

	return 0;
}



int CCameraCtrl::CameraClean(Camera * camera)
{
	int ret = 0;
	ret = camera->StopGrabbing();
	if (ret)
	{
		return ret;
	}
	ret = camera->CloseDevice();
	if (ret)
	{
		return ret;
	}

	return 1;
}


int CCameraCtrl::GrabImage(Camera * camera, Mat * matimage)
{
	MV_FRAME_OUT stOutFrame;
	memset(&stOutFrame, 0, sizeof(MV_FRAME_OUT));

	int ret = 0;
	ret = camera->GetImage(stOutFrame);
	if (ret == MV_OK)
	{
		Mat matImage{
			cvSize(ImageWidth,ImageHeight),
			CV_8UC1,
			(stOutFrame).pBufAddr
		};
		*matimage = matImage.clone();
	}
	else
	{
		printf("Grab Image Error!\n");
		return -1;
	}
	return 0;
}

int CCameraCtrl::GrabImage(Camera * camera, string img_file)
{
	bool bResult = false;
	MV_FRAME_OUT stOutFrame;
	memset(&stOutFrame, 0, sizeof(MV_FRAME_OUT));

	int ret = 0;
	ret = camera->GetImage(stOutFrame);
	if (ret == MV_OK)
	{
		Mat matImage{
			cvSize(ImageWidth,ImageHeight),
			CV_8UC1,
			(stOutFrame).pBufAddr
		};
		if (!matImage.empty())
		{
			bResult = imwrite(img_file, matImage);
			if (bResult)
				return 0;
			else
			{
				printf("Image file write failed...\n");
				return -2;
			}	
				
		}

	}
	else
	{
		printf("Grab Image Error!\n");
		return -1;
	}
	return 0;
}
