#include "camera_sdk.h"

 

//Get Device List
int Camera::GetDevList(CameraInitParam &camerainitparam)
{
	//获取设备枚举列表
	memset(&stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	nRet = MV_CC_EnumDevices(MV_USB_DEVICE, &stDevList);
	
	if (MV_OK != nRet)
	{
		printf("Enum Devices fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	
	if (stDevList.nDeviceNum > 0)
	{
		for (unsigned int i = 0; i < stDevList.nDeviceNum; i++)
		{
			//printf("[device %d]:\n", i);
			//设备信息
			MV_CC_DEVICE_INFO* pDeviceInfo = stDevList.pDeviceInfo[i];
			if (NULL == pDeviceInfo)
			{
				break;
				return -1;
			}

			//PrintDeviceInfo(pDeviceInfo);

			//int strl = 0;
			//strl = strlen((const char*)camerainitparam.SerialNum);

			//通过序列号查找相机
			if (!strcmp((const char *)camerainitparam.SerialNum, (const char*)stDevList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chSerialNumber)) {
				printf("%s Camera Launched\n", camerainitparam.SerialNum);
				camerainitparam.devNum = i;
				GetCamera = true;
			}

			
		}
		if (!GetCamera) {
			printf("Cannot Find the Camera you Choose!");
			//搜索相机异常 弹出程序
			return -2;
		}
	}
	else
	{
		printf("Find No Devices!\n");
		//搜索相机异常 弹出程序
		return -2;
	}
	
	return MV_OK;
}

//Open Camera 
int Camera::OpenCamera(CameraInitParam &camerainitparam)
{
	if (camerainitparam.devNum >= stDevList.nDeviceNum)
	{
		printf("Input error!\n");
		return -1;
	}
	//选择设备并创建句柄
	nRet = MV_CC_CreateHandle(&handle, stDevList.pDeviceInfo[camerainitparam.devNum]);
	if (MV_OK != nRet)
	{
		printf("Create Handle fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	nRet = MV_CC_OpenDevice(handle);
	if (MV_OK != nRet)
	{
		printf("Open Device fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	else {
		printf("Device is ready!\n");
	}

	//设置触发模式为off
	nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
	if (MV_OK != nRet)
	{
		printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
		return nRet;
	}

	//获取数据包大小
	MVCC_INTVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
	if (MV_OK != nRet)
	{
		printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	g_nPayloadSize = stParam.nCurValue;

	//Get the width and Height of the Camera
	MVCC_INTVALUE pIntValue;
	memset(&pIntValue, 0, sizeof(MVCC_INTVALUE));
	nRet = MV_CC_GetIntValue(handle, "Width", &pIntValue);
	if (MV_OK != nRet)
	{
		printf("Get Width fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	camerainitparam.in_w = pIntValue.nCurValue;
	//printf("%d", camerainitparam.in_w);
	nRet = MV_CC_GetIntValue(handle, "Height", &pIntValue);
	if (MV_OK != nRet)
	{
		printf("Get Height fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	camerainitparam.in_h = pIntValue.nCurValue;
	//printf("%d", camerainitparam.in_h);
	return nRet;
}

//Set ExposureTime 
int Camera::SetExposureTime(float newExposureTime) 
{
	nRet = MV_CC_SetFloatValue(handle, "ExposureTime", newExposureTime);
	if (MV_OK != nRet)
	{
		printf("Set ExposureTime fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Set Exposureauto
int Camera::SetExposureAuto(int newExposureAuto)
{
	nRet = MV_CC_SetEnumValue(handle, "ExposureAuto", newExposureAuto);
	if (MV_OK != nRet)
	{
		printf("Set ExposureAuto fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;

}

//Set AcquisitionFrameRate
int Camera::SetAcquisitionFrameRate(float newAcquisitionFrameRate) 
{
	nRet = MV_CC_SetFloatValue(handle, "AcquisitionFrameRate", newAcquisitionFrameRate);
	if (MV_OK != nRet)
	{
		printf("Set AcquisitionFrameRate fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Set Gain
int Camera::SetGain(float newGain) 
{
	nRet = MV_CC_SetFloatValue(handle, "Gain", newGain);
	if (MV_OK != nRet)
	{
		printf("Set Gain fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Set GainAuto[GainSelector]
int Camera::SetGainAuto(int newGainAuto)
{
	nRet = MV_CC_SetEnumValue(handle, "GainAuto", newGainAuto);
	if (MV_OK != nRet)
	{
		printf("Set GainAuto fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Set DeviceUserID
int Camera::SetDeviceUserID(const char* newDeviceUserID)
{
	nRet = MV_CC_SetDeviceUserID(handle, newDeviceUserID);
	if (MV_OK != nRet)
	{
		printf("Set DeviceUserID fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Set RegionSelector 
int Camera::SetRegionSelector(int newRegionSelector)
{
	nRet = MV_CC_SetEnumValue(handle, "RegionSelector ", newRegionSelector);
	if (MV_OK != nRet)
	{
		printf("Set RegionSelector fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Set Width
int Camera::SetWidth(int newWidth)
{
	nRet = MV_CC_SetWidth(handle, newWidth);
	if (MV_OK != nRet)
	{
		printf("Set ROIWidth fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Set Height
int Camera::SetHeight(int newHeight)
{
	nRet = MV_CC_SetHeight(handle, newHeight);
	if (MV_OK != nRet)
	{
		printf("Set ROIHeight fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Set OffsetX
int Camera::SetOffsetX(int newOffsetX)
{
	nRet = MV_CC_SetIntValue(handle, "OffsetX", newOffsetX);
	if (MV_OK != nRet)
	{
		printf("Set ROIOffsetX fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Set OffsetY
int Camera::SetOffsetY(int newOffsetY)
{
	nRet = MV_CC_SetIntValue(handle, "OffsetY", newOffsetY);
	if (MV_OK != nRet)
	{
		printf("Set ROIOffsetY fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Set FrameSpecInfoSelector
int Camera::SetFrameSpecInfoSelector(int newFrameSpecInfoSelector)
{
	nRet = MV_CC_SetEnumValue(handle, "FrameSpecInfoSelector", newFrameSpecInfoSelector);
	if (MV_OK != nRet)
	{
		printf("Set FrameSpecInfoSelector fail! nRet [0x%x]\n", nRet);
	}
	return nRet;
}

//Set FrameSpecInfo
int Camera::SetFrameSpecInfo(int newFrameSpecInfo)
{
	nRet = MV_CC_SetBoolValue(handle, "FrameSpecInfo", newFrameSpecInfo);
	if (MV_OK != nRet)
	{
		printf("SetFrameSpecInfo fail! nRet [0x%x]\n", nRet);
	}
	return nRet;
}

//Set TriggerSelector
int Camera::SetTriggerSelector(int newTriggerSelector)
{
	nRet = MV_CC_SetEnumValue(handle, "TriggerSelector", newTriggerSelector);
	if (MV_OK != nRet)
	{
		printf("Set TriggerSelector fail! nRet [0x%x]\n", nRet);
	}
	return nRet;
}

//Set TriggerMode
int Camera::SetTriggerMode(int newTriggerMode)
{
	nRet = MV_CC_SetEnumValue(handle, "TriggerMode", newTriggerMode);
	if (MV_OK != nRet)
	{
		printf("Set TriggerMode fail! nRet [0x%x]\n", nRet);
	}
	return nRet;
}

//Set TriggerSource
int Camera::SetTriggerSource(int newTriggerSource)
{
	nRet = MV_CC_SetEnumValue(handle, "TriggerSource", newTriggerSource);
	if (MV_OK != nRet)
	{
		printf("Set TriggerSource fail! nRet [0x%x]\n", nRet);
	}
	return nRet;
}

//Set TriggerActivation
int Camera::SetTriggerActivation(int newTriggerActivation)
{
	nRet = MV_CC_SetEnumValue(handle, "TriggerActivation", newTriggerActivation);
	if (MV_OK != nRet)
	{
		printf("Set TriggerActivation fail! nRet [0x%x]\n", nRet);
	}
	return nRet;
}

//Set TriggerDelay
int Camera::SetTriggerDelay(float newTriggerDelay) {
	nRet = MV_CC_SetFloatValue(handle, "TriggerDelay", newTriggerDelay);
	if (MV_OK != nRet)
	{
		printf("Set TriggerDelay fail! nRet [0x%x]\n", nRet);
	}
	return nRet;
}

//Set TriggerCacheEnable
int Camera::SetTriggerCacheEnable(int newTriggerCacheEnable)
{
	nRet = MV_CC_SetBoolValue(handle, "TriggerCacheEnable", newTriggerCacheEnable);
	if (MV_OK != nRet)
	{
		printf("Set TriggerCacheEnable fail! nRet [0x%x]\n", nRet);
	}
	return nRet;
}



//Start Grabbing
int Camera::StartGrabbing() 
{
	nRet = MV_CC_StartGrabbing(handle);
	if (MV_OK != nRet)
	{
		printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	//memset(&stOutFrame, 0, sizeof(MV_FRAME_OUT));
	return nRet;
}

//Get Image
int Camera::GetImage(MV_FRAME_OUT &stOutFrame) {
	//stOutFrame = { 0 };
	//memset(&stOutFrame, 0, sizeof(MV_FRAME_OUT));
	//stOutFrame = (MV_FRAME_OUT*)malloc(sizeof(MV_FRAME_OUT));
	nRet = MV_CC_GetImageBuffer(handle, &stOutFrame, 1000);
	if (nRet == MV_OK)
	{
		//printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
			//(stOutFrame).stFrameInfo.nWidth, (stOutFrame).stFrameInfo.nHeight, (stOutFrame).stFrameInfo.nFrameNum);
	}
	else
	{
		printf("No data[0x%x]\n", nRet);
	}
	if (NULL != (stOutFrame).pBufAddr)
	{
		nRet = MV_CC_FreeImageBuffer(handle, &stOutFrame);
		if (nRet != MV_OK)
		{
			printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
		}
		
	}
	return nRet;
}

//Stop Grabing Image
int Camera::StopGrabbing()
{
	nRet = MV_CC_StopGrabbing(handle);
	if (MV_OK != nRet)
	{
		printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	return nRet;
}

//Close Device
int Camera::CloseDevice()
{
	nRet = MV_CC_CloseDevice(handle);
	if (MV_OK != nRet)
	{
		printf("ClosDevice fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	nRet = MV_CC_DestroyHandle(handle);
	if (MV_OK != nRet)
	{
		printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
		return nRet;
	}
	if (nRet != MV_OK)
	{
		if (handle != NULL)
		{
			MV_CC_DestroyHandle(handle);
			handle = NULL;
		}
		return nRet;
	}
	return nRet;
}

bool  Camera::PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
	if (pstMVDevInfo == NULL)
	{
		printf("The Pointer of pstMVDevInfo is NULL!\n");
		return false;
	}
	if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
		printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);


	}
	else
	{
		printf("Not Support!\n");
	}
	return true;
}

//Get DeviceInfo
bool Camera::GetDevInfo(int devNum,  MV_CC_DEVICE_INFO &pstMVDevInfo)
{
	//获取设备枚举列表
	memset(&stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	nRet = MV_CC_EnumDevices(MV_USB_DEVICE, &stDevList);

	if (MV_OK != nRet)
	{
		printf("Enum Devices fail! nRet [0x%x]\n", nRet);
		return nRet;
	}

	if (stDevList.nDeviceNum > 0)
	{
		for (unsigned int i = 0; i < stDevList.nDeviceNum; i++)
		{
			if (i == devNum)
			{
				pstMVDevInfo = *stDevList.pDeviceInfo[i];
			}
		}
	}
	else
	{
		printf("Find No Devices!\n");
		return -1;
	}
	return MV_OK;
}