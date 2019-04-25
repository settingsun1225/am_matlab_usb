#ifndef DATETRANSFER_H
#define DATETRANSFER_H
#include <iostream>
#include <string>
#include <thread>
#include "Setting.h"

#define MAX_USBPKT_LENGTH 1024*64

using namespace std;

class DataTransfer :public thread {
public:
	DataTransfer(fmcw *fmcw);
	~DataTransfer();
	void run();
	void Setting_Set(Setting s);
private:
	int i;
	BYTE usbdata[MAX_USBPKT_LENGTH];

};
#endif