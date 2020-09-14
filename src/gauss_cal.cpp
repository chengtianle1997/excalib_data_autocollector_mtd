#define _CRT_SECURE_NO_WARNINGS

// Define the way to calculate center
//#define GAUSS_CENTER
#define WEIGHT_CENTER

#include "gauss_cal.h"
//#include "conio.h"

GaussCal GausscalPrivate;

GaussCal* GaussCal::guasscalstatic;

GaussCal::GaussCal()
{
	guasscalstatic = this;
}
//calculate the n_th of x
int GaussCal::fx(int x, int n) {
	int y;
	if (n == 0)
	{
		y = 1;
	}
	else if (n == 1) {
		y = x;
	}
	else if (n == 2) {
		y = x * x;
	}
	return y;
}

//generate x and z Cvmat 
int GaussCal::getXZmatrix(CvMat* X, CvMat* Z, int n, GPoint* gpoint) {
	//n个数据点 以n行形式存入
	for (int i = 0; i < n; i++) {
		//顺序存入 1  x  x^2
		//double* xData = (double*)(X->data.ptr + i * X->step);
		//double* zData = (double*)(X->data.ptr + i * X->step);
		for (int j = 0; j < 3; j++) {
			cvmSet(X, i, j, fx(gpoint[i].x, j));
			//xData[j] = fx(gpoint[i].x, j);
			//cout << j << endl;
		}
		//y存入Z矩阵
		cvmSet(Z, i, 0, log(gpoint[i].brightness));
		//zData[0] = log(gpoint[i].brightness);
	}
	return 1;
}

//Sub-Pixel center-line detection algorithm base on GaussFit
void GaussCal::GaussCenter(GaussCalParam& guasscalparam) {
	Mat cloneImage = guasscalparam.matImage.clone();
	//Mat OrgnImage = matImage.clone();
	//先运用canny检测得到初步中心线
	//int g_nCannyLowThreshold = 80;//canny检测低阈值
	//int minCanny = 200;//canny平均点筛选
	//Mat tmpImage, dstImage;
	//blur(cloneImage, tmpImage, Size(3, 3));
	//Canny(tmpImage, dstImage, g_nCannyLowThreshold, g_nCannyLowThreshold * 3);
	//namedWindow("canny function");
	//imshow("canny function", dstImage);
	int Rows = cloneImage.rows;
	int Cols = cloneImage.cols * cloneImage.channels();
	//int *brightness;
	//int threads = 2;//调用线程数
	//brightness = new int[Rows];
	//memset(brightness, 0, Rows);
	//getPeaker1(matImage, point);
#pragma omp parallel for num_threads(guasscalparam.threads)
	for (int i = 0; i < Rows; i++)
	{
		uchar* data = guasscalparam.matImage.ptr<uchar>(i);
		int MaxPixel = data[0];
		int MaxX(0);
		//for (int j = guasscalparam.StartScan; j < guasscalparam.EndScan; j++)
		for (int j = StartScan; j < EndScan; j++)
		{
			if (data[j] > MaxPixel) {
				MaxPixel = data[j];
				MaxX = j;
			}
		}
		guasscalparam.point[i].y = i;
		guasscalparam.point[i].x = MaxX;
		//point[i].bright = MaxPixel;
		guasscalparam.point[i].brightness = MaxPixel;
		//point[i].Rows = Rows;
	}

	//parallel_for(0, Rows / guasscalparam.CalScale, [](int i) {
	//	uchar* data = guasscalparam.matImage.ptr<uchar>(guasscalparam.CalScale*i);
	//	int MaxPixel = data[0];
	//	int MaxX(0);
	//	for (int j = guasscalparam.StartScan; j < guasscalparam.EndScan; j++)
	//	{
	//		if (data[j] > MaxPixel) {
	//			MaxPixel = data[j];
	//			MaxX = j;
	//		}
	//	}
	//	guasscalparam.point[guasscalparam.CalScale*i].y = guasscalparam.CalScale*i;
	//	guasscalparam.point[guasscalparam.CalScale*i].x = MaxX;
	//	//point[i].bright = MaxPixel;
	//	guasscalparam.point[guasscalparam.CalScale*i].brightness = MaxPixel;
	//});
	/*for (int i = 0; i < Rows; i++)
	{
		brightness[i] = point[i].bright;
	}*/
	//int x[100];
	//int px = 0; 


	//int sum = 0;
	//int average = 0;
	////getPeaker(matImage, brightness);
	//for (int j = 0; j < Rows; j++) {
	//	uchar* data = dstImage.ptr<uchar>(j);
	//	uchar* odata = matImage.ptr<uchar>(j);
	//	for (int i = 0; i < Cols; i++) {
	//		int PixelDataof = data[i];
	//		if (PixelDataof > minCanny) {//修改canny检测后的边缘阈值
	//			x[px] = i;
	//			px++;
	//			sum = sum + i;
	//			if (px > 100) {
	//				cout << "there are too many canny points" << endl;
	//			}
	//		}
	//	}
	//	//逐行计算平均点
	//	if (px) {
	//		average = sum * 1.0 / px;
	//	}
	//	point[j].x = average;
	//	point[j].y = j;
	//	brightness[j] = odata[average];
	//	
	//	average = 0;
	//	sum = 0;
	//	px = 0;
	//	memset(x, 0, px);
	//	//cout << "(" << point[j].cx << "," << point[j].cy << ")" << endl;
	//}

	//读取point中的值
//	int Cols = cloneImage.cols;//x
//	int Rows = cloneImage.rows;//y

#ifdef WEIGHT_CENTER
	//逐行存储所有点的x坐标和亮度值以便分析 在此只存入高斯点
#pragma omp parallel for num_threads(guasscalparam.threads)
	for (int i = 0; i < Rows; i++) {
		int PixelData;
		int Pixnum = 0;
		GPoint* gpoint;
		gpoint = new GPoint[Rows];
		Pixnum = 0;
		//高斯点选取 
		//watch.restart();
		guasscalparam.xRange = 10;
		guasscalparam.minError = 0.6;
		uchar* data = guasscalparam.matImage.ptr<uchar>(i);
		for (int j = guasscalparam.point[i].x - guasscalparam.xRange; j <= guasscalparam.point[i].x + guasscalparam.xRange; j++) {
			PixelData = data[j];
			//cout << PixelData << endl;
			//minerror和maxerror条件筛选高斯点  //后期在此处考虑xRange
			//cout << "condition1" << (PixelData > minError*brightness[i]) << endl;
			//cout << "condition2" << (PixelData < ((1 - maxError)*brightness[i]))<<endl;
			//cout << "condition3" << (abs(j - point[i].x) < xRange) << endl;

			if (PixelData > guasscalparam.minError * guasscalparam.point[i].brightness) {
				gpoint[Pixnum].x = j;
				gpoint[Pixnum].brightness = PixelData;
				Pixnum++;
			}
			/*if ((j - point[i].x) > xRange)
				break;*/
		}
		//watch.stop();
		//cout << "高斯点选取耗时:" << watch.elapsed() << endl;
		if (Pixnum > 0) {
			int n = Pixnum;

			//结果计算
			//watch.restart();
			float sum_num = 0, sum_pos = 0;
			for (int m = 0; m < n; m++)
			{
				sum_num += gpoint[m].x * gpoint[m].brightness;
				sum_pos += gpoint[m].brightness;
			}

			guasscalparam.point[i].cx = sum_num / sum_pos;
			guasscalparam.point[i].cy = i;


			//RadialCorrection
			/*double x = (guasscalparam.point[i].cx - guasscalparam.uo) / guasscalparam.fx;
			double y = (guasscalparam.point[i].cy - guasscalparam.vo) / guasscalparam.fy;
			double rsqrt = x * x + y * y;
			double rsqrts = rsqrt * rsqrt;
			guasscalparam.point[i].cx = guasscalparam.uo + (x * (1 + guasscalparam.k1*rsqrt + guasscalparam.k2*rsqrts))*guasscalparam.fx;
			guasscalparam.point[i].cy = guasscalparam.vo + (y * (1 + guasscalparam.k1*rsqrt + guasscalparam.k2*rsqrts))*guasscalparam.fy;*/

			//TangentialCorrection
			/*x = (guasscalparam.point[i].cx - guasscalparam.uo) / guasscalparam.fx;
			y = (guasscalparam.point[i].cy - guasscalparam.vo) / guasscalparam.fy;
			rsqrt = x * x + y * y;
			guasscalparam.point[i].cx = (x + 2 * guasscalparam.p1*x*y + guasscalparam.p2 * (rsqrt + 2 * x*x))*guasscalparam.fx + guasscalparam.uo;
			guasscalparam.point[i].cy = (y + guasscalparam.p1*(rsqrt + 2 * y*y) + 2 * guasscalparam.p2*x*y)*guasscalparam.fy + guasscalparam.vo;*/

			guasscalparam.point[i].bright = gpoint[(int)guasscalparam.point[i].cx].brightness;

			//guasscalparam.point[i].ay = atan((guasscalparam.vo - guasscalparam.point[i].cy) / guasscalparam.fy);
			//guasscalparam.point[i].ay = asin(guasscalparam.ky*sin(atan((guasscalparam.vo - guasscalparam.point[i].cy)/ guasscalparam.fy)));

			//double sDistance = guasscalparam.b*tan(guasscalparam.phi + atan((guasscalparam.uo - guasscalparam.point[i].cx) / guasscalparam.fx)) / cos(guasscalparam.point[i].ay);

			//output the Raw Data  s: x  ay: y
			double sDistance = guasscalparam.point[i].cx;
			guasscalparam.point[i].ay = guasscalparam.point[i].cy;

			if (guasscalparam.point[i].ay < -200 || guasscalparam.point[i].ay>2248)
			{
				guasscalparam.point[i].ay = 0;
			}
			guasscalparam.point[i].s = sDistance;
			if (guasscalparam.point[i].s < 100 || guasscalparam.point[i].s >1700)
			{
				guasscalparam.point[i].s = 0;
			}
			//if (!isnan(guasscalparam.point[i].ay) && sDistance >= RangeMin && sDistance <= RangeMax&& guasscalparam.point[i].ay<AyMax && guasscalparam.point[i].ay>AyMin)
			//{

			//}			
			//else 
			//{
				//guasscalparam.point[i].s = 0;
				//guasscalparam.point[i].ay = 0;
			//}



			//guasscalparam.point[i].s = guasscalparam.b*tan(guasscalparam.phi + asin(guasscalparam.kx*sin(atan((guasscalparam.uo - guasscalparam.point[i].cx) / guasscalparam.fx)))) / cos(guasscalparam.point[i].ay);

			//watch.stop();
			//cout << "结果计算耗时:" << watch.elapsed() << endl;
		}
		else {
			guasscalparam.point[i].cx = 0;
			guasscalparam.point[i].ay = 0;
			guasscalparam.point[i].bright = 0;
			guasscalparam.point[i].ay = 0;
			guasscalparam.point[i].s = 0;
		}
		guasscalparam.point[i].cy = i;
		//printf("(%lf , %lf): %d)\n", guasscalparam.point[i].cx, guasscalparam.point[i].cy, guasscalparam.point[i].brightness);
		delete[]gpoint;
	}
#endif


#ifdef GAUSS_CENTER
	//逐行存储所有点的x坐标和亮度值以便分析 在此只存入高斯点
#pragma omp parallel for num_threads(guasscalparam.threads)
	for (int i = 0; i < Rows; i++) {
		int PixelData;
		int Pixnum = 0;
		GPoint* gpoint;
		gpoint = new GPoint[Rows];
		Pixnum = 0;
		//高斯点选取 
		//watch.restart();
		uchar* data = guasscalparam.matImage.ptr<uchar>(i);
		for (int j = guasscalparam.point[i].x - guasscalparam.xRange; j <= guasscalparam.point[i].x + guasscalparam.xRange; j++) {
			PixelData = data[j];
			//cout << PixelData << endl;
			//minerror和maxerror条件筛选高斯点  //后期在此处考虑xRange
			//cout << "condition1" << (PixelData > minError*brightness[i]) << endl;
			//cout << "condition2" << (PixelData < ((1 - maxError)*brightness[i]))<<endl;
			//cout << "condition3" << (abs(j - point[i].x) < xRange) << endl;

			if (PixelData > guasscalparam.minError * guasscalparam.point[i].brightness && PixelData < ((1 - guasscalparam.maxError) * guasscalparam.point[i].brightness)) {
				gpoint[Pixnum].x = j;
				gpoint[Pixnum].brightness = PixelData;
				Pixnum++;
			}
			/*if ((j - point[i].x) > xRange)
				break;*/
		}
		//watch.stop();
		//cout << "高斯点选取耗时:" << watch.elapsed() << endl;
		if (Pixnum >= 3) {
			int n = Pixnum;
			CvMat* X = cvCreateMat(n, 3, CV_64FC1);
			CvMat* Z = cvCreateMat(n, 1, CV_64FC1);
			//CvMat* XT = cvCreateMat(3, n, CV_64FC1);
			CvMat* B = cvCreateMat(3, 1, CV_64FC1);
			CvMat* SA = cvCreateMat(3, 3, CV_64FC1);
			CvMat* SAN = cvCreateMat(3, 3, CV_64FC1);
			CvMat* SC = cvCreateMat(3, n, CV_64FC1);
			//获取矩阵
			//watch.restart();
			getXZmatrix(X, Z, n, gpoint);
			//watch.stop();
			//cout << "矩阵获取耗时:" << watch.elapsed() << endl;
			//	/*for (int i = 0; i < n; i++) {
			//		for (int j = 0; j < 3; j++) {
			//			cout << cvmGet(X, i, j) << "\t";
			//		}
			//		cout << endl;
			//	}*/
			//	
			//	for (int i = 0; i < 3; i++) {
			//		for (int j = 0; j < n; j++) {
			//			cout << cvmGet(XT, i, j) << "\t";
			//		}
			//		cout << endl;
			//	}*/
			//乘法1
			//watch.restart();
			cvGEMM(X, X, 1, NULL, 0, SA, CV_GEMM_A_T);
			//watch.stop();
			//cout << "乘法1耗时:" << watch.elapsed() << endl;
			//for (int i = 0; i < 3; i++) {
			//		for (int j = 0; j < 3; j++) {
			//			cout << cvmGet(SA, i, j) << "\t";
			//		}
			//		cout << endl;
			//	}*/
			//矩阵求逆
			//watch.restart();
			cvInvert(SA, SAN, CV_LU);  //高斯消去法
			//watch.stop();
			//cout << "矩阵求逆耗时:" << watch.elapsed() << endl;
			/*for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {

					cout << cvmGet(SAN, i, j) << "\t";
				}
				cout << endl;
			}*/
			//矩阵乘法2
			//watch.restart();
			cvGEMM(SAN, X, 1, NULL, 0, SC, CV_GEMM_B_T);
			//watch.stop();
			//cout << "矩阵乘法2耗时:" << watch.elapsed() << endl;
			//	/*for (int i = 0; i < 3; i++) {
			//		for (int j = 0; j < n; j++) {
			//			cout << cvmGet(SC, i, j) << "\t";
			//		}
			//		cout << endl;
			//	}*/
			//	/*for (int i = 0; i < n; i++) {
			//		for (int j = 0; j < 1; j++) {
			//			cout << cvmGet(Z, i, j) << "\t";
			//		}
			//		cout << endl;
			//	}*/
			//矩阵乘法3
			//watch.restart();
			cvGEMM(SC, Z, 1, NULL, 0, B, 0);
			//watch.stop();
			//cout << "矩阵乘法3耗时:" << watch.elapsed() << endl;
			//	/*for (int i = 0; i < 3; i++) {
			//		cout << cvmGet(B, i, 0)<<"\t";
			//	}
			//	cout << endl;*/
			//结果计算
			//watch.restart();
			if (cvmGet(B, 2, 0) == 0)
			{
				guasscalparam.point[i].cx = 0;
				guasscalparam.point[i].cy = i;
			}
			else
			{
				guasscalparam.point[i].cx = (-cvmGet(B, 1, 0)) * 1.0 / (2 * cvmGet(B, 2, 0));
				guasscalparam.point[i].cy = i;
			}

			//RadialCorrection
			/*double x = (guasscalparam.point[i].cx - guasscalparam.uo) / guasscalparam.fx;
			double y = (guasscalparam.point[i].cy - guasscalparam.vo) / guasscalparam.fy;
			double rsqrt = x * x + y * y;
			double rsqrts = rsqrt * rsqrt;
			guasscalparam.point[i].cx = guasscalparam.uo + (x * (1 + guasscalparam.k1*rsqrt + guasscalparam.k2*rsqrts))*guasscalparam.fx;
			guasscalparam.point[i].cy = guasscalparam.vo + (y * (1 + guasscalparam.k1*rsqrt + guasscalparam.k2*rsqrts))*guasscalparam.fy;*/

			//TangentialCorrection
			/*x = (guasscalparam.point[i].cx - guasscalparam.uo) / guasscalparam.fx;
			y = (guasscalparam.point[i].cy - guasscalparam.vo) / guasscalparam.fy;
			rsqrt = x * x + y * y;
			guasscalparam.point[i].cx = (x + 2 * guasscalparam.p1*x*y + guasscalparam.p2 * (rsqrt + 2 * x*x))*guasscalparam.fx + guasscalparam.uo;
			guasscalparam.point[i].cy = (y + guasscalparam.p1*(rsqrt + 2 * y*y) + 2 * guasscalparam.p2*x*y)*guasscalparam.fy + guasscalparam.vo;*/

			guasscalparam.point[i].bright = exp(cvmGet(B, 0, 0) - cvmGet(B, 1, 0) * cvmGet(B, 1, 0) / (4 * cvmGet(B, 2, 0)));

			//guasscalparam.point[i].ay = atan((guasscalparam.vo - guasscalparam.point[i].cy) / guasscalparam.fy);
			//guasscalparam.point[i].ay = asin(guasscalparam.ky*sin(atan((guasscalparam.vo - guasscalparam.point[i].cy)/ guasscalparam.fy)));

			//double sDistance = guasscalparam.b*tan(guasscalparam.phi + atan((guasscalparam.uo - guasscalparam.point[i].cx) / guasscalparam.fx)) / cos(guasscalparam.point[i].ay);

			//output the Raw Data  s: x  ay: y
			double sDistance = guasscalparam.point[i].cx;
			guasscalparam.point[i].ay = guasscalparam.point[i].cy;

			if (guasscalparam.point[i].ay < -200 || guasscalparam.point[i].ay>2248)
			{
				guasscalparam.point[i].ay = 0;
			}
			guasscalparam.point[i].s = sDistance;
			if (guasscalparam.point[i].s < 100 || guasscalparam.point[i].s >1700)
			{
				guasscalparam.point[i].s = 0;
			}
			//if (!isnan(guasscalparam.point[i].ay) && sDistance >= RangeMin && sDistance <= RangeMax&& guasscalparam.point[i].ay<AyMax && guasscalparam.point[i].ay>AyMin)
			//{

			//}			
			//else 
			//{
				//guasscalparam.point[i].s = 0;
				//guasscalparam.point[i].ay = 0;
			//}



			//guasscalparam.point[i].s = guasscalparam.b*tan(guasscalparam.phi + asin(guasscalparam.kx*sin(atan((guasscalparam.uo - guasscalparam.point[i].cx) / guasscalparam.fx)))) / cos(guasscalparam.point[i].ay);

			//watch.stop();
			//cout << "结果计算耗时:" << watch.elapsed() << endl;
			cvReleaseMat(&X);
			cvReleaseMat(&Z);
			cvReleaseMat(&B);
			cvReleaseMat(&SA);
			cvReleaseMat(&SAN);
			cvReleaseMat(&SC);
		}
		else {
			guasscalparam.point[i].cx = 0;
			guasscalparam.point[i].ay = 0;
			guasscalparam.point[i].bright = 0;
			guasscalparam.point[i].ay = 0;
			guasscalparam.point[i].s = 0;
		}
		guasscalparam.point[i].cy = i;
		//printf("(%lf , %lf): %d)\n", guasscalparam.point[i].cx, guasscalparam.point[i].cy, guasscalparam.point[i].brightness);
		delete[]gpoint;
	}
#endif
}

//Automatical Identification for error base on the gate you can choose
void GaussCal::GaussIdentify(GaussIdentifyParam gaussidentifyparam) {
	int Rows = gaussidentifyparam.matImage.rows;//y
	//int Cols = matImage.cols;
	int Cols = gaussidentifyparam.matImage.cols * gaussidentifyparam.matImage.channels();//x
	//int div = 64;
	double error;
	for (int j = 1; j < Rows; j++) {
		//point[j].errorup = point[j].cx - point[j - 1].cx;
		if (abs(gaussidentifyparam.point[j].cx - gaussidentifyparam.point[j - 1].cx) > gaussidentifyparam.doorin) {
			line(gaussidentifyparam.matImage, Point((gaussidentifyparam.point[j].cx - 30), gaussidentifyparam.point[j].cy), Point((gaussidentifyparam.point[j].cx + 30), gaussidentifyparam.point[j].cy), Scalar(255, 100, 100), 2, 8, 0);
			//line(matImage, Point(point[j].cx, point[j].cy - 30), Point(point[j].cx, point[j].cy + 30), Scalar(255, 100, 100), 2, 8, 0);
			error = gaussidentifyparam.point[j].cx - gaussidentifyparam.point[j - 1].cx;
			ostringstream oss;
			oss << error;
			string texterror = oss.str();
			putText(gaussidentifyparam.matImage, texterror, Point(gaussidentifyparam.point[j].cx + 40, gaussidentifyparam.point[j].cy), 2, 1.5, Scalar(255, 100, 100), 1, 8, 0);
		}
	}
	namedWindow("GaussIdentify", 0);
	imshow("GaussIdentify", gaussidentifyparam.matImage);
	cvWaitKey(0);
}

int GaussCal::bound(short i, short a, short b)

{

	return min(max(i, min(a, b)), max(a, b));

}



CvScalar GaussCal::getInverseColor(CvScalar c)

{

	CvScalar s;

	for (int i = 0; i <= 2; ++i)

	{

		s.val[i] = 255 - c.val[i];

	}

	return s;

}


void  GaussCal::on_mouse(int event, int x, int y, int flags, void* ustc)

{

	Point pt;

	Point tmp_pt = { -1,-1 };

	char temp[100];

	Size text_size;

	int baseline;

	guasscalstatic->MouseX = x;

	guasscalstatic->MouseY = y;

	Scalar clrPoint = Scalar(255, 0, 0, 0);

	Scalar clrText = Scalar(255, 255, 255, 0);

	MPoint* point = (MPoint*)ustc;

	//MPoint *point = { 0 };

	if (event == CV_EVENT_MOUSEMOVE)

	{

		GausscalPrivate.dst.copyTo(GausscalPrivate.src);

		//x = bound(x, 0, &src->width - 1);

		//y = bound(y, 0, src->height - 1);

		pt = Point(x, y);

		circle(GausscalPrivate.src, pt, 2, clrPoint, 1, 8, 0);


		//sprintf(temp, "%d (%d,%d)", n + 1, x, y);

		if (y > 1 && y < point[0].Rows && x < (2592 - 35) && x>0)
		{
			sprintf(temp, "Row:%d , center:%f , X:%d , ErrorUp:%f , ErrorDown:%f", y, point[y].cx, x, point[y].cx - point[y - 1].cx, point[y].cx - point[y + 1].cx);


			//getTextSize(temp, 2, &text_size, &baseline);

			//tmp_pt.x = bound(pt.x, 0, src->width - text_size.width);

			//tmp_pt.y = bound(pt.y, text_size.height + baseline, src->height - 1 - baseline);

			tmp_pt = Point(x + 30, y);

			putText(GausscalPrivate.src, temp, tmp_pt, 2, 1, clrText, 1, 8, 0);

			imshow("GaussManuIden", GausscalPrivate.src);
		}

	}

	else if (event == CV_EVENT_LBUTTONDOWN)

	{

		pt = cvPoint(x, y);

		GausscalPrivate.points.push_back(pt); GausscalPrivate.n++;

		circle(GausscalPrivate.src, pt, 2, clrPoint, 1, 8, 0);

		//sprintf(temp, "%d (%d,%d)", n, x, y);

		if (y > 1 && y < point[0].Rows && x < (2592 - 35) && x>0)
		{
			sprintf(temp, "Row:%d , center:%f , X:%d , ErrorUp:%f , ErrorDown:%f", y, point[y].cx, x, point[y].cx - point[y - 1].cx, point[y].cx - point[y + 1].cx);

			//cvGetTextSize(temp, &font, &text_size, &baseline);

			//tmp_pt.x = bound(pt.x, 0, src->width - text_size.width);

			//tmp_pt.y = bound(pt.y, text_size.height + baseline, src->height - 1 - baseline);


			tmp_pt = Point(x + 30, y);

			putText(GausscalPrivate.src, temp, tmp_pt, 2, 1, clrText, 1, 8, 0);

			GausscalPrivate.src.copyTo(GausscalPrivate.dst);

			imshow("GaussManuIden", GausscalPrivate.src);

			cvWaitKey(0);
		}

	}

	else if (event == CV_EVENT_RBUTTONDOWN)

	{
		/*GausscalPrivate.ori.copyTo(GausscalPrivate.src);

		GausscalPrivate.ori.copyTo(GausscalPrivate.dst);

		imshow("GaussManuIden", GausscalPrivate.src);



		while (!GausscalPrivate.points.empty()) {
			GausscalPrivate.points.pop_back();
		}
		GausscalPrivate.n = 0;*/

		//{

		//	

		//	//pt = points.back();



		//	/*circle(src, pt, 2, getInverseColor(clrPoint), CV_FILLED, CV_AA, 0);



		//	sprintf(temp, "%d (%d,%d)", n, pt.x, pt.y); --n;

		//	cvGetTextSize(temp, &font, &text_size, &baseline);

		//	tmp_pt.x = bound(pt.x, 0, src->width - text_size.width);

		//	tmp_pt.y = bound(pt.y, text_size.height + baseline, src->height - 1 - baseline);

		//	cvPutText(src, temp, tmp_pt, &font, getInverseColor(clrText));*/

		//	//src.copyTo(dst);

		//	imshow("src", src);

		//}

	}

}

void GaussCal::GaussManuIdenInit()
{

	//cvNamedWindow("GaussManuIden", 0);

}

//Manual indentification for error by mouse click
void GaussCal::GaussManuIden(GaussManuIdenParam gaussmanuidenparam) {
	//src = imread("lena.jpg", 1);
	GausscalPrivate.src = gaussmanuidenparam.matImage.clone();

	GausscalPrivate.dst = GausscalPrivate.src.clone();

	GausscalPrivate.ori = GausscalPrivate.src.clone();
	//cvNamedWindow("GaussManuIden", 0);
	cvNamedWindow("GaussManuIden", 0);



	Point pt = Point(MouseX, MouseY);

	Scalar clrPoint = Scalar(255, 0, 0, 0);

	Scalar clrText = Scalar(255, 255, 255, 0);

	circle(GausscalPrivate.src, pt, 2, clrPoint, 1, 8, 0);

	char temp[100];

	//sprintf(temp, "%d (%d,%d)", n + 1, x, y);

	if (MouseY > 1 && MouseY < gaussmanuidenparam.point[0].Rows && MouseX < (2592 - 35) && MouseX>0)
	{
		sprintf(temp, "Row:%d , center:%f , X:%d, ErrorUp:%f , ErrorDown:%f", MouseY, gaussmanuidenparam.point[MouseY].cx, MouseX, gaussmanuidenparam.point[MouseY].cx - gaussmanuidenparam.point[MouseY - 1].cx, gaussmanuidenparam.point[MouseY].cx - gaussmanuidenparam.point[MouseY + 1].cx);


		//getTextSize(temp, 2, &text_size, &baseline);

		//tmp_pt.x = bound(pt.x, 0, src->width - text_size.width);

		//tmp_pt.y = bound(pt.y, text_size.height + baseline, src->height - 1 - baseline);

		Point tmp_pt = Point(MouseX + 30, MouseY);

		putText(GausscalPrivate.src, temp, tmp_pt, 2, 1, clrText, 1, 8, 0);

		imshow("GaussManuIden", GausscalPrivate.src);
	}
	else
	{
		imshow("GaussManuIden", GausscalPrivate.src);
	}

	cvSetMouseCallback("GaussManuIden", GausscalPrivate.on_mouse, (void*)gaussmanuidenparam.point);




	//cvWaitKey(100);



	//if (getch()==8)
	//{
	//	cvWaitKey(0);
	//}

	//if (_kbhit()) {//如果有按键按下，则_kbhit()函数返回真
	//	int ch = _getch();//使用_getch()函数获取按下的键值
	//	cout << ch;
	//	if (ch == 8) { cvWaitKey(0); }//当按下ESC时循环，ESC键的键值时27.
	//}
	//else
	//{
	cvWaitKey(50);
	//}


	//cvDestroyAllWindows();

	//GausscalPrivate.src.release();

	//GausscalPrivate.dst.release();



	/*ofstream file("GaussManuIden.txt");

	if (!file)

	{

		cout << "open file error!";

	}

	vector<Point>::iterator it = points.begin();

	for (; it != points.end(); ++it)

	{

		file << it->x << ',' << it->y << endl;

	}

	file << endl;

	file.close();*/
}

