#include "Setting.h"

Setting::Setting() {
	buf1[0] = buf2[0] = 0x20;
	buf1[1] = 0x01;
	buf2[1] = 0x02;
	buf1[2] = 0x00;
	buf2[2] = 0x00;
	buf1[3] = 0x00;
	buf2[3] = 0x00;
	buf1[1] = buf1[1] | 0x20, buf2[1] = buf2[1] | 0x20;

}
void Setting::SetValue(int SampleFrequency, string OperatingMode, int RefClk) {
	if (OperatingMode == "2TX & 3RX")
		buf1[0] = buf2[0] = 0x00;
	else if (OperatingMode == "2TX & 6RX")
		buf1[0] = buf2[0] = 0x20;
	buf1[1] = 0x01;
	buf2[1] = 0x02;
	buf1[2] = 0x00;
	buf2[2] = 0x00;
	buf1[3] = 0x00;
	buf2[3] = 0x00;
	if (RefClk == 1)
		buf1[1] = buf1[1] | 0x20, buf2[1] = buf2[1] | 0x20;

}