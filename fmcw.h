#pragma once
#include <Windows.h>
#include "CyAPI.h"
#include <iostream>
#include "DataTransfer.h"
#include "Setting.h"
#include <mutex>

using namespace std;

#define PI 3.1415926

class fmcw {
public:
	fmcw();
	~fmcw();
	int start();
	int stop;
	void setFrameLength(int val);
	void setOperatingMode(int val);
	bool checkUSBDevice();
	int getFrameLength();
	void setSetting();
	bool getData(int* data);
	void setDataUsed(bool dataUsed);
	bool isDataRefreshed();
	void processConfig(int N_use_begin_t, int N_total_t, int N_chirp_start_t, int N_chirp_use_t, int Num_Chirps_t, int Num_Channels_t);

	BYTE *dataTransferBuffer;
	int **ch_data;
	mutex transferMutex;
	mutex processMutex;

private:
	DataTransfer* dataTranserThread;
	DataProcess *dataProcessThread;
	Setting s_HardwareSetting;
	bool b_Transfer;
	bool b_Process;
	bool b_Show;
	int i_FrameLength;
	int b_DeviceExist;
	int sampleFrequency;
	int refClk;
	string sOperatingMode;

	int N_chirp_use;
	int Num_Chirps;
	int Num_Channels;
	bool dataTransRefresh;
	int cyOpenTest();
};