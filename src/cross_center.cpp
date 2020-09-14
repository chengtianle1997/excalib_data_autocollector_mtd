#include "cross_center.h"
#include <iostream>
#include "wchar.h"


extern CCameraCtrl CamCtrl;


CrossCenterCalc::CrossCenterCalc()
{

}

CrossCenterCalc::~CrossCenterCalc()
{
	
}

Coordinate CrossCenterCalc::Calculate(string img_file)
{
	Coordinate cc;
	cc.x = -1;
	cc.y = -1;

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	//char szCommandLine[] = "python cross_center.py --file=image2.jpg";

	//wchar_t program[MAX_PATH];// path to the python.exe;
	wchar_t args[MAX_PATH]; // name of python script;
	wchar_t filename[MAX_PATH];
	swprintf(filename, MAX_PATH, L"%hs", img_file.c_str());
	wsprintf(args, L"python.exe cross_center.py --file=%s", filename);

	string s = startprocess(NULL, args);
	if (s.length() > 0)
	{
		vector<string> result;
		split(s, result, ',');
		if (result.size() == 2)
		{
			cc.x = strtof(result[0].c_str(),NULL);
			cc.y = strtof(result[1].c_str(), NULL);
		}
	}

	return cc;
}

void CrossCenterCalc::CalculateMtd(string img_file, promise<Coordinate> &ccpro)
{
	Coordinate cc;

	cc.x = -1;
	cc.y = -1;

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	//char szCommandLine[] = "python cross_center.py --file=image2.jpg";

	//wchar_t program[MAX_PATH];// path to the python.exe;
	wchar_t args[MAX_PATH]; // name of python script;
	wchar_t filename[MAX_PATH];
	swprintf(filename, MAX_PATH, L"%hs", img_file.c_str());
	wsprintf(args, L"python.exe cross_center.py --file=%s", filename);

	string s = startprocess(NULL, args);
	if (s.length() > 0)
	{
		vector<string> result;
		split(s, result, ',');
		if (result.size() == 2)
		{
			cc.x = strtof(result[0].c_str(), NULL);
			cc.y = strtof(result[1].c_str(), NULL);
		}
	}
	ccpro.set_value(cc);
}

bool CrossCenterCalc::GetCrossPoint(Camera camera, Coordinate * pCrossPt)
{
	Mat matimg;
	Coordinate RC[IMAGE_SAMPLES];

	for (int i = 0; i < IMAGE_SAMPLES; i++)
	{
		RC[i].x = 0;
		RC[i].y = 0;
	}

	int valid_count = 0;
	int try_timeout = IMAGE_SAMPLES*5;

	while ((valid_count < IMAGE_SAMPLES) && (try_timeout > 0))
	{
		//capture image and save image
		CamCtrl.GrabImage(&camera, "sample_img.bmp");

		//calculate cross point
		Coordinate cc = Calculate("sample_img.bmp");
		if ((cc.x > 0)&&(cc.y>0))
		{
			RC[valid_count].x += cc.x;
			RC[valid_count].y += cc.y;
			valid_count++;
		}
		try_timeout--;
	}

	if (try_timeout == 0)
		return false;
	else
	{
		double x_avg = 0;
		double y_avg = 0;
		for (int i = 1; i < IMAGE_SAMPLES; i++)
		{
			x_avg += RC[i].x;
			y_avg += RC[i].y;
		}

		pCrossPt->x = x_avg / (IMAGE_SAMPLES-1);
		pCrossPt->y = y_avg / (IMAGE_SAMPLES-1);

		return true;
	}
}

bool CrossCenterCalc::GetCrossPointMultiThread(Camera camera, Coordinate* pCrossPt)
{
	Mat matimg;
	Coordinate RC[IMAGE_SAMPLES];

	for (int i = 0; i < IMAGE_SAMPLES; i++)
	{
		RC[i].x = 0;
		RC[i].y = 0;
	}

	int valid_count = 0;
	int step_count = 0;
	int try_step = 5;
	int try_timeout = IMAGE_SAMPLES * try_step;

	vector <thread> _threads;
	
	while (step_count < try_step)
	{
		vector<promise <Coordinate>> ccpro(IMAGE_SAMPLES);
		vector<future <Coordinate>> ccfu(IMAGE_SAMPLES);
		for (int i = 0; i < IMAGE_SAMPLES; i++)
		{
			char file_img[64];
			sprintf_s(file_img, 64, "sample_img%d.bmp", i);
			//capture image and save image
			CamCtrl.GrabImage(&camera, file_img);
			//promise <Coordinate> ccpro;
			ccfu[i] = ccpro[i].get_future();
			_threads.push_back(thread(&CrossCenterCalc::CalculateMtd, this, file_img, ref(ccpro[i])));
			try_timeout--;
			Sleep(100);
		}
		for (int i = 0; i < IMAGE_SAMPLES; i++)
		{
			_threads[i].join();
			Coordinate cc = ccfu[i].get();
			if ((cc.x > 0) && (cc.y > 0))
			{
				RC[valid_count].x += cc.x;
				RC[valid_count].y += cc.y;
				valid_count++;
			}
		}
		_threads.clear();
		if (valid_count >= IMAGE_SAMPLES)
			break;
		step_count++;
	}

	if (try_timeout == 0)
		return false;
	else
	{
		double x_avg = 0;
		double y_avg = 0;
		for (int i = 1; i < IMAGE_SAMPLES; i++)
		{
			x_avg += RC[i].x;
			y_avg += RC[i].y;
		}

		pCrossPt->x = x_avg / (IMAGE_SAMPLES - 1);
		pCrossPt->y = y_avg / (IMAGE_SAMPLES - 1);

		return true;
	}
}



string CrossCenterCalc::startprocess(LPCWSTR program, LPWSTR args)
{
	const int MY_PIPE_BUFFER_SIZE = 1024;
	string result_str="";

	//initialize pipe
	HANDLE hPipeRead;
	HANDLE hPipeWrite;
	SECURITY_ATTRIBUTES saOutPipe;
	::ZeroMemory(&saOutPipe, sizeof(saOutPipe));
	saOutPipe.nLength = sizeof(SECURITY_ATTRIBUTES);
	saOutPipe.lpSecurityDescriptor = NULL;
	saOutPipe.bInheritHandle = TRUE;

	string s;

	if (CreatePipe(&hPipeRead, &hPipeWrite, &saOutPipe, MY_PIPE_BUFFER_SIZE))
	{
		PROCESS_INFORMATION processInfo;
		::ZeroMemory(&processInfo, sizeof(processInfo));
		STARTUPINFO startupInfo;
		::ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(STARTUPINFO);
		startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		startupInfo.hStdOutput = hPipeWrite;
		startupInfo.hStdError = hPipeWrite;
		startupInfo.wShowWindow = SW_HIDE;

		int ret;
		
		//wchar_t filename[] = L"python.exe cross_center.py --file=image2.jpg";
		ret = ::CreateProcess(NULL, args,
			NULL,  // process security
			NULL,  // thread security
			TRUE,  //inheritance
			NULL,
			NULL,  // no special environment
			NULL,  //default startup directory
			&startupInfo,
			&processInfo);

		::Sleep(200);
		WaitForSingleObject(processInfo.hProcess, 5000);
		if (ret == 0)
		{
			int nRet = GetLastError();
			printf("CreateProcess last error %d \n", nRet);
		}
		else
		{
			DWORD dwReadLen = 0;
			DWORD dwStdLen = 0;
			BOOL bResult = PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwReadLen, NULL);

			if (bResult && dwReadLen > 0)
			{
				char szPipeOut[MY_PIPE_BUFFER_SIZE];
				::ZeroMemory(szPipeOut, sizeof(szPipeOut));
				if (ReadFile(hPipeRead, szPipeOut, dwReadLen, &dwStdLen, NULL))
				{
					s = string(szPipeOut);
					string::size_type idx;
					string key = "CENTER=";
					idx = s.find(key);
					if (idx != string::npos)
					{
						result_str = s.substr(idx+ key.length(),s.length()-key.length());
					}
					else
					{
						printf("cross center not found..\n");
						//system("pause");
					}
						

				}
			}

		}
		if (processInfo.hProcess)
		{
			CloseHandle(processInfo.hProcess);
		}
		if (processInfo.hThread)
		{
			CloseHandle(processInfo.hThread);
		}
	}
	CloseHandle(hPipeRead);
	CloseHandle(hPipeWrite);

	return result_str;
}

void CrossCenterCalc::split(const string & s, vector<string>& tokens, char delim)
{
	tokens.clear();
	auto string_find_first_not = [s, delim](size_t pos = 0) -> size_t {
		for (size_t i = pos; i < s.size(); i++) 
		{
			if (s[i] != delim) return i;
		}
		return string::npos;
	};

	size_t lastPos = string_find_first_not(0);
	size_t pos = s.find(delim, lastPos);
	while (lastPos != string::npos) {
		tokens.emplace_back(s.substr(lastPos, pos - lastPos));
		lastPos = string_find_first_not(pos);
		pos = s.find(delim, lastPos);
	}
}

wchar_t * CrossCenterCalc::char2wchar(const char * cchar)
{
	wchar_t *m_wchar;
	int len = MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), NULL, 0);
	m_wchar = new wchar_t[len + 1];
	MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), m_wchar, len);
	m_wchar[len] = '\0';
	return m_wchar;
}





