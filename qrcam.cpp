/** @file qrcam.cpp
* Reading QR code using webcam
* and printing at terminal
* only once.
*/

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <zbar.h>

#include "qrmosq.h"

using namespace cv;
using namespace std;
using namespace zbar;

//int main(void)
string qrMqtt::qrcam()
{
	string qrdata;
	VideoCapture cap(0);

	if (!cap.isOpened()){
		cerr << "Cannot open Webcam\n";
		//return -1;
		throw "#FAILED";
	}

	ImageScanner scanner;				// zbar::ImageScanner
	scanner.set_config (ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	while (1){
	Mat frame;							// Mat - opencv n-dimension array class.
	bool bSuccess = cap.read (frame);
	if (!bSuccess){
	  cout << "cannot read a frame video\n";
	  break;
	}

	Mat gray;
	cvtColor (frame, gray, CV_BGR2GRAY);	// convert frame array to grey 
											// as transform RGB space to GRAY.
	int width = frame.cols;
	int height = frame.rows;
	uchar *raw = (uchar *)gray.data;

	Image image(width, height, "Y800", raw, width * height);		// zbar::Image::Image()
																	// Y800 = 8 bit monochrome format.
																	// GRAY also accepted.

	int n = scanner.scan (image);

	for (Image::SymbolIterator symbol = image.symbol_begin();
	  symbol != image.symbol_end(); ++symbol){

	  qrdata = symbol->get_data();
	  cout << "Main Data ->" << qrdata <<endl;

	  }

	  if (!qrdata.empty()){
		break;
	  }

	  }
	return qrdata;
}
