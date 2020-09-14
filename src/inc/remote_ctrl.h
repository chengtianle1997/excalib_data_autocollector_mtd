#pragma once

#include <process.h>    
#include "TChar.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <cctype>
//#include <Windows.h>
//#include<afxwin.h>
#include "serial_port.h"  


using namespace std; 



class MotorControl
{
public:
	void PosOrNeg_Rot(int i,bool dir,int speed);

	void Stop_Rot(int i );

	void Rot_Once(int i, bool direction, int speed, int dis);

	void RailPosition(int position);

	void PlatePosition(int position);

	void Init(int i);

	int DistanceCom = 1;
	int RotateCom = 1;
	int RailDistance = 0;
	int PlateDistance = 0;
	//void Close();
	void RelayOn();
	void RelayOff();
private:
	CSerialPort Distance1;
	int length = 8;//定义传输的长度
	
	
	//unsigned char *ddis = new unsigned char[13];//set distance
	//unsigned char *dspeed = new unsigned char[8];//set speed
	//unsigned char *dstop = new unsigned char[8];//stop
	//unsigned char *drun = new unsigned char[8];//run controled 
	//unsigned char *ddir = new unsigned char[8];//dir+ 
	//unsigned char *_ddir = new unsigned char[8];//dir-
	//unsigned char *dfree = new unsigned char[8];//run free +
	//unsigned char *_dfree = new unsigned char[8];//run free -

	//unsigned char *stop = new unsigned char[8];//stop

};
