
#include "serial_port.h"  
#include "remote_ctrl.h"  
#include "crc.h"  
#include <iostream>

using namespace std;
//
//CSerialPort Distance1;//首先将之前定义的类实例化
//int length = 8;//定义传输的长度
//unsigned char *dDis = new unsigned char[13];//set distance
//unsigned char *dSpeed = new unsigned char[8];//set speed
//unsigned char *dStop = new unsigned char[8];//stop
//unsigned char *dRun = new unsigned char[8];//run controled 
//unsigned char *dDir = new unsigned char[8];//dir+ 
//unsigned char *_dDir = new unsigned char[8];//dir-
//unsigned char *dFree = new unsigned char[8];//run free +
//unsigned char *_dFree = new unsigned char[8];//run free -

uint8_t ddis[] = { 0x01, 0x10, 0x00, 0x08, 0x00, 0x04, 0x08, 0x03, 0xe8, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x01, 0x16, 0x7c }; // set distance
uint8_t dspeed[] = { 0x01, 0x06, 0x00, 0x08, 0x00, 0x02, 0x89, 0xc9 }; //set speed
uint8_t dstop[] = { 0x01, 0x05, 0x00, 0x03, 0xff, 0x00, 0x7c, 0x3a };//stop
uint8_t drun[] = { 0x01, 0x05, 0x00, 0x07, 0xff, 0x00, 0x3d, 0xfb };//run controled 
uint8_t ddir[] = { 0x01, 0x06, 0x00, 0x11, 0x00, 0x00, 0xd9, 0xcf };//dir+ 
uint8_t _ddir[] = { 0x01, 0x06, 0x00, 0x11, 0x00, 0x01, 0x18, 0x0f };//dir-
uint8_t dfree[] = { 0x01, 0x05, 0x00, 0x04, 0xFF, 0x00, 0xCD, 0xFB };//run free +
uint8_t _dfree[] = { 0x01, 0x05, 0x00, 0x05, 0xff, 0x00, 0x9c, 0x3b };//run free -
uint8_t On[] = { 0x01 };
uint8_t Off[] = { 0x00 };

void MotorControl::Init(int i)
{

	if (!Distance1.InitPort(i, CBR_9600, 'N', 8, 1, EV_RXCHAR))//是否打开串口，3就是你外设连接电脑的com口，可以在设备管理器查看，然后更改这个参数
	{
		cout << "COM" << i << " init failed!" << endl;
	}
	else
	{
		cout << "COM" << i << " init success!" << endl;
	}

	ddis[0] = i;
	dspeed[0] = i;
	dstop[0] = i;
	drun[0] = i;
	ddir[0] = i;
	_ddir[0] = i;
	dfree[0] = i;
	_dfree[0] = i;
	////write distance
	//ddis[0] = 0x01;
	//ddis[1] = 0x10;
	//ddis[2] = 0x00;
	//ddis[3] = 0x09;
	//ddis[4] = 0x00;
	//ddis[5] = 0x02;
	//ddis[6] = 0x04;
	//ddis[7] = 0x00;
	//ddis[8] = 0xc8;//low distance=200
	//ddis[9] = 0x00;
	//ddis[10] = 0x00;//high
	//ddis[11] = 0xb2;
	//ddis[12] = 0x3b;//crc

	////write speed
	//dspeed[0] = 0x01;
	//dspeed[1] = 0x06;
	//dspeed[2] = 0x00;
	//dspeed[3] = 0x08;
	//dspeed[4] = 0x00;
	//dspeed[5] = 0x02;//speed=2
	//dspeed[6] = 0x89;
	//dspeed[7] = 0xc9;//crc

	////stop
	//dstop[0] = 0x01;
	//dstop[1] = 0x05;
	//dstop[2] = 0x00;
	//dstop[3] = 0x03;
	//dstop[4] = 0xff;
	//dstop[5] = 0x00;
	//dstop[6] = 0x7c;
	//dstop[7] = 0x3a;//crc

	////run
	//drun[0] = 0x01;
	//drun[1] = 0x05;
	//drun[2] = 0x00;
	//drun[3] = 0x07;
	//drun[4] = 0xff;
	//drun[5] = 0x00;
	//drun[6] = 0x3d;
	//drun[7] = 0xfb;//crc

	////dir+
	//ddir[0] = 0x01;
	//ddir[1] = 0x06;
	//ddir[2] = 0x00;
	//ddir[3] = 0x11;
	//ddir[4] = 0x00;
	//ddir[5] = 0x00;
	//ddir[6] = 0xd9;
	//ddir[7] = 0xcf;//crc

	////dir-
	//_ddir[0] = 0x01;
	//_ddir[1] = 0x06;
	//_ddir[2] = 0x00;
	//_ddir[3] = 0x11;
	//_ddir[4] = 0x00;
	//_ddir[5] = 0x01;
	//_ddir[6] = 0x18;
	//_ddir[7] = 0x0f;//crc

	////free+
	//dfree[0] = 0x01;
	//dfree[1] = 0x05;
	//dfree[2] = 0x00;
	//dfree[3] = 0x04;
	//dfree[4] = 0xff;
	//dfree[5] = 0x00;
	//dfree[6] = 0xcd;
	//dfree[7] = 0xfb;//crc

	////free-
	//_dfree[0] = 0x01;
	//_dfree[1] = 0x05;
	//_dfree[2] = 0x00;
	//_dfree[3] = 0x05;
	//_dfree[4] = 0xff;
	//_dfree[5] = 0x00;
	//_dfree[6] = 0x9c;
	//_dfree[7] = 0x3b;//crc

	////stop
	//stop[0] = 01;
	//stop[1] = 05;
	//stop[2] = 00;
	//stop[3] = 03;
	//stop[4] = 255;
	//stop[5] = 00;
	//stop[6] = 124;
	//stop[7] = 58;//crc

}


void MotorControl::Rot_Once(int i, bool dir, int speed, int dis)
{
	ddis[0] = i;
	drun[0] = i;
	drun[6] = crc16table(drun, sizeof(drun) - 2) % 256;
	drun[7] = crc16table(drun, sizeof(drun) - 2) / 256;
	ddis[8] = speed % 256;
	ddis[7] = speed / 256;

	ddis[10] = dis % 256;
	ddis[9] = (dis / 256) % 256;
	ddis[12] = ((dis / 256) / 256) % 256;
	ddis[11] = ((dis / 256) / 256) / 256;

	if (dir)
	{
		ddis[14] = 0;
	}
	else
	{
		ddis[14] = 1;
	}

	ddis[15] = crc16table(ddis, sizeof(ddis) - 2) % 256;
	ddis[16] = crc16table(ddis, sizeof(ddis) - 2) / 256;
/*
	printf(" value : 0x%x\n", ddis[0]);
	printf(" value : 0x%x\n", ddis[1]);
	printf(" value : 0x%x\n", ddis[2]);
	printf(" value : 0x%x\n", ddis[3]);
	printf(" value : 0x%x\n", ddis[4]);
	printf(" value : 0x%x\n", ddis[5]);
	printf(" value : 0x%x\n", ddis[6]);
	printf(" value : 0x%x\n", ddis[7]);
	printf(" value : 0x%x\n", ddis[8]);
	printf(" value : 0x%x\n", ddis[9]);
	printf(" value : 0x%x\n", ddis[10]);
	printf(" value : 0x%x\n", ddis[11]);
	printf(" value : 0x%x\n", ddis[12]);
	printf(" value : 0x%x\n", ddis[13]);
	printf(" value : 0x%x\n", ddis[14]);
	printf(" value : 0x%x\n", ddis[15]);
	printf(" value : 0x%x\n", ddis[16]);
*/
	cout << Distance1.WriteData(ddis, 17) << endl;
	cout << Distance1.GetBytesInCOM() << endl;

	cout << Distance1.WriteData(drun, 8) << endl;
	cout << Distance1.GetBytesInCOM() << endl;
}


void MotorControl::PosOrNeg_Rot(int i, bool dir, int speed)
{
	dspeed[0] = i;
	dfree[0] = i;
	_dfree[0] = i;

	dspeed[5] = speed % 256;
	dspeed[4] = speed / 256;

	dspeed[6] = crc16table(dspeed, sizeof(dspeed) - 2) % 256;
	dspeed[7] = crc16table(dspeed, sizeof(dspeed) - 2) / 256;
	dfree[6] = crc16table(dfree, sizeof(dfree) - 2) % 256;
	dfree[7] = crc16table(dfree, sizeof(dfree) - 2) / 256;
	_dfree[6] = crc16table(_dfree, sizeof(_dfree) - 2) % 256;
	_dfree[7] = crc16table(_dfree, sizeof(_dfree) - 2) / 256;
	/*printf(" value : 0x%x\n", dspeed[0]);
	printf(" value : 0x%x\n", dspeed[1]);
	printf(" value : 0x%x\n", dspeed[2]);
	printf(" value : 0x%x\n", dspeed[3]);
	printf(" value : 0x%x\n", dspeed[4]);
	printf(" value : 0x%x\n", dspeed[5]);
	printf(" value : 0x%x\n", dspeed[6]);
	printf(" value : 0x%x\n", dspeed[7]);

	printf(" value : 0x%x\n", dfree[0]);
	printf(" value : 0x%x\n", dfree[1]);
	printf(" value : 0x%x\n", dfree[2]);
	printf(" value : 0x%x\n", dfree[3]);
	printf(" value : 0x%x\n", dfree[4]);
	printf(" value : 0x%x\n", dfree[5]);
	printf(" value : 0x%x\n", dfree[6]);
	printf(" value : 0x%x\n", dfree[7]);*/

	cout << Distance1.WriteData(dspeed, 8) << endl;
	cout << Distance1.GetBytesInCOM() << endl;


	if (dir)
	{
		cout << Distance1.WriteData(dfree, 8) << endl;
		cout << Distance1.GetBytesInCOM() << endl;
	}
	else
	{
		cout << Distance1.WriteData(_dfree, 8) << endl;
		cout << Distance1.GetBytesInCOM() << endl;
	}
}

void MotorControl::Stop_Rot(int i)
{
	dstop[0] = i;
	dstop[6] = crc16table(dstop, sizeof(dstop) - 2) % 256;
	dstop[7] = crc16table(dstop, sizeof(dstop) - 2) / 256;
	cout << Distance1.WriteData(dstop, 8) << endl;
	cout << Distance1.GetBytesInCOM() << endl;
}


void MotorControl::RailPosition(int position)
{
	int k;
	int speed = 100;
	k = position;
	position = position - RailDistance;
	RailDistance = k;
	if (position < 0)
	{
		ddis[14] = 0;
		position = -position;
	}
	else
	{
		ddis[14] = 1;
	}

	ddis[0] = DistanceCom;
	drun[0] = DistanceCom;

	ddis[8] = speed % 256;
	ddis[7] = speed / 256;

	ddis[10] = position % 256;
	ddis[9] = (position / 256) % 256;
	ddis[12] = ((position / 256) / 256) % 256;
	ddis[11] = ((position / 256) / 256) / 256;

	ddis[15] = crc16table(ddis, sizeof(ddis) - 2) % 256;
	ddis[16] = crc16table(ddis, sizeof(ddis) - 2) / 256;
	drun[6] = crc16table(drun, sizeof(drun) - 2) % 256;
	drun[7] = crc16table(drun, sizeof(drun) - 2) / 256;
	Distance1.WriteData(ddis, 17);
	Distance1.WriteData(drun, 8);
	Sleep(position*22/10);
}



void MotorControl::PlatePosition(int position)
{
	int k;
	int speed = 1000;
	k = position;
	position = position - PlateDistance;
	PlateDistance = k;
	if (position < 0)
	{
		ddis[14] = 1;
		position = -position;
	}
	else
	{
		ddis[14] = 0;
	}

	ddis[0] = RotateCom;
	drun[0] = RotateCom;

	ddis[8] = speed % 256;
	ddis[7] = speed / 256;

	ddis[10] = position % 256;
	ddis[9] = (position / 256) % 256;
	ddis[12] = ((position / 256) / 256) % 256;
	ddis[11] = ((position / 256) / 256) / 256;

	ddis[15] = crc16table(ddis, sizeof(ddis) - 2) % 256;
	ddis[16] = crc16table(ddis, sizeof(ddis) - 2) / 256;

	drun[6] = crc16table(drun, sizeof(drun) - 2) % 256;
	drun[7] = crc16table(drun, sizeof(drun) - 2) / 256;
/*
	printf(" value : 0x%x\n", ddis[0]);
	printf(" value : 0x%x\n", ddis[1]);
	printf(" value : 0x%x\n", ddis[2]);
	printf(" value : 0x%x\n", ddis[3]);
	printf(" value : 0x%x\n", ddis[4]);
	printf(" value : 0x%x\n", ddis[5]);
	printf(" value : 0x%x\n", ddis[6]);
	printf(" value : 0x%x\n", ddis[7]);
	printf(" value : 0x%x\n", ddis[8]);
	printf(" value : 0x%x\n", ddis[9]);
	printf(" value : 0x%x\n", ddis[10]);
	printf(" value : 0x%x\n", ddis[11]);
	printf(" value : 0x%x\n", ddis[12]);
	printf(" value : 0x%x\n", ddis[13]);
	printf(" value : 0x%x\n", ddis[14]);
	printf(" value : 0x%x\n", ddis[15]);
	printf(" value : 0x%x\n", ddis[16]);

	printf(" value : 0x%x\n", drun[0]);
	printf(" value : 0x%x\n", drun[1]);
	printf(" value : 0x%x\n", drun[3]);
	printf(" value : 0x%x\n", drun[4]);
	printf(" value : 0x%x\n", drun[5]);
	printf(" value : 0x%x\n", drun[6]);
	printf(" value : 0x%x\n", drun[7]);
*/
	Distance1.WriteData(ddis, 17);

	Distance1.WriteData(drun, 8);

	Sleep(position*7/10);
}

void MotorControl::RelayOn()
{
	Distance1.WriteData(On, 1);
}

void MotorControl::RelayOff()
{
	Distance1.WriteData(Off, 1);
}