#pragma once

#include "resource.h"

void MainProgram();
void USBCheck();
int CyOpenTest();
void Init();
bool MemInit();
bool MemUninit();
UINT ReadDataThread(LPVOID pParam);
bool ReadDevice(BYTE * usbdata, DWORD & lenBytes);
void GetSetting();
bool USBInit();

// 两个buf
class Setting
{
public:
	char buf1[4];
	char buf2[4];
	void SetValue(int SampleFrequency, int PAChannel, int RefClk);
};
