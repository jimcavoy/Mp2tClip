// mp2tclip.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Mp2tClipImpl/CommandLineParser.h>
#include <Mp2tClipImpl/Clipper.h>
#include <Mp2tClipImpl/Monitor.h>


#ifdef _WIN32
#include <io.h>
#include <Windows.h>
#endif

#include <iostream>
#include <thread>

#ifdef _WIN32
BOOL CtrlHandler(DWORD fdwCtrlType);
#endif

ThetaStream::Clipper* pClipper;
ThetaStream::Monitor* pMonitor;

using namespace ThetaStream;
using namespace std;

void banner()
{
	std::cerr << "Mp2tClip: MPEG-2 TS Clipper Application v1.0.1" << std::endl;
	std::cerr << "Copyright (c) 2025 ThetaStream Consulting, jimcavoy@thetastream.com" << std::endl;
}

int main(int argc, char* argv[])
{
	try
	{
		banner();

		CommandLineParser cmdline;
		cmdline.parse(argc, argv, "Mp2tClip");

		std::cerr << std::endl << "Enter Ctrl-C to exit" << std::endl << std::endl;

#ifdef _WIN32
		if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)) {
			const DWORD err = GetLastError();
			std::cerr << "Failed to set console control handler.  Error Code = " << err << std::endl;
			return err;
		}
#endif
		Clipper clipper(cmdline);
		pClipper = &clipper;
		Monitor monitor(clipper, cmdline.length());
		pMonitor = &monitor;

		thread clipperThread(&Clipper::operator(), &clipper);
		thread monitorThread(&Monitor::operator(), &monitor);

		clipperThread.join();
		monitorThread.join();
	}
	catch (std::exception & ex)
	{
		cerr << endl << ex.what() << endl;
		return -1;
	}
	catch (...)
	{
		cerr << "Unknown exception thrown" << endl;
		return -1;
	}

	return 0;
}

#ifdef _WIN32
BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal.
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	case CTRL_LOGOFF_EVENT:
		pClipper->stop();
		pMonitor->stop();
		std::cerr << "Closing down, please wait" << std::endl;
		Sleep(1000);
		return TRUE;
	default:
		return FALSE;
	}
}
#endif


