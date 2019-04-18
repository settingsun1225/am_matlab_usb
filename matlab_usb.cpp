// matlab_usb.cpp : 定义控制台应用程序的入口点。
//

#include "afxwin.h"
#include "stdafx.h"
#include "matlab_usb.h"
#include "CyAPI.h"
#include<vector>;

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_BUFFER_SIZE  1024*64
#define MAX_BUFFER_COUNT 250
#define MAX_BUFFER_MEM 300*1024*1024
#define MAX_BLOCK_SIZE 65536
#define MAX_SAMPLE_LENGTH 160000000

#define ERROR_RESAMPLE 1
#define ERROR_RESET 2
#define ERROR_STOP 3


CString m_sStatus;
CString RunningPath;
int SampleTime;
int PaChannel;

int MaxDataLength;
int DataLength;
char *DataTransferBuffer;// 数据传输缓存
char *DataProcessBuffer;// 数据处理缓存

CCyUSBDevice USBDevice;// USB设备
CCyUSBDevice *g_pUSBDevice = &USBDevice;// 指向USB设备的指针
Setting g_setting;

int bDeviceExist;
bool g_bTransfer;

int ERROR_STATE;
int ERROR_STATE_USB = 0;
int ERROR_COUNT;
bool SUCCESS;

int DataTransferBufferPos = 0;
int MaxTransferBufferSize;
int BufferWritePos;
int BufferUnreadPos;
int BufferUnreadSize;
BYTE usbdata[MAX_BUFFER_SIZE];


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: 更改错误代码以符合您的需要
			_tprintf(_T("错误: MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: 在此处为应用程序的行为编写代码。
			MainProgram();
		}
	}
	else
	{
		// TODO: 更改错误代码以符合您的需要
		_tprintf(_T("错误: GetModuleHandle 失败\n"));
		nRetCode = 1;
	}

	return nRetCode;
}

char * DataReadBuffer;
void MainProgram()
{
	Init();
	USBCheck();
	GetSetting();
	g_setting.SetValue(40, PaChannel, 0);
	g_bTransfer = true;
	MaxDataLength = SampleTime * 10 * 1e3 * 16;
	USBInit();
	MemInit();
	DataReadBuffer = new char[MaxDataLength];
	int UnreadPos = 0;
	FILE* fp = NULL;
	AfxBeginThread(ReadDataThread, NULL, THREAD_PRIORITY_HIGHEST);
	Sleep(100);
	while (g_bTransfer)
	{
		if (PathIsDirectory(RunningPath + "\\temp\\Stop"))
		{
			RemoveDirectory(RunningPath + "\\temp\\Stop");
			if (g_bTransfer)
				g_bTransfer = false;
			continue;
		}
		if (PathIsDirectory(RunningPath + "\\temp\\Begin"))
		{
		//	g_bTransfer = true;
			UnreadPos = BufferUnreadPos;
			if (UnreadPos + DataLength < MAX_SAMPLE_LENGTH)
				memcpy(DataReadBuffer, DataTransferBuffer + UnreadPos, DataLength);
			else
			{
				int size1 = MAX_SAMPLE_LENGTH - UnreadPos;
				int size2 = DataLength - size1;

				memcpy(DataReadBuffer, DataTransferBuffer + UnreadPos, size1);
				memcpy(DataReadBuffer + size1, DataTransferBuffer, size2);
			}
			fopen_s(&fp, RunningPath + "\\data\\data", "wb");
		//	fp = fopen(RunningPath + "\\data\\data", "wb");
			fwrite(DataReadBuffer, DataLength, 1, fp);
			fclose(fp);

			CreateDirectory(RunningPath + "\\temp\\Finish", NULL);
			RemoveDirectory(RunningPath + "\\temp\\Begin");

			Sleep(200);
		}
	}
	delete[] DataReadBuffer;
	MemUninit();
	return;
}


int CyOpenTest()
{
	CCyUSBDevice *USBDevicetest = new CCyUSBDevice();
	int count = USBDevicetest->DeviceCount();

	if (count > 0)
	{
		if (strstr(USBDevicetest->FriendlyName, "SAT-USB") != NULL)
			bDeviceExist = 2;
		else if (strstr(USBDevicetest->FriendlyName, "Streamer") != NULL)
			bDeviceExist = 3;
	}
	else
		bDeviceExist = 0;
	delete USBDevicetest;
	return bDeviceExist;
}

void Init()
{
	char pFileName[MAX_PATH];
	GetModuleFileName(NULL, pFileName, MAX_PATH);
	CString csFullPath(pFileName);
	int npos = csFullPath.ReverseFind('\\');
	RunningPath = csFullPath.Left(npos);


	if (!PathIsDirectory(RunningPath + "\\temp"))
		CreateDirectory(RunningPath + "\\temp", 0);
	if (!PathIsDirectory(RunningPath + "\\data"))
		CreateDirectory(RunningPath + "\\data", 0);
}

void USBCheck()
{
	bDeviceExist = 0;
	bDeviceExist = CyOpenTest();
	CStdioFile IfUSB;
	CString Filename;
	if (bDeviceExist == 3)
	{
		m_sStatus = "Ready (3.0)";
		Filename = RunningPath + "\\temp\\USB3Linked.txt";
		if (IfUSB.Open(Filename, CFile::modeWrite | CFile::modeCreate))
		{
			IfUSB.WriteString("USB3Linked");
			IfUSB.Close();
		}
	}
	else if (bDeviceExist)
	{
		m_sStatus = "Ready";
		Filename = RunningPath + "\\temp\\USBLinked.txt";
		if (IfUSB.Open(Filename, CFile::modeWrite | CFile::modeCreate))
		{
			IfUSB.WriteString("USBLinked");
			IfUSB.Close();
		}
	}
	else
	{
		m_sStatus = "Unavailable";
		Filename = RunningPath + "\\temp\\USBUnlinked.txt";
		if (IfUSB.Open(Filename, CFile::modeWrite | CFile::modeCreate))
		{
			IfUSB.WriteString("USBUnlinked\n");
			IfUSB.Close();
		}
	}
}

void GetSetting()
{
	CString Filename;
	Filename = RunningPath + "\\Setting.txt";
	CStdioFile file;
	file.Open(Filename, CFile::modeRead);
	std::vector<CString> vecResult;
	CString strValue = _T("");
	while (file.ReadString(strValue))
	{
		vecResult.push_back(strValue);
	}
	file.Close();
	if (vecResult.size() != 3)
	{
		return;
	}
	SampleTime = atoi(vecResult[1]);
	PaChannel = atoi(vecResult[2]);
}

bool USBInit()
{
	if (!g_pUSBDevice->IsOpen())
		return false;

	CCyBulkEndPoint *BulkInEpt = NULL;
	BulkInEpt = (CCyBulkEndPoint *)g_pUSBDevice->EndPoints[2];
	int size = BulkInEpt->MaxPktSize;
	BulkInEpt->PktsPerFrame = 16;
	BulkInEpt->TimeOut = 200;

	// Sending 00 06 00 00 for start pulse.
	CCyControlEndPoint *ept = g_pUSBDevice->ControlEndPt;
	ept->Target = TGT_DEVICE;
	ept->ReqType = REQ_CLASS;
	ept->Direction = DIR_TO_DEVICE;
	ept->ReqCode = (UCHAR)0x01;
	ept->Value = 0;
	ept->Index = 0;

	char *pBuf1 = g_setting.buf1;
	LONG iBufSize = 4;
	bool bResult = ept->XferData((PUCHAR)pBuf1, iBufSize);

	// Sending 00 00 00 00 for start pulse (clear FIFO).
	ept->Target = TGT_DEVICE;
	ept->ReqType = REQ_CLASS;
	ept->Direction = DIR_TO_DEVICE;
	ept->ReqCode = (UCHAR)0x01;
	ept->Value = 0;
	ept->Index = 0;
	char *pBuf2 = g_setting.buf2;
	bResult = ept->XferData((PUCHAR)pBuf2, iBufSize);
	return bResult;
}

bool USBUninit()
{
	if (!g_pUSBDevice->IsOpen())
		return false;
	CCyControlEndPoint *ept = g_pUSBDevice->ControlEndPt;
	//	ept->Target = TGT_DEVICE;
	ept->Target = TGT_ENDPT;
	ept->ReqType = REQ_STD;
	//	ept->ReqType = REQ_VENDOR;
	ept->Direction = DIR_TO_DEVICE;
	ept->ReqCode = (UCHAR)0x01;
	ept->Value = 0x0000;
	ept->Index = 0x0081;
	char *pBuf = "ab";
	LONG iBufSize = 0;
	bool bResult = ept->XferData((PUCHAR)pBuf, iBufSize);
	return bResult;
}

void Setting::SetValue(int SampleFrequency, int PAChannel, int RefClk)
{
	if (PAChannel == 1)
		buf1[0] = buf2[0] = 0x00;
	else if (PAChannel == 1234)
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

bool MemInit()
{
	DataTransferBuffer = new char[MAX_SAMPLE_LENGTH];
	DataProcessBuffer = new char[MAX_SAMPLE_LENGTH];
	return true;
}

bool MemUninit()
{
	if (g_bTransfer)
		g_bTransfer = false;
	delete DataTransferBuffer;
	delete DataProcessBuffer;
	DataTransferBuffer = NULL;
	DataProcessBuffer = NULL;
	return true;
}




UINT ReadDataThread(LPVOID pParam)
{
	DWORD lenBytes = 0;
	DataTransferBufferPos = 0;
	BufferUnreadPos = 0;
	BufferUnreadSize = 0;
	//USBInit();
	while (g_bTransfer)
	{
		ReadDevice(usbdata, lenBytes);
		if (!g_bTransfer)
			break;
		if (lenBytes == 0)
			continue;
		if (DataTransferBufferPos + (int)lenBytes <= MAX_SAMPLE_LENGTH)
		{
			memcpy(DataTransferBuffer + DataTransferBufferPos, usbdata, lenBytes);
			DataTransferBufferPos += lenBytes;
		}
		else
		{
			int size1 = MAX_SAMPLE_LENGTH - DataTransferBufferPos;
			int size2 = lenBytes - size1;

			memcpy(DataTransferBuffer + DataTransferBufferPos, usbdata, size1);
			memcpy(DataTransferBuffer, usbdata + size1, size2);
			DataTransferBufferPos += lenBytes;
			DataTransferBufferPos = DataTransferBufferPos % MAX_SAMPLE_LENGTH;
		}
		BufferUnreadSize += lenBytes;
		if (BufferUnreadSize > MaxDataLength)
		{
			DataLength = BufferUnreadSize - lenBytes;
			BufferUnreadPos = DataTransferBufferPos - BufferUnreadSize;
			BufferUnreadPos += MAX_SAMPLE_LENGTH;
			BufferUnreadPos = BufferUnreadPos % MAX_SAMPLE_LENGTH;
			BufferUnreadSize = 0;
		}
	}
	//	USBUninit();
	return 0;
}

bool ReadDevice(BYTE * usbdata, DWORD & lenBytes)
{
	LONG len = MAX_BUFFER_SIZE;
	bool state = true;
	CCyBulkEndPoint *BulkInEpt = NULL;
	BulkInEpt = (CCyBulkEndPoint *)g_pUSBDevice->BulkInEndPt;
	ZeroMemory(usbdata, len);
	if (BulkInEpt->XferData(usbdata, len))
		ERROR_STATE_USB = 0;
	else
	{
		state = false;
		ERROR_STATE_USB = ERROR_RESET;

		g_pUSBDevice->Reset();
		//		USBUninit();
		USBInit();
	}
	lenBytes = len;
	return state;
}