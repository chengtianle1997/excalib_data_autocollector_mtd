#define _CRT_SECURE_NO_WARNINGS


#define SAMPLE 18
#define CENTER_SAMPLE 5
#define LINE_SAMPLE 10

#include "serial_port.h"  
#include "crc.h"  
#include "remote_ctrl.h"  
#include "gauss_cal.h"
#include "cameractrl.h"
#include "cross_center.h"
#include "angle_sensor.h"
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

#define AXIS_ANG_ERR	0.3
#define AXIS_ANG_ERR2	2.0

#define IMAGE_HEIGHT 2048

#define VERBOSE_MODE

CCameraCtrl CamCtrl;
GaussCal gausscal;

bool b_use_multithread = true;


struct DATA {
	int y, ang;
	double x, dis;
}datagroup[7];

struct MEASURE_DATA {
	double x;
	double xc;
	double y;
	double agl;
	double dis;
};

const int Inf[20] = { -7600,-3200,0,3200,7600,-6400,-2400,800,4000,0,-4800,-1600,0,1600,4800,-4000,-800,2400,0,6400 };

int main()
{
	MotorControl DistanceCtrl;
	MotorControl RotateCtrl;
	MotorControl RelayCtrl;
	CAngleSensor Agl_Sensor;

	CrossCenterCalc cc_calc;

	bool bUseAngleSensor = false;
	bool bSaveImage = false;


	//CameraInitParam camerainitparam;
	//Camera camera;
	double LightCenter = 0;
	double Rail_Length_Scale = 0;
	int	CamCenterPosition = 0;

	int value = GetPrivateProfileIntA("IMAGE_PROCESS_PARAM",
		"Save_Image_Enable",
		0,
		".\\excalib_param.ini");
	if (value == 1)
		bSaveImage = true;


	int port_num = GetPrivateProfileIntA("MOTOR_PARAM", 
		"Rotate_Motor_PORT",				
		1,					
		".\\excalib_param.ini"); 

	RotateCtrl.Init(port_num);

	port_num = GetPrivateProfileIntA("MOTOR_PARAM", 
		"Distance_Motor_PORT",				
		1,					
		".\\excalib_param.ini"); 
	DistanceCtrl.Init(port_num);

	port_num = GetPrivateProfileIntA("MOTOR_PARAM",
		"Laser_Relay_PORT",				
		1,					
		".\\excalib_param.ini"); 
	RelayCtrl.Init(port_num);

	//Input Param for Test Camera
	CamCtrl.camerainitparam.ExposureTime = GetPrivateProfileIntA("CAMERA_PARAM",
		"CAM_Exposure_Time",
		5000,
		".\\excalib_param.ini");

	CamCtrl.camerainitparam.Gain = GetPrivateProfileIntA("CAMERA_PARAM",
		"CAM_Gain",
		15,
		".\\excalib_param.ini");

	int Rail_Pos = GetPrivateProfileIntA("CALIB_PARAM",
		"Rail_Pos",
		0,
		".\\excalib_param.ini");

	int Rail_PosStep = GetPrivateProfileIntA("CALIB_PARAM",
		"Rail_PosStep",
		0,
		".\\excalib_param.ini");

	int cam_start = GetPrivateProfileIntA("CALIB_PARAM",
		"CAM_Start",
		1,
		".\\excalib_param.ini");

	int cam_qtn = GetPrivateProfileIntA("CALIB_PARAM",
		"CAMs_to_Calib",
		8,
		".\\excalib_param.ini");
	//		int POS[10] = { 0,0,0,0,0,0,0,0,0 };

	int single_cam_calib_mode = GetPrivateProfileIntA("CALIB_PARAM",
		"SINGLE_CAM_Calib_Mode",
		0,
		".\\excalib_param.ini");


	int use_agl_sensor = GetPrivateProfileIntA("ANGLE_SENSOR_PARAM",
		"Angle_Sensor_Enable",
		0,
		".\\excalib_param.ini");

	if (use_agl_sensor == 0)
		bUseAngleSensor = false;
	else
		bUseAngleSensor = true;

	if (bUseAngleSensor)
	{
		port_num = GetPrivateProfileIntA("ANGLE_SENSOR_PARAM",
			"Angle_Sensor_PORT",
			18,
			".\\excalib_param.ini");

		int baudrate = GetPrivateProfileIntA("ANGLE_SENSOR_PARAM",
			"Angle_Sensor_BAUDRATE",
			38400,
			".\\excalib_param.ini");
		Agl_Sensor.Init(port_num, baudrate);
		
	}


	double Initial_Distance = 0;
	char szInitDistanceString[64];
	GetPrivateProfileStringA("CALIB_PARAM",
		"Init_Distance",
		"0",
		szInitDistanceString,
		sizeof(szInitDistanceString),
		".\\excalib_param.ini");
	Initial_Distance = strtod(szInitDistanceString, NULL);

	SYSTEMTIME systime;
	GetLocalTime(&systime);

	for (int cam_idx = cam_start; cam_idx < cam_start+ cam_qtn; cam_idx++)
	{
		//Input Initial Distance for current camera
		if (cam_idx < 8)
		{
			if (1 == GetPrivateProfileIntA("CALIB_PARAM",
				"Init_Distance_Need_Confirm",
				0,
				".\\excalib_param.ini"))
			{
				cout << "Initial distance =" << Initial_Distance << endl;
				cout << "Press ENTER to confirm or input new value:";
				char szNewInitDistValue[64] = {0};
				cin.getline(szNewInitDistValue, 64, '\n');
				double S_input = strtod(szNewInitDistValue, NULL);
				if (S_input != 0)
					Initial_Distance = S_input;

				//system("pause");
			}
			else
				Sleep(3000);
			//RotateCtrl.PlatePosition(1600);
		}
		
		//Input SerialNum for Test Camera
		char szKey[64];
		sprintf(szKey, "CAM%d_Serial", cam_idx);
		char szCamSerial[64];
		int serial_len = GetPrivateProfileStringA("CALIB_PARAM",
			szKey,
			"",
			szCamSerial,
			sizeof(szCamSerial),
			".\\excalib_param.ini");

		//const char *CST = "00D36305974";
		memcpy(CamCtrl.camerainitparam.SerialNum, szCamSerial, sizeof(CamCtrl.camerainitparam.SerialNum));


		int ret = 0;
		//CameraInit
		ret = CamCtrl.CameraInit(&CamCtrl.camera, CamCtrl.camerainitparam);
		if (ret)
		{
			printf("Camera%d Init failed\n",cam_idx);
			return -1;
		}


		Mat matimg;
		GaussCal Gausscal;
		GaussCalParam Mat1;
		int yint = 0;



		GetPrivateProfileStringA("CALIB_PARAM",
			"Rail_Length_Scale",
			"0",
			szInitDistanceString,
			sizeof(szInitDistanceString),
			".\\excalib_param.ini");
		Rail_Length_Scale = strtod(szInitDistanceString, NULL);

		int rail_move_speed = GetPrivateProfileIntA("MOTOR_PARAM",
			"Rail_Move_Speed",
			10,
			".\\excalib_param.ini");

		int rotate_interval = GetPrivateProfileIntA("MOTOR_PARAM",
			"Rotate_Interval",
			500,
			".\\excalib_param.ini");



		ostringstream ostr;
		ostr << ".\\CAM" << cam_idx << "_pics\\";

		string path = ostr.str();
		string path_prefix = path;

		ostringstream fstr;
		fstr << ".\\CAM" << cam_idx << "_calibdata_" << \
			setw(4) << systime.wYear << setfill('0') << setw(2) << systime.wMonth << \
			setfill('0') << setw(2) << systime.wDay << setfill('0') << setw(2) << systime.wHour << \
			setfill('0') << setw(2) << systime.wMinute << setfill('0') << setw(2) << systime.wSecond << ".csv";
		string result_file_name = fstr.str();

		string path2;
		
		//rotate to correspoding cam center angle
		if (single_cam_calib_mode == 0)
		{
			//get lightcenter of current camera
			char szCamLightCenter[64];
			//char szKey[64];
			sprintf(szKey, "CAM%d_LightCenter", cam_idx);
			GetPrivateProfileStringA("CALIB_PARAM",
				szKey,
				"0",
				szCamLightCenter,
				sizeof(szCamLightCenter),
				".\\excalib_param.ini");
			LightCenter = strtod(szCamLightCenter, NULL);

			/*char szCamCenterPosition[64];
			sprintf(szKey, "CAM%d_POS", cam_idx);
			GetPrivateProfileStringA("CALIB_PARAM",
				szKey,
				"0",
				szCamCenterPosition,
				sizeof(szCamCenterPosition),
				".\\excalib_param.ini");
			CamCenterPosition = atoi(szCamCenterPosition);*/

			char szCamCenterPosition[64];
			sprintf(szKey, "CAM%d_ANGLE", cam_idx);
			GetPrivateProfileStringA("CALIB_PARAM",
				szKey,
				"0",
				szCamCenterPosition,
				sizeof(szCamCenterPosition),
				".\\excalib_param.ini");
			CamCenterPosition = int(atof(szCamCenterPosition)*400);



			RotateCtrl.PlatePosition(CamCenterPosition);
		}
		else
		{
			//get lightcenter of current camera
			char szCamLightCenter[64];
			//char szKey[64];
			sprintf(szKey, "CAM%d_LightCenter", cam_idx);
			GetPrivateProfileStringA("CALIB_PARAM",
				szKey,
				"0",
				szCamLightCenter,
				sizeof(szCamLightCenter),
				".\\excalib_param.ini");
			LightCenter = strtod(szCamLightCenter, NULL);

			CamCenterPosition = 0;
		}
		

		//open vertical laser
		RelayCtrl.RelayOn();

		//print result file title
		FILE *fp = NULL;
		fp = fopen(result_file_name.c_str(), "a");
		if (fp)
		{
			fprintf(fp, "X , Y , DISTANCE, ANGLE\n");
			fclose(fp);
		}

		
		//find cam_center position
		int PlatePos = CamCenterPosition;
		double current_ref_agl;
		Coordinate cross_pt;

		bool result;
		if (b_use_multithread)
			result = cc_calc.GetCrossPointMultiThread(CamCtrl.camera, &cross_pt);
		else
			result = cc_calc.GetCrossPoint(CamCtrl.camera, &cross_pt);

#ifdef VERBOSE_MODE
		if (!result)
		{
			cout << "center cross not found! Adjust camera position.\n" << endl;
			system("pause");
			//return 0;
		}
		printf("cam%d axis=%.3f\n", cam_idx, LightCenter);
		printf("cc=%f\r", cross_pt.y);
#endif
		while (abs(cross_pt.y - LightCenter) > AXIS_ANG_ERR)
		{
			PlatePos = PlatePos + 5 * (cross_pt.y - LightCenter);
			RotateCtrl.PlatePosition(PlatePos);
			int sleep_time = abs(cross_pt.y - LightCenter) * 8;
			if (sleep_time < 500)
				sleep_time = 500;
			Sleep(sleep_time);
			//Sleep(1000);
			bool ret;
			if (b_use_multithread)
				ret = cc_calc.GetCrossPointMultiThread(CamCtrl.camera, &cross_pt);
			else
				ret = cc_calc.GetCrossPoint(CamCtrl.camera, &cross_pt);
			printf("cc=%.3f\r", cross_pt.y);
		}

		if (bUseAngleSensor)
		{
			//get current angle_value
			Agl_Sensor.SetRelZero();
			current_ref_agl = Agl_Sensor.GetAbsAngleValueF();
			printf("CAM%d center position = %d\n", cam_idx, CamCenterPosition);
		}
		else
		{
			printf("CAM%d center position = %d\n", cam_idx, CamCenterPosition);
			printf("CAM%d begin position = %d\n", cam_idx, PlatePos);
		}

		printf("CAM%d start calibration for %d Distance Position and %d Angle Samples\n\n", cam_idx, Rail_Pos, SAMPLE);

		for (int i = 0; i < Rail_Pos; i++)
		{
			DistanceCtrl.RailPosition((0-Rail_PosStep)*i);
			if(i!=0)
				Sleep(Rail_PosStep * rail_move_speed);

			printf("cam%d axis=%.3f\n", cam_idx, LightCenter);

			if (bUseAngleSensor)		//using angle sensor
			{
				MEASURE_DATA mdata[SAMPLE + 2];

				bool ret;
				if (b_use_multithread)
					ret = cc_calc.GetCrossPointMultiThread(CamCtrl.camera, &cross_pt);
				else
					ret = cc_calc.GetCrossPoint(CamCtrl.camera, &cross_pt);

				printf("cc=%.3f\r", cross_pt.y);
				while (!ret)
				{
					RotateCtrl.PlatePosition(CamCenterPosition);
					Sleep(abs(PlatePos - CamCenterPosition));
					PlatePos = CamCenterPosition;
					if (b_use_multithread)
						ret = cc_calc.GetCrossPointMultiThread(CamCtrl.camera, &cross_pt);
					else
						ret = cc_calc.GetCrossPoint(CamCtrl.camera, &cross_pt);
					printf("cc=%.3f\r", cross_pt.y);

				}
				while (abs(cross_pt.y - LightCenter) > AXIS_ANG_ERR)
				{
					PlatePos = PlatePos + 5 * (cross_pt.y - LightCenter);
					RotateCtrl.PlatePosition(PlatePos);
					int sleep_time = abs(cross_pt.y - LightCenter) * 8;
					if (sleep_time < 500)
						sleep_time = 500;
					Sleep(sleep_time);

					bool ret;
					if (b_use_multithread)
						ret = cc_calc.GetCrossPointMultiThread(CamCtrl.camera, &cross_pt);
					else
						ret = cc_calc.GetCrossPoint(CamCtrl.camera, &cross_pt);

					printf("cc=%.3f\r", cross_pt.y);
					if (!ret)
					{
						RotateCtrl.PlatePosition(CamCenterPosition);
						Sleep(abs(PlatePos - CamCenterPosition));
						PlatePos = CamCenterPosition;
					}
				}
				// get current angle_value
				Agl_Sensor.SetRelZero();
				// save mdata on lightcenter
				mdata[0].xc = cross_pt.x;
				mdata[0].y = cross_pt.y;
				//mdata[0].agl = target_angle;
				mdata[0].agl = 0.0;
				mdata[0].dis = Rail_PosStep * i * Rail_Length_Scale + Initial_Distance;
				//switch on/off vertical laser for indicator
				RelayCtrl.RelayOff();
				Sleep(100);

				// Calculate the laser center for CENTER_SAMPLE frames
				double center_x = 0.0;
				for (int i = 0; i < CENTER_SAMPLE; i++)
				{
					Mat center_img;
					CamCtrl.GrabImage(&CamCtrl.camera, &center_img);
					GaussCalParam gauss_param;
					gauss_param.matImage = center_img;
					gauss_param.point = new MPoint[IMAGE_HEIGHT];
					gausscal.GaussCenter(gauss_param);
					int line_counter = 0;
					double line_center = 0.0;
					for (int j = int(round(cross_pt.y)) - int(LINE_SAMPLE / 2); j < int(round(cross_pt.y)) + int(LINE_SAMPLE / 2); j++)
					{
						line_center += gauss_param.point[j].cx;
						line_counter++;
					}
					center_x += line_center / line_counter;
				}
				
				mdata[0].x = center_x / CENTER_SAMPLE;
				RelayCtrl.RelayOn();

				current_ref_agl = Agl_Sensor.GetRelAngleValueF();
				printf("CAM%d set center reference position.\n", cam_idx);
				printf("%.3f , %.3f , %.1f, %.2f, %.3f\n", mdata[0].x, mdata[0].y, mdata[0].dis, mdata[0].agl, mdata[0].xc);

				double target_angle;
				double current_angle = current_ref_agl;
				
				for (int m = 1; m < SAMPLE + 2; m++)
				{
					target_angle = (m - int(SAMPLE/2) - 1)*2.0;
					printf("current angle = %.2f deg (%d/%d, All:%d/%d)\r", current_angle, m, SAMPLE + 1, m + i * (SAMPLE + 1), Rail_Pos * (SAMPLE + 1));
					PlatePos = PlatePos + 400 * (target_angle - current_angle);
					RotateCtrl.PlatePosition(PlatePos);
					int sleep_time = abs(target_angle - current_angle) * 200;
					if (sleep_time < 500)
						sleep_time = 500;
					Sleep(sleep_time);		//modify here to set delay time for waiting the rotate in position
					//adjust in small steps
					current_angle = Agl_Sensor.GetRelAngleValueF();
					while (abs(current_angle - target_angle) > 0.1)	//this threshold should be determined through experiment
					{
						PlatePos = PlatePos + 10 * (target_angle - current_angle);
						RotateCtrl.PlatePosition(PlatePos);
						int sleep_time = abs(target_angle - current_angle) * 200;
						if (sleep_time < 500)
							sleep_time = 500;
						Sleep(sleep_time);
						current_angle = Agl_Sensor.GetRelAngleValueF();
					}
					//acquisition image and calc the cross point coordinates
					bool ret;
					if (b_use_multithread)
						ret = cc_calc.GetCrossPointMultiThread(CamCtrl.camera, &cross_pt);
					else
						ret = cc_calc.GetCrossPoint(CamCtrl.camera, &cross_pt);

					if (!ret)
					{
						mdata[m].x = 0;
						mdata[m].xc = 0;
						mdata[m].y = 0;
						//mdata[m].agl = target_angle;
						mdata[m].agl = current_angle;
						mdata[m].dis = Rail_PosStep * i * Rail_Length_Scale + Initial_Distance;
						printf("%.3f , %.3f , %.1f, %.2f, %.3f\n", mdata[m].x, mdata[m].y, mdata[m].dis, mdata[m].agl, mdata[m].xc);
						continue;
					}
					mdata[m].xc = cross_pt.x;
					mdata[m].y = cross_pt.y;
					//mdata[m].agl = target_angle;
					mdata[m].agl = current_angle;
					mdata[m].dis = Rail_PosStep * i * Rail_Length_Scale + Initial_Distance;

					if (bSaveImage)
					{
						path2 = to_string(i + 1) + "-" + to_string(target_angle) + ".bmp";
						path = path_prefix + path2;

						Mat result_img;
						CamCtrl.GrabImage(&CamCtrl.camera, &result_img);

						cv::Point cc(cross_pt.x, cross_pt.y);
						cv::Scalar color(0, 0, 255); // (B, G, R)
						cv::circle(result_img, cc, 6, color, -1);
						imwrite(path, result_img);

						//delete(Mat1.point);
					}


					//switch on/off vertical laser for indicator
					RelayCtrl.RelayOff();
					Sleep(100);

					// Calculate the laser center for CENTER_SAMPLE frames
					double center_x = 0.0;
					for (int i = 0; i < CENTER_SAMPLE; i++)
					{
						Mat center_img;
						CamCtrl.GrabImage(&CamCtrl.camera, &center_img);
						GaussCalParam gauss_param;
						gauss_param.matImage = center_img;
						gauss_param.point = new MPoint[IMAGE_HEIGHT];
						gausscal.GaussCenter(gauss_param);
						int line_counter = 0;
						double line_center = 0.0;
						for (int j = int(round(cross_pt.y)) - int(LINE_SAMPLE / 2); j < int(round(cross_pt.y)) + int(LINE_SAMPLE / 2); j++)
						{
							line_center += gauss_param.point[j].cx;
							line_counter++;
						}
						center_x += line_center / line_counter;
					}

					mdata[m].x = center_x / CENTER_SAMPLE;
					RelayCtrl.RelayOn();
					printf("%.3f , %.3f , %.1f, %.2f, %.3f\n", mdata[m].x, mdata[m].y, mdata[m].dis, mdata[m].agl, mdata[m].xc);
					Sleep(100);

				}
				/*for (int m = 0; m < SAMPLE + 2; m++)
				{
					printf("%.3f , %.3f , %.1f, %.2f, %.3f\n", mdata[m].x, mdata[m].y, mdata[m].dis, mdata[m].agl, mdata[m].xc);
				}*/

				fp = fopen(result_file_name.c_str(), "a");
				if (fp)
				{
					for (int m = 0; m < SAMPLE + 2; m++)
					{
						fprintf(fp, "%.3f , %.3f , %.1f, %.2f, %.3f\n", mdata[m].x, mdata[m].y, mdata[m].dis, mdata[m].agl, mdata[m].xc);
					}
					fclose(fp);
				}
				RotateCtrl.PlatePosition(CamCenterPosition);
				PlatePos = CamCenterPosition;

			}
			else   //not using angle sensor
			{
				printf("CAM%d current position = %d\n", cam_idx, PlatePos);
				for (int ij = 0; ij < 4; ij++)
				{
				AGAIN:
					cc_calc.GetCrossPoint(CamCtrl.camera, &cross_pt);
					while (abs(cross_pt.y - LightCenter) > AXIS_ANG_ERR)
					{
						PlatePos = PlatePos + 5 * (cross_pt.y - LightCenter);
						RotateCtrl.PlatePosition(PlatePos);

						cc_calc.GetCrossPoint(CamCtrl.camera, &cross_pt);
					}
						
					for (int j = 0; j < 5; j++)
					{
						RotateCtrl.PlatePosition(PlatePos + Inf[ij * 5 + j]);	///
						Sleep(300);
						cc_calc.GetCrossPoint(CamCtrl.camera, &cross_pt);
						if (Inf[ij * 5 + j] == 0 && (LightCenter - cross_pt.y > AXIS_ANG_ERR2 || LightCenter - cross_pt.y < (0 - AXIS_ANG_ERR2)))
						{
							printf("rotate back to center with error...reset calibration procedure!\n");
							goto AGAIN;
						}

						yint = (int)cross_pt.y;

						path2 = to_string(i + 1) + "-" + to_string(ij * 5 + j + 1) + ".bmp";
						path = path_prefix + path2;


						Mat result_img;
						CamCtrl.GrabImage(&CamCtrl.camera, &result_img);
						////CLOSEIT();
						RelayCtrl.RelayOff();
						Sleep(rotate_interval);
						CamCtrl.GrabImage(&CamCtrl.camera, &matimg);
						//imshow("CameraTest", matimg);


						Mat1.matImage = matimg;
						Mat1.point = new MPoint[2048];
						Gausscal.GaussCenter(Mat1);
						cross_pt.x = Mat1.point[yint].cx;
						datagroup[j].x = cross_pt.x;
						datagroup[j].y = yint;

						datagroup[j].dis = Rail_PosStep * i * Rail_Length_Scale + Initial_Distance;
						datagroup[j].ang = Inf[ij * 5 + j] / 400;

						cv::Point cc(datagroup[j].x, datagroup[j].y);
						cv::Scalar color(0, 0, 255); // (B, G, R)
						cv::circle(result_img, cc, 6, color, -1);
						imwrite(path, result_img);

						delete(Mat1.point);


						//switch on/off vertical laser for indicator
						RelayCtrl.RelayOff();
						Sleep(rotate_interval);
						RelayCtrl.RelayOn();
						Sleep(100);

					}
					for (int j = 0; j < 5; j++)
					{
						printf("%.3f , %d , %.1f, %d\n", datagroup[j].x, datagroup[j].y, datagroup[j].dis, datagroup[j].ang);
					}

					//result_file_name
					//FILE *fp = NULL;
					fp = fopen(result_file_name.c_str(), "a");
					if (fp)
					{
						for (int j = 0; j < 5; j++)
						{
							fprintf(fp, "%.3f , %d , %.1f, %d\n", datagroup[j].x, datagroup[j].y, datagroup[j].dis, datagroup[j].ang);
						}
						fclose(fp);
					}
					RotateCtrl.PlatePosition(PlatePos);
				}

			}

			
		}
		DistanceCtrl.RailPosition(0);
		
		CamCtrl.CameraClean(&CamCtrl.camera);
		//system("Beep");
		cout << endl;
		cout << "CAM" << cam_idx << " Calibration Finished!" << endl;
		

	}
	system("pause");
	return 0;
}