#pragma once

#include "camera_sdk.h"
//#include "Windows.h"


using namespace cv;
using namespace std;

class CCameraCtrl {
public:
	CCameraCtrl();
	~CCameraCtrl();

public:
	int CameraInit(Camera *camera, CameraInitParam& camerainitparam);
	int CameraClean(Camera *camera);
	int GrabImage(Camera *camera, Mat* matimage);
	int GrabImage(Camera *camera, string img_file);


public:
	CameraInitParam camerainitparam;
	Camera camera;

private:
	int SizeofPixels = 0;
	int ImageWidth = 0;
	int ImageHeight = 0;

};