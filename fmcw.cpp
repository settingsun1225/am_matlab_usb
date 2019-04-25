#include "fmcw.h"

fmcw::fmcw() {
	b_Transfer = 0;
	i_FrameLength = 100 * 10 * 1e3 * 16;
	b_DeviceExist = 0;
	sOperatingMode = "2TX & 6RX";
	sampleFrequency = 40;
	refClk = 0;
	dataTransRefresh = false;
	dataTranserThread = new DataTransfer(this);
	dataProcessThread = new DataProcess(this);
}

fmcw::~fmcw() {
	if (dataTransferBuffer != NULL)
		delete dataTransferBuffer;
	// todo  判断线程是否关闭
}

void fmcw::setFrameLength(int val) {
	i_FrameLength = val;
}
void fmcw::setOperatingMode(int val) {
	if (val == 0)
		sOperatingMode = "2TX & 3RX";
	else
		sOperatingMode = "2TX & 6RX";
}

bool fmcw::checkUSBDevice() {
	cyOpenTest();
	if (b_DeviceExist == 3)
		return true;
	else
		return false;
}
int fmcw::getFrameLength() {
	return i_FrameLength;
}
void fmcw::setSetting() {
	s_HardwareSetting.SetValue(sampleFrequency, sOperatingMode, refClk);
}
bool fmcw::getData(int* data) {
	while (true) {
		processMutex.lock();
		if (isDataRefreshed()) break;
		processMutex.unlock();
		Sleep(5);
	}
	memcpy(data, ch_data, N_chirp_use*Num_Channels*Num_Chirps*sizeof(int));
	setDataUsed(true);
	processMutex.unlock();
	return true;
}
void fmcw::setDataUsed(bool dataUsed) {
	if (dataUsed)
		dataTransRefresh = false;
	else
		dataTransRefresh = true;
}
bool fmcw::isDataRefreshed() {
	if (dataTransRefresh) return true;
	else return false;
}
void fmcw::processConfig(int N_use_begin_t, int N_total_t, int N_chirp_start_t, int N_chirp_use_t, int Num_Chirps_t, int Num_Channels_t) {
	dataProcessThread->Config(N_use_begin_t, N_total_t, N_chirp_start_t, N_chirp_use_t, Num_Chirps_t, Num_Channels_t);
	N_chirp_use = N_chirp_use_t;
	Num_Channels = Num_Channels_t;
	Num_Chirps = Num_Chirps_t;
}
int cyOpenTest();