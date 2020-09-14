#pragma once
#include "serial_port.h"  
#include <iostream>

using namespace std;



class CAngleSensor
{
public:
	CAngleSensor();
	~CAngleSensor();

public:
	double GetAbsAngleValueF(void);
	double GetRelAngleValueF(void);

	BOOL Init(int port, int baudrate);
	void SetRelZero(void);

private:
	bool OpenUpdateThread();
	bool CloseUpdateThread();
	static UINT WINAPI UpdateThread(void* pParam);


private:
	CSerialPort AngSensorPort;
	
	BOOL	bSerialPortOpened;
	double	abs_angle;
	double	rel_angle;
	double	zero_pos_angle;

	HANDLE	m_hUpdateThread;
	bool	bExitThread;

};