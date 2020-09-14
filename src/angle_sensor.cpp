#include "angle_sensor.h"

CAngleSensor::CAngleSensor()
{
	bSerialPortOpened = FALSE;
	abs_angle = 0;
	rel_angle = 0;
	zero_pos_angle = 0;

}

CAngleSensor::~CAngleSensor()
{
	//if (bSerialPortOpened)
	//{
	//	AngSensorPort.ClosePort();
	//}
}

double CAngleSensor::GetAbsAngleValueF(void)
{
	Sleep(1);
	return abs_angle;
}

double CAngleSensor::GetRelAngleValueF(void)
{
	Sleep(1); 
	rel_angle = abs_angle - zero_pos_angle;
	while (rel_angle < -180)
		rel_angle += 360;
	while(rel_angle > 180)
		rel_angle -= 360;
	return rel_angle;
}

BOOL CAngleSensor::Init(int port, int baudrate)
{
	BOOL bResult;
	bResult = AngSensorPort.InitPort(port, baudrate, 'N', 8, 1, EV_RXCHAR);
	if (!bResult)
	{
		cout << "COM" << port << " init failed!" << endl;
		return FALSE;
	}
	cout << "COM" << port << " init success!" << endl;
	//begin update thread
	OpenUpdateThread();



	return TRUE;
}

void CAngleSensor::SetRelZero(void)
{
	zero_pos_angle = abs_angle;
}


bool CAngleSensor::OpenUpdateThread()
{
	UINT threadId;

	m_hUpdateThread = (HANDLE)_beginthreadex(NULL, 0, UpdateThread, this, 0, &threadId);
	if (!m_hUpdateThread)
	{
		return false;
	}
	return true;
}

bool CAngleSensor::CloseUpdateThread()
{
	if (m_hUpdateThread != INVALID_HANDLE_VALUE)
	{

		bExitThread = true;

		Sleep(10);


		CloseHandle(m_hUpdateThread);
		m_hUpdateThread = INVALID_HANDLE_VALUE;
	}
	return true;
}


UINT __stdcall CAngleSensor::UpdateThread(void * pParam)
{
	CAngleSensor *pAngSen = reinterpret_cast<CAngleSensor*>(pParam);
	string sbuf;
	uint8_t  rcv_char;
	string str_space = "\r\n";
	string sub_s;

	while (!pAngSen->bExitThread)
	{
		pAngSen->AngSensorPort.ReadChar(&rcv_char);
		if (rcv_char)
			sbuf.push_back((char)rcv_char);

		string::size_type pos;
		pos = sbuf.find(str_space);
		if (pos != string::npos)
		{
			sub_s = sbuf.substr(0, pos);
			sbuf.erase(0, pos + 2);
			pAngSen->abs_angle = atof(sub_s.c_str());
		}
		if (pAngSen->abs_angle < -180)
			pAngSen->abs_angle += 360;
		else if (pAngSen->abs_angle > 180)
			pAngSen->abs_angle -= 360;
		Sleep(1);
	}
	return 0;
}
