#ifndef SETTING_H
#define SETTING_H

#include <iostream>
#include <string>
using namespace std;

class Setting {
public:
	Setting();
	char buf1[4];
	char buf2[4];
	void SetValue(int SampleFrequency, string OperatingMode, int RefClk);
};
#endif