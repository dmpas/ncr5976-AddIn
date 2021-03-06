#include "NcrComPort.h"
#include <fstream>

extern int timer_counter = 0;

static void FailLog(const std::string &text)
{
	std::ofstream out("c:/pos/fail.txt", std::ios_base::app | std::ios_base::ate);
	out << text << std::endl;
}


NcrComPort::NcrComPort()
{
}

NcrComPort::NcrComPort(const char *port)
{
    Open(port);
}

void NcrComPort::Close()
{
	if (hPort != 0)
		ClosePort();
}



NcrComPort::~NcrComPort()
{
    if (hPort != 0)
		ClosePort();
}

void NcrComPort::Open(const char *port)
{
    this->Port = port;
	OpenPort();
}

void NcrComPort::FillDcb(DCB &dcb)
{
    dcb.BaudRate = CBR_9600; //9600 Baud
    dcb.ByteSize = 8; //8 data bits
    dcb.Parity = NOPARITY; //no parity
    dcb.StopBits = ONESTOPBIT; //1 stop
}

bool NcrComPort::SendData(const char *data)
{

	if (hPort == 0) {
		if (OpenPort() == -1)
			return false;
	}

    DWORD byteswritten;

    bool retVal = WriteFile(hPort, data, 1, &byteswritten, NULL);

    return retVal;
}

bool NcrComPort::SendByte(int byte)
{
	char b = byte;
	return SendData(&b);
}

int NcrComPort::OpenPort()
{
	if (hPort != 0)
		ClosePort();

    hPort = CreateFileA(
        this->Port,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (!GetCommState(hPort, &dcb))
        return -1;

    FillDcb(dcb);

    if (!SetCommState(hPort, &dcb))
        return -1;
	return 0;
}

void NcrComPort::ClosePort()
{
	CloseHandle(hPort);
	hPort = 0;
}

int NcrComPort::ReadByte()
{
    DCB dcb;
    int retVal;
    BYTE Byte;
    DWORD dwBytesTransferred;
    DWORD dwCommModemStatus;

    HANDLE hPort = CreateFileA(
        this->Port,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (!GetCommState(hPort, &dcb))
        return 0x100;

    FillDcb(dcb);

    if (!SetCommState(hPort,&dcb))
        return 0x100;

    SetCommMask (hPort, EV_RXCHAR | EV_ERR); //receive character event
    WaitCommEvent (hPort, &dwCommModemStatus, 0); //wait for character

    if (dwCommModemStatus & EV_RXCHAR) {
        ReadFile (hPort, &Byte, 1, &dwBytesTransferred, 0); //read 1
        retVal = Byte;
    } else if (dwCommModemStatus & EV_ERR)
        retVal = 0x101;

    CloseHandle(hPort);
    return retVal;

}

void NcrComPort::SetDeviceName(const std::wstring &name)
{
    m_DeviceName = name;
}
const std::wstring &NcrComPort::GetDeviceName() const
{
    return m_DeviceName;
}


void NcrComPort::SetModel(int model)
{
    m_model = model;
}

int NcrComPort::GetModel() const
{
    return m_model;
}

NcrDeviceList::NcrDeviceList()
    : m_CurrentDeviceNumber(-1)
{

}

int NcrDeviceList::GetCurrentDeviceNumber() const
{
    return m_CurrentDeviceNumber;
}

void NcrDeviceList::AddDevice()
{
    push_back(NcrComPort());
    m_CurrentDeviceNumber = size() - 1;
}

NcrComPort &NcrDeviceList::Current()
{
    if (m_CurrentDeviceNumber == -1) {
		FailLog("No current device! (182)");
        throw std::exception();
	}
    return at(m_CurrentDeviceNumber);
}
const NcrComPort &NcrDeviceList::Current() const
{
    if (m_CurrentDeviceNumber == -1) {
		FailLog("No current device! (190)");
        throw std::exception();
	}
    return at(m_CurrentDeviceNumber);
}

void NcrDeviceList::Current(unsigned DeviceNumber)
{
	if (DeviceNumber < size())
		m_CurrentDeviceNumber = DeviceNumber;
}

void NcrDeviceList::DeleteDevice()
{
	if (m_CurrentDeviceNumber < size())
		erase(begin() + m_CurrentDeviceNumber);
	if (m_CurrentDeviceNumber >= size())
		m_CurrentDeviceNumber = size() - 1;
}

int NcrComPort::GetResult() const
{
	return m_Result;
}


void NcrComPort::TurnOn()
{
	SendByte(0x1B);
	SendByte(0x05);

	SendByte(0x1B);
	SendByte(0x0C); // ScreenSave = Off

	m_turned_on = true;
}

void NcrComPort::TurnOff()
{
	SendByte(0x1B);
	SendByte(0x0B); // ScreenSave = On

	m_turned_on = false;
}

bool NcrComPort::TurnedOn() const
{
	return m_turned_on;
}

void NcrComPort::ClearText()
{
	SendByte(0x1B);
	SendByte(0x02);
}

void NcrComPort::SendString(const char *str)
{
	while (*str) {
		SendByte(*str++);
	}
}

void NcrComPort::SendStringLimited(const char *str, int n)
{
	while (*str && n) {
		SendByte(*str++);
		--n;
	}
}

void NcrComPort::SendStringLimitedBlanked(const char *str, int n)
{
	while (*str && n) {
		SendByte(*str++);
		--n;
	}
	if (n) {
		while (n--)
			SendByte(' ');
	}
}

void NcrComPort::SetCursorPos(int Row, int Col)
{
	int X = Row*GetColumnCount() + Col;
	SendByte(0x1B);
	SendByte(0x13);
	SendByte(X);
}

void NcrComPort::SendStringPosThreaded(int row, int column, const char *str)
{
	SetCursorPos(row, column);
	SendString(str);
}

unsigned NcrComPort::GetColumnCount() const
{
	return 20;
}

NcrWindow::NcrWindow()
	: timerId(0)
{
}

int NcrComPort::WindowCount() const
{
	return m_windows.size();
}

static NcrWindow no_window;

NcrWindow &NcrComPort::CurrentWindow()
{
	if (m_current_window < m_windows.size())
		return m_windows[m_current_window];
	/*
	FailLog("No current window! (306)");
	throw std::exception();
	*/
	return no_window;
}

const NcrWindow &NcrComPort::CurrentWindow() const
{
	if (m_current_window < m_windows.size())
		return m_windows[m_current_window];
	/*
	FailLog("No current window! (314)");
	throw std::exception();
	*/
	return no_window;
}

int NcrComPort::CurrentWindowNumber() const
{
	return m_current_window;
}

void NcrComPort::CreateNcrWindow(int Yview, int Xview, int Hview, int Wview, int Hwindow, int Wwindow)
{

	NcrWindow w(Port, Yview, Xview, Hview, Wview, Hwindow, Wwindow);
	m_current_window = m_windows.size();
	m_windows.push_back(w);

}

void NcrComPort::DeleteNcrWindow()
{
	if (m_current_window < m_windows.size()) {
		m_windows.erase(m_windows.begin() + m_current_window);
		if (m_current_window >= m_windows.size())
			m_current_window = m_windows.size() - 1;
	}
}

void NcrComPort::CurrentWindowNumber(int num)
{
	m_current_window = num;
}

NcrWindow::NcrWindow(
		const char *port,
		int Yview,
		int Xview,
		int Hview,
		int Wview,
		int Hwindow,
		int Wwindow)
		:
	port (port),
	Yview (Yview),
	Xview (Xview),
	Hview (Hview),
	Wview (Wview),
	Hwindow (Hwindow),
	Wwindow (Wwindow)

{}




NcrWindow::~NcrWindow()
{
	if (timerId)
		StopMarquee();
}

void NcrWindow::OutputText()
{
}

void NcrWindow::StartMarquee()
{
	if (timerId)
		StopMarquee();
	timerId = AddMarquee(port, Text, Xview, Yview, MarqueeUnitWait, Wwindow);
}

void NcrWindow::StopMarquee()
{
	if (timerId) {
		DeleteMarquee(timerId);
		timerId = 0;
	}
}

void NcrMarqueeData::Step()
{
	NcrComPort D(port);
	D.SetCursorPos(y, x);
	D.SendStringLimitedBlanked(&text.c_str()[pos], width);
	if (pos < text.size())
		++pos;
	else {
		pos = 0;
	}
}

NcrMarqueeData::NcrMarqueeData()
	: timerId(0)
{}

struct MarqueeThreadData {
	HANDLE			ThreadId;
	const char	   *port;
	std::string		text;
	int				x;
	int				y;
	int				W;
	int				ms;
	volatile bool	finish;
};

std::vector<MarqueeThreadData *> Threads;

static DWORD CALLBACK
ThreadTimerProc(LPVOID data)
{
	MarqueeThreadData *tData = static_cast<MarqueeThreadData *>(data);

	NcrMarqueeData M;
	M.port = tData->port;
	M.text = tData->text;
	M.x = tData->x;
	M.y = tData->y;
	M.width = tData->W;

	do {
		M.Step();
		Sleep(tData->ms);
	} while (!tData->finish);

	//delete tData; // ������!
	ExitThread(0);
	return 0;
}

HANDLE AddMarquee(const char *port, const std::string &text, int x, int y, int ms, int W)
{

	if (ms == 0)
		return 0;
	if (text.size() == 0)
		return 0;

	
	if (ms < MIN_DISPLAY_INTERVAL)
		ms = MIN_DISPLAY_INTERVAL;
	
	MarqueeThreadData *M = new MarqueeThreadData();
	M->port = port;
	M->text = text;
	M->x = x;
	M->y = y;
	M->ms = ms;
	M->W = W;
	M->finish = false;

	HANDLE hThread = CreateThread(NULL, 0, ThreadTimerProc, M, CREATE_SUSPENDED, NULL);

	M->ThreadId = hThread;

	Threads.push_back(M);

	ResumeThread(hThread);

	return hThread;
}

void DeleteMarquee(HANDLE timerId)
{
	std::vector<MarqueeThreadData *>::iterator cit;
	for (cit = Threads.begin(); cit != Threads.end(); ++cit)
		if ((*cit)->ThreadId == timerId) {
			(*cit)->finish = true;
			Threads.erase(cit, cit);
			break;
		}
}

void StopAllMarquees()
{
	std::vector<MarqueeThreadData *>::iterator cit;
	for (cit = Threads.begin(); cit != Threads.end(); ++cit)
		(*cit)->finish = true;
}

NcrMarqueeData::NcrMarqueeData(const NcrMarqueeData &src)
	: text(src.text), x(src.x), y(src.y)
	, timerId(src.timerId), port(src.port)
	, width(src.width)
{}

void NcrMarqueeData::Stop()
{
	if (timerId) {
		std::vector<MarqueeThreadData *>::iterator cit;
		for (cit = Threads.begin(); cit != Threads.end(); ++cit)
			if ((*cit)->ThreadId == timerId) {
				(*cit)->finish = true;
				Threads.erase(cit, cit);
				break;
			}
		timerId = 0;
	}
}