#pragma once

#define PY_SSIZE_T_CLEA
//#include <Python.h>
#include <string>
#include "Windows.h"
#include <vector>
#include <thread>
#include <future>
#include "cameractrl.h"


using namespace std;

#define IMAGE_SAMPLES	6



struct Coordinate
{
	double x, y;
};

class CrossCenterCalc {
public:
	CrossCenterCalc();
	~CrossCenterCalc();
	
	bool  GetCrossPoint(Camera camera, Coordinate* pCrossPt);
	bool  GetCrossPointMultiThread(Camera camera, Coordinate* pCrossPt);

private:
	Coordinate Calculate(string img_file);
	void CalculateMtd(string img_file, promise<Coordinate> &ccpro);

	string startprocess(LPCWSTR program, LPWSTR args);
	void split(const string& s, vector<string>& tokens, char delim = ' ');

	wchar_t* char2wchar(const char* cchar);

private:


};
